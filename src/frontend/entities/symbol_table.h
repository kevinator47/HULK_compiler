#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "../common/common.h"
#include <stdbool.h>

// Estructura para un símbolo (variable)
typedef struct Symbol {
    char* name;
    ASTNode* expression;    // AST de la expresión asignada
    struct Symbol* next;    // Para manejar colisiones
} Symbol;

// Estructura para la tabla de símbolos
typedef struct SymbolTable {
    Symbol** table;
    int size;
    struct SymbolTable* parent;
} SymbolTable;

// Funciones para manejar la tabla de símbolos
SymbolTable* create_symbol_table(int size, SymbolTable* parent);
void free_symbol_table(SymbolTable* st);

// Funciones para manejar símbolos
bool insert_symbol(SymbolTable* st, const char* name, ASTNode* expression);
Symbol* lookup_symbol(SymbolTable* st, const char* name);

#endif