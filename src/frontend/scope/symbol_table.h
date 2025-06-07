#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "../hulk_type/hulk_type.h"
#include "../common/common.h"

// Foward declarations
typedef struct TypeDescriptor TypeDescriptor;
typedef struct ASTNode ASTNode;

typedef struct Symbol {
    // - name: nombre del simbolo
    // - kind: tipo de simbolo (variable, funcion, parametro, builtin, etc.)
    // - type: tipo de dato del simbolo (TypeDescriptor)
    // - value: nodo AST asociado al simbolo (puede ser NULL si no hay)
    char* name;
    SymbolKind kind;
    TypeDescriptor* type;
    ASTNode* value;

    // Solo para funciones
    char** params_names;
    int params_count;
} Symbol;
typedef struct SymbolTable {
    // Tabla que contiene los simbolos de un scope.
    Symbol** symbols;
    int capacity;
    int size;
    struct SymbolTable* parent; // para scopes anidados
} SymbolTable;

// Prototipos para simbolos
Symbol* create_symbol(const char *name, SymbolKind kind, TypeDescriptor* type, ASTNode* value, char **params_names, int params_count);
void free_symbol(Symbol *symbol);

// Prototipos para tabla de simbolos
SymbolTable* create_symbol_table(SymbolTable *parent);
void free_symbol_table(SymbolTable *table);

void insert_symbol(SymbolTable *table, Symbol *symbol);
Symbol* lookup_symbol(SymbolTable *table, const char *name, bool search_in_parent);
Symbol* lookup_function_by_signature(SymbolTable* table, const char* name, int arg_count);
void set_symbol_return_type(SymbolTable *table, const char *name, TypeDescriptor *return_type);


#endif // SYMBOL_TABLE_H