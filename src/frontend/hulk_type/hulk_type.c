#include "hulk_type.h"

TypeDescriptor *create_builtin_type(HULK_Type tag, const char *type_name) {
    // Crea un tipo de dato primitivo en el lenguaje Hulk.
    TypeDescriptor *type = malloc(sizeof(TypeDescriptor));
    if (!type) {
        return NULL; // Error al asignar memoria
    }
    type->type_name = strdup(type_name);
    type->tag = tag;
    type->info = NULL; // Tipos primitivos no tienen información adicional
    return type;
}

TypeDescriptor *create_user_defined_type(TypeInfo *info, const char *type_name) {
    // Crea un tipo de dato definido por el usuario en el lenguaje Hulk.
    TypeDescriptor *type = malloc(sizeof(TypeDescriptor));
    if (!type) {
        return NULL; // Error al asignar memoria
    }
    type->type_name = strdup(type_name);
    type->tag = HULK_Type_UserDefined;
    type->info = info; // Asigna la información del tipo definido por el usuario
    return type;
}


int is_builtin_type(const TypeDescriptor *type) {
    // Verifica si un tipo es un tipo primitivo en el lenguaje Hulk.
    return type->tag == HULK_Type_Number || type->tag == HULK_Type_String ||
           type->tag == HULK_Type_Boolean || type->tag == HULK_Type_Null ||
           type->tag == HULK_Type_Undefined;
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
        for (int i = 0; i < info->attribute_count; i++) {
            free_attribute(info->attributes[i]);
        }
        free(info->attributes);
                
        // TODO: implementar liberación de la tabla de métodos
        free(info);
    }
}

void free_attribute(Attribute *attr) {
    // Libera la memoria de un atributo.
    if (attr) {
        free(attr->attr_name);
        if (attr->initializer) {
            free_ast_node(attr->initializer); // Asumiendo que el ASTNode también tiene una función de liberación
        }
        free(attr);
    }
}