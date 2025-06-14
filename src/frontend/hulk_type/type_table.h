#ifndef TYPE_TABLE_H
#define TYPE_TABLE_H

#include "hulk_type.h"

typedef struct TypeTable {
    TypeDescriptor **types;
    int count;
    int capacity;
} TypeTable;

// Prototipos funciones
TypeTable* create_type_table();
void add_type(TypeTable *table, TypeDescriptor *type);
void add_user_defined_type(TypeTable* table, TypeDefinitionNode* type_def_node, const char* parent_name, SymbolTable* current_scope);
void register_builtin_types(TypeTable* type_table);
TypeDescriptor* require_type(TypeTable* table, const char* name);
TypeDescriptor* type_table_lookup(TypeTable *table, const char *name);
void free_type_table(TypeTable *table);

#endif
