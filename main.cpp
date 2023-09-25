#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <limits.h>

typedef int elem_t;
typedef unsigned long long canary_t;
unsigned long long canary_value = 0XB00B5;
#define POISON INT_MIN
const size_t increasing_capacity = 2;


typedef struct{
    canary_t left_canary = canary_value;
    elem_t * data = NULL;
    size_t size = 0;
    size_t capacity = 0;
    canary_t right_canary = canary_value;
}Stack;

typedef struct{
    int line_init = POISON;
    const char *func_init = NULL;
    const char *file_init = NULL;
}DebugInfo;

DebugInfo stk_d_info;




size_t StackVerification(Stack *stk);
void StackDump(Stack *stk, size_t errors, const char *stk_name, int line_err, 
                const char *func_err, const char *file_err);
#define STACK_CTOR(stk, capacity) StackCtor(stk, capacity, __LINE__, __PRETTY_FUNCTION__, __FILE__)
#define STACK_DUMP(stk, errors) StackDump(stk, errors, #stk, __LINE__, __PRETTY_FUNCTION__, __FILE__)




#define INIT_CAPACITY 25

enum err{
    ERR_SIZE_CAPACITY     =   1,
    ERR_DATA              =   2,
    ERR_STACK             =   4,
    ERR_CAPACITY          =   8,
    ERR_POP               =  16,
    ERR_LEFT_STR_CANARY   =  32,
    ERR_RIGHT_STR_CANARY  =  64,
    ERR_LEFT_DATA_CANARY  = 128,
    ERR_RIGHT_DATA_CANARY = 256,
};













size_t StackVerification(Stack *stk)
{
    int errors = 0;

    if(stk->size > stk->capacity)
        errors |= ERR_SIZE_CAPACITY;
    if(!stk->data)
        errors |= ERR_DATA;
    if(!stk)
        errors |= ERR_STACK;
    if(!stk->capacity)
        errors |= ERR_CAPACITY;
    if(stk->left_canary != canary_value)
        errors |= ERR_LEFT_STR_CANARY;
    if(stk->right_canary != canary_value)
        errors |= ERR_RIGHT_STR_CANARY;
    if(*((canary_t *)stk->data - 1) != canary_value)
        errors |= ERR_LEFT_DATA_CANARY;
    if(*(canary_t *)(stk->data + stk->capacity) != canary_value)
        errors |= ERR_RIGHT_DATA_CANARY;
    return errors;
}

size_t StackCtor(Stack *stk, size_t capacity, int line_init, const char *func_init, const char *file_init)
{
    stk_d_info.line_init = line_init;
    stk_d_info.func_init = func_init;
    stk_d_info.file_init = file_init;

    stk->left_canary = canary_value;
    stk->right_canary = canary_value;

    stk->data = (elem_t *)calloc(capacity * sizeof(elem_t) + 2 * sizeof(canary_t), sizeof(char));
    stk->capacity = capacity;
    stk->size = 0;
    
    *(canary_t *)stk->data = canary_value;
    stk->data = (elem_t *)((canary_t *)stk->data + 1);
    *(canary_t *)(stk->data + stk->capacity) = canary_value;  

    return StackVerification(stk);
}

size_t StackDtor(Stack *stk)
{
    free((canary_t *)stk->data - 1);
    stk->size = 0;
    stk->capacity = 0;
    stk = NULL;
    
    return 0;
}

size_t StackRealloc(Stack *stk)
{
    canary_t right_old_canary = *(canary_t *)((elem_t *)stk->data + stk->capacity);
    
    //right_old_canary = canary_value;

    stk->data = (elem_t *)((canary_t *)stk->data - 1);
    
    if(stk->size == stk->capacity){
        stk->capacity *= increasing_capacity;

        stk->data = (elem_t *)realloc(stk->data, 
            stk->capacity * sizeof(elem_t) + 2 * sizeof(canary_t));
        *((canary_t *)((char *)stk->data + 
            + stk->capacity * sizeof(elem_t) + sizeof(canary_t))) = right_old_canary;
    }

    if(stk->capacity > stk->size * increasing_capacity * 2 && stk->capacity > INIT_CAPACITY){
        stk->capacity /= increasing_capacity;

        stk->data = (elem_t *)realloc(stk->data, 
            stk->capacity * sizeof(elem_t) + 2 * sizeof(canary_t));
        
        *((canary_t *)((char *)stk->data + 
            + stk->capacity * sizeof(elem_t) + sizeof(canary_t))) = right_old_canary;
    }
    
    stk->data = (elem_t *)((canary_t *)stk->data + 1);
    
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
    //STACK_DUMP(stk, StackVerification(stk));
    return StackVerification(stk);
}




void StackDump(Stack *stk, size_t errors, const char *stk_name, int line_err, 
                const char *func_err, const char *file_err)
{    
    if(errors)
        printf("ERRORS: \n");
    if(errors & ERR_SIZE_CAPACITY)
        printf("capacity < size\n");
    if(errors & ERR_DATA)
        printf("Pointer to data == NULL\n");
    if(errors & ERR_STACK)
        printf("Pointer to Stack structure == NULL\n");
    if(errors & ERR_CAPACITY)
        printf("Pointer to capacity == NULL\n");
    if(errors & ERR_POP)
        printf("pop try to take, but size == 0\n");
    if(errors & ERR_LEFT_STR_CANARY)
        printf("Left canary (of struct) changes\n");
    if(errors & ERR_RIGHT_STR_CANARY)
        printf("Right canary (of struct) changes\n");
    if(errors & ERR_LEFT_DATA_CANARY)
        printf("Left canary (of data) changes\n");
    if(errors & ERR_RIGHT_DATA_CANARY)
        printf("Right canary (of data) changes\n");
    
    printf("STACK[%p] \'%s\' from %s(%d) %s called by %s(%d) %s\n", stk, stk_name, stk_d_info.file_init, stk_d_info.line_init, stk_d_info.func_init, file_err, line_err, func_err);
    
    printf("Left canary (of struct) = [%0llX]\n", stk->left_canary);
    printf("Right canary (of struct) = [%0llX]\n", stk->right_canary);
    
    printf("Left canary (of data) = [%0llX]\n", *((canary_t *)stk->data - 1));
    printf("Right canary (of data) = [%0llX]\n", *(canary_t *)(stk->data + stk->capacity));

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
