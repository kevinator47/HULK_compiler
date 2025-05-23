#ifndef AST_ACCEPT_H
#define AST_ACCEPT_H

#include "../entities/ast.h"
#include "../codegen/llvm/generator.h"

LLVMValueRef generic_ast_accept(struct ASTNode* self, struct LLVMCodeGenerator* visitor);

#endif // AST_ACCEPT_H