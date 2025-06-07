#include "visitors.h"
#include <stdio.h>

// --- Implementaciones de los métodos visit_ para Expresiones ---

LLVMValueRef visit_NumberLiteral_impl(LLVMCodeGenerator* self, NumberLiteralNode* node) {
    LLVMTypeRef double_type = LLVMDoubleTypeInContext(self->context);
    return LLVMConstReal(double_type, node->value);
}

LLVMValueRef visit_BooleanLiteral_impl(LLVMCodeGenerator* self, BooleanLiteralNode* node) {
    LLVMTypeRef bool_type = LLVMInt1TypeInContext(self->context);
    return LLVMConstInt(bool_type, node->value ? 1 : 0, 0);
}

LLVMValueRef visit_StringLiteral_impl(LLVMCodeGenerator* self, StringLiteralNode* node) {
    LLVMValueRef global_string_constant = LLVMConstString(node->value, strlen(node->value), 1);
    LLVMValueRef global_var = LLVMAddGlobal(self->module, 
        LLVMArrayType(LLVMInt8TypeInContext(self->context), strlen(node->value) + 1), 
        ".str");
    LLVMSetInitializer(global_var, global_string_constant);
    LLVMSetGlobalConstant(global_var, 1);
    LLVMSetLinkage(global_var, LLVMPrivateLinkage);

    LLVMValueRef indices[] = {
        LLVMConstInt(LLVMInt32TypeInContext(self->context), 0, 0),
        LLVMConstInt(LLVMInt32TypeInContext(self->context), 0, 0)
    };
    return LLVMBuildInBoundsGEP2(self->builder, 
        LLVMArrayType(LLVMInt8TypeInContext(self->context), strlen(node->value) + 1), 
        global_var, indices, 2, "stringPtr");
}
LLVMValueRef visit_UnaryOp_impl(LLVMCodeGenerator* self, UnaryOperationNode* node) {
    LLVMValueRef operand_val = node->operand->accept(node->operand, self);
    if (!operand_val) {
        fprintf(stderr, "Error: No se pudo generar el valor del operando para la operación unaria.\n");
        return NULL;
    }

    LLVMTypeRef type = LLVMTypeOf(operand_val);

    switch (node->operator) {
        case MINUS_TK:
            if (LLVMGetTypeKind(type) == LLVMDoubleTypeKind) {
                return LLVMBuildFNeg(self->builder, operand_val, "negateTmp");
            }
            fprintf(stderr, "Error: El operador '-' solo es aplicable a tipos numéricos (double).\n");
            return NULL;

        case NOT_TK:
            if (LLVMGetTypeKind(type) == LLVMIntegerTypeKind && LLVMGetIntTypeWidth(type) == 1) {
                return LLVMBuildNot(self->builder, operand_val, "notTmp");
            }
            fprintf(stderr, "Error: El operador '!' solo es aplicable a tipos booleanos.\n");
            return NULL;

        default:
            fprintf(stderr, "Error: Operador unario desconocido %d.\n", node->operator);
            return NULL;
    }
}

LLVMValueRef visit_BinaryOp_impl(LLVMCodeGenerator* self, BinaryOperationNode* node) {
    printf("[visit_BinaryOp_impl] Operación: %d\n", node->operator);
    LLVMValueRef left_val = node->left->accept(node->left, self);
    LLVMValueRef right_val = node->right->accept(node->right, self);

    printf("[visit_BinaryOp_impl] left_val: %p, right_val: %p\n", (void*)left_val, (void*)right_val);

    if (!left_val || !right_val) {
        fprintf(stderr, "Error: No se pudieron generar los valores de los operandos para la operación binaria.\n");
        return NULL;
    }
    
    LLVMTypeRef left_type = LLVMTypeOf(left_val);
    LLVMTypeRef right_type = LLVMTypeOf(right_val);

    char* left_type_str = LLVMPrintTypeToString(left_type);
    char* right_type_str = LLVMPrintTypeToString(right_type);
    printf("[visit_BinaryOp_impl] Tipo izquierdo: %s, Tipo derecho: %s\n", left_type_str, right_type_str);
    LLVMDisposeMessage(left_type_str);
    LLVMDisposeMessage(right_type_str);

    // --- Operaciones entre números (double) ---
    if (LLVMGetTypeKind(left_type) == LLVMDoubleTypeKind && 
        LLVMGetTypeKind(right_type) == LLVMDoubleTypeKind) {
        switch (node->operator) {
            case PLUS_TK:
                return LLVMBuildFAdd(self->builder, left_val, right_val, "addtmp");
            case MINUS_TK:
                return LLVMBuildFSub(self->builder, left_val, right_val, "subtmp");
            case MULT_TK:
                return LLVMBuildFMul(self->builder, left_val, right_val, "multmp");
            case DIV_TK:
                return LLVMBuildFDiv(self->builder, left_val, right_val, "divtmp");
            default:
                fprintf(stderr, "Error: Operador binario desconocido para números.\n");
                return NULL;
        }
    }

    // --- Operaciones entre booleanos (int1) ---
    if (LLVMGetTypeKind(left_type) == LLVMIntegerTypeKind && LLVMGetIntTypeWidth(left_type) == 1 &&
        LLVMGetTypeKind(right_type) == LLVMIntegerTypeKind && LLVMGetIntTypeWidth(right_type) == 1) {
        switch (node->operator) {
            case AND_TK:
                return LLVMBuildAnd(self->builder, left_val, right_val, "andtmp");
            case OR_TK:
                return LLVMBuildOr(self->builder, left_val, right_val, "ortmp");
            default:
                fprintf(stderr, "Error: Operador binario desconocido para booleanos.\n");
                return NULL;
        }
    }

    // --- Manejo de errores: Tipos incompatibles ---
    fprintf(stderr, "Error: Operador binario no soportado o tipos incompatibles.\n");
    char* ltype = LLVMPrintTypeToString(left_type);
    char* rtype = LLVMPrintTypeToString(right_type);
    fprintf(stderr, "  Tipo izquierdo: %s\n", ltype);
    fprintf(stderr, "  Tipo derecho: %s\n", rtype);
    LLVMDisposeMessage(ltype);
    LLVMDisposeMessage(rtype);

    return NULL;
}