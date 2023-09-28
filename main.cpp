#include "stack.h"
#include "stack.h"



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
