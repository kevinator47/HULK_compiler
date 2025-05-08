// ast.c
#include "ast.h"
#include "symbol_table.h"
#include "../common/common.h"
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

ASTNode* make_let_node(SymbolTable* scope, ASTNode* body) {
    LetNode* node = malloc(sizeof(LetNode));
    if (!node) return NULL;

    node->base.type = Let_Node;
    node->scope = scope;
    node->body = body;
    return (ASTNode*)node;
}

ASTNode* make_variable_node(char* name, SymbolTable* scope) {
    VariableNode* node = malloc(sizeof(VariableNode));
    if (!node) return NULL;

    node->base.type = Variable_Node;
    node->name = strdup(name);
    node->scope = scope;
    return (ASTNode*)node;
}

ASTNode* make_conditional_node(ASTNode* condition, ASTNode* if_branch, ASTNode** elif_conditions, ASTNode** elif_branches, int elif_count, ASTNode* else_branch) {
    
    ConditionalNode* node = malloc(sizeof(ConditionalNode));
    if (!node) return NULL;

    node->base.type = Conditional_Node;
    node->condition = condition;
    node->if_branch = if_branch;
    node->elifs.conditions = elif_conditions;
    node->elifs.branches = elif_branches;
    node->elifs.count = elif_count;
    node->else_branch = else_branch;

    return (ASTNode*)node;
}

ASTNode* make_function_call_node(char* name, ASTNode** arguments, int arg_count) {
    FunctionCallNode* node = malloc(sizeof(FunctionCallNode));
    if (!node) return NULL;

    node->base.type = Function_Call_Node;
    node->name = strdup(name);
    node->arguments = malloc(sizeof(ASTNode*) * arg_count);
    memcpy(node->arguments, arguments, sizeof(ASTNode*) * arg_count);
    node->arg_count = arg_count;
    
    return (ASTNode*)node;
}

void free_ast(ASTNode* node) {
    if (!node) return;

    switch (node->type) {
        case String_Literal_Node: {
            StringLiteralNode* n = (StringLiteralNode*)node;
            free(n->value);
            break;
        }
        case Unary_Op_Node: {
            UnaryOpNode* n = (UnaryOpNode*)node;
            free_ast(n->operand);
            break;
        }
        case Binary_Op_Node: {
            BinaryOpNode* n = (BinaryOpNode*)node;
            free_ast(n->left);
            free_ast(n->right);
            break;
        }
        case Expression_Block_Node: {
            ExpressionBlockNode* n = (ExpressionBlockNode*)node;
            for (int i = 0; i < n->count; i++) {
                free_ast(n->expressions[i]);
            }
            free(n->expressions);
            break;
        }
        case Variable_Node: {
            VariableNode* n = (VariableNode*)node;
            free(n->name);
            break;
        }
        case Function_Call_Node: {
            FunctionCallNode* n = (FunctionCallNode*)node;
            free(n->name);
            for (int i = 0; i < n->arg_count; i++) {
                free_ast(n->arguments[i]);
            }
            free(n->arguments);
            break;
        }
        case Conditional_Node: {
            ConditionalNode* n = (ConditionalNode*)node;
            free_ast(n->condition);
            free_ast(n->if_branch);
            for (int i = 0; i < n->elifs.count; i++) {
                free_ast(n->elifs.conditions[i]);
                free_ast(n->elifs.branches[i]);
            }
            free(n->elifs.conditions);
            free(n->elifs.branches);
            free_ast(n->else_branch);
            break;
        }
        case Let_Node: {
            LetNode* n = (LetNode*)node;
            free_symbol_table(n->scope);
            free_ast(n->body);
            break;
        }
    }
    free(node);
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

        case Variable_Node: {
            VariableNode* n = (VariableNode*)node;
            printf("Variable: %s\n", n->name);
            
            // Buscar la variable en el scope
            Symbol* symbol = lookup_symbol(n->scope, n->name);
            if (symbol) {
                printf("%*sValue:\n", (indent_level + 1) * 2, "");
                print_ast(symbol->expression, indent_level + 2);
            } else {
                printf("%*sUndefined variable\n", (indent_level + 1) * 2, "");
            }
            break;
        }

        case Let_Node: {
            LetNode* n = (LetNode*)node;
            printf("Let Expression:\n");
            // Imprimir variables en el scope
            Symbol** table = n->scope->table;
            for (int i = 0; i < n->scope->size; i++) {
                Symbol* current = table[i];
                while (current) {
                    printf("%*s%s = \n", (indent_level + 1) * 2, "", current->name);
                    print_ast(current->expression, indent_level + 2);
                    current = current->next;
                }
            }
            printf("%*sIn:\n", (indent_level + 1) * 2, "");
            print_ast(n->body, indent_level + 2);
            break;
        }

        case Conditional_Node: {
            ConditionalNode* n = (ConditionalNode*)node;
            printf("Conditional:\n");
            printf("%*sIf:\n", (indent_level + 1) * 2, "");
            print_ast(n->condition, indent_level + 2);
            printf("%*sThen:\n", (indent_level + 1) * 2, "");
            print_ast(n->if_branch, indent_level + 2);
        
            for (int i = 0; i < n->elifs.count; i++) {
                printf("%*sElif:\n", (indent_level + 1) * 2, "");
                print_ast(n->elifs.conditions[i], indent_level + 2);
                printf("%*sThen:\n", (indent_level + 1) * 2, "");
                print_ast(n->elifs.branches[i], indent_level + 2);
            }
        
            printf("%*sElse:\n", (indent_level + 1) * 2, "");
            print_ast(n->else_branch, indent_level + 2);
            break;
        }

        case Function_Call_Node: {
            FunctionCallNode* n = (FunctionCallNode*)node;
            printf("Function Call: %s\n", n->name);
            printf("%*sArguments:\n", (indent_level + 1) * 2, "");
            for (int i = 0; i < n->arg_count; i++) {
                print_ast(n->arguments[i], indent_level + 2);
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
