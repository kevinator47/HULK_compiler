#include "ast.h"
#include "../hulk_type/hulk_type.h"
#include "../scope/symbol_table.h"

void free_ast_node(ASTNode *node) {
    if (!node) return;

    switch (node->type) {
        case AST_Node_Literal: {
            free_literal_node((LiteralNode*) node);
            break;
        }
        case AST_Node_Unary_Operation: {
            free_unary_operation_node((UnaryOperationNode*) node);
            break;
        }
        case AST_Node_Binary_Operation: {
            free_binary_operation_node((BinaryOperationNode*) node);
            break;
        }
        case AST_Node_Expression_Block: {
            free_expression_block_node((ExpressionBlockNode*) node);
            break;
        }
        case AST_Node_Conditional: {
            free_conditional_node((ConditionalNode*) node);
            break;
        }
        case AST_Node_While_Loop: {
            free_while_loop_node((WhileLoopNode*) node);
            break;
        }
        case AST_Node_Let_In: {
            free_let_in_node((LetInNode*) node);
            break;
        }
        case AST_Node_Variable: {
            free_variable_node((VariableNode*) node);
            break;
        }
        case AST_Node_Variable_Assigment: {
            free_variable_assigment_node((VariableAssigmentNode*)node);
            break;
        }
        case AST_Node_Reassign: {
            free_reassign_node((ReassignNode*) node);
            break;
        }
        case AST_Node_Function_Definition: {
            free_function_definition_node((FunctionDefinitionNode*) node);
            break;
        }
        case AST_Node_Function_Definition_List: {
            free_function_definition_list_node((FunctionDefinitionListNode*) node);
            break;
        }
        case AST_Node_Function_Call : {
            free_function_call_node((FunctionCallNode*) node);
            break;
        }
        case AST_Node_Type_Definition : {
            free_type_definition_node((TypeDefinitionNode*)node);
            break;
        }
        case AST_Node_Type_Definition_List : {
            free_type_definition_list_node((TypeDefinitionListNode*)node);
            break;
        }
        case AST_Node_Program: {
            free_program_node((ProgramNode*) node);
            break;
        }
        default:
            free(node);
            break;
    }
}

void free_literal_node(LiteralNode* node) {
    // Libera la memoria reservada por un nodo de literal
    if (!node) return;

    if (node->base.return_type && node->base.return_type->tag == HULK_Type_String) {
        free(node->value.string_value);
    }
    free(node);
}

void free_unary_operation_node(UnaryOperationNode *node) {
    // Libera la memoria reservada por un nodo de operacion unaria
    if (!node) return;
    
    free_ast_node(node->operand);
    free(node);
}

void free_binary_operation_node(BinaryOperationNode *node) {
    // Libera la memoria reservada por un nodo de operacion binaria
    if (!node) return;

    free_ast_node(node->left);
    free_ast_node(node->right);
    free(node);
}

void free_expression_block_node(ExpressionBlockNode *node) {
    // Libera la memoria reservada por un nodo de bloque de expresiones
    if (!node) return;

    if(node->expressions)
    {
        for (int i = 0; i < node->expression_count; i++) {
            free_ast_node(node->expressions[i]);
        }
        free(node->expressions);
    }
    free(node);
}

void free_conditional_node(ConditionalNode *node) {
    // Libera la memoria reservada por un nodo de expresion condicional
    if (!node) return;

    free_ast_node(node->condition);
    free_ast_node(node->then_branch);
    free_ast_node(node->else_branch);
    
    free(node);
}

void free_while_loop_node(WhileLoopNode *node) {
    // Libera la memoria reservada por un nodo de expresion While
    if (!node) return;

    free_ast_node(node->condition);
    free_ast_node(node->body);
    free(node);
}

void free_let_in_node(LetInNode *node) {
    // Libera la memoria reservada por un nodo Let In
    if (!node) return;

    if (node->assigments) 
    {
        for (int i = 0; i < node->assigment_count; ++i) 
        {
            VariableAssigment* a = node->assigments[i];
            if (a) {
                free(a->name);
                free(a->static_type);
                free_ast_node(a->value);
                free(a);
            }
        }
        free(node->assigments);
    }

    free_ast_node(node->body);
    free(node);
}

void free_variable_node(VariableNode *node) {
    // Libera la memoria reservada por un nodo Variable
    if (!node) return;

    if(node->name)
        free(node->name);
    free(node);
}

void free_variable_assigment_node(VariableAssigmentNode* node) {
    // Libera la memoria reservada por un nodo de asignacion de variable
    if(!node) return;

    if (node->assigment) 
    {
        free(node->assigment->name);
        
        free_ast_node(node->assigment->value);
        
        free(node->assigment);
    }

    free(node);
}

void free_reassign_node(ReassignNode *node) {
    // Libera la memoria reservada por un nodo Reassign
    if (!node) return;

    if(node->name)
        free(node->name);
    free_ast_node(node->value);
    free(node);
}

void free_function_definition_node(FunctionDefinitionNode* node) {
    // Libera la memoria reservada por un nodo Definición de Función
    if (node->name)
        free(node->name);
    
    if (node->static_return_type) 
        free(node->static_return_type);
    
    if (node->params) 
    {
        for (int i = 0; i < node->param_count; ++i) {
            if (node->params[i]) {
                if (node->params[i]->name) free(node->params[i]->name);
                if (node->params[i]->static_type) free(node->params[i]->static_type);
                free(node->params[i]);
            }
        }
        free(node->params);
    }
    free_ast_node(node->body);
    free(node);
}

void free_function_definition_list_node(FunctionDefinitionListNode *node) {
    // Libera la memoria reservada por un nodo Lista Definiciones de Funciones
    if (!node) return;

    if(node->functions)
    {
        for (int i = 0; i < node->function_count; i++) {
            free_function_definition_node(node->functions[i]);
        }
        free(node->functions);
    }
    free(node);
}

void free_function_call_node(FunctionCallNode* node) {
    // Libera la memoria reservada por un nodo Llamado de Función
    if (!node) return;

    if (node->name) free(node->name);

    if (node->args)
    {
        for (int i = 0; i < node->arg_count; i++) {
            free_ast_node(node->args[i]);
        }

        free(node->args);
    }
    free(node);
}

void free_type_definition_node(TypeDefinitionNode* node) {
    // Libera la memoria reservada por un nodo Definición de Tipo
    if (!node) return;

    if(node->params)
    {
        for (int i = 0; i < node->param_count; i++) 
        {
            free(node->params[i]->name);
            free(node->params[i]->static_type);
            free(node->params[i]);
        }
        free(node->params);
    }
    
    if(node->parent_args)
    {
        for (int i = 0; i < node->parent_arg_count; i++) 
        {
            free_ast_node(node->parent_args[i]);
        }
        free(node->parent_args);
    }
    
    free(node->type_name);
    free(node->parent_name);

    free_expression_block_node(node->body);
    free(node);
}

void free_type_definition_list_node(TypeDefinitionListNode* node) {
    // Libera la memoria reservada por un nodo Lista Definiciones de Funciones
    if (!node) return;

    for (int i = 0; i < node->count; i++) {
        free_type_definition_node(node->definitions[i]);
    }

    free(node->definitions);
    free(node);
}

void free_instanciate_type_node(InstanciateNode* node)
{
    // Libera la memoria reservada por un nodo Instanciacion de Tipo
    if (!node) return;

    if (node->type_name) free(node->type_name);

    if (node->args)
    {
        for (int i = 0; i < node->arg_count; i++) {
            free_ast_node(node->args[i]);
        }

        free(node->args);
    }
    free(node);
}

void free_func_call_type_node(FuntionCallTypeNode* node)
{
    if(!node) return;

    if(node->type_name) free(node->type_name);
    if(node->func_name) free(node->func_name);

    if(node->args)
    {
        for (int i = 0; i < node->arg_count; i++)
        {
            free_ast_node(node->args[i]);
        }
        free(node->args);
    }
    free(node);
}

void free_program_node(ProgramNode* node) {
    // Libera la memoria reservada por un nodo Programa
    if (!node) return;

    free_function_definition_list_node(node->function_list);
    free_type_definition_list_node(node->type_definitions);
    free_ast_node(node->root);

    free(node);
}