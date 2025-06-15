#ifndef LLVM_VISITORS_H
#define LLVM_VISITORS_H

#include "generator.h"

// Declaraciones de las funciones visitor para cada tipo de nodo AST
LLVMValueRef visit_Literal_impl(LLVMCodeGenerator* self, LiteralNode* node);
LLVMValueRef visit_UnaryOp_impl(LLVMCodeGenerator* self, UnaryOperationNode* node);
LLVMValueRef visit_BinaryOp_impl(LLVMCodeGenerator* self, BinaryOperationNode* node);
LLVMValueRef visit_ExpressionBlock_impl(LLVMCodeGenerator* self, ExpressionBlockNode* node);
LLVMValueRef visit_Let_impl(LLVMCodeGenerator* self, LetInNode* node);
LLVMValueRef visit_Variable_impl(LLVMCodeGenerator* self, VariableNode* node);
LLVMValueRef visit_ReassignNode_impl(LLVMCodeGenerator* self, ReassignNode* node);
LLVMValueRef visit_Conditional_impl(LLVMCodeGenerator* self, ConditionalNode* node);
LLVMValueRef visit_WhileLoop_impl(LLVMCodeGenerator* self, WhileLoopNode* node);
LLVMValueRef visit_ExpressionBlock_impl(LLVMCodeGenerator* self, ExpressionBlockNode* node);
void define_FunctionBodies_impl(LLVMCodeGenerator* self, FunctionDefinitionListNode* node);
void declare_FunctionHeaders_impl(LLVMCodeGenerator* self, FunctionDefinitionListNode* node);
LLVMValueRef visit_FunctionDefinition_impl(LLVMCodeGenerator* self, FunctionDefinitionNode* node);
LLVMValueRef visit_FunctionCall_impl(LLVMCodeGenerator* self, FunctionCallNode* node);
void declare_method_signature_impl(LLVMCodeGenerator* self, TypeDescriptor* type, FunctionDefinitionNode* fn);
void define_method_body_impl(LLVMCodeGenerator* self, TypeDescriptor* type, FunctionDefinitionNode* fn);
LLVMValueRef visit_NewNode_impl(LLVMCodeGenerator* self, NewNode* node);

#endif // LLVM_VISITORS_H