#ifndef SEMANTIC_VISITOR_H
#define SEMANTIC_VISITOR_H

#include "../ast/ast.h"
#include "../common/common.h"
#include "../hulk_type/type_table.h"

typedef struct SemanticVisitor {
    TypeTable* typeTable;
} SemanticVisitor;

SemanticVisitor* init_semantic_visitor(TypeTable* type_table);
TypeDescriptor*  semantic_visit(SemanticVisitor* visitor, ASTNode* node, SymbolTable* current_scope);

#endif