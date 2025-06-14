#include <stdio.h>
#include <stdlib.h>
#include "generator.h"
#include "common/common.h"
#include "ast/ast.h"
#include "hulk_type/hulk_type.h"
#include "hulk_type/type_table.h"
#include "semantic_check/semantic_visitor.h"
#include "scope/function.h"
#include "../build/parser.tab.h"

// Declaraciones externas del parser
extern int yyparse();
extern FILE* yyin; // Flex usa esta variable para la entrada
extern ASTNode* root_node;

int main(int argc, char **argv) {
    
    // Declaraciones externas del parser
    extern int yyparse();
    extern FILE* yyin; 
    extern ASTNode* root_node;
    extern TypeTable* type_table;
    type_table = create_type_table();

    // Registrar tipos predefinidos
    register_builtin_types(type_table);
    printf("Se han cargado %d tipos builtin en la tabla de tipos.\n", type_table->count);

    // Registrar funciones predefinidas
    SymbolTable* global_scope = create_symbol_table(NULL);
    register_predefined_functions(global_scope, type_table);
    
    // Seleccionar fuente de entrada
    if (argc > 1) {
        yyin = fopen(argv[1], "r");
        if (!yyin) {
            fprintf(stderr, "No se pudo abrir el archivo '%s'\n", argv[1]);
            return 1;
        }
    } else {
        yyin = stdin;
    }

    // Parsear la entrada
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
        
    // Chequeo Semantico
    SemanticVisitor* visitor = init_semantic_visitor(type_table);
    semantic_visit(visitor,root_node, global_scope);

    printf("Chequeo semántico completado.\n");
    print_ast_node(root_node, 0); // Imprimir el AST para depuración

    // Generación de código LLVM
    const char* module_name = "hulk_module";
    LLVMCodeGenerator* generator = create_llvm_code_generator("hulk_module", type_table);
    LLVMModuleRef module = generate_code((ProgramNode*)root_node, generator);

    // Imprimir a archivo
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

    // Limpieza final
    free_ast_node(root_node);
    free_type_table(type_table);
    return 0;
}