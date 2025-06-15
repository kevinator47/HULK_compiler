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
        return type_table_lookup(visitor->typeTable,"Null");
    }
    
    switch (node->type)
    {
    case AST_Node_Literal:
        return check_semantic_literal_node(node);
        break;
    
    case AST_Node_Unary_Operation: {
        UnaryOperationNode* unary_node = (UnaryOperationNode*) node;
        semantic_visit(visitor, unary_node->operand, current_scope); // Visit the operand first
        return check_semantic_unary_operation_node(unary_node);
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
        return check_semantic_conditional_node(conditional_node);
        break;
    }
    case AST_Node_While_Loop: {
        WhileLoopNode* while_node = (WhileLoopNode*) node;
        semantic_visit(visitor, while_node->condition, current_scope); // Visit the condition first
        semantic_visit(visitor, while_node->body, current_scope);      // Visit the body next
        return check_semantic_while_loop_node(while_node);
        break;
    }
    case AST_Node_Let_In: {
        LetInNode* let_in_node = (LetInNode*) node;
        let_in_node->scope = create_symbol_table(current_scope);
        
        // Visit the assigments first
        for (int i = 0; i < let_in_node->assigment_count; i++) {
            VariableAssigmentNode* assign_node = let_in_node->assigments[i];
            semantic_visit(visitor, (ASTNode*) assign_node, let_in_node->scope);

            // Añadir la variable al scope luego del chequeo
            Symbol* symbol = create_symbol(assign_node->assigment->name, SYMBOL_VARIABLE, assign_node->base.return_type, NULL);
            insert_symbol(let_in_node->scope, symbol);
        }

        semantic_visit(visitor, let_in_node->body, let_in_node->scope);

        return check_semantic_let_in_node(let_in_node);
        
        break;
    }
    case AST_Node_Variable_Assigment: {
        VariableAssigmentNode* assign_node = (VariableAssigmentNode*) node;

        // Establecer el scope actual
        assign_node->scope = current_scope;

        // Visitar la expresión de valor primero
        semantic_visit(visitor, assign_node->assigment->value, current_scope);

        return check_semantic_variable_assigment_node(assign_node, visitor->typeTable);
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

        // Visit each function definition
        for (int i = 0; i < function_list_node->function_count; i++) {
            FunctionDefinitionNode* function_node = function_list_node->functions[i];
            semantic_visit(visitor, (ASTNode*) function_node, current_scope);
        }
        return type_table_lookup(visitor->typeTable,"Null");
        break;
    }
    case AST_Node_Function_Definition: {
        FunctionDefinitionNode* function_node = (FunctionDefinitionNode*) node;
        
        // TODO: inferencia de tipo sobre los parametros para determinar su tipo si no se especifica estaticamente

        // Visit the body of the function
        semantic_visit(visitor, function_node->body, function_node->scope);
        check_semantic_function_definition_node(function_node, visitor->typeTable);

        // TODO: Set the return type of the function in the global scope(when inference)
        
        return type_table_lookup(visitor->typeTable,"Null"); 
        break;
    }
    case AST_Node_Function_Call : {
        FunctionCallNode* func_call = (FunctionCallNode*) node;
        
        // Set scope
        func_call->scope = current_scope;

        // Visit each argument
        for (int i = 0; i < func_call->arg_count; i++)
        {
            semantic_visit(visitor, func_call->args[i] , current_scope);
        }

        return check_semantic_function_call_node(func_call, current_scope);
        break;
    }
    case AST_Node_Type_Definition: {
        TypeDefinitionNode* type_node = (TypeDefinitionNode*) node;
        
        // First visit the attribute assigment without self in the scope
        for (int i = 0; i < type_node->body->expression_count; i++) 
        {
            ASTNode* expr = type_node->body->expressions[i];
            if (expr->type == AST_Node_Variable_Assigment)
                semantic_visit(visitor, type_node->body->expressions[i], type_node->scope);
        }

        // Add the self parameter
        TypeDescriptor* descriptor = type_table_lookup(visitor->typeTable, type_node->type_name);
        Symbol* self_symbol = create_symbol("self", SYMBOL_TYPE_FIELD, descriptor, NULL);
        insert_symbol(type_node->scope, self_symbol);

        // Then visit the rest of the expression on the body
        for (int i = 0; i < type_node->body->expression_count; i++) 
        {
            ASTNode* expr = type_node->body->expressions[i];
            if (expr->type != AST_Node_Variable_Assigment) 
                semantic_visit(visitor, expr, type_node->scope);
        }

        // Finally visit the parent arguments if they exist
        for (int i = 0; i < type_node->parent_arg_count; i++) {
            semantic_visit(visitor, type_node->parent_args[i], type_node->scope);
        }
        
        return check_semantic_type_definition_node(type_node, visitor->typeTable);
        break;
    }

    case AST_Node_Type_Definition_List: {
        TypeDefinitionListNode* list_node = (TypeDefinitionListNode*) node;

        // Visitar cada definiicion de tipo
        for (int i = 0; i < list_node->count; i++) {
            semantic_visit(visitor, (ASTNode*)list_node->definitions[i], current_scope);
        }

        return type_table_lookup(visitor->typeTable, "Null");
        break;
    }
    case AST_Node_New: {
        NewNode* new_node = (NewNode*) node;

        // Visitar argumentos primero
        for (int i = 0; i < new_node->arg_count; i++) 
            semantic_visit(visitor, new_node->args[i], current_scope);
            
        return check_semantic_new_node(new_node, visitor->typeTable);
    }
    case AST_Node_Attribute_Access: {
        AttributeAccessNode* access_node = (AttributeAccessNode*) node;

        // Visitar el objeto primero
        semantic_visit(visitor, access_node->object, current_scope);

        // Visitar cada argumento si es una llamada a método
        if (access_node->is_method_call) 
        {
            for (int i = 0; i < access_node->arg_count; i++) 
                semantic_visit(visitor, access_node->args[i], current_scope);
        }

        // Realizar chequeo semántico del nodo
        return check_semantic_attribute_access_node(access_node);
    }
    case AST_Node_Program: {
        ProgramNode* program_node = (ProgramNode*) node;

        // Register types and function so they will be avaiable during the semantic check
        register_globals(program_node, current_scope, visitor->typeTable);
        
        // Visit the type and function definitions
        semantic_visit(visitor, (ASTNode*)program_node->type_definitions, current_scope);
        semantic_visit(visitor, (ASTNode*)program_node->function_list, current_scope);
        
        // Visit the root expression
        program_node->base.return_type = semantic_visit(visitor, program_node->root, current_scope);
        return program_node->base.return_type;
        break; 
    }
    default:
        return type_table_lookup(visitor->typeTable,"undefined");
        break;
    };
}