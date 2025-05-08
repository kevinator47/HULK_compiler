%{
#include <stdio.h>
#include <stdlib.h>
#include "ast.h"

extern int yylex();
void yyerror(const char *s);

ASTNode *root;
%}

%union {
    double dval;
    int ival;
    char* sval;
    TokenType token;
    struct ASTNode* node;
    struct {
        ASTNode** nodes;
        int count;
    } node_list;
}

%token <dval> NUMBER
%token <ival> BOOLEAN
%token <sval> STRING
%token <token> COMP
%token ADD SUB MUL DIV MOD POW
%token CONCAT DCONCAT
%token AND OR NOT
%token LPAREN RPAREN LBRACKET RBRACKET SEMICOLON

%type <node> Statement Expression BlockExpr
%type <node_list> StatementList
%type <node> OrExpr 
%type <node> AndExpr
%type <node> CompExpr
%type <node> AddExpr 
%type <node> MultExpr 
%type <node> PowExpr 
%type <node> T

%left OR
%left AND
%left COMP
%left ADD SUB
%left MUL DIV MOD
%right POW
%right NOT

%%

Program     : Statement OptionalEnd     { root = $1; }
            ;

OptionalEnd : SEMICOLON     
            | /* empty */
            ;

Statement   : Expression                { $$ = $1; }
            ;

Expression  :  OrExpr                    { $$ = $1; }
            ;

BlockExpr   : LBRACKET StatementList RBRACKET 
            {
                $$ = make_expression_block_node($2.nodes, $2.count);
                free($2.nodes);
            }

StatementList : Statement SEMICOLON 
            {
                ASTNode** nodes = malloc(sizeof(ASTNode*));
                nodes[0] = $1;
                $$.nodes = nodes;
                $$.count = 1;
            }
            | StatementList Statement SEMICOLON
            {
                int new_count = $1.count + 1;
                ASTNode** nodes = realloc($1.nodes, new_count * sizeof(ASTNode*));
                nodes[new_count - 1] = $2;
                $$.nodes = nodes;
                $$.count = new_count;
            }
            ;


OrExpr      : OrExpr OR AndExpr         { $$ = make_binary_op_literal_node($1, $3, OR_TK); }
            | AndExpr                   { $$ = $1; }
            ;

AndExpr     : AndExpr AND CompExpr      { $$ = make_binary_op_literal_node($1, $3, AND_TK); }
            | CompExpr                  { $$ = $1; }
            ;

CompExpr    : CompExpr COMP AddExpr     { $$ = make_binary_op_literal_node($1, $3, $2); }
            | AddExpr                   { $$ = $1; }
            ;

AddExpr     : AddExpr ADD MultExpr      { $$ = make_binary_op_literal_node($1, $3, PLUS_TK); }
            | AddExpr SUB MultExpr      { $$ = make_binary_op_literal_node($1, $3, MINUS_TK); }
            | AddExpr CONCAT MultExpr   { $$ = make_binary_op_literal_node($1, $3, CONCAT_TK); }
            | AddExpr DCONCAT MultExpr  { $$ = make_binary_op_literal_node($1, $3, D_CONCAT_TK); }
            | MultExpr                  { $$ = $1; }
            ;

MultExpr    : MultExpr MUL PowExpr      { $$ = make_binary_op_literal_node($1, $3, MULT_TK); }
            | MultExpr DIV PowExpr      { $$ = make_binary_op_literal_node($1, $3, DIV_TK); }
            | MultExpr MOD PowExpr      { $$ = make_binary_op_literal_node($1, $3, MOD_TK); }
            | PowExpr                   { $$ = $1; }
            ;

PowExpr     : T POW PowExpr             { $$ = make_binary_op_literal_node($1, $3, EXP_TK); }
            | T                         { $$ = $1; }
            ;

T           : NUMBER                    { $$ = make_number_literal_node($1); }
            | BOOLEAN                   { $$ = make_boolean_literal_node($1); }
            | STRING                    { $$ = make_string_literal_node($1); }
            | LPAREN Expression RPAREN  { $$ = $2; }
            | BlockExpr                 { $$ = $1; }
            | NOT T                     { $$ = make_unary_op_literal_node($2, NOT_TK); }
            | SUB T                     { $$ = make_unary_op_literal_node($2, MINUS_TK); }
            ;

%%

void yyerror(const char *s) {
    fprintf(stderr, "Error: %s\n", s);
}

int main() {
    yyparse();
    if (root != NULL) {
        printf("AST:\n");
        print_ast(root, 0);
    }
    return 0;
}