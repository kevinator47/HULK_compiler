#ifndef BUILTINS_H
#define BUILTINS_H

#include <llvm-c/Core.h>
#include "../../../frontend/ast/ast.h"
#include "utils.h"
#include "generator.h"



LLVMValueRef emit_builtin_print(LLVMCodeGenerator* self, FunctionCallNode* node);
LLVMValueRef generate_builtin_function(LLVMCodeGenerator* self, FunctionCallNode* node);

#endif // BUILTINS_H