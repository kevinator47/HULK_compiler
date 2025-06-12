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


%}

%union {
    double dval;
    int ival;
    char* sval;
    HULK_Op token;
    struct ASTNode* node;
    struct { struct ASTNode** nodes;  int count;} node_list;
    struct VariableAssigment* var_assig;
    struct { struct VariableAssigment** list; int count; } var_assig_list;
    struct { char* name; char* type;} param_info;
    struct { char** names ; char** types; int count;} param_list_info;
    struct { char* name; char** param_names; char** param_types; int param_count; char* return_type;} function_header;
    struct { char* name; char** param_names; char** param_types; int param_count;  
             char* parent_name; struct ASTNode** parent_args; int parent_args_count;} type_def_header;
    struct { char* name; struct ASTNode** nodes; int count ;} inherit_info;
    struct { TypeDefinitionNode** nodes; int count;} type_def_list;
    
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
%token ASSIGN REASSIGN FUNCTION ARROW TYPE INHERITS

%type<node> Expression ExprBlock OrExpr AndExpr CompExpr AddExpr MultExpr PowExpr T WhileLoopExpr LetInExpr
%type<node> IfExpr Else_Elif_Branch FunctionDefList FunctionDefinition FunctionBody TypeDefinition TypeDefinitionBody TypeDefExpr
%type<node_list> ExpressionList OptionalExpressionList ArgList OptionalArgList OptionalParentArgs TypeExprList 
%type<type_def_list> TypeDefinitionList
%type<var_assig> VariableAssigment
%type<var_assig_list> VariableAssigmentList
%type<function_header> FunctionHeader
%type<sval> OptionalType 
%type<inherit_info>OptionalInherits
%type<param_info> Parameter
%type<param_list_info> ParameterList OptionalTypeParams
%type<type_def_header> TypeDefinitionHeader

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
Program                 : FunctionDefList TypeDefinitionList Expression OptionalEnd 
                        { 
                            root_node = create_program_node(
                                $1, 
                                create_type_definition_list_node($2.nodes, $2.count, type_table),
                                $3, 
                                type_table); 
                        }
                        ;

FunctionDefList         : FUNCTION FunctionDefinition FunctionDefList     
                        { 
                          $$ = append_function_definition_to_list( (FunctionDefinitionListNode*)$3 , (FunctionDefinitionNode*)$2 ); 
                        } 
                        | /* vacío */   { $$ = create_function_definition_list_node(type_table) ; } 
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

FunctionHeader          :  ID LPAREN ParameterList RPAREN OptionalType 
                        {
                           $$.name = $1;
                           $$.param_names = $3.names;
                           $$.param_types = $3.types;
                           $$.param_count = $3.count;
                           $$.return_type = $5;
                        }
                        ;

OptionalType            : /* empty */                       {$$ = strdup("Undefined");}
                        | COLON ID                          {$$ = $2;}
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

TypeDefinitionList      : /* empty */
                        {
                            $$.nodes = NULL;
                            $$.count = 0;
                        }
                        | TypeDefinition TypeDefinitionList
                        {
                            int new_count = $2.count + 1;
                            TypeDefinitionNode** nodes = realloc($2.nodes, new_count * sizeof(TypeDefinitionNode*));
                            if (!nodes) DIE("Out of memory in TypeDefinitionList");

                            nodes[new_count - 1] = (TypeDefinitionNode*)$1;
                            $$.nodes = nodes;
                            $$.count = new_count;
                        }
                        ;

TypeDefinition          : TypeDefinitionHeader TypeDefinitionBody
                        {
                            $$ = create_type_definition_node($1.name, $1.param_names, $1.param_types, $1.param_count, $1.parent_name, 
                            $1.parent_args, $1.parent_args_count, $2, type_table);
                        }
                        ;

TypeDefinitionHeader    : TYPE ID OptionalTypeParams OptionalInherits 
                        {
                          $$.name = $2;
                          $$.param_names = $3.names;
                          $$.param_types = $3.types;
                          $$.param_count = $3.count;
                          $$.parent_name = $4.name;
                          $$.parent_args = $4.nodes;
                          $$.parent_args_count = $4.count;
                        }
                        ;

OptionalTypeParams      : /*empty*/                     {$$.names = NULL; $$.types = NULL; $$.count = 0; }
                        | LPAREN ParameterList RPAREN   {$$ = $2;}
                        ;

OptionalInherits        : INHERITS ID OptionalParentArgs {$$.name = $2; $$.nodes = $3.nodes; $$.count = $3.count;}
                        | /*empty*/                      {$$.name = "Object"; $$.nodes = NULL; $$.count = 0;}

OptionalParentArgs      : /*empty*/                      {$$.nodes = NULL; $$.count = 0;}
                        | LPAREN ArgList RPAREN          {$$ = $2;}


TypeDefinitionBody      : LBRACKET TypeExprList RBRACKET
                        {
                            $$ = create_expression_block_node($2.nodes, $2.count, type_table);
                        }

TypeExprList            : /* empty */                    { $$.nodes = NULL; $$.count = 0; }
                        | TypeExprList TypeDefExpr 
                        {
                            int new_count = $1.count + 1;
                            ASTNode** nodes = realloc($1.nodes, new_count * sizeof(ASTNode*));
                            if (!nodes) DIE("Out of memory in TypeExprList");
                            nodes[new_count - 1] = $2;
                            $$.nodes = nodes;
                            $$.count = new_count;
                        }
                        ;

TypeDefExpr             : FunctionDefinition              {$$ = $1;}
                        | VariableAssigment SEMICOLON     {$$ = create_variable_assigment_node($1 , type_table);}
                        ;

Expression              : OrExpr  {$$ = $1;}
                        | IfExpr  {$$ = $1;}
                        | WhileLoopExpr {$$ = $1;}
                        | LetInExpr {$$ = $1;}
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
                            VariableAssigment** list = malloc(sizeof(VariableAssigment*));
                            list[0] = $1;
                            $$.list = list;
                            $$.count = 1;
                        }
                        | VariableAssigmentList COMMA VariableAssigment
                        {
                            int new_count = $1.count + 1;
                            VariableAssigment** list = realloc($1.list, new_count * sizeof(VariableAssigment*));
                            list[new_count - 1] = $3;
                            $$.list = list;
                            $$.count = new_count;
                        }
                        ;

VariableAssigment       : Parameter ASSIGN Expression    
                        { VariableAssigment* var = malloc(sizeof(VariableAssigment)); 
                          var->name = $1.name; 
                          var->value = $3; 
                          var->static_type = $1.type;
                          $$ = var;
                        }
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

ArgList                 : OptionalArgList Expression
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

ExprBlock               : LBRACKET ExpressionList RBRACKET 
                        {
                            $$ = create_expression_block_node($2.nodes, $2.count, type_table);
                            free($2.nodes);
                        }

ExpressionList           : OptionalExpressionList Expression OptionalEnd
                        {
                            int new_count = $1.count + 1;
                            ASTNode** all_nodes = realloc($1.nodes, new_count * sizeof(ASTNode*));
                            if (!all_nodes) DIE("Out of memory in ExpressionList");
                            all_nodes[new_count - 1] = $2;
                            $$.nodes = all_nodes;
                            $$.count = new_count;
                        }
                        |   /*empty*/               { $$.nodes = NULL; $$.count = 0; }
                        ;

OptionalExpressionList   : /* empty */               { $$.nodes = NULL; $$.count = 0; }
                        | OptionalExpressionList Expression SEMICOLON
                        {
                            int new_count = $1.count + 1;
                            ASTNode** nodes = realloc($1.nodes, new_count * sizeof(ASTNode*));
                            if (!nodes) DIE("Out of memory in ExpressionListOpt");
                            nodes[new_count - 1] = $2;
                            $$.nodes = nodes;
                            $$.count = new_count;
                        }
                        ;
                        
OptionalEnd             : SEMICOLON
                        | /* empty */
                        ;
%%

void yyerror(const char *s) {
    fprintf(stderr, "Error: %s\n", s);
}

