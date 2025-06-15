#include "hulk_type.h"
#include "../ast/ast.h"

TypeDescriptor* create_builtin_type(HULK_Type tag, const char *type_name, TypeDescriptor* parent) {
    // Crea un tipo de dato primitivo del lenguaje HULK.
    TypeDescriptor *type = malloc(sizeof(TypeDescriptor));
    if (!type) return NULL; 
    
    type->type_name = strdup(type_name);
    type->tag = tag;
    type->info = NULL;      
    type->parent = parent;
    type->initializated = true;
    type->llvm_type = NULL;
    type->type_id = 0; 
    return type;
}

TypeDescriptor* create_user_defined_type(const char *name, TypeInfo* info, TypeDescriptor* parent, bool init) {
    // Crea un tipo de dato definido por el usuario en el lenguaje HULK.
    TypeDescriptor *type = malloc(sizeof(TypeDescriptor));
    if (!type)
        return NULL; // Error al asignar memoria
    
    type->type_name = strdup(name);
    type->tag = HULK_Type_UserDefined;
    type->info = info;
    type->parent = parent;
    type->initializated = init;    
    type->llvm_type = NULL;  
    type->type_id = 0;
    return type;
}

TypeInfo* create_type_info( Param** params, int count, SymbolTable* scope, TypeDefinitionNode* type_def) {
    TypeInfo* info = malloc(sizeof(TypeInfo));
    if (!info) return NULL;

    if (count > 0) {
        info->param_count = count;
        info->params_name = malloc(count * sizeof(char*));
        for (int i = 0; i < count; i++)
            info->params_name[i] = strdup(params[i]->name);
    } 
    else {
        info->param_count = 0;
        info->params_name = NULL;
    }
    info->type_def = type_def;
    info->scope = scope;
    return info;
}

void modify_type(TypeDescriptor* t, TypeInfo* info, TypeDescriptor* parent, bool init ) {
    // Modifica los valores de un tipo
    if (info) t->info = info;
    if (parent) t->parent = parent;
    if (init) t->initializated = init;
}

bool inherits_from(TypeDescriptor* t1, TypeDescriptor* t2) {
    // Retorna true si t1 hereda de t2
    if (!t1 || !t2 || t1 == t2) return false;

    TypeDescriptor* current = t1->parent;
    while (current) {
        if (cmp_type(current, t2)) return true;
        current = current->parent;
    }

    return false;
}

bool conforms(TypeDescriptor* t1, TypeDescriptor* t2) {
    // Retorna true si t1 conforma t2
    if (!t1 || !t2) return false;
    return cmp_type(t1, t2) || inherits_from(t1, t2);
}

bool cmp_type(TypeDescriptor* t1, TypeDescriptor* t2) {
    // Retorna true si t1 y t2 son iguales
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
    if (!info) return ;
                
    // TODO: implementar liberación de la tabla de simbolos
    free(info);   
}
