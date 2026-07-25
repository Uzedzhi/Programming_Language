#include <cstdint>
#include "../stack/stack.hpp"
#include "../helpers/helpers.hpp"
#include "error_manage.hpp"

bool is_file_exists(const char *file_name)
{
    sassert(file_name, ERR_PTR_NULL);

    FILE *fp = fopen(file_name, "r");
    if (fp == NULL)
    {
        add_error(ERR_FILE_DOES_NOT_EXIST, "none");
        return 0;
    }
    fclose(fp);
    return 1;
}

bool is_same(double a, double b) {
    return abs(a - b) < FLT_ERR;
}

void nullify_anything_extra(char *buffer, size_t file_size, size_t actually_read)
{
    sassert(buffer != NULL, ERR_PTR_NULL);

    while (actually_read < file_size)
    {
        buffer[++actually_read] = '\0';
    }
}

void *reallocate_array(void **array, size_t capacity, size_t new_bytes)
{
    sassert(array != NULL, ERR_PTR_NULL);

    if ((double)SIZE_MAX / (double)capacity <= (double)new_bytes / (double)capacity)
        push_error(ERR_OVERFLOW, "buffer overflew, maybe you have to many elements in a stack?");

    void *new_array = realloc(*array, new_bytes);
    sassert(new_array != NULL, ERR_PTR_NULL);

    *array = new_array;
    return *array;
}

char *get_buffer_from_file(FILE *fp, size_t file_size)
{
    sassert(fp, ERR_PTR_NULL);

    char *compile_buffer = (char *)calloc(file_size + 1, sizeof(char));
    sassert(compile_buffer, ERR_PTR_NULL);

    size_t actually_read = fread(compile_buffer, sizeof(char), file_size, fp);
    nullify_anything_extra(compile_buffer, file_size, actually_read);

    return compile_buffer;
}

asmArg_t get_type_of_arg(int num_of_command)
{
    switch (num_of_command)
    {
    case PUSH:
    case POW:
    case DRAW:
    case PUSHMN:
    case POPMN:
        return ONE_ARG_CMD;
    case JMP:
    case JB:
    case JE:
    case JAE:
    case JNE:
    case JBE:
    case JA:
    case CALL:
        return LABEL_ARG_CMD;
    case PUSHR:
    case POPR:
        return REG_CMD;
    case PUSHM:
    case POPM:
        return MEMORY_ARG_CMD;
    default:
        return ZERO_ARG_CMD;
    }
}

int max(int a, int b)
{
    return (a > b) ? a : b;
}

int min(int a, int b)
{
    return (a < b) ? a : b;
}

size_t get_file_size(FILE *fp)
{
    sassert(fp, ERR_PTR_NULL);

    fseek(fp, 0, SEEK_END);
    size_t file_size = (size_t)ftell(fp);
    rewind(fp);

    return file_size;
}

bool is_in_array(const char *command, size_t size)
{
    sassert(command, ERR_PTR_NULL);

    for (size_t i = 0; i < size; i++)
    {
        if (strcmp(command, all_commands_text[i]) == 0)
            return true;
    }

    return false;
}