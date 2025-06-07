#include "semantic_visitor.h"
#include "check_semantic.h"
#include "../common/common.h"
#include "../hulk_type/type_table.h"

SemanticVisitor* init_semantic_visitor(TypeTable* type_table) {
    SemanticVisitor* visitor = malloc(sizeof(SemanticVisitor));
    visitor->typeTable = type_table;
    return visitor;
}

TypeDescriptor* semantic_visit(SemanticVisitor* visitor, ASTNode* node, SymbolTable* current_scope) {
    if(!node) {
        return type_table_lookup(visitor->typeTable,"undefined");
    }
    
    switch (node->type)
    {
    case AST_Node_Literal:
        return check_semantic_literal_node(node);
        break;
    
    case AST_Node_Unary_Operation: {
        UnaryOperationNode* unary_node = (UnaryOperationNode*) node;
        semantic_visit(visitor, unary_node->operand, current_scope); // Visit the operand first
        return check_semantic_unary_operation_node(unary_node, visitor->typeTable);
        break;
    }
    case AST_Node_Binary_Operation: {
        BinaryOperationNode* binary_node = (BinaryOperationNode*) node;
        semantic_visit(visitor, binary_node->left, current_scope);  // Visit the left operand first
        semantic_visit(visitor, binary_node->right, current_scope); // Visit the right operand next
        return check_semantic_binary_operation_node(binary_node, visitor->typeTable);
        break;
    }
    case AST_Node_Expression_Block: {
        ExpressionBlockNode* block_node = (ExpressionBlockNode*) node;
        for (int i = 0; i < block_node->expression_count; i++) {
            semantic_visit(visitor, block_node->expressions[i], current_scope); // Visit each expression in the block
        }
        return check_semantic_expression_block_node(block_node, visitor->typeTable);
        break;
    }
    case AST_Node_Conditional: {
        ConditionalNode* conditional_node = (ConditionalNode*) node;
        semantic_visit(visitor, conditional_node->condition, current_scope); // Visit the condition first
        semantic_visit(visitor, conditional_node->then_branch,current_scope); // Visit the then branch next
        if (conditional_node->else_branch) {
            semantic_visit(visitor, conditional_node->else_branch, current_scope); // Visit the else branch if it exists
        }
        return check_semantic_conditional_node(conditional_node, visitor->typeTable);
        break;
    }
    case AST_Node_While_Loop: {
        WhileLoopNode* while_node = (WhileLoopNode*) node;
        semantic_visit(visitor, while_node->condition, current_scope); // Visit the condition first
        semantic_visit(visitor, while_node->body, current_scope); // Visit the body next
        return check_semantic_while_loop_node(while_node, visitor->typeTable);
        break;
    }
    case AST_Node_Let_In: {
        LetInNode* let_in_node = (LetInNode*) node;

        create_scope_let_in_node(let_in_node, current_scope);        
        // Visit the assigments first
        for (int i = 0; i < let_in_node->assigment_count; i++) {
            VariableAssigment* assigment = &let_in_node->assigments[i];
            semantic_visit(visitor, assigment->value, let_in_node->scope);
            add_assigment_to_scope(let_in_node->scope, assigment); 
        }        

        // Visit the body
        semantic_visit(visitor, let_in_node->body, let_in_node->scope);
        return check_semantic_let_in_node(let_in_node);
        break;
    }
    case AST_Node_Variable: {
        VariableNode* variable_node = (VariableNode*) node;
        variable_node->scope = current_scope;
        return check_semantic_variable_node(variable_node);
        break;
    }
    case AST_Node_Reassign: {
        ReassignNode* reassign_node = (ReassignNode*) node;
        reassign_node->scope = current_scope;
        semantic_visit(visitor, reassign_node->value, current_scope); // Visit the value first
        
        return check_semantic_reassign_node(reassign_node);
        break;
    }
    case AST_Node_Function_Definition_List: {
        FunctionDefinitionListNode* function_list_node = (FunctionDefinitionListNode*) node;

        // Register each function definition in the current scope
        for (int i = 0; i < function_list_node->function_count; i++) {
            FunctionDefinitionNode* function_node = function_list_node->functions[i];
            register_function_definition(function_node, current_scope);
        }

        // Visit each function definition
        for (int i = 0; i < function_list_node->function_count; i++) {
            FunctionDefinitionNode* function_node = function_list_node->functions[i];
            semantic_visit(visitor, (ASTNode*) function_node, current_scope);
        }
        return type_table_lookup(visitor->typeTable,"null");
        break;
    }
    case AST_Node_Function_Definition: {
        FunctionDefinitionNode* function_node = (FunctionDefinitionNode*) node;
        
        // Creates function's scope and register the parameters
        register_func_params(function_node, current_scope, visitor->typeTable);

        // Visit the body of the function
        semantic_visit(visitor, function_node->body, function_node->scope);
        TypeDescriptor* return_type = check_semantic_function_definition_node(function_node, visitor->typeTable);

        // Set the return type of the function in the global scope
        set_symbol_return_type(current_scope, function_node->name, return_type);
        return return_type;     
    
        break;
    }
    case AST_Node_Function_Call : {
        FunctionCallNode* func_call = (FunctionCallNode*) node;
        // Set scope
        func_call->scope = current_scope;

        // Find function declaration with the same signature
        Symbol* func_definition = lookup_function_by_signature(current_scope, func_call->name, func_call->arg_count);
        
        if(!func_definition) {
            fprintf(stderr, "Error: Undefined function '%s' with %d arguments\n", func_call->name, func_call->arg_count);
            exit(1);
        }

        SymbolTable* def_scope = ((FunctionDefinitionNode*)func_definition->value)->scope;

        // Visit each argument and check if it's type is compatible with the inferred one on the declaration
        for (int i = 0; i < func_call->arg_count; i++)
        {
            TypeDescriptor* arg_type = semantic_visit(visitor, func_call->args[i], current_scope);
            TypeDescriptor* expected_type = lookup_symbol(def_scope, func_definition->params_names[i], false)->type;
            if(arg_type->tag != expected_type->tag && arg_type->tag != HULK_Type_Any && expected_type->tag != HULK_Type_Any)
            {
                fprintf(stderr, "Type error in argument %d of function '%s'\n", i, func_call->name);
                exit(1);
            }
        }
        
        // Set the return type to the declaration's return type
        func_call->base.return_type = ((FunctionDefinitionNode*)func_definition->value)->body->return_type;
        return func_call->base.return_type;
        break;
    }
    case AST_Node_Program: {
        ProgramNode* program_node = (ProgramNode*) node;
        semantic_visit(visitor, (ASTNode*)program_node->function_list, current_scope);
        semantic_visit(visitor, program_node->root, current_scope);
        return program_node->base.return_type;
        break; 
    }
    default:
        return type_table_lookup(visitor->typeTable,"undefined");
        break;
    };
}