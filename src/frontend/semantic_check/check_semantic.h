#include "../hulk_type/hulk_type.h"
#include "../ast/ast.h"
#include "../scope/symbol_table.h"

TypeDescriptor* check_semantic_literal_node(ASTNode* node);
TypeDescriptor* check_semantic_unary_operation_node(UnaryOperationNode* node);
TypeDescriptor* check_semantic_binary_operation_node(BinaryOperationNode* node, TypeTable* table);
TypeDescriptor* check_semantic_expression_block_node(ExpressionBlockNode* node, TypeTable* table);
TypeDescriptor* check_semantic_conditional_node(ConditionalNode* node);
TypeDescriptor* check_semantic_while_loop_node(WhileLoopNode* node);
TypeDescriptor* check_semantic_let_in_node(LetInNode* node);
TypeDescriptor* check_semantic_variable_node(VariableNode* node);
TypeDescriptor* check_semantic_variable_assigment_node(VariableAssigmentNode* node, TypeTable* table);
TypeDescriptor* check_semantic_reassign_node(ReassignNode* node);
TypeDescriptor* check_semantic_function_definition_node(FunctionDefinitionNode* node, TypeTable* table);
TypeDescriptor* check_semantic_function_call_node(FunctionCallNode* node, SymbolTable* current_scope);
TypeDescriptor* check_semantic_type_definition_node(TypeDefinitionNode* node, TypeTable* table);
