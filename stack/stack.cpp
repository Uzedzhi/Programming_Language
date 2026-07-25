#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <cstdint>

#include "../stack/stack.hpp"
#include "../helpers/helpers.hpp"

static int count = 1;

const static size_t POWER = 1007;
const static size_t POISON = 0x2134256754321123;
const static size_t MAX_CAPACITY = 0xffffff;
const static size_t MAX_PRINT_ROWS = 5;
const static size_t strlen_format_string = strlen(format_string);
const static size_t num_of_errors = sizeof(err_strings) / sizeof(err_strings[0]);

struct stack_t
{
#if LEVEL_OF_CHECK > 0
    const char *func_name;
    const char *file_name;
    const char *var_name;
    size_t hash;
#endif
    size_t line;
    stack_var_t *stack;
    stack_var_t *raw;
    size_t total_bytes;
    size_t var_size;
    size_t size;
    size_t capacity;
};

// function bodies
size_t min(size_t a, size_t b)
{
    return (a < b) ? a : b;
}

bool is_same(double a, double b) {
    return abs(a - b) < FLT_ERR;
}

stack_t *create_stack(string var_name, size_t buffer_size,
                      const char *file_name, const char *func_name, size_t line)
{
    sassert(file_name, ERR_PTR_NULL);
    sassert(func_name, ERR_PTR_NULL);

    stack_t *stk = (stack_t *)calloc(1, sizeof(stack_t));
    sassert(stk != NULL, ERR_PTR_NULL);

#if LEVEL_OF_CHECK > 1
    stk->func_name = func_name;
    stk->file_name = file_name;
    stk->var_name = var_name;
#endif
    stk->line = line;
    stk->capacity = buffer_size;
    stk->size = 0;
    stk->var_size = sizeof(stack_var_t);

    stk->raw = (stack_var_t *)calloc(buffer_size + 2, sizeof(stack_var_t));
    sassert(stk->raw != NULL, ERR_PTR_NULL);

    stk->stack = stk->raw + 1;
    for (size_t i = 0; i < stk->capacity; i++)
    {
        stk->stack[i] = (stack_var_t)POISON;
    }
    *stk->raw = (stack_var_t)CANAREIKA;
    stk->stack[stk->capacity] = (stack_var_t)CANAREIKA;

#if LEVEL_OF_CHECK > 1
    stk->hash = get_stack_hash(*stk);
#endif
    return stk;
}

stack_var_t get_top(stack_t *stack) {
    return stack->stack[stack->size - 1];
}

size_t get_size(stack_t *stack) {
    return stack->size;
}

bool check_if_overflow(size_t first, size_t second) {
    return second > SIZE_MAX - first;
}

void print_whole_var_info(stack_t *stack) {
    sassert(stack, ERR_PTR_NULL);

    print_part_of_var_info(stack, 0, stack->size, 1);
    if (stack->capacity - stack->size > MAX_PRINT_ROWS)
    {
        print_part_of_var_info(stack, stack->size, stack->size + MAX_PRINT_ROWS, 0);
        print_with_otstyp(otstyp + 20, "...\n");
    }
    else
        print_part_of_var_info(stack, stack->size, stack->capacity, 0);
}

void print_part_of_var_info(stack_t *stack, size_t start, size_t end, size_t is_occupied) {
    sassert(stack, ERR_PTR_NULL);

    char occupied_symbol = (is_occupied) ? '*' : ' ';
    for (size_t i = start; i < end; i++)
    {
        print_with_otstyp(otstyp, "    %c[%zu] = %lf", occupied_symbol, i, (double)stack->stack[i]);
        for (size_t j = 0; j < stack->var_size; j++)
        {
            printf(" [%02X]", *((unsigned char *)(stack->stack + i) + j));
        }
        printf(" %p", stack->stack + i);
        if (is_same(stack->stack[i], (stack_var_t)POISON))
            printf(MAGENTA " [POISON]" WHITE);
        putchar('\n');
    }
}

void print_bytes_right_canareika(stack_t *stack)
{
    sassert(stack, ERR_PTR_NULL);

    for (size_t i = 0; i < 3 * sizeof(stack_var_t); i++)
    {
        if (i == 2 * sizeof(stack_var_t))
        {
            printf(" | " BYELLOW);
        }
        printf("[%02X]", *((unsigned char *)(stack->stack + stack->capacity - 2) + i));
    }
}

void print_bytes_left_canareika(stack_t *stack)
{
    sassert(stack, ERR_PTR_NULL);

    for (size_t i = 0; i < 3 * sizeof(stack_var_t); i++)
    {
        if (i == sizeof(stack_var_t))
        {
            printf(WHITE " | ");
        }
        printf("[%02X]", *((unsigned char *)stack->raw + i));
    }
}

bool is_error_active(int errors, asmErr_t error)
{
    return (errors & (1 << error)) == 1 << error;
}

int stackDump(stack_t *stack, int is_end, int print_raw, int errors, const char *file_name, const char *func_name, size_t line)
{
    sassert(stack, ERR_PTR_NULL);
    sassert(file_name, ERR_PTR_NULL);
    sassert(func_name, ERR_PTR_NULL);
    sassert(is_error_active(errors, ERR_PTR_NULL) == 0, ERR_PTR_NULL);
    if (print_raw == true)
    {
        for (size_t i = 0; i < stack->capacity; i++)
        {
            stack_var_t value = stack->stack[i];
            if (i % 10 == 0)
            {
                printf("\n");
                print_with_otstyp(otstyp, "    ");
            }
            if (i == stack->size - 1)
                printf(GREEN);
            if (is_same(stack->stack[i], POISON))
                printf("[PSN] ");
            else
                printf("[%-3d] ", (int)stack->stack[i]);
            printf(WHITE);
        }
        printf("\n");
        return 0;
    }

    printf(RED "%sSTART DUMP%s" WHITE " (count=%d)\n", format_string, format_string, count++);
    print_with_otstyp(otstyp, "reasons : ");
    print_all_reasons(errors);
    print_with_otstyp(otstyp, "var stack      | <%s>\n", stack->var_name);
    print_with_otstyp(otstyp, "HEX stack      | <%p>\n", stack->stack);
    print_with_otstyp(otstyp, "your type bytes| <%zu> ", sizeof(stack_var_t));
    (is_error_active(errors, ERR_DIFFERENT_TYPE)) ? printf(RED "<-needed %zu bytes %s\n" WHITE, stack->var_size, bad) : printf("\n");
    print_with_otstyp(otstyp, "init where     | <%s; %s:%zu>\n", stack->file_name, stack->func_name, stack->line);
    print_with_otstyp(otstyp, "dump where     | <%s; %s:%zu>\n", file_name, func_name, line);
    print_with_otstyp(otstyp, "var size       | <%zu> " RED "%s\n" WHITE, stack->size, (is_error_active(errors, ERR_SIZE_INVALID)) ? bad : "\n");
    print_with_otstyp(otstyp, "var capacity   | <%zu> " RED "%s\n" WHITE, stack->capacity, (is_error_active(errors, ERR_CAPACITY_INVALID)) ? bad : "\n");

    BEGIN
    if (is_error_active(errors, ERR_STACK_NULL))
        break;

    print_with_otstyp(otstyp, "var (%d bytes) values {\n", (int)(stack->var_size * stack->capacity));

    print_left_canareika(stack, errors);
    if (is_error_active(errors, ERR_SIZE_INVALID))
    {
        print_part_of_var_info(stack, 0, min(stack->capacity, MAX_PRINT_ROWS), 0);
        print_with_otstyp(3 * otstyp, "...\n");
    }
    else
        print_whole_var_info(stack);
    print_right_canareika(stack, errors);

    print_with_otstyp(otstyp, "   };\n");
    print_with_otstyp(otstyp, "}\n");

    print_canareika_bytes(stack);
    END

        if (is_end)
            printf(RED "\n%sENDING DUMP%s\n\n" WHITE, format_string, format_string);
    return 0;
}

void reallocate_stack(stack_t *stack, double multiplier)
{
    sassert(stack != NULL, ERR_PTR_NULL);

    *(stack->stack + stack->capacity) = 0x0;

    if ((double)SIZE_MAX / (double)stack->capacity <= multiplier)
        push_error(ERR_OVERFLOW, "buffer overflew, maybe you have to many elements in a stack?");

    stack->capacity = (size_t)((double)stack->capacity * multiplier);
    stack_var_t *temp_stack = (stack_var_t *)realloc(stack->raw, (stack->capacity + 2) * sizeof(stack_var_t));
    sassert(temp_stack != NULL, ERR_PTR_NULL);

    stack->raw = temp_stack;
    stack->stack = stack->raw + 1;
    for (size_t i = stack->size; i < stack->capacity; i++)
    {
        stack->stack[i] = (stack_var_t)POISON;
    }

    stack->stack[stack->capacity] = (stack_var_t)CANAREIKA;
}

void print_all_reasons(int errors)
{
    for (size_t i = 0; i < num_of_errors; i++)
    {
        if ((errors & 1) == 1)
            printf("<%s> ", err_strings[i]);
        errors >>= 1;
    }
    putchar('\n');
}

int stackPush_internal(stack_t *stack, stack_var_t value, const char *file_name, const char *func_name, size_t line)
{
    sassert(stack, ERR_PTR_NULL);

#if LEVEL_OF_CHECK > 1
    sassert(file_name, ERR_PTR_NULL);
    sassert(func_name, ERR_PTR_NULL);
    STACK_ERR_CHECK(stack, 0, file_name, func_name, line);
#endif

    if (stack->size >= stack->capacity)
    {
        reallocate_stack(stack, 2);
    }

    stack->stack[stack->size++] = value;

#if LEVEL_OF_CHECK > 1
    stack->hash = get_stack_hash(*stack);
    STACK_ERR_CHECK(stack, 0, file_name, func_name, line);
#endif

    return 0;
}

int stackPop_internal(stack_t *stack, stack_var_t *value, const char *file_name, const char *func_name, size_t line)
{
    sassert(stack, ERR_PTR_NULL);
    sassert(value, ERR_PTR_NULL);

#if LEVEL_OF_CHECK > 1
    sassert(file_name, ERR_PTR_NULL);
    sassert(func_name, ERR_PTR_NULL);
    STACK_ERR_CHECK(stack, 1, file_name, func_name, line);
#endif

    *value = stack->stack[--stack->size];
    stack->stack[stack->size] = (stack_var_t)POISON;

#if LEVEL_OF_CHECK > 1
    stack->hash = get_stack_hash(*stack);
    STACK_ERR_CHECK(stack, 0, file_name, func_name, line);
#endif
    return 0;
}

size_t mod_pow(size_t base, size_t exp, size_t mod)
{
    size_t result = 1;
    while (exp > 0)
    {
        if (exp % 2 == 1)
        {
            result = (result * base) % mod;
        }
        base = (base * base) % mod;
        exp /= 2;
    }
    return result;
}

size_t get_stack_hash(stack_t stack)
{
    size_t hash = 0;
    for (size_t i = 0; i < sizeof(stack_t) - sizeof(size_t); i++)
    {
        hash += mod_pow((size_t)*((char *)&stack + i) + i, POWER, MOD);
    }
    return hash;
}

void stackDtor(stack_t *stack)
{
    if (stack != NULL)
    {
        if (stack->raw != NULL)
            free(stack->raw);
        free(stack);
    }
}

int stackErrcheck(stack_t *stack, bool is_pt)
{
    int errors = 0;
    BEGIN
    if (stack->stack == NULL || stack->raw == NULL)
    {
        errors |= 1 << ERR_PTR_NULL;
        break;
    }

    check_if_canareika_correct(stack);

    if (stack->capacity > MAX_CAPACITY)
        errors |= 1 << ERR_CAPACITY_INVALID;
    if (stack->size > stack->capacity)
        errors |= 1 << ERR_SIZE_INVALID;
    if (is_pt == 1 && stack->size - 1 > stack->capacity)
    {
        errors |= 1 << ERR_SIZE_INVALID;
        stack->size--;
    }
    END return errors;
}