#include "function.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void register_predefined_functions(SymbolTable* global_scope, TypeTable* type_table) {
    register_predefined_functions_one_param(global_scope, type_table);
    register_predefined_functions_two_params(global_scope, type_table);
    register_special_functions(global_scope, type_table);
}

void register_predefined_functions_one_param(SymbolTable* global_scope, TypeTable* type_table) {
    int count = 5;
    char *one_params_aritmetic_function[] = {"sqrt", "sin", "cos", "exp", "log"};

    for (int i = 0; i < count; i++) {
        char* name = "x";
        char* type = "Number";
        Param** param = create_predefined_function_params(&name, &type, 1);
        FunctionDefinitionNode* func = create_predefined_function(one_params_aritmetic_function[i], param, 1, global_scope, "Number", type_table);
        insert_function(one_params_aritmetic_function[i], func, global_scope, type_table_lookup(type_table, "Number"));
    }

    count = 2;
    char *one_params_str_function[] = {"strlen", "puts"};
    for (int i = 0; i < count; i++) {
        char* name = "str";
        char* type = "String";
        Param** param = create_predefined_function_params(&name, &type, 1);
        FunctionDefinitionNode* func = create_predefined_function(one_params_str_function[i], param, 1, global_scope, "Number", type_table);
        insert_function(one_params_str_function[i], func, global_scope, type_table_lookup(type_table, "Number"));
    }
}

void register_predefined_functions_two_params(SymbolTable* global_scope, TypeTable* type_table) {
    int count = 2;
    char* two_params_aritmetic_function[] = {"pow", "fmod"};
    for (int i = 0; i < count; i++) {
        char* params_names[] = {"x", "y"};
        char* params_types[] = {"Number", "Number"};
        Param** params = create_predefined_function_params(params_names, params_types, 2);
        FunctionDefinitionNode* func = create_predefined_function(two_params_aritmetic_function[i], params, 2, global_scope, "Number", type_table);
        insert_function(two_params_aritmetic_function[i], func, global_scope, type_table_lookup(type_table, "Number"));
    }
}

void register_special_functions(SymbolTable* global_scope, TypeTable* type_table) {
    FunctionDefinitionNode* func_rand = create_predefined_function("rand", NULL, 0, global_scope, "Number", type_table);
    insert_function("rand", func_rand, global_scope, type_table_lookup(type_table, "Number"));

    char* name = "code";
    char* type = "Number";
    Param** param = create_predefined_function_params(&name, &type, 1);
    FunctionDefinitionNode* func_exit = create_predefined_function("exit", param, 1, global_scope, NULL, type_table);
    insert_function("exit", func_exit, global_scope, type_table_lookup(type_table, "Null"));
}

Param** create_predefined_function_params(char** params_names, char** params_types, int count) {
    Param** result = malloc(sizeof(Param *) * count);
    for (int i = 0; i < count; i++) {
        result[i] = malloc(sizeof(Param));
        result[i]->name = params_names[i];
        result[i]->static_type = params_types[i];
    }
    return result;
}

FunctionDefinitionNode* create_predefined_function(char* function_name, Param** params, int param_count, SymbolTable* global_scope, char* return_type, TypeTable* type_table) {
    FunctionDefinitionNode* node = malloc(sizeof(FunctionDefinitionNode));
    node->base.type = AST_Node_Function_Definition;
    node->base.accept = generic_ast_accept;
    node->base.return_type = type_table_lookup(type_table, "Null");

    node->body = NULL;
    node->name = function_name;
    node->params = params;
    node->param_count = param_count;
    node->scope = create_symbol_table(global_scope);
    node->static_return_type = return_type;

    for (int i = 0; i < param_count; i++) {
        Symbol* param_symbol = malloc(sizeof(Symbol));
        param_symbol->name = params[i]->name;
        param_symbol->kind = SYMBOL_PARAMETER;
        param_symbol->type = type_table_lookup(type_table, params[i]->static_type);
        param_symbol->value = NULL;
        insert_symbol(node->scope, param_symbol);
    }

    return node;
}

void insert_function(char* func_name, FunctionDefinitionNode* node, SymbolTable* global_scope, TypeDescriptor* return_type) {
    Symbol* s = malloc(sizeof(Symbol));
    s->name = func_name;
    s->kind = SYMBOL_FUNCTION;
    s->value = node;
    s->type = return_type;
    insert_symbol(global_scope, s);
}
