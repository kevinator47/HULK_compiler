%{
#include "ast.h"
#include <stdio.h>
extern ASTNode* root;
int yyerror(const char* msg); 
extern int yylex(void);  // Declaración de yylex
%}

%union {
    double double_val;
    char* string_val;
    ASTNode* node;
}

%token <double_val> NUMBER
%token <string_val> VARIABLE
%token ADD SUB MUL DIV LPAREN RPAREN

%type <node> expr term factor

%%

program:
    expr { root = $1; }
;

expr:
    term                { $$ = $1; }
    | expr ADD term     { $$ = create_binop($1, '+', $3); }
    | expr SUB term    { $$ = create_binop($1, '-', $3); }
;

term:
    factor              { $$ = $1; }
    | term MUL factor  { $$ = create_binop($1, '*', $3); }
    | term DIV factor   { $$ = create_binop($1, '/', $3); }
;

factor:
    NUMBER              { $$ = create_number($1); }
    | VARIABLE          { $$ = create_variable($1); }
    | LPAREN expr RPAREN { $$ = $2; }
;

%%

ASTNode* root = NULL;

int yyerror(const char* msg) {
    fprintf(stderr, "Error de sintaxis: %s\n", msg);
    return 0;
}

int main() {
    yyparse();
    if (root) {
        printf("AST generado:\n");
        print_ast(root, 0);
    }
    return 0;
}