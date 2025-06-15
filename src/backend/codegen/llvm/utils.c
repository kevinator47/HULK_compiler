#include <llvm-c/Core.h>
#include "../../../frontend/ast/ast.h"
#include "utils.h"
#include "generator.h"

LLVMTypeRef get_llvm_type_from_descriptor(TypeDescriptor* desc, LLVMCodeGenerator* generator) {
    if (!desc) {
        fprintf(stderr, "Error: Descriptor de tipo nulo al obtener tipo LLVM.\n");
        return NULL;
    }
    printf("[get_llvm_type_from_descriptor] Resolviendo tipo: %s (tag=%d)\n", desc->type_name, desc->tag);
    if (desc->llvm_type && !(LLVMGetTypeKind(desc->llvm_type) == LLVMStructTypeKind && LLVMIsOpaqueStruct(desc->llvm_type)))
        return desc->llvm_type;

    switch (desc->tag) {
        case HULK_Type_Number:
            desc->llvm_type = LLVMDoubleTypeInContext(generator->context);
            break;
        case HULK_Type_Boolean:
            desc->llvm_type = LLVMInt1TypeInContext(generator->context);
            break;
        case HULK_Type_String:
            desc->llvm_type = LLVMPointerType(LLVMInt8TypeInContext(generator->context), 0);
            break;
        case HULK_Type_UserDefined: 
            // Si no está creado, créalo (opaque por ahora)
            if (!desc->llvm_type)
                desc->llvm_type = LLVMStructCreateNamed(generator->context, desc->type_name);

            // Si ya tiene body, retorna
            if (LLVMGetTypeKind(desc->llvm_type) == LLVMStructTypeKind && !LLVMIsOpaqueStruct(desc->llvm_type))
                return desc->llvm_type;

            TypeInfo* info = desc->info;
            SymbolTable* scope = info->scope;
            int n = 0;
            for (int i = 0; i < scope->size; ++i) {
                if (scope->symbols[i]->kind == SYMBOL_TYPE_FIELD && !is_self_instance(scope->symbols[i]->name)) {
                    n = n + 1;
                }
            }
            LLVMTypeRef* members = malloc(sizeof(LLVMTypeRef) * n);
            int idx = 0;
            for (int i = 0; i < scope->size; ++i) {
                if (scope->symbols[i]->kind == SYMBOL_TYPE_FIELD && !is_self_instance(scope->symbols[i]->name)) {
                    TypeDescriptor* attr_desc = scope->symbols[i]->type;
                    if (!attr_desc) {
                        fprintf(stderr, "Error: No se encontró el tipo del atributo '%s' en el tipo '%s'.\n", scope->symbols[i]->name, desc->type_name);
                        members[idx++] = LLVMInt8TypeInContext(generator->context); // fallback
                    } else {
                        members[idx++] = get_llvm_type_from_descriptor(attr_desc, generator);
                    }
                }
            }
            printf("Antes de LLVMStructSetBody: %s, n=%d\n", desc->type_name, n);
            for (int i = 0; i < n; ++i) {
                printf("  Campo %d: tipo LLVM kind = %d\n", i, LLVMGetTypeKind(members[i]));
            }
            LLVMStructSetBody(desc->llvm_type, members, n, 0);
            printf("Después de LLVMStructSetBody: %s, kind=%d\n", desc->type_name, LLVMGetTypeKind(desc->llvm_type)); 
            printf("Definiendo body de %s: desc=%p, llvm_type=%p\n", desc->type_name, (void*)desc, (void*)desc->llvm_type);
            printf("Struct %s creado con %d campos:\n", desc->type_name, n);
            for (int i = 0; i < n; ++i) {
                printf("  Campo %d: tipo LLVM kind = %d\n", i, LLVMGetTypeKind(members[i]));
            }
            free(members);          
            break;
        
        default:
            fprintf(stderr, "Error: Tipo no soportado para generación de código LLVM: %d\n", desc->tag);
            desc->llvm_type = LLVMInt8TypeInContext(generator->context); // fallback
            break;
    }
    return desc->llvm_type;
}

BuiltinKind get_builtin_kind(const char* name) {
    if (strcmp(name, "print") == 0) return BUILTIN_PRINT;
    if (strcmp(name, "sqrt") == 0) return BUILTIN_SQRT;
    if (strcmp(name, "sin") == 0) return BUILTIN_SIN;
    if (strcmp(name, "cos") == 0) return BUILTIN_COS;
    if (strcmp(name, "exp") == 0) return BUILTIN_EXP;
    if (strcmp(name, "log") == 0) return BUILTIN_LOG;
    if (strcmp(name, "pow") == 0) return BUILTIN_POW;
    if (strcmp(name, "fmod") == 0) return BUILTIN_FMOD;
    if (strcmp(name, "rand") == 0) return BUILTIN_RAND;
    return BUILTIN_NONE;
}

const char* get_print_format(LLVMTypeRef type, LLVMContextRef context) {
    if (LLVMGetTypeKind(type) == LLVMDoubleTypeKind) return "%g\n";
    if (LLVMGetTypeKind(type) == LLVMIntegerTypeKind && LLVMGetIntTypeWidth(type) == 1) return "%s\n";
    if (LLVMGetTypeKind(type) == LLVMPointerTypeKind) return "%s\n";
    return "<unknown>\n";
}

bool is_self_instance(char* name) {
    return strcmp(name, "self") == 0 || strcmp(name, "this") == 0;
}