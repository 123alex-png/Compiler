#include "symbol_table.h"
#include "semantic.h"
#include "stack.h"
#include "common.h"

unsigned int hash_val(char* name){ 
    unsigned int val = 0, i; 
    for (; *name; ++name){ 
        val = (val << 2) + *name; 
        if (i = val & ~HASH_TABLE_SIZE) val = (val ^ (i >> 12)) & HASH_TABLE_SIZE; 
    } 
    return val; 
}



void hash_insert(FieldList field, ScopeList cur){
    unsigned int v = hash_val(field->name);
    SymbolNode obj = hash_table[v];
    SymbolNode new_node = (SymbolNode)malloc(sizeof(struct SymbolNode_));
    new_node->val = field;
    new_node->next_node = obj;
    new_node->hash_val = v;
    hash_table[v] = new_node;
    if(obj){
        obj->prev_node = new_node;
    }
    //要求2.2新增代码
    // ScopeList cur = scope_stack_top();
    field->scope = cur;
    if(!cur->head){
        cur->head = cur->tail = new_node;
        new_node->tail_node = NULL;
    }
    else{
        cur->tail->tail_node = new_node;
        cur->tail = new_node;
    }
}

FieldList find(char* name){
    unsigned int v = hash_val(name);
    SymbolNode obj = hash_table[v];
    for(; obj; obj = obj->next_node){
        if(0 == strcmp(obj->val->name, name)){
            return obj->val;
        }
    }
    return NULL;
}

FieldList find_member_in_structure(FieldList f, char *var_name){
    FieldList cur = f->type.u.structure;
    for(; cur; cur = cur->tail){
        if(0 == strcmp(cur->name, var_name)){
            return cur;
        }
    }
    return NULL;
}

int check_function_define(FieldList old, FieldList new){
    struct func *x = &old->type.u.function, *y = &new->type.u.function;
    if(!check_type(x->return_type, y->return_type)) return 0;
    if(x->argc != y->argc) return 0;
    int n = x->argc;
    FieldList cur_arg_x = x->argv, cur_arg_y = y->argv;
    for(int i = 0; i < n; i++){
        if(!check_type(&cur_arg_x->type, &cur_arg_y->type)) return 0;
        cur_arg_x = cur_arg_x->tail;
        cur_arg_y = cur_arg_y->tail;
    }
    return 1;
}

int check_type_dfs(Type a, Type b, int depth){
    // Assert(a->kind != STRUCTURE_TAG);
    // Assert(b->kind != STRUCTURE_TAG);
    if(depth > 100000) return 1;
    if(!a || !b) return 1;
    if(a->kind != b->kind) return 0;
    if(a->kind == BASIC){
        return a->u.basic == b->u.basic;
    }
    if(a->kind == ARRAY){
        if(a->u.array.size != a->u.array.size) return 0;
        return check_type_dfs(a->u.array.elem, b->u.array.elem, depth+1);
    }
    if(a->kind == STRUCTURE_TAG){
        FieldList cur_a = a->u.structure, cur_b = b->u.structure;
        while(cur_a && cur_b){
            if(check_type_dfs(&cur_a->type, &cur_b->type, depth+1) == 0) return 0;
            cur_a = cur_a->tail;
            cur_b = cur_b->tail;
        }
        if(cur_a || cur_b) return 0;//结构体内域的数目不等
        return 1;
    }
    if(a->kind == STRUCTURE){
        return check_type_dfs(&a->u.structure->type, &b->u.structure->type, depth+1);
    }
    Assert(0);
}

int check_type(Type a, Type b){
    return check_type_dfs(a, b, 0);
}

int check_args(FieldList a, FieldList b){
    FieldList cur_a = a, cur_b = b;
    while(cur_a && cur_b){
        if(!check_type(&cur_a->type, &cur_b->type)) return 0;
        cur_a = cur_a->tail;
        cur_b = cur_b->tail;
    }
    if(cur_a || cur_b) return 0;
    return 1;
}

//要求2.2新增代码
void delete_node(SymbolNode node){
    Assert(node);
    SymbolNode nxt = node->next_node, prv = node->prev_node;
    void *head = hash_table[node->hash_val];
    if(node == head){
        hash_table[node->hash_val] = nxt;
    }
    else{
        Assert(prv);
        prv->next_node = nxt;
        if(nxt){
            nxt->prev_node = prv;
        }
    }
    // free(node);
}

//要求2.1新增代码
void func_insert(FieldList field, int row){
    FunDecList new_dec = (FunDecList)malloc(sizeof(struct FunDecList_));
    Assert(new_dec);
    field->type.u.function.p = new_dec;
    strcpy(new_dec->name, field->name);
    new_dec->row = row;
    new_dec->has_def = field->type.has_define;
    if(!fun_dec_head){
        Assert(!fun_dec_tail);
        fun_dec_head = fun_dec_tail = new_dec;
    }
    else{
        Assert(fun_dec_tail);
        fun_dec_tail->next = new_dec;
        fun_dec_tail = new_dec;
    }

}
//要求2.1新增代码
void update_row(FunDecList p, int row){
    Assert(p);
    p->row = row;
}

//要求2.1新增代码
void check_fundec(){
    FunDecList cur = fun_dec_head;
    for(; cur; cur = cur->next){
        if(!cur->has_def){
            // printf("Error type 18 at Line %d: Undefined function \"%s\".\n", cur->row, cur->name);
            semantic_error(18, cur->row, cur->name, NULL);
        }
    }
}

//要求2.2新增代码
ScopeList create_scope(){
    ScopeList ret = (ScopeList)malloc(sizeof(struct ScopeList_));
    *ret = (struct ScopeList_){NULL, NULL, NULL};
    scope_stack_push(ret);
    return ret;
}

//要求2.2新增代码
void delete_scope(){
    Assert(!scope_stack_empty());
    ScopeList t = scope_stack_top();
    SymbolNode cur = t->head;
    for(; cur; cur = cur->tail_node){
        delete_node(cur);
    }
    while(cur){
        SymbolNode prv = cur;
        // delete_node(prv);
        cur = prv->tail_node;
        free(prv);
    }
    t->head = t->tail =  NULL;
    t->next = NULL;
    scope_stack_pop();
    free(t);
}

//要求2.2新增代码
int is_same_scope(FieldList f){
    ScopeList cur_scope = scope_stack_top();
    return cur_scope == f->scope;
}