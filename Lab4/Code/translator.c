#include "translator.h"
#include "common.h"
extern InterCodes ir_head, ir_tail;
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
    Assert(0 == strcmp(children->childs[0]->name, "Specifier"));
    if(cur->right_num == 2){//(2)
        Assert(0 == strcmp(children->childs[1]->name, "SEMI"));
    }
    else{
        Assert(cur->right_num == 3);
        if(0 == strcmp(children->childs[1]->name, "ExtDecList")){//(1)
            Assert(0 == strcmp(children->childs[2]->name, "SEMI"));
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
            ret = new_operand(OP_STRUCTURE, 0, 0, name);
            ret->structure = entry->type.u.structure;
            int size =  entry->type.u.structure->type.size;
            new_ir(IR_DEC, ret, NULL, NULL, size, NULL);
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
    new_ir(IR_FUNCTION, func_op, NULL, NULL, 0, NULL);
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
                // printf("%d\n", arg->type.kind);
                Assert(arg->type.kind == STRUCTURE);
                op = new_operand(OP_STRUCTURE, 0, 0, arg->name);
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
void tr_StmtList(Node *cur){
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
    if(cur == NULL) return;
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
            if(entry->is_arg == 1){
                Operand array_op = new_operand(OP_ARRAY, 0, 0, entry->name);
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
        else if(entry->type.kind == STRUCTURE){
            if(entry->is_arg == 1){
                Operand struct_op = new_operand(OP_STRUCTURE, 0, 0, entry->name);
                place->kind = OP_ADDRESS;
                place->u.addr_no = addr_no++;
                new_ir(IR_ASSIGN, place, struct_op, NULL, 0, NULL);
            }
            else{
                place->kind = OP_STRUCTURE;
                place->u.struct_no = entry->no;
            }
            place->structure = entry->type.u.structure;
        }
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
        Operand op_true = new_operand(OP_CONSTANT, 1, 0, NULL);
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
    else if(num == 3 && 0 == strcmp(children->childs[1]->name, "ASSIGNOP")){//1
        Operand dst = new_temp();
        tr_Exp(children->childs[0], dst);
        Operand src = new_temp();
        tr_Exp(children->childs[2], src);
        if(dst->kind ==  OP_ADDRESS || dst->kind == OP_ARRAY){//有疑问
            if(src->kind == OP_ADDRESS || src->kind == OP_ARRAY){
                array_copy(dst, src);
            }
            else{
                Assert(dst->kind == OP_ADDRESS);
                new_ir(IR_STORE, dst, src, NULL, 0, NULL);
            }
        }
        else{
            if(src->kind == OP_ADDRESS){
                load_val(src);
            }
            new_ir(IR_ASSIGN, dst, src, NULL, 0, NULL);
        }
        place->kind = src->kind;
        place->u = src->u;
    }
    else if(num == 3 &&
        ( 0 == strcmp(children->childs[1]->name, "PLUS")
        ||0 == strcmp(children->childs[1]->name, "MINUS")
        ||0 == strcmp(children->childs[1]->name, "STAR")
        ||0 == strcmp(children->childs[1]->name, "DIV")
        )){//5,6,7,8
        Operand t1 = new_temp();
        tr_Exp(children->childs[0], t1);
        if(t1->kind == OP_ADDRESS){
            load_val(t1);
        }
        Operand t2 = new_temp();
        tr_Exp(children->childs[2], t2);
        if(t2->kind == OP_ADDRESS){
           load_val(t2);
        }
            int kind;
            // int val;
            if(t1->kind == OP_CONSTANT && t2->kind == OP_CONSTANT){
                place->kind = OP_CONSTANT;
                int val;
                if(0 == strcmp(children->childs[1]->name, "PLUS")){
                    // kind = IR_ADD;
                    val = t1->u.const_value + t2->u.const_value;
                }
                else if(0 == strcmp(children->childs[1]->name, "MINUS")){
                    // kind = IR_SUB;
                    val = t1->u.const_value - t2->u.const_value;
                }
                else if(0 == strcmp(children->childs[1]->name, "STAR")){
                    // kind = IR_MUL;
                    val = t1->u.const_value * t2->u.const_value;
                }
                else{
                    // kind = IR_DIV;
                    // val = t1->u.const_value / t2->u.const_value;
                    int l = t1->u.const_value;
                    int r = t2->u.const_value;
                    if (l < 0 && r > 0) {
                        val = (l - r + 1) / r;
                    } else if (l > 0 && r < 0) {
                        val = (l - r - 1) / r;
                    } else {
                        val = r ? l / r : 0;
                    }
                }
                place->u.const_value = val;
            }
            else{
                if(0 == strcmp(children->childs[1]->name, "PLUS")){
                    kind = IR_ADD;
                    // val = t1->u.const_value + t2->u.const_value;
                }
                else if(0 == strcmp(children->childs[1]->name, "MINUS")){
                    kind = IR_SUB;
                    // val = t1->u.const_value - t2->u.const_value;
                }
                else if(0 == strcmp(children->childs[1]->name, "STAR")){
                    kind = IR_MUL;
                    // val = t1->u.const_value * t2->u.const_value;
                }
                else{
                    kind = IR_DIV;
                    // val = t1->u.const_value / t2->u.const_value;
                }
                new_ir(kind, place, t1, t2, 0, NULL);
            }
        }
        else if(num == 3 && 0 == strcmp(children->childs[0]->name, "LP")){//9
            tr_Exp(children->childs[1], place);
        }
        //     | NOT Exp                 11        
        //     | ID LP Args RP           12          
        //     | ID LP RP                13         
        //     | Exp LB Exp RB           14 
        //     | Exp DOT ID              15      
        //     | ID                      16
        else if(num == 4 && 0 == strcmp(children->childs[1]->name, "LP")){//12
            FieldList entry = find(children->childs[0]->type_content);
            Assert(entry);
            if(0 == strcmp(children->childs[0]->type_content, "write")){
                tr_Args(children->childs[2], 1);
                place->kind = OP_CONSTANT;
                place->u.const_value = 0;
            }
            else{
                tr_Args(children->childs[2], 0);
                Operand t = new_operand(OP_FUNCTION, 0, 0, entry->name);
                new_ir(IR_CALL, place, t, NULL, 0, NULL);
            }
        }
        else if(num == 3 && 0 == strcmp(children->childs[1]->name, "LP")){//13
            FieldList entry = find(children->childs[0]->type_content);
            Assert(entry);
            if(0 == strcmp(children->childs[0]->type_content, "read")){
                new_ir(IR_READ, place, NULL, NULL, 0, NULL);
            }
            else{
                Operand t = new_operand(OP_FUNCTION, 0, 0, entry->name);
                new_ir(IR_CALL, place, t, NULL, 0, NULL);
            }
        }
        else if(num == 4 && 0 == strcmp(children->childs[1]->name, "LB")){//14
            Operand t1 = new_temp();
            tr_Exp(children->childs[0], t1);
            Operand t2 = new_temp();
            tr_Exp(children->childs[2], t2);
            Operand offset = new_temp();
            int width = get_size(t1->type);
            if(t2->kind == OP_CONSTANT){
                offset->kind = OP_CONSTANT;
                offset->u.const_value = width * t2->u.const_value;
            }
            else{
                if(t2->kind == OP_ADDRESS){
                    load_val(t2);
                }
                Operand op_width = new_operand(OP_CONSTANT, width, 0, NULL);
                new_ir(IR_MUL, offset, t2, op_width, 0, NULL);
            }
            place->kind = OP_ADDRESS;
            place->u.addr_no = addr_no++;
            if(t1->kind == OP_ARRAY){
                if(offset->kind == OP_CONSTANT && offset->u.const_value == 0){
                    new_ir(IR_ADDR, place, t1, NULL, 0, NULL);
                }
                else{
                    Operand base = new_addr();
                    new_ir(IR_ADDR, base, t1, NULL, 0, NULL);    
                    new_ir(IR_ADD, place, base, offset, 0, NULL);
                }
            }
            else{
                Assert(t1->kind == OP_ADDRESS);
                new_ir(IR_ADD, place, t1, offset, 0, NULL);
            }
            if(t1->type->kind == BASIC){
                place->type = NULL;
                place->size = 0;
            }
            else{
                // Assert(t1->type->kind == ARRAY);
                place->type = t1->type->u.array.elem;
                place->size = t1->type->u.array.size;
            }
        }
        else if(num == 3 && 0 == strcmp(children->childs[1]->name, "DOT")){
            Operand t1 = new_temp();
            tr_Exp(children->childs[0], t1);
            char *name = children->childs[2]->type_content;
            FieldList entry = find(name);
            
            place->kind = OP_ADDRESS;
            place->u.addr_no = addr_no++;

            Operand offset = new_temp();
            offset->kind = OP_CONSTANT;
            offset->u.const_value = entry->offset;

            if(t1->kind == OP_STRUCTURE){
                if(entry->offset > 0){
                    Operand base = new_addr();
                    new_ir(IR_ADDR, base, t1, NULL, 0, NULL);    
                    new_ir(IR_ADD, place, base, offset, 0, NULL);
                }
                else{
                    new_ir(IR_ADDR, place, t1, NULL, 0, NULL);
                }
            }
            else{
                Assert(t1->kind == OP_ADDRESS);
                new_ir(IR_ADD, place, t1, offset, 0, NULL);
            }
            
            
            if(entry->type.kind == ARRAY){
                place->type = entry->type.u.array.elem;
            }
            else if(entry->type.kind == STRUCTURE){
                place->structure = entry->type.u.structure;
            }
        }
}


// Args: Exp COMMA Args  
//     | Exp
void tr_Args(Node *cur, int is_write){
    if(cur == NULL) return;
    child_node *children = get_childs(cur);
    int num = children->num;
    Assert(num == cur->right_num);
    Assert(0 == strcmp(children->childs[0]->name, "Exp"));
    if(num == 3){
        Assert(!is_write);
        tr_Args(children->childs[2], 0);
    }
    Operand t = new_temp();
    tr_Exp(children->childs[0], t);
    if(is_write){
        if(t->kind == OP_ADDRESS){
            load_val(t);
        }
        new_ir(IR_WRITE, t, NULL, NULL, 0, NULL);
    }
    else{
        if(t->kind == OP_ARRAY || t->kind == OP_STRUCTURE){
            t = get_addr(t);
        }
        else if(t->kind == OP_ADDRESS){
            if(t->type == NULL){
                load_val(t);
            }
        }
        new_ir(IR_ARG, t, NULL, NULL, 0, NULL);
    }
}

//Cond: Exp RELOP EXP 1
//     |NOT Exp       2
//     |Exp AND Exp   3
//     |Exp OR Exp    4
//     |Other         5
void tr_Cond(Node *cur, Operand true_label, Operand false_label){
    if(cur == NULL);
    child_node *children = get_childs(cur);
    int num = children->num;
    Assert(num == cur->right_num);
    
    if(num == 3 && 0 == strcmp(children->childs[1]->name, "RELOP")){//1
        Operand t1 = new_temp();
        tr_Exp(children->childs[0], t1);
        if(t1->kind == OP_ADDRESS){
            load_val(t1);
        }
        Operand t2 = new_temp();
        tr_Exp(children->childs[2], t2);
        if(t2->kind == OP_ADDRESS){
            load_val(t2);
        }
        char *op = children->childs[1]->type_content;
        new_ir(IR_IF_GOTO, t1, t2, true_label, 0, op);
        new_ir(IR_GOTO, false_label, NULL, NULL, 0, NULL);
    }
    else if(num == 2 && 0 == strcmp(children->childs[0]->name, "NOT")){//2
        tr_Cond(children->childs[1], false_label, true_label);
    }
    else if(num == 3 && 0 == strcmp(children->childs[1]->name, "AND")){//3
        Operand l1 = new_label();
        tr_Cond(children->childs[0], l1, false_label);
        new_ir(IR_LABEL, l1, NULL, NULL, 0, NULL);
        tr_Cond(children->childs[2], true_label, false_label);
    }
    else if(num == 3 && 0 == strcmp(children->childs[1]->name, "OR")){//4
        Operand l1 = new_label();
        tr_Cond(children->childs[0], true_label, l1);
        new_ir(IR_LABEL, l1, NULL, NULL, 0, NULL);
        tr_Cond(children->childs[2], true_label, false_label);
    }
    else{
        Operand t = new_temp();
        tr_Exp(cur, t);
        if(t->kind == OP_ADDRESS){
            load_val(t);
        }
        Operand zero_op = new_operand(OP_CONSTANT, 0, 0, NULL);
        new_ir(IR_IF_GOTO, t, zero_op, true_label, 0, "!=");
        new_ir(IR_GOTO, false_label, NULL, NULL, 0, NULL);
    }
}

void array_copy(Operand dst, Operand src){
    Operand dst_addr = dst, src_addr = src;
    if(dst_addr->kind == OP_ARRAY){
        dst_addr = get_addr(dst_addr);
    }
    if(src_addr->kind == OP_ARRAY){
        src_addr = get_addr(src_addr);
    }
    int dst_size = get_size(dst->type) * dst->size, src_size = get_size(src->type) * src->size;
    int size = min(dst_size, src_size);
    Operand left = new_addr(), right = new_addr(), tmp = new_temp();
    // Operand tmp = new_temp();
    new_ir(IR_LOAD, tmp, src_addr, NULL, 0, NULL);
    new_ir(IR_STORE, dst_addr, tmp, NULL, 0, NULL);
    for(int i = 4; i < size; i += 4){
        Operand offset = new_operand(OP_CONSTANT, i, 0, NULL);
        new_ir(IR_ADD, left, dst_addr, offset, 0, NULL);
        new_ir(IR_ADD, right, src_addr, offset, 0, NULL);
        new_ir(IR_LOAD, tmp, right, NULL, 0, NULL);
        new_ir(IR_STORE, left, tmp, NULL, 0, NULL);
    }
}
