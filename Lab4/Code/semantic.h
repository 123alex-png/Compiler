#ifndef __SEMANTIC_H_
#define __SEMANTIC_H__

#include "symbol_table.h"
#include "SyntaxTree.h"
#include <stdio.h>

/***************For dfs***********/
void Program(Node *root);
void ExtDefList(Node *root);
void ExtDef(Node *root);
void ExtDecList(Node *root, Type type);
Type Specifier(Node *root);
Type StructSpecifier(Node *root);
char* OptTag(Node *root);
char* Tag(Node *root);

FieldList VarDec(Node *root, Type type, FieldList field); 
void FunDec(Node *root, Type type, int is_def);   
void VarList(Node *root, FieldList field); 
FieldList ParamDec(Node *root);

void CompSt(Node *root, Type type);  
void StmtList(Node *root, Type type); 
void Stmt(Node *root, Type type);   

void DefList(Node *root, FieldList field);  
void Def(Node *root, FieldList field);
void DecList(Node *root, Type type, FieldList field);
void Dec(Node *root, Type type, FieldList field);

Type Exp(Node *root);
FieldList Args(Node *root);
/*********************************/

void semantic_error(int type, int row, const char *name,  const char *name1);
char *type2name(FieldList args);

#endif