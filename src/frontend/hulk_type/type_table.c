#include "../common/common.h"
#include "type_table.h"

#define INITIAL_CAPACITY 8

void print_debug_type(TypeDefinitionNode* type_def_node)
{
    printf("%s, father_count : %d, param_count: %d\n" , type_def_node->type_name, type_def_node->parent_arg_count, type_def_node->param_count);
}

void print_info( TypeInfo* info) {
    printf("Info : params: %d ", info->param_count);
    for (int i = 0; i < info->param_count; i++)
    {
        printf("%s ", info->params_name[i]);
    }
    printf("\n");    
    print_debug_type(info->type_def);
}

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
    type->type_id = table->count; // Asignar un ID único al tipo
    table->types[table->count++] = type;
}

void add_user_defined_type(TypeTable* table, TypeDefinitionNode* type_def_node, SymbolTable* current_scope) {
    // Este algoritmo permite insertar un nodo A que hereda de un nodo B chequeando que no se produzca una dependencia circular

    TypeDescriptor* A = type_table_lookup(table, type_def_node->type_name);
    TypeDescriptor* B = type_table_lookup(table, type_def_node->parent_name);
    TypeDescriptor* object_type = type_table_lookup(table, "Object");

    // Crear el scope del tipo, con el scope actual como padre
    SymbolTable* type_scope = create_symbol_table(current_scope);

    // Crear TypeInfo con parámetros y el scope creado
    TypeInfo* info = create_type_info(type_def_node, type_scope);
    
    if(!B) 
    {
        /* Si B no se encuentra en la tabla (crear una entrada "falsa" de B heredando de Object)
        B se insertará realmente cuando se encuentre su definición   */
        B = create_user_defined_type(type_def_node->parent_name, NULL, object_type, false );
        add_type(table, B);
    }

    // [Dependencia Circular]  A -> ... B -> ... -> A
    if(inherits_from(B,A))
    {
        fprintf(stderr, "Error: Detected circular hierachy dependency between types \"%s\" and \"%s\" \n", type_def_node->type_name, type_def_node->parent_name);
        exit(1);
    }

    if(A)   // si A se encuentra en la tabla
    {
        if(A->initializated)    // A ya habia sido definido
        {
            fprintf(stderr, "Error: Detected redefinition of type \"%s\" \n", type_def_node->type_name);
            exit(1);
        }
        
        modify_type(A, info, B, true);   // modificar A  para que herede de B
    }
    else    // si A no se encuentra en la tabla, ingresarlo
    {
        A = create_user_defined_type(type_def_node->type_name, info, B, true);
        add_type(table, A);
    }   
}

void register_builtin_types(TypeTable* type_table) {
    TypeDescriptor* object_type = create_builtin_type(HULK_Type_Object, "Object", NULL);

    TypeDescriptor* builtins[] = {
        object_type,
        create_builtin_type(HULK_Type_Number, "Number", object_type),
        create_builtin_type(HULK_Type_Boolean, "Bool", object_type),
        create_builtin_type(HULK_Type_String, "String", object_type),
        create_builtin_type(HULK_Type_Null, "Null", object_type),
        create_builtin_type(HULK_Type_Undefined, "Undefined", object_type)
    };

    int count = sizeof(builtins) / sizeof(builtins[0]);
    for (int i = 0; i < count; ++i) {
        add_type(type_table, builtins[i]);
    }
}


TypeDescriptor* require_type(TypeTable* table, const char* name) {
    TypeDescriptor* td = type_table_lookup(table, name);
    if (!td) {
        fprintf(stderr, "Error: tipo '%s' no encontrado\n", name);
        exit(1);
    }
    return td;
}

TypeDescriptor* type_table_lookup(TypeTable *table, const char *name) {
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
