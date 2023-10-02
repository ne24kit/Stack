#include "stack.h"

int main() {
    elem_t ret_value = 0;
    Stack stk;

    STACK_CTOR(&stk, INIT_CAPACITY);  

    StackPush(&stk, 1);
    StackPop(&stk, &ret_value);
    
    STACK_DUMP(&stk, 0);
    StackDtor(&stk);
}
