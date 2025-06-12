#include "symbol_table.h"

Symbol* create_symbol(const char* name, SymbolKind kind, TypeDescriptor* type, ASTNode* value) {
    Symbol* symbol = malloc(sizeof(Symbol));
    if (!symbol) {
        return NULL; // Error allocating memory
    }
    symbol->name = strdup(name);
    if (!symbol->name) {
        free(symbol);
        return NULL; // Error allocating memory
    }
    symbol->kind = kind;
    symbol->type = type;
    symbol->value = value; // Puede ser NULL si no hay un nodo AST asociado
    return symbol;
}

SymbolTable* create_symbol_table(SymbolTable* parent) {
    SymbolTable* table = malloc(sizeof(SymbolTable));
    if (!table) {
        return NULL; // Error allocating memory
    }
    table->capacity = 10; // Initial capacity
    table->size = 0;
    table->symbols = malloc(table->capacity * sizeof(Symbol*));
    if (!table->symbols) {
        free(table);
        return NULL; // Error allocating memory
    }
    table->parent = parent;
    return table;
}

void insert_symbol(SymbolTable* table, Symbol* symbol) {
    
    // Check for valid input
    if (!table || !symbol || !symbol->name) {
        DIE("Invalid input to insert_symbol");
    }
    
    // Check if the symbol already exists in the current table
    if (lookup_symbol(table, symbol->name, false)) {
        // Replace the existing symbol
        for (int i = 0; i < table->size; i++) {
            if (strcmp(table->symbols[i]->name, symbol->name) == 0) {
                free_symbol(table->symbols[i]); // Free the old symbol
                table->symbols[i] = symbol; // Replace with the new symbol
                return;
            }
        }
    }
    
    // Resize the symbols array if necessary
    if (table->size >= table->capacity) {
    
        table->capacity *= 2;
        Symbol** new_symbols = realloc(table->symbols, table->capacity * sizeof(Symbol*));
        if (!new_symbols) {
            DIE("Failed to resize symbol table");
        }
        table->symbols = new_symbols;
    }
    // Insert the symbol into the table
    table->symbols[table->size++] = symbol;
}

Symbol* lookup_symbol(SymbolTable* table, const char* name, bool search_parent) {
    if (!table || !name) {
        return NULL; // Invalid input
    }
    
    // Search for the symbol in the current table
    for (int i = 0; i < table->size; i++) {
        if (strcmp(table->symbols[i]->name, name) == 0) {
            return table->symbols[i];
        }
    }
    // If not found in the current table, check the parent table if search_parent is true
    if (table->parent && search_parent) {
        return lookup_symbol(table->parent, name, true);
    }
    return NULL; // Symbol not found
}

Symbol* lookup_symbol_type_field(SymbolTable* table, const char* name, bool search_parent) {
    Symbol* symbol = lookup_symbol(table, name, search_parent);
    if (symbol && symbol->kind == SYMBOL_TYPE_FIELD) {
        return symbol;
    }
    return NULL; // No se encontró o no es un campo de tipo
}

Symbol* lookup_function_by_signature(SymbolTable* table, const char* name, int arg_count) {
    Symbol* func_symbol = lookup_symbol(table, name, true);

    if(!func_symbol || (func_symbol->kind != SYMBOL_FUNCTION)) return NULL;
    if(((FunctionDefinitionNode*)func_symbol->value)->param_count != arg_count) return NULL;
    return func_symbol;
}

void set_symbol_return_type(SymbolTable* table, const char* name, TypeDescriptor* return_type) {
    if (!table || !name || !return_type) {
        DIE("Invalid input to set_function_symbol_return_type");
    }
    
    Symbol* symbol = lookup_symbol(table, name, true);
    if (symbol) {
        symbol->type = return_type; // Update the type of the function symbol
    } else {
        DIE("Function not found or not a function symbol");
    }
}

void free_symbol(Symbol* symbol) {
    if (symbol) {
        free(symbol->name);
        free(symbol);
    }
}

void free_symbol_table(SymbolTable* table) {
    if (table) {
        for (int i = 0; i < table->size; i++) {
            free_symbol(table->symbols[i]);
        }
        free(table->symbols);
        free(table);
    }
}