#include <stdio.h>
#include <stdlib.h>
#include "backend/codegen/llvm/generator.h"
#include "frontend/hulk_type/hulk_type.h"
#include "frontend/hulk_type/type_table.h"
#include "frontend/semantic_check/semantic_visitor.h"
#include "frontend/ast/ast.h"
#include "../build/parser.tab.h"

// Declaraciones externas del parser
extern int yyparse();
extern FILE* yyin; // Flex usa esta variable para la entrada
extern ASTNode* root_node;

int main(int argc, char **argv) {
    //  1. Inicializacion tabla de simbolos
    extern TypeTable* type_table;
    type_table = create_type_table();

    // 2. Creacion tipos builtin
    TypeDescriptor* object_type = create_builtin_type(HULK_Type_Object, "Object", NULL);
    TypeDescriptor* number_type = create_builtin_type(HULK_Type_Number, "Number", object_type);
    TypeDescriptor* bool_type = create_builtin_type(HULK_Type_Boolean, "Bool", object_type);
    TypeDescriptor* string_type = create_builtin_type(HULK_Type_String, "String", object_type);
    TypeDescriptor* null_type = create_builtin_type(HULK_Type_Null, "Null", object_type);
    TypeDescriptor* undefined_type = create_builtin_type(HULK_Type_Undefined, "Undefined", object_type);
    
    // Agregarlos a la tabla 
    add_type(type_table, object_type);
    add_type(type_table, number_type);
    add_type(type_table, bool_type);
    add_type(type_table, string_type);
    add_type(type_table, null_type);
    add_type(type_table, undefined_type);

    // Mensaje de verificación(debug)
    printf("Se han cargado %d tipos builtin en la tabla de tipos.\n", type_table->count);
    
    // Declaraciones externas del parser
    extern int yyparse();
    extern FILE* yyin; // Flex usa esta variable para la entrada
    extern ASTNode* root_node;
    SymbolTable* global_scope = create_symbol_table(NULL); // Crear el scope global

    // 3. Seleccionar fuente de entrada
    if (argc > 1) {
        yyin = fopen(argv[1], "r");
        if (!yyin) {
            fprintf(stderr, "No se pudo abrir el archivo '%s'\n", argv[1]);
            return 1;
        }
    } else {
        yyin = stdin;
    }

    // 4. Parsear la entrada
    int parse_result = yyparse();
    if (parse_result != 0 || root_node == NULL) {
        fprintf(stderr, "Error de parsing.\n");
        if (yyin != stdin) fclose(yyin);
        return 1;
    }
    if (yyin != stdin) fclose(yyin);

    if (root_node == NULL) {
    fprintf(stderr, "El AST está vacío. No se generará código LLVM.\n");
    return 1;
    }
        
    // 5. Chequeo Semantico
    SemanticVisitor* visitor = init_semantic_visitor(type_table);
    semantic_visit(visitor,root_node, global_scope);

    printf("Chequeo semántico completado.\n");
    print_ast_node(root_node, 0); // Imprimir el AST para depuración

    // 6. Generación de código LLVM
    const char* module_name = "hulk_module";
    LLVMCodeGenerator* generator = create_llvm_code_generator("hulk_module");
    LLVMModuleRef module = generate_code((ProgramNode*)root_node, generator);

    // 7. Imprimir a archivo
    if (module) {
        char* output_filename = "output.ll";
        char* error_message = NULL;
       
        if (LLVMPrintModuleToFile(module, output_filename, &error_message) != 0) {
            fprintf(stderr, "Error al escribir el modulo LLVM: %s\n", error_message);
            LLVMDisposeMessage(error_message);
        } else {
            fprintf(stderr, "Archivo '%s' generado exitosamente.\n", output_filename);
        }
    } else {
        fprintf(stderr, "La generacion de codigo LLVM fallo.\n");
        destroy_llvm_code_generator(generator);
        return 1;
    }
    destroy_llvm_code_generator(generator);

    // 8.Limpieza final
    free_type_table(type_table);
    free_ast_node(root_node);
    return 0;
}