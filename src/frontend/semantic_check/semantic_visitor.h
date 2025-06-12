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
void register_types(TypeDefinitionListNode* node, TypeTable* table, SymbolTable* scope);
void register_type_fields(TypeDefinitionNode* node, TypeTable* table, SymbolTable* current_scope);
void add_param_to_scope(SymbolTable* scope, Param* param, TypeTable* table);
void register_field(TypeDefinitionNode* node, ASTNode* expression, TypeTable* table);
#endif