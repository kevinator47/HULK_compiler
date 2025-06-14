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
    if (lookup_symbol(table, symbol->name, symbol->kind, false)) 
    {
        // Lanzar error 
        fprintf(stderr, "Error[%d]: Redefinition of symbol \"%s\"\n", symbol->kind, symbol->name);
        exit(1);        
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

Symbol* lookup_symbol(SymbolTable* table, const char* name, SymbolKind kind, bool search_parent) {
    if (!table || !name) {
        return NULL; // Invalid input
    }
    
    // Search for the symbol in the current table
    for (int i = 0; i < table->size; i++) {
        Symbol* sym = table->symbols[i];
        if (strcmp(sym->name, name) == 0 && (kind == SYMBOL_ANY || sym->kind == kind)) 
            return sym;        
    }
    // If not found in the current table, check the parent table if search_parent is true
    if (table->parent && search_parent) {
        return lookup_symbol(table->parent, name, kind, true);
    }
    return NULL; // Symbol not found
}

Symbol* lookup_function_by_signature(SymbolTable* table, const char* name, int arg_count) {
    Symbol* func_symbol = lookup_symbol(table, name, SYMBOL_FUNCTION, true);

    if(!func_symbol) return NULL;
    if(((FunctionDefinitionNode*)func_symbol->value)->param_count != arg_count) return NULL;
    return func_symbol;
}

void set_symbol_return_type(SymbolTable* table, Symbol* symbol, TypeDescriptor* return_type) {
    
    if (!table || !symbol || !return_type) 
        DIE("Invalid input to set_function_symbol_return_type");
    
    symbol->type = return_type; // Update the type of the function symbol
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

void debug_print_scope(SymbolTable* scope, const char* scope_name) {
    if (!scope) {
        printf("Scope '%s' is NULL!\n", scope_name);
        return;
    }
    printf("=== Scope '%s' ===\n", scope_name);
    for (int i = 0; i < scope->size; i++) {
        Symbol* sym = scope->symbols[i];
        printf("  [%d] name: '%s', kind: %d, type: %s\n",
               i,
               sym->name,
               sym->kind,
               sym->type ? sym->type->type_name : "NULL");
    }
    printf("====================\n");
}