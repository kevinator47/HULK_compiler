// En codegen/llvm_codegen.c
#include "visitors.h"
#include "scope_stack.h"
#include "utils.h"
#include "../../../frontend/ast/ast.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// --- Funciones de inicialización y limpieza del generador de código ---
LLVMCodeGenerator* create_llvm_code_generator(const char* module_name, TypeTable* type_table) {
    LLVMCodeGenerator* generator = (LLVMCodeGenerator*)malloc(sizeof(LLVMCodeGenerator));
    if (!generator) {
        perror("Failed to allocate LLVMCodeGenerator");
        exit(EXIT_FAILURE);
    }

    // Inicializar LLVM Context, Module y Builder
    generator->context = LLVMContextCreate();
    generator->module = LLVMModuleCreateWithNameInContext(module_name, generator->context);
    generator->builder = LLVMCreateBuilderInContext(generator->context);
    generator->type_table = type_table;
    
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
    generator->visit_WhileLoop = visit_WhileLoop_impl;
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
    printf("Declarando funciones externas\n");
    declare_external_functions(generator->module, generator->context);
    printf("Declarando encabezados de funciones\n");
    declare_FunctionHeaders_impl(generator, program->function_list);

    //Generar el cuerpo de la funcion main
    LLVMTypeRef double_type = LLVMDoubleTypeInContext(generator->context);
    LLVMTypeRef main_fn_type = LLVMFunctionType(get_llvm_type_from_descriptor(program->root->return_type, generator), NULL, 0, 0);
    LLVMValueRef main_fn = LLVMAddFunction(generator->module, "main", main_fn_type);
    LLVMBasicBlockRef entry_block = LLVMAppendBasicBlockInContext(generator->context, main_fn, "entry");

    define_FunctionBodies_impl(generator, program->function_list);

    LLVMPositionBuilderAtEnd(generator->builder, entry_block);
    print_ast_node(program->root, 1);
    printf("Llamando a accept del nodo raíz (tipo %d)\n", program->root->type);
    if(program->root->accept){
        printf("El nodo raíz tiene un método accept definido.\n");
    } else {
        printf("El nodo raíz NO tiene un método accept definido.\n");
    }
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

void declare_external_functions(LLVMModuleRef module, LLVMContextRef context) {
    // strcpy
    LLVMTypeRef strcpy_type = LLVMFunctionType(LLVMVoidTypeInContext(context),
        (LLVMTypeRef[]){
            LLVMPointerType(LLVMInt8TypeInContext(context), 0),
            LLVMPointerType(LLVMInt8TypeInContext(context), 0)
        }, 2, 0);
    LLVMAddFunction(module, "strcpy", strcpy_type);

    // strcat
    LLVMTypeRef strcat_type = LLVMFunctionType(LLVMVoidTypeInContext(context),
        (LLVMTypeRef[]){
            LLVMPointerType(LLVMInt8TypeInContext(context), 0),
            LLVMPointerType(LLVMInt8TypeInContext(context), 0)
        }, 2, 0);
    LLVMAddFunction(module, "strcat", strcat_type);

    // Funciones matemáticas (double -> double)
    LLVMTypeRef double_func_type = LLVMFunctionType(LLVMDoubleTypeInContext(context),
        (LLVMTypeRef[]){LLVMDoubleTypeInContext(context)}, 1, 0);
    LLVMAddFunction(module, "sqrt", double_func_type);
    LLVMAddFunction(module, "sin", double_func_type);
    LLVMAddFunction(module, "cos", double_func_type);
    LLVMAddFunction(module, "exp", double_func_type);
    LLVMAddFunction(module, "log", double_func_type);

    // pow y fmod (double, double) -> double
    LLVMTypeRef pow_type = LLVMFunctionType(LLVMDoubleTypeInContext(context),
        (LLVMTypeRef[]){LLVMDoubleTypeInContext(context), LLVMDoubleTypeInContext(context)}, 2, 0);
    LLVMAddFunction(module, "pow", pow_type);
    LLVMAddFunction(module, "fmod", pow_type);

    // rand
    LLVMTypeRef rand_type = LLVMFunctionType(LLVMInt32TypeInContext(context), NULL, 0, 0);
    LLVMAddFunction(module, "rand", rand_type);

    // strlen
    LLVMTypeRef strlen_type = LLVMFunctionType(LLVMInt64TypeInContext(context),
        (LLVMTypeRef[]){LLVMPointerType(LLVMInt8TypeInContext(context), 0)}, 1, 0);
    LLVMAddFunction(module, "strlen", strlen_type);

    // malloc
    LLVMTypeRef malloc_type = LLVMFunctionType(
        LLVMPointerType(LLVMInt8TypeInContext(context), 0),
        (LLVMTypeRef[]){LLVMInt64TypeInContext(context)}, 1, 0);
    LLVMAddFunction(module, "malloc", malloc_type);

    // snprintf
    LLVMTypeRef snprintf_type = LLVMFunctionType(LLVMInt32TypeInContext(context),
        (LLVMTypeRef[]){
            LLVMPointerType(LLVMInt8TypeInContext(context), 0),
            LLVMInt64TypeInContext(context),
            LLVMPointerType(LLVMInt8TypeInContext(context), 0)
        }, 3, 1);
    LLVMAddFunction(module, "snprintf", snprintf_type);

    // strcmp
    LLVMTypeRef strcmp_type = LLVMFunctionType(LLVMInt32TypeInContext(context),
        (LLVMTypeRef[]){
            LLVMPointerType(LLVMInt8TypeInContext(context), 0),
            LLVMPointerType(LLVMInt8TypeInContext(context), 0)
        }, 2, 0);
    LLVMAddFunction(module, "strcmp", strcmp_type);

    // printf
    LLVMTypeRef printf_type = LLVMFunctionType(LLVMInt32TypeInContext(context),
        (LLVMTypeRef[]){LLVMPointerType(LLVMInt8TypeInContext(context), 0)}, 1, 1);
    LLVMValueRef printf_func = LLVMAddFunction(module, "printf", printf_type);
    LLVMSetLinkage(printf_func, LLVMExternalLinkage);

    // exit
    if (!LLVMGetNamedFunction(module, "exit")) {
        LLVMTypeRef exit_type = LLVMFunctionType(LLVMVoidTypeInContext(context), 
            (LLVMTypeRef[]){LLVMInt32TypeInContext(context)}, 1, 0);
        LLVMAddFunction(module, "exit", exit_type);
    }

    // puts
    if (!LLVMGetNamedFunction(module, "puts")) {
        LLVMTypeRef puts_type = LLVMFunctionType(LLVMInt32TypeInContext(context), 
            (LLVMTypeRef[]){LLVMPointerType(LLVMInt8TypeInContext(context), 0)}, 1, 0);
        LLVMAddFunction(module, "puts", puts_type);
    }
}