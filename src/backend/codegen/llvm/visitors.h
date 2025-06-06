#ifndef LLVM_VISITORS_H
#define LLVM_VISITORS_H

#include "generator.h"

// Declaraciones de las funciones visitor para cada tipo de nodo AST
LLVMValueRef visit_NumberLiteral_impl(LLVMCodeGenerator* self, NumberLiteralNode* node);
LLVMValueRef visit_BooleanLiteral_impl(LLVMCodeGenerator* self, BooleanLiteralNode* node);
LLVMValueRef visit_StringLiteral_impl(LLVMCodeGenerator* self, StringLiteralNode* node);
LLVMValueRef visit_UnaryOp_impl(LLVMCodeGenerator* self, UnaryOperationNode* node);
LLVMValueRef visit_BinaryOp_impl(LLVMCodeGenerator* self, BinaryOperationNode* node);
LLVMValueRef visit_ExpressionBlock_impl(LLVMCodeGenerator* self, ExpressionBlockNode* node);
LLVMValueRef visit_Let_impl(LLVMCodeGenerator* self, LetInNode node);
LLVMValueRef visit_Variable_impl(LLVMCodeGenerator* self, VariableNode* node);
LLVMValueRef visit_Conditional_impl(LLVMCodeGenerator* self, ConditionalNode* node);
LLVMValueRef visit_FunctionCall_impl(LLVMCodeGenerator* self, FunctionCallNode* node);

#endif // LLVM_VISITORS_H