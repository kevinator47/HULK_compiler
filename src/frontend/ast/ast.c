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
    node->base.return_type = type_table_lookup(table, "number");
    node->base.accept = generic_ast_accept;
    node->value.number_value = value;

    return (ASTNode*) node;
}

ASTNode* create_string_literal_node(char *value, TypeTable *table) {
    LiteralNode *node = malloc(sizeof(LiteralNode));
    if (!node) return NULL;

    node->base.type = AST_Node_Literal;
    node->base.return_type = type_table_lookup(table, "string");
    node->base.accept = generic_ast_accept;
    node->value.string_value = strdup(value); 

    return (ASTNode*) node;
}

ASTNode* create_bool_literal_node(int value, TypeTable *table) {
    LiteralNode *node = malloc(sizeof(LiteralNode));
    if (!node) return NULL;

    node->base.type = AST_Node_Literal;
    node->base.return_type = type_table_lookup(table, "bool");
    node->base.accept = generic_ast_accept;
    node->value.bool_value = value != 0; // asegurar 0 o 1

    return (ASTNode*) node;
}
ASTNode* create_unary_operation_node(HULK_Op operator, ASTNode *operand, TypeTable *table) {
    UnaryOperationNode *node = malloc(sizeof(UnaryOperationNode));
    if (!node) return NULL;

    node->base.type = AST_Node_Unary_Operation;
    node->base.return_type = type_table_lookup(table, "undefined");
    node->base.accept = generic_ast_accept;
    node->operator = operator;
    node->operand = operand;

    return (ASTNode*) node;
}

ASTNode* create_binary_operation_node(HULK_Op operator, ASTNode *left, ASTNode *right, TypeTable *table) {
    BinaryOperationNode *node = malloc(sizeof(BinaryOperationNode));
    if (!node) return NULL;

    node->base.type = AST_Node_Binary_Operation;
    node->base.return_type = type_table_lookup(table, "undefined"); 
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
    node->base.return_type = type_table_lookup(table, "undefined");
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
    node->base.return_type = type_table_lookup(table, "undefined");
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
    node->base.return_type = type_table_lookup(table, "undefined");
    node->base.accept = generic_ast_accept;
    node->condition = condition;
    node->body = body;

    return (ASTNode*) node;
}

ASTNode* create_let_in_node(VariableAssigment*assigments, int assigment_count, ASTNode *body, TypeTable *table) {
    LetInNode *node = malloc(sizeof(LetInNode));
    if (!node) return NULL;

    node->base.type = AST_Node_Let_In;
    node->base.return_type = type_table_lookup(table, "undefined");
    node->base.accept = generic_ast_accept;
    node->scope = NULL; // scope will be created later

    node->assigments = malloc(sizeof(VariableAssigment) * assigment_count);
    if (!node->assigments) {
        free(node);
        return NULL; // Error allocating memory
    }

    for (int i = 0; i < assigment_count; i++) {
        node->assigments[i] = assigments[i]; // copy the struct
    }
    node->assigment_count = assigment_count;
    node->body = body;

    return (ASTNode*) node;
}

void create_scope_let_in_node(LetInNode* node, SymbolTable* parent_scope) {
    // Crea un nuevo scope para el bloque LET-IN
    node->scope = create_symbol_table(parent_scope);
}

ASTNode* create_variable_node(char *name, TypeTable *table) {
    VariableNode *node = malloc(sizeof(VariableNode));
    if (!node) return NULL;

    node->base.type = AST_Node_Variable;
    node->base.return_type = type_table_lookup(table, "undefined");
    node->name = strdup(name);
    node->scope = NULL;
    node->base.accept = generic_ast_accept;

    return (ASTNode*) node;
}

ASTNode* create_reassign_node(char *name, ASTNode *value, TypeTable *table) {
    ReassignNode *node = malloc(sizeof(ReassignNode));
    if (!node) return NULL;

    node->base.type = AST_Node_Reassign;
    node->base.return_type = type_table_lookup(table, "undefined");
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
    node->base.return_type = type_table_lookup(table, "null");
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
    node->base.return_type = type_table_lookup(table, "null");
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

void register_func_params(FunctionDefinitionNode* node, SymbolTable* parent_scope, TypeTable* table) {
    // Crea un nuevo scope para la función
    node->scope = create_symbol_table(parent_scope);

    // Agrega los parámetros al scope de la función
    for (int i = 0; i < node->param_count; i++) {
        char* param_name = node->params[i]->name;
        TypeDescriptor* param_type = type_table_lookup(table, node->params[i]->static_type);
        if(!param_type)
        {
            fprintf(stderr, "Error: Undefined type \"%s\" \n", node->params[i]->static_type);
            exit(1);
        }
        insert_symbol(node->scope, create_symbol(param_name, SYMBOL_PARAMETER, param_type, NULL));
    }
}

ASTNode* create_function_call_node(char* name, ASTNode** args, int arg_count, TypeTable *table) {
    FunctionCallNode *node = malloc(sizeof(FunctionCallNode));
    if (!node) return NULL;

    node->base.type = AST_Node_Function_Call;
    node->base.return_type = type_table_lookup(table, "undefined");
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

ASTNode* create_program_node(FunctionDefinitionListNode *function_list, ASTNode *root, TypeTable *table) {
    ProgramNode *node = malloc(sizeof(ProgramNode));
    if (!node) return NULL;

    node->base.type = AST_Node_Program;
    node->base.return_type = type_table_lookup(table, "null");
    node->base.accept = generic_ast_accept;
    node->function_list = function_list;
    node->root = root;

    return (ASTNode*) node;
}