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

TypeDescriptor** ancestors(TypeDescriptor* t, int* count) {
    *count = 0;
    
    if(!t) return NULL;
    if(!t->parent) return malloc(0);    // lista vacia
    
    int capacity = 8;
    TypeDescriptor** result = malloc(sizeof(TypeDescriptor*) * capacity);

    TypeDescriptor* current = t->parent;
    while (current) {
        if (*count >= capacity) {
            capacity *= 2;
            result = realloc(result, sizeof(TypeDescriptor*) * capacity);
        }
        result[(*count)++] = current;

        current = current->parent;
    }

    // Ajustar memoria al tamaño exacto
    if (*count < capacity) {
        result = realloc(result, sizeof(TypeDescriptor*) * (*count));
    }

    return result;
}

bool conforms(TypeDescriptor* t1, TypeDescriptor* t2) {
    if (!t1 || !t2) return false;
    if (t1 == t2) return true;

    int count = 0;
    TypeDescriptor** list = ancestors(t1, &count);

    for (int i = 0; i < count; i++) {
        if (cmp_type(list[1] , t2)) {
            free(list);
            return true;
        }
    }

    free(list);
    return false;
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
