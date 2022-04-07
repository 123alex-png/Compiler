#ifndef __TREE_H__
#define __TREE_H__

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
    int row;
    enum TYPE type;
    int is_token;
}Node;

Node *get_node(const char *name, const char *val, enum TYPE type, int row);
Node *new_node(const char *name, int row, int num, ...);
void print_tree(Node *root, int depth);
#endif