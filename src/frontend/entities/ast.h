// ast.h
#ifndef AST_H
#define AST_H

#include "../common/common.h"
#include "symbol_table.h"
#include <stdbool.h>
#include <llvm-c/Core.h>
#include <llvm-c/Analysis.h>

struct LLVMCodeGenerator; // Forward declaration

typedef enum {
    Number_Literal_Node , Boolean_Literal_Node , String_Literal_Node , Unary_Op_Node ,
    Binary_Op_Node , Expression_Block_Node, Variable_Node, Let_Node, Reassign_Node, 
    Conditional_Node, Function_Call_Node, While_Loop_Node,
} NodeType;

typedef enum {
    NOT_TK , AND_TK , OR_TK ,
    PLUS_TK , MINUS_TK , MULT_TK , DIV_TK ,
    EXP_TK , MOD_TK , CONCAT_TK , D_CONCAT_TK , 
    GT_TK , GE_TK , LT_TK , LE_TK , EQ_TK , NE_TK ,
} TokenType;

typedef struct ASTNode {
    NodeType type;
    LLVMValueRef (*accept)(struct ASTNode* self, struct LLVMCodeGenerator* visitor);
} ASTNode;

typedef struct NumberLiteralNode {
    ASTNode base;
    double value;
} NumberLiteralNode;

typedef struct BooleanLiteralNode {
    ASTNode base;
    int value;
} BooleanLiteralNode;

typedef struct StringLiteralNode {
    ASTNode base;
    char* value;
} StringLiteralNode;

typedef struct UnaryOpNode {
    ASTNode base;
    ASTNode* operand;
    TokenType operation;
} UnaryOpNode;

typedef struct BinaryOpNode {
    ASTNode base;
    ASTNode* left;
    ASTNode* right;
    TokenType operation;
} BinaryOpNode;

typedef struct ExpressionBlockNode {
    ASTNode base;
    ASTNode** expressions;
    int count;
} ExpressionBlockNode;

typedef struct VariableNode {
    ASTNode base;
    char* name;
    SymbolTable* scope;
} VariableNode;

typedef struct LetNode {
    ASTNode base;
    SymbolTable* scope;
    ASTNode* body;
} LetNode;

typedef struct ReassignNode {
    ASTNode base;
    char* name;
    ASTNode* value;
    SymbolTable* scope;
} ReassignNode;
typedef struct ConditionalNode {
    ASTNode base;
    ASTNode* condition;       
    ASTNode* if_branch;       
    struct {
        ASTNode** conditions; 
        ASTNode** branches;   
        int count;            
    } elifs;
    ASTNode* else_branch;     
} ConditionalNode;

typedef struct FunctionCallNode {
    ASTNode base;
    char* name;
    ASTNode** arguments;
    int arg_count;
} FunctionCallNode;

typedef struct WhileLoopNode
{
    ASTNode base;
    ASTNode* condition;
    ASTNode* body;
}WhileLoopNode;

// Prototypes for AST node creation functions
ASTNode* make_number_literal_node(double value);
ASTNode* make_boolean_literal_node(int value);
ASTNode* make_string_literal_node(char* value);
ASTNode* make_unary_op_literal_node(ASTNode* operand, TokenType operation);
ASTNode* make_binary_op_literal_node(ASTNode* left, ASTNode* right, TokenType operation);
ASTNode* make_expression_block_node(ASTNode** expressions, int count);
ASTNode* make_variable_node(char* name, SymbolTable* scope);
ASTNode* make_let_node(SymbolTable* scope, ASTNode* body);
ASTNode* make_reassign_node(char* name, ASTNode* value, SymbolTable* scope);
ASTNode* make_conditional_node(ASTNode* condition, ASTNode* if_branch,  ASTNode** elif_conditions, ASTNode** elif_branches, int elif_count, ASTNode* else_branch);
ASTNode* make_function_call_node(char* name, ASTNode** arguments, int arg_count);
ASTNode* make_while_loop_node(ASTNode* condition, ASTNode* body);

// Prototypes
void print_ast(ASTNode* node, int indent_level);
void free_ast(ASTNode* node);

#endif