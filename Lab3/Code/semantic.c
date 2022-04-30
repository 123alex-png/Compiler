#include "semantic.h"
#include "stack.h"
#include "common.h"
// #include "symbol_table.h"
extern ScopeList scope_stack[0x3fff+1];
static ScopeList global_sc;
//Program: ExtDefList

void insert_function(char *name){
    FieldList f = (FieldList)malloc(sizeof(struct FieldList_));
    f->is_arg = 0;
    strcpy(f->name, name);
    f->type.has_define = 1;
    f->type.kind = FUNCTION;
    f->type.u.function.argc = 0;
    f->tail = NULL;
    Type int_type = (Type)malloc(sizeof(struct Type_));
    int_type->kind = BASIC;
    int_type->u.basic = BASIC_INT;
    f->type.u.function.return_type = int_type;
    if(0 == strcmp(name, "write")){
        FieldList arg = (FieldList)malloc(sizeof(struct FieldList_));
        arg->is_arg = 0;
        arg->tail = NULL;
        arg->type = *int_type;
        f->type.u.function.argc = 1;
        f->type.u.function.argv = arg;
    }
    else{
        f->type.u.function.argc = 0;
        f->type.u.function.argv = NULL;
    }
    hash_insert(f, global_sc);
}

void Program(Node *cur){
    if(cur == NULL) return;
    global_sc = create_scope();
    insert_function("read");
    insert_function("write");
    child_node *children = get_childs(cur);
    Assert(cur->right_num == 1);
    ExtDefList(children->childs[0]);
    check_fundec();
    delete_scope();
}

// ExtDefList: ExtDef ExtDefList | /*Empty*/ 
void ExtDefList(Node *cur){
    if(cur == NULL) return;
    child_node *children = get_childs(cur);
    Assert(cur->right_num == 2);
    ExtDef(children->childs[0]);
    ExtDefList(children->childs[1]);
}

// ExtDef: Specifier ExtDecList SEMI    (1)
//         | Specifier SEMI             (2)
//         | Specifier FunDec CompSt    (3)
//         | Specifier FunDec SEMI      (4)
void ExtDef(Node *cur){
    if(cur == NULL) return;
    child_node *children = get_childs(cur);
    // int num = children->num;
    Assert(0 == strcmp(children->childs[0]->name, "Specifier"));
    Type type = Specifier(children->childs[0]);
    if(cur->right_num == 2){//(2)
        Assert(0 == strcmp(children->childs[1]->name, "SEMI"));
    }
    else{
        Assert(cur->right_num == 3);
        if(0 == strcmp(children->childs[1]->name, "ExtDecList")){//(1)
            Assert(0 == strcmp(children->childs[2]->name, "SEMI"));
            ExtDecList(children->childs[1], type);
        }
        else{
            if(0 == strcmp(children->childs[2]->name, "CompSt")){//(3)define
                Assert(0 == strcmp(children->childs[1]->name, "FunDec"));
                FunDec(children->childs[1], type, 1);
                CompSt(children->childs[2], type);
                delete_scope();
            }
            else if(0 == strcmp(children->childs[2]->name, "SEMI")){//(4)declare
                Assert(0 == strcmp(children->childs[1]->name, "FunDec"));
                FunDec(children->childs[1], type, 0);
            }
            else{
                Assert(0);
            }
            
        }
    }
}

// ExtDecList: VarDec                       (1)
//             | VarDec COMMA ExtDecList    (2)
void ExtDecList(Node *cur, Type type){
    if(cur == NULL) return;
    child_node *children = get_childs(cur);
    // int num = children->num;
    Assert(0 == strcmp(children->childs[0]->name, "VarDec"));
    if(cur->right_num == 1){//(1)
        VarDec(children->childs[0], type, NULL);
    }
    else{//(2)
        Assert(cur->right_num == 3);
        VarDec(children->childs[0], type, NULL);
        ExtDecList(children->childs[2], type);
    }
}

// Specifier: TYPE    
//            | StructSpecifier 
Type Specifier(Node *cur){
    if(cur == NULL) return NULL;
    child_node *children = get_childs(cur);
    // int num = children->num;
    if(0 == strcmp(children->childs[0]->name, "TYPE")){//(1)
        Type ret = (Type)malloc(sizeof(struct Type_));
        Node *t = children->childs[0];
        ret->kind = BASIC;
        if(0 == strcmp(t->type_content, "int")){
            ret->u.basic = BASIC_INT;
        }
        else{
            Assert(0 == strcmp(t->type_content, "float"));
            ret->u.basic = BASIC_FLOAT;
        }
        return ret;
    }
    //(2)
    Assert(0 == strcmp(children->childs[0]->name, "StructSpecifier"));
    return StructSpecifier(children->childs[0]);
}

// StructSpecifier: STRUCT OptTag LC DefList RC (1)
//                 | STRUCT Tag                 (2)
Type StructSpecifier(Node *cur){
    if(cur == NULL) return NULL;
    Type ret = NULL;
    child_node *children = get_childs(cur);
    int num = children->num;
    Assert(0 == strcmp(children->childs[0]->name, "STRUCT"));
    if(cur->right_num == 5){//(1)
        char *tag_name = NULL;
        Node *q = find_child(cur, "OptTag");
        if(q){
            tag_name = OptTag(children->childs[1]);
        }
        if(tag_name && find(tag_name)){
            semantic_error(16, cur->row, tag_name, NULL);
            return NULL;
        }
        ret = (Type)malloc(sizeof(struct Type_));
        ret->kind = STRUCTURE;
        FieldList new_field = (FieldList)malloc(sizeof(struct FieldList_));
        new_field->is_arg = 0;
        new_field->tail = NULL;
        if(tag_name) strcpy(new_field->name, tag_name);
        new_field->type.kind = STRUCTURE_TAG;
        new_field->type.u.structure = NULL;
        new_field->type.has_define = 1;
        if(tag_name) {
            hash_insert(new_field, global_sc);
        }
        // Assert(0 == strcmp(children->childs[num-2]->name, "DefList"));
        Node *w = find_child(cur, "DefList");
        if(w){
            create_scope();
            DefList(children->childs[num-2], new_field);
            delete_scope();
        }
        
        ret->u.structure = new_field;
    }
    else{//(2)
        Assert(cur->right_num == 2);
        char *tag_name = Tag(children->childs[1]);
        Assert(tag_name);
        FieldList field = find(tag_name);
        if(!field || !field->type.has_define){
            semantic_error(17, cur->row, tag_name, NULL);
            return NULL;
        }
        ret = (Type)malloc(sizeof(struct Type_));
        ret->kind = STRUCTURE;
        ret->u.structure = field;
    }
    return ret;
}

// OptTag: ID
char *OptTag(Node *cur){
    if(cur == NULL) return NULL;
    child_node *children = get_childs(cur);
    return children->childs[0]->type_content;
}

// Tag: ID  
char *Tag(Node *cur){
    if(cur == NULL) return NULL;
    child_node *children = get_childs(cur);
    return children->childs[0]->type_content;
}

// VarDec: ID  (1)
//         | VarDec LB BASIC_INT RB  (2)
FieldList VarDec(Node *cur, Type type, FieldList field){
    if(cur == NULL) return NULL;
    FieldList ret;
    child_node *children = get_childs(cur);
    int num = children->num;
    Assert(num == cur->right_num);
    if(num == 1){//(1)
        char *name = children->childs[0]->type_content;
        if(field && field->type.kind == STRUCTURE_TAG && find_member_in_structure(field, name)){
            semantic_error(15, cur->row, name, NULL);
            return NULL;
        }
        ret = (FieldList)malloc(sizeof(struct FieldList_));
        ret->is_arg = 0;
        strcpy(ret->name, name);
        ret->type = *type;
        ret->tail = NULL;
        FieldList entry = find(name);
        if(entry){
            if(entry->type.kind == STRUCTURE_TAG || type->kind == STRUCTURE_TAG){
                semantic_error(3, cur->row, name, NULL);
            }
            else if(is_same_scope(entry)){
                semantic_error(3, cur->row, name, NULL);
            }
            else{
                ScopeList sc = scope_stack_top();
                hash_insert(ret, sc);
            }
        }
        // if(entry && is_same_scope(entry)){
        //     semantic_error(3, cur->row, name, NULL);
        // }
        else{
            ScopeList sc = scope_stack_top();
            hash_insert(ret, sc);
        }
        return ret;
    }
    //(2)
    Type array = (Type)malloc(sizeof(struct Type_));
    array->kind = ARRAY;
    array->u.array.size = children->childs[2]->type_int;
    array->u.array.elem = type;
    return VarDec(children->childs[0], array, field);
}

// FunDec: ID LP VarList RP   (1)
//         | ID LP RP         (2)
void FunDec(Node *cur, Type type, int is_def){
    if(cur == NULL) return;
    child_node *children = get_childs(cur);
    int num = children->num;
    Assert(num == cur->right_num);
    char *name = children->childs[0]->type_content;
    FieldList entry = find(name);
    FieldList f = (FieldList)malloc(sizeof(struct FieldList_));
    f->tail = NULL;
    strcpy(f->name, name);
    f->is_arg = 0;
    f->type.kind = FUNCTION;
    f->type.u.function.return_type = type;
    f->type.u.function.argv =NULL;
    f->type.u.function.argc = 0;
    f->type.has_define = is_def;
    if(!entry) {
        ScopeList sc = scope_stack_top();
        hash_insert(f, sc);
        func_insert(f, cur->row);
    }
    create_scope();
    if(num == 4){//(1)
        VarList(children->childs[2], f);
    }
    else{
        Assert(num == 3);
        f->type.u.function.argc = 0;
        f->type.u.function.argv = NULL;
    }
    if(entry){
        update_row(entry->type.u.function.p, cur->row);
        if(entry->type.has_define && is_def){
            semantic_error(4, cur->row, name, NULL);
        }
        else{
            if(!check_function_define(entry, f)){
                semantic_error(19, cur->row, name, NULL);
            }
            if(is_def){
                entry->type.has_define = 1;
                entry->type.u.function.p->has_def = 1;
            }
        }
    }
    return;
}

// VarList: ParamDec COMMA VarList (1)
//         | ParamDec    (2)
void VarList(Node *cur, FieldList field){
    if(cur == NULL) return;
    child_node *children = get_childs(cur);
    int num = children->num;
    Assert(num == cur->right_num);
    Assert(0 == strcmp(children->childs[0]->name, "ParamDec"));
    struct func *f = &field->type.u.function;
    FieldList arg = ParamDec(children->childs[0]);
    arg->is_arg = 1;
    Assert(arg);
    FieldList t = f->argv;
    if(!t){
        f->argv = arg;
    }
    else{
        FieldList nxt = t->tail;
        while(nxt){
            t = t->tail;
            nxt = nxt->tail;
        }
        t->tail = arg;
    }
    // arg->tail = t;
    // f->argv = arg;
    ++f->argc;
    if(num == 3){//(1)
        VarList(children->childs[2], field);
    }
    else{
        Assert(num == 1);
    }
}

// ParamDec: Specifier VarDec 
FieldList ParamDec(Node *cur){
    if(cur == NULL) return NULL;
    child_node *children = get_childs(cur);
    int num = children->num;
    Assert(num == cur->right_num);
    Type type = Specifier(children->childs[0]);
    Assert(type);
    return VarDec(children->childs[1], type, NULL);
}

// CompSt: LC DefList StmtList RC 
void CompSt(Node *cur, Type type){
    if(cur == NULL) return;
    child_node *children = get_childs(cur);
    int num = children->num;
    // Assert(num == cur->right_num);//存疑
    Node *n1 = find_child(cur, "DefList"), *n2 = find_child(cur, "StmtList");
    DefList(n1, NULL);
    StmtList(n2, type);
}

// StmtList: Stmt StmtList 
void StmtList(Node *cur, Type type){
    if(cur == NULL) return;
    child_node *children = get_childs(cur);
    int num = children->num;
    // Assert(num == cur->right_num);//存疑
    Node *n1 = find_child(cur, "Stmt"), *n2 = find_child(cur, "StmtList");
    Stmt(n1, type);
    StmtList(n2, type);
}

// Stmt: Exp SEMI   1
//     | CompSt      2          
//     | RETURN Exp SEMI   3
//     | IF LP Exp RP Stmt  4
//     | IF LP Exp RP Stmt ELSE Stmt  5
//     | WHILE LP Exp RP Stmt   6
void Stmt(Node *cur, Type type){
    if(cur == NULL) return;
    child_node *children = get_childs(cur);
    int num = children->num;
    Assert(num == cur->right_num);
    if(num == 2){//1
        Exp(children->childs[0]);
    }
    else if(num == 1){//2
        create_scope();
        CompSt(children->childs[0], type);
        delete_scope();
    }
    else if(num == 3){//3
        Type t = Exp(children->childs[1]);
        // Assert(t);
        if(check_type(t, type) == 0){
            semantic_error(8, cur->row, NULL, NULL);
        }
    }
    else if(num == 7){//5
        Type t = Exp(children->childs[2]);
        Assert(t);
        if(!(t->kind == BASIC && t->u.basic == BASIC_INT)){
            semantic_error(7, cur->row, NULL, NULL);
        }
        Stmt(children->childs[4], type);
        Stmt(children->childs[6], type);
    }
    else {  //4 or 6
        Assert(num == 5);
        Type t = Exp(children->childs[2]);
        // Assert(t);
        if(t && !(t->kind == BASIC && t->u.basic == BASIC_INT)){
            semantic_error(7, cur->row, NULL, NULL);
        }
        Stmt(children->childs[4], type);
    }
}

// DefList: Def DefList  
void DefList(Node *cur, FieldList field){
    if(cur == NULL) return;
    child_node *children = get_childs(cur);
    int num = children->num;
    // Assert(num == cur->right_num);
    Node *n1 = find_child(cur, "Def"), *n2 = find_child(cur, "DefList");
    Def(children->childs[0], field);
    DefList(children->childs[1], field);
}

// Def: Specifier DecList SEMI
void Def(Node *cur, FieldList field){
    if(cur == NULL) return;
    child_node *children = get_childs(cur);
    int num = children->num;
    Assert(num == cur->right_num);
    Type type = Specifier(children->childs[0]);
    // Assert(type);
    if(type){
        DecList(children->childs[1], type, field);
    }
}

// DecList: Dec   
//         | Dec COMMA DecList
void DecList(Node *cur, Type type, FieldList field){
    if(cur == NULL) return;
    child_node *children = get_childs(cur);
    int num = children->num;
    Assert(num == cur->right_num);
    Dec(children->childs[0], type, field);
    if(num == 3){ 
        // Assert(num == 3);
        DecList(children->childs[2], type, field);
    }
    else{
        Assert(num == 1);
    }
}

// Dec: VarDec   
//     | VarDec ASSIGNOP Exp 
void Dec(Node *cur, Type type, FieldList field){
    if(cur == NULL) return;
    child_node *children = get_childs(cur);
    int num = children->num;
    Assert(num == cur->right_num);
    FieldList f = VarDec(children->childs[0], type, field);
    f->is_arg = 0;
    if(field && field->type.kind == STRUCTURE_TAG){
        if(f){
            FieldList t = field->type.u.structure;
            f->tail = t;
            field->type.u.structure = f;
        }
        if(num == 3){
            semantic_error(15, cur->row, NULL, NULL);
            return;
        }
    }
    if(num == 3){
        Assert(0 == strcmp(children->childs[2]->name, "Exp"));
        Type exp_type = Exp(children->childs[2]);
        // Assert(exp_type);
        if(check_type(exp_type, &f->type) == 0){
            semantic_error(5, cur->row, NULL, NULL);
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
Type Exp(Node *cur){
    if(cur == NULL) return NULL;
    Type ret = NULL;
    child_node *children = get_childs(cur);
    int num = children->num;
    Assert(num == cur->right_num);
    if(num == 3 && 0 == strcmp(children->childs[0]->name, "Exp") 
        && 0 == strcmp(children->childs[2]->name, "Exp")){

        if(0 == strcmp(children->childs[1]->name, "ASSIGNOP")){//1
            Node *left = children->childs[0], *right = children->childs[2];
            ret = Exp(left);
            //左值只有14，15，17合法
            child_node *t = get_childs(left);
            if(!((left->right_num == 1 && 0 == strcmp(t->childs[0]->name, "ID"))
              ||(left->right_num == 4 && 0 == strcmp(t->childs[1]->name, "LB"))
              ||(left->right_num == 3 && 0 == strcmp(t->childs[1]->name, "DOT")))){
                  semantic_error(6, cur->row, NULL, NULL);
              }
            if(check_type(ret, Exp(right)) == 0){
                semantic_error(5, cur->row, NULL, NULL);
            }
        }
        else{//2,3,4,5,6,7,8
            Type type_left = Exp(children->childs[0]), type_right = Exp(children->childs[2]);
            if(check_type(type_left, type_right) == 0){
                semantic_error(7, cur->row, NULL, NULL);
            }
            else{
                enum{ BASIC_INT, BASIC_FLOAT} basic; 
                if(0 == strcmp(children->childs[1]->name, "AND")
                 ||0 == strcmp(children->childs[1]->name, "OR")
                 ||0 == strcmp(children->childs[1]->name, "RELOP")){
                    if(type_left && !(type_left->kind == BASIC && type_left->u.basic == BASIC_INT)){
                        semantic_error(7, cur->row, NULL, NULL);
                    }
                    basic = BASIC_INT;
                }
                else{
                    if(type_left && !(type_left->kind == BASIC)){
                        semantic_error(7, cur->row, NULL, NULL);
                    }
                    basic = type_left->u.basic;
                }
                ret = (Type)malloc(sizeof(struct Type_));
                ret->kind = BASIC;
                ret->u.basic = basic;
            }
        }
        // else if(0 == strcmp(children->childs[0]->name, "LP")){//9
        //     ret = Exp(children->childs[1]);
        // }
        // else if(0 == strcmp(children->childs[0]->name, "ID")){//13

        // }
    }
    else if(num == 3 && 0 == strcmp(children->childs[0]->name, "LP")){//9
        ret = Exp(children->childs[1]);
    }
    else if(num == 2 && 0 == strcmp(children->childs[0]->name, "MINUS")){//10
        ret = Exp(children->childs[1]);
        if(ret && !(ret->kind == BASIC)){
            semantic_error(7, cur->row, NULL, NULL);
        }
    }
    else if(num == 2 && 0 == strcmp(children->childs[0]->name, "NOT")){//11
        ret = Exp(children->childs[1]);
        if(ret && !(ret->kind == BASIC && ret->u.basic == BASIC_INT)){
            semantic_error(7, cur->row, NULL, NULL);
        }
    }
    else if(num >= 3 && 0 == strcmp(children->childs[1]->name, "LP")){//12,13
        Assert(0 == strcmp(children->childs[0]->name, "ID"));
        FieldList entry = find(children->childs[0]->type_content);
        if(!entry){
            semantic_error(2, cur->row, children->childs[0]->type_content, NULL);
        }
        else if(entry->type.kind != FUNCTION){
            semantic_error(11, cur->row, children->childs[0]->type_content, NULL);
        }
        else if(num == 3){//13
            if(entry->type.u.function.argv){
                // char t[130];
                // sprintf(t, "%s%s", children->childs[0]->type_content, type2name(entry->type.u.function.argv));
                semantic_error(9, cur->row, NULL, NULL);
            }
        }
        else{//12
            Assert(num == 4);
            FieldList args = Args(children->childs[2]);
            if(check_args(args, entry->type.u.function.argv) == 0){
                // char t[130];
                // sprintf(t, "%s%s", children->childs[0]->type_content, type2name(entry->type.u.function.argv));
                semantic_error(9, cur->row, NULL, NULL);
            }
        }
        if(entry && entry->type.kind == FUNCTION){
            ret = entry->type.u.function.return_type;
        }
    }   
//     | Exp LB Exp RB           14 
//     | Exp DOT ID              15      
//     | ID                      16
//     | BASIC_INT                     17     
//     | BASIC_FLOAT                   18   
    else if(num == 4){//14
        Type type_id = Exp(children->childs[0]), type_no = Exp(children->childs[2]);
        if(type_id){
            if(type_id->kind != ARRAY){
                semantic_error(10, cur->row, NULL, NULL);
            }
            else{
                ret = type_id->u.array.elem;
            }
        }
        if(type_no){
            if(!(type_no->kind ==BASIC && type_no->u.basic == BASIC_INT)){
                semantic_error(12, cur->row, NULL, NULL);
            }
        }
    }
    else if(num == 3){//15
        Type type_struct = Exp(children->childs[0]);
        if(type_struct){
            if(type_struct->kind != STRUCTURE){
                semantic_error(13, cur->row, ".", NULL);
            }
            else{
                char *name = children->childs[2]->type_content;
                FieldList r = find_member_in_structure(type_struct->u.structure, name);
                if(!r){
                    semantic_error(14, cur->row, name, NULL);
                }
                else{
                    ret = &r->type;
                }
            }
        }
    }
    else if(0 == strcmp(children->childs[0]->name, "ID")){//16
        FieldList entry = find(children->childs[0]->type_content);
        if(!entry || !(entry->type.kind == BASIC || entry->type.kind == STRUCTURE || entry->type.kind == ARRAY)){
            semantic_error(1, cur->row, children->childs[0]->type_content, NULL);
        }
        else{
            ret = &entry->type;
        }
    }
    else{//17,18
        ret = (Type)malloc(sizeof(struct Type_));
        ret->kind = BASIC;
        if(0 == strcmp(children->childs[0]->name, "INT")){//17
            ret->u.basic = BASIC_INT;
        }
        else{//18
            Assert(0 == strcmp(children->childs[0]->name, "FLOAT"));
            ret->u.basic = BASIC_FLOAT;
        }
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
    ret->is_arg = 0;
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

void semantic_error(int type, int row, const char *name, const char *name1){
    switch (type)
    {
    case 1:
        printf("Error type 1 at Line %d: Undefined variable \"%s\".\n", row, name);
        break;
    case 2:
        printf("Error type 2 at Line %d: Undefined function \"%s\".\n", row, name);
        break;
    case 3:
        printf("Error type 3 at Line %d: Redefined variable \"%s\".\n", row, name);
        break;
    case 4:
        printf("Error type 4 at Line %d: Redefined function \"%s\".\n", row, name);
        break;
    case 5:
        printf("Error type 5 at Line %d: Type mismatched for assignment.\n", row);
        break;
    case 6:
        printf("Error type 6 at Line %d: The left-hand side of an assignment must be a variable.\n", row);
        break;
    case 7:
        printf("Error type 7 at Line %d: Type mismatched for operands.\n", row);
        break;
    case 8:
        printf("Error type 8 at Line %d: Type mismatched for return.\n", row);
        break;
    case 9:
        printf("Error type 9 at Line %d: Function is not applicable for arguments .\n", row);
        break;
    case 10:
        printf("Error type 10 at Line %d: variable is not an array.\n", row);
        break;
    case 11:
        printf("Error type 11 at Line %d: \"%s\" is not a function.\n", row, name);
        break;
    case 12:
        printf("Error type 12 at Line %d: variable is not an integer.\n", row);
        break;
    case 13:
        printf("Error type 13 at Line %d: Illegal use of \"%s\".\n", row, name);
        break;
    case 14:
        printf("Error type 14 at Line %d: Non-existent field \"%s\".\n", row, name);
        break;
    case 15:
        printf("Error type 15 at Line %d: Redefined field.\n", row);
        break;
    case 16:
        printf("Error type 16 at Line %d: Duplicated name \"%s\".\n", row, name);
        break;
    case 17:
        printf("Error type 17 at Line %d: Undefined structure \"%s\".\n", row, name);
        break;
    case 18:
        printf("Error type 18 at Line %d: Undefined function \"%s\".\n", row, name);
        break;
    case 19:
        printf("Error type 19 at Line %d: Inconsistent declaration of function \"%s\".\n", row, name);
        break;
    default:
        break;
    }
    
}

// static char *type_name[4] = {"int", "float", "structure", "function"};

// char *type2name(FieldList args){
//     if(args == NULL) return "()";
//     char *ret = (char *)malloc(sizeof(char)*100);  
//     int i = 1;
//     while(args){
//         if(args->type.kind == BASIC){
//             if(args->type.u.basic == BASIC_INT){
//                 sprintf(ret+i, "%s", type_name[0]);
//                 i+=3;
//             }
//             else if(args->type.kind == BASIC_FLOAT){
//                 sprintf(ret+i, "%s", type_name[1]);
//                 i+=5;
//             }
//         }
//         else if(args->type.kind == STRUCTURE){
//             sprintf(ret+i, "%s", type_name[2]);
//             i+=9;
//         }
//         else if(args->type.kind == FUNCTION){
//             sprintf(ret+i, "%s", type_name[3]);
//             i+=8;
//         }
//         else{
//             Assert(0);
//         }
//         args = args->tail;
//         if(args){
//             ret[i++] = ',';
//             ret[i++] = ' ';
//         }
//     }
//     ret[0] = '(';
//     ret[i++] = ')';
//     ret[i] = '\0';
//     return ret;
// }