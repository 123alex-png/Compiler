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
        // t->next = ir_head;
        // ir_head->prev = t;
        // ir_head = t;
        ir_tail->next = t;
        t->prev = ir_tail;
        ir_tail = t;
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

Operand new_operand(int kind, int val, int number, char* name){
    Operand ret = (Operand)malloc(sizeof(struct Operand_));
    ret->kind = kind;
    FieldList entry = NULL;
    switch (kind)
    {
    case OP_VARIABLE:
        entry = find(name);
        Assert(entry);
        ret->u.var_no = entry->no;
        break;
    case OP_CONSTANT:
        ret->u.const_value = val;
        break;
    case OP_ADDRESS:
        ret->u.addr_no = number;
        break;
    case OP_ARRAY:
        entry = find(name);
        Assert(entry);
        ret->u.arrray_no = entry->no;
        break;
    case OP_TEMP:
        ret->u.temp_no = number;
        break;
    case OP_FUNCTION:
        entry = find(name);
        Assert(entry);
        ret->u.func_name = name;
        break;
    case OP_LABEL:
        ret->u.label_no = number;
        break;
    case OP_STRUCTURE:
        entry = find(name);
        Assert(entry);
        ret->u.struct_no = entry->no;
        break;
    default:
        break;
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
    if(type == NULL) return 0;
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

void print_inter_code(FILE *fp){
    InterCodes cur = ir_head;
    for(; cur; cur = cur->next){
        Assert(cur->code);
        switch(cur->code->kind){
        case IR_LABEL:
            fprintf(fp, "LABEL ");
            print_op(cur->code->u.unitary.op, fp);
            fprintf(fp, ": ");
            break;
        case IR_FUNCTION:
            fprintf(fp, "FUNCTION ");
            print_op(cur->code->u.unitary.op, fp);
            fprintf(fp, ": ");
            break;
        case IR_ASSIGN:
            print_op(cur->code->u.assign.left, fp);
            fprintf(fp, ":= ");
            print_op(cur->code->u.assign.right, fp);
            break;
        case IR_ADD:
            print_op(cur->code->u.biassign.res, fp);
            fprintf(fp, ":= ");
            print_op(cur->code->u.biassign.op1, fp);
            fprintf(fp, "+ ");
            print_op(cur->code->u.biassign.op2, fp);
            break;
        case IR_SUB:
            print_op(cur->code->u.biassign.res, fp);
            fprintf(fp, ":= ");
            print_op(cur->code->u.biassign.op1, fp);
            fprintf(fp, "- ");
            print_op(cur->code->u.biassign.op2, fp);
            break;
        case IR_MUL:
            print_op(cur->code->u.biassign.res, fp);
            fprintf(fp, ":= ");
            print_op(cur->code->u.biassign.op1, fp);
            fprintf(fp, "* ");
            print_op(cur->code->u.biassign.op2, fp);
            break;
        case IR_DIV:
            print_op(cur->code->u.biassign.res, fp);
            fprintf(fp, ":= ");
            print_op(cur->code->u.biassign.op1, fp);
            fprintf(fp, "/ ");
            print_op(cur->code->u.biassign.op2, fp);
            break;
        case IR_ADDR:
            print_op(cur->code->u.assign.left, fp);
            fprintf(fp, ":= &");
            print_op(cur->code->u.assign.right, fp);
            break;
        case IR_LOAD:
            print_op(cur->code->u.assign.left, fp);
            fprintf(fp, ":= *");
            print_op(cur->code->u.assign.right, fp);
            break;
        case IR_STORE:
            fprintf(fp, "*");
            print_op(cur->code->u.assign.left, fp);
            fprintf(fp, ":= ");
            print_op(cur->code->u.assign.right, fp);
            break;
        case IR_GOTO:
            fprintf(fp, "GOTO ");
            print_op(cur->code->u.unitary.op, fp);
            break;
        case IR_IF_GOTO:
            fprintf(fp, "IF ");
            print_op(cur->code->u.if_goto.op1, fp);
            fprintf(fp, "%s ", cur->code->u.if_goto.relop);
            print_op(cur->code->u.if_goto.op2, fp);
            fprintf(fp, "GOTO ");
            print_op(cur->code->u.if_goto.op3, fp);
            break;
        case IR_RETURN:
            fprintf(fp, "RETURN ");
            print_op(cur->code->u.unitary.op, fp);
            break;
        case IR_DEC:
            fprintf(fp, "DEC ");
            print_op(cur->code->u.dec.op, fp);
            fprintf(fp, "%d ", cur->code->u.dec.size);
            break;
        case IR_ARG:
            fprintf(fp, "ARG ");
            print_op(cur->code->u.unitary.op, fp);
            break;
        case IR_CALL:
            print_op(cur->code->u.assign.left, fp);
            fprintf(fp, ":= CALL ");
            print_op(cur->code->u.assign.right, fp);
            break;
        case IR_PARAM:
            fprintf(fp, "PARAM ");
            print_op(cur->code->u.unitary.op, fp);
            break;
        case IR_READ:
            fprintf(fp, "READ ");
            print_op(cur->code->u.unitary.op, fp);
            break;
        case IR_WRITE:
            fprintf(fp, "WRITE ");
            print_op(cur->code->u.unitary.op, fp);
            break;
        default:
            Assert(0);
        }
        fprintf(fp, "\n");
    }
}

void print_op(Operand op, FILE *fp){
    Assert(op);
    switch(op->kind){
    case OP_VARIABLE:
        fprintf(fp, "v%d ", op->u.var_no);
        break;
    case OP_CONSTANT:
        fprintf(fp, "#%d ", op->u.const_value);
        break;
    case OP_ADDRESS:
        fprintf(fp, "addr%d ", op->u.addr_no);
        break;
    case OP_ARRAY:
        fprintf(fp, "arr%d ", op->u.arrray_no);
        break;
    case OP_TEMP:
        fprintf(fp, "t%d ", op->u.temp_no);
        break;
    case OP_FUNCTION:
        fprintf(fp, "%s ", op->u.func_name);
        break;
    case OP_LABEL:
        fprintf(fp, "label%d ", op->u.label_no);
        break;
    case OP_STRUCTURE:
        fprintf(fp, "struct%d ", op->u.struct_no);
        break;
    default:
        // printf("%d\n", op->kind);
        // Assert(0);
        break;
    }   
    
}