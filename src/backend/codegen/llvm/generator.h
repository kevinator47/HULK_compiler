#ifndef LLVM_CODEGEN_H
#define LLVM_CODEGEN_H

#include "../../../frontend/ast/ast.h"
#include "../../../frontend/common/common.h"
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
    TypeTable* type_table; // Tabla de tipos para resolver tipos de nodos AST

    // Punteros a las implementaciones de visit_ para CADA tipo de nodo AST
    LLVMValueRef (*visit_Literal)(LLVMCodeGenerator* self, LiteralNode* node);
    LLVMValueRef (*visit_UnaryOp)(LLVMCodeGenerator* self, UnaryOperationNode* node);
    LLVMValueRef (*visit_BinaryOp)(LLVMCodeGenerator* self, BinaryOperationNode* node);
    LLVMValueRef (*visit_WhileLoop)(LLVMCodeGenerator* self, WhileLoopNode* node);
    LLVMValueRef (*visit_ExpressionBlock)(LLVMCodeGenerator* self, ExpressionBlockNode* node);
    LLVMValueRef (*visit_Let)(LLVMCodeGenerator* self, LetInNode* node);
    LLVMValueRef (*visit_Variable)(LLVMCodeGenerator* self, VariableNode* node);
    LLVMValueRef (*visit_ReassignNode)(LLVMCodeGenerator* self, ReassignNode* node);
    LLVMValueRef (*visit_Conditional)(LLVMCodeGenerator* self, ConditionalNode* node);
    LLVMValueRef (*visit_FunctionDefinition)(LLVMCodeGenerator* self, FunctionDefinitionNode* node);
    LLVMValueRef (*visit_FunctionCall)(LLVMCodeGenerator* self, FunctionCallNode* node);
    void (*declare_FunctionHeaders_impl)(LLVMCodeGenerator* self, FunctionDefinitionListNode* node);
    void (*define_FunctionBodies_impl)(LLVMCodeGenerator* self, FunctionDefinitionListNode* node);
};

LLVMCodeGenerator* create_llvm_code_generator(const char* module_name, TypeTable* type_table);
void destroy_llvm_code_generator(LLVMCodeGenerator* generator);
LLVMModuleRef generate_code(ProgramNode* program, LLVMCodeGenerator* generator);
void declare_external_functions(LLVMModuleRef module, LLVMContextRef context);

#endif // LLVM_CODEGEN_H