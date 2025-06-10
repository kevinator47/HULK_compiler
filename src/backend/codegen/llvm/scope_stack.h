#ifndef SCOPE_STACK_H
#define SCOPE_STACK_H

#include <stddef.h>
#include "ir_symbol_table.h"

// Pila de tablas de símbolos (scopes anidados)
typedef struct ScopeStack {
    IrSymbolTable** tables; // Arreglo dinámico de punteros a SymbolTable
    size_t capacity;
    size_t size;
} ScopeStack;

ScopeStack* create_scope_stack();
void destroy_scope_stack(ScopeStack* stack);
void push_scope(ScopeStack* stack, IrSymbolTable* table);
void pop_scope(ScopeStack* stack);
IrSymbolTable* current_scope(ScopeStack* stack);

#endif // SCOPE_STACK_H