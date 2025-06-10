#include "ir_symbol_table.h"
#include <stdlib.h>
#include <string.h>

#define DEFAULT_TABLE_SIZE 32

static unsigned int hash(const char* str, int size) {
    unsigned int h = 5381;
    while (*str)
        h = ((h << 5) + h) + (unsigned char)(*str++);
    return h % size;
}

IrSymbolTable* create_ir_symbol_table(int size, IrSymbolTable* parent) {
    IrSymbolTable* st = (IrSymbolTable*)malloc(sizeof(IrSymbolTable));
    if (!st) return NULL;
    st->size = (size > 0) ? size : DEFAULT_TABLE_SIZE;
    st->table = (IrSymbol**)calloc(st->size, sizeof(IrSymbol*));
    st->parent = parent;
    return st;
}

void free_ir_symbol_table(IrSymbolTable* st) {
    if (!st) return;
    for (int i = 0; i < st->size; ++i) {
        IrSymbol* sym = st->table[i];
        while (sym) {
            IrSymbol* next = sym->next;
            free(sym->name);
            free(sym);
            sym = next;
        }
    }
    free(st->table);
    free(st);
}

bool insert_ir_symbol(IrSymbolTable* st, const char* name, LLVMValueRef value) {
    if (!st || !name) return false;
    unsigned int idx = hash(name, st->size);
    IrSymbol* sym = (IrSymbol*)malloc(sizeof(IrSymbol));
    if (!sym) return false;
    sym->name = strdup(name);
    sym->value = value;
    sym->next = st->table[idx];
    st->table[idx] = sym;
    return true;
}

IrSymbol* lookup_ir_symbol(IrSymbolTable* st, const char* name) {
    if (!st || !name) return NULL;
    unsigned int idx = hash(name, st->size);
    IrSymbol* sym = st->table[idx];
    while (sym) {
        if (strcmp(sym->name, name) == 0)
            return sym;
        sym = sym->next;
    }
    // Buscar en el padre si no se encuentra en el actual
    if (st->parent)
        return lookup_ir_symbol(st->parent, name);
    return NULL;
}