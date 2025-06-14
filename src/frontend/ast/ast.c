#include "ast.h"
#include "../common/common.h"
#include "../hulk_type/type_table.h"
#include "../../backend/codegen/ast_accept.h"

const char *Hulk_Op_Names[] = {
    [AND_TK] = "AND", [OR_TK] = "OR", [NOT_TK] = "NOT",
    [PLUS_TK] = "PLUS", [MINUS_TK] = "MINUS", [MULT_TK] = "MULT",
    [DIV_TK] = "DIV", [MOD_TK] = "MOD", [EXP_TK] = "EXP",
    [CONCAT_TK] = "CONCAT", [D_CONCAT_TK] = "D_CONCAT",
    [GT_TK] = "GT", [GE_TK] = "GE", [LT_TK] = "LT",
    [LE_TK] = "LE", [EQ_TK] = "EQ", [NE_TK] = "NE",
};

ASTNode* create_number_literal_node(double value, TypeTable *table) {
    LiteralNode *node = malloc(sizeof(LiteralNode));
    if (!node) return NULL;

    node->base.type = AST_Node_Literal;
    node->base.return_type = type_table_lookup(table, "Number");
    node->base.accept = generic_ast_accept;
    node->value.number_value = value;

    return (ASTNode*) node;
}

ASTNode* create_string_literal_node(char *value, TypeTable *table) {
    LiteralNode *node = malloc(sizeof(LiteralNode));
    if (!node) return NULL;

    node->base.type = AST_Node_Literal;
    node->base.return_type = type_table_lookup(table, "String");
    node->base.accept = generic_ast_accept;
    node->value.string_value = strdup(value); 

    return (ASTNode*) node;
}

ASTNode* create_bool_literal_node(int value, TypeTable *table) {
    LiteralNode *node = malloc(sizeof(LiteralNode));
    if (!node) return NULL;

    node->base.type = AST_Node_Literal;
    node->base.return_type = type_table_lookup(table, "Bool");
    node->base.accept = generic_ast_accept;
    node->value.bool_value = value != 0; // asegurar 0 o 1

    return (ASTNode*) node;
}
ASTNode* create_unary_operation_node(HULK_Op operator, ASTNode *operand, TypeTable *table) {
    UnaryOperationNode *node = malloc(sizeof(UnaryOperationNode));
    if (!node) return NULL;

    node->base.type = AST_Node_Unary_Operation;
    node->base.return_type = type_table_lookup(table, "Undefined");
    node->base.accept = generic_ast_accept;
    node->operator = operator;
    node->operand = operand;

    return (ASTNode*) node;
}

ASTNode* create_binary_operation_node(HULK_Op operator, ASTNode *left, ASTNode *right, TypeTable *table) {
    BinaryOperationNode *node = malloc(sizeof(BinaryOperationNode));
    if (!node) return NULL;

    node->base.type = AST_Node_Binary_Operation;
    node->base.return_type = type_table_lookup(table, "Undefined"); 
    node->base.accept = generic_ast_accept;
    node->operator = operator;
    node->left = left;
    node->right = right;

    return (ASTNode*) node;
}

ASTNode* create_expression_block_node(ASTNode **expressions, int count, TypeTable *table) {
    ExpressionBlockNode *node = malloc(sizeof(ExpressionBlockNode));
    if (!node) return NULL;

    node->base.type = AST_Node_Expression_Block;
    node->base.return_type = type_table_lookup(table, "Undefined");
    node->base.accept = generic_ast_accept;
    
    // do not assign the array, copy each expression
    node->expressions = malloc(sizeof(ASTNode*) * count);
    
    for (int i = 0; i < count; i++) {
        node->expressions[i] = expressions[i]; // copy the pointer
    }
    node->expression_count = count;

    return (ASTNode*) node;
}

ASTNode* create_conditional_node(ASTNode *condition, ASTNode *then_branch, ASTNode *else_branch, TypeTable *table) {
    ConditionalNode *node = malloc(sizeof(ConditionalNode));
    if (!node) return NULL;

    node->base.type = AST_Node_Conditional;
    node->base.return_type = type_table_lookup(table, "Undefined");
    node->base.accept = generic_ast_accept;
    node->condition = condition;
    node->then_branch = then_branch;
    node->else_branch = else_branch;

    return (ASTNode*) node;
}

ASTNode* create_while_loop_node(ASTNode *condition, ASTNode *body, TypeTable *table) {
    WhileLoopNode *node = malloc(sizeof(WhileLoopNode));
    if (!node) return NULL;

    node->base.type = AST_Node_While_Loop;
    node->base.return_type = type_table_lookup(table, "Undefined");
    node->base.accept = generic_ast_accept;
    node->condition = condition;
    node->body = body;

    return (ASTNode*) node;
}

ASTNode* create_variable_assigment_node(VariableAssigment* assigment, TypeTable* table) {
    VariableAssigmentNode* node = malloc(sizeof(VariableAssigmentNode));
    if(!node) return NULL;

    node->base.type = AST_Node_Variable_Assigment;
    node->base.return_type = type_table_lookup(table, "Undefined");
    node->base.accept = generic_ast_accept;
    node->scope = NULL;
    node->assigment = assigment;

    return (ASTNode*) node;
}

ASTNode* create_let_in_node(VariableAssigmentNode** assigments, int assigment_count, ASTNode* body, TypeTable* table) {
    LetInNode* node = malloc(sizeof(LetInNode));
    if (!node) return NULL;

    node->base.type = AST_Node_Let_In;
    node->base.return_type = type_table_lookup(table, "Undefined");
    node->base.accept = generic_ast_accept;
    node->scope = NULL;

    node->assigments = malloc(sizeof(VariableAssigmentNode*) * assigment_count);
    if (!node->assigments) {
        free(node);
        return NULL;
    }

    for (int i = 0; i < assigment_count; i++) {
        node->assigments[i] = assigments[i];
    }

    node->assigment_count = assigment_count;
    node->body = body;

    return (ASTNode*) node;
}


ASTNode* create_variable_node(char *name, TypeTable *table) {
    VariableNode *node = malloc(sizeof(VariableNode));
    if (!node) return NULL;

    node->base.type = AST_Node_Variable;
    node->base.return_type = type_table_lookup(table, "Undefined");
    node->name = strdup(name);
    node->scope = NULL;
    node->base.accept = generic_ast_accept;

    return (ASTNode*) node;
}

ASTNode* create_reassign_node(char *name, ASTNode *value, TypeTable *table) {
    ReassignNode *node = malloc(sizeof(ReassignNode));
    if (!node) return NULL;

    node->base.type = AST_Node_Reassign;
    node->base.return_type = type_table_lookup(table, "Undefined");
    node->base.accept = generic_ast_accept;
    node->name = strdup(name);
    node->value = value;
    node->scope = NULL;

    return (ASTNode*) node;
}

ASTNode* create_function_definition_node(const char* name, char** param_names, char** param_types, int param_count, char* return_type, ASTNode* body, TypeTable* table) {
    FunctionDefinitionNode* node = malloc(sizeof(FunctionDefinitionNode));
    if (!node) return NULL;

    node->base.type = AST_Node_Function_Definition;
    node->base.return_type = type_table_lookup(table, "Null");
    node->base.accept = generic_ast_accept;

    node->name = strdup(name);
    node->param_count = param_count;

    node->params = malloc(sizeof(Param*) * param_count);
    node->body = body;
    node->scope = NULL;

    node->static_return_type = return_type;

    for (int i = 0; i < param_count; i++) {
        Param* param = malloc(sizeof(Param));
        param->name = strdup(param_names[i]);
        param->static_type = strdup(param_types[i]);
        node->params[i] = param;
    }

    return (ASTNode*) node;
}

ASTNode* create_function_definition_list_node(TypeTable* table) {
    FunctionDefinitionListNode* node = malloc(sizeof(FunctionDefinitionListNode));
    node->base.type = AST_Node_Function_Definition_List;
    node->base.return_type = type_table_lookup(table, "Null");
    node->base.accept = generic_ast_accept;
    node->functions = NULL;
    node->function_count = 0;
    return (ASTNode*) node;
}
    
ASTNode* append_function_definition_to_list(FunctionDefinitionListNode* list, FunctionDefinitionNode* def) {
    list->base.accept = generic_ast_accept;
    list->functions = realloc(list->functions, sizeof(FunctionDefinitionNode*) * (list->function_count + 1));
    list->functions[list->function_count] = def;
    list->function_count++;
    return (ASTNode*)list;
}

ASTNode* create_function_call_node(char* name, ASTNode** args, int arg_count, TypeTable *table) {
    FunctionCallNode *node = malloc(sizeof(FunctionCallNode));
    if (!node) return NULL;

    node->base.type = AST_Node_Function_Call;
    node->base.return_type = type_table_lookup(table, "Undefined");
    node->base.accept = generic_ast_accept;
    node->name = strdup(name);
    node->arg_count = arg_count;
    node->scope = NULL;


    node->args = malloc(sizeof(ASTNode*) * arg_count);
    
    for (int i = 0; i < arg_count; i++) {
        node->args[i] = args[i];
    }
    return (ASTNode*) node ;    
}

ASTNode* create_type_definition_node(char* type_name, char** param_names, char** param_types, int param_count, char* parent_name, ASTNode** parent_args, int parent_arg_count, ASTNode* body, TypeTable* table)
{
    TypeDefinitionNode* node = malloc(sizeof(TypeDefinitionNode));

    node->base.type = AST_Node_Type_Definition;
    node->base.return_type = type_table_lookup(table, "Null");
    node->base.accept = generic_ast_accept;

    node->type_name = strdup(type_name);
    node->parent_name = strdup(parent_name);

    node->body = (ExpressionBlockNode*)body;
    node->scope = NULL;

    node->param_count = param_count;
    if(param_count == 0)
    {
        node->params = NULL;
    }
    else
    {
        node->params = malloc(sizeof(Param*) * node->param_count);
        for (int i = 0; i < node->param_count; i++)
        {
            Param* param = malloc(sizeof(Param));
            param->name = strdup(param_names[i]);
            param->static_type = strdup(param_types[i]);
            node->params[i] = param;
        }
    }

    node->parent_arg_count = parent_arg_count;
    if(parent_arg_count == 0)
    {
        node->parent_args = NULL;
    }
    else
    {
        node->parent_args = malloc(sizeof(ASTNode*) * node->parent_arg_count);
        for (int i = 0; i < node->parent_arg_count; i++)
        {
            node->parent_args[i] = parent_args[i];
        }
    }

    return (ASTNode*)node;
}

ASTNode* create_type_definition_list_node(TypeDefinitionNode** list, int count, TypeTable* table) {
    TypeDefinitionListNode* node = malloc(sizeof(TypeDefinitionListNode));
    if (!node) return NULL;

    node->base.type = AST_Node_Type_Definition_List;
    node->base.return_type = type_table_lookup(table, "Null");
    node->base.accept = generic_ast_accept;

    node->count = count;
    node->definitions = malloc(sizeof(TypeDefinitionNode*) * count);

    for (int i = 0; i < count; i++) {
        node->definitions[i] = list[i];
    }

    return (ASTNode*) node;
}

NewNode* create_new_node(const char* type_name, int arg_count, ASTNode** args, TypeTable* table) {
    NewNode* node = malloc(sizeof(NewNode));
    node->base.type = AST_Node_New;  
    node->base.return_type = type_table_lookup(table, "Undefined");
    node->base.accept = generic_ast_accept;
    node->type_name = strdup(type_name);
    node->arg_count = arg_count;
    
    for (int i = 0; i < arg_count; i++)
    {
        node->args[i] = args[i];
    }
    
    return node;
}

AttributeAccessNode* create_attribute_access_node(ASTNode* object, const char* attribute_name, ASTNode** args, int arg_count, bool is_method_call, TypeTable* table) {
    AttributeAccessNode* node = malloc(sizeof(AttributeAccessNode));
    node->base.type = AST_Node_Attribute_Access;
    node->base.return_type = type_table_lookup(table, "Undefined");
    node->base.accept = generic_ast_accept;

    node->object = object;
    node->attribute_name = strdup(attribute_name); // Copia del string
    node->arg_count = arg_count;
    node->is_method_call = is_method_call;
    node->scope = NULL;

    for (int i = 0; i < arg_count; i++) {
        node->args[i] = args[i]; // Copia de los nodos de argumentos
    }

    return node;
}

ASTNode* create_program_node(ASTNode* function_list, ASTNode* type_definitions, ASTNode *root, TypeTable *table) {
    ProgramNode *node = malloc(sizeof(ProgramNode));
    if (!node) return NULL;

    node->base.type = AST_Node_Program;
    node->base.return_type = type_table_lookup(table, "Null");
    node->base.accept = generic_ast_accept;
    node->function_list = (FunctionDefinitionListNode*) function_list;
    node->type_definitions = (TypeDefinitionListNode*) type_definitions;
    node->root = root;

    return (ASTNode*) node;
}

VariableAssigment* create_variable_assigment(const char* name, const char* static_type, ASTNode* value) {
    if (!name || !static_type || !value) return NULL;

    VariableAssigment* assignment = malloc(sizeof(VariableAssigment));
    if (!assignment) return NULL;

    assignment->name = strdup(name);
    assignment->static_type = strdup(static_type);
    assignment->value = value;

    return assignment;
}