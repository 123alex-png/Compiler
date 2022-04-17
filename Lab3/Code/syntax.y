%{
    #include <stdio.h>
    #include "lex.yy.c"
    #include "SyntaxTree.h"
    void yyerror(const char *msg);
    int syntax_error_num = 0;
    Node *root;
%}
%locations
%define parse.error verbose
%union{
    struct Node *node;
}

%token  <node> INT 
%token  <node> FLOAT  
%token  <node> ID;
%token  <node> TYPE;
%token  <node> SEMI COMMA ASSIGNOP RELOP PLUS MINUS STAR DIV AND OR DOT NOT LP RP LB RB LC RC
%token  <node> STRUCT RETURN IF ELSE WHILE


%type   <node> Program
%type   <node> ExtDefList
%type   <node> ExtDef
%type   <node> ExtDecList

%type   <node> Specifier
%type   <node> StructSpecifier
%type   <node> OptTag
%type   <node> Tag

%type   <node> VarDec
%type   <node> FunDec
%type   <node> VarList
%type   <node> ParamDec


%type   <node> CompSt
%type   <node> StmtList
%type   <node> Stmt

%type   <node> DefList
%type   <node> Def
%type   <node> DecList
%type   <node> Dec

%type   <node> Exp
%type   <node> Args



%right  ASSIGNOP
%left   OR 
%left   AND
%left   RELOP 
%left   PLUS MINUS
%left   STAR DIV
%left   UMINUS
%right  NOT
%left   DOT LP RP LB RB

%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

%%
//7.1.2
Program: ExtDefList                         {$$ = new_node("Program", @$.first_line, 1, $1); root = $$;};

ExtDefList: ExtDef ExtDefList               {$$ = new_node("ExtDefList", @$.first_line, 2, $1, $2);}
            | /*Empty*/                     {$$ = NULL;};

ExtDef: Specifier ExtDecList SEMI           {$$ = new_node("ExtDef", @$.first_line, 3, $1, $2, $3);}
        | Specifier SEMI                    {$$ = new_node("ExtDef", @$.first_line, 2, $1, $2);}
        | Specifier FunDec CompSt           {$$ = new_node("ExtDef", @$.first_line, 3, $1, $2, $3);}
        | Specifier FunDec SEMI             {$$ = new_node("ExtDef", @$.first_line, 3, $1, $2, $3);}//要求2.1新增
        | error CompSt                      {yyerrok;}
        | error SEMI                        {yyerrok;}
;
ExtDecList: VarDec                          {$$ = new_node("ExtDecList", @$.first_line, 1, $1);}
            | VarDec COMMA ExtDecList       {$$ = new_node("ExtDecList", @$.first_line, 3, $1, $2, $3);}
;
    
//7.1.3
Specifier: TYPE                             {$$ = new_node("Specifier", @$.first_line, 1, $1);}
           | StructSpecifier                {$$ = new_node("Specifier", @$.first_line, 1, $1);}
;

StructSpecifier: STRUCT OptTag LC DefList RC{$$ = new_node("StructSpecifier", @$.first_line, 5, $1, $2, $3, $4, $5);}
                | STRUCT Tag                {$$ = new_node("StructSpecifier", @$.first_line, 2, $1, $2);};
                | STRUCT OptTag LC error RC {yyerrok;}
;

OptTag: ID                                  {$$ = new_node("OptTag", @$.first_line, 1, $1);}
        | /*Empty*/                         {$$ = NULL;}
;

Tag: ID                                     {$$ = new_node("Tag", @$.first_line, 1, $1);}
;

//7.1.4
VarDec: ID                                  {$$ = new_node("VarDec", @$.first_line, 1, $1);}
        | VarDec LB INT RB                  {$$ = new_node("VarDec", @$.first_line, 4, $1, $2, $3, $4);}
;

FunDec: ID LP VarList RP                    {$$ = new_node("FunDec", @$.first_line, 4, $1, $2, $3, $4);}
        | ID LP RP                          {$$ = new_node("FunDec", @$.first_line, 3, $1, $2, $3);};

VarList: ParamDec COMMA VarList             {$$ = new_node("VarList", @$.first_line, 3, $1, $2, $3);}
        | ParamDec                          {$$ = new_node("VarList", @$.first_line, 1, $1);};


ParamDec: Specifier VarDec                  {$$ = new_node("ParamDec", @$.first_line, 2, $1, $2);}
;

//7.1.5
CompSt: LC DefList StmtList RC              {$$ = new_node("CompSt", @$.first_line, 4, $1, $2, $3, $4);};

StmtList: Stmt StmtList                     {$$ = new_node("StmtList", @$.first_line, 2, $1, $2);}
        | /*Empty*/                         {$$ = NULL;};


Stmt: Exp SEMI                              {$$ = new_node("Stmt", @$.first_line, 2, $1, $2);}
    | CompSt                                {$$ = new_node("Stmt", @$.first_line, 1, $1);}
    | RETURN Exp SEMI                       {$$ = new_node("Stmt", @$.first_line, 3, $1, $2, $3);}
    | IF LP Exp RP Stmt %prec LOWER_THAN_ELSE {$$ = new_node("Stmt", @$.first_line, 5, $1, $2, $3, $4, $5);}
    | IF LP Exp RP Stmt ELSE Stmt           {$$ = new_node("Stmt", @$.first_line, 7, $1, $2, $3, $4, $5, $6, $7);}
    | WHILE LP Exp RP Stmt                  {$$ = new_node("Stmt", @$.first_line, 5, $1, $2, $3, $4, $5);}
    | error SEMI                            {yyerrok;}
    | error Stmt                            {yyerrok;}
    | IF LP error RP Stmt %prec LOWER_THAN_ELSE {yyerrok;}
    | IF LP error RP Stmt ELSE Stmt         {yyerrok;}
    | WHILE LP error RP Stmt                {yyerrok;}
    | error RC                              {yyerrok;}
;

//7.1.6
DefList: Def DefList                        {$$ = new_node("DefList", @$.first_line, 2, $1, $2);}
        | /*Empty*/                         {$$ = NULL;};

Def: Specifier DecList SEMI                 {$$ = new_node("Def", @$.first_line, 3, $1, $2, $3);}
        | Specifier error SEMI              {yyerrok;}
;

DecList: Dec                                {$$ = new_node("DecList", @$.first_line, 1, $1);}
        | Dec COMMA DecList                 {$$ = new_node("DecList", @$.first_line, 3, $1, $2, $3);}
;

Dec: VarDec                                 {$$ = new_node("Dec", @$.first_line, 1, $1);}
    | VarDec ASSIGNOP Exp                   {$$ = new_node("Dec", @$.first_line, 3, $1, $2, $3);}
;

//7.1.7
Exp: Exp ASSIGNOP Exp                       {$$ = new_node("Exp", @$.first_line, 3, $1, $2, $3);}
    | Exp AND Exp                           {$$ = new_node("Exp", @$.first_line, 3, $1, $2, $3);}
    | Exp OR Exp                            {$$ = new_node("Exp", @$.first_line, 3, $1, $2, $3);}
    | Exp RELOP Exp                         {$$ = new_node("Exp", @$.first_line, 3, $1, $2, $3);}
    | Exp PLUS Exp                          {$$ = new_node("Exp", @$.first_line, 3, $1, $2, $3);}
    | Exp MINUS Exp                         {$$ = new_node("Exp", @$.first_line, 3, $1, $2, $3);}
    | Exp STAR Exp                          {$$ = new_node("Exp", @$.first_line, 3, $1, $2, $3);}
    | Exp DIV Exp                           {$$ = new_node("Exp", @$.first_line, 3, $1, $2, $3);}
    | LP Exp RP                             {$$ = new_node("Exp", @$.first_line, 3, $1, $2, $3);}
    | MINUS Exp %prec UMINUS                {$$ = new_node("Exp", @$.first_line, 2, $1, $2);}
    | NOT Exp                               {$$ = new_node("Exp", @$.first_line, 2, $1, $2);}
    | ID LP Args RP                         {$$ = new_node("Exp", @$.first_line, 4, $1, $2, $3, $4);}
    | ID LP RP                              {$$ = new_node("Exp", @$.first_line, 3, $1, $2, $3);}
    | Exp LB Exp RB                         {$$ = new_node("Exp", @$.first_line, 4, $1, $2, $3, $4);}
    | Exp DOT ID                            {$$ = new_node("Exp", @$.first_line, 3, $1, $2, $3);}
    | ID                                    {$$ = new_node("Exp", @$.first_line, 1, $1);}
    | INT                                   {$$ = new_node("Exp", @$.first_line, 1, $1);}
    | FLOAT                                 {$$ = new_node("Exp", @$.first_line, 1, $1);}   
//     | LP error RP                           {yyerrok;}   
//     | ID LP error RP                        {yyerrok;}  
//     | Exp LB error RB                       {yyerrok;}  

;

Args: Exp COMMA Args                        {$$ = new_node("Args", @$.first_line, 3, $1, $2, $3);}
    | Exp                                   {$$ = new_node("Args", @$.first_line, 1, $1);};
    | error COMMA Exp                       {yyerrok;}
    // | error "\n"                            {yyerrok;}


%%
void yyerror(const char *msg){
    syntax_error_num++;
    printf("Error type B at Line %d: %s, near \"%s\".\n", yylineno, msg, yytext);
}