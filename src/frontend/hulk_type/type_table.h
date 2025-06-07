#ifndef TYPE_TABLE_H
#define TYPE_TABLE_H

#include "hulk_type.h"

typedef struct TypeTable {
    TypeDescriptor **types;
    int count;
    int capacity;
} TypeTable;

TypeTable* create_type_table();
void add_type(TypeTable *table, TypeDescriptor *type);
TypeDescriptor* type_table_lookup(TypeTable *table, const char *name);
void free_type_table(TypeTable *table);

#endif
