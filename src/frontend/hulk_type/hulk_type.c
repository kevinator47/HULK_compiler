#include "hulk_type.h"

TypeDescriptor* create_builtin_type(HULK_Type tag, const char *type_name, TypeDescriptor* parent) {
    // Crea un tipo de dato primitivo del lenguaje HULK.
    
    TypeDescriptor *type = malloc(sizeof(TypeDescriptor));
    if (!type) {
        return NULL; 
    }
    type->type_name = strdup(type_name);
    type->tag = tag;
    type->info = NULL;      
    type->parent = parent;
    type->initializated = true;
    return type;
}

TypeDescriptor* create_user_defined_type(const char *name, TypeInfo* info, TypeDescriptor* parent, bool init) {
    // Crea un tipo de dato definido por el usuario en el lenguaje Hulk.
    TypeDescriptor *type = malloc(sizeof(TypeDescriptor));
    if (!type) {
        return NULL; // Error al asignar memoria
    }
    type->type_name = strdup(name);
    type->tag = HULK_Type_UserDefined;
    type->info = info;
    type->parent = parent;
    type->initializated = init;      
    return type;
}

int is_builtin_type(const TypeDescriptor *type) {
    // Verifica si un tipo es un tipo primitivo en el lenguaje Hulk.
    return type->tag == HULK_Type_Number || type->tag == HULK_Type_String ||
           type->tag == HULK_Type_Boolean || type->tag == HULK_Type_Null ||
           type->tag == HULK_Type_Undefined;
}

bool is_compatible(TypeDescriptor* t1 , TypeDescriptor *t2) {
    //  TODO : add check if t2 if ancestor of t1
    return (t1->tag == t2->tag || t2->tag == HULK_Type_Object);
}

void free_type_descriptor(TypeDescriptor *type) {
    // Libera la memoria de un TypeDescriptor.
    if (type) {
        free(type->type_name); // Libera el nombre del tipo
        if (type->info) 
            free_type_info(type->info);
        free(type);
    }
}

void free_type_info(TypeInfo *info) {
    // Libera la memoria asociada a TypeInfo.
    if (info) {
        for (int i = 0; i < info->param_count; i++) {
            free(info->params_name[i]);
        }
        free(info->params_name);
                
        // TODO: implementar liberación de la tabla de métodos
        free(info);
    }
}
