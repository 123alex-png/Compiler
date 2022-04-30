#ifndef __SYMBOL_TABLE_H__
#define __SYMBOL_TABLE_H__

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

//完全按讲义实现
#define HASH_TABLE_SIZE 0x3fff
typedef struct Type_* Type; 
typedef struct FieldList_* FieldList; 
typedef struct SymbolNode_* SymbolNode;
typedef struct ScopeList_* ScopeList;
typedef struct FunDecList_* FunDecList;

SymbolNode hash_table[HASH_TABLE_SIZE+1];
FunDecList fun_dec_head, fun_dec_tail;

struct Type_{ 
  enum{ BASIC, ARRAY, STRUCTURE_TAG, FUNCTION, STRUCTURE} kind; 
  union{ 
    // 基本类型 
    enum{ BASIC_INT, BASIC_FLOAT} basic; 
    // 数组类型信息包括元素类型与数组大小构成 
    struct{
      Type elem;
      int size; 
    }array;
    // 结构体类型信息是一个链表 
    FieldList structure; 
    struct func{
      Type return_type;
      int argc;
      FieldList argv;//与原参数列表颠倒
      FunDecList p;//在函数定义链表中的位置，方便后续声明转定义时修改
    }function;
  }u; 
  int has_define;
}; 
 
struct FieldList_{ 
  char name[32];  // 域的名字 
  struct Type_ type;  // 域的类型  
  FieldList tail;  // 下一个域，用于讲义中的上下链表
  ScopeList scope;//属于哪个作用域
  int no;
  int is_arg;
};

struct SymbolNode_{
  FieldList val; //符号值
  unsigned int hash_val;
  SymbolNode next_node; //下一节点，用于每个槽位的链表
  SymbolNode prev_node; //上一节点，用于每个槽位的链表
  SymbolNode tail_node; //属于同一作用域的下一个槽位
};

struct ScopeList_{
  SymbolNode head;//当前作用域第一个槽位
  SymbolNode tail;//当前作用域最后一个槽位
  ScopeList next;//下一作用域
};

struct FunDecList_{
  char name[32];
  int row;
  FunDecList next;
  int has_def;
};

unsigned int hash_val(char* name);
void hash_insert(FieldList field, ScopeList cur);
FieldList find(char* name);
int check_function_define(FieldList old, FieldList new);//检查函数的声明和定义是否一致
FieldList find_member_in_structure(FieldList f, char *var_name);//查找结构体中的成员
int check_type(Type a, Type b);//检查两个变量的类型是否一致
int check_args(FieldList a, FieldList b);//检查两个函数的参数是否一致

//要求2.1新增代码
void func_insert(FieldList field, int row);
void update_row(FunDecList p, int row);
void check_fundec();

//要求2.2新增代码
void delete_node(SymbolNode node);
ScopeList create_scope();
void delete_scope();
int is_same_scope(FieldList f);
#endif