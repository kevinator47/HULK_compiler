#include "ast.h"
#include "../hulk_type/hulk_type.h"
#include "../scope/symbol_table.h"

void free_ast_node(ASTNode *node) {
    if (!node) return;

    switch (node->type) {
        case AST_Node_Literal: {
            LiteralNode *literal_node = (LiteralNode*) node;
            free_literal_node(literal_node);
            break;
        }
        case AST_Node_Unary_Operation: {
            UnaryOperationNode *unary_node = (UnaryOperationNode*) node;
            free_unary_operation_node(unary_node);
            break;
        }
        case AST_Node_Binary_Operation: {
            BinaryOperationNode *binary_node = (BinaryOperationNode*) node;
            free_binary_operation_node(binary_node);
            break;
        }
        case AST_Node_Expression_Block: {
            ExpressionBlockNode *block_node = (ExpressionBlockNode*) node;
            free_expression_block_node(block_node);
            break;
        }
        case AST_Node_Conditional: {
            ConditionalNode *conditional_node = (ConditionalNode*) node;
            free_conditional_node(conditional_node);
            break;
        }
        case AST_Node_While_Loop: {
            WhileLoopNode *while_node = (WhileLoopNode*) node;
            free_while_loop_node(while_node);
            break;
        }
        case AST_Node_Let_In: {
            LetInNode *let_in_node = (LetInNode*) node;
            free_let_in_node(let_in_node);
            break;
        }
        case AST_Node_Variable: {
            VariableNode *variable_node = (VariableNode*) node;
            free_variable_node(variable_node);
            break;
        }
        case AST_Node_Reassign: {
            ReassignNode *reassign_node = (ReassignNode*) node;
            free_reassign_node(reassign_node);
            break;
        }
        case AST_Node_Function_Definition: {
            FunctionDefinitionNode *function_def_node = (FunctionDefinitionNode*) node;
            free_function_definition_node(function_def_node);
            break;
        }
        case AST_Node_Function_Definition_List: {
            FunctionDefinitionListNode *function_def_list_node = (FunctionDefinitionListNode*) node;
            free_function_definition_list(function_def_list_node);
            break;
        }
        case AST_Node_Function_Call : {
            FunctionCallNode * func_call_node = (FunctionCallNode*) node;
            free_function_call_node(func_call_node);
            break;
        }
        case AST_Node_Program: {
            ProgramNode *program_node = (ProgramNode*) node;
            free_program_node(program_node);
            break;
        }
        default:
            free(node);
            break;
    }
}

void free_literal_node(LiteralNode *node) {    
    if (node->base.return_type->tag == HULK_Type_String) {
        free(node->value.string_value);
    }
    free(node);
}

void free_unary_operation_node(UnaryOperationNode *node) {
    free_ast_node(node->operand);
    free(node);
}

void free_binary_operation_node(BinaryOperationNode *node) {
    free_ast_node(node->left);
    free_ast_node(node->right);
    free(node);
}

void free_expression_block_node(ExpressionBlockNode *node) {
    if (!node) return;

    for (int i = 0; i < node->expression_count; i++) {
        free_ast_node(node->expressions[i]);
    }
    free(node->expressions);
    free(node);
}

void free_conditional_node(ConditionalNode *node) {
    if (!node) return;

    free_ast_node(node->condition);
    free_ast_node(node->then_branch);
    if (node->else_branch) {
        free_ast_node(node->else_branch);
    }
    free(node);
}

void free_while_loop_node(WhileLoopNode *node) {
    if (!node) return;

    free_ast_node(node->condition);
    free_ast_node(node->body);
    free(node);
}

void free_let_in_node(LetInNode *node) {
    if (!node) return;

    for (int i = 0; i < node->assigment_count; i++) {
        free(node->assigments[i].name);
        free_ast_node(node->assigments[i].value);
    }
    free(node->assigments);
    free_symbol_table(node->scope);
    free_ast_node(node->body);
    free(node);
}

void free_variable_node(VariableNode *node) {
    if (!node) return;

    free(node->name);
    free(node);
}

void free_reassign_node(ReassignNode *node) {
    if (!node) return;

    free(node->name);
    free_ast_node(node->value);
    free(node);
}

void free_function_definition_node(FunctionDefinitionNode *node) {
    if (!node) return;

    free(node->name);
    for (int i = 0; i < node->param_count; i++) {
        free(node->params[i]->name);
        free(node->params[i]);
    }
    free(node->params);
    free_ast_node(node->body);
    free(node);
}

void free_function_definition_list(FunctionDefinitionListNode *list) {
    if (!list) return;

    for (int i = 0; i < list->function_count; i++) {
        free_function_definition_node(list->functions[i]);
    }
    free(list->functions);
    free(list);
}

void free_function_call_node(FunctionCallNode* node) {
    if (!node) return;

    if (node->name) free(node->name);

    for (int i = 0; i < node->arg_count; i++) {
        free_ast_node(node->args[i]);
    }

    free(node->args);
    free(node);
}

void free_program_node(ProgramNode *node) {
    if (!node) return;

    free_function_definition_list(node->function_list);
    free_ast_node(node->root);
    free(node);
}