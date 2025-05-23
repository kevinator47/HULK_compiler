
#ifndef LLVM_CODEGEN_H
#define LLVM_CODEGEN_H

#include "../entities/ast.h"
#include "../common/common.h"
#include "scope_stack.h"
#include <llvm-c/Core.h>
#include <llvm-c/Analysis.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct LLVMCodeGenerator LLVMCodeGenerator;

struct LLVMCodeGenerator {
    LLVMContextRef context;
    LLVMModuleRef module;
    LLVMBuilderRef builder;
    ScopeStack* scope_stack; 

        // Punteros a las implementaciones de visit_ para CADA tipo de nodo AST
    LLVMValueRef (*visit_NumberLiteral)(LLVMCodeGenerator* self, NumberLiteralNode* node);
    LLVMValueRef (*visit_BooleanLiteral)(LLVMCodeGenerator* self, BooleanLiteralNode* node);
    LLVMValueRef (*visit_StringLiteral)(LLVMCodeGenerator* self, StringLiteralNode* node);
    LLVMValueRef (*visit_UnaryOp)(LLVMCodeGenerator* self, UnaryOpNode* node);
    LLVMValueRef (*visit_BinaryOp)(LLVMCodeGenerator* self, BinaryOpNode* node);
    LLVMValueRef (*visit_ExpressionBlock)(LLVMCodeGenerator* self, ExpressionBlockNode* node);
    LLVMValueRef (*visit_Let)(LLVMCodeGenerator* self, LetNode* node);
    LLVMValueRef (*visit_Variable)(LLVMCodeGenerator* self, VariableNode* node);
    LLVMValueRef (*visit_Conditional)(LLVMCodeGenerator* self, ConditionalNode* node);
    LLVMValueRef (*visit_FunctionCall)(LLVMCodeGenerator* self, FunctionCallNode* node);
};

LLVMCodeGenerator* create_llvm_code_generator(const char* module_name);
void destroy_llvm_code_generator(LLVMCodeGenerator* generator);
LLVMModuleRef generate_code(ASTNode* root_node, LLVMCodeGenerator* generator);

#endif // LLVM_CODEGEN_H