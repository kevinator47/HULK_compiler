#include "symbol_table.h"
#include <stdlib.h>
#include <string.h>

// Función simple de hash para strings
static unsigned int hash(const char* str, int size) {
    unsigned int hash = 0;
    while (*str) {
        hash = hash * 31 + *str++;
    }
    return hash % size;
}

SymbolTable* create_symbol_table(int size, SymbolTable* parent) {
    SymbolTable* st = malloc(sizeof(SymbolTable));
    if (!st) return NULL;

    st->table = calloc(size, sizeof(Symbol*));
    if (!st->table) {
        free(st);
        return NULL;
    }

    st->size = size;
    st->parent = parent;
    return st;
}

bool insert_symbol(SymbolTable* st, const char* name, ASTNode* expression) {
    unsigned int index = hash(name, st->size);
    
    // Verificar si ya existe en el scope actual
    Symbol* current = st->table[index];
    while (current) {
        if (strcmp(current->name, name) == 0) {
            // Actualizar el valor del símbolo existente
            current->expression = expression;
            return true;
        }
        current = current->next;
    }

    // Crear nuevo símbolo
    Symbol* symbol = malloc(sizeof(Symbol));
    if (!symbol) return false;

    symbol->name = strdup(name);
    symbol->expression = expression;
    
    // Insertar al inicio de la lista
    symbol->next = st->table[index];
    st->table[index] = symbol;

    return true;
}

Symbol* lookup_symbol(SymbolTable* st, const char* name) {
    SymbolTable* current = st;
    
    // Buscar en el scope actual y scopes padres
    while (current) {
        unsigned int index = hash(name, current->size);
        Symbol* symbol = current->table[index];
        
        while (symbol) {
            if (strcmp(symbol->name, name) == 0) {
                return symbol;
            }
            symbol = symbol->next;
        }
        
        current = current->parent;
    }
    
    return NULL;  // No se encontró el símbolo
}

void remove_symbol(SymbolTable* st, const char* name) {
    unsigned int index = hash(name, st->size);
    Symbol* current = st->table[index];
    Symbol* prev = NULL;

    while (current) {
        if (strcmp(current->name, name) == 0) {
            if (prev) {
                prev->next = current->next;
            } else {
                st->table[index] = current->next;
            }
            free(current->name);
            // No liberamos expression aquí ya que es parte del AST
            free(current);
            return;
        }
        prev = current;
        current = current->next;
    }
}

void free_symbol_table(SymbolTable* st) {
    if (!st) return;

    for (int i = 0; i < st->size; i++) {
        Symbol* current = st->table[i];
        while (current) {
            Symbol* next = current->next;
            free(current->name);
            // No liberamos expression aquí ya que es parte del AST
            free(current);
            current = next;
        }
    }
    
    free(st->table);
    free(st);
}