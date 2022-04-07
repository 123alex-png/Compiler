#include "SyntaxTree.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

Node *get_node(const char *name, const char *val, enum TYPE type, int row){
    Node *ret = (Node *)malloc(sizeof(Node));
    strcpy(ret->name, name);
    ret->child = ret->nxt = NULL;
    ret->type = type;
    ret->row = row;
    ret->is_token = 1;
    if(type == TYPE_DEC){
        sscanf(val, "%u", &ret->type_int);
    }
    else if(type == TYPE_HEX){
        sscanf(val, "%x", &ret->type_int);
    }
    else if(type == TYPE_OCT){
        sscanf(val, "%o", &ret->type_int);
    }
    else if(type == TYPE_FLOAT){
        sscanf(val, "%f", &ret->type_float);
    }
    else if(type == TYPE_CONTENT){
        sscanf(val, "%s", ret->type_content);
    }
    return ret;
}

Node *new_node(const char *name, int row, int num, ...){
    Node *ret = (Node *)malloc(sizeof(Node));
    ret->child = NULL;
    ret->nxt = NULL;
    ret->row = row;
    ret->is_token = 0;
    strcpy(ret->name, name);
    if(num){
        va_list ap;
        va_start(ap, num);
        Node *cur = va_arg(ap, Node *);
        ret->child = cur;
        for(int i = 1; i < num; i++){
            assert(cur);
            cur->nxt = va_arg(ap, Node *);
            if(cur->nxt){
                cur = cur->nxt;
            }
        }
        va_end(ap);
    }
    return ret;
}

void print_tree(Node *root, int depth){
    if(root == NULL) return;
    for(int i = 0; i < depth; i++){
        printf("  ");
    }
    if(root->is_token == 1){
        printf("%s", root->name);
        if(root->type == TYPE_CONTENT){
            printf(": %s\n", root->type_content);
        }
        else if(root->type == TYPE_FLOAT){
            printf(": %f\n", root->type_float);
        }
        else if(root->type != TYPE_OTHER){
            printf(": %u\n", root->type_int);
        }
        else{
            printf("\n");
        }
    }
    else{
        printf("%s (%d)\n", root->name, root->row);
    }
    print_tree(root->child, depth+1);
    print_tree(root->nxt, depth);   
}