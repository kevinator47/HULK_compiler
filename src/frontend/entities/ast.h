// ast.h
#ifndef AST_H
#define AST_H

typedef enum {
    Number_Literal_Node ,
    Boolean_Literal_Node ,
    String_Literal_Node ,
    Unary_Op_Node ,
    Binary_Op_Node ,
    Expression_Block_Node,
} NodeType;

typedef enum {
    NOT_TK ,
    AND_TK ,
    OR_TK  ,
    PLUS_TK ,
    MINUS_TK ,
    MULT_TK ,
    DIV_TK ,
    MOD_TK ,
    CONCAT_TK ,
    D_CONCAT_TK ,
    EXP_TK ,
    GT_TK ,
    GE_TK ,
    LT_TK ,
    LE_TK ,
    EQ_TK ,
    NE_TK ,
} TokenType;

typedef struct ASTNode {
    NodeType type;
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

ASTNode* make_number_literal_node(double value);
ASTNode* make_boolean_literal_node(int value);
ASTNode* make_string_literal_node(char* value);
ASTNode* make_unary_op_literal_node(ASTNode* operand, TokenType operation);
ASTNode* make_binary_op_literal_node(ASTNode* left, ASTNode* right, TokenType operation);
ASTNode* make_expression_block_node(ASTNode** expressions, int count);
void print_ast(ASTNode* node, int indent_level);
#endif