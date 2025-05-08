// ast.c
#include "ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ASTNode* make_number_literal_node(double value) {
    NumberLiteralNode* node = malloc(sizeof(NumberLiteralNode));
    if(!node) return NULL;

    node->base.type = Number_Literal_Node;
    node->value = value;
    return (ASTNode*) node;
}

ASTNode* make_boolean_literal_node(int value) {
    BooleanLiteralNode* node = malloc(sizeof(BooleanLiteralNode));
    if(!node) return NULL;

    node->base.type = Boolean_Literal_Node;
    node->value = value;
    return (ASTNode*) node;
}

ASTNode* make_string_literal_node(char* value) {
    StringLiteralNode* node = malloc(sizeof(StringLiteralNode));
    if(!node) return NULL;

    node->base.type = String_Literal_Node;
    node->value = value;
    return (ASTNode*) node;
}

ASTNode* make_unary_op_literal_node(ASTNode* operand, TokenType operation) {
    
    if (!operand) return NULL;
    UnaryOpNode* node = malloc(sizeof(UnaryOpNode));
    if(!node) return NULL;

    node->base.type = Unary_Op_Node;
    node->operand = operand;
    node->operation = operation;
    return (ASTNode*) node;
}

ASTNode* make_binary_op_literal_node(ASTNode* left, ASTNode* right, TokenType operation) {
    
    if(!left || !right) return NULL;
    BinaryOpNode* node = malloc(sizeof(BinaryOpNode));
    
    if(!node) return NULL;

    node->base.type = Binary_Op_Node;
    node->left = left;
    node->right = right;
    node->operation = operation;
    return (ASTNode*) node;
}

ASTNode* make_expression_block_node(ASTNode** expressions, int count) {
    ExpressionBlockNode* node = malloc(sizeof(ExpressionBlockNode));
    if (!node) return NULL;

    // Crear un nuevo array para las expresiones
    node->expressions = malloc(count * sizeof(ASTNode*));
    if (!node->expressions) {
        free(node);
        return NULL;
    }

    // Copiar cada expresión
    for (int i = 0; i < count; i++) {
        node->expressions[i] = expressions[i];
    }

    node->base.type = Expression_Block_Node;
    node->count = count;
    return (ASTNode*)node;
}

void print_ast(ASTNode* node, int indent_level) {
    if (node == NULL) return;
    
    // Imprimir indentación
    for (int i = 0; i < indent_level; i++) {
        printf("  ");
    }
    
    // Imprimir nodo según su tipo
    switch (node->type) {
        case Number_Literal_Node: {
            NumberLiteralNode* n = (NumberLiteralNode*)node;
            printf("Number: %f\n", n->value);
            break;
        }
        case Boolean_Literal_Node: {
            BooleanLiteralNode* n = (BooleanLiteralNode*)node;
            printf("Boolean: %s\n", n->value ? "true" : "false");
            break;
        }

        case String_Literal_Node: {
            StringLiteralNode* n = (StringLiteralNode*)node;
            printf("String: \"%s\"\n", n->value);
            break;
        }

        case Unary_Op_Node: {
            UnaryOpNode* n = (UnaryOpNode*)node;
            printf("UnaryOp: ");
            switch (n->operation) {
                case NOT_TK: printf("NOT\n"); break;
                case MINUS_TK: printf("NEGATE\n"); break;
                default: printf("Unknown\n");
            }
            print_ast(n->operand, indent_level + 1);
            break;
        }

        case Expression_Block_Node: {
            ExpressionBlockNode* n = (ExpressionBlockNode*)node;
            printf("ExpressionBlock: (%d expressions)\n", n->count);
            for(int i = 0; i < n->count; i++) {
                print_ast(n->expressions[i], indent_level + 1);
            }
            break;
        }

        case Binary_Op_Node: {
            BinaryOpNode* n = (BinaryOpNode*)node;
            printf("BinaryOp: ");
            switch (n->operation) {
                case PLUS_TK: printf("+\n"); break;
                case MINUS_TK: printf("-\n"); break;
                case MULT_TK: printf("*\n"); break;
                case DIV_TK: printf("\\\n"); break;
                case MOD_TK: printf("MOD\n"); break;
                case EXP_TK: printf("^\n"); break;
                case AND_TK: printf("&\n"); break;
                case OR_TK: printf("|\n"); break;
                case GT_TK: printf(">\n"); break;
                case GE_TK: printf(">=\n"); break;
                case LT_TK: printf("<\n"); break;
                case LE_TK: printf("<=\n"); break;
                case EQ_TK: printf("==\n"); break;
                case NE_TK: printf("!=\n"); break;
                case CONCAT_TK: printf("@\n"); break;
                case D_CONCAT_TK: printf("@@\n"); break;
                default: printf("Unknown\n");
            }
            print_ast(n->left, indent_level + 1);
            print_ast(n->right, indent_level + 1);
            break;
        }
    }
}
