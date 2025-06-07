#include "check_semantic.h"
#include "../hulk_type/type_table.h"
#include "../common/common.h"

TypeDescriptor* check_semantic_literal_node(ASTNode* node) {
    return node->return_type;
}

TypeDescriptor* check_semantic_unary_operation_node(UnaryOperationNode* node, TypeTable* table) {
    // Verificar el tipo del operando
    ASTNode* operand = node->operand;
    
    // Validar el operador unario
    switch (node->operator) {
        case NOT_TK:
            type_inference(operand, type_table_lookup(table, "bool"));
            if (operand->return_type->tag != HULK_Type_Boolean) {
                DIE("Invalid type for NOT operator, expected boolean");
            }
            break;
        case MINUS_TK:
            if (operand->return_type->tag != HULK_Type_Number) {
                DIE("Invalid type for MINUS operator, expected number");
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
    TypeDescriptor* number_type = type_table_lookup(table, "number");
    TypeDescriptor* bool_type = type_table_lookup(table, "bool");

    // Validar el operador binario
    switch (node->operator) {
        case PLUS_TK:
        case MINUS_TK:
        case MULT_TK:
        case DIV_TK:
        case MOD_TK:
        case EXP_TK:
            type_inference(left, number_type);
            type_inference(right, number_type);
            
            if (left->return_type->tag != HULK_Type_Number || right->return_type->tag != HULK_Type_Number) {
                printf("%d\n",left->return_type->tag);
                DIE("Invalid types for arithmetic operator, expected numbers");
            }
            node->base.return_type = number_type; 
            break;

        case EQ_TK:
        case NE_TK:
        case GT_TK:
        case GE_TK:
        case LT_TK:
        case LE_TK:
            type_inference(left, number_type);
            type_inference(right, number_type);
            
            if (left->return_type->tag != HULK_Type_Number || right->return_type->tag != HULK_Type_Number) {
                DIE("Invalid types for comparison operator, expected numbers");
            }
            node->base.return_type = bool_type;
            break;

        case AND_TK:
        case OR_TK:
            type_inference(left, bool_type);
            type_inference(right, bool_type);

            if (left->return_type->tag != HULK_Type_Boolean || right->return_type->tag != HULK_Type_Boolean) {
                DIE("Invalid types for logical operator, expected booleans");
            }
            node->base.return_type = bool_type;
            break;

        case CONCAT_TK:
        case D_CONCAT_TK:
            type_inference(left, type_table_lookup(table, "string"));

            if (left->return_type->tag != HULK_Type_String) {
                DIE("Invalid types for concatenation operator, expected string on the left");
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
    if (node->expression_count == 0) {
        node->base.return_type = type_table_lookup(table, "null");
    }
    else {
        node->base.return_type = node->expressions[node->expression_count - 1]->return_type;
    }
    return node->base.return_type;
}

TypeDescriptor* check_semantic_conditional_node(ConditionalNode* node, TypeTable* table) {
    // Verificar el tipo de la condición
    ASTNode* condition = node->condition;

    TypeDescriptor* bool_type = type_table_lookup(table, "bool");
    
    type_inference(condition, bool_type);
    if (condition->return_type->tag != HULK_Type_Boolean) {
        DIE("Invalid type for condition, expected boolean");
    }

    // Verificar los tipos de las ramas
    ASTNode* then_branch = node->then_branch;
    ASTNode* else_branch = node->else_branch;

    // Inferencia de tipos, una rama es capaz de inferir el tipo si se conoce el de la otra
    if(else_branch)
    {
        if(else_branch->return_type->tag != HULK_Type_Any)
            type_inference(then_branch, else_branch->return_type);  // el then infiere su tipo gracias al else

        else if (then_branch->return_type->tag != HULK_Type_Any)
        {
            printf("Infereing else type..\n");
            type_inference(else_branch, then_branch->return_type);  // el else infiere del then
        }
        
    }
    // Asignar el tipo de retorno del nodo condicional
    if (else_branch->return_type->tag == HULK_Type_Null || then_branch->return_type->tag == else_branch->return_type->tag ) {
        node->base.return_type = then_branch->return_type; 
    } else {
        printf("Then: %d, Else: %d\n", then_branch->return_type->tag, else_branch->return_type->tag);
        DIE("Types of THEN and ELSE branches do not match");
    }

    return node->base.return_type;
}

TypeDescriptor* check_semantic_while_loop_node(WhileLoopNode* node, TypeTable* table) {
    // Verificar el tipo de la condición
    ASTNode* condition = node->condition;
    type_inference(condition, type_table_lookup(table,"bool"));
    
    if (condition->return_type->tag != HULK_Type_Boolean) {
        DIE("Invalid type for while loop condition, expected boolean");
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

TypeDescriptor* check_semantic_variable_node(VariableNode* node) {
    // Al ejecutarse esta funcion se asume que la referencia al scope ya ha sido asignada
    Symbol* symbol = lookup_symbol(node->scope, node->name, true);
    if (!symbol) {
        DIE("Use of undeclared variable");
    }
    
    // Asignar el tipo de retorno del nodo variable
    node->base.return_type = symbol->type;

    return node->base.return_type;
}

TypeDescriptor* check_semantic_reassign_node(ReassignNode* node) {
    // Verificar que la variable ya esté declarada
    Symbol* symbol = lookup_symbol(node->scope, node->name, true);
    if (!symbol) {
        DIE("Reassignment to undeclared variable");
    }

    // Insertar el simbolo con el nuevo valor(sobreescribiendo el valor y tipo anterior)
    insert_symbol(node->scope, create_symbol(node->name, SYMBOL_VARIABLE, node->value->return_type, node->value, NULL, 0));

    // Asignar el tipo de retorno del nodo de reasignación]
    node->base.return_type = node->value->return_type;

    return node->base.return_type;
}

TypeDescriptor *check_semantic_function_definition_node(FunctionDefinitionNode* node, TypeTable* table) {
    // Al ejecutarse esta funcion, se asume que el scope ha sido creado, los parametros agregados y el cuerpo ha sido chequeado semánticamente
    TypeDescriptor* return_type = node->body->return_type;
    
    // Asignar como tipo de retorno el tipo del cuerpo de la función
    if(return_type->tag != HULK_Type_Undefined) {
        node->base.return_type = return_type;
    }
    else {
        // El tipo es undefined si se retorna el llamado una funcion que no se ha chequeado semánticamente
        // TODO: Manejar ese caso realizando chequeos semanticos recursivos evitando caer en un ciclo infinito
        // Por ahora se asigna el tipo any para evitar errores de compilación
        node->base.return_type = type_table_lookup(table, "any");
    }
    return node->base.return_type;   
}

void add_assigment_to_scope(SymbolTable* scope, VariableAssigment* assigment) {
    // Crear un nuevo símbolo para la asignación
    TypeDescriptor* type = assigment->value->return_type;
    Symbol* symbol = create_symbol(assigment->name, SYMBOL_VARIABLE, type, assigment->value, NULL, 0);
    
    // Insertar el símbolo en el scope
    insert_symbol(scope, symbol);
}

void register_function_definition(FunctionDefinitionNode* node, SymbolTable* scope) {
    // Crear un nuevo símbolo para la definición de la función
    Symbol* symbol = create_symbol(node->name, SYMBOL_FUNCTION, node->body->return_type, (ASTNode*)node, node->params_names, node->param_count);
    
    // Insertar el símbolo en el scope
    insert_symbol(scope, symbol);
}

void type_inference(ASTNode* node, TypeDescriptor* expected_type) {
    if(node->type == AST_Node_Variable)
    {
        VariableNode* variable = (VariableNode*) node;
        TypeDescriptor* dinamyc_type = lookup_symbol(variable->scope,variable->name, true)->type ;
        if(dinamyc_type->tag == HULK_Type_Any)
        {
            variable->base.return_type = expected_type;
            set_symbol_return_type(variable->scope, variable->name, expected_type);
        }
        else if (variable->base.return_type->tag == HULK_Type_Any)
        {
            variable->base.return_type = dinamyc_type;
        }

    }
    // TODO : Add inference for function call
    if(node->type == AST_Node_Function_Call)
    {
        printf("Infereing function call type...\n");
        FunctionCallNode* func_call = (FunctionCallNode*) node;
        printf("Func Call type: %d\n", func_call->base.return_type->tag);

        if(func_call->base.return_type->tag == HULK_Type_Any || func_call->base.return_type->tag == HULK_Type_Undefined)
        {
            func_call->base.return_type = expected_type;
        }

    }    
}

