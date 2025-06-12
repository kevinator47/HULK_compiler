#include "semantic_visitor.h"

void register_types(TypeDefinitionListNode* node, TypeTable* table, SymbolTable* current_scope) {
    for (int i = 0; i < node->count; i++)
    {
        register_type_fields(node->definitions[i], table, current_scope);
    }
}

void register_type_fields(TypeDefinitionNode* node, TypeTable* table, SymbolTable* current_scope) {
    TypeDescriptor* parent_type = type_table_lookup(table, node->parent_name);

    if(!parent_type || ! parent_type->initializated)
    {
        fprintf(stderr, "Error: Unknown type \"%s\"\n", node->parent_name);
        exit(1);
    }
    
    node->scope = create_symbol_table(current_scope);
    insert_symbol(node->scope, create_symbol("self", SYMBOL_TYPE_FIELD, type_table_lookup(table, node->type_name), NULL));
    
    for (int i = 0; i < node->param_count; i++)
    {
        add_param_to_scope(node->scope, node->params[i], table);
        ExpressionBlockNode* type_body = node->body;

        for (int i = 0; i < type_body->expression_count; i++)
        {
            register_field(node, type_body->expressions[i], table);
        }
    }

    TypeInfo* info = create_type_info(node->scope, node->param_count, node->params);
    modify_type(type_table_lookup(table, node->type_name), info, NULL, NULL);
    
}

void add_param_to_scope(SymbolTable* scope, Param* param, TypeTable* table)
{
    // Inserta un parametro en un scope
    insert_symbol(scope, create_symbol(param->name , SYMBOL_PARAMETER, type_table_lookup(table, param->static_type), NULL));
}

void register_field(TypeDefinitionNode* node, ASTNode* expression, TypeTable* table) {
    switch (expression->type)
    {
    case AST_Node_Variable_Assigment: {
            VariableAssigmentNode* assign_node = (VariableAssigmentNode*) expression;
            Symbol* symbol = create_symbol(assign_node->assigment->name, SYMBOL_TYPE_FIELD, type_table_lookup(table, assign_node->assigment->static_type), assign_node->assigment->value);
            insert_symbol(node->scope, symbol);
            break;
    }
    case AST_Node_Function_Definition : {
        FunctionDefinitionNode* func_node = (FunctionDefinitionNode*) expression;
        Symbol* symbol = create_symbol(func_node->name, SYMBOL_TYPE_METHOD, type_table_lookup(table,func_node->static_return_type), expression);
        insert_symbol(node->scope, symbol);
        break;
    }
    default:
        break;
    }
}