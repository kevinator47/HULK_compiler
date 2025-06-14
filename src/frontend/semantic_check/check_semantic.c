#include "check_semantic.h"
#include "../hulk_type/type_table.h"
#include "../common/common.h"

TypeDescriptor* check_semantic_literal_node(ASTNode* node) {
    return node->return_type;
}

TypeDescriptor* check_semantic_unary_operation_node(UnaryOperationNode* node) {
    // Verificar el tipo del operando
    ASTNode* operand = node->operand;
    
    // Validar el operador unario
    switch (node->operator) {
        case NOT_TK:
            if (operand->return_type->tag != HULK_Type_Boolean) 
            {
                fprintf(stderr, "Error: Invalid type for operator \"%s\" , expected \"Bool\" but received \"%s\" \n", Hulk_Op_Names[node->operator], operand->return_type->type_name);
                exit(1);
            }
            break;
        case MINUS_TK:
            if (operand->return_type->tag != HULK_Type_Number) 
            {
                fprintf(stderr, "Error: Invalid type for operator \"%s\" , expected \"Number\" but received \"%s\" \n", Hulk_Op_Names[node->operator], operand->return_type->type_name);
                exit(1);
            }
            break;
        default:
            DIE("Unsupported unary operator");
    }

    // Asignar el tipo de retorno del nodo
    node->base.return_type = operand->return_type;

    return node->base.return_type;
}

TypeDescriptor* check_semantic_binary_operation_node(BinaryOperationNode* node, TypeTable* table) {
    // Verificar los tipos de los operandos
    ASTNode* left = node->left;
    ASTNode* right = node->right;

    // Para inferencia de tipos
    TypeDescriptor* number_type = type_table_lookup(table, "Number");
    TypeDescriptor* bool_type = type_table_lookup(table, "Bool");

    // Validar el operador binario
    switch (node->operator) {
        case PLUS_TK:
        case MINUS_TK:
        case MULT_TK:
        case DIV_TK:
        case MOD_TK:
        case EXP_TK:
            if (left->return_type->tag != HULK_Type_Number || right->return_type->tag != HULK_Type_Number) 
            {
                fprintf(stderr, "Error: Invalid types for operator \"%s\" , expected \"Number\" and \"Number\" but received \"%s\" and \"%s\"\n", Hulk_Op_Names[node->operator], left->return_type->type_name, right->return_type->type_name);
                exit(1);
            }
            node->base.return_type = number_type; 
            break;

        case EQ_TK:
        case NE_TK:
        case GT_TK:
        case GE_TK:
        case LT_TK:
        case LE_TK:
            if (left->return_type->tag != HULK_Type_Number || right->return_type->tag != HULK_Type_Number) 
            {
                fprintf(stderr, "Error: Invalid types for operator \"%s\" , expected \"Number\" and \"Number\" but received \"%s\" and \"%s\"\n", Hulk_Op_Names[node->operator], left->return_type->type_name, right->return_type->type_name);
                exit(1);
            }
            node->base.return_type = bool_type;
            break;

        case AND_TK:
        case OR_TK:

            if (left->return_type->tag != HULK_Type_Boolean || right->return_type->tag != HULK_Type_Boolean) 
            {
                fprintf(stderr, "Error: Invalid types for operator \"%s\" , expected \"Bool\" and \"Bool\" but received \"%s\" and \"%s\"\n", Hulk_Op_Names[node->operator], left->return_type->type_name, right->return_type->type_name);
                exit(1);
            }
            node->base.return_type = bool_type;
            break;

        case CONCAT_TK:
        case D_CONCAT_TK:

            if (left->return_type->tag != HULK_Type_String) 
            {
                fprintf(stderr, "Error: Invalid types for operator \"%s\" , expected \"String\" and \"Any\" but received \"%s\" and \"%s\"\n", Hulk_Op_Names[node->operator], left->return_type->type_name, right->return_type->type_name);
                exit(1);;
            }
            node->base.return_type = left->return_type;
            break;
        default:
            DIE("Unsupported binary operator");
    }

    return node->base.return_type;
}

TypeDescriptor* check_semantic_expression_block_node(ExpressionBlockNode* node, TypeTable* table) {
    // Asignar como tipo de retorno el tipo del último nodo del bloque
    if (node->expression_count == 0) 
        node->base.return_type = type_table_lookup(table, "Null");

    else 
        node->base.return_type = node->expressions[node->expression_count - 1]->return_type ;
    
    return node->base.return_type;
}

TypeDescriptor* check_semantic_conditional_node(ConditionalNode* node) {
    // Verificar el tipo de la condición
    ASTNode* condition = node->condition;
    
    if (condition->return_type->tag != HULK_Type_Boolean) 
    {
        fprintf(stderr, "Error: Invalid type for condition on if statement, expected \"Bool\" but received \"%s\" \n", condition->return_type->type_name);
        exit(1);
    }

    // Verificar los tipos de las ramas
    ASTNode* then_branch = node->then_branch;
    ASTNode* else_branch = node->else_branch;

    // Asignar el tipo de retorno del nodo condicional
    if (!else_branch || conforms(then_branch->return_type , else_branch->return_type))
        node->base.return_type = then_branch->return_type;  
    else 
    {
        fprintf(stderr, "Error: Then and Else branches of if-else statetemnt must match, but received \"%s\" and \"%s\"\n", then_branch->return_type->type_name, else_branch->return_type->type_name);
        exit(1);
    }

    return node->base.return_type;
}

TypeDescriptor* check_semantic_while_loop_node(WhileLoopNode* node) {
    // Verificar el tipo de la condición
    ASTNode* condition = node->condition;
    
    if (condition->return_type->tag != HULK_Type_Boolean) 
    {
        fprintf(stderr, "Error: Invalid type for condition on while loop statement, expected \"Bool\" but received \"%s\" \n", condition->return_type->type_name);
        exit(1);
    }

    // Asignar el tipo de retorno del nodo while loop
    node->base.return_type = node->body->return_type;

    return node->base.return_type;
}

TypeDescriptor* check_semantic_let_in_node(LetInNode* node)
{
    // Al ejecutarse esta funcion ya el scope se ha llenado y se ha realizado el chequeo semantico de asignaciones y el cuerpo
    node->base.return_type = node->body->return_type; 
    return node->base.return_type;       
}

TypeDescriptor* check_semantic_variable_assigment_node(VariableAssigmentNode* node, TypeTable* table)
{
    VariableAssigment* assign = node->assigment;

    // Verificar tipos estatico y dinamico son compatibles
    TypeDescriptor* static_type = require_type(table, assign->static_type);

    // Verificar que el valor tenga tipo
    if (!assign->value || !assign->value->return_type) 
    {
        fprintf(stderr, "Error: Invalid return value on assigment to variable '%s'\n", assign->name);
        exit(1);
    }

    TypeDescriptor* dinamyc_type = assign->value->return_type;

    // Verificar compatibilidad de tipos si el tipo estático fue declarado
    if (static_type->tag != HULK_Type_Undefined && !conforms(dinamyc_type, static_type)) {
        fprintf(stderr, "Error: tipo '%s' del valor no es compatible con tipo declarado '%s' en variable '%s'\n", dinamyc_type->type_name, static_type->type_name, assign->name);
        exit(1);
    }

    // Asignar tipo dinamico a la variable si no se declaró estáticamente
    node->base.return_type = static_type->tag == HULK_Type_Undefined ? dinamyc_type : static_type;

    return node->base.return_type;
}

TypeDescriptor* check_semantic_variable_node(VariableNode* node) {
    // Al ejecutarse esta funcion se asume que la referencia al scope ya ha sido asignada
    Symbol* symbol = lookup_symbol(node->scope, node->name, SYMBOL_ANY, true);
    if (!symbol) {
        fprintf(stderr, "Error: uso de variable no declarada '%s'\n", node->name);
        exit(1);
    }
    
    // Asignar el tipo de retorno del nodo variable
    node->base.return_type = symbol->type;

    return node->base.return_type;
}

TypeDescriptor* check_semantic_reassign_node(ReassignNode* node) {
    // Verificar que la variable ya esté declarada
    Symbol* symbol = lookup_symbol(node->scope, node->name, SYMBOL_ANY, true);
    if (!symbol) {
        printf("Reassignment to undeclared variable '%s'", node->name);
        exit(1);
    }

     // Verificar que el símbolo no sea una función ni un tipo
    if (symbol->kind == SYMBOL_FUNCTION || symbol->kind == SYMBOL_TYPE_METHOD) {
        printf("Invalid reassignment: '%s' is not a variable", node->name);
        exit(1);
    }

    // Verificar que el tipo del nuevo valor sea compatible con el tipo de la variable
    if (!conforms(node->value->return_type, symbol->type)) {
        printf("Type mismatch in reassignment to variable '%s': expected '%s', got '%s'", node->name, symbol->type->type_name, node->value->return_type->type_name);
        exit(1);
    }

    // Establecer el tipo de retorno del nodo de reasignación
    node->base.return_type = node->value->return_type;

    return node->base.return_type;
}


TypeDescriptor* check_semantic_function_definition_node(FunctionDefinitionNode* node, TypeTable* table) {
    // Al ejecutarse esta funcion, se asume que el scope ha sido creado, los parametros agregados y el cuerpo ha sido chequeado semánticamente
    TypeDescriptor* body_return_type = node->body->return_type;

    TypeDescriptor* static_return_type = type_table_lookup(table, node->static_return_type);

    if(!static_return_type)
    {
        fprintf(stderr, "Error: Undefined type \"%s\" \n", node->static_return_type);
        exit(1);
    }
    if(!body_return_type)
    {
        fprintf(stderr, "Error: Undefined type \n");
        exit(1);
    }
    
    if (!conforms(body_return_type, static_return_type))
    {
        fprintf(stderr, "Error: Function %s static return type is %s, but it returns %s\n", node->name, node->static_return_type, body_return_type->type_name);
        exit(1);
    }

    return body_return_type;
}

TypeDescriptor* check_semantic_function_call_node(FunctionCallNode* node, SymbolTable* current_scope ) {
    // Al ejecutar este metodo se asume que la definicion de funcion ha sido registrada y los argumentos han sido chequeados semanticamente

    // Find the function definition
    Symbol* func_symbol = lookup_function_by_signature(current_scope, node->name, node->arg_count);
        
    if(!func_symbol) {
        fprintf(stderr, "Error: Undefined function '%s' with %d arguments\n", node->name, node->arg_count);
        exit(1);
    }

    FunctionDefinitionNode* func_def = ((FunctionDefinitionNode*)func_symbol->value);
    
    // Visit each argument and check if it's type is compatible with the one on the declaration
    for (int i = 0; i < node->arg_count; i++)
    {
        TypeDescriptor* arg_type = node->args[i]->return_type ;
        TypeDescriptor* expected_type = lookup_symbol(func_def->scope, func_def->params[i]->name, SYMBOL_PARAMETER ,false)->type;
    
        if(!conforms(arg_type, expected_type))
        {           
            fprintf(stderr, "Type error in argument %d of function '%s': expected '%s', got '%s'\n", i, node->name, expected_type->type_name, arg_type->type_name);
            exit(1);
        }
    }

    // Set func_call return type and return it
    node->base.return_type = func_symbol->type;
    return node->base.return_type;

}

TypeDescriptor* check_semantic_type_definition_node(TypeDefinitionNode* node, TypeTable* table) {
    // TODO : chequear que argumentos del padre coinciden en tipo con sus parametros
    return type_table_lookup(table, "Null");    
}

