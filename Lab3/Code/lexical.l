%{
    #include "syntax.tab.h"
    #include "SyntaxTree.h"
    int yycolumn = 1;
    #define YY_USER_ACTION \
    yylloc.first_line = yylloc.last_line = yylineno; \
    yylloc.first_column = yycolumn; \
    yylloc.last_column = yycolumn + yyleng - 1; \
    yycolumn += yyleng;

    int lex_error_num = 0;    
    // #define DEBUG
    void print(char *s){
        #ifdef DEBUG
            printf("line: %d, type: %s, content: %s\n", yylineno, s, yytext);
        #endif
    }
    void print_error(char *s){
        lex_error_num++;
        printf("Error type A at line %d: %s \"%s\".\n", yylineno, s, yytext);
    }
// ERROR_DEC:
// 前两个都是数字：
//     后面有一个字母就不对
// 第一个是0，第二个是字母[a-wA-WyzYZ]：
//     必不对
// 第一个是[1-9]，第二个是字母：
//     必不对

// ERROR_OCT:
//     必须0开头，后面出现8或9

//ERROR_HEX:
//     必须0[xX]开头，后面出现[g-zG-Z_]

// ERROR_FLOAT:
//      非科学计数法：小数点前面或后面缺数字或多个小数点
//      科学计数法：基数无小数点或没有指数
%}
%option yylineno
letter_     [a-zA-Z_]
digit_letter {letter_}|[0-9]
DEC         0|[1-9][0-9]*
ERROR_DEC   [0-9][0-9]+{letter_}+[{digit_letter}*|0[a-wA-WyzYZ_]{digit_letter}*|[1-9]{letter_}{digit_letter}*
OCT         0[0-7]+
ERROR_OCT   0[0-7]*[89]+[0-9]*
HEX         0[xX][0-9a-fA-F]+
ERROR_HEX   0[xX][0-9a-fA-F]*[g-zG-Z_]+{digit_letter}*
NORMAL_FLOAT [0-9]*\.[0-9]+|[0-9]+\.
FLOAT       [0-9]+\.[0-9]+|{NORMAL_FLOAT}[eE][+-]?[0-9]+ 
ERROR_FLOAT [0-9]+\.|\.[0-9]+|[0-9]+[eE][+-]?[0-9]*|{NORMAL_FLOAT}[eE][+-]?
ID          {letter_}({letter_}|[0-9])*
COMMENT     "//"
LCOMMENT    "/*"
SEMI        ";"
COMMA       ","
ASSIGNOP    "="
RELOP       >|<|>=|<=|==|!=
PLUS        "+"
MINUS       "-"
STAR        "*"
DIV         "/"
AND         "&&"
OR          "||"
DOT         "."
NOT         "!"
TYPE        int|float
LP          "("
RP          ")"
LB          "["
RB          "]"
LC          "{"
RC          "}"
STRUCT      struct
RETURN      return
IF          "if"
ELSE        "else"
WHILE       "while"
WHITESPACE  [ \t\r\n]+
%%


{DEC}   {
    yylval.node = get_node("INT", yytext, TYPE_DEC, yylineno);
    print("DEC");
    return INT;
}

{ERROR_DEC} {
    print_error("illegal decimal number");
    return INT;
}

{OCT}   {
    yylval.node = get_node("INT", yytext, TYPE_OCT, yylineno);
    print("OCT");
    return INT;
}

{ERROR_OCT} {
    print_error("octal");
    return INT;
}

{HEX}   {
    yylval.node = get_node("INT", yytext, TYPE_HEX, yylineno);
    print("HEX");
    return INT;
}

{ERROR_HEX} {
    print_error("illegal hexadecimal number");
    return INT;
}

{FLOAT} {
    yylval.node = get_node("FLOAT", yytext, TYPE_FLOAT, yylineno);
    print("FLOAT");
    return FLOAT;
}

{ERROR_FLOAT} {
    print_error("illegal floatint point number");
    return FLOAT;
}

{COMMENT}   {
    print("COMMENT");
    char a = input();
    while(a != '\n'){
        a = input();
    }
    
}

{LCOMMENT}  {
    print("PAIRCOMMENT");
    char a = input();
    while(1){
        if(!a){
            print_error("Missing");
            lex_error_num++;
            break;
        }
        else if(a == '*'){
            a = input();
            if(a == '/'){
                #ifdef DEBUG
                    printf("line: %d, type: %s, content: %s\n", yylineno, "PAIRCOMMENT", "*/");
                #endif
                break;
            }   
        }
        else{
            a = input();
        }
    }
}

{SEMI}  {
    yylval.node = get_node("SEMI", NULL, TYPE_OTHER, yylineno);
    print("SEMI");
    return SEMI;
}

{COMMA}   {
    yylval.node = get_node("COMMA", NULL, TYPE_OTHER, yylineno);
    print("COMMA");
    return COMMA;
}

{ASSIGNOP} {
    yylval.node = get_node("ASSIGNOP", NULL, TYPE_OTHER, yylineno);
    print("ASSIGNOP");
    return ASSIGNOP;
}

{RELOP} {
    yylval.node = get_node("RELOP", yytext, TYPE_OTHER, yylineno);
    print("RELOP");
    return RELOP;
}

{PLUS}  {
    yylval.node = get_node("PLUS", NULL, TYPE_OTHER, yylineno);
    print("PLUS");
    return PLUS;
}

{MINUS} {
    yylval.node = get_node("MINUS", NULL, TYPE_OTHER, yylineno);
    print("MINUS");
    return MINUS;
}

{STAR}  {
    yylval.node = get_node("STAR", NULL, TYPE_OTHER, yylineno);
    print("STAR");
    return STAR;
}

{DIV}   {
    yylval.node = get_node("DIV", NULL, TYPE_OTHER, yylineno);
    print("DIV");
    return DIV;
}

{AND}   {
    yylval.node = get_node("AND", NULL, TYPE_OTHER, yylineno);
    print("AND");
    return AND;
}

{OR}    {
    yylval.node = get_node("OR", NULL, TYPE_OTHER, yylineno);
    print("OR");
    return OR;
}

{DOT}   {
    yylval.node = get_node("DOT", NULL, TYPE_OTHER, yylineno);
    print("DOT");
    return DOT;
}

{NOT}   {
    yylval.node = get_node("NOT", NULL, TYPE_OTHER, yylineno);
    print("NOT");
    return NOT;
}

{TYPE}  {
    yylval.node = get_node("TYPE", yytext, TYPE_CONTENT, yylineno);
    print("TYPE");
    return TYPE;
}

{LP}    {
    yylval.node = get_node("LP", NULL, TYPE_OTHER, yylineno);
    print("LP");
    return LP;
}

{RP}    {
    yylval.node = get_node("RP", NULL, TYPE_OTHER, yylineno);
    print("RP");
    return RP;
}

{LB}    {
    yylval.node = get_node("LB", NULL, TYPE_OTHER, yylineno);
    print("LB");
    return LB;
}

{RB}    {
    yylval.node = get_node("RB", NULL, TYPE_OTHER, yylineno);
    print("RB");
    return RB;
}

{LC}    {
    yylval.node = get_node("LC", NULL, TYPE_OTHER, yylineno);
    print("LC");
    return LC;
}

{RC}    {
    yylval.node = get_node("RC", NULL, TYPE_OTHER, yylineno);
    print("RC");
    return RC;
}

{STRUCT}    {
    yylval.node = get_node("STRUCT", NULL, TYPE_OTHER, yylineno);
    print("STRUCT");
    return STRUCT;
}

{RETURN}    {
    yylval.node = get_node("RETURN", NULL, TYPE_OTHER, yylineno);
    print("RETURN");
    return RETURN;
}

{IF}    {
    yylval.node = get_node("IF", NULL, TYPE_OTHER, yylineno);
    print("IF");
    return IF;
}

{ELSE}  {
    yylval.node = get_node("ELSE", NULL, TYPE_OTHER, yylineno);
    print("ELSE");
    return ELSE;
}

{WHILE} {
    yylval.node = get_node("WHILE", NULL, TYPE_OTHER, yylineno);
    print("WHILE");
    return WHILE;
}

{ID}    {
    yylval.node = get_node("ID", yytext, TYPE_CONTENT, yylineno);
    print("ID");
    return ID;
}

"\n"    {
    yycolumn = 1;
}

{WHITESPACE}    {

}

.   {print_error("Mysterious characters");}

%%

