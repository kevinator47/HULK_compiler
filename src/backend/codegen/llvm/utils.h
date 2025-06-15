#ifndef UTILS_H
#define UTILS_H


#include <llvm-c/Core.h>
#include "../../../frontend/ast/ast.h"
#include "generator.h"

typedef enum {
    BUILTIN_NONE,
    BUILTIN_PRINT,
    BUILTIN_SQRT,
    BUILTIN_SIN,
    BUILTIN_COS,
    BUILTIN_EXP,
    BUILTIN_LOG,
    BUILTIN_POW,
    BUILTIN_FMOD,
    BUILTIN_RAND
} BuiltinKind;

LLVMTypeRef get_llvm_type_from_descriptor(TypeDescriptor* desc, LLVMCodeGenerator* generator); 
BuiltinKind get_builtin_kind(const char* name);
const char* get_print_format(LLVMTypeRef type, LLVMContextRef context);
bool is_self_instance(char* name);

#endif // UTILS_H