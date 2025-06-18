#include "semantic_visitor.h"
#include "check_semantic.h"
#include "../common/common.h"
#include "../hulk_type/type_table.h"
#include <stdarg.h>

int semantic_error_count = 0;

void report_semantic_error(ASTNode* node, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "Semantic error at line %d: ", node->line);
    if (node->line_text)
        fprintf(stderr, "    %s\n", node->line_text);
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);
    semantic_error_count++;
}

// Imprime el backtrace simple cuando una subexpresión retorna _Error
static void print_backtrace(ASTNode* node) {
    fprintf(stderr, "Backtrace to line %d: %s\n", node->line, node->line_text ? node->line_text : "(no source)");
}

SemanticVisitor* init_semantic_visitor(TypeTable* type_table) {
    SemanticVisitor* visitor = malloc(sizeof(SemanticVisitor));
    visitor->typeTable = type_table;
    return visitor;
}

TypeDescriptor* semantic_visit(SemanticVisitor* visitor, ASTNode* node, SymbolTable* current_scope) {
    if (!node) {
        return type_table_lookup(visitor->typeTable, "Null");
    }

    TypeDescriptor* error_type = type_table_lookup(visitor->typeTable, "_Error");

    switch (node->type) {
    case AST_Node_Literal:
        return check_semantic_literal_node(node);

    case AST_Node_Unary_Operation: {
        UnaryOperationNode* unary_node = (UnaryOperationNode*) node;
        TypeDescriptor* operand_type = semantic_visit(visitor, unary_node->operand, current_scope);
        if (operand_type == error_type) {
            print_backtrace(node);
            return error_type;
        }
        return check_semantic_unary_operation_node(unary_node, visitor->typeTable);
    }

    case AST_Node_Binary_Operation: {
        BinaryOperationNode* binary_node = (BinaryOperationNode*) node;
        TypeDescriptor* left_type = semantic_visit(visitor, binary_node->left, current_scope);
        TypeDescriptor* right_type = semantic_visit(visitor, binary_node->right, current_scope);
        
        if (left_type == error_type || right_type == error_type) {
            print_backtrace(node);
            return error_type;
        }
        return check_semantic_binary_operation_node(binary_node, visitor->typeTable);
    }

    case AST_Node_Expression_Block: {
        ExpressionBlockNode* block_node = (ExpressionBlockNode*) node;
        for (int i = 0; i < block_node->expression_count; i++) {
            semantic_visit(visitor, block_node->expressions[i], current_scope);
        }
        return check_semantic_expression_block_node(block_node, visitor->typeTable);
    }

    case AST_Node_Conditional: {
        ConditionalNode* conditional_node = (ConditionalNode*) node;
        TypeDescriptor* cond_type = semantic_visit(visitor, conditional_node->condition, current_scope);
        TypeDescriptor* then_type = semantic_visit(visitor, conditional_node->then_branch, current_scope);
        TypeDescriptor* else_type = NULL;
        if (conditional_node->else_branch)
            else_type = semantic_visit(visitor, conditional_node->else_branch, current_scope);

        // Si alguna rama tiene error, imprime el backtrace pero chequea igual para propagar el tipo error
        if (cond_type == error_type || then_type == error_type || (conditional_node->else_branch && else_type == error_type)) {
            print_backtrace(node);
            return error_type;
        }
        return check_semantic_conditional_node(conditional_node, visitor->typeTable);
    }

    case AST_Node_While_Loop: {
        WhileLoopNode* while_node = (WhileLoopNode*) node;
        TypeDescriptor* cond_type = semantic_visit(visitor, while_node->condition, current_scope);
        // Analiza el cuerpo siempre, aunque la condición tenga error
        TypeDescriptor* body_type = semantic_visit(visitor, while_node->body, current_scope);
        if (cond_type == error_type) {
            print_backtrace(node);
            return error_type;
        }
        return check_semantic_while_loop_node(while_node, visitor->typeTable);
    }

    case AST_Node_Let_In: {
        LetInNode* let_in_node = (LetInNode*) node;
        let_in_node->scope = create_symbol_table(current_scope);

        for (int i = 0; i < let_in_node->assigment_count; i++) {
            VariableAssigmentNode* assign_node = let_in_node->assigments[i];
            semantic_visit(visitor, (ASTNode*) assign_node, let_in_node->scope);
            Symbol* symbol = create_symbol(assign_node->assigment->name, SYMBOL_VARIABLE, assign_node->base.return_type, NULL);
            insert_symbol(let_in_node->scope, symbol);
        }

        semantic_visit(visitor, let_in_node->body, let_in_node->scope);
        return check_semantic_let_in_node(let_in_node, visitor->typeTable);
    }

    case AST_Node_Variable_Assigment: {
        VariableAssigmentNode* assign_node = (VariableAssigmentNode*) node;
        assign_node->scope = current_scope;
        semantic_visit(visitor, assign_node->assigment->value, current_scope);
        return check_semantic_variable_assigment_node(assign_node, visitor->typeTable);
    }

    case AST_Node_Variable: {
        VariableNode* variable_node = (VariableNode*) node;
        variable_node->scope = current_scope;
        return check_semantic_variable_node(variable_node, visitor->typeTable);
    }

    case AST_Node_Reassign: {
        ReassignNode* reassign_node = (ReassignNode*) node;
        reassign_node->scope = current_scope;
        semantic_visit(visitor, reassign_node->value, current_scope);
        return check_semantic_reassign_node(reassign_node, visitor->typeTable);
    }

    case AST_Node_Function_Definition_List: {
        FunctionDefinitionListNode* function_list_node = (FunctionDefinitionListNode*) node;
        for (int i = 0; i < function_list_node->function_count; i++) {
            FunctionDefinitionNode* function_node = function_list_node->functions[i];
            semantic_visit(visitor, (ASTNode*) function_node, current_scope);
        }
        return type_table_lookup(visitor->typeTable, "Null");
    }

    case AST_Node_Function_Definition: {
        FunctionDefinitionNode* function_node = (FunctionDefinitionNode*) node;
        TypeDescriptor* body_type = semantic_visit(visitor, function_node->body, function_node->scope);
        if(body_type == error_type)
        {
            print_backtrace(node);
            return error_type;
        }
        check_semantic_function_definition_node(function_node, visitor->typeTable);
        return type_table_lookup(visitor->typeTable, "Null");
    }

    case AST_Node_Function_Call: {
        FunctionCallNode* func_call = (FunctionCallNode*) node;
        func_call->scope = current_scope;
        for (int i = 0; i < func_call->arg_count; i++) {
            semantic_visit(visitor, func_call->args[i], current_scope);
        }
        return check_semantic_function_call_node(func_call, current_scope, visitor->typeTable);
    }

    case AST_Node_Type_Definition: {
        bool has_error = false ;
        TypeDefinitionNode* type_node = (TypeDefinitionNode*) node;
        //  First visit the attributes initialization

        for (int i = 0; i < type_node->body->expression_count; i++) {
            ASTNode* expr = type_node->body->expressions[i];
            if (expr->type == AST_Node_Variable_Assigment)
            {
                TypeDescriptor* assing_type = semantic_visit(visitor, expr, type_node->scope);
                if(assing_type == error_type)
                {
                    has_error = true;
                    print_backtrace(node);
                }
            }
        }

        if (has_error)
            return error_type;

        // Add the self field
        TypeDescriptor* descriptor = type_table_lookup(visitor->typeTable, type_node->type_name);
        Symbol* self_symbol = create_symbol("self", SYMBOL_TYPE_FIELD, descriptor, NULL);
        insert_symbol(type_node->scope, self_symbol);
        
        // Visit the method definitions
        for (int i = 0; i < type_node->body->expression_count; i++) 
        {
            ASTNode* expr = type_node->body->expressions[i];
            if (expr->type != AST_Node_Variable_Assigment)
            {
                TypeDescriptor* method_type = semantic_visit(visitor, expr, type_node->scope);
                if(method_type == error_type)
                {
                    has_error = true;
                    print_backtrace(node);
                }
            }
                
        }

        for (int i = 0; i < type_node->parent_arg_count; i++) 
        {   
            TypeDescriptor* arg_type = semantic_visit(visitor, type_node->parent_args[i], type_node->scope);
            if(arg_type == error_type)
            {
                has_error = true;
                print_backtrace(node);
            }
        }
        return check_semantic_type_definition_node(type_node, visitor->typeTable);
    }

    case AST_Node_Type_Definition_List: {
        TypeDefinitionListNode* list_node = (TypeDefinitionListNode*) node;
        for (int i = 0; i < list_node->count; i++) {
            semantic_visit(visitor, (ASTNode*)list_node->definitions[i], current_scope);
        }
        return type_table_lookup(visitor->typeTable, "Null");
    }

    case AST_Node_New: {
        NewNode* new_node = (NewNode*) node;
        for (int i = 0; i < new_node->arg_count; i++)
            semantic_visit(visitor, new_node->args[i], current_scope);
        return check_semantic_new_node(new_node, visitor->typeTable);
    }

    case AST_Node_Attribute_Access: {
        AttributeAccessNode* access_node = (AttributeAccessNode*) node;
        semantic_visit(visitor, access_node->object, current_scope);
        if (access_node->is_method_call) {
            for (int i = 0; i < access_node->arg_count; i++)
                semantic_visit(visitor, access_node->args[i], current_scope);
        }
        return check_semantic_attribute_access_node(access_node, visitor->typeTable);
    }

    case AST_Node_Program: {
        ProgramNode* program_node = (ProgramNode*) node;
        register_globals(program_node, current_scope, visitor->typeTable);
        semantic_visit(visitor, (ASTNode*)program_node->type_definitions, current_scope);
        semantic_visit(visitor, (ASTNode*)program_node->function_list, current_scope);
        program_node->base.return_type = semantic_visit(visitor, program_node->root, current_scope);
        return program_node->base.return_type;
    }

    default:
        return type_table_lookup(visitor->typeTable, "undefined");
    }
}