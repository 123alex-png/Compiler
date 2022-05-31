#ifndef __STACK_H__
#define __STACK_H__

#include "symbol_table.h"

#define STACK_SIZE 0x3fff

ScopeList scope_stack_top();
int scope_stack_empty();
void scope_stack_pop();
void scope_stack_push(ScopeList val);

#endif