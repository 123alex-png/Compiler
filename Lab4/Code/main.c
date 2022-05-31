#include <stdio.h>
#include "syntax.tab.h"
#include "SyntaxTree.h"
#include "semantic.h"
#include "intercode.h"
#include "translator.h"
#include "objcode.h"
extern FILE *yyin;
extern Node *root;
int yylex();
int yyrestart();
extern int lex_error_num, syntax_error_num;
extern int semantic_error_num;
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
    if(semantic_error_num == 0){
      tr_Program(root);
      if(argc == 2)
        print_inter_code(stdout);
      else{

        FILE *fp = fopen("output.ir", "w+");
        print_inter_code(fp);
      }
      if(argc == 2){
        gen_objcode(stdout);
      }
      else{
        FILE *fp = fopen(argv[2], "w+");
        gen_objcode(fp);
      }
    }
  }
  return 0;
}