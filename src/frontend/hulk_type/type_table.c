#include "../common/common.h"
#include "type_table.h"

#define INITIAL_CAPACITY 8

TypeTable *create_type_table() {
    TypeTable *table = malloc(sizeof(TypeTable));
    table->count = 0;
    table->capacity = INITIAL_CAPACITY;
    table->types = malloc(sizeof(TypeDescriptor *) * table->capacity);
    return table;
}

void add_type(TypeTable *table, TypeDescriptor *type) {
    if (table->count == table->capacity) {
        table->capacity *= 2;
        table->types = realloc(table->types, sizeof(TypeDescriptor *) * table->capacity);
    }
    table->types[table->count++] = type;
}

TypeDescriptor *type_table_lookup(TypeTable *table, const char *name) {
    for (int i = 0; i < table->count; i++) {
        if (strcmp(table->types[i]->type_name, name) == 0) {
            return table->types[i];
        }
    }
    return NULL; // Tipo no encontrado
}

void free_type_table(TypeTable *table) {
    for (int i = 0; i < table->count; i++) {
        free_type_descriptor(table->types[i]);
    }
    free(table->types);
    free(table);
}
