#ifndef FUNCTION_H
#define FUNCTION_H

#include <stdlib.h>
#include "ast.h"
#include "hulk_type/type_table.h"
#include "../../backend/codegen/ast_accept.h"

// Prototypes
void register_predefined_functions(SymbolTable* global_scope, TypeTable* type_table);
void register_predefined_functions_one_param(SymbolTable* global_scope, TypeTable* type_table);
void register_predefined_functions_two_params(SymbolTable* global_scope, TypeTable* type_table);
void register_special_functions(SymbolTable* global_scope, TypeTable* type_table);
Param** create_predefined_function_params(char** params_names, char** params_types, int count);
FunctionDefinitionNode* create_predefined_function(char* function_name, Param** params,int param_count, SymbolTable* global_scope, char* return_type, TypeTable* type_table);
void insert_function(char* func_name, FunctionDefinitionNode* node, SymbolTable* global_scope, TypeDescriptor* return_type);
#endif