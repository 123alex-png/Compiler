#ifndef __TRANSLATOR_H_
#define __TRANSLATOR_H__

#include "symbol_table.h"
#include "SyntaxTree.h"
#include <stdio.h>
#include "intercode.h"

/***************For dfs***********/
void tr_Program(Node *root);
void tr_ExtDefList(Node *root);
void tr_ExtDef(Node *root);
// void tr_ExtDecList(Node *root, Type type);
// Type tr_Specifier(Node *root);
// Type tr_StructSpecifier(Node *root);
// char* tr_OptTag(Node *root);
// char* tr_Tag(Node *root);

Operand tr_VarDec(Node *root); 
void tr_FunDec(Node *root);   
// void tr_VarList(Node *root, FieldList field); 
// FieldList tr_ParamDec(Node *root);

void tr_CompSt(Node *root);  
void tr_StmtList(Node *root); 
void tr_Stmt(Node *root);   

void tr_Cond(Node *root, Operand true_label, Operand false_label);

void tr_DefList(Node *root);  
void tr_Def(Node *root);
void tr_DecList(Node *root);
void tr_Dec(Node *root);

Type tr_Exp(Node *root, Operand op);
FieldList tr_Args(Node *root);
/*********************************/
void array_copy(Operand dst, Operand src);

#endif