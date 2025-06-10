#include <llvm-c/Core.h>
#include "../../../frontend/ast/ast.h"
#include "utils.h"

LLVMTypeRef get_llvm_type_from_descriptor(TypeDescriptor* desc, LLVMContextRef context) {
    switch (desc->tag) {
        case HULK_Type_Number:   return LLVMDoubleTypeInContext(context);
        case HULK_Type_Boolean:  return LLVMInt1TypeInContext(context);
        case HULK_Type_String:   return LLVMPointerType(LLVMInt8TypeInContext(context), 0);

        default: return NULL;
    }
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