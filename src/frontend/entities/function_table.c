#include "function_table.h"
#include <stdlib.h>
#include <string.h>

FunctionTable* create_function_table(int initial_capacity) {
    FunctionTable* ft = malloc(sizeof(FunctionTable));
    if (!ft) return NULL;

    ft->functions = malloc(sizeof(Function*) * initial_capacity);
    if (!ft->functions) {
        free(ft);
        return NULL;
    }

    ft->count = 0;
    ft->capacity = initial_capacity;
    return ft;
}

bool register_function(FunctionTable* ft, const char* name, char** parameters, 
                      int param_count, ASTNode* body, bool is_block) {
    // Verificar si la función ya existe
    for (int i = 0; i < ft->count; i++) {
        if (strcmp(ft->functions[i]->name, name) == 0) {
            return false;
        }
    }

    // Crear nueva función
    Function* func = malloc(sizeof(Function));
    if (!func) return false;

    func->name = strdup(name);
    func->parameters = malloc(sizeof(char*) * param_count);
    for (int i = 0; i < param_count; i++) {
        func->parameters[i] = strdup(parameters[i]);
    }
    func->param_count = param_count;
    func->body = body;
    func->is_block = is_block;

    // Expandir la tabla si es necesario
    if (ft->count >= ft->capacity) {
        int new_capacity = ft->capacity * 2;
        Function** new_functions = realloc(ft->functions, sizeof(Function*) * new_capacity);
        if (!new_functions) {
            free(func->name);
            free(func->parameters);
            free(func);
            return false;
        }
        ft->functions = new_functions;
        ft->capacity = new_capacity;
    }

    ft->functions[ft->count++] = func;
    return true;
}

Function* lookup_function(FunctionTable* ft, const char* name) {
    for (int i = 0; i < ft->count; i++) {
        if (strcmp(ft->functions[i]->name, name) == 0) {
            return ft->functions[i];
        }
    }
    return NULL;
}

void free_function_table(FunctionTable* ft) {
    if (!ft) return;

    for (int i = 0; i < ft->count; i++) {
        Function* func = ft->functions[i];
        free(func->name);
        for (int j = 0; j < func->param_count; j++) {
            free(func->parameters[j]);
        }
        free(func->parameters);
        free(func);
    }
    free(ft->functions);
    free(ft);
}