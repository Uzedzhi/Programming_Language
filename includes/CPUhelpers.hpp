#ifndef HELPERS_H
#define HELPERS_H

#include <stdio.h>
#include <stdlib.h>

#define DEBUG_FILE_NAME "debug_file.txt"

#ifndef LEVEL_OF_CHECK
#define LEVEL_OF_CHECK 3
#endif // level_of_check

typedef const char * const string;
const size_t CMD_RANGE          = 10;
const size_t MAX_ARR_SIZE       = 500;
const size_t MAX_STR_SIZE       = 300;
const size_t num_of_regs        = 9;
const size_t num_of_labels      = 500;
const double VERSION            = 1.0;
const size_t RAM_SIZE           = 100;
const size_t LABEL_SIZE         = 100;
const size_t SIGN               = 0xB333DEDDAL + 0xC0CAC0LU;
const size_t otstyp             = 10;
const size_t VIEW_OF_CMD        = 4;
const size_t MAX_MIN            = 100;
const size_t MAX_TRAILING_NUM   = 100;
const size_t X_RES              = 5;
const size_t Y_RES              = 2;
const size_t START_VAL          = 15;
const string VIDEO_HEADER_STR   = "; assembly for video by dimooooon";

#define CALC_INSTRUCTIONS(n) \
    n(PUSH, 0)      \
    n(POP, 1)       \
                    \
    n(ADD, 2)       \
    n(SUB, 3)       \
    n(MUL, 4)       \
    n(QROOT, 5)     \
    n(DIV, 6)       \
    n(POW, 7)       \
                    \
    n(PUSHR, 8)     \
    n(POPR, 9)      \
                    \
    n(IN, 10)       \
    n(DUMP, 11)     \
    n(OUT, 12)      \
    n(HLT, 13)      \
                    \
    n(JMP, 14)      \
    n(JB, 15)       \
    n(JE, 16)       \
    n(JA, 17)       \
    n(JBE, 18)      \
    n(JNE, 19)      \
    n(JAE, 20)      \
                    \
    n(UNDEF_CMD, 21)\
    n(DET, 22)      \
    n(ROOT, 23)     \
                    \
    n(CALL, 24)     \
    n(RET, 25)      \
                    \
    n(POPM, 26)     \
    n(PUSHM, 27)    \
                    \
    n(DRAW, 28)     \
    n(MOD, 29)      \
    n(TOINT, 30)    \
    n(PUSHMN, 31)   \
    n(POPMN,  32)   \

#define CALC_REGS(n)    \
    n(RAX, 0)           \
    n(RBX, 1)           \
    n(RCX, 2)           \
    n(RDX, 3)           \
    n(REX, 4)           \
    n(RTX, 5)           \
    n(TRX, 6)           \
    n(DED, 7)           \
    n(UNDEF_REG, 8)

#define INIT_ENUM_VAL(command, value) \
    command = value,
#define INIT_STR_ARRAY(command, value) \
    #command,
enum calcInst_t {
    CALC_INSTRUCTIONS(INIT_ENUM_VAL)
};
enum regs_enum {
    CALC_REGS(INIT_ENUM_VAL)
};
enum compile_type {
    NORMAL_COMPILING = 'N', VIDEO_COMPILING = 'V'
};
enum asmArg_t {
    ZERO_ARG_CMD, REG_CMD, ONE_ARG_CMD, LABEL_ARG_CMD, MEMORY_ARG_CMD
};

struct line_format {
    calcInst_t num_of_command;
    stack_var_t cmd_arg;
    asmArg_t type_of_arg;
};

struct VID {
    bool is_video;
    bool has_audio;
    size_t width;
    size_t height;
    char audio_file_name[MAX_STR_SIZE];
    size_t fps;
};

struct label {
    unsigned long label_hash;
    size_t label_num_of_line;
};

struct dimensions {
    size_t width;
    size_t height;
};

struct RAM {
    double *procRAM;
    size_t capacity;
    size_t size;
};

struct header_t {
    size_t signature;
    double version;
};
struct extra_info {
    size_t sign;
    size_t num_of_lines;
};

string all_regs_str[num_of_regs] = {CALC_REGS(INIT_STR_ARRAY)};
const string error_text[] =    {"your pointer is null", "stack is null", "capacity is invalid number", "size is invalid number",
                                "type of your argument is different from initialized", "something changed region to the left of an array",
                                "something changed region to the right of an array", "some number overflew past limit", "buffer size should be more than 0!",
                                "hash of your function unexpectedly changed, maybe you swapped or edited elements by yourself?", "command is undefined",
                                "file does not exist", "size is invalid", "signature is incorrect COPYRIGHT!!1!1", "Version is imcompatible",
                                "fatal error, cant progress", "no error", "argument to a command is incorrect", "label is incorrect", 
                                "type of your argument is incorrect", "you entered incorrect command, try again",
                                "video division into frames is unsuccessful", "video header is incorrect"};
const string all_commands_text[] = {CALC_INSTRUCTIONS(INIT_STR_ARRAY)};
const static size_t num_of_commands = sizeof(all_commands_text) / sizeof(string);
const string all_hashable_text[] = {CALC_INSTRUCTIONS(INIT_STR_ARRAY) CALC_REGS(INIT_STR_ARRAY)};
string hashes_file_name = "hash/hashed_commands.bin";
bool is_file_exists(const char * file_name);
void nullify_anything_extra(char * buffer, size_t file_size, size_t actually_read);
char * get_buffer_from_file(FILE * fp, size_t file_size);
bool is_in_array(const char * command, size_t size);
size_t get_file_size(FILE * fp);
bool is_same(double a, double b);
asmArg_t get_type_of_arg(int num_of_command);

int max(int a, int b);
int min(int a, int b);

#endif // helpers_h