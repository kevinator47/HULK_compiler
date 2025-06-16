#include "visitors.h"
#include "../../../frontend/hulk_type/type_table.h"
#include "type_scope_stack.h"
#include "utils.h"
#include <stdio.h>
#include "builtins.h"
#include <llvm-c/Target.h>

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
    printf("[visit_BinaryOp_impl] node: %p\n", (void*)node);
    printf("[visit_BinaryOp_impl] node->operator: %d\n", node->operator);
    printf("[visit_BinaryOp_impl] node->left: %p\n", (void*)node->left);
    printf("[visit_BinaryOp_impl] node->right: %p\n", (void*)node->right);
    if (!node->left || !node->left->accept) {
    fprintf(stderr, "Error: Nodo izquierdo o su método accept es NULL en BinaryOp.\n");
    fprintf(stderr, "Tipo de nodo izquierdo: %d\n", node->left ? node->left->type : -1);
    abort();
    }
    LLVMValueRef left_val = node->left->accept(node->left, self);
    LLVMValueRef right_val = node->right->accept(node->right, self);

    if (LLVMGetTypeKind(LLVMTypeOf(left_val)) == LLVMPointerTypeKind)
         left_val = LLVMBuildLoad2(self->builder, LLVMGetElementType(LLVMTypeOf(left_val)), left_val, "loadtmp");
    if (LLVMGetTypeKind(LLVMTypeOf(right_val)) == LLVMPointerTypeKind)
        right_val = LLVMBuildLoad2(self->builder, LLVMGetElementType(LLVMTypeOf(right_val)), right_val, "loadtmp");

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
        if (LLVMGetTypeKind(LLVMTypeOf(then_val)) == LLVMPointerTypeKind)
            then_val = LLVMBuildLoad2(self->builder, LLVMGetElementType(LLVMTypeOf(then_val)), then_val, "loadtmp");
        if (LLVMGetTypeKind(LLVMTypeOf(else_val)) == LLVMPointerTypeKind)
            else_val = LLVMBuildLoad2(self->builder, LLVMGetElementType(LLVMTypeOf(else_val)), else_val, "loadtmp");

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
        VariableAssigment* assign = node->assigments[i]->assigment;
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

LLVMValueRef visit_WhileLoop_impl(LLVMCodeGenerator* self, WhileLoopNode* node) {
    LLVMValueRef function = LLVMGetBasicBlockParent(LLVMGetInsertBlock(self->builder));
    LLVMBasicBlockRef cond_bb = LLVMAppendBasicBlockInContext(self->context, function, "while.cond");
    LLVMBasicBlockRef body_bb = LLVMAppendBasicBlockInContext(self->context, function, "while.body");
    LLVMBasicBlockRef after_bb = LLVMAppendBasicBlockInContext(self->context, function, "while.after");

    // Salto a la condición
    LLVMBuildBr(self->builder, cond_bb);

    // Condición
    LLVMPositionBuilderAtEnd(self->builder, cond_bb);
    LLVMValueRef cond_val = node->condition->accept(node->condition, self);
    if (LLVMGetTypeKind(LLVMTypeOf(cond_val)) == LLVMDoubleTypeKind) {
        cond_val = LLVMBuildFCmp(
            self->builder, LLVMRealONE, cond_val,
            LLVMConstReal(LLVMDoubleTypeInContext(self->context), 0.0),
            "whilecond"
        );
    }
    LLVMBuildCondBr(self->builder, cond_val, body_bb, after_bb);

    // Cuerpo del while
    LLVMPositionBuilderAtEnd(self->builder, body_bb);
    LLVMValueRef body_val = node->body->accept(node->body, self);
    LLVMBuildBr(self->builder, cond_bb);

    // Después del while
    LLVMPositionBuilderAtEnd(self->builder, after_bb);

    return LLVMConstNull(LLVMDoubleTypeInContext(self->context)); // O el valor que quieras retornar
}

LLVMValueRef visit_ExpressionBlock_impl(LLVMCodeGenerator* self, ExpressionBlockNode* node) {
    LLVMValueRef last_val = NULL;
    for (int i = 0; i < node->expression_count; ++i) {
        last_val = node->expressions[i]->accept(node->expressions[i], self);
    }
    if (last_val) {
        LLVMTypeRef t = LLVMTypeOf(last_val);
        if (LLVMGetTypeKind(t) == LLVMVoidTypeKind) {
            return NULL;
        }
        if (LLVMGetTypeKind(t) == LLVMPointerTypeKind &&
            LLVMGetElementType(t) != LLVMInt8TypeInContext(self->context)) {
            last_val = LLVMBuildLoad2(self->builder, LLVMGetElementType(t), last_val, "loadtmp");
        }
    }
    return last_val;
}

LLVMValueRef visit_Variable_impl(LLVMCodeGenerator* self, VariableNode* node) {
    IrSymbolTable* current = current_scope(self->scope_stack);
    IrSymbol* symbol = lookup_ir_symbol(current, node->name);

    if (symbol) {
        LLVMTypeRef t = LLVMTypeOf(symbol->value);
        if (LLVMGetTypeKind(t) == LLVMPointerTypeKind &&
            LLVMGetElementType(t) == LLVMInt8TypeInContext(self->context)) {
            return symbol->value;
        }
        if (LLVMGetTypeKind(t) == LLVMPointerTypeKind) {
            return LLVMBuildLoad2(self->builder, LLVMGetElementType(t), symbol->value, node->name);
        }
        return symbol->value;
    }

    // --- Si no está en el scope local, se busca en self (campos del struct) ---
    IrSymbol* self_symbol = lookup_ir_symbol(current, "self");
    if (self_symbol) {
        LLVMValueRef self_ptr = self_symbol->value;
        TypeDescriptor* type = current_type(self->type_scope_stack);
        if (!type) {
            fprintf(stderr, "Error: No hay tipo actual en la pila de tipos para acceder a campos.\n");
            return NULL;
        }
        int field_index = -1;
        int struct_field_idx = 0;
        SymbolTable* scope = type->info->scope;
        for (int i = 0; i < scope->size; ++i) {
            Symbol* sym = scope->symbols[i];
            if (sym->kind == SYMBOL_TYPE_FIELD) {
                if (strcmp(sym->name, node->name) == 0) {
                    field_index = struct_field_idx;
                    break;
                }
                struct_field_idx+=1;
            }
        }
        printf("DEBUG: type=%p, type->llvm_type=%p, self_ptr=%p, field_index=%d, node->name=%s\n",
            (void*)type, (void*)type->llvm_type, (void*)self_ptr, field_index, node->name);
        if (field_index != -1) {
            LLVMValueRef self_val = LLVMBuildLoad2(self->builder, LLVMPointerType(type->llvm_type, 0), self_ptr, "self_val"); // %Point*
            LLVMValueRef field_ptr = LLVMBuildStructGEP2(self->builder, type->llvm_type, self_val, field_index, node->name);
            return LLVMBuildLoad2(self->builder, get_llvm_type_from_descriptor(scope->symbols[field_index]->type, self), field_ptr, node->name);
        }
    }

    fprintf(stderr, "Error: Variable o campo '%s' no encontrado en el ámbito actual ni en self.\n", node->name);
    return NULL;
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
        Symbol* param_symbol = lookup_symbol(node->scope, node->params[i]->name, SYMBOL_ANY ,true);
        if (!param_symbol) {
            fprintf(stderr, "Error: Símbolo de parámetro '%s' no encontrado en el scope de la función '%s'.\n", node->params[i]->name, node->name);
            free(param_types);
            return NULL;
        }
        param_types[i] = get_llvm_type_from_descriptor(param_symbol->type, self);
    }
    Symbol* fn_symbol = lookup_symbol(node->scope, node->name, SYMBOL_ANY, true);

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
    LLVMTypeRef ret_type = get_llvm_type_from_descriptor(fn_symbol->type, self);

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
        Symbol* function_symbol = lookup_symbol(fn_node->scope, fn_node->name, SYMBOL_ANY, true);
        if(fn_node == NULL) {
            fprintf(stderr, "Error: Nodo de función nulo en declare_FunctionHeaders_impl.\n");
            continue;
        }
        if (fn_node->scope == NULL || fn_node->scope->symbols == NULL || fn_node->param_count < 0) {
            fprintf(stderr, "Error: Tipo de scope para la función '%s'.\n", fn_node->name);
            return;
        }
        LLVMTypeRef* param_types = malloc(fn_node->param_count * sizeof(LLVMTypeRef));
        for (int j = 0; j < fn_node->param_count; ++j) {
            Symbol* param_symbol = lookup_symbol(fn_node->scope, fn_node->params[j]->name, SYMBOL_ANY, true);
            if (param_symbol == NULL) {
                fprintf(stderr, "Error: Símbolo de parámetro '%s' no encontrado en el scope de la función '%s'.\n", fn_node->params[j]->name, fn_node->name);
                free(param_types);
                return;
            }
            printf("param_symbol->type ptr: %p\n", (void*)param_symbol->type);
            param_types[j] = get_llvm_type_from_descriptor(param_symbol->type, self);
        }
        if(fn_node->static_return_type == NULL) {
            fprintf(stderr, "Error: Tipo de retorno estático nulo para la función '%s'.\n", fn_node->name);
            free(param_types);
            return;
        }
        if(fn_node->base.return_type == NULL) {
            fprintf(stderr, "Error: Tipo de retorno nulo para la función '%s'.\n", fn_node->name);
            free(param_types);
            return;
        }
        printf("La funcion devuelve: %s\n", function_symbol->type->type_name);
        printf("El tipo de retorno de la función %s es %s\n", fn_node->name, function_symbol->type->type_name);
        LLVMTypeRef ret_type = get_llvm_type_from_descriptor(function_symbol->type, self);
        if (!ret_type) {
            fprintf(stderr, "Error: ret_type es NULL para la función '%s'.\n", fn_node->name);
            free(param_types);
            return;
        }
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

void declare_method_signature_impl(LLVMCodeGenerator* self, TypeDescriptor* type, FunctionDefinitionNode* fn) {
    // El primer parámetro es self (puntero al struct)
    printf("[declare] fn=%p, fn->name='%s'\n", (void*)fn, fn->name ? fn->name : "(null)");
    Symbol* fn_symbol = lookup_symbol(fn->scope, fn->name, SYMBOL_ANY, true);
    int total_params = fn->param_count + 1;
    LLVMTypeRef* param_types = malloc(sizeof(LLVMTypeRef) * total_params);
    param_types[0] = LLVMPointerType(type->llvm_type, 0); // self
    for (int i = 0; i < fn->param_count; ++i) {
        Symbol* param_symbol = lookup_symbol(fn->scope, fn->params[i]->name, SYMBOL_ANY, true);
        param_types[i+1] = get_llvm_type_from_descriptor(param_symbol->type, self);
    }
    LLVMTypeRef ret_type = get_llvm_type_from_descriptor(fn_symbol->type, self);
    char method_name[256];
    snprintf(method_name, sizeof(method_name), "%s_%s", type->type_name, fn->name);

    printf("[declare_method_signature_impl] Declarando método: %s\n", method_name);

    LLVMTypeRef fn_type = LLVMFunctionType(ret_type, param_types, total_params, 0);
    LLVMAddFunction(self->module, method_name, fn_type);
    free(param_types);
}

void define_method_body_impl(LLVMCodeGenerator* self, TypeDescriptor* type, FunctionDefinitionNode* fn) {
    push_type(self->type_scope_stack, type);
    printf("[define ] fn=%p, fn->name='%s'\n", (void*)fn, fn->name ? fn->name : "(null)");
    char method_name[256];
    snprintf(method_name, sizeof(method_name), "%s_%s", type->type_name, fn->name);

    printf("[define_method_body_impl] Definiendo método: %s\n", method_name);

    Symbol* fn_symbol = lookup_symbol(fn->scope, fn->name, SYMBOL_ANY, true);
    LLVMValueRef llvm_fn = LLVMGetNamedFunction(self->module, method_name);
    if (!llvm_fn) {
        fprintf(stderr, "Error: método '%s' no declarado.\n", method_name);
        return;
    }

    // Prepara el scope para el método
    IrSymbolTable* method_scope = create_ir_symbol_table(0, current_scope(self->scope_stack));
    push_scope(self->scope_stack, method_scope);

    LLVMBasicBlockRef entry = LLVMAppendBasicBlockInContext(self->context, llvm_fn, "entry");
    LLVMPositionBuilderAtEnd(self->builder, entry);    

    // Mapea el parámetro self (primer parámetro del método)
    LLVMValueRef self_param = LLVMGetParam(llvm_fn, 0);
    LLVMTypeRef self_type = LLVMPointerType(type->llvm_type, 0);
    LLVMValueRef self_alloca = LLVMBuildAlloca(self->builder, self_type, "self");
    LLVMBuildStore(self->builder, self_param, self_alloca);
    insert_ir_symbol(method_scope, "self", self_alloca);

    // Mapea los parámetros del método (a partir del segundo parámetro)
    for (int i = 0; i < fn->param_count; ++i) {
        LLVMValueRef param = LLVMGetParam(llvm_fn, i + 1);
        Symbol* param_symbol = lookup_symbol(fn->scope, fn->params[i]->name, SYMBOL_ANY, true);
        LLVMTypeRef param_type = get_llvm_type_from_descriptor(param_symbol->type, self);
        LLVMValueRef alloca = LLVMBuildAlloca(self->builder, param_type, fn->params[i]->name);
        LLVMBuildStore(self->builder, param, alloca);
        insert_ir_symbol(method_scope, fn->params[i]->name, alloca);
    }

    LLVMValueRef body_val = fn->body->accept(fn->body, self);

    LLVMTypeRef ret_type = get_llvm_type_from_descriptor(fn_symbol->type, self);

    // Retorna el valor adecuado
    if (ret_type == LLVMVoidTypeInContext(self->context)) {
        LLVMBuildRetVoid(self->builder);
    } else if (body_val) {
        LLVMBuildRet(self->builder, body_val);
    } else {
        LLVMBuildRet(self->builder, LLVMConstNull(ret_type));
    }

    pop_scope(self->scope_stack);
    pop_type(self->type_scope_stack);
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
        if (LLVMGetTypeKind(LLVMTypeOf(args[i])) == LLVMPointerTypeKind)
            args[i] = LLVMBuildLoad2(self->builder, LLVMGetElementType(LLVMTypeOf(args[i])), args[i], "loadtmp");
        if (!args[i]) {
            fprintf(stderr, "Error: No se pudo generar el argumento %d para la función '%s'.\n", i, node->name);
            free(args);
            return NULL;
        }
    }
    LLVMTypeRef fn_type = LLVMGetElementType(LLVMTypeOf(fn));
    LLVMTypeRef ret_type = LLVMGetReturnType(fn_type);
    if (ret_type == LLVMVoidTypeInContext(self->context)) {
        LLVMBuildCall2(self->builder, fn_type, fn, args, node->arg_count, "");
        free(args);
        return NULL; // No hay valor de retorno
    } else {
        LLVMValueRef call = LLVMBuildCall2(self->builder, fn_type, fn, args, node->arg_count, "calltmp");
        free(args);
        return call;
    }
}

LLVMValueRef visit_NewNode_impl(LLVMCodeGenerator* self, NewNode* node) {
    TypeDescriptor* desc = type_table_lookup(self->type_table, node->type_name);
    printf("Instanciando: %s, desc=%p, llvm_type=%p, kind=%d\n",
        desc->type_name, (void*)desc, (void*)desc->llvm_type, LLVMGetTypeKind(desc->llvm_type));
    if (!desc) {
        fprintf(stderr, "Error: Tipo '%s' no encontrado en NewNode.\n", node->type_name);
        return NULL;
    }
    if (desc->llvm_type == NULL || !desc->llvm_type) {
        fprintf(stderr, "Error: Tipo '%s' no tiene un tipo LLVM asociado.\n", node->type_name);
        return NULL;
    }
    printf("Creando instancia de tipo: %s\n", desc->type_name);
    printf("desc->llvm_type: %p\n", (void*)desc->llvm_type);
    printf("LLVMGetTypeKind(desc->llvm_type): %d\n", LLVMGetTypeKind(desc->llvm_type));
    LLVMTypeRef struct_type = desc->llvm_type;
    LLVMTypeRef i64_type = LLVMInt64TypeInContext(self->context);
    printf("struct_type: %p\n", (void*)struct_type);
    if (LLVMGetTypeKind(struct_type) != LLVMStructTypeKind || LLVMCountStructElementTypes(struct_type) == 0) {
        fprintf(stderr, "Error: struct_type aún no tiene un cuerpo definido.\n");
        exit(1);
    }
    if (LLVMCountStructElementTypes(struct_type) == 0) {
        fprintf(stderr, "Error: struct_type '%s' está sin definir.\n", desc->type_name);
        exit(1);
    }
    LLVMTargetDataRef data_layout = LLVMGetModuleDataLayout(self->module);
    if (!data_layout) {
        fprintf(stderr, "Error: No se pudo obtener el layout de datos del módulo.\n");
        exit(1);
    }

    // Obtener tamaño en bytes
    uint64_t struct_size_bytes = LLVMStoreSizeOfType(data_layout, struct_type);

    // Crear valor LLVM con tamaño
    LLVMValueRef struct_size = LLVMConstInt(i64_type, struct_size_bytes, 0);    
    LLVMValueRef malloc_fn = LLVMGetNamedFunction(self->module, "malloc");
    LLVMValueRef raw_ptr = LLVMBuildCall2(self->builder, LLVMGetElementType(LLVMTypeOf(malloc_fn)), malloc_fn, &struct_size, 1, "malloc_call");
    LLVMValueRef instance = LLVMBuildBitCast(self->builder, raw_ptr, LLVMPointerType(struct_type, 0), "instance");

    SymbolTable* scope = desc->info->scope;
    TypeDefinitionNode* type_def = desc->info->type_def;
    ExpressionBlockNode* body = type_def->body;

    int field_index = 0;
    for (int i = 0; i < scope->size; ++i) {
        Symbol* field_sym = scope->symbols[i];
        if (field_sym->kind != SYMBOL_TYPE_FIELD || is_self_instance(field_sym->name)) continue;
        char* field_name = field_sym->name;
        printf("Procesando campo: %s\n", field_name);

        // Busca VariableAssignmentNode para este campo en el body del tipo
        VariableAssigmentNode* assign_node = NULL;
        for (int j = 0; j < body->expression_count; ++j) {
            if (body->expressions[j]->type == AST_Node_Variable_Assigment) {
                VariableAssigmentNode* va = (VariableAssigmentNode*)body->expressions[j];
                if (strcmp(va->assigment->name, field_name) == 0) {
                    assign_node = va;
                    break;
                }
            }
        }
        printf("Asignación encontrada: %s\n", assign_node ? "Sí" : "No");
        LLVMValueRef value_to_store = NULL;
        if (assign_node) {
            ASTNode* rhs = assign_node->assigment->value;
            // Si el valor es un parámetro, usa el argumento recibido
            if (rhs->type == AST_Node_Variable) {
                VariableNode* var = (VariableNode*)rhs;
                int param_index = -1;
                for (int k = 0; k < type_def->param_count; ++k) {
                    if (strcmp(type_def->params[k]->name, var->name) == 0) {
                        param_index = k;
                        break;
                    }
                }
                if (param_index != -1 && param_index < node->arg_count) {
                    value_to_store = node->args[param_index]->accept(node->args[param_index], self);
                } else {
                    value_to_store = rhs->accept(rhs, self);
                }
            } else {
                value_to_store = rhs->accept(rhs, self);
            }
        } else {
            LLVMTypeRef field_type = get_llvm_type_from_descriptor(field_sym->type, self);
            value_to_store = LLVMConstNull(field_type);
        }
        if (!value_to_store) {
            fprintf(stderr, "Error: No se pudo generar el valor para el campo '%s'.\n", field_name);
            continue;
        }

        LLVMValueRef field_ptr = LLVMBuildStructGEP2(self->builder, struct_type, instance, field_index, field_name);
        LLVMBuildStore(self->builder, value_to_store, field_ptr);
        field_index += 1;
    }

    return instance;
}

LLVMValueRef visit_AttributeAccess_impl(LLVMCodeGenerator* self, AttributeAccessNode* node) {
    // Evalúa el objeto (instancia)
    LLVMValueRef obj_val = node->object->accept(node->object, self);

    // Obtén el tipo de la instancia
    TypeDescriptor* obj_type = node->object->return_type;
    if (!obj_type || obj_type->tag != HULK_Type_UserDefined) {
        fprintf(stderr, "Error: acceso a atributo/método en tipo no soportado.\n");
        return NULL;
    }

    if (!node->is_method_call) {
        // --- ACCESO A ATRIBUTO ---
        // Busca el índice del campo
        int field_index = -1;
        SymbolTable* scope = obj_type->info->scope;
        for (int i = 0, idx = 0; i < scope->size; ++i) {
            Symbol* sym = scope->symbols[i];
            if (sym->kind == SYMBOL_TYPE_FIELD && strcmp(sym->name, node->attribute_name) == 0) {
                field_index = idx;
                break;
            }
            if (sym->kind == SYMBOL_TYPE_FIELD) idx++;
        }
        if (field_index == -1) {
            fprintf(stderr, "Error: atributo '%s' no encontrado en tipo '%s'.\n", node->attribute_name, obj_type->type_name);
            return NULL;
        }
        LLVMValueRef gep = LLVMBuildStructGEP2(self->builder, obj_type->llvm_type, obj_val, field_index, node->attribute_name);
        return LLVMBuildLoad2(self->builder, get_llvm_type_from_descriptor(scope->symbols[field_index]->type, self), gep, "");
    } else {
        // --- LLAMADA A MÉTODO ---
        // Construye el nombre del método
        char method_name[256];
        snprintf(method_name, sizeof(method_name), "%s_%s", obj_type->type_name, node->attribute_name);
        LLVMValueRef fn = LLVMGetNamedFunction(self->module, method_name);
        if (!fn) {
            fprintf(stderr, "Error: método '%s' no encontrado en tipo '%s'.\n", node->attribute_name, obj_type->type_name);
            return NULL;
        }
        LLVMTypeRef fn_type = LLVMGetElementType(LLVMTypeOf(fn));
        int total_args = node->arg_count + 1;
        LLVMValueRef* args = malloc(sizeof(LLVMValueRef) * total_args);
        args[0] = obj_val; // self
        for (int i = 0; i < node->arg_count; ++i) {
            args[i+1] = node->args[i]->accept(node->args[i], self);
        }
        LLVMValueRef call = LLVMBuildCall2(self->builder, fn_type, fn, args, total_args, "");
        free(args);
        return call;
    }
}