#ifndef MYLIB_H
#define MYLIB_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "../my_libs/sassert.hpp"
#include "../helpers/helpers.hpp"

#define push_error(ERROR_CODE, ...) \
    sassert(0, ERROR_CODE, ##__VA_ARGS__);

// main structs

const size_t CANAREIKA = 0xB333DEDDAL + 0xC0CAC0LU;
const double FLT_ERR = 1e-6;
struct stack_t;
string format_string = "\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/";
string err_strings[] = {"your ptr is null", "stack is null", "capacity is invalid number", "size is invalid number", "type of your argument is different from initialized", "something changed region to the left of an array", "something changed region to the right of an array", "some number overflew past limit", "buffer size should be more than 0!", "hash of your function unexpectedly changed, maybe you swapped or edited elements by yourself?", "no error"};
string bad = "MEOW!!!";

// error check
#if LEVEL_OF_CHECK > 1
#define STACK_ERR_CHECK(stack, is_pt, ...)               \
    {                                                    \
        int errors = stackErrcheck(stack, is_pt);        \
        if (errors != 0)                                 \
        {                                                \
            stackDump(stack, 1, 0, errors, __VA_ARGS__); \
            stack->hash = get_stack_hash(*stack);        \
            return errors;                               \
        }                                                \
    }
#else
#define STACK_ERR_CHECK(stack, is_pt, ...) ;
#endif // stack_err_sheck

#if LEVEL_OF_CHECK > 1
#define print_canareika_bytes(stack)                  \
    if (is_error_active(errors, ERR_STACK_NULL) == 0) \
    {                                                 \
        printf(BLACK "canareika left -> " BYELLOW);   \
        print_bytes_left_canareika(stack);            \
        printf("\n                  ");               \
        print_bytes_right_canareika(stack);           \
        printf(BLACK " <- canareika right" WHITE);    \
    }
#define print_left_canareika(stack, errors)                                                                                                    \
    {                                                                                                                                          \
        if (is_error_active(errors, ERR_CANAREIKA_LEFT_CHANGE))                                                                                \
        {                                                                                                                                      \
            print_with_otstyp(otstyp, "    *[-1]: %lf" RED " [LEFT CANAREIKA CHANGE] %s" WHITE " %p\n", (double)*stack->raw, bad, stack->raw); \
        }                                                                                                                                      \
        else                                                                                                                                   \
        {                                                                                                                                      \
            print_with_otstyp(otstyp, "    *[-1]: %lf" YELLOW " [LEFT CANAREIKA]" WHITE " %p\n", (double)*stack->raw, stack->raw);             \
        }                                                                                                                                      \
    }
#define print_right_canareika(stack, errors)                                                                                                                                                            \
    {                                                                                                                                                                                                   \
        if (is_error_active(errors, ERR_CANAREIKA_RIGHT_CHANGE))                                                                                                                                        \
        {                                                                                                                                                                                               \
            print_with_otstyp(otstyp, "    *[%zu]: %lf" RED " [RIGHT CANAREIKA CHANGE] %s" WHITE " %p\n", stack->capacity, (double)stack->stack[stack->capacity], bad, stack->stack + stack->capacity); \
        }                                                                                                                                                                                               \
        else                                                                                                                                                                                            \
        {                                                                                                                                                                                               \
            print_with_otstyp(otstyp, "    *[%zu]: %lf" YELLOW " [RIGHT CANAREIKA]" WHITE " %p\n", stack->capacity, (double)stack->stack[stack->capacity], stack->stack + stack->capacity);             \
        }                                                                                                                                                                                               \
    }
#define check_if_canareika_correct(stack)                        \
    stack_var_t left_canareika = *stack->raw;                    \
    stack_var_t right_canareika = stack->stack[stack->capacity]; \
    if (!is_same(left_canareika, (stack_var_t)CANAREIKA))        \
        errors |= 1 << ERR_CANAREIKA_LEFT_CHANGE;                \
    if (!is_same(right_canareika, (stack_var_t)CANAREIKA))       \
        errors |= 1 << ERR_CANAREIKA_RIGHT_CHANGE;
#else
#define check_if_canareika_correct(stack) ;
#define print_canareika(stack, errors) ;
#define print_right_canareika(stack, errors) ;
#define print_canareika_bytes(stack) ;
#define print_left_canareika(stack, errors) ;
#endif // print_canareika_bytes

#if LEVEL_OF_CHECK > 2
#define get_total_bytes(stack, first_expr, second_expr) \
    stack->total_bytes = first_expr + second_expr;      \
    if (check_if_overflow(first_expr, second_expr))     \
        push_error(ERR_OVERFLOW, "realloc didnt make it... overflow with %zu bytes", stack->total_bytes);
#define check_if_hash_correct(stack)                                                       \
    {                                                                                      \
        if (get_stack_hash(*stack) != stack->hash)                                         \
            sassert(0, ERR_HASH_CHANGED, "FATAL ERROR, cant progress. Your hash changed"); \
    }
#else
#define check_if_hash_correct(stack) (void *)0
#define get_total_bytes(stack, first_expr, second_expr) \
    stack->total_bytes = first_expr + second_expr;
#endif // get_total_bytes and chekc it

// initialization
#define init_stack(stk, buffer_size)                                                           \
    sassert(buffer_size > 0, ERR_BUFFER_SIZE_INVALID, "buffer size should be more than zero"); \
    stack_t *stk = create_stack(#stk, buffer_size, __FILE__, __func__, __LINE__);

// centralize
#if LEVEL_OF_CHECK > 1
#define print_with_otstyp(otstyp, str, ...) \
    {                                       \
        for (size_t k = 0; k < otstyp; k++) \
            putchar(' ');                   \
        printf(str, ##__VA_ARGS__);         \
    }
#else
#define print_with_otstyp(...) ;
#endif

// main pop
#define stackPop(stack, value) \
    stackPop_internal(stack, value, __FILE__, __func__, __LINE__);

// main push
#define stackPush(stack, value) \
    stackPush_internal(stack, value, __FILE__, __func__, __LINE__);

#define STACKDUMP(stack, is_end, print_raw, errors) \
    stackDump(stack, is_end, print_raw, errors, __FILE__, __func__, __LINE__);

// declarations
stack_t *create_stack(string var_name, size_t buffer_size,
                      const char *file_name, const char *func_name, size_t line);
size_t min(size_t a, size_t b);
size_t get_stack_hash(stack_t stack);
size_t mod_pow(size_t base, size_t exp, size_t mod);
int stackErrcheck(stack_t *stack, bool is_pt);
int stackPush_internal(stack_t *stack, stack_var_t value,
                       const char *file_name, const char *func_name, size_t line);
int stackPop_internal(stack_t *stack, stack_var_t *value, const char *file_name, const char *func_name, size_t line);
void stackDtor(stack_t *stack);
stack_var_t get_top(stack_t *stack);
size_t get_size(stack_t *stack);
void print_all_reasons(int errors);
void print_part_of_var_info(stack_t *stack, size_t start, size_t end, size_t is_occupied);
void print_whole_var_info(stack_t *stack);
void print_bytes_left_canareika(stack_t *stack);
void print_bytes_right_canareika(stack_t *stack);
int stackDump(stack_t *stack, int is_end, int print_raw, int errors,
              const char *file_name, const char *func_name, size_t line);
void reallocate_stack(stack_t *, double);
bool check_if_overflow(size_t first, size_t second);
bool is_error_active(int errors, asmErr_t error);

#endif // mylib.h