#include <llvm-c/Core.h>
#include "../../../frontend/ast/ast.h"
#include "utils.h"
#include "generator.h"

LLVMValueRef emit_builtin_print(LLVMCodeGenerator* self, FunctionCallNode* node) {
    LLVMValueRef printf_func = LLVMGetNamedFunction(self->module, "printf");
    LLVMValueRef arg = node->arg_count > 0 ? node->args[0]->accept(node->args[0], self) : NULL;
    LLVMValueRef fmt;
    LLVMValueRef args[2];
    int num_args = 1;

    if (!arg) {
        fmt = LLVMBuildGlobalStringPtr(self->builder, "\n", "fmt");
        args[0] = fmt;
    } else {
        LLVMTypeRef arg_type = LLVMTypeOf(arg);
        const char* format = get_print_format(arg_type, self->context);
        fmt = LLVMBuildGlobalStringPtr(self->builder, format, "fmt");
        args[0] = fmt;
        if (LLVMGetTypeKind(arg_type) == LLVMIntegerTypeKind && LLVMGetIntTypeWidth(arg_type) == 1) {
            LLVMValueRef true_str = LLVMBuildGlobalStringPtr(self->builder, "true", "true_str");
            LLVMValueRef false_str = LLVMBuildGlobalStringPtr(self->builder, "false", "false_str");
            LLVMValueRef cond_str = LLVMBuildSelect(self->builder, arg, true_str, false_str, "bool_str");
            args[1] = cond_str;
        } else if (LLVMGetTypeKind(arg_type) == LLVMPointerTypeKind || LLVMGetTypeKind(arg_type) == LLVMDoubleTypeKind) {
            args[1] = arg;
        } else {
            LLVMValueRef unknown_str = LLVMBuildGlobalStringPtr(self->builder, "<unknown>", "unknown_str");
            args[1] = unknown_str;
        }
        num_args = 2;
    }
    LLVMTypeRef printf_type = LLVMFunctionType(LLVMInt32TypeInContext(self->context),
        (LLVMTypeRef[]){LLVMPointerType(LLVMInt8TypeInContext(self->context), 0)}, 1, 1);
    return LLVMBuildCall2(self->builder, printf_type, printf_func, args, num_args, "");
}

LLVMValueRef generate_builtin_function(LLVMCodeGenerator* self, FunctionCallNode* node) {
    printf("generate_builtin_function: name = %s\n", node->name);
    switch (get_builtin_kind(node->name)) {
        case BUILTIN_PRINT:
            return emit_builtin_print(self, node);
        case BUILTIN_SQRT:
        case BUILTIN_SIN:
        case BUILTIN_COS:
        case BUILTIN_EXP:
        case BUILTIN_LOG: {
            LLVMValueRef arg = node->arg_count > 0 ? node->args[0]->accept(node->args[0], self) : NULL;
            LLVMValueRef fn = LLVMGetNamedFunction(self->module, node->name);
            LLVMTypeRef fn_type = LLVMFunctionType(LLVMDoubleTypeInContext(self->context),
                (LLVMTypeRef[]){LLVMDoubleTypeInContext(self->context)}, 1, 0);
            return LLVMBuildCall2(self->builder, fn_type, fn, &arg, 1, "");
        }
        case BUILTIN_POW:
        case BUILTIN_FMOD: {
            if (node->arg_count < 2) return NULL;
            LLVMValueRef arg1 = node->args[0]->accept(node->args[0], self);
            LLVMValueRef arg2 = node->args[1]->accept(node->args[1], self);
            LLVMValueRef fn = LLVMGetNamedFunction(self->module, node->name);
            LLVMTypeRef fn_type = LLVMFunctionType(LLVMDoubleTypeInContext(self->context),
                (LLVMTypeRef[]){LLVMDoubleTypeInContext(self->context), LLVMDoubleTypeInContext(self->context)}, 2, 0);
            LLVMValueRef args[2] = {arg1, arg2};
            return LLVMBuildCall2(self->builder, fn_type, fn, args, 2, "");
        }
        case BUILTIN_RAND: {
            LLVMValueRef fn = LLVMGetNamedFunction(self->module, "rand");
            LLVMTypeRef fn_type = LLVMFunctionType(LLVMInt32TypeInContext(self->context), NULL, 0, 0);
            LLVMValueRef rand_val = LLVMBuildCall2(self->builder, fn_type, fn, NULL, 0, "");
            LLVMValueRef rand_double = LLVMBuildSIToFP(self->builder, rand_val, LLVMDoubleTypeInContext(self->context), "");
            LLVMValueRef rand_max = LLVMConstReal(LLVMDoubleTypeInContext(self->context), RAND_MAX);
            return LLVMBuildFDiv(self->builder, rand_double, rand_max, "");
        }
        default:
            return NULL;
    }
}