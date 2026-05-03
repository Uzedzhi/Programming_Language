#include <stdio.h>
#include <stdlib.h>
#include "../my_libs/FlagParser.h"

enum {
    INPUT_FLAG      = 0,
    OUTPUT_FLAG     = 1,
    ASSEMBLY_FLAG   = 2,
    TREE_FLAG       = 3
};
static const char *AllFlagsStr[]        = {"input", "output", "assembly", "tree"};
static const size_t AllFlagsCount       = sizeof(AllFlagsStr) / sizeof(AllFlagsStr[0]);
static const int AllFlagsNumValues[]    = {1, 1, 1, 1};
static const size_t MAX_STR_SIZE        = 300;


void FreeFlags(Flag_t *FlagParser) {
    for (size_t i = 0; i < AllFlagsCount; i++) {
        free(FlagParser[i].FlagValues);
    }
}

int main(int argc, char *argv[]) {
    FlagParseCtor(FlagParser, AllFlagsCount, AllFlagsStr, AllFlagsNumValues);
    int parseRes = ParseAllFlags(FlagParser, argc, argv, AllFlagsCount, AllFlagsStr, AllFlagsNumValues);
    if (parseRes != 0) {
        for (size_t i = 0; i < AllFlagsCount; i++) free(FlagParser[i].FlagValues);
        return -1;
    }
    if (!FlagParser[0].IsActive) {
        printf("введите файл для компиляции");
        return -1;
    }
    const char *inputFile       = FlagParser[INPUT_FLAG].FlagValues[INPUT_FLAG];

    bool MakeAssembly           = FlagParser[ASSEMBLY_FLAG].IsActive;
    bool MakeTree               = FlagParser[TREE_FLAG].IsActive;
    bool MakeOutput             = FlagParser[OUTPUT_FLAG].IsActive;
    const char *AssemblyFile    = (MakeAssembly == true)    ? FlagParser[ASSEMBLY_FLAG].FlagValues[0]   : "outassembly.txt";
    const char *OutputFile      = (MakeOutput == true)      ? FlagParser[OUTPUT_FLAG].FlagValues[0]     : "out.txt";
    const char *TreeFile        = (MakeTree == true)        ? FlagParser[TREE_FLAG].FlagValues[0]       : "outtree.txt";

    char command[MAX_STR_SIZE] = {};
    snprintf(command, sizeof(command), "./bin/Lang %s %s", inputFile, TreeFile);
    if (system(command) != 0) {
        FreeFlags(FlagParser);
        return -1;
    }

    snprintf(command, sizeof(command), "./bin/ProgramToAsm %s %s", TreeFile, AssemblyFile);
    if (system(command) != 0) {
        FreeFlags(FlagParser);
        return -1;
    }

    snprintf(command, sizeof(command), "./bin/Assembly %s %s", AssemblyFile, OutputFile);
    if (system(command) != 0) {
        FreeFlags(FlagParser);
        return -1;
    }

    if (!MakeAssembly) remove(AssemblyFile);
    if (!MakeTree) remove(TreeFile);

    FreeFlags(FlagParser);
    return 0;
}