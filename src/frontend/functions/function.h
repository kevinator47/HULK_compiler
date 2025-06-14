#ifndef FUNCTION_H
#define FUNCTION_H

#include <stdlib.h>
#include "../ast/ast.h"
#include "../../backend/codegen/ast_accept.h"

FunctionDefinitionNode* create_predefined_function(char* function_name, Param** params,int param_count, SymbolTable* global_scope, char* return_type);
Param** Init_Params(int count, char** params_names, char** params_types);
void Insert_Function(char* func_name, FunctionDefinitionNode* node, SymbolTable* global_scope);
void Init_Predefined_Functions_One_Param(SymbolTable* global_scope);
void Init_Predefined_Functions_Two_Param(SymbolTable* global_scope);
void Init_Special_Functions(SymbolTable* global_scope);
void Init_Predefined_Functions(SymbolTable* global_scope);
#endif