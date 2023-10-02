#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <limits.h>
#include <cstring>

#define INIT_CAPACITY 25
#define POISON INT_MIN


typedef int elem_t;
typedef unsigned long long canary_t;

typedef struct{
    canary_t left_canary = 0;
    elem_t * data = NULL;
    size_t size = 0;
    size_t capacity = 0;
    int hash_value = 0;
    int hash_struct = 0;
    bool is_stack_destroyed = false;
    int hash_data = 0;
    canary_t right_canary = 0;
}Stack;

typedef struct{
    int line_init = POISON;
    const char *func_init = NULL;
    const char *file_init = NULL;
}DebugInfo;

#define STACK_CTOR(stk, capacity) StackCtor(stk, capacity,  \
                    __LINE__, __PRETTY_FUNCTION__, __FILE__)
#define STACK_DUMP(stk, errors) StackDump(stk, errors, #stk,\
                    __LINE__, __PRETTY_FUNCTION__, __FILE__)

enum err{
    ERR_SIZE_CAPACITY     =    1,
    ERR_DATA              =    2,
    ERR_STACK             =    4,
    ERR_CAPACITY          =    8,
    ERR_POP               =   16,
    ERR_LEFT_STR_CANARY   =   32,
    ERR_RIGHT_STR_CANARY  =   64,
    ERR_LEFT_DATA_CANARY  =  128,
    ERR_RIGHT_DATA_CANARY =  256,
    ERR_DATA_HASH         =  512,
    ERR_STR_HASH          = 1024,
    ERR_STACK_DTOR        = 2048
};

typedef struct{
    err stk_err;
    const char * msg;
}ErrorHandler;

int StackVerification(Stack *stk);


size_t StackCtor(Stack *stk, size_t capacity, int line_init,
                const char *func_init, const char *file_init);

int StackDtor(Stack *stk);

int GetHash(const void * ptr, size_t size_memory);
void FillHash(Stack *stk);

size_t StackRealloc(Stack *stk);
size_t StackPush(Stack *stk, elem_t value);
size_t StackPop(Stack *stk, elem_t *ret_value);

void   StackDump(Stack *stk, size_t errors, const char *stk_name, int line_err,
                const char *func_err, const char *file_err);

bool IsMacthHashStruct(Stack *stk);
bool IsMacthHashData(Stack *stk);                
