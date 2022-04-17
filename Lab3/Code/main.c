#include <stdio.h>
#include "syntax.tab.h"
#include "SyntaxTree.h"
#include "semantic.h"
extern FILE *yyin;
extern Node *root;
int yylex();
int yyrestart();
extern int lex_error_num, syntax_error_num;
// int main(int argc, char **argv){
//     if(argc > 1){
//         if(!(yyin = fopen(argv[1], "r"))){
//             perror(argv[1]);
//             return 1;
//         }
//     }
//     while(yylex() != 0);
//     return 0;
// }
int main(int argc, char** argv){
 if (argc <= 1) return 1;
 FILE* f = fopen(argv[1], "r");
 if (!f){
    perror(argv[1]);
    return 1;
 }
 yyrestart(f);
 yyparse();
 if(lex_error_num == 0 && syntax_error_num == 0){
   //  print_tree(root, 0);
   Program(root);
 }
 return 0;
}
