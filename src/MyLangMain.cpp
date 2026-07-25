#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <sys/wait.h>

#include "../my_libs/sassert.hpp"

const size_t MAX_STR_SIZE       = 300;
const size_t NUM_OF_FILE_NAMES  = 4;

typedef const char * const string;

void print_help(char * str) {
    printf("Usage: %s [-h|--help] [-i|--input <file_name>] [-a|--assembly <file_name>][-t|--tree <file_name>]\n", str);
}

enum FlagErr_t {
    ERR_PTR_NULL        = -1,
    OK                  = 0,
    ERR_INVALID_FLAGS   = 1,
    ERR_COMMAND_FAILED  = 2,
    ERR_FATAL_ERROR     = 3
};

enum FlagNames {
    INPUT_FLAG      = 0,
    OUTPUT_FLAG     = 1,
    ASSEMBLY_FLAG   = 2,
    TREE_FLAG       = 3
};

FlagErr_t ExecuteProgram(string program, string file1, string file2) {
    sassert(program, ERR_PTR_NULL);
    sassert(file1, ERR_PTR_NULL);
    sassert(file2, ERR_PTR_NULL);

    pid_t pid = fork();
    if (pid < 0) { // failed
        return ERR_COMMAND_FAILED;
    } else if (pid == 0) { // child
        char buf[MAX_STR_SIZE] = {};
        snprintf(buf, MAX_STR_SIZE - 1, "./bin/%s", program);

        char *argv[] = {buf, (char *) file1, (char *) file2, NULL};
        execvp(argv[0], argv);

        // if return then error
        exit(ERR_COMMAND_FAILED);
    } else { // parent
        int status = 0;
        waitpid(pid, &status, 0);

        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            return OK;
        }

        return ERR_COMMAND_FAILED;
    }
}

int main(int argc, char *argv[]) {
    int opt;
    
    static struct option long_options[] = {
        {"input",       required_argument, 0, 'i'},
        {"output",      required_argument, 0, 'o'},
        {"assembly",    required_argument, 0, 'a'},
        {"tree",        required_argument, 0, 't'},
        {"help",        required_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    char *values[NUM_OF_FILE_NAMES] = {};
    while ((opt = getopt_long(argc, argv, "hi:o:a:t:", long_options, NULL)) != -1) {
        switch (opt) {
            case 'h':
                print_help(argv[0]);
                return OK;
            case 'i':
                values[INPUT_FLAG]      = optarg;
                break;
            case 'o':
                values[OUTPUT_FLAG]     = optarg;
                break;
            case 'a':
                values[ASSEMBLY_FLAG]   = optarg;
                break;
            case 't':
                values[TREE_FLAG]       = optarg;
                break;
            default:
                exit(ERR_FATAL_ERROR);
        }
    }

    int count = 0;
    for (int i = optind; i < argc && i < optind + NUM_OF_FILE_NAMES; i++) {
        if (values[count] == NULL)
            values[count] = argv[i];
        count++;
    }

    RET_ASSERT(values[INPUT_FLAG], ERR_INVALID_FLAGS, "укажите имя файла для компиляции");
    
    bool MakeAssembly           = values[ASSEMBLY_FLAG];
    bool MakeTree               = values[TREE_FLAG];
    const char *InputFile       = values[INPUT_FLAG];
    const char *TreeFile        = (values[TREE_FLAG])        ? values[TREE_FLAG]       : "tree.txt";
    const char *OutputFile      = (values[OUTPUT_FLAG])      ? values[OUTPUT_FLAG]     : "a.out";
    const char *AssemblyFile    = (values[ASSEMBLY_FLAG])    ? values[ASSEMBLY_FLAG]   : "asm.asm";

    RET_ASSERT(ExecuteProgram("Lang",          InputFile,      TreeFile)        == OK, ERR_COMMAND_FAILED, "модуль Lang завершился с ошибкой!");
    RET_ASSERT(ExecuteProgram("Assembly",      AssemblyFile,   OutputFile)      == OK, ERR_COMMAND_FAILED, "модуль Assembly завершился с ошибкой!");
    RET_ASSERT(ExecuteProgram("ProgramToAsm",  TreeFile,       AssemblyFile)    == OK, ERR_COMMAND_FAILED, "модуль ProgramToAsm завершился с ошибкой!");

    if (!MakeAssembly)  remove(AssemblyFile);
    if (!MakeTree)      remove(TreeFile);

    return OK;
}
