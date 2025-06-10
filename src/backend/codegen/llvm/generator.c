// En codegen/llvm_codegen.c
#include "visitors.h"
#include "scope_stack.h"
#include "../../../frontend/ast/ast.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// --- Funciones de inicialización y limpieza del generador de código ---
LLVMCodeGenerator* create_llvm_code_generator(const char* module_name) {
    LLVMCodeGenerator* generator = (LLVMCodeGenerator*)malloc(sizeof(LLVMCodeGenerator));
    if (!generator) {
        perror("Failed to allocate LLVMCodeGenerator");
        exit(EXIT_FAILURE);
    }

    // Inicializar LLVM Context, Module y Builder
    generator->context = LLVMContextCreate();
    generator->module = LLVMModuleCreateWithNameInContext(module_name, generator->context);
    generator->builder = LLVMCreateBuilderInContext(generator->context);
    
    // Inicializar el stack de ambitos
    generator->scope_stack = create_scope_stack();
    if (!generator->scope_stack) {
        fprintf(stderr, "Error al crear el stack de ambitos.\n");
        destroy_llvm_code_generator(generator);
        return NULL;
    }

    // Asignar las implementaciones de los metodos visit_
    generator->visit_Literal = visit_Literal_impl;
    generator->visit_UnaryOp = visit_UnaryOp_impl;
    generator->visit_BinaryOp = visit_BinaryOp_impl;
    generator->visit_Let = visit_Let_impl;
    generator->visit_Variable = visit_Variable_impl;
    generator->visit_Conditional = visit_Conditional_impl;
    generator->visit_ExpressionBlock = visit_ExpressionBlock_impl;
    generator->visit_FunctionDefinition = visit_FunctionDefinition_impl;
    generator->visit_FunctionCall = visit_FunctionCall_impl;
    generator->declare_FunctionHeaders_impl = declare_FunctionHeaders_impl;
    generator->define_FunctionBodies_impl = define_FunctionBodies_impl;

    return generator;
}

void destroy_llvm_code_generator(LLVMCodeGenerator* generator) {
    if (!generator) return;

    // Destruir el stack de ambitos
    destroy_scope_stack(generator->scope_stack);

    LLVMDisposeBuilder(generator->builder);
    if (generator->module) {
        LLVMDisposeModule(generator->module);
    }
    LLVMContextDispose(generator->context);
    free(generator);
}

// --- Funcion principal para generar codigo ---
LLVMModuleRef generate_code(ProgramNode* program, LLVMCodeGenerator* generator) {
    printf("Entrando a generate_code\n");
    if (!program) {
        printf("El root es NULL\n");
        return NULL;
    }
    if (!generator) {
        fprintf(stderr, "Error: generator es NULL.\n");
        return NULL;
    }
    printf("Declarando encabezados de funciones\n");
    declare_FunctionHeaders_impl(generator, program->function_list);

    //Generar el cuerpo de la funcion main
    LLVMTypeRef double_type = LLVMDoubleTypeInContext(generator->context);
    LLVMTypeRef main_fn_type = LLVMFunctionType(double_type, NULL, 0, 0);
    LLVMValueRef main_fn = LLVMAddFunction(generator->module, "main", main_fn_type);
    LLVMBasicBlockRef entry_block = LLVMAppendBasicBlockInContext(generator->context, main_fn, "entry");

    define_FunctionBodies_impl(generator, program->function_list);

    LLVMPositionBuilderAtEnd(generator->builder, entry_block);
    
    printf("Llamando a accept del nodo raíz (tipo %d)\n", program->root->type);
    LLVMValueRef program_result = program->root->accept(program->root, generator);

    if (program_result) {
        LLVMBuildRet(generator->builder, program_result);
    } else {
        fprintf(stderr, "Error: El nodo raiz no produjo ningun valor.\n");
        LLVMBuildRet(generator->builder, LLVMConstReal(double_type, 0.0));
    }

    // Verificar el modulo generado
    char* error = NULL;
    if (LLVMVerifyModule(generator->module, LLVMPrintMessageAction, &error) != 0) {
        fprintf(stderr, "Error de verificacion del modulo LLVM:\n%s\n", error);
        LLVMDisposeMessage(error);
        return NULL;
    }
    LLVMDisposeMessage(error);

    return generator->module;
}