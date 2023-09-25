#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <limits.h>

typedef int elem_t;
typedef struct{
    elem_t * data = NULL;
    size_t size = 0;
    size_t capacity = 0;
}Stack;

#define POISON INT_MIN
const size_t increasing_capacity = 2;

size_t StackVerification(Stack *stk);
#define STACK_CTOR(stk, capacity) StackCtor(stk, capacity, __LINE__, __PRETTY_FUNCTION__, __FILE__)
#define STACK_DUMP(stk, errors) StackDump(stk, errors, #stk, __LINE__, __PRETTY_FUNCTION__, __FILE__)

typedef struct{
    int line_init = POISON;
    const char *func_init = NULL;
    const char *file_init = NULL;
}StackDebugInfo;

StackDebugInfo stk_d_info;


#define INIT_CAPACITY 25

enum err{
    ERR_SIZE_CAPACITY  =  1,
    ERR_DATA           =  2,
    ERR_STACK           =  4,
    ERR_CAPACITY       =  8,
    ERR_POP            = 16
};

size_t StackVerification(Stack *stk)
{
    int errors = 0;

    if(stk->size > stk->capacity)
        errors |= ERR_SIZE_CAPACITY;
    if(stk->data == NULL)
        errors |= ERR_DATA;
    if(stk == NULL)
        errors |= ERR_STACK;
    if(!stk->capacity)
        errors |= ERR_CAPACITY;

    return errors;
}

size_t StackCtor(Stack *stk, size_t capacity, int line_init, const char *func_init, const char *file_init)
{
    stk_d_info.line_init = line_init;
    stk_d_info.func_init = func_init;
    stk_d_info.file_init = file_init;
    stk->data = (elem_t *)calloc(capacity, sizeof(elem_t));
    stk->size = 0;
    stk->capacity = capacity;

    return StackVerification(stk);
}

size_t StackDtor(Stack *stk)
{
    free(stk->data);
    stk->size = 0;
    stk->capacity = 0;
    stk = NULL;
    
    return 0;
}

size_t StackRealloc(Stack *stk)
{
    if(stk->size == stk->capacity){
        stk->capacity *= increasing_capacity;
        stk->data = (elem_t *)realloc(stk->data, stk->capacity * sizeof(elem_t));
    }
    if(stk->capacity > stk->size * increasing_capacity * 2 && stk->capacity > INIT_CAPACITY){
        stk->capacity /= increasing_capacity;
        stk->data = (elem_t *)realloc(stk->data, stk->capacity * sizeof(elem_t));
    }
    return StackVerification(stk);
}

size_t StackPush(Stack *stk, elem_t value)
{
    StackRealloc(stk);
    stk->data[stk->size++] = value;
    
    return StackVerification(stk);
}

size_t StackPop(Stack *stk, elem_t *ret_value)
{
    if (!stk->size)
        return StackVerification(stk) | ERR_POP;

    *ret_value = stk->data[--(stk->size)];
    StackRealloc(stk);
    stk->data[stk->size] = POISON;
    
    return StackVerification(stk);
}




void StackDump(Stack *stk, size_t errors, const char *stk_name, int line_err, 
                const char *func_err, const char *file_err)
{
    if (errors)
        printf("ERRORS: \n");
    if(errors & ERR_SIZE_CAPACITY)
        printf("Warring capacity < size\n");

    printf("STACK[%p] \'%s\' from %s(%d) %s called by %s(%d) %s\n", stk, stk_name, stk_d_info.file_init, stk_d_info.line_init, stk_d_info.func_init, file_err, line_err, func_err);
    printf("size = %ld\n", stk->size);
    printf("capacity = %ld\n", stk->capacity);
    printf("data[%p]\n", stk->data);
    for(int i = 0; i < stk->capacity; i++)
        printf("%c[%d] = %d\n", (i < stk->size || stk->data[i] != POISON) ? '*' : ' ', i, stk->data[i]);
}

int main() {
    elem_t ret_value = 0;
    Stack stk = {};
    STACK_CTOR(&stk, INIT_CAPACITY);
    for(int i = 0; i < 50; i++)
        StackPush(&stk, i);
    StackPop(&stk, &ret_value);
    STACK_DUMP(&stk, 0);
    StackDtor(&stk);
}
