#ifndef HULK_TYPE_H
#define HULK_TYPE_H

#include <llvm-c/Core.h>
#include "../ast/ast.h"
#include "../common/common.h"

// Foward declarations
typedef struct TypeInfo TypeInfo;
typedef struct Attribute Attribute;
typedef struct ASTNode ASTNode;
typedef struct SymbolTable SymbolTable;

typedef struct TypeDescriptor {
    // Describe un tipo de dato en el lenguaje Hulk.
    char* type_name;                // Nombre del tipo
    HULK_Type tag;                  // Especifica el tipo de dato(puede ser un tipo primitivo o un tipo definido por el usuario)
    TypeInfo* info;                 // NULL para tipos primitivos, apunta a la información del tipo para tipos definidos por el usuario.
    struct TypeDescriptor* parent;  // Tipo padre, Object por defecto
    bool initializated;             // Especifica si el tipo ya ha sido inicializado(para tipos del usuario)

    LLVMTypeRef llvm_type; // Referencia al tipo de dato en LLVM, NULL si no se ha generado
} TypeDescriptor;

typedef struct TypeInfo {
    // Información adicional sobre un tipo definido por el usuario.
    char** params_name;             // Nombres de los parámetros del tipo, NULL si no tiene parámetros         
    int param_count;                // Cantidad de parámetros del tipo
    struct SymbolTable* scope;      // Tabla de simbolos donde iran atributos y metodos
} TypeInfo;


// Prototipos de funciones
TypeDescriptor* create_builtin_type(HULK_Type tag, const char *type_name, TypeDescriptor* parent);
TypeDescriptor* create_user_defined_type(const char *name, TypeInfo* info, TypeDescriptor* parent, bool init);
bool inherits_from(TypeDescriptor* t1, TypeDescriptor* t2);
bool conforms(TypeDescriptor* t1 , TypeDescriptor* t2);
bool cmp_type(TypeDescriptor* t1, TypeDescriptor* t2);
void free_type_descriptor(TypeDescriptor* type);
void free_type_info(TypeInfo* info);
#endif // HULK_TYPE_H