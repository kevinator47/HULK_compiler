#include "ast.h"
#include "../common/common.h"
#include "../hulk_type/hulk_type.h"

void print_ast_node(ASTNode *node, int indent) {
    if (!node) return;
    
    print_indent(indent);

    switch (node->type) {
        case AST_Node_Literal: {
            print_literal_node((LiteralNode*) node, indent);
            break;
        }
        case AST_Node_Unary_Operation: {
            print_unary_operation_node((UnaryOperationNode*) node, indent);
            break;
        }
        case AST_Node_Binary_Operation: {
            print_binary_operation_node((BinaryOperationNode*) node, indent);
            break;
        }
        case AST_Node_Expression_Block: {
            print_expression_block_node((ExpressionBlockNode*) node, indent);
            break;
        }
        case AST_Node_Conditional: {
            print_conditional_node((ConditionalNode*) node, indent);
            break;
        }
        case AST_Node_While_Loop: {
            print_while_loop_node((WhileLoopNode*) node, indent);
            break;
        }
        case AST_Node_Let_In: {
            print_let_in_node((LetInNode*) node, indent);
            break;
        }
        case AST_Node_Variable: {
            print_variable_node((VariableNode*) node, indent);
            break;
        }
        case AST_Node_Reassign: {
            print_reassign_node((ReassignNode*) node, indent);
            break;
        }
        case AST_Node_Function_Definition: {
            print_function_definition_node((FunctionDefinitionNode*) node, indent);
            break;
        }
        case AST_Node_Function_Definition_List: {
            print_function_definition_list_node((FunctionDefinitionListNode*) node, indent);
            break;
        }
        case AST_Node_Function_Call : {
            print_function_call_node((FunctionCallNode*) node , indent);
            break;
        }
        case AST_Node_Program: {
            print_program_node((ProgramNode*) node, indent);
            break;
        }
        default:
            printf("Unknown AST Node Type\n");
            break;
    }
}

void print_literal_node(LiteralNode *node, int indent) {
    if (!node) return;

    print_indent(indent);

    switch (node->base.return_type->tag) {
        case HULK_Type_Number:
            printf("%f\n", node->value.number_value);
            break;
        case HULK_Type_String:
            printf("%s\n", node->value.string_value);
            break;
        case HULK_Type_Boolean:
            printf("%s\n", node->value.bool_value ? "true" : "false");
            break;
        default:
            printf("Unknown Literal Type\n");
            break;
    }
}

void print_unary_operation_node(UnaryOperationNode *node, int indent) {
    if (!node) return;

    print_indent(indent);

    printf("%s:\n", Hulk_Op_Names[node->operator]);
    print_ast_node(node->operand, indent + 1);
}

void print_binary_operation_node(BinaryOperationNode *node, int indent) {
    if (!node) return;

    print_indent(indent);

    printf("%s:\n", Hulk_Op_Names[node->operator]);
    print_ast_node(node->left, indent + 1);
    print_ast_node(node->right, indent + 1);
}

void print_expression_block_node(ExpressionBlockNode *node, int indent) {
    if (!node) return;

    print_indent(indent);

    printf("Expression Block:\n");
    for (int i = 0; i < node->expression_count; i++) {
        print_ast_node(node->expressions[i], indent);
    }
}

void print_conditional_node(ConditionalNode *node, int indent) {
    if (!node) return;

    print_indent(indent);

    printf("IF:\n");
    print_ast_node(node->condition, indent + 1);

    print_indent(indent);
    
    printf("THEN: \n");
    print_ast_node(node->then_branch, indent + 1);
    
    if (node->else_branch) {

        print_indent(indent);
        
        printf("ELSE:\n");
        print_ast_node(node->else_branch, indent + 1);
    }
}

void print_while_loop_node(WhileLoopNode *node, int indent) {
    if (!node) return;

    print_indent(indent);

    printf("WHILE:\n");
    print_ast_node(node->condition, indent + 1);
    
    print_indent(indent);
    
    printf("BODY:\n");
    print_ast_node(node->body, indent + 1);
}

void print_let_in_node(LetInNode *node, int indent) {
    if (!node) return;

    print_indent(indent);

    printf("LET:\n");
    for (int i = 0; i < node->assigment_count; i++) {
        VariableAssigment *assigment = &node->assigments[i];
        print_indent(indent);
        printf("%s = \n", assigment->name);
        print_ast_node(assigment->value, indent + 1);
    }
    
    print_indent(indent);
    printf("IN:\n");
    print_ast_node(node->body, indent + 1);
}

void print_variable_node(VariableNode *node, int indent) {
    if (!node) return;

    if (node->scope)
    {
        
        Symbol *symbol = lookup_symbol(node->scope, node->name, true);
        if (symbol) {
            print_indent(indent);
            printf("Variable: \n");
            
            print_indent(indent + 1);
        
            printf("Name: %s\n", node->name);

            print_indent(indent + 1);

            printf("Type: %s\n", symbol->type->type_name);

            print_indent(indent + 1);

            if(symbol->value) {
                printf("Value: \n");
                print_ast_node(symbol->value, indent + 2);
            } else {
                printf("Value: NULL\n");
            }
        }
        else {
            print_indent(indent);
            printf("Variable: \n%s (not initialized)", node->name);
        }
    }
}

void print_reassign_node(ReassignNode *node, int indent) {
    if (!node) return;

    print_indent(indent);

    printf("REASSIGN:\n");
    print_indent(indent + 1);
    
    printf("Name: %s\n", node->name);
    
    print_indent(indent + 1);

    printf("Type: %s\n", lookup_symbol(node->scope, node->name, true)->type->type_name);

    print_indent(indent + 1);
    
    printf("Value:\n");
    print_ast_node(node->value, indent + 2);
}

void print_function_definition_node(FunctionDefinitionNode *node, int indent) {
    if (!node) return;

    print_indent(indent);

    printf("FUNCTION: %s\n", node->name);
    
    print_indent(indent + 1);
    
    printf("Parameters: ");
    for (int i = 0; i < node->param_count; i++) {
        if( i == node->param_count - 1)
            printf("%s \n", node->params_names[i]);
        else
        {
            printf("%s, ", node->params_names[i]);
        }        
    }

    print_indent(indent + 1);
    
    printf("Body:\n");
    print_ast_node(node->body, indent + 1);
}

void print_function_definition_list_node(FunctionDefinitionListNode *node, int indent) {
    if (!node) return;

    for (int i = 0; i < node->function_count; i++) {
        print_function_definition_node(node->functions[i], indent);
    }
}

void print_function_call_node(FunctionCallNode* node, int indent) {
    if (!node) return;

    print_indent(indent);
    printf("FunctionCall: %s\n", node->name);
    
    print_indent(indent);
    printf("Return type: %s\n",node->base.return_type->type_name);

    print_indent(indent);
    printf("Argument count: %d\n", node->arg_count);

    for (int i = 0; i < node->arg_count; i++) {
        print_ast_node(node->args[i], indent);
    }
}

void print_program_node(ProgramNode *node, int indent) {
    if (!node) return;

    print_indent(indent);

    printf("PROGRAM:\n");
    
    print_indent(indent + 1);
    
    printf("Function Definitions:\n");
    print_function_definition_list_node(node->function_list, indent + 2);
    
    print_indent(indent + 1);
    
    printf("Code:\n");
    print_ast_node(node->root, indent + 2);
}

void print_indent(int indent) {
    for (int i = 0; i < indent + 1; i++) {
        printf("  ");
    }
}