#include "../codegen/llvm/generator.h"
#include "./entities/ast.h"
#include <stdio.h>
#include <stdlib.h>
#include "../../build/parser.tab.h"

// Declaraciones externas del parser
extern int yyparse();
extern FILE* yyin; // Flex usa esta variable para la entrada
extern ASTNode* root_node;

int main(int argc, char **argv) {
    // 1. Seleccionar fuente de entrada
    if (argc > 1) {
        yyin = fopen(argv[1], "r");
        if (!yyin) {
            fprintf(stderr, "No se pudo abrir el archivo '%s'\n", argv[1]);
            return 1;
        }
    } else {
        yyin = stdin;
    }

    // 2. Parsear la entrada
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
    else {
        print_ast(root_node, 0); // Imprimir el AST para depuración
    }

    // 3. Generación de código LLVM
    const char* module_name = "hulk_module";
    LLVMCodeGenerator* generator = create_llvm_code_generator("hulk_module");
    LLVMModuleRef module = generate_code(root_node, generator);

    // 4. Imprimir a archivo
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
    // 5. Limpieza final
    free_ast(root_node); // Si tienes esta función

    return 0;
}