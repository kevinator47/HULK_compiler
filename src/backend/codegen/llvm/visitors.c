#include "visitors.h"
#include "utils.h"
#include <stdio.h>
#include "builtins.h"

// --- Implementaciones de los métodos visit_ para Expresiones ---

LLVMValueRef visit_Literal_impl(LLVMCodeGenerator* self, LiteralNode* node) {
    switch (node->base.return_type->tag) {
        case HULK_Type_Number:
            return LLVMConstReal(LLVMDoubleTypeInContext(self->context), node->value.number_value);

        case HULK_Type_Boolean:
            return LLVMConstInt(LLVMInt1TypeInContext(self->context), node->value.bool_value ? 1 : 0, 0);

        case HULK_Type_String:
            return LLVMBuildGlobalStringPtr(self->builder, node->value.string_value, "str");

        default:
            fprintf(stderr, "Error: Tipo de literal no soportado en visit_Literal_impl.\n");
            return NULL;
    }
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
            case PLUS_TK:    return LLVMBuildFAdd(self->builder, left_val, right_val, "addtmp");
            case MINUS_TK:   return LLVMBuildFSub(self->builder, left_val, right_val, "subtmp");
            case MULT_TK:    return LLVMBuildFMul(self->builder, left_val, right_val, "multmp");
            case DIV_TK:     return LLVMBuildFDiv(self->builder, left_val, right_val, "divtmp");
            case MOD_TK:     return LLVMBuildFRem(self->builder, left_val, right_val, "modtmp");
            case EXP_TK:     // Exponenciación
                {
                    LLVMValueRef pow_fn = LLVMGetNamedFunction(self->module, "llvm.pow.f64");
                    if (!pow_fn) {
                        LLVMTypeRef args[2] = {LLVMDoubleTypeInContext(self->context), LLVMDoubleTypeInContext(self->context)};
                        LLVMTypeRef pow_type = LLVMFunctionType(LLVMDoubleTypeInContext(self->context), args, 2, 0);
                        pow_fn = LLVMAddFunction(self->module, "llvm.pow.f64", pow_type);
                    }
                    LLVMValueRef args[2] = {left_val, right_val};
                    return LLVMBuildCall2(self->builder, LLVMGetElementType(LLVMTypeOf(pow_fn)), pow_fn, args, 2, "exptmp");
                }
            case GT_TK:  return LLVMBuildFCmp(self->builder, LLVMRealOGT, left_val, right_val, "gttmp");
            case GE_TK:  return LLVMBuildFCmp(self->builder, LLVMRealOGE, left_val, right_val, "getmp");
            case LT_TK:  return LLVMBuildFCmp(self->builder, LLVMRealOLT, left_val, right_val, "lttmp");
            case LE_TK:  return LLVMBuildFCmp(self->builder, LLVMRealOLE, left_val, right_val, "letmp");
            case EQ_TK:  return LLVMBuildFCmp(self->builder, LLVMRealOEQ, left_val, right_val, "eqtmp");
            case NE_TK:  return LLVMBuildFCmp(self->builder, LLVMRealONE, left_val, right_val, "netmp");
            default:
                fprintf(stderr, "Error: Operador binario desconocido para números (valor: %d).\n", node->operator);
                return NULL;
        }
    }

    // --- Operaciones entre booleanos (int1) ---
    if (LLVMGetTypeKind(left_type) == LLVMIntegerTypeKind && LLVMGetIntTypeWidth(left_type) == 1 &&
        LLVMGetTypeKind(right_type) == LLVMIntegerTypeKind && LLVMGetIntTypeWidth(right_type) == 1) {
        switch (node->operator) {
            case AND_TK: return LLVMBuildAnd(self->builder, left_val, right_val, "andtmp");
            case OR_TK:  return LLVMBuildOr(self->builder, left_val, right_val, "ortmp");
            case EQ_TK:  return LLVMBuildICmp(self->builder, LLVMIntEQ, left_val, right_val, "eqtmp");
            case NE_TK:  return LLVMBuildICmp(self->builder, LLVMIntNE, left_val, right_val, "netmp");
            default:
                fprintf(stderr, "Error: Operador binario desconocido para booleanos (valor: %d).\n", node->operator);
                return NULL;
        }
    }

    // --- Concatenación de strings (i8*) ---
    // Puedes implementar CONCAT_TK y D_CONCAT_TK aquí si tienes soporte de strings

    fprintf(stderr, "Error: Operador binario no soportado o tipos incompatibles.\n");
    char* ltype = LLVMPrintTypeToString(left_type);
    char* rtype = LLVMPrintTypeToString(right_type);
    fprintf(stderr, "  Tipo izquierdo: %s\n", ltype);
    fprintf(stderr, "  Tipo derecho: %s\n", rtype);
    LLVMDisposeMessage(ltype);
    LLVMDisposeMessage(rtype);

    return NULL;
}

LLVMValueRef visit_Conditional_impl(LLVMCodeGenerator* self, ConditionalNode* node) {
    LLVMValueRef cond_val = node->condition->accept(node->condition, self);
    if (!cond_val) {
        fprintf(stderr, "Error: No se pudo generar el valor de la condición.\n");
        return NULL;
    }

    LLVMTypeRef cond_type = LLVMTypeOf(cond_val);
    if (LLVMGetTypeKind(cond_type) == LLVMDoubleTypeKind) {
        cond_val = LLVMBuildFCmp(
            self->builder, LLVMRealONE, cond_val,
            LLVMConstReal(LLVMDoubleTypeInContext(self->context), 0.0),
            "ifcond"
        );
    }

    LLVMValueRef function = LLVMGetBasicBlockParent(LLVMGetInsertBlock(self->builder));
    LLVMBasicBlockRef then_bb = LLVMAppendBasicBlockInContext(self->context, function, "then");
    LLVMBasicBlockRef else_bb = LLVMAppendBasicBlockInContext(self->context, function, "else");
    LLVMBasicBlockRef merge_bb = LLVMAppendBasicBlockInContext(self->context, function, "ifcont");

    LLVMBuildCondBr(self->builder, cond_val, then_bb, else_bb);

    LLVMPositionBuilderAtEnd(self->builder, then_bb);
    LLVMValueRef then_val = node->then_branch->accept(node->then_branch, self);
    LLVMBuildBr(self->builder, merge_bb);
    then_bb = LLVMGetInsertBlock(self->builder); 

    LLVMPositionBuilderAtEnd(self->builder, else_bb);
    LLVMValueRef else_val = NULL;
    if (node->else_branch) {
        else_val = node->else_branch->accept(node->else_branch, self);
    }
    LLVMBuildBr(self->builder, merge_bb);
    else_bb = LLVMGetInsertBlock(self->builder); 

    LLVMPositionBuilderAtEnd(self->builder, merge_bb);

    if (then_val && else_val) {
        LLVMTypeRef phi_type = LLVMTypeOf(then_val);
        LLVMValueRef phi = LLVMBuildPhi(self->builder, phi_type, "iftmp");
        LLVMAddIncoming(phi, &then_val, &then_bb, 1);
        LLVMAddIncoming(phi, &else_val, &else_bb, 1);
        return phi;
    }

    if (then_val) return then_val;

    return NULL;
}

LLVMValueRef visit_Let_impl(LLVMCodeGenerator* self, LetInNode* node){
    IrSymbolTable* new_scope = create_ir_symbol_table(0, current_scope(self->scope_stack));
    push_scope(self->scope_stack, new_scope);

    LLVMBasicBlockRef current_block = LLVMGetInsertBlock(self->builder);
    LLVMValueRef current_func = LLVMGetBasicBlockParent(current_block);
    LLVMBasicBlockRef entry_block = LLVMGetEntryBasicBlock(current_func);

    printf("Visitando LetIn \n");
    printf("Analizando asignaciones \n");
    for (int i = 0; i < node->assigment_count; ++i) {
        VariableAssigment* assign = &node->assigments[i];
        char* name = assign->name;
        LLVMValueRef init_val = assign->value->accept(assign->value, self);
        LLVMTypeRef type = LLVMTypeOf(init_val);

        LLVMPositionBuilderAtEnd(self->builder, entry_block);
        LLVMValueRef alloca = LLVMBuildAlloca(self->builder, type, name);
        LLVMPositionBuilderAtEnd(self->builder, current_block); // Restaurar posición

        LLVMBuildStore(self->builder, init_val, alloca);
        insert_ir_symbol(current_scope(self->scope_stack), name, alloca);
    }

    if (node->body == NULL) {
        fprintf(stderr, "Error: El cuerpo (body) del let-in es NULL.\n");
        pop_scope(self->scope_stack);
        return NULL;
    }

    LLVMValueRef body_value = node->body->accept(node->body, self);
    pop_scope(self->scope_stack);
    return body_value;
}

LLVMValueRef visit_ExpressionBlock_impl(LLVMCodeGenerator* self, ExpressionBlockNode* node) {
    LLVMValueRef last_val = NULL;
    for (int i = 0; i < node->expression_count; ++i) {
        last_val = node->expressions[i]->accept(node->expressions[i], self);
    }
    return last_val;
}

LLVMValueRef visit_Variable_impl(LLVMCodeGenerator* self, VariableNode* node) {
    IrSymbolTable* current = current_scope(self->scope_stack);
    IrSymbol* symbol = lookup_ir_symbol(current, node->name);

    if (!symbol) {
        fprintf(stderr, "Error: Variable '%s' no encontrada en el ámbito actual.\n", node->name);
        return NULL;
    }

    return LLVMBuildLoad2(self->builder, LLVMTypeOf(symbol->value), symbol->value, node->name);
}

LLVMValueRef visit_ReassignNode_impl(LLVMCodeGenerator* self, ReassignNode* node){
    IrSymbol* symbol = lookup_ir_symbol(current_scope(self->scope_stack), node->name);
    if (!symbol) {
        fprintf(stderr, "Error: Variable '%s' no encontrada en el ámbito actual.\n", node->name);
        return NULL;
    }
    LLVMValueRef new_value = node->value->accept(node->value, self);
    if (!new_value) {
        fprintf(stderr, "Error: No se pudo generar el valor para la reasignación de la variable '%s'.\n", node->name);
        return NULL;
    }
    LLVMTypeRef type = LLVMTypeOf(new_value);
    if (LLVMGetTypeKind(type) != LLVMGetTypeKind(LLVMTypeOf(symbol->value))) {
        fprintf(stderr, "Error: Tipo de valor '%s' no coincide con el tipo de la variable.\n", node->name);
        return NULL;
    }
    LLVMBuildStore(self->builder, new_value, symbol->value);
    return symbol->value;
}

LLVMValueRef visit_FunctionDefinition_impl(LLVMCodeGenerator* self, FunctionDefinitionNode* node) {
    LLVMTypeRef* param_types = malloc(node->param_count * sizeof(LLVMTypeRef));
    for (int i = 0; i < node->param_count; ++i) {
        param_types[i] = get_llvm_type_from_descriptor(node->scope->symbols[i]->type, self->context);
    }

    LLVMTypeRef ret_type = LLVMDoubleTypeInContext(self->context); //tipo correcto

    LLVMValueRef fn = LLVMGetNamedFunction(self->module, node->name);
    if (!fn) {
        fprintf(stderr, "Error: función '%s' no declarada antes de definir su cuerpo.\n", node->name);
        free(param_types);
        return NULL;
    }

    LLVMTypeRef fn_type = LLVMGetElementType(LLVMTypeOf(fn));
    if (LLVMGetTypeKind(fn_type) != LLVMFunctionTypeKind) {
        fprintf(stderr, "Error: '%s' no es una función válida.\n", node->name);
        free(param_types);
        return NULL;
    }
    if (LLVMCountParams(fn) != node->param_count) {
        fprintf(stderr, "Error: Número de parámetros no coincide para la función '%s'.\n", node->name);
        free(param_types);
        return NULL;
    }

    LLVMBasicBlockRef entry = LLVMAppendBasicBlockInContext(self->context, fn, "entry");
    LLVMPositionBuilderAtEnd(self->builder, entry);

    IrSymbolTable* fn_scope = create_ir_symbol_table(0, current_scope(self->scope_stack));
    push_scope(self->scope_stack, fn_scope);

  for (int i = 0; i < node->param_count; ++i) {
        if (!param_types[i]) {
            fprintf(stderr, "Error: param_types[%d] es NULL en la función '%s'.\n", i, node->name);
            continue;
        }
        if (!node->params[i] || !node->params[i]->name) {
            fprintf(stderr, "Error: params[%d] o su nombre es NULL en la función '%s'.\n", i, node->name);
            continue;
        }
        LLVMValueRef param = LLVMGetParam(fn, i);
        if (!param) {
            fprintf(stderr, "Error: LLVMGetParam devolvió NULL para el parámetro %d de la función '%s'.\n", i, node->name);
            continue;
        }
        LLVMValueRef alloca = LLVMBuildAlloca(self->builder, param_types[i], node->params[i]->name);
        LLVMBuildStore(self->builder, param, alloca);
        insert_ir_symbol(fn_scope, node->params[i]->name, alloca);
    }

    LLVMBasicBlockRef body_bb = LLVMAppendBasicBlockInContext(self->context, fn, "body");
    LLVMBuildBr(self->builder, body_bb);
    LLVMPositionBuilderAtEnd(self->builder, body_bb);

    LLVMValueRef body_val = node->body->accept(node->body, self);

    if (ret_type == LLVMVoidTypeInContext(self->context)) {
        LLVMBuildRetVoid(self->builder);
    } else if (body_val) {
        LLVMBuildRet(self->builder, body_val);
    } else {
        printf("Error no se generó el cuerpo de la función: %s", node->name);
        LLVMBuildRet(self->builder, LLVMConstNull(ret_type));
    }

    pop_scope(self->scope_stack);
    free(param_types);
    return fn;
}

void declare_FunctionHeaders_impl(LLVMCodeGenerator* self, FunctionDefinitionListNode* node) {
    for (int i = 0; i < node->function_count; i++) {
        FunctionDefinitionNode* fn_node = node->functions[i];
        if(fn_node == NULL) {
            fprintf(stderr, "Error: Nodo de función nulo en declare_FunctionHeaders_impl.\n");
            continue;
        }
        if (fn_node->scope == NULL || fn_node->scope->symbols == NULL || fn_node->param_count <= 0) {
            fprintf(stderr, "Error: Tipo de scope para la función '%s'.\n", fn_node->name);
            return;
        }
        LLVMTypeRef* param_types = malloc(fn_node->param_count * sizeof(LLVMTypeRef));
        for (int j = 0; j < fn_node->param_count; ++j) {
            param_types[j] = get_llvm_type_from_descriptor(fn_node->scope->symbols[j]->type, self->context);
        }
        LLVMTypeRef ret_type = LLVMDoubleTypeInContext(self->context); // O el tipo correcto
        LLVMTypeRef fn_type = LLVMFunctionType(ret_type, param_types, fn_node->param_count, 0);
        LLVMAddFunction(self->module, fn_node->name, fn_type);
        free(param_types);
    }
}

void define_FunctionBodies_impl(LLVMCodeGenerator* self, FunctionDefinitionListNode* node) {
    for (int i = 0; i < node->function_count; i++) {
        visit_FunctionDefinition_impl(self, node->functions[i]);
    }
}

void declare_FunctionDefinitionList_impl(LLVMCodeGenerator* self, FunctionDefinitionListNode* node){
    for(int i = 0 ; i < node->function_count; i++){
        visit_FunctionDefinition_impl(self, node->functions[i]);
    }
}

LLVMValueRef visit_FunctionCall_impl(LLVMCodeGenerator* self, FunctionCallNode* node) {
    LLVMValueRef builtin_result = generate_builtin_function(self, node);
    if (builtin_result) return builtin_result;

    // Llamada a función de usuario
    LLVMValueRef fn = LLVMGetNamedFunction(self->module, node->name);
    if (!fn) {
        fprintf(stderr, "Error: función '%s' no encontrada en el módulo LLVM.\n", node->name);
        return NULL;
    }
    LLVMValueRef* args = malloc(node->arg_count * sizeof(LLVMValueRef));
    for (int i = 0; i < node->arg_count; ++i) {
        args[i] = node->args[i]->accept(node->args[i], self);
        if (!args[i]) {
            fprintf(stderr, "Error: No se pudo generar el argumento %d para la función '%s'.\n", i, node->name);
            free(args);
            return NULL;
        }
    }
    LLVMTypeRef fn_type = LLVMGetElementType(LLVMTypeOf(fn));
    LLVMValueRef call = LLVMBuildCall2(self->builder, fn_type, fn, args, node->arg_count, "calltmp");
    free(args);
    return call;
}