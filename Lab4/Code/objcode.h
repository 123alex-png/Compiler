#ifndef __OBJCODE_H__
#define __OBJCODE_H__
#include "intercode.h"
#include <stdint.h>
#define REG_NUM 32
typedef struct Stackframe_* Stackframe;
// typedef struct Stack_* Stack;
struct Reg_ {
    char *name;
    int is_busy;
    uint32_t val;
};

struct Stackframe_ {
    int offset;
    int kind;
    int no;
    Stackframe next;
};

// struct Stack_ {
//     Stackframe stackframe;
//     Stack next;
// };

void gen_objcode();
void new_objcode(InterCode t, InterCodes code);
void new_code_ir_label(InterCode ir);
void new_code_ir_function(InterCode ir, InterCodes code);
void new_code_ir_assign(InterCode ir);
void new_code_ir_arith(InterCode ir);
void new_code_ir_addr(InterCode ir);
void new_code_ir_load(InterCode ir);
void new_code_ir_store(InterCode ir);
void new_code_ir_goto(InterCode ir);
void new_code_ir_if_goto(InterCode ir);
void new_code_ir_return(InterCode ir);
void new_code_ir_dec(InterCode ir);
void new_code_ir_arg(InterCode ir);
void new_code_ir_call(InterCode ir);
void new_code_ir_param(InterCode ir);
void new_code_ir_read(InterCode ir);
void new_code_ir_write(InterCode ir);


char *relop2order(char *relop);
void create_new_stack(InterCodes start);
#endif