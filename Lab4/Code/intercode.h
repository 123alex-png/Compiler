#ifndef __INTERCODE_H__
#define __INTERCODE_H__
#include "symbol_table.h"
#include "SyntaxTree.h"
#include <stdio.h>

typedef struct Operand_* Operand; 
typedef struct InterCode_* InterCode;
typedef struct InterCodes_* InterCodes;
struct Operand_ { 
    enum { OP_VARIABLE, OP_CONSTANT, OP_ADDRESS, OP_ARRAY, OP_TEMP, OP_FUNCTION, OP_LABEL, OP_STRUCTURE} kind; 
    union { 
        int var_no; 
        int const_value;  
        int addr_no;
        int arrray_no;
        int temp_no;
        int label_no;
        char *func_name;
        int struct_no;
    } u; 
    Type type;
    int size;
    FieldList structure;
}; 

struct InterCode_ { 
    enum{ 
        IR_LABEL,           //LABEL x
        IR_FUNCTION,        //FUNCTION f
        IR_ASSIGN,          //x := y
        IR_ADD,             //x := y + z
        IR_SUB,             //x := y - z
        IR_MUL,             //x := y * z
        IR_DIV,             //x := y / z
        IR_ADDR,            //x := &y
        IR_LOAD,            //x := *y
        IR_STORE,           // *x = y
        IR_GOTO,            //GOTO x
        IR_IF_GOTO,         //IF x [relop] y GOTO z
        IR_RETURN,          //RETURN x
        IR_DEC,             //DEC x [size]
        IR_ARG,             //ARG x
        IR_CALL,            //x := CALL f
        IR_PARAM,           //PARAM x
        IR_READ,            //READ x
        IR_WRITE            //WRITE x
    } kind; 
    union { 
        struct{
            Operand op;
        } unitary; //LABEL, FUNCTION, GOTO, RETURN, ARG, PARAM, READ, WRITE
        struct{
            Operand left, right;
        } assign; //ASSIGN, ADDR, LOAD, STORE, CALL
        struct{
            Operand res, op1, op2;
        } biassign;  
        struct{
            Operand op;
            int size;
        } dec;
        struct{
            Operand op1, op2, op3;
            char relop[32];
        } if_goto;
    } u; 
};

struct InterCodes_ {
    InterCode code;
    InterCodes prev, next;
};

void add_ir(InterCode ir);
void delete_ir(InterCodes ir);

InterCode new_ir(int kind, Operand op1, Operand op2, Operand op3, int size, char* relop);  
Operand new_operand(int kind, int val, int number, char* name);
Operand new_temp();
Operand new_addr();
Operand new_label();

void load_val(Operand op);
int get_size(Type type);
Operand get_addr(Operand addr);

void print_inter_code(FILE *fp);
void print_op(Operand op, FILE *fp);
#endif