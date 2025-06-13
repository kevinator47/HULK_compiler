#ifndef SEMANTIC_VISITOR_H
#define SEMANTIC_VISITOR_H

#include "../ast/ast.h"
#include "../common/common.h"
#include "../hulk_type/type_table.h"

typedef struct SemanticVisitor {
    TypeTable* typeTable;
} SemanticVisitor;

SemanticVisitor* init_semantic_visitor(TypeTable* type_table);
TypeDescriptor* semantic_visit(SemanticVisitor* visitor, ASTNode* node, SymbolTable* current_scope);
void register_globals(ProgramNode* program, SymbolTable* current_scope, TypeTable* type_table);
void register_types(TypeDefinitionListNode* list, SymbolTable* current_scope, TypeTable* type_table);
void register_params(TypeDefinitionNode* type_def_node, SymbolTable* type_scope, TypeTable* type_table);
void register_fields(TypeDefinitionNode* type_def_node, SymbolTable* type_scope, TypeTable* type_table);
void register_functions(FunctionDefinitionListNode* list, SymbolTable* global_scope, TypeTable* type_table);
void register_prototype(FunctionDefinitionNode* func_node, SymbolTable* scope, TypeTable* type_table);
void register_function_params(FunctionDefinitionNode* node, TypeTable* table);
#endif