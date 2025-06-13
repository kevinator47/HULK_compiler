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
            VariableAssigment* assigment = let_in_node->assigments[i]; 
            semantic_visit(visitor, assigment->value, let_in_node->scope);
            add_assigment_to_scope(let_in_node->scope, assigment, visitor->typeTable);
        }

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

        // Register each function definition in the current scope and register it's params in it's own scope
        for (int i = 0; i < function_list_node->function_count; i++) {
            FunctionDefinitionNode* function_node = function_list_node->functions[i];
            register_function_definition(function_node, current_scope, visitor->typeTable);
            register_func_params(function_node, current_scope, visitor->typeTable);
        }

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

        // Paso 1: Validar los argumentos al padre
        TypeDescriptor* parent_type = type_table_lookup(visitor->typeTable, type_node->parent_name);
        if (!parent_type || !parent_type->initializated || (parent_type->tag == HULK_Type_UserDefined && !parent_type->info)) {
            fprintf(stderr, "[Semantic Error] Type '%s' inherits from unknown or uninitialized type '%s'\n", type_node->type_name, type_node->parent_name);
            exit(1);
        }
        if (parent_type->tag != HULK_Type_UserDefined)
        {
            if(type_node->parent_arg_count != 0)
            {
                fprintf(stderr, "[Semantic Error] Builtin type '%s' cannot receive arguments'\n", parent_type->type_name);
                exit(1);    
            }
        }
        else if (type_node->parent_arg_count != parent_type->info->param_count) {
            fprintf(stderr, "[Semantic Error] Type '%s' inherits from '%s' with incorrect number of arguments (%d expected, %d given)\n",
                type_node->type_name, type_node->parent_name,
                parent_type->info->param_count, type_node->parent_arg_count);
            exit(1);
        }
        
        // Paso 2: Validar tipos de los argumentos heredados
        for (int i = 0; i < type_node->parent_arg_count; i++) { 

            char* arg_name = parent_type->info->params_name[i];
            SymbolTable* parent_scope = parent_type->info->scope;
            Symbol* field_symbol = lookup_symbol_type_field(parent_scope, arg_name, true);

            TypeDescriptor* expected = field_symbol->type;
            TypeDescriptor* given = semantic_visit(visitor, type_node->parent_args[i], type_node->scope);

            if (!conforms(given, expected)) {
                fprintf(stderr, "[Semantic Error] Inheritance argument %d in type '%s' expects type '%s' but got '%s'\n",
                    i, type_node->type_name,
                    expected->type_name, given->type_name);
                exit(1);
            }
        }

        // Paso 3: Validar el cuerpo del tipo
        for (int i = 0; i < type_node->body->expression_count; i++) {
            semantic_visit(visitor, type_node->body->expressions[i], type_node->scope);
        }

        return type_table_lookup(visitor->typeTable, "Null");
        break;
    }

    case AST_Node_Type_Definition_List: {
        TypeDefinitionListNode* list_node = (TypeDefinitionListNode*) node;

        // Visitar cada definiicion de tipo
        for (int i = 0; i < list_node->count; i++) {
            semantic_visit(visitor, (ASTNode*) list_node->definitions[i], current_scope);
        }

        return type_table_lookup(visitor->typeTable, "Null");
        break;
    }

    case AST_Node_Instaciate_Type: {
        InstanciateNode* instance_node = (InstanciateNode*) node;
        instance_node->symbol_table = current_scope;

        for (int i = 0; i < instance_node->arg_count; i++)
        {
            semantic_visit(visitor, (ASTNode*) instance_node->args[i], current_scope);
        }
        return check_semantic_type_instanciate_node(instance_node, current_scope, visitor->typeTable);
        break;
    }

    case AST_Node_Function_Call_Type:{
        FuntionCallTypeNode* func_call_type_node = (FuntionCallTypeNode*) node;
        func_call_type_node->scope = current_scope;

        for (int i = 0; i < func_call_type_node->arg_count; i++)
        {
            semantic_visit(visitor, (ASTNode*) func_call_type_node->args[i], current_scope);
        }
        
    }

    case AST_Node_Program: {
        ProgramNode* program_node = (ProgramNode*) node;
        register_types(program_node->type_definitions, visitor->typeTable, current_scope);
        semantic_visit(visitor, (ASTNode*)program_node->function_list, current_scope);
        semantic_visit(visitor, (ASTNode*)program_node->type_definitions, current_scope);
        program_node->base.return_type = semantic_visit(visitor, program_node->root, current_scope);
        return program_node->base.return_type;
        break; 
    }
    default:
        return type_table_lookup(visitor->typeTable,"undefined");
        break;
    };
}