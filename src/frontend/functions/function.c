#include "function.h"
FunctionDefinitionNode* create_predefined_function(char* function_name, Param** params,int param_count, 
    SymbolTable* global_scope, char* return_type)
{
    FunctionDefinitionNode* node = malloc(sizeof(FunctionDefinitionNode));
    node->base.type = AST_Node_Function_Definition;
    node->base.accept = generic_ast_accept;
    node->base.return_type = NULL;

    node->body = NULL;
    node->name = function_name;
    node->params = params;
    node->param_count = param_count;
    node->scope = global_scope;
    node->static_return_type = return_type;

    return node;
}
Param** Init_Params(int count, char** params_names, char** params_types)
{
    Param** result = malloc(sizeof(Param *) * count);
    for (int i = 0; i < count; i++)
    {
        result[i] = malloc(sizeof(Param));
        result[i]->name = params_names[i];
        result[i]->static_type = params_types[i];
    }
    return result;
}
void Insert_Function(char* func_name, FunctionDefinitionNode* node, SymbolTable* global_scope)
{
    Symbol* s = malloc(sizeof(Symbol));
    s->name = func_name;
    s->kind = SYMBOL_FUNCTION;
    s->value = node;
    s->type = NULL;
    insert_symbol(global_scope, s);
}
void Init_Predefined_Functions_One_Param(SymbolTable* global_scope)
{
    int count = 5;
    char *one_params_aritmetic_function[] =  {"sqrt", "sin", "cos", "exp", "log"};

    for (int i = 0; i < count; i++)
    {
        char* name = "x";
        char* type = "Number";
        Param** param = Init_Params(1, &name, &type);
        FunctionDefinitionNode* func = create_predefined_function(one_params_aritmetic_function[i], param, 1, global_scope, "Number");
        Insert_Function(one_params_aritmetic_function[i], func, global_scope);
    }

    count = 2;
    char *one_params_str_function[] = {"strlen", "puts"};
    for (int i = 0; i < count; i++)
    {
        Param** param = Init_Params(1, "str", "String");
        FunctionDefinitionNode* func = create_predefined_function(one_params_str_function[i], param, 1, global_scope, "Number");
        Insert_Function(one_params_str_function[i], func, global_scope);
    }
}
void Init_Predefined_Functions_Two_Param(SymbolTable* global_scope)
{
    int count = 2;
    char* two_params_aritmetic_function[] = {"pow", "fmod"};
    for (int i = 0; i < count; i++)
    {
        char* params_names[] = {"x", "y"};
        char* params_types[] = {"Number", "Number"};
        Param** params = Init_Params(2, params_names, params_types);
        FunctionDefinitionNode* func = create_predefined_function(two_params_aritmetic_function[i], params, 2, global_scope, "Number");
        Insert_Function(two_params_aritmetic_function[i], func, global_scope);
    }
}
void Init_Special_Functions(SymbolTable* global_scope)
{
    FunctionDefinitionNode* func_rand = create_predefined_function("rand", NULL, 0, global_scope, "Number");
    Insert_Function("rand", func_rand, global_scope);

    Param** param = Init_Params(1, "code", "Number");
    FunctionDefinitionNode* func_exit = create_predefined_function("exit", param, 1, global_scope, NULL);
    Insert_Function("exit", func_exit, global_scope);
}
void Init_Predefined_Functions(SymbolTable* global_scope)
{
    Init_Predefined_Functions_One_Param(global_scope);
    Init_Predefined_Functions_Two_Param(global_scope);
    Init_Special_Functions(global_scope);
}