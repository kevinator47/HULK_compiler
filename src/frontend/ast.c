// ast.c
#include "ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ASTNode* create_number(double value) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = 'N';
    node->value.number = value;
    return node;
}

ASTNode* create_variable(char* name) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = 'V';
    node->value.variable = strdup(name);
    return node;
}

ASTNode* create_binop(ASTNode* left, char op, ASTNode* right) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = 'B';
    node->value.binop.op = op;
    node->value.binop.left = left;
    node->value.binop.right = right;
    return node;
}

void print_ast(ASTNode* node, int level) {
    if (!node) return;
    for (int i = 0; i < level; i++) printf("  ");
    switch (node->type) {
        case 'N':
            printf("Number: %f\n", node->value.number);
            break;
        case 'V':
            printf("Variable: %s\n", node->value.variable);
            break;
        case 'B':
            printf("BinOp: %c\n", node->value.binop.op);
            print_ast(node->value.binop.left, level + 1);
            print_ast(node->value.binop.right, level + 1);
            break;
    }
}