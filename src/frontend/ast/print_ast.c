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
        case AST_Node_Variable_Assigment: {
            print_variable_assigment_node((VariableAssigmentNode*)node, indent);
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
        case AST_Node_Type_Definition: {
            print_type_definition_node((TypeDefinitionNode*)node, indent);
            break;
        }
        case AST_Node_Type_Definition_List: {
            print_type_definition_list_node((TypeDefinitionListNode*)node, indent);
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

void print_literal_node(LiteralNode* node, int indent) {
    // Imprime un nodo literal [para DEBUG]
    if(!node) return;

    print_indent(indent);
    printf("LiteralNode(");
    
    switch (node->base.return_type->tag) {
        case HULK_Type_Number:
            printf("Number: %f", node->value.number_value);
            break;
        case HULK_Type_String:
            printf("String: \"%s\"", node->value.string_value);
            break;
        case HULK_Type_Boolean:
            printf("Bool: %s", node->value.bool_value ? "true" : "false");
            break;
        default:
            printf("Unknown type");
            break;
    }
    printf(")\n");
}

void print_unary_operation_node(UnaryOperationNode *node, int indent) {
    // Imprime un nodo de operacion unaria [para DEBUG]
    if (!node) return;

    print_indent(indent);
    printf("%s:\n", Hulk_Op_Names[node->operator]);
    print_ast_node(node->operand, indent + 1);
}

void print_binary_operation_node(BinaryOperationNode *node, int indent) {
    // Imprime un nodo de operacion binaria [para DEBUG]
    if (!node) return;

    print_indent(indent);

    printf("%s:\n", Hulk_Op_Names[node->operator]);
    print_ast_node(node->left, indent + 1);
    print_ast_node(node->right, indent + 1);
}

void print_expression_block_node(ExpressionBlockNode *node, int indent) {
    // Imprime un nodo de bloque de expresiones [para DEBUG]
    if (!node) return;
    
    print_indent(indent);

    printf("Expression Block(%d expressions):\n", node->expression_count);
    for (int i = 0; i < node->expression_count; i++) {
        print_ast_node(node->expressions[i], indent + 1);
    }
}

void print_conditional_node(ConditionalNode* node, int indent) {
    // Imprime un nodo de expresion condicional [para DEBUG]
    if(!node) return;

    print_indent(indent);
    printf("ConditionalNode:\n");

    print_indent(indent + 1);
    printf("Condition:\n");
    print_ast_node(node->condition, indent + 2);

    print_indent(indent + 1);
    printf("Then branch:\n");
    print_ast_node(node->then_branch, indent + 2);

    if (node->else_branch) {
        print_indent(indent + 1);
        printf("Else branch:\n");
        print_ast_node(node->else_branch, indent + 2);
    } else {
        print_indent(indent + 1);
        printf("Else branch: NULL\n");
    }
}

void print_while_loop_node(WhileLoopNode* node, int indent) {
    // Imprime un nodo de expresion While [para DEBUG]
    if(!node) return;

    print_indent(indent);
    printf("WhileLoopNode:\n");

    print_indent(indent + 1);
    printf("Condition:\n");
    print_ast_node(node->condition, indent + 2);

    print_indent(indent + 1);
    printf("Body:\n");
    print_ast_node(node->body, indent + 2);
}

void print_variable_assigment_node(VariableAssigmentNode* node, int indent) {
    // Imprime un nodo de asignacion de variable [para DEBUG]
    print_indent(indent);
    printf("VariableAssigmentNode:\n");

    print_indent(indent + 1);
    printf("Name: %s\n", node->assigment->name);

    print_indent(indent + 1);
    printf("Static Type: %s\n", node->assigment->static_type);
    
    print_indent(indent + 1);
    printf("Value:\n");
    print_ast_node(node->assigment->value, indent + 2);
}

void print_let_in_node(LetInNode* node, int indent) {
    // Imprime un nodo Let In [para DEBUG]
    if (!node) return;

    print_indent(indent);
    printf("LetInNode:\n");

    for (int i = 0; i < node->assigment_count; ++i) {
        print_indent(indent + 1);
        printf("Assigment %d:\n", i + 1);

        VariableAssigment* a = node->assigments[i];
        print_indent(indent + 2);
        printf("Name: %s\n", a->name);

        print_indent(indent + 2);
        printf("Static Type: %s\n", a->static_type);

        print_indent(indent + 2);
        printf("Value:\n");
        print_ast_node(a->value, indent + 3);
    }

    print_indent(indent + 1);
    printf("Body:\n");
    print_ast_node(node->body, indent + 2);
}


void print_variable_node(VariableNode* node, int indent) {
    // Imprime un nodo Variable [para DEBUG]
    if (!node) return;

    print_indent(indent);
    printf("VariableNode:\n");

    print_indent(indent + 1);
    printf("Name: %s\n", node->name);

    
    if(node->scope)
    {
        Symbol* var_symbol = lookup_symbol(node->scope, node->name, true);
        print_indent(indent + 1);
        if (var_symbol->type)
        {
            printf("Type: %s\n", var_symbol->type->type_name);
        }
        else
        {
            printf("Type: (Variable not initialized)\n");
        }
    }
}

void print_reassign_node(ReassignNode* node, int indent) {
    // Imprime un nodo de reasignacion [para DEBUG]
    if (!node) return;

    print_indent(indent);
    printf("ReassignNode:\n");

    print_indent(indent + 1);
    printf("Name: %s\n", node->name);

    print_indent(indent + 1);
    printf("Value:\n");
    print_ast_node(node->value, indent + 2);
}

void print_function_definition_node(FunctionDefinitionNode* node, int indent) {
    // Imprime un nodo de Definición de Función [para DEBUG]
    if (!node) return;

    print_indent(indent);
    printf("FunctionDefinitionNode:\n");

    print_indent(indent + 1);
    printf("Name: %s\n", node->name);

    print_indent(indent + 1);
    printf("Parameters (%d):\n", node->param_count);
    for (int i = 0; i < node->param_count; ++i) {
        Param* p = node->params[i];
        print_indent(indent + 2);
        printf("Param %d - Name: %s, Type: %s\n", i + 1, p->name, p->static_type);
    }

    print_indent(indent + 1);
    printf("Static return type: %s\n", node->static_return_type);

    print_indent(indent + 1);
    printf("Body:\n");
    print_ast_node(node->body, indent + 2);
}

void print_function_definition_list_node(FunctionDefinitionListNode *node, int indent) {
    // Imprime un nodo de Lista de Definiciones de Funciones [para DEBUG]
    if (!node) return;

    for (int i = 0; i < node->function_count; i++) {
        print_function_definition_node(node->functions[i], indent);
    }
}

void print_function_call_node(FunctionCallNode* node, int indent) {
    // Imprime un nodo de Llamado de Función [para DEBUG]
    if (!node) return;

    print_indent(indent);
    printf("FunctionCall: %s\n", node->name);
    
    print_indent(indent);
    printf("Return type: %s\n",node->base.return_type->type_name);

    print_indent(indent);
    printf("Arguments (%d) :\n", node->arg_count);

    for (int i = 0; i < node->arg_count; i++) {
        print_ast_node(node->args[i], indent + 1);
    }
}

void print_type_definition_node(TypeDefinitionNode* node, int indent) {
    // Imprime un nodo de Definición de Tipo [para DEBUG]
    if (!node) return;

    print_indent(indent);
    printf("TypeDefinitionNode: %s\n", node->type_name);

    print_indent(indent + 1);
    printf("Parameters (count = %d):\n", node->param_count);
    
    for (int i = 0; i < node->param_count; i++) 
    {
        print_indent(indent + 2);
        printf("Param %d: name = %s, type = %s\n", i, node->params[i]->name, node->params[i]->static_type);
    }

    print_indent(indent + 1);
    printf("Parent Name: %s\n", node->parent_name);

    print_indent(indent + 1);
    printf("Parent Arguments (count = %d):\n", node->parent_arg_count);
    for (int i = 0; i < node->parent_arg_count; i++) 
    {
        print_ast_node(node->parent_args[i], indent + 2);
    }

    print_indent(indent + 1);
    printf("Body:\n");
    print_expression_block_node(node->body, indent + 2);
}

void print_type_definition_list_node(TypeDefinitionListNode* node, int indent) {
    // Imprime un nodo de Lista Definiciones de Tipos [para DEBUG]
    if (!node) return;

    for (int i = 0; i < node->count; i++) {
        print_type_definition_node(node->definitions[i], indent);
    }
}

void print_program_node(ProgramNode* node, int indent) {
    // Imprime un nodo Programa [para DEBUG]
    if (!node) return;

    print_indent(indent);
    printf("ProgramNode:\n");

    print_indent(indent + 1);
    printf("Function Definitions:\n");
    print_function_definition_list_node(node->function_list, indent + 2);

    print_indent(indent + 1);
    printf("Type Definitions:\n");
    print_type_definition_list_node(node->type_definitions, indent + 2);

    print_indent(indent + 1);
    printf("Root Expression:\n");
    print_ast_node(node->root, indent + 2);
}

void print_instanciate_type_node(InstanciateNode* node, int indent_level){
    if(!node) return;

    print_indent(indent_level);
    
    printf("Instance Type %s \n", node->type_name);
    print_indent(indent_level);

    printf("Return Type: %s\n", node->base.return_type);
    print_indent(indent_level);

    printf("Args: %d\n", node->arg_count);
    print_indent(indent_level);

    for (int i = 0; i < node->arg_count; i++)
    {
        print_ast_node(node->args[i], indent_level + 1);
    }
}

void print_func_call_type_node(FuntionCallTypeNode* node, int indent_level)
{
    if(!node) return;
    print_indent(indent_level);

    printf("Function of Type: %s.%s", node->type_name, node->func_name);
    print_indent(indent_level);

    printf("Return Type: %s\n", node->base.return_type);
    print_indent(indent_level);

    printf("Args: %d\n", node->arg_count);
    print_indent(indent_level);

    for (int i = 0; i < node->arg_count; i++)
    {
        print_ast_node(node->args[i], indent_level + 1);
    }
    
}

void print_indent(int indent) {
    for (int i = 0; i < indent + 1; i++) {
        printf("  ");
    }
}