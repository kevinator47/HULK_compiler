#ifndef FUNCTION_TABLE_H
#define FUNCTION_TABLE_H

#include "ast.h"
#include <stdbool.h>

typedef struct {
    char* name;
    char** parameters;    // Array de nombres de parámetros
    int param_count;
    ASTNode* body;       // Expresión o bloque que forma el cuerpo
    bool is_block;       // true si el cuerpo es un bloque, false si es una expresión
} Function;

typedef struct {
    Function** functions;
    int count;
    int capacity;
} FunctionTable;

// Crear y destruir la tabla
FunctionTable* create_function_table(int initial_capacity);
void free_function_table(FunctionTable* ft);

// Operaciones sobre la tabla
bool register_function(FunctionTable* ft, const char* name, char** parameters, 
                      int param_count, ASTNode* body, bool is_block);
Function* lookup_function(FunctionTable* ft, const char* name);

#endif