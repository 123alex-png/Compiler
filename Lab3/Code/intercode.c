#include "intercode.h"
#include "symbol_table.h"
#include "common.h"
InterCodes ir_head = NULL, ir_tail = NULL;
int temp_no, addr_no, label_no;
void add_ir(InterCode ir){
    Assert(ir);
    InterCodes t = (InterCodes)malloc(sizeof(struct InterCodes_));
    t->code = ir;
    if(!ir_head){
        Assert(!ir_tail);
        ir_head = ir_tail = t;
    }
    else{
        t->next = ir_head;
        ir_head->prev = t;
        ir_head = t;
    }
}

void delete_ir(InterCodes ir){//没有free
    Assert(ir);
    InterCodes prv = ir->prev, nxt = ir->next;
    if(prv){
        prv->next = nxt;
    }
    if(nxt){
        nxt->prev = prv;
    }
}


InterCode new_ir(int kind, Operand op1, Operand op2, Operand op3, int size, char *relop){
    InterCode ret = (InterCode)malloc(sizeof(struct InterCode_));
    ret->kind = kind;
    switch (kind){
    case IR_LABEL:
    case IR_FUNCTION:
    case IR_GOTO:
    case IR_RETURN:
    case IR_ARG:
    case IR_PARAM:
    case IR_READ:
    case IR_WRITE:
        ret->u.unitary.op = op1;
        break;
    case IR_DEC:
        ret->u.dec.op = op1;
        ret->u.dec.size = size;
        break;
    case IR_ASSIGN:
    case IR_ADDR:
    case IR_LOAD:
    case IR_STORE:
    case IR_CALL:
        ret->u.assign.left = op1;
        ret->u.assign.right = op2;
        break;
    case IR_ADD:
    case IR_SUB:
    case IR_MUL:
    case IR_DIV:
        ret->u.biassign.res = op1;
        ret->u.biassign.op1 = op2;
        ret->u.biassign.op2 = op3;
        break;
    case IR_IF_GOTO:
        ret->u.if_goto.op1 = op1;
        ret->u.if_goto.op2 = op2;
        ret->u.if_goto.op3 = op3;
        strcpy(ret->u.if_goto.relop, relop);
        break;
    default:
        Assert(0);
        break;
    }
    add_ir(ret);
    return ret;
}

int get_size(Type type){
    int ret = 0;
    if(type->kind == BASIC){
        ret = 4;
    }
    else if(type->kind == ARRAY){
        ret = type->u.array.size * get_size(type->u.array.elem);
    }
    else if(type->kind == STRUCTURE){
        for(FieldList cur = type->u.structure; cur; cur = cur->tail){
            ret += get_size(&cur->type);
        }
    }
    else{
        Assert(0);
    }
    return ret;
}

Operand new_temp(){
    return new_operand(OP_TEMP, 0, temp_no++, NULL);
}

Operand new_addr(){
    return new_operand(OP_ADDRESS, 0, addr_no++, NULL);
}

Operand new_label(){
    return new_operand(OP_LABEL, 0, label_no++, NULL);
}

void load_val(Operand op){
    Operand t = new_temp();
    new_ir(IR_LOAD, t, op, NULL, 0, NULL);
    *op = *t;
}

Operand get_addr(Operand addr){
    Operand ret = new_addr();
    new_ir(IR_ADDR, ret, addr, NULL, 0, NULL);
    return ret;
}