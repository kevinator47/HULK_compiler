// ast.h
#ifndef AST_H
#define AST_H

typedef struct ASTNode {
    char type; // 'N' (número), 'V' (variable), 'B' (operación binaria)
    union {
        double number;
        char* variable;
        struct { 
            struct ASTNode* left;
            struct ASTNode* right;
            char op; 
        } binop;
    } value;
} ASTNode;

ASTNode* create_number(double value);
ASTNode* create_variable(char* name);
ASTNode* create_binop(ASTNode* left, char op, ASTNode* right);
void print_ast(ASTNode* node, int level);

#endif