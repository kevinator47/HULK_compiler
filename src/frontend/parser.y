%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "entities/ast.h"
#include "entities/symbol_table.h"
#include "entities/function_table.h"

extern int yylex();
void yyerror(const char *s);

ASTNode *root;
SymbolTable* current_scope;
FunctionTable* function_table;
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
    
    struct {
        char* name;
        ASTNode* value;
        struct SymbolTable* scope; 
    } var_decl;

    struct {
        ASTNode** conditions;
        ASTNode** branches;
        int count;
    } elif_list;

    struct {
        char* name;
        char** parameters;
        int param_count;
    } function_header;

    struct {
        char** names;
        int count;
    } param_list;
}

%token <dval> NUMBER
%token <ival> BOOLEAN
%token <sval> STRING ID
%token <token> COMP
%token ADD SUB MUL DIV MOD POW
%token CONCAT DCONCAT
%token AND OR NOT
%token LPAREN RPAREN LBRACKET RBRACKET COMMA SEMICOLON 
%token LET IN ASSIGN
%token IF ELIF ELSE
%token FUNCTION ARROW

%type <node> Statement Expression BlockExpr
%type <node_list> StatementList
%type <elif_list> ElifList
%type <node> LetExpr ConditionalExpr
%type <var_decl> VarDecl
%type <var_decl> VarDeclList
%type <function_header> FunctionHeader
%type <node> FunctionBody
%type <param_list> ParameterList
%type <sval> Parameter
%type <node_list> Arguments
%type <node_list> ArgumentList
%type <node_list> IDRest
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

Program     : FunctionList Statement OptionalEnd     { root = $2; }
            ;

FunctionList: /* empty */
            | FunctionList FunctionDecl
            ;

FunctionDecl: FunctionHeader FunctionBody
            {
                SymbolTable* func_scope = create_symbol_table(100, current_scope);
                register_function(function_table, $1.name, $1.parameters, $1.param_count, $2, func_scope);
                for(int i = 0; i < $1.param_count; i++) free($1.parameters[i]);
                free($1.parameters);
            }
            ;

FunctionHeader: FUNCTION ID LPAREN ParameterList RPAREN
            {
                $$.name = $2;
                $$.parameters = $4.names;
                $$.param_count = $4.count;
            }
            ;

FunctionBody: ARROW Expression SEMICOLON         { $$ = $2; }
            | BlockExpr OptionalEnd              { $$ = $1; }
            ;

ParameterList: /* empty */
            {
                $$.names = NULL;
                $$.count = 0;
            }
            | Parameter
            {
                $$.names = malloc(sizeof(char*));
                $$.names[0] = $1;
                $$.count = 1;
            }
            | ParameterList COMMA Parameter
            {
                int new_count = $1.count + 1;
                $$.names = realloc($1.names, new_count * sizeof(char*));
                $$.names[new_count - 1] = $3;
                $$.count = new_count;
            }
            ;

Parameter   : ID                        { $$ = strdup($1); }
            ;

OptionalEnd : SEMICOLON     
            | /* empty */
            ;

Statement   : Expression                { $$ = $1; }
            ;

Expression  :  OrExpr                    { $$ = $1; }
            |  LetExpr                   { $$ = $1; }
            |  ConditionalExpr           { $$ = $1; }
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

LetExpr    : LET VarDeclList IN Expression 
           { 
               $$ = make_let_node($2.scope, $4);
               current_scope = $2.scope->parent;
           }
           ;

VarDeclList : VarDecl  
            {
               SymbolTable* scope = create_symbol_table(100, current_scope);
               insert_symbol(scope, $1.name, $1.value);
               current_scope = scope;
               $$.scope = scope;
            }
            | VarDeclList COMMA VarDecl
            {
               insert_symbol($1.scope, $3.name, $3.value);
               $$.scope = $1.scope;
            }
            ;

VarDecl    : ID ASSIGN Expression          
           { 
               $$.name = $1; 
               $$.value = $3; 
           }
           ;

ConditionalExpr 
            : IF LPAREN Expression RPAREN Expression ElifList ELSE Expression
            {
                $$ = make_conditional_node($3, $5, $6.conditions, $6.branches, $6.count, $8);
            }
            ;

ElifList    : /* empty */
            {
                $$.conditions = NULL;
                $$.branches = NULL;
                $$.count = 0;
            }
            | ElifList ELIF LPAREN Expression RPAREN Expression
            {
                int new_count = $1.count + 1;
                $$.conditions = realloc($1.conditions, new_count * sizeof(ASTNode*));
                $$.branches = realloc($1.branches, new_count * sizeof(ASTNode*));
                $$.conditions[new_count - 1] = $4;
                $$.branches[new_count - 1] = $6;
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
            | ID IDRest                 { 
                                            if ($2.count == -1) {
                                            // Es una variable normal
                                                $$ = make_variable_node($1, current_scope);
                                            } else {
                                                // Es una llamada a función
                                                $$ = make_function_call_node($1, $2.nodes, $2.count);
                                                free($2.nodes);
                                            }
                                        }
            ;

IDRest      : LPAREN Arguments RPAREN  { $$ = $2; }
            | /* empty */              { $$.count = -1; $$.nodes = NULL; }
            ;

Arguments   : /* empty */              { $$.nodes = NULL; $$.count = 0; }
            | ArgumentList             { $$ = $1; }
            ;

ArgumentList: Expression               {
                $$.nodes = malloc(sizeof(ASTNode*));
                $$.nodes[0] = $1;
                $$.count = 1;
            }
            | ArgumentList COMMA Expression {
                int new_count = $1.count + 1;
                $$.nodes = realloc($1.nodes, new_count * sizeof(ASTNode*));
                $$.nodes[new_count - 1] = $3;
                $$.count = new_count;
            }
            ;

%%

void yyerror(const char *s) {
    fprintf(stderr, "Error: %s\n", s);
}

int main() {
    current_scope = create_symbol_table(100, NULL);
    function_table = create_function_table(100);
    yyparse();
    if (root != NULL) {
        printf("AST:\n");
        print_ast(root, 0);
        free_ast(root);
    }
    free_symbol_table(current_scope);
    free_function_table(function_table);
    return 0;
}