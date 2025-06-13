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
    AST_Node_Variable_Assigment,
    AST_Node_Reassign,
    AST_Node_Function_Definition,
    AST_Node_Function_Definition_List,
    AST_Node_Function_Call,
    AST_Node_Program,
    AST_Node_Type_Definition,
    AST_Node_Type_Definition_List,
    AST_Node_Instanciate_Type,
    AST_Node_Function_Call_Type
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
    } value;    
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
    int expression_count;    // Número de expresiones en el bloque
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
    ASTNode* body;      // Cuerpo del bucle puede ser una expresion o un bloque de expresiones 
} WhileLoopNode;

typedef struct VariableAssigment {
    char* name;
    char* static_type;
    ASTNode* value;
} VariableAssigment;

typedef struct LetInNode {
    ASTNode base;
    SymbolTable* scope;
    VariableAssigment** assigments;
    int assigment_count;
    ASTNode* body;
} LetInNode;

typedef struct VariableNode {
    ASTNode base;
    char* name;
    SymbolTable* scope; // Scope donde se encuentra la variable
} VariableNode;

typedef struct VariableAssigmentNode {
    ASTNode base;
    VariableAssigment* assigment;
    SymbolTable* scope;
} VariableAssigmentNode;

typedef struct ReassignNode {
    ASTNode base ;
    char* name;
    ASTNode* value;
    SymbolTable* scope;
} ReassignNode;

typedef struct Param {
    char* name ;        // nombre del parametro
    char* static_type;  // nombre del tipo(no se puede usar TypeDescriptor* porque no estaran creados los definidos por el usuario)
} Param ;

typedef struct FunctionDefinitionNode {
    ASTNode base ;
    char* name;                         // Nombre de la función
    SymbolTable* scope;                 // Scope de la función(donde iran los parametros)
    Param** params;                     // Parametros
    int param_count;                    // Número de parámetros
    char* static_return_type;           // Tipo de retorno de la funcion
    ASTNode* body;                      // Cuerpo de la función    
} FunctionDefinitionNode;

typedef struct FunctionDefinitionListNode {
    // Lista de definiciones de funciones
    ASTNode base;                       // Nodo base del AST
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

typedef struct TypeDefinitionNode {
    ASTNode base;          
    
    char* type_name;        // Nombre del tipo
    
    Param** params;         // Parametros pasados al constructor(NULL si no se especifica)
    int param_count;        // Cantidad de parametros(0 si no se especifica)

    char* parent_name;      // Nombre del padre (Object si no se especifica)
    ASTNode** parent_args;  // Argumentos para pasar al padre(NULL si no se especifica)
    int parent_arg_count;   // Cantidad de argumentos(0 si no se especifica)
    
    SymbolTable* scope;     // Scope donde estaran atributos y funciones
    ExpressionBlockNode* body;          // Cuerpo de la declaracion
} TypeDefinitionNode;

typedef struct TypeDefinitionListNode {
    ASTNode base;
    TypeDefinitionNode** definitions;
    int count;
} TypeDefinitionListNode ;

typedef struct InstanciateNode{
    ASTNode base;
    char* type_name;
    ASTNode** args;
    int arg_count;
}InstanciateNode;

typedef struct FuntionCallTypeNode{
    ASTNode base;
    char* type_name;
    char* func_name;
    ASTNode** args;
    int arg_count;
    SymbolTable* scope;
}FuntionCallTypeNode;

typedef struct ProgramNode {
    // Nodo que representa un programa completo
    ASTNode base; // Nodo base del AST
    FunctionDefinitionListNode* function_list;  // Lista de definiciones de funciones
    TypeDefinitionListNode* type_definitions;   // Lista de definiciones de tipos
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
ASTNode* create_let_in_node(VariableAssigment** assigments, int assigment_count, ASTNode *body, TypeTable *table);
ASTNode* create_variable_node(char *name, TypeTable *table);
ASTNode* create_variable_assigment_node(VariableAssigment* assigment, TypeTable* table);
ASTNode* create_reassign_node(char *name, ASTNode *value, TypeTable *table);
ASTNode* create_function_definition_node(const char* name, char** param_names, char** param_types, int param_count, char* return_type, ASTNode* body, TypeTable* table);
ASTNode* create_function_definition_list_node(TypeTable *table);
ASTNode* create_function_call_node(char* name, ASTNode** args, int arg_count, TypeTable *table);
ASTNode* create_type_definition_node(char* type_name, char** param_names, char** param_types, int param_count, char* parent_name, ASTNode** parent_args, int parent_arg_count, ASTNode* body, TypeTable* table);
ASTNode* create_type_definition_list_node(TypeDefinitionNode** list, int count, TypeTable* table);
ASTNode* create_instanciate_node(char* type_name, ASTNode** args, int arg_count, TypeTable* table);
ASTNode* create_func_call_type_node(char* type_name, char* func_name, ASTNode** args, int arg_count, TypeTable* table);
ASTNode* create_program_node(ASTNode* function_list, ASTNode* type_list, ASTNode *root, TypeTable *table);

void register_func_params(FunctionDefinitionNode* node, SymbolTable* parent_scope, TypeTable* table);
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
void print_variable_assigment_node(VariableAssigmentNode*, int indent_level);
void print_reassign_node(ReassignNode *node, int indent_level);
void print_function_definition_node(FunctionDefinitionNode *node, int indent_level);
void print_function_definition_list_node(FunctionDefinitionListNode *node, int indent_level);
void print_type_definition_node(TypeDefinitionNode* node, int indent_level);
void print_type_definition_list_node(TypeDefinitionListNode* node, int indent_level);
void print_function_call_node(FunctionCallNode* node , int indent_level);
void print_instanciate_type_node(InstanciateNode* node, int indent_level);
void print_func_call_type_node(FuntionCallTypeNode* node, int indent_level);
void print_program_node(ProgramNode *node, int indent_level);
void print_indent(int indent);

// Prototipos para liberar nodos AST
void free_ast_node(ASTNode* node);
void free_literal_node(LiteralNode *node);
void free_unary_operation_node(UnaryOperationNode* node);
void free_binary_operation_node(BinaryOperationNode* node);
void free_expression_block_node(ExpressionBlockNode* node);
void free_conditional_node(ConditionalNode* node);
void free_while_loop_node(WhileLoopNode* node);
void free_let_in_node(LetInNode* node);
void free_variable_node(VariableNode* node);
void free_variable_assigment_node(VariableAssigmentNode* node);
void free_reassign_node(ReassignNode* node);
void free_function_definition_node(FunctionDefinitionNode* node);
void free_function_definition_list_node(FunctionDefinitionListNode* node);
void free_function_call_node(FunctionCallNode* node);
void free_type_definition_node(TypeDefinitionNode* node);
void free_type_definition_list_node(TypeDefinitionListNode* node);
void free_instanciate_type_node(InstanciateNode* node);
void free_func_call_type_node(FuntionCallTypeNode* node);
void free_program_node(ProgramNode* node);

#endif // AST_H