#ifndef AST_H
#define AST_H

#include "../common/common.h"
#include "../scope/symbol_table.h"
#include <llvm-c/Core.h>
#include <llvm-c/Analysis.h>

// Foward Declarations
typedef struct TypeDescriptor TypeDescriptor;
typedef struct TypeTable TypeTable;
typedef struct SymbolTable SymbolTable;
struct LLVMCodeGenerator;

typedef enum {
    AST_Node_Literal,
    AST_Node_Unary_Operation,
    AST_Node_Binary_Operation,
    AST_Node_Expression_Block,
    AST_Node_Conditional,
    AST_Node_While_Loop,
    AST_Node_Let_In,
    AST_Node_Variable,
    AST_Node_Reassign,
    AST_Node_Function_Definition,
    AST_Node_Function_Definition_List,
    AST_Node_Function_Call,
    AST_Node_Program
} ASTNodeType;

typedef struct ASTNode {
    // Nodo base del AST (Abstract Syntax Tree)
    ASTNodeType type;            // Tipo de nodo AST
    TypeDescriptor* return_type; // Tipo de retorno del nodo AST
    LLVMValueRef (*accept)(struct ASTNode* self, struct LLVMCodeGenerator* visitor);
} ASTNode;

typedef struct LiteralNode {
    // Nodo que representa un literal en el AST
    ASTNode base;           
    union {
        double number_value;
        char *string_value;
        int bool_value;
    } value;    // Valor del literal (puede ser un número, cadena o booleano)
} LiteralNode;

typedef struct UnaryOperationNode {
    // Nodo que representa una operación unaria en el AST
    ASTNode base;
    HULK_Op operator; // Operador unario
    ASTNode *operand; // Operando del operador unario
} UnaryOperationNode;

typedef struct BinaryOperationNode {
    // Nodo que representa una operación binaria en el AST
    ASTNode base;
    HULK_Op operator;   // Operador binario
    ASTNode *left;      // Operando izquierdo
    ASTNode *right;     // Operando derecho
} BinaryOperationNode;

typedef struct ExpressionBlockNode {
    // Nodo que representa un bloque de expresiones en el AST
    ASTNode base;
    ASTNode **expressions;   // Array de expresiones dentro del bloque
    int expression_count; // Número de expresiones en el bloque
} ExpressionBlockNode;

typedef struct ConditionalNode {
    ASTNode base;
    ASTNode* condition;
    ASTNode* then_branch;
    ASTNode* else_branch; // puede ser NULL o un nuevo ConditionalNode
} ConditionalNode;

typedef struct WhileLoopNode {
    ASTNode base;
    ASTNode* condition; // Condición del bucle
    ASTNode* body; // Cuerpo del bucle puede ser una expresion o un bloque de expresiones 
} WhileLoopNode;

typedef struct VariableAssigment {
    char* name;
    ASTNode* value;
} VariableAssigment;

typedef struct LetInNode {
    ASTNode base;
    SymbolTable* scope;
    VariableAssigment* assigments;
    int assigment_count;
    ASTNode* body;
} LetInNode;

typedef struct VariableNode {
    ASTNode base;
    char* name;
    SymbolTable* scope; // Scope donde se encuentra la variable
} VariableNode;

typedef struct ReassignNode {
    ASTNode base ;
    char* name;
    ASTNode* value;
    SymbolTable* scope;
}ReassignNode;

typedef struct FunctionDefinitionNode {
    ASTNode base ;
    char* name;                         // Nombre de la función
    SymbolTable* scope;                 // Scope de la función(donde iran los parametros)
    char** params_names;                // Nombres de los parámetros
    int param_count;                    // Número de parámetros
    ASTNode* body;                      // Cuerpo de la función
} FunctionDefinitionNode;

typedef struct FunctionDefinitionListNode {
    // Lista de definiciones de funciones
    ASTNode base; // Nodo base del AST
    FunctionDefinitionNode** functions; // Array de nodos de definición de funciones
    int function_count;                 // Número de funciones en la lista
} FunctionDefinitionListNode;

typedef struct FunctionCallNode {
    ASTNode base;
    char* name;
    ASTNode** args; 
    int arg_count;
    SymbolTable* scope;
} FunctionCallNode;

typedef struct ProgramNode {
    // Nodo que representa un programa completo
    ASTNode base; // Nodo base del AST
    FunctionDefinitionListNode* function_list; // Lista de definiciones de funciones
    ASTNode* root; // Bloque principal del programa
} ProgramNode;

// Prototipos para crear nodos AST
ASTNode* create_number_literal_node(double value, TypeTable *table);
ASTNode* create_string_literal_node(char *value, TypeTable *table);
ASTNode* create_bool_literal_node(int value, TypeTable *table);
ASTNode* create_unary_operation_node(HULK_Op operator, ASTNode *operand, TypeTable *table);
ASTNode* create_binary_operation_node(HULK_Op operator, ASTNode *left, ASTNode *right, TypeTable *table);
ASTNode* create_expression_block_node(ASTNode **expressions, int expression_count, TypeTable *table);
ASTNode* create_conditional_node(ASTNode *condition, ASTNode *then_branch, ASTNode *else_branch, TypeTable *table);
ASTNode* create_while_loop_node(ASTNode *condition, ASTNode *body, TypeTable *table);
ASTNode* create_let_in_node(VariableAssigment *assigments, int assigment_count, ASTNode *body, TypeTable *table); 
ASTNode* create_variable_node(char *name, TypeTable *table);
ASTNode* create_reassign_node(char *name, ASTNode *value, TypeTable *table);
ASTNode* create_function_definition_node(char *name, char **params_names, int param_count, ASTNode *body, TypeTable *table);
ASTNode* create_function_definition_list_node(TypeTable *table);
ASTNode* create_function_call_node(char* name, ASTNode** args, int arg_count, TypeTable *table);
ASTNode* create_program_node(FunctionDefinitionListNode *function_list, ASTNode *root, TypeTable *table);

void create_scope_let_in_node(LetInNode* node, SymbolTable* parent_scope);
void register_func_params(FunctionDefinitionNode* node, SymbolTable* parent_scope, TypeTable* type_table);
ASTNode* append_function_definition_to_list(FunctionDefinitionListNode* list, FunctionDefinitionNode* def);

// Prototipos para imprimir nodos(Debug)
void print_ast_node(ASTNode *node, int indent_level);
void print_literal_node(LiteralNode *node, int indent_level);
void print_unary_operation_node(UnaryOperationNode *node, int indent);
void print_binary_operation_node(BinaryOperationNode *node, int indent);
void print_expression_block_node(ExpressionBlockNode *node, int indent_level);
void print_conditional_node(ConditionalNode *node, int indent_level);
void print_while_loop_node(WhileLoopNode *node, int indent_level);
void print_let_in_node(LetInNode *node, int indent_level);
void print_variable_node(VariableNode *node, int indent_level);
void print_reassign_node(ReassignNode *node, int indent_level);
void print_function_definition_node(FunctionDefinitionNode *node, int indent_level);
void print_function_definition_list_node(FunctionDefinitionListNode *list, int indent_level);
void print_function_call_node(FunctionCallNode* node , int indent_level);
void print_program_node(ProgramNode *node, int indent_level);
void print_indent(int indent);

// Prototipos para liberar nodos AST
void free_ast_node(ASTNode *node);
void free_literal_node(LiteralNode *node);
void free_unary_operation_node(UnaryOperationNode *node);
void free_binary_operation_node(BinaryOperationNode *node);
void free_expression_block_node(ExpressionBlockNode *node);
void free_conditional_node(ConditionalNode *node);
void free_while_loop_node(WhileLoopNode *node);
void free_let_in_node(LetInNode *node);
void free_variable_node(VariableNode *node);
void free_reassign_node(ReassignNode *node);
void free_function_definition_node(FunctionDefinitionNode *node);
void free_function_definition_list(FunctionDefinitionListNode *list);
void free_function_call_node(FunctionCallNode *node);
void free_program_node(ProgramNode *node);
#endif // AST_H