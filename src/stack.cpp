#include "stack.h"

canary_t canary_value = 0XB00B5;
const size_t increasing_capacity = 2;

DebugInfo stk_d_info;

ErrorHandler errors_with_msg[] = {
    {ERR_STACK,             "Pointer to Stack structure == NULL\n"},
    {ERR_SIZE_CAPACITY,     "capacity <= size\n"},
    {ERR_DATA,              "Pointer to data == NULL\n"},
    {ERR_CAPACITY,          "Pointer to capacity == NULL\n"},
    {ERR_POP,               "pop try to take, but size == 0\n"},
    {ERR_LEFT_STR_CANARY,   "Left canary (of struct) changes\n"},
    {ERR_RIGHT_STR_CANARY,  "Right canary (of struct) changes\n"},
    {ERR_LEFT_DATA_CANARY,  "Left canary (of data) changes\n"},
    {ERR_RIGHT_DATA_CANARY, "Right canary (of data) changes\n"},
    {ERR_DATA_HASH,         "Hash (of data) mismatch\n"},
    {ERR_STR_HASH,          "Hash (of struct) mismatch\n"},
    {ERR_STACK_DTOR,        "Stack is destoyed\n"}
};

void FillHash(Stack *stk)
{
    stk->hash_struct = 0;
    stk->hash_data = 0;

    stk->hash_struct = GetHash(stk, sizeof(*stk));
    stk->hash_data = GetHash((canary_t *)stk->data - 1, 
                                stk->capacity * sizeof(elem_t) +
                                + 2 *sizeof(canary_t));
}

bool IsMacthHashStruct(Stack *stk)
{
    int prev_hash_struct = stk->hash_struct;
    int prev_hash_data = stk->hash_data;

    stk->hash_struct = 0;
    stk->hash_data = 0;

    bool res = (prev_hash_struct == GetHash(stk, sizeof(*stk)));
    
    stk->hash_struct = prev_hash_struct;
    stk->hash_data = prev_hash_data;

    return res;
}

bool IsMacthHashData(Stack *stk)
{
    return (stk->hash_data == GetHash((canary_t *)stk->data - 1,
            stk->capacity * sizeof(elem_t) + 2 *sizeof(canary_t)));
}

int GetHash(const void * ptr, size_t size_memory)
{
    const char *str = (const char *)ptr;
    
    const int base = 33;
    int hash = 0;
    
    for(int i = 0; i < (int)size_memory; ++i){
        hash += *(str + i);
        hash *= base;
    }
    return hash;
}

int StackVerification(Stack *stk)
{
    int errors = 0;
    
    if(!stk){
        errors |= ERR_STACK;
        return errors;
    }
    if(stk->is_stack_destroyed) {
        errors |= ERR_STACK_DTOR;
        return errors;
    }
    if(stk->size > stk->capacity)
        errors |= ERR_SIZE_CAPACITY;
    if(!stk->data)
        errors |= ERR_DATA;
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
    if(!IsMacthHashData(stk))
        errors |= ERR_DATA_HASH;
    if(!IsMacthHashStruct(stk))
        errors |= ERR_STR_HASH;
    
    return errors;
}

size_t StackCtor(Stack *stk, size_t capacity, int line_init, 
                    const char *func_init, const char *file_init)
{
    if(!stk){
        STACK_DUMP(stk, ERR_STACK);
        return ERR_STACK;
    }
    
    stk_d_info.line_init = line_init;
    stk_d_info.func_init = func_init;
    stk_d_info.file_init = file_init;

    

    stk->left_canary = canary_value;
    stk->right_canary = canary_value;

    stk->data = (elem_t *)calloc(capacity * sizeof(elem_t) + 
                                + 2 * sizeof(canary_t), sizeof(char));
    stk->capacity = capacity;
    stk->size = 0;
    
    *(canary_t *)stk->data = canary_value;
    stk->data = (elem_t *)((canary_t *)stk->data + 1);
    *(canary_t *)(stk->data + stk->capacity) = canary_value;  
    
    FillHash(stk);
    
    int errors = StackVerification(stk);
    if(errors){
        STACK_DUMP(stk, errors);
        return errors;
    }

    return StackVerification(stk);
}

int StackDtor(Stack *stk)
{
    int errors = StackVerification(stk);
    if(errors){
        STACK_DUMP(stk, errors);
        return errors;
    }
    
    free((canary_t *)stk->data - 1);
    stk->left_canary = 0;
    stk->right_canary = 0; 
    stk->size = 0;
    stk->capacity = 0;
    stk->is_stack_destroyed = true;
    
    return errors;
}

size_t StackRealloc(Stack *stk)
{
    int errors = StackVerification(stk);
    if(errors){
        STACK_DUMP(stk, errors);
        return errors;
    }

    canary_t right_old_canary = *(canary_t *)((elem_t *)stk->data +
                                    + stk->capacity);

    stk->data = (elem_t *)((canary_t *)stk->data - 1);
    
    bool is_changed = false;

    if(stk->size == stk->capacity){
        stk->capacity *= increasing_capacity;

        stk->data = (elem_t *)realloc(stk->data, 
            stk->capacity * sizeof(elem_t) + 2 * sizeof(canary_t));

       
        *((canary_t *)((char *)stk->data + 
            + stk->capacity * sizeof(elem_t) + 
            + sizeof(canary_t))) = right_old_canary;
        
        memset((elem_t *)((canary_t *)stk->data + 1) + stk->size, 
                '\0', (stk->capacity - stk->size) * sizeof(elem_t));

        is_changed = true;
    }

    if(stk->capacity > stk->size * increasing_capacity *
            2 && stk->capacity > INIT_CAPACITY){
        
        stk->capacity /= increasing_capacity;

        stk->data = (elem_t *)realloc(stk->data, 
            stk->capacity * sizeof(elem_t) + 2 * sizeof(canary_t));
        
        *((canary_t *)((char *)stk->data + 
            + stk->capacity * sizeof(elem_t) +
            + sizeof(canary_t))) = right_old_canary;
            
        is_changed = true;
    }
    
    stk->data = (elem_t *)((canary_t *)stk->data + 1);

    if(is_changed)
        FillHash(stk);
    
    return StackVerification(stk);
}

size_t StackPush(Stack *stk, elem_t value)
{
    int errors = StackVerification(stk);

    if(errors){
        STACK_DUMP(stk, errors);
        return errors;
    }

    StackRealloc(stk);
    stk->data[stk->size++] = value;

    FillHash(stk);
    return errors;
}

size_t StackPop(Stack *stk, elem_t *ret_value)
{
    int errors = StackVerification(stk);
    
    if(errors){
        STACK_DUMP(stk, errors);
        return errors;
    }

    if (!stk->size)
        return StackVerification(stk) | ERR_POP;
        
    *ret_value = stk->data[--(stk->size)];
    stk->data[stk->size] = POISON;
    
    FillHash(stk);
    StackRealloc(stk);  

    return StackVerification(stk);
}

void StackDump(Stack *stk, size_t errors, const char *stk_name, int line_err,
                const char *func_err, const char *file_err)
{    
    if(errors)
        printf("ERRORS: \n");

    
    for(size_t i = 0; 
        i < sizeof(errors_with_msg) / sizeof(errors_with_msg[0]); i++){

        if (errors & errors_with_msg[i].stk_err)
            printf("%s", errors_with_msg[i].msg);
        
    }

    if(errors & errors_with_msg[0].stk_err)
        return;
        

    printf("STACK[%p] \'%s\' from %s(%d) %s called by %s(%d) %s\n", stk, 
            stk_name, stk_d_info.file_init, stk_d_info.line_init, 
            stk_d_info.func_init, file_err, line_err, func_err);

    printf("Hash (of struct) = %d\n", stk->hash_struct);
    printf("Hash (of data) = %d\n", stk->hash_data);

    printf("Left canary (of struct) = [%0llX]\n", stk->left_canary);
    printf("Right canary (of struct) = [%0llX]\n", stk->right_canary);
    
    printf("Left canary (of data) = [%0llX]\n", *((canary_t *)stk->data - 1));
    printf("Right canary (of data) = [%0llX]\n",
           *(canary_t *)(stk->data + stk->capacity));

    printf("size = %ld\n", stk->size);
    printf("capacity = %ld\n", stk->capacity);
    printf("data[%p]\n", stk->data);

    for(size_t i = 0; i < stk->capacity; i++)
        printf("\t%c[%2ld] = %2d\n", (i < stk->size && stk->data[i] != POISON) ?
                                    '*' : ' ', i, stk->data[i]);
}