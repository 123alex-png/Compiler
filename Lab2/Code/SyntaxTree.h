#ifndef __TREE_H__
#define __TREE_H__

#include <string.h>
enum TYPE{
    TYPE_DEC = 1,
    TYPE_HEX,
    TYPE_OCT,
    TYPE_FLOAT,
    TYPE_CONTENT,
    TYPE_OTHER
};

typedef struct Node{
    char name[32];
    union{
        unsigned type_int;
        float type_float;
        char type_content[32];
    };
    struct Node *child;
    struct Node *nxt;
    int right_num;//产生式右部的数目
    int row;
    enum TYPE type;
    int is_token;
}Node;

Node *get_node(const char *name, const char *val, enum TYPE type, int row);
Node *new_node(const char *name, int row, int num, ...);
void print_tree(Node *root, int depth);

#define MAX_CHILD_NUM 10
typedef struct {
    int num;
    Node *childs[MAX_CHILD_NUM];   
}child_node;

child_node *get_childs(Node *parent);
Node *find_child(Node *parent, const char *name);
#endif