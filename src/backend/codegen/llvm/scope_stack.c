#include "scope_stack.h"
#include <stdlib.h>

#define INITIAL_CAPACITY 8

ScopeStack* create_scope_stack() {
    ScopeStack* stack = (ScopeStack*)malloc(sizeof(ScopeStack));
    if (!stack) return NULL;
    stack->capacity = INITIAL_CAPACITY;
    stack->size = 0;
    stack->tables = (IrSymbolTable**)malloc(sizeof(IrSymbolTable*) * stack->capacity);
    return stack;
}

void destroy_scope_stack(ScopeStack* stack) {
    if (!stack) return;
    for (size_t i = 0; i < stack->size; ++i) {
        free_ir_symbol_table(stack->tables[i]);
    }
    free(stack->tables);
    free(stack);
}

void push_scope(ScopeStack* stack, IrSymbolTable* table) {
    if (!stack || !table) return;
    if (stack->size == stack->capacity) {
        stack->capacity *= 2;
        stack->tables = (IrSymbolTable**)realloc(stack->tables, sizeof(IrSymbolTable*) * stack->capacity);
    }
    stack->tables[stack->size++] = table;
}

void pop_scope(ScopeStack* stack) {
    if (!stack || stack->size == 0) return;
    free_ir_symbol_table(stack->tables[--stack->size]);
}

IrSymbolTable* current_scope(ScopeStack* stack) {
    if (!stack || stack->size == 0) return NULL;
    return stack->tables[stack->size - 1];
}