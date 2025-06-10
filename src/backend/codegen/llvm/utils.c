#include <llvm-c/Core.h>
#include "../../../frontend/ast/ast.h"

LLVMTypeRef get_llvm_type_from_descriptor(TypeDescriptor* desc, LLVMContextRef context) {
    switch (desc->tag) {
        case HULK_Type_Number:   return LLVMDoubleTypeInContext(context);
        case HULK_Type_Boolean:  return LLVMInt1TypeInContext(context);
        case HULK_Type_String:   return LLVMPointerType(LLVMInt8TypeInContext(context), 0);

        default: return NULL;
    }
}
