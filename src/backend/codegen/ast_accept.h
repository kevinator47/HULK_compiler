#ifndef AST_ACCEPT_H
#define AST_ACCEPT_H

#include "ast/ast.h"
#include "llvm/generator.h"

LLVMValueRef generic_ast_accept(struct ASTNode* self, struct LLVMCodeGenerator* visitor);

#endif // AST_ACCEPT_H