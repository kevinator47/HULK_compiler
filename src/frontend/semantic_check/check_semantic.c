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
            
            if (left->return_type->tag != HULK_Type_Number || right->return_type->tag != HULK_Type_Number) {
                DIE("Invalid types for comparison operator, expected numbers");
            }
            node->base.return_type = bool_type;
            break;

        case AND_TK:
        case OR_TK:

            if (left->return_type->tag != HULK_Type_Boolean || right->return_type->tag != HULK_Type_Boolean) {
                DIE("Invalid types for logical operator, expected booleans");
            }
            node->base.return_type = bool_type;
            break;

        case CONCAT_TK:
        case D_CONCAT_TK:

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
        node->base.return_type = type_table_lookup(table, "Null");
    }
    else {
        node->base.return_type = node->expressions[node->expression_count - 1]->return_type;
    }
    return node->base.return_type;
}

TypeDescriptor* check_semantic_conditional_node(ConditionalNode* node) {
    // Verificar el tipo de la condición
    ASTNode* condition = node->condition;
    
    if (condition->return_type->tag != HULK_Type_Boolean) {
        DIE("Invalid type for condition, expected boolean");
    }

    // Verificar los tipos de las ramas
    ASTNode* then_branch = node->then_branch;
    ASTNode* else_branch = node->else_branch;

    // Asignar el tipo de retorno del nodo condicional
    if (!else_branch || conforms(then_branch->return_type , else_branch->return_type)) {
        node->base.return_type = then_branch->return_type; 
    } else {
        printf("Then: %d, Else: %d\n", then_branch->return_type->tag, else_branch->return_type->tag);
        DIE("Types of THEN and ELSE branches do not match");
    }

    return node->base.return_type;
}

TypeDescriptor* check_semantic_while_loop_node(WhileLoopNode* node) {
    // Verificar el tipo de la condición
    ASTNode* condition = node->condition;
    
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
    insert_symbol(node->scope, create_symbol(node->name, SYMBOL_VARIABLE, node->value->return_type, node->value));

    // Asignar el tipo de retorno del nodo de reasignación]
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
        TypeDescriptor* expected_type = lookup_symbol(func_def->scope, func_def->params[i]->name, false)->type;
    
        if(!conforms(arg_type, expected_type))
        {           
            fprintf(stderr, "Type error in argument %d of function '%s'\n", i, node->name);
            exit(1);
        }
    }

    // Set func_call return type and return it
    node->base.return_type = func_symbol->type;
    return node->base.return_type;

}

TypeDescriptor* check_semantic_type_definition_node(TypeDefinitionNode* node, TypeTable* table) {
    
    // Paso 1: Validar los argumentos al padre
    TypeDescriptor* parent_type = type_table_lookup(table, node->parent_name);
    if (!parent_type || !parent_type->initializated || (parent_type->tag == HULK_Type_UserDefined && !parent_type->info)) {
        fprintf(stderr, "[Semantic Error] Type '%s' inherits from unknown or uninitialized type '%s'\n", node->type_name, node->parent_name);
        exit(1);
    }
    
    if (parent_type->tag != HULK_Type_UserDefined)
    {
        if(node->parent_arg_count != 0)
        {
            fprintf(stderr, "[Semantic Error] Builtin type '%s' cannot receive arguments'\n", parent_type->type_name);
            exit(1);    
        }
    }
    else if (node->parent_arg_count != parent_type->info->param_count) {
        fprintf(stderr, "[Semantic Error] Type '%s' inherits from '%s' with incorrect number of arguments (%d expected, %d given)\n",
        node->type_name, node->parent_name,
        parent_type->info->param_count, node->parent_arg_count);
        exit(1);
    }
        
    // Paso 2: Validar tipos de los argumentos heredados
    for (int i = 0; i < node->parent_arg_count; i++) { 
        char* arg_name = parent_type->info->params_name[i];
        SymbolTable* parent_scope = parent_type->info->scope;
        Symbol* field_symbol = lookup_symbol_type_field(parent_scope, arg_name, true);

        TypeDescriptor* expected = field_symbol->type;
        TypeDescriptor* given = node->parent_args[i]->return_type;

        if (!conforms(given, expected)) {
            fprintf(stderr, "[Semantic Error] Inheritance argument %d in type '%s' expects type '%s' but got '%s'\n",
            i, node->type_name,
            expected->type_name, given->type_name);
            exit(1);
        }
    }
    return type_table_lookup(table, "Null");    
}

TypeDescriptor* check_semantic_type_instanciate_node(InstanciateNode* node, TypeTable* table) {
    TypeDescriptor* type = type_table_lookup(table, node->type_name);
    if (!type)
    {
        fprintf(stderr, "The type %s does'nt exists \n", node->type_name);
        exit(1);
    }
    if (type->tag != HULK_Type_UserDefined)
    {
        fprintf(stderr, "Error: Cannot instanciate builtin-type \"%s\" \n", type->type_name);
        exit(1);
    }
    if(type->info->param_count != node->arg_count)
    {
        fprintf(stderr, "The type %s not wait for %d params \n", type->type_name, node->arg_count);
        exit(1);
    }

    for (int i = 0; i < node->arg_count; i++)
    {
        TypeDescriptor* arg_type = node->args[i]->return_type;
        TypeDescriptor* expected_type = lookup_symbol(type->info->scope, type->info->params_name[i], false)->type;
        if (!conforms(arg_type, expected_type))
        {
            fprintf(stderr, "Type error in argument %d of instance '%s'\n", i, node->type_name);
            exit(1);
        }
    }
    node->base.return_type = type;
    return node->base.return_type;
}

void add_assigment_to_scope(SymbolTable* scope, VariableAssigment* assigment, TypeTable* table) {
    Symbol* symbol ;    // symbol that will be inserted into the scope

    TypeDescriptor* dinamyc_type = assigment->value->return_type;
    printf("%s\n", assigment->static_type);
    TypeDescriptor* static_type = type_table_lookup(table, assigment->static_type);

    
    if(static_type->tag == HULK_Type_Undefined)
    {
        symbol = create_symbol(assigment->name, SYMBOL_VARIABLE, dinamyc_type, assigment->value);
    }
    
    else if (conforms(dinamyc_type, static_type))
    {
        symbol = create_symbol(assigment->name, SYMBOL_VARIABLE, static_type, assigment->value);
    }

    else
    {
        fprintf(stderr , "Error: Variable \"%s\" with static type \"%s\" cannot hold \"%s\" type \n", assigment->name, assigment->static_type, dinamyc_type->type_name );
        exit(1);
    }

    // Insertar el símbolo en el scope
    insert_symbol(scope, symbol);
}

void register_function_definition(FunctionDefinitionNode* node, SymbolTable* scope, TypeTable* table) {
    // Crear un nuevo símbolo para la definición de la función
    // TODO: inferencia de tipo para determinar tipo de retorno de la funcion(algortimo de unificacion)
    TypeDescriptor* static_return_type = type_table_lookup(table, node->static_return_type);

    if(!static_return_type)
    {
        fprintf(stderr, "Error: Undefined type \"%s\" \n", node->static_return_type);
        exit(1);
    }
    Symbol* symbol = create_symbol(node->name, SYMBOL_FUNCTION, static_return_type, (ASTNode*)node);
    
    // Insertar el símbolo en el scope
    insert_symbol(scope, symbol);
}
