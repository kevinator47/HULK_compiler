#ifndef UTILS_H
#define UTILS_H


#include <llvm-c/Core.h>
#include "../../../frontend/ast/ast.h"

LLVMTypeRef get_llvm_type_from_descriptor(TypeDescriptor* desc, LLVMContextRef context); 

#endif // UTILS_H