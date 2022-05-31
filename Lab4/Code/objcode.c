#include "objcode.h"
#include "intercode.h"
#include "common.h"
extern InterCodes ir_head, ir_tail;
static Stackframe fp, sp;
static Stackframe head, tail;

FILE *out_file;
struct Reg_ reg[REG_NUM];

char *name[] = {"zero", "at", "v0", "v1", "a0", "a1", "a2", "a3", "t0", "t1", "t2"
, "t3", "t4", "t5", "t6", "t7", "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7", "t8"
, "t9", "k0", "k1", "gp", "sp", "fp", "ra"};
void init_regs(){
    for(int i = 0; i < REG_NUM; i++){
        reg[i].name = name[i];
        reg[i].is_busy = 0;
        reg[i].val = 0;
    }
}
void init_environment() {
    fprintf(out_file, ".data\n");
    fprintf(out_file, "_prompt: .asciiz \"Enter an integer:\"\n");
    fprintf(out_file, "_ret: .asciiz \"\\n\"\n");
    fprintf(out_file, ".globl main\n");

    fprintf(out_file, ".text\n");
    fprintf(out_file, "read:\n");
    fprintf(out_file, "  li $v0, 4\n");
    fprintf(out_file, "  la $a0, _prompt\n");
    fprintf(out_file, "  syscall\n");
    fprintf(out_file, "  li $v0, 5\n");
    fprintf(out_file, "  syscall\n");
    fprintf(out_file, "  jr $ra\n");
    fprintf(out_file, "\n");

    fprintf(out_file, "write:\n");
    fprintf(out_file, "  li $v0, 1\n");
    fprintf(out_file, "  syscall\n");
    fprintf(out_file, "  li $v0, 4\n");
    fprintf(out_file, "  la $a0, _ret\n");
    fprintf(out_file, "  syscall\n");
    fprintf(out_file, "  move $v0, $0\n");
    fprintf(out_file, "  jr $ra\n");
    fprintf(out_file, "\n");
}

int get_reg(InterCode ir, Operand op){
    int ret;
    switch (ir->kind)
    {
    case IR_ARG:
        ret = 16;
        break;
    case IR_CALL:
        ret = 2;
        break;
    case IR_ASSIGN:
    case IR_ADDR:
    case IR_LOAD:
    case IR_STORE:
        if(ir->u.assign.left == op){
            ret = 8;
        }
        else{
            ret = 9;
        }
        break;
    case IR_IF_GOTO:
        if(ir->u.if_goto.op1 == op){
            ret = 8;
        }
        else{
            Assert(ir->u.if_goto.op2 == op);
            ret = 9;
        }
        break;
    case IR_ADD:
    case IR_SUB:
    case IR_MUL:
    case IR_DIV:
        if(ir->u.biassign.res == op){
            ret = 8;
        }
        else if(ir->u.biassign.op1 == op){
            ret = 9;
        }
        else{
            ret = 10;
        }
    case IR_RETURN:
        ret = 8;
        break;
    default:
        Assert(0);
        break;
    }
    return ret;
}

int find_op(Operand op){
    Stackframe cur = fp;
    int ret = 0;
    for(; cur; cur = cur->next){
        if(cur->no == op->u.addr_no && cur->kind == op->kind){
            ret = 1;
        }
    }
    return ret;   
}

int op2offset(Operand op){
    Stackframe cur = fp;
    int ret = -1;
    for(; cur; cur = cur->next){
        if(cur->no == op->u.addr_no && cur->kind == op->kind){
            ret = cur->offset;
        }
    }
    Assert(ret != -1);
    return ret;   
}

void load_reg(int reg_no, Operand op){
    int offset;
    switch (op->kind)
    {
    case OP_VARIABLE:
    case OP_TEMP:
        offset = op2offset(op);
        fprintf(out_file, "   lw %s, %d($out_file)\n", reg[reg_no].name, -offset);
        break;
    case OP_ADDRESS:
    case OP_ARRAY:
        offset = op2offset(op);
        fprintf(out_file, "   la %s, %d($out_file)\n", reg[reg_no].name, -offset);
        break;
    case OP_CONSTANT:
        fprintf(out_file, "   li %s, %d\n", reg[reg_no].name, op->u.const_value);
        break;
    case OP_FUNCTION:
        fprintf(out_file, "   la %s, %s\n", reg[reg_no].name, -offset);
        break;
    case OP_LABEL:
        fprintf(out_file, "   la %s, label%d\n", reg[reg_no].name, op->u.label_no);
        break;
    default:
        Assert(0);
        break;
    }
    
}

void store_reg_mem(int reg_no, Operand op){
    Assert(op->kind == OP_TEMP || op->kind == OP_VARIABLE);
    int offset = op2offset(op);
    fprintf(out_file, "   sw %s, %d($out_file)\n", reg[reg_no].name, -offset);
}

void push_op(Operand op, int offset){
    Stackframe new_stack_frame = (Stackframe)malloc(sizeof(struct Stackframe_));
    new_stack_frame->kind = op->kind;
    new_stack_frame->no = op->u.addr_no;//union随便选一个，反正不可能是函数
    new_stack_frame->offset = offset;
    new_stack_frame->next = NULL;
    sp->next = new_stack_frame;
    sp = new_stack_frame;
}

void pop_cur_stack(){
    Stackframe cur = head;
    Assert(cur);
    for(; cur; cur = cur->next){
        if(cur == fp){
            break;
        }
    }
    Assert(cur == fp);
    fp = head;
    sp = cur;
}

void push_var_op(Operand op, int offset, int dsize){
    // Assert(op->kind == OP_VARIABLE || op->kind == OP_TEMP);
    if(find_op(op) == 0){
        offset += dsize;
        push_op(op, offset);
    }
}

void create_new_stack(InterCodes start){
    int offset = 8;
    InterCodes cur = start;
    for(; cur; cur = cur->next){
        InterCode cur_code = cur->code;
        switch (cur->code->kind)
        {
        case IR_DEC:
            push_var_op(cur_code->u.dec.op, &offset, cur_code->u.dec.size);
            // Operand op = cur_code->u.dec.op;
            // if(find_op(op) == 0){
            //     offset += cur_code->u.dec.size;
            //     push_op(op, offset);
            // }
            break;
        case IR_ARG:
            push_var_op(cur_code->u.unitary.op, &offset, 4);
            break;
        case IR_ADD:
        case IR_SUB:
        case IR_MUL:
        case IR_DIV:
            push_var_op(cur_code->u.biassign.op1, &offset, 4);
            push_var_op(cur_code->u.biassign.op2, &offset, 4);
            push_var_op(cur_code->u.biassign.res, &offset, 4);
            break;
        case IR_READ:
            push_var_op(cur_code->u.unitary.op, &offset, 4);
            break;
        case IR_ASSIGN:
            push_var_op(cur_code->u.assign.left, &offset, 4);
            push_var_op(cur_code->u.assign.right, &offset, 4);
            break;
        case IR_CALL:
            push_var_op(cur_code->u.assign.left, &offset, 4);
            break;
        default:
            break;
        }
    }
    fprintf(out_file, "addi $sp, $sp, -%d\n", offset);
    fprintf(out_file, "sw $ra, %d($sp)\n", offset-4);
    fprintf(out_file, "sw $out_file, %d($sp)\n", offset-8);
    fprintf(out_file, "addi $out_file, $sp, %d", offset);
}

void new_objcode(InterCode t, InterCodes code){
    switch(t->kind){
        case IR_LABEL:
            new_code_ir_label(t);
            break;
        case IR_FUNCTION:
            new_code_ir_function(t, code);
            break;
        case IR_ASSIGN:
        case IR_ADDR:
        case IR_LOAD:
        case IR_STORE:
            new_code_ir_assign(t);
            break;
        case IR_ADD:
        case IR_SUB:
        case IR_MUL:
        case IR_DIV:
            new_code_ir_arith(t);
            break;
        // case IR_ADDR:
        //     new_code_ir_addr(t);
        //     break;
        // case IR_LOAD:
        //     new_code_ir_load(t);
        //     break;
        // case IR_STORE:
        //     new_code_ir_store(t);
        //     break;
        case IR_GOTO:
            new_code_ir_goto(t);
            break;
        case IR_IF_GOTO:
            new_code_ir_if_goto(t);
            break;
        case IR_RETURN:
            new_code_ir_return(t);
            break;
        case IR_DEC:
            new_code_ir_dec(t);
            break;
        case IR_ARG:
            new_code_ir_arg(t);
            break;
        case IR_CALL:
            new_code_ir_call(t);
            break;
        case IR_PARAM:
            new_code_ir_param(t);
            break;
        case IR_READ:
            new_code_ir_read(t);
            break;
        case IR_WRITE:
            new_code_ir_write(t);
            break;
        default:
            Assert(0);
        }
}

void gen_objcode(FILE *file){
    init_regs();
    init_environment();
    out_file = file;
    InterCodes cur = ir_head;
    for(; cur; cur = cur->next){
        Assert(cur->code);
        new_objcode(cur->code, cur);
    }
}

void new_code_ir_function(InterCode ir, InterCodes code){
    char *func_name = ir->u.unitary.op->u.func_name;
    pop_op();
    fprintf("%s:\n", func_name);
    if(0 == strcmp("main", func_name)){
        create_new_stack(code);
    }
}

void new_code_ir_label(InterCode ir){
    fprintf(out_file, "label%d:\n", ir->u.unitary.op->u.label_no);
}

void new_code_ir_assign(InterCode ir){
    Operand left = ir->u.assign.left, right = ir->u.assign.right;
    int left_reg = get_reg(ir, left), right_reg = get_reg(ir, right);
    load_reg(left_reg, left);
    load_reg(right_reg, right);
    fprintf(out_file, "   move %s, %s\n", reg[left_reg].name, reg[right_reg].name);
    store_reg_mem(left_reg, left);
    store_reg_mem(right_reg, right);
}

void new_code_ir_arith(InterCode ir){
    Operand res = ir->u.biassign.res, op1 = ir->u.biassign.op1, op2 = ir->u.biassign.op2;
    int res_reg = get_reg(ir, res), op1_reg = get_reg(ir, op1), op2_reg = get_reg(ir, op2);
    load_reg(op1_reg, op1);
    load_reg(op2_reg, op2);
    switch (ir->kind)
    {
    case IR_ADD:
        fprintf(out_file, "   add %s, %s, %s", reg[res_reg].name, reg[op1_reg].name, reg[op2_reg].name);   
        break;
    case IR_SUB:
        fprintf(out_file, "   sub %s, %s, %s", reg[res_reg].name, reg[op1_reg].name, reg[op2_reg].name);   
        break;
    case IR_MUL:
        fprintf(out_file, "   mul %s, %s, %s", reg[res_reg].name, reg[op1_reg].name, reg[op2_reg].name);   
        break;
    case IR_DIV:
        fprintf(out_file, "   div %s, %s, %s", reg[res_reg].name, reg[op1_reg].name, reg[op2_reg].name);   
        fprintf(out_file, "   mflo %s\n", reg[res_reg].name);
        break;
    default:
        break;
    }
    store_reg_mem(res_reg, res);
}

void new_code_ir_if_goto(InterCode ir){
    Operand op1 = ir->u.if_goto.op1, op2 = ir->u.if_goto.op2, op3 = ir->u.if_goto.op3;
    char *relop = ir->u.if_goto.relop;
    int op1_reg = get_reg(ir, op1), op2_reg = get_reg(ir, op2);
    load_reg(op1_reg, op1);
    load_reg(op2_reg, op2);
    char *relop_order = relop2order(relop);
    fprintf(out_file, "   %s %s, %s, label %d\n", relop_order, reg[op1_reg].name, reg[op2_reg].name, op3->u.label_no);
}

void new_code_ir_goto(InterCode ir){
    fprintf(out_file, "   j label%d\n", ir->u.unitary.op->u.label_no);
}

void new_code_ir_return(InterCode ir){
    Operand ret_op = ir->u.unitary.op;
	int value = ret_op->u.const_value;
	int reg_no = get_reg(ir, NULL);;
	load_reg(reg_no, ret_op);
    fprintf(out_file,"  move $v0, %s\n",reg[reg_no].name);

	fprintf(out_file, "  move $sp, $out_file\n");
	fprintf(out_file, "  lw $ra, -4($out_file)\n");
	fprintf(out_file, "  lw $out_file, -8($out_file)\n");
	fprintf(out_file, "  jr $ra\n");
}

void new_code_ir_arg(InterCode ir){
    fprintf(out_file, "addi $sp, $sp, -4\n");
    int reg_no = get_reg(ir, NULL);
    load_reg(reg_no, ir->u.unitary.op);
    fprintf(out_file, "   sw %s, 0($sp)\n", reg[reg_no].name);
}

void new_code_ir_call(InterCode ir){
    fprintf(out_file, "   jal %s\n", ir->u.assign.right->u.func_name);
    int reg_no = get_reg(ir, NULL);
    store_reg_mem(reg_no, ir->u.assign.left);
}

void new_code_ir_param(InterCode ir){

}

void new_code_ir_dec(InterCode ir){

}

void new_code_ir_arg(InterCode ir){

}

void new_code_ir_call(InterCode ir){

}

void new_code_ir_read(InterCode ir){

}

void new_code_ir_write(InterCode ir){

}


char *relop2order(char *relop){
    char ret[4];
    if(0 == strcmp(relop, "==")){
        sprintf(ret, "beq");
    }
    else if(0 == strcmp(relop, "!=")){
        sprintf(ret, "bne");
    }
    else if(0 == strcmp(relop, ">")){
        sprintf(ret, "bgt");
    }
    else if(0 == strcmp(relop, "<")){
        sprintf(ret, "blt");
    }
    else if(0 == strcmp(relop, ">=")){
        sprintf(ret, "bge");
    }
    else if(0 == strcmp(relop, "<=")){
        sprintf(ret, "ble");
    }
    return ret;
}