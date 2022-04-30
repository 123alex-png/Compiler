#include "translator.h"

extern InterCodes ir_head, ir_tail;
// extern int var_cnt, array_cnt, func_cnt, struct_cnt;
extern int temp_no, addr_no, label_no;
//Program: ExtDefList
void tr_Program(Node *cur){
    if(cur == NULL) return;
    child_node *children = get_childs(cur);
    Assert(cur->right_num == 1);
    tr_ExtDefList(children->childs[0]);
}

// ExtDefList: ExtDef ExtDefList | /*Empty*/ 
void tr_ExtDefList(Node *cur){
    if(cur == NULL) return;
    child_node *children = get_childs(cur);
    Assert(cur->right_num == 2);
    tr_ExtDef(children->childs[0]);
    tr_ExtDefList(children->childs[1]);
}

// ExtDef: Specifier ExtDecList SEMI    (1)
//         | Specifier SEMI             (2)
//         | Specifier FunDec CompSt    (3)
//         | Specifier FunDec SEMI      (4)
void tr_ExtDef(Node *cur){
    if(cur == NULL) return;
    child_node *children = get_childs(cur);
    // int num = children->num;
    Assert(0 == strcmp(children->childs[0]->name, "Specifier"));
    if(cur->right_num == 2){//(2)
        Assert(0 == strcmp(children->childs[1]->name, "SEMI"));
    }
    else{
        Assert(cur->right_num == 3);
        if(0 == strcmp(children->childs[1]->name, "ExtDecList")){//(1)
            Assert(0 == strcmp(children->childs[2]->name, "SEMI"));
            // tr_ExtDecList(children->childs[1]);
        }
        else{
            if(0 == strcmp(children->childs[2]->name, "CompSt")){//(3)define
                Assert(0 == strcmp(children->childs[1]->name, "FunDec"));
                tr_FunDec(children->childs[1]);
                tr_CompSt(children->childs[2]);
            }
            else if(0 == strcmp(children->childs[2]->name, "SEMI")){//(4)declare
                Assert(0 == strcmp(children->childs[1]->name, "FunDec"));
                tr_FunDec(children->childs[1]);
            }
            else{
                Assert(0);
            }
            
        }
    }
}


// VarDec: ID  (1)
//         | VarDec LB BASIC_INT RB  (2)
Operand tr_VarDec(Node *cur){
    if(cur == NULL) return NULL;
    Operand ret;
    child_node *children = get_childs(cur);
    int num = children->num;
    Assert(num == cur->right_num);
    if(num == 1){//(1)
        char *name = children->childs[0]->type_content;
        FieldList entry = find(name);
        Assert(entry);
        if(entry->type.kind == ARRAY){
            ret = new_operand(OP_ARRAY, 0, 0, name);
            ret->type = entry->type.u.array.elem;
            ret->size = entry->type.u.array.size;
            int size =  get_size(&entry->type);
            new_ir(IR_DEC, ret, NULL, NULL, size, NULL);
        }
        else if(entry->type.kind == BASIC){
            ret = new_operand(OP_VARIABLE, 0, 0, name);
        }
        else if(entry->type.kind == STRUCTURE){
            //TBD
        }
        else{
            Assert(0);
        }
        return ret;
    }
    //(2)
    return tr_VarDec(children->childs[0]);
}

// FunDec: ID LP VarList RP   (1)
//         | ID LP RP         (2)
void tr_FunDec(Node *cur){
    if(cur == NULL) return;
    child_node *children = get_childs(cur);
    int num = children->num;
    Assert(num == cur->right_num);
    char *name = children->childs[0]->type_content;
    FieldList entry = find(name);
    Operand func_op = new_operand(OP_FUNCTION, 0, 0, name);
    if(num == 4){//(1)
        FieldList arg = entry->type.u.function.argv;
        for(; arg; arg = arg->tail){
            Operand op;
            if(arg->type.kind == BASIC){
                op = new_operand(OP_VARIABLE, 0, 0, arg->name);
            }
            else if(arg->type.kind == ARRAY){
                op = new_operand(OP_ARRAY, 0, 0, arg->name);
            }
            else{

            }
            new_ir(IR_PARAM, op, NULL, NULL, 0, NULL);
        }
    }
}


// CompSt: LC DefList StmtList RC 
void tr_CompSt(Node *cur){
    if(cur == NULL) return;
    child_node *children = get_childs(cur);
    int num = children->num;
    // Assert(num == cur->right_num);//存疑
    Node *n1 = find_child(cur, "DefList"), *n2 = find_child(cur, "StmtList");
    tr_DefList(n1);
    tr_StmtList(n2);
}

// StmtList: Stmt StmtList 
void StmtList(Node *cur, Type type){
    if(cur == NULL) return;
    child_node *children = get_childs(cur);
    int num = children->num;
    // Assert(num == cur->right_num);//存疑
    Node *n1 = find_child(cur, "Stmt"), *n2 = find_child(cur, "StmtList");
    tr_Stmt(n1);
    tr_StmtList(n2);
}

// Stmt: Exp SEMI   1
//     | CompSt      2          
//     | RETURN Exp SEMI   3
//     | IF LP Exp RP Stmt  4
//     | IF LP Exp RP Stmt ELSE Stmt  5
//     | WHILE LP Exp RP Stmt   6
void tr_Stmt(Node *cur){
    if(cur == NULL) return;
    child_node *children = get_childs(cur);
    int num = children->num;
    Assert(num == cur->right_num);
    if(num == 2){//1
        tr_Exp(children->childs[0], new_temp());
    }
    else if(num == 1){//2
        tr_CompSt(children->childs[0]);
    }
    else if(num == 3){//3
        Operand t = new_temp();
        tr_Exp(children->childs[1], t);
        if(t->kind == OP_ADDRESS){
            load_val(t);
        }
        new_ir(IR_RETURN, t, NULL, NULL, 0, NULL);
    }
    else if(num == 7){//5
        Operand l1 = new_label();
        Operand l2 = new_label();
        Operand l3 = new_label();
        tr_Cond(children->childs[2], l1, l2);
        new_ir(IR_LABEL, l1, NULL, NULL, 0, NULL);
        tr_Stmt(children->childs[4]);
        new_ir(IR_GOTO, l3, NULL, NULL, 0, NULL);
        new_ir(IR_LABEL, l2, NULL, NULL, 0, NULL);
        tr_Stmt(children->childs[6]);
        new_ir(IR_LABEL, l3, NULL, NULL, 0, NULL);
    }
    else if(0 == strcmp(children->childs[0]->name, "IF")){  //4
        Operand l1 = new_label();
        Operand l2 = new_label();
        tr_Cond(children->childs[2], l1, l2);
        new_ir(IR_LABEL, l1, NULL, NULL, 0, NULL);
        tr_Stmt(children->childs[4]);
        new_ir(IR_LABEL, l2, NULL, NULL, 0, NULL);
    }

    else{ //6
        Operand l1 = new_label();
        Operand l2 = new_label();
        Operand l3 = new_label();
        new_ir(IR_LABEL, l1, NULL, NULL, 0, NULL);
        tr_Cond(children->childs[2], l2, l3);
        new_ir(IR_LABEL, l2, NULL, NULL, 0, NULL);
        tr_Stmt(children->childs[4]);
        new_ir(IR_GOTO, l1, NULL, NULL, 0, NULL);
        new_ir(IR_LABEL, l3, NULL, NULL, 0, NULL);
    }
}

// DefList: Def DefList  
void tr_DefList(Node *cur){
    if(cur == NULL) return;
    child_node *children = get_childs(cur);
    int num = children->num;
    // Assert(num == cur->right_num);
    // Node *n1 = find_child(cur, "Def"), *n2 = find_child(cur, "DefList");
    tr_Def(children->childs[0]);
    tr_DefList(children->childs[1]);
}

// Def: Specifier DecList SEMI
void tr_Def(Node *cur){
    if(cur == NULL) return;
    child_node *children = get_childs(cur);
    int num = children->num;
    Assert(num == cur->right_num);
    tr_DecList(children->childs[1]);
}

// DecList: Dec   
//         | Dec COMMA DecList
void tr_DecList(Node *cur){
    if(cur == NULL) return;
    child_node *children = get_childs(cur);
    int num = children->num;
    Assert(num == cur->right_num);
    tr_Dec(children->childs[0]);
    if(num == 3){ 
        // Assert(num == 3);
        tr_DecList(children->childs[2]);
    }
    else{
        Assert(num == 1);
    }
}

// Dec: VarDec   
//     | VarDec ASSIGNOP Exp 
void tr_Dec(Node *cur){
    if(cur == NULL) return;
    child_node *children = get_childs(cur);
    int num = children->num;
    Assert(num == cur->right_num);
    Operand op = tr_VarDec(children->childs[0]);
    if(num == 3){
        Operand t = new_temp();
        tr_Exp(children->childs[2], t);
        if(op->kind == OP_ARRAY){
            array_copy(op, t);
        }
        else if(op->kind == OP_VARIABLE){
            if(t->kind == OP_ADDRESS){
                load_val(t);
            }
            new_ir(IR_ASSIGN, op, t, NULL, 0, NULL);
        }
    }
}

// Exp: Exp ASSIGNOP Exp     1        
//     | Exp AND Exp         2             
//     | Exp OR Exp          3          
//     | Exp RELOP Exp       4        
//     | Exp PLUS Exp        5                 
//     | Exp MINUS Exp       6              
//     | Exp STAR Exp        7             
//     | Exp DIV Exp         8             
//     | LP Exp RP           9               
//     | MINUS Exp %prec UMINUS  10   
//     | NOT Exp                 11        
//     | ID LP Args RP           12          
//     | ID LP RP                13         
//     | Exp LB Exp RB           14 
//     | Exp DOT ID              15      
//     | ID                      16
//     | BASIC_INT                     17     
//     | BASIC_FLOAT                   18   
void tr_Exp(Node *cur, Operand place){
    if(cur == NULL) return NULL;
    Type ret = NULL;
    child_node *children = get_childs(cur);
    int num = children->num;
    Assert(num == cur->right_num);
    
    if(num == 1 && 0 == strcmp(children->childs[0]->name, "INT")){//17
        place->kind = OP_CONSTANT;
        place->u.const_value = children->childs[0]->type_int;
    }
    else if(num == 1 && 0 == strcmp(children->childs[0]->name, "ID")){//16
        char *name = children->childs[0]->type_content;
        Assert(name);
        FieldList entry = find(name);
        if(entry->type.kind == BASIC){
            place->kind = OP_VARIABLE;
            place->u.var_no = entry->no;
        }
        else if(entry->type.kind == ARRAY){
            Operand array_op = new_operand(OP_ARRAY, 0, 0, entry->name);
            if(entry->is_arg == 1){
                place->kind = OP_ADDRESS;
                place->u.addr_no = addr_no++;
                new_ir(IR_ASSIGN, place, array_op, NULL, 0, NULL);
            }
            else{
                place->kind = OP_ARRAY;
                place->u.arrray_no = entry->no;
            }
            place->type = entry->type.u.array.elem;
            place->size = entry->type.u.array.size;
        }
        else if(entry->type.kind == STRUCTURE){//TBD

        }
        else if((num == 2 && 0 == strcmp(children->childs[0]->name, "NOT"))
        || (num == 3 && 0 == strcmp(children->childs[1]->name, "RELOP"))
        || (num == 3 && 0 == strcmp(children->childs[1]->name, "AND"))
        || (num == 3 && 0 == strcmp(children->childs[1]->name, "OR"))){//2,3,4,11
            Operand l1 = new_label(), l2 = new_label();
            Operand op_false = new_operand(OP_CONSTANT, 0, 0, NULL);
            new_ir(IR_ASSIGN, place, op_false, NULL, 0, NULL);
            tr_Cond(cur, l1, l2);
            new_ir(IR_LABEL, l1, NULL, NULL, 0, NULL);
            Operand op_true = new_operand(OP_CONSTANT, 0, 0, NULL);
            new_ir(IR_ASSIGN, place, op_true, NULL, 0, NULL);
            new_ir(IR_LABEL, l2, NULL, NULL, 0, NULL);
        }
        else if(num == 2 && 0 == strcmp(children->childs[0]->name, "MINUS")){//10
            Operand t = new_temp();
            tr_Exp(children->childs[1], t);
            if(t->kind == OP_ADDRESS){
                load_val(t);
            }
            if(t->kind == OP_CONSTANT){
                place->kind = OP_CONSTANT;
                place->u.const_value = -1 * t->u.const_value;
            }
            else{
                Operand op_zero = new_operand(OP_CONSTANT, 0, 0, NULL);
                new_ir(IR_SUB, place, op_zero, t, 0, NULL);
            }
        }
        else 
    }
    return ret;
}

// Args: Exp COMMA Args  
//     | Exp
FieldList Args(Node *cur){
    if(cur == NULL) return NULL;
    child_node *children = get_childs(cur);
    int num = children->num;
    Assert(num == cur->right_num);
    Assert(0 == strcmp(children->childs[0]->name, "Exp"));
    Type type_arg = Exp(children->childs[0]);
    if(!type_arg) return NULL;
    FieldList ret = (FieldList)malloc(sizeof(struct FieldList_));
    ret->type = *type_arg;
    if(num == 3){//1
        ret->tail = Args(children->childs[2]);
    }
    else{
        Assert(num == 1);
        ret->tail = NULL;
    }
    return ret;
}

void array_copy(Operand dst, Operand src){
    Operand dst_addr = get_addr(dst), src_addr = get_addr(src);
    int dst_size = get_size(&dst->type) * dst->size, src_size = get_size(&src->type) * src->size;
    int size = min(dst_size, src_size);
    Operand left = new_addr(), right = new_addr(), tmp = new_temp();
    // Operand tmp = new_temp();
    new_ir(IR_LOAD, tmp, src_addr, NULL, 0, NULL);
    new_ir(IR_STORE, dst_addr, tmp, NULL, 0, NULL);
    for(int i = 0; i < size; i += 4){
        Operand offset = new_operand(OP_CONSTANT, i, 0, NULL);
        new_ir(IR_ADD, left, dst_addr, offset, 0, NULL);
        new_ir(IR_ADD, right, src_addr, offset, 0, NULL);
        new_ir(IR_LOAD, tmp, right, NULL, 0, NULL);
        new_ir(IR_STORE, left, tmp, NULL, 0, NULL);
    }
}