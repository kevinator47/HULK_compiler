#include "semantic_visitor.h"

void register_globals(ProgramNode* node, SymbolTable* current_scope, TypeTable* type_table) {
    register_types(node->type_definitions, current_scope, type_table);
    register_functions(node->function_list, current_scope, type_table);
}

void register_types(TypeDefinitionListNode* list, SymbolTable* current_scope, TypeTable* type_table) {
    // Primera Pasada: agregar los tipos a la tabla de símbolos
    for (int i = 0; i < list->count; i++) {
        TypeDefinitionNode* type_def_node = list->definitions[i];
        add_user_defined_type(type_table, type_def_node, "Object", current_scope);
    }

    // Segunda Pasada: registrar parámetros y campos de cada tipo
    for (int i = 0; i < list->count; i++) {
        TypeDefinitionNode* type_def_node = list->definitions[i];
        TypeDescriptor* descriptor = require_type(type_table, type_def_node->type_name);
        
        if (!descriptor->info || !descriptor->info->scope) {
            fprintf(stderr, "Error interno: El tipo '%s' no tiene información válida (posible error en add_user_defined_type)\n", type_def_node->type_name);
            exit(1);
        }

        type_def_node->scope = descriptor->info->scope;

        // Registrar parámetros en el scope del tipo
        register_params(type_def_node, type_def_node->scope, type_table);

        // Registrar atributos y métodos en el scope del tipo
        register_fields(type_def_node, type_def_node->scope, type_table);
    }
}

void register_params(TypeDefinitionNode* type_def_node, SymbolTable* type_scope, TypeTable* type_table) {
    for (int i = 0; i < type_def_node->param_count; i++) {
        Param* param = type_def_node->params[i];

        if (strcmp(param->name, "self") == 0) {
            fprintf(stderr, "Error: 'self' is not a valid parameter name because it causes ambiguity on '%s' type \n", type_def_node->type_name);
            exit(1);
        }

        TypeDescriptor* param_type = require_type(type_table, param->static_type);

        Symbol* param_symbol = create_symbol(param->name, SYMBOL_PARAMETER, param_type, NULL);
        insert_symbol(type_scope, param_symbol);
    }
}

void register_fields(TypeDefinitionNode* type_def_node, SymbolTable* type_scope, TypeTable* type_table) {
    for (int i = 0; i < type_def_node->body->expression_count; i++) {
        ASTNode* node = type_def_node->body->expressions[i];

        switch (node->type) {
            case AST_Node_Variable_Assigment: {
                VariableAssigmentNode* assign_node = (VariableAssigmentNode*)node;
                
                TypeDescriptor* field_type = require_type(type_table, assign_node->assigment->static_type);

                Symbol* field_symbol = create_symbol(assign_node->assigment->name, SYMBOL_TYPE_FIELD, field_type, NULL);
                insert_symbol(type_scope, field_symbol);
                break;
            }
            case AST_Node_Function_Definition: {
                FunctionDefinitionNode* func_node = (FunctionDefinitionNode*)node;
                TypeDescriptor* func_rtn_type = require_type(type_table, func_node->static_return_type);

                Symbol* method_symbol = create_symbol(func_node->name, SYMBOL_TYPE_METHOD, func_rtn_type, node);
                insert_symbol(type_scope, method_symbol);

                // Crear un nuevo scope para la función
                SymbolTable* function_scope = create_symbol_table(type_def_node->scope);
                func_node->scope = function_scope;

                register_function_params(func_node, type_table);

                break;
            }
            default:
                fprintf(stderr, "Advertencia: nodo inesperado en el cuerpo del tipo '%s'\n",
                        type_def_node->type_name);
                break;
        }
    }
}

void register_functions(FunctionDefinitionListNode* list, SymbolTable* global_scope, TypeTable* type_table) {
    for (int i = 0; i < list->function_count; i++) {
        FunctionDefinitionNode* func_node = list->functions[i];

        register_prototype(func_node, global_scope, type_table);

        SymbolTable* function_scope = create_symbol_table(global_scope);
        func_node->scope = function_scope;

        register_function_params(func_node, type_table);
    }
}

void register_prototype(FunctionDefinitionNode* func_node, SymbolTable* scope, TypeTable* type_table) {
    TypeDescriptor* return_type = require_type(type_table, func_node->static_return_type);

    Symbol* function_symbol = create_symbol(func_node->name, SYMBOL_FUNCTION, return_type, (ASTNode*)func_node);
    insert_symbol(scope, function_symbol);
}

void register_function_params(FunctionDefinitionNode* node, TypeTable* table) {
    for (int i = 0; i < node->param_count; i++) {
        char* param_name = node->params[i]->name;
        TypeDescriptor* param_type = require_type(table, node->params[i]->static_type);

        insert_symbol(node->scope, create_symbol(param_name, SYMBOL_PARAMETER, param_type, NULL));
    }
}
