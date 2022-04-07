#include "stack.h"
#include <assert.h>

ScopeList scope_stack[STACK_SIZE+1];
static int top_index;
ScopeList scope_stack_top(){
    return scope_stack[top_index-1];
}

int scope_stack_empty(){
    return top_index <= 0;
}

void scope_stack_pop(){
    assert(top_index > 0);
    scope_stack[top_index-1] = NULL;
    top_index--;
}

void scope_stack_push(ScopeList val){
    scope_stack[top_index] = val;
    assert(++top_index < STACK_SIZE);
}
