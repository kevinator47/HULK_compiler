#include "visitor.h"
#include "common.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

Visitor* init_visitor(ASTNode* node, FunctionTable* table)
{
    Visitor* visitor = malloc(sizeof(Visitor));
    visitor->functionTable = table;
    visitor->root = node;
    return visitor;
}

DataType check_semantic_expression(Visitor* visitor,ASTNode* node)
{
    switch (node->type)
    {
    case Number_Literal_Node:
        return NUMBER_TYPE;
    case Boolean_Literal_Node:
        return BOOL_TYPE;
    case String_Literal_Node:
        return STRING_TYPE;
    case Unary_Op_Node:{
        UnaryOpNode* unary = (UnaryOpNode*) node;
        if (unary->operation == NOT_TK)
        {
            DataType type = check_semantic_expression(visitor,unary->operand);
            if(type == BOOL_TYPE) return BOOL_TYPE;
            return ERROR_TYPE;
        }
    }
    case Binary_Op_Node:{
        BinaryOpNode* binary = (BinaryOpNode*) node;
        DataType leftType = check_semantic_expression(visitor,binary->left);
        DataType rightType = check_semantic_expression(visitor, binary->right);
        if (binary->operation == PLUS_TK ||
            binary->operation == MINUS_TK ||
            binary->operation == MULT_TK ||
            binary->operation == DIV_TK ||
            binary->operation == MOD_TK ||
            binary->operation == EXP_TK)
        {
            if(leftType == NUMBER_TYPE && rightType == NUMBER_TYPE) return NUMBER_TYPE;
            return ERROR_TYPE;
        }
        if (binary->operation == GT_TK||
            binary->operation == GE_TK||
            binary->operation == LT_TK||
            binary->operation == LE_TK)
        {
            if(leftType == NUMBER_TYPE && rightType == NUMBER_TYPE) return BOOL_TYPE;
            return ERROR_TYPE;
        }
        if (binary->operation == EQ_TK ||
            binary->operation == NE_TK)
        {
            if(leftType == rightType) return BOOL_TYPE;
            return ERROR_TYPE;
        }
        if (binary->operation == CONCAT_TK || 
            binary->operation == D_CONCAT_TK)
        {
            if(leftType == STRING_TYPE) return STRING_TYPE;
            return ERROR_TYPE;
        }
        if (binary->operation == AND_TK ||
            binary->operation == OR_TK)
        {
            if(leftType == BOOL_TYPE && rightType == BOOL_TYPE) return BOOL_TYPE;
            return ERROR_TYPE;
        }
    }
    case Expression_Block_Node:{
        ExpressionBlockNode* block = (ExpressionBlockNode*) node;
        for (int i = 0; i < block->count - 1; i++)
        {
            DataType type = check_semantic_expression(visitor, block->expressions[i]);
            if(type == ERROR_TYPE) return ERROR_TYPE;
        }
        return check_semantic_expression(visitor, block->expressions[block->count - 1]);
    }
    case Let_Node:{
        LetNode* let = (LetNode*) node;
        return check_semantic_expression(visitor,let->body);
    }
    case Variable_Node:{
        VariableNode* var = (VariableNode*) node;
        Symbol* symbol = lookup_symbol(var->scope, var->name);
        if(!symbol) return ERROR_TYPE; //Esto es lo que busca si se definio o no la variable
        return check_semantic_expression(visitor, symbol->expression);
    }
    case Conditional_Node:{
        ConditionalNode* conditional = (ConditionalNode*) node;
        DataType condType = check_semantic_expression(visitor, conditional->condition);
        if(condType != BOOL_TYPE) return ERROR_TYPE;
        DataType ifBranchType = check_semantic_expression(visitor, conditional->if_branch);
        if(ifBranchType == ERROR_TYPE) return ERROR_TYPE;
        for (int i = 0; i < conditional->elifs.count; i++)
        {
            DataType cond = check_semantic_expression(visitor, conditional->elifs.conditions[i]);
            if(cond != BOOL_TYPE) return ERROR_TYPE;
            DataType branch = check_semantic_expression(visitor, conditional->elifs.branches[i]);
            if(ifBranchType != branch) return ERROR_TYPE;
        }
        DataType elseType = check_semantic_expression(visitor, conditional->else_branch);
        if(elseType == ifBranchType) return elseType;
        return ERROR_TYPE;
    }
    case Function_Call_Node:{
        FunctionCallNode* func = (FunctionCallNode*)node;

    }
    default:
        break;
    }
}

DataType check_semantic_in_function(Visitor* visitor, FunctionCallNode* node)
{
    Function* function = lookup_function(visitor->functionTable, node->name);
    if(!function) return ERROR_TYPE;
    if(function->param_count != node->arg_count) return ERROR_TYPE;

    SymbolTable* funcScope = create_symbol_table(function->param_count, NULL);
    for (int i = 0; i < function->param_count; i++)
    {
        DataType argType = check_semantic_expression(visitor, node->arguments[i]);
        insert_symbol(funcScope, function->parameters[i], node->arguments[i]);
    }
    return check_semantic_expression(visitor, function->body);
}