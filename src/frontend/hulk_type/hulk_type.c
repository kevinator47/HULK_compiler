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
    type->llvm_type = NULL;
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

bool inherits_from(TypeDescriptor* t1, TypeDescriptor* t2) {
    if (!t1 || !t2 || t1 == t2) return false;

    TypeDescriptor* current = t1->parent;
    while (current) {
        if (cmp_type(current, t2)) return true;
        current = current->parent;
    }

    return false;
}

bool conforms(TypeDescriptor* t1, TypeDescriptor* t2) {
    if (!t1 || !t2) return false;
    return cmp_type(t1, t2) || inherits_from(t1, t2);
}

bool cmp_type(TypeDescriptor* t1, TypeDescriptor* t2) {
    if (t1 == NULL || t2 == NULL) return false;

    if (t1->tag == HULK_Type_UserDefined && t2->tag == HULK_Type_UserDefined) {
        return strcmp(t1->type_name, t2->type_name) == 0;
    }

    return t1->tag == t2->tag;
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
