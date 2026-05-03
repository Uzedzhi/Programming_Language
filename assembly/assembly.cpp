#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "error_manage.h"
#include "sassert.h"
#include "assembly.h"
#include "../helpers/helpers.h"
#include "videotoasm.h"

error_t error = {};

bool isallowed(char ch)
{
    return strchr(" \t\r\n;{}[]:", ch) == NULL;
}

static unsigned long sdbm(const char *str)
{
    unsigned long hash = 0;
    int c = 0;

    while ((c = *str++) != '\0' && isallowed(c))
    {
        hash = c + (hash << 6) + (hash << 16) - hash;
    }

    return hash;
}

int strcmp_until(const char *str1, const char *str2)
{
    while (isallowed(*str1) && (*str1 == *str2))
    {
        str1++;
        str2++;
    }
    if (!isallowed(*str1) && !isallowed(*str2))
        return 0;
    return *str1 - *str2;
}

calcInst_t get_num_of_command(asm_t *assembly, char *command)
{
    sassert(command, ERR_PTR_NULL);

    unsigned long cur_hash = sdbm(command);
    for (size_t i = 0; i < num_of_commands; i++)
    {
        if (assembly->hashes_of_cmd[i] == cur_hash)
        {
            return (calcInst_t)i;
        }
    }
    return UNDEF_CMD;
}

regs_enum get_reg_type(asm_t *assembly, char *reg)
{
    sassert(reg, ERR_PTR_NULL);

    unsigned long cur_hash = sdbm(reg);
    for (size_t i = 0; i < num_of_regs; i++)
    {
        if (assembly->hashes_of_regs[i] == cur_hash)
        {
            return (regs_enum)i;
        }
    }
    return UNDEF_REG;
}

bool die_internal(const char *file, const char *func, size_t line)
{
    print_error(error_text);
    sassert(0, FATAL_ERROR, "fatal error. aborting at %s:%s, %zu", file, func, line);
    return 0;
}

error_t get_label_index(char *file_buffer, asm_t *assembly, size_t *label_value_index, size_t cycle_count)
{
    bool is_okay = false;

    unsigned long cur_hash = sdbm(file_buffer + 1);
    for (size_t i = 0; i < assembly->label_capacity; i++)
    {
        if (cur_hash == assembly->labels[i].label_hash)
        {
            *label_value_index = i;
            return error;
        }
    }
    if (cycle_count == 1)
    {
        add_error(ERR_INCORRECT_LABEL, "none");
    }
    return error;
}

error_t place_command_argument(asm_t *assembly, size_t cycle_count, char *file_buffer, stack_var_t *value, asmArg_t type_of_arg)
{
    sassert(file_buffer, ERR_PTR_NULL);
    sassert(value, ERR_PTR_NULL);

    if (type_of_arg == ZERO_ARG_CMD)
        return error;

    (file_buffer = strpbrk(file_buffer, " \t\r\n")) != NULL || die();
    if (file_buffer[0] != ' ' && file_buffer[0] != '\t')
    {
        add_error(ERR_INCORRECT_ARGUMENT, "nonen");
        return error;
    }

    file_buffer++;
    if (*file_buffer == '[')
        file_buffer++;

    *value = atof(file_buffer);
    if (isalpha(*file_buffer))
    {
        *value = get_reg_type(assembly, file_buffer);
    }
    if (*file_buffer == ':')
    {
        size_t label_value_index = 0;
        get_label_index(file_buffer, assembly, &label_value_index, cycle_count);
        if (error.is_error == true)
            return error;
        *value = assembly->labels[label_value_index].label_num_of_line;
    }
    if (*file_buffer == '\'')
        *value = file_buffer[1];
    return error;
}

size_t get_num_of_lines(char *file_buffer)
{
    sassert(file_buffer, ERR_PTR_NULL);

    size_t count = 1;
    for (size_t i = 0; file_buffer[i] != '\0'; i++)
    {
        if (file_buffer[i] == '\n' && file_buffer[i + 1] != '\n' && file_buffer[i + 1] != '\r')
            count++;
    }
    return count;
}

void hash_cmds_to_file(const char * FileName) {
    sassert(FileName, ERR_PTR_NULL);

    FILE *fp = fopen(FileName, "wb");
    sassert(fp, ERR_PTR_NULL);

    for (size_t i = 0; i < sizeof(all_hashable_text) / sizeof(all_hashable_text[0]); i++) {
        unsigned long hash = sdbm(all_hashable_text[i]);
        fwrite(&hash, sizeof(unsigned long), 1, fp);
    }

    fclose(fp);
}

error_t init_asm_internal(asm_t *assembly, char *user_file_compile)
{
    sassert(user_file_compile, ERR_PTR_NULL);
    if (!is_file_exists(user_file_compile))
    {
        add_error(ERR_FILE_DOES_NOT_EXIST, "%s", user_file_compile);
        return error;
    }

    FILE *fp = fopen(user_file_compile, "r");
    sassert(fp, ERR_PTR_NULL);

    size_t file_size = get_file_size(fp);

    if (!is_file_exists(hashes_file_name)) {
        hash_cmds_to_file(hashes_file_name);
    }
    FILE *fp_hashes = fopen(hashes_file_name, "rb");
    sassert(fp_hashes, ERR_PTR_NULL);

    fread(&assembly->hashes_of_cmd, sizeof(unsigned long), num_of_commands, fp_hashes);
    fread(&assembly->hashes_of_regs, sizeof(unsigned long), num_of_regs, fp_hashes);
    fclose(fp_hashes);

    assembly->labels = (label *)calloc(num_of_labels, sizeof(label));
    assembly->label_capacity = num_of_labels;
    assembly->file_buffer = get_buffer_from_file(fp, file_size);
    assembly->num_of_lines = get_num_of_lines(assembly->file_buffer);
    assembly->bytecode_buffer = (line_format *)calloc(assembly->num_of_lines + 1, sizeof(line_format));
    sassert(assembly->bytecode_buffer, ERR_PTR_NULL);

    fclose(fp);
    return error;
}

void place_label(asm_t *assembly, char *file_buffer, size_t num_of_line)
{
    sassert(file_buffer, ERR_PTR_NULL);
    sassert(assembly, ERR_PTR_NULL);
    unsigned long cur_hash = sdbm(file_buffer + 1);
    size_t i = 0;
    for (i = 0; i < assembly->label_capacity; i++)
    {
        if (assembly->labels[i].label_hash == 0)
        {
            assembly->labels[i].label_hash = cur_hash;
            assembly->labels[i].label_num_of_line = num_of_line;
            break;
        }
    }
    if (i == assembly->label_capacity - 1)
    {
        reallocate_array((void **)&assembly->labels, assembly->label_capacity, assembly->label_capacity * 2 * sizeof(label));
        assembly->labels[assembly->label_capacity].label_hash = cur_hash;
        assembly->labels[assembly->label_capacity].label_num_of_line = num_of_line;
        assembly->label_capacity *= 2;
    }
}

void skip_all_spaces(char **file_buffer)
{
    sassert(file_buffer, ERR_PTR_NULL);
    while (**file_buffer == ' ' || **file_buffer == '\t')
    {
        (*file_buffer)++;
    }
}

bool correct_video_header(char *start_of_file)
{
    if (strncmp(start_of_file, VIDEO_HEADER_STR, strlen(VIDEO_HEADER_STR)) == 0)
        return true;
    return false;
}

error_t scan_header(asm_t *assembly)
{
    if (correct_video_header(assembly->file_buffer))
    {

        assembly->video.is_video = true;
        size_t scanned_cnt = sscanf(assembly->file_buffer, "%*[^\n]\n; width = %zu, height = %zu, fps = %d, has audio = %d\n", &(assembly->video.width), &(assembly->video.height), &(assembly->video.fps), &(assembly->video.has_audio));
        if (scanned_cnt != 4)
        {
            add_error(ERR_INCORRECT_VIDEO_HEADER, "none");
            return error;
        }

        if (assembly->video.has_audio)
        {
            scanned_cnt = sscanf(assembly->file_buffer, "%*[^\n]\n%*[^\n]\n; audio file name = %s\n", &(assembly->video.audio_file_name));

            if (scanned_cnt != 1)
            {
                add_error(ERR_INCORRECT_VIDEO_HEADER, "none");
                return error;
            }
        }
    }
    return error;
}

error_t compile_file(asm_t *assembly)
{
    sassert(assembly, ERR_PTR_NULL);
    error.is_error == false || die();

    scan_header(assembly);
    if (error.is_error)
        return error;

    for (size_t i = 0; i < 2; i++)
    {
        size_t num_of_line = 0;
        char *file_buffer = assembly->file_buffer - 1;
        while (file_buffer++ != NULL)
        {
            skip_all_spaces(&file_buffer);
            switch (file_buffer[0])
            {
            case ':':
                place_label(assembly, file_buffer, num_of_line);
            case COMMENT:
            case FIGURE_END:
            case FIGURE_START:
            case '\n':
            case '\r':
            case '\0':
                file_buffer = strchr(file_buffer, '\n');
                continue;
            }

            line_format *cur_line = &(assembly->bytecode_buffer[num_of_line]);
            calcInst_t num_of_command = get_num_of_command(assembly, file_buffer);

            if (num_of_command == UNDEF_CMD)
            {
                add_error(ERR_UNDEFINED_CMD, "none");
                return error;
            }
            cur_line->num_of_command = num_of_command;
            cur_line->type_of_arg = get_type_of_arg(cur_line->num_of_command);
            place_command_argument(assembly, i, file_buffer, &(cur_line->cmd_arg), cur_line->type_of_arg);
            if (error.is_error == 1)
                return error;

            file_buffer = strchr(file_buffer, '\n');
            num_of_line++;
        }
    }
    return error;
}

error_t place_bytecode(asm_t *assembly, char *output_file)
{
    sassert(assembly, ERR_PTR_NULL);
    sassert(output_file, ERR_PTR_NULL);
    (error.is_error == false) || die();

    FILE *fp = fopen(output_file, "wb");
    sassert(fp, ERR_PTR_NULL);

    header_t header = {SIGN, VERSION};
    size_t count = 0;

    fwrite(&header, sizeof(header_t), 1, fp);
    fwrite(&(assembly->num_of_lines), sizeof(size_t), 1, fp);
    fwrite(&(assembly->video.is_video), sizeof(bool), 1, fp);
    if (assembly->video.is_video)
    {
        fwrite(&(assembly->video.width), sizeof(size_t), 1, fp);
        fwrite(&(assembly->video.height), sizeof(size_t), 1, fp);
        fwrite(&(assembly->video.fps), sizeof(size_t), 1, fp);
        fwrite(&(assembly->video.has_audio), sizeof(bool), 1, fp);

        if (assembly->video.has_audio)
        {
            size_t len_audio_file = strlen(assembly->video.audio_file_name);
            fwrite(&len_audio_file, sizeof(size_t), 1, fp);
            fwrite(&(assembly->video.audio_file_name), sizeof(char), len_audio_file, fp);
        }
    }
    fwrite(assembly->bytecode_buffer, sizeof(line_format), assembly->num_of_lines, fp);
    fclose(fp);

    return error;
}

void print_help()
{
    printf(MAGENTA "type 1) file you want to compile(needed)\n"
                   "     2) file where you want to compile(optional)\n" WHITE);
}

void asmDtor(asm_t **assembly)
{
    sassert(assembly, ERR_PTR_NULL);
    BEGIN
    if (*assembly == NULL)
        break;
    if ((*assembly)->bytecode_buffer != NULL)
        free((*assembly)->bytecode_buffer);
    if ((*assembly)->file_buffer != NULL)
        free((*assembly)->file_buffer);
    if ((*assembly)->labels != NULL)
        free((*assembly)->labels);
    free(*assembly);
    END
}