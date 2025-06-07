#ifndef HULK_TYPE_H
#define HULK_TYPE_H

#include "../ast/ast.h"
#include "../common/common.h"

// Foward declarations
typedef struct TypeInfo TypeInfo;
typedef struct Attribute Attribute;
typedef struct ASTNode ASTNode;

typedef struct TypeDescriptor {
    // Describe un tipo de dato en el lenguaje Hulk.
    char* type_name;        // Nombre del tipo
    HULK_Type tag;          // Especifica el tipo de dato(puede ser un tipo primitivo o un tipo definido por el usuario)
    TypeInfo* info;         // NULL para tipos primitivos, apunta a la información del tipo para tipos definidos por el usuario.
} TypeDescriptor;

typedef struct TypeInfo {
    // Información adicional sobre un tipo definido por el usuario.
    char** params_name;             // Nombres de los parámetros del tipo, NULL si no tiene parámetros         
    int param_count;                // Cantidad de parámetros del tipo
    Attribute** attributes;         // Atributos del tipo, NULL si no tiene atributos
    int attribute_count;            // Cantidad de atributos del tipo
    struct FunctionTable* methods;  // Tabla de métodos del tipo, NULL si no tiene métodos
    struct TypeDescriptor* parent;  // Tipo padre, Object por defecto
} TypeInfo;

typedef struct Attribute {
    // Un atributo es un par nombre-valor, donde el valor es una expresión AST
    char* attr_name;
    ASTNode* initializer;
    TypeDescriptor* type;
} Attribute;

// Prototipos de funciones
TypeDescriptor *create_builtin_type(HULK_Type tag, const char* type_name);
TypeDescriptor *create_user_defined_type(TypeInfo* info, const char* type_name);
int is_builtin_type(const TypeDescriptor* type);
void free_type_descriptor(TypeDescriptor* type);
void free_type_info(TypeInfo* info);
void free_attribute(Attribute* attr);

#endif // HULK_TYPE_H