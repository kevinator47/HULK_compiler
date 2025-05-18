#ifndef VISITOR_H
#define VISITOR_H

#include "ast.h"
#include "common.h"
#include "symbol_table.h"
#include "function_table.h"
typedef enum
{
    NUMBER_TYPE,
    BOOL_TYPE,
    STRING_TYPE,
    ERROR_TYPE,
    CORRECT_TYPE,
} DataType;

typedef struct 
{
    ASTNode* root;
    FunctionTable* functionTable;
} Visitor;

Visitor* init_visitor(ASTNode* node, FunctionTable* table);
DataType check_semantic_expression(Visitor* visitor, ASTNode* node);
DataType check_semantic_in_function(Visitor* visitor,FunctionCallNode* node);
void freeVisitor();
#endif