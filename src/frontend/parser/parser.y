%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../ast/ast.h"
#include "../common/common.h"
#include "../hulk_type/type_table.h"

extern int yylex();
void yyerror(const char *s);

ASTNode *root_node;
TypeTable *type_table;

char* strtolower(char* str) {
    char* result = strdup(str);
    for (int i = 0; result[i]; i++) {
        result[i] = tolower(result[i]);
    }
    return result;
}

%}

%union {
    double dval;
    int ival;
    char* sval;
    struct ASTNode* node;
    VariableAssigment var_assig;
    HULK_Op token;
    struct { struct ASTNode** nodes;  int count;} node_list;
    struct { struct VariableAssigment* list; int count; } var_assig_list;
    struct { char* name; char* type;} param_info;
    struct { char** names ; char** types; int count;} param_list_info;
    struct { char* name; char** param_names; char** param_types ; int param_count; char* return_type;} function_header;
    
}

%token <dval> NUMBER
%token <ival> BOOLEAN
%token <sval> STRING ID 
%token <token> COMP
%token ADD SUB MUL DIV MOD POW
%token CONCAT DCONCAT
%token AND OR NOT
%token SEMICOLON COLON COMMA LPAREN RPAREN LBRACKET RBRACKET
%token IF ELIF ELSE WHILE LET IN
%token ASSIGN REASSIGN FUNCTION ARROW

%type<node> Expression OptionalExpression ExprBlock OrExpr AndExpr CompExpr AddExpr MultExpr PowExpr T
%type<node> IfExpr Else_Elif_Branch WhileLoopExpr LetInExpr FunctionDefList FunctionDefinition FunctionBody
%type<node_list> StatementList OptionalStatementList ArgList OptionalArgList
%type<var_assig> VariableAssigment
%type<var_assig_list> VariableAssigmentList
%type<function_header> FunctionHeader
%type<sval> OptionalType
%type<param_info> Parameter
%type<param_list_info> ParameterList

%left OR
%left AND
%left COMP
%left ADD SUB
%left MUL DIV MOD
%right POW
%right NOT
%nonassoc IFX
%nonassoc ELSE


%%
Program                 : FunctionDefList OptionalExpression OptionalEnd { root_node = create_program_node((FunctionDefinitionListNode*)$1, $2, type_table) ; }
                        ;

FunctionDefList         : FunctionDefinition FunctionDefList     
                        { $$ = append_function_definition_to_list(
                          (FunctionDefinitionListNode*)$2, 
                          (FunctionDefinitionNode*)$1); 
                        } 
                        | /* vacío */                            { $$ = create_function_definition_list_node(type_table) ; } 
                        ;

FunctionDefinition      : FunctionHeader FunctionBody 
                        { $$ = create_function_definition_node(
                          $1.name,
                          $1.param_names,
                          $1.param_types,
                          $1.param_count,
                          $1.return_type,
                          $2,
                          type_table);
                        }
                        ;

FunctionHeader          : FUNCTION ID LPAREN ParameterList RPAREN OptionalType 
                        {
                            $$.name = $2;
                            $$.param_names = $4.names;
                            $$.param_types = $4.types;
                            $$.param_count = $4.count;
                            $$.return_type = $6;
                        }
                        ;

OptionalType      : /* empty */                             {$$ = "undefined";}
                        | COLON ID                          {$$ = strtolower($2);}
                        ;

ParameterList           : /* empty */ 
                        {
                            $$.names = NULL;
                            $$.types = NULL;
                            $$.count = 0;
                        }
                        | Parameter
                        {
                            $$.names = malloc(sizeof(char*));
                            $$.types = malloc(sizeof(char*));
                            $$.names[0] = $1.name;
                            $$.types[0] = $1.type;
                            $$.count = 1;
                        }
                        | ParameterList COMMA Parameter 
                        {
                            int new_count = $1.count + 1;
                            $$.names = realloc($1.names, new_count * sizeof(char*));
                            $$.types = realloc($1.types, new_count * sizeof(char*));
                            $$.names[new_count - 1] = $3.name;
                            $$.types[new_count - 1] = $3.type;
                            $$.count = new_count;
                        }
                        ;

Parameter               : ID OptionalType                    { $$.name = $1;   $$.type = $2; }
                        ;

FunctionBody            : ARROW Expression SEMICOLON         { $$ = $2;}
                        | ExprBlock OptionalEnd              { $$ = $1; }
                        ;

OptionalEnd             : SEMICOLON
                        | /* empty */
                        ;

Expression              : OrExpr  {$$ = $1;}
                        | IfExpr  {$$ = $1;}
                        | WhileLoopExpr {$$ = $1;}
                        | LetInExpr {$$ = $1;}
                        ;

OptionalExpression      : Expression
                        | /* empty */   {$$ = NULL;}
                        ;

IfExpr                  : IF LPAREN Expression RPAREN Expression  Else_Elif_Branch %prec IFX
                        {
                            $$ = create_conditional_node($3, $5, $6, type_table);
                        }
                        ;

Else_Elif_Branch        : /* empty */ 
                        {
                            $$ = NULL; // No else or elif branches
                        }
                        | ELIF LPAREN Expression RPAREN Expression Else_Elif_Branch
                        {
                            $$ = create_conditional_node($3, $5, $6, type_table);
                        }
                        | ELSE Expression
                        {
                            $$ = $2;
                        }
                        ;
                    
WhileLoopExpr           : WHILE LPAREN Expression RPAREN Expression
                        {
                            $$ = create_while_loop_node($3, $5, type_table);
                        }
                        ;

LetInExpr               : LET VariableAssigmentList IN Expression
                        {
                            $$ = create_let_in_node($2.list, $2.count, $4, type_table);
                        }
                        ;

VariableAssigmentList   : VariableAssigment
                        {
                            VariableAssigment* list = malloc(sizeof(VariableAssigment));
                            list[0] = $1;
                            $$.list = list;
                            $$.count = 1;
                        }
                        | VariableAssigmentList COMMA VariableAssigment
                        {
                            int new_count = $1.count + 1;
                            VariableAssigment* list = realloc($1.list, new_count * sizeof(VariableAssigment));
                            list[new_count - 1] = $3;
                            $$.list = list;
                            $$.count = new_count;
                        }
                        ;   

VariableAssigment       : ID ASSIGN Expression  { $$.name = $1; $$.value = $3; }
                        ;


OrExpr                  : OrExpr OR AndExpr         { $$ = create_binary_operation_node(OR_TK, $1, $3, type_table); }
                        | AndExpr                   { $$ = $1; }
                        ;

AndExpr                 : AndExpr AND CompExpr      { $$ = create_binary_operation_node(AND_TK, $1, $3, type_table); }
                        | CompExpr                  { $$ = $1; }
                        ;

CompExpr                : CompExpr COMP AddExpr     { $$ = create_binary_operation_node($2, $1, $3, type_table); }
                        | AddExpr                   { $$ = $1; }
                        ;

AddExpr                 : AddExpr ADD MultExpr      { $$ = create_binary_operation_node(PLUS_TK, $1, $3, type_table ); }
                        | AddExpr SUB MultExpr      { $$ = create_binary_operation_node(MINUS_TK, $1, $3, type_table); }
                        | AddExpr CONCAT MultExpr   { $$ = create_binary_operation_node(CONCAT_TK, $1, $3, type_table); }
                        | AddExpr DCONCAT MultExpr  { $$ = create_binary_operation_node(D_CONCAT_TK, $1, $3, type_table); }
                        | MultExpr                  { $$ = $1; }
                        ;

MultExpr                : MultExpr MUL PowExpr      { $$ = create_binary_operation_node(MULT_TK, $1, $3, type_table); }
                        | MultExpr DIV PowExpr      { $$ = create_binary_operation_node(DIV_TK, $1, $3, type_table ); }
                        | MultExpr MOD PowExpr      { $$ = create_binary_operation_node(MOD_TK, $1, $3, type_table); }
                        | PowExpr                   { $$ = $1; }
                        ;

PowExpr                 : T POW PowExpr             { $$ = create_binary_operation_node(EXP_TK, $1, $3, type_table); }
                        | T                         { $$ = $1; }
                        ;

T                       : NUMBER                    { $$ = create_number_literal_node($1 , type_table); }
                        | BOOLEAN                   { $$ = create_bool_literal_node($1, type_table); }
                        | STRING                    { $$ = create_string_literal_node($1, type_table); }
                        | LPAREN Expression RPAREN  { $$ = $2; }
                        | NOT T                     { $$ = create_unary_operation_node(NOT_TK, $2, type_table); }
                        | SUB T                     { $$ = create_unary_operation_node(MINUS_TK, $2, type_table); }
                        | ExprBlock                 { $$ = $1; }  
                        | ID                        { $$ = create_variable_node($1, type_table); }
                        | ID REASSIGN Expression    { $$ = create_reassign_node($1, $3, type_table);}
                        | ID LPAREN ArgList RPAREN
                        { 
                            $$ = create_function_call_node($1, $3.nodes, $3.count, type_table);
                        }
                        ;

ArgList                 : OptionalArgList Expression OptionalEnd
                        {
                            int new_count = $1.count + 1;
                            ASTNode** all_nodes = realloc($1.nodes, new_count * sizeof(ASTNode*));
                            if (!all_nodes) DIE("Out of memory in ArgumentList");
                            all_nodes[new_count - 1] = $2;
                            $$.nodes = all_nodes;
                            $$.count = new_count;
                        }
                        |   /*empty*/               { $$.nodes = NULL; $$.count = 0; }
                        ;

OptionalArgList         :  /*empty*/                { $$.nodes = NULL; $$.count = 0;}
                        |  OptionalArgList Expression COMMA
                        {
                            int new_count = $1.count + 1;
                            ASTNode** nodes = realloc($1.nodes, new_count * sizeof(ASTNode*));
                            if (!nodes) DIE("Out of memory in ExpressionListOpt");
                            nodes[new_count - 1] = $2;
                            $$.nodes = nodes;
                            $$.count = new_count;
                        }
                        ;

ExprBlock               : LBRACKET StatementList RBRACKET 
                        {
                            $$ = create_expression_block_node($2.nodes, $2.count, type_table);
                            free($2.nodes);
                        }

StatementList           : OptionalStatementList Expression OptionalEnd
                        {
                            int new_count = $1.count + 1;
                            ASTNode** all_nodes = realloc($1.nodes, new_count * sizeof(ASTNode*));
                            if (!all_nodes) DIE("Out of memory in StatementList");
                            all_nodes[new_count - 1] = $2;
                            $$.nodes = all_nodes;
                            $$.count = new_count;
                        }
                        |   /*empty*/               { $$.nodes = NULL; $$.count = 0; }
                        ;

OptionalStatementList   : /* empty */               { $$.nodes = NULL; $$.count = 0; }
                        | OptionalStatementList Expression SEMICOLON
                        {
                            int new_count = $1.count + 1;
                            ASTNode** nodes = realloc($1.nodes, new_count * sizeof(ASTNode*));
                            if (!nodes) DIE("Out of memory in ExpressionListOpt");
                            nodes[new_count - 1] = $2;
                            $$.nodes = nodes;
                            $$.count = new_count;
                        }
                        ;
%%

void yyerror(const char *s) {
    fprintf(stderr, "Error: %s\n", s);
}

