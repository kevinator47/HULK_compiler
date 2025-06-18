#include "check_semantic.h"
#include "../hulk_type/type_table.h"
#include "../common/common.h"

TypeDescriptor* check_semantic_literal_node(ASTNode* node) {
    return node->return_type;
}

TypeDescriptor* check_semantic_unary_operation_node(UnaryOperationNode* node, TypeTable* table) {
    ASTNode* operand = node->operand;
    TypeDescriptor* error_type = type_table_lookup(table, "_Error");
    switch (node->operator) {
        case NOT_TK:
            if (operand->return_type->tag != HULK_Type_Boolean) {
                report_semantic_error((ASTNode*)node, "Invalid type for operator \"%s\": expected \"Bool\" but received \"%s\"", Hulk_Op_Names[node->operator], operand->return_type->type_name);
                node->base.return_type = error_type;
                return error_type;
            }
            break;
        case MINUS_TK:
            if (operand->return_type->tag != HULK_Type_Number) {
                report_semantic_error((ASTNode*)node, "Invalid type for operator \"%s\": expected \"Number\" but received \"%s\"", Hulk_Op_Names[node->operator], operand->return_type->type_name);
                node->base.return_type = error_type;
                return error_type;
            }
            break;
        default:
            DIE("Unsupported unary operator");
    }
    node->base.return_type = operand->return_type;
    return node->base.return_type;
}

TypeDescriptor* check_semantic_binary_operation_node(BinaryOperationNode* node, TypeTable* table) {
    ASTNode* left = node->left;
    ASTNode* right = node->right;
    TypeDescriptor* number_type = type_table_lookup(table, "Number");
    TypeDescriptor* bool_type = type_table_lookup(table, "Bool");
    TypeDescriptor* string_type = type_table_lookup(table, "String");
    TypeDescriptor* error_type = type_table_lookup(table, "_Error");

    // Evita errores en cascada
    if (left->return_type == error_type || right->return_type == error_type) {
        node->base.return_type = error_type;
        return error_type;
    }

    switch (node->operator) {
        case PLUS_TK:
        case MINUS_TK:
        case MULT_TK:
        case DIV_TK:
        case MOD_TK:
        case EXP_TK:
            if (left->return_type->tag != HULK_Type_Number || right->return_type->tag != HULK_Type_Number) {
                report_semantic_error((ASTNode*)node, "Invalid types for operator \"%s\": expected \"Number\" and \"Number\" but received \"%s\" and \"%s\"",
                    Hulk_Op_Names[node->operator], left->return_type->type_name, right->return_type->type_name);
                node->base.return_type = error_type;
                return error_type;
            }
            node->base.return_type = number_type;
            break;

        case EQ_TK:
        case NE_TK:
        case GT_TK:
        case GE_TK:
        case LT_TK:
        case LE_TK:
            if (left->return_type->tag != HULK_Type_Number || right->return_type->tag != HULK_Type_Number) {
                report_semantic_error((ASTNode*)node, "Invalid types for operator \"%s\": expected \"Number\" and \"Number\" but received \"%s\" and \"%s\"",
                    Hulk_Op_Names[node->operator], left->return_type->type_name, right->return_type->type_name);
                node->base.return_type = error_type;
                return error_type;
            }
            node->base.return_type = bool_type;
            break;

        case AND_TK:
        case OR_TK:
            if (left->return_type->tag != HULK_Type_Boolean || right->return_type->tag != HULK_Type_Boolean) {
                report_semantic_error((ASTNode*)node, "Invalid types for operator \"%s\": expected \"Bool\" and \"Bool\" but received \"%s\" and \"%s\"",
                    Hulk_Op_Names[node->operator], left->return_type->type_name, right->return_type->type_name);
                node->base.return_type = error_type;
                return error_type;
            }
            node->base.return_type = bool_type;
            break;

        case CONCAT_TK:
        case D_CONCAT_TK:
            if (left->return_type->tag != HULK_Type_String) {
                report_semantic_error((ASTNode*)node, "Invalid types for operator \"%s\": expected \"String\" and \"Any\" but received \"%s\" and \"%s\"",
                    Hulk_Op_Names[node->operator], left->return_type->type_name, right->return_type->type_name);
                node->base.return_type = error_type;
                return error_type;
            }
            node->base.return_type = string_type;
            break;

        default:
            DIE("Unsupported binary operator");
    }

    return node->base.return_type;
}

TypeDescriptor* check_semantic_expression_block_node(ExpressionBlockNode* node, TypeTable* table) {
    if (node->expression_count == 0) 
        node->base.return_type = type_table_lookup(table, "Null");
    else 
        node->base.return_type = node->expressions[node->expression_count - 1]->return_type;
    return node->base.return_type;
}

TypeDescriptor* check_semantic_conditional_node(ConditionalNode* node, TypeTable* table) {
    TypeDescriptor* error_type = type_table_lookup(table, "_Error");
    ASTNode* condition = node->condition;
    if (condition->return_type == error_type) {
        node->base.return_type = error_type;
        return error_type;
    }
    if (condition->return_type->tag != HULK_Type_Boolean) {
        report_semantic_error((ASTNode*)node, "Invalid type for condition on if statement, expected \"Bool\" but received \"%s\"", condition->return_type->type_name);
        node->base.return_type = error_type;
        return error_type;
    }

    ASTNode* then_branch = node->then_branch;
    ASTNode* else_branch = node->else_branch;

    if ((then_branch && then_branch->return_type == error_type) || (else_branch && else_branch->return_type == error_type)) {
        node->base.return_type = error_type;
        return error_type;
    }

    if (!else_branch || conforms(then_branch->return_type, else_branch->return_type))
        node->base.return_type = then_branch->return_type;
    else {
        report_semantic_error((ASTNode*)node, "Then and Else branches of if-else statement must match, but received \"%s\" and \"%s\"",
            then_branch->return_type->type_name, else_branch->return_type->type_name);
        node->base.return_type = error_type;
    }

    return node->base.return_type;
}

TypeDescriptor* check_semantic_while_loop_node(WhileLoopNode* node, TypeTable* table) {
    TypeDescriptor* error_type = type_table_lookup(table, "_Error");
    ASTNode* condition = node->condition;
    if (condition->return_type == error_type) {
        node->base.return_type = error_type;
        return error_type;
    }
    if (condition->return_type->tag != HULK_Type_Boolean) {
        report_semantic_error((ASTNode*)node, "Invalid type for condition on while loop statement, expected \"Bool\" but received \"%s\"", condition->return_type->type_name);
        node->base.return_type = error_type;
        return error_type;
    }
    node->base.return_type = node->body->return_type;
    return node->base.return_type;
}

TypeDescriptor* check_semantic_let_in_node(LetInNode* node, TypeTable* table) {
    node->base.return_type = node->body->return_type;
    return node->base.return_type;
}

TypeDescriptor* check_semantic_variable_assigment_node(VariableAssigmentNode* node, TypeTable* table) {
    TypeDescriptor* error_type = type_table_lookup(table, "_Error");
    VariableAssigment* assign = node->assigment;
    TypeDescriptor* static_type = require_type(table, assign->static_type);

    if (!assign->value || !assign->value->return_type || assign->value->return_type == error_type) {
        report_semantic_error((ASTNode*)node, "Invalid return value on assignment to variable '%s'", assign->name);
        node->base.return_type = error_type;
        return error_type;
    }

    TypeDescriptor* dynamic_type = assign->value->return_type;

    if (static_type->tag != HULK_Type_Undefined && !conforms(dynamic_type, static_type)) {
        report_semantic_error((ASTNode*)node, "Type '%s' of value is not compatible with declared type '%s' in variable '%s'",
            dynamic_type->type_name, static_type->type_name, assign->name);
        node->base.return_type = error_type;
        return error_type;
    }

    node->base.return_type = static_type->tag == HULK_Type_Undefined ? dynamic_type : static_type;
    return node->base.return_type;
}

TypeDescriptor* check_semantic_variable_node(VariableNode* node, TypeTable* table) {
    TypeDescriptor* error_type = type_table_lookup(table, "_Error");
    Symbol* symbol = lookup_symbol(node->scope, node->name, SYMBOL_ANY, true);
    if (!symbol) {
        report_semantic_error((ASTNode*)node, "Use of undeclared variable '%s'", node->name);
        node->base.return_type = error_type;
        return error_type;
    }
    node->base.return_type = symbol->type;
    return node->base.return_type;
}

TypeDescriptor* check_semantic_reassign_node(ReassignNode* node, TypeTable* table) {
    TypeDescriptor* error_type = type_table_lookup(table, "_Error");
    Symbol* symbol = lookup_symbol(node->scope, node->name, SYMBOL_ANY, true);
    if (!symbol) {
        report_semantic_error((ASTNode*)node, "Reassignment to undeclared variable '%s'", node->name);
        node->base.return_type = error_type;
        return error_type;
    }
    if (symbol->kind == SYMBOL_FUNCTION || symbol->kind == SYMBOL_TYPE_METHOD) {
        report_semantic_error((ASTNode*)node, "Invalid reassignment: '%s' is not a variable", node->name);
        node->base.return_type = error_type;
        return error_type;
    }
    if (!conforms(node->value->return_type, symbol->type)) {
        report_semantic_error((ASTNode*)node, "Type mismatch in reassignment to variable '%s': expected '%s', got '%s'",
            node->name, symbol->type->type_name, node->value->return_type->type_name);
        node->base.return_type = error_type;
        return error_type;
    }
    node->base.return_type = node->value->return_type;
    return node->base.return_type;
}

TypeDescriptor* check_semantic_function_definition_node(FunctionDefinitionNode* node, TypeTable* table) {
    TypeDescriptor* error_type = type_table_lookup(table, "_Error");
    TypeDescriptor* body_return_type = node->body->return_type;
    TypeDescriptor* static_return_type = type_table_lookup(table, node->static_return_type);

    if (!static_return_type) {
        report_semantic_error((ASTNode*)node, "Undefined type \"%s\" for function return type", node->static_return_type);
        return error_type;
    }
    if (!body_return_type || body_return_type == error_type) {
        report_semantic_error((ASTNode*)node, "Undefined return type in function body");
        return error_type;
    }
    if (static_return_type->tag == HULK_Type_Undefined || !conforms(body_return_type, static_return_type)) {
        report_semantic_error((ASTNode*)node, "Function %s static return type is %s, but it returns %s",
            node->name, node->static_return_type, body_return_type->type_name);
        return error_type;
    }
    return body_return_type;
}

TypeDescriptor* check_semantic_function_call_node(FunctionCallNode* node, SymbolTable* current_scope, TypeTable* table) {
    TypeDescriptor* error_type = type_table_lookup(table, "_Error");
    Symbol* func_symbol = lookup_function_by_signature(current_scope, node->name, node->arg_count);
    if (!func_symbol) {
        report_semantic_error((ASTNode*)node, "Undefined function '%s' with %d arguments", node->name, node->arg_count);
        node->base.return_type = error_type;
        return error_type;
    }
    FunctionDefinitionNode* func_def = ((FunctionDefinitionNode*)func_symbol->value);
    for (int i = 0; i < node->arg_count; i++) {
        TypeDescriptor* arg_type = node->args[i]->return_type;
        TypeDescriptor* expected_type = lookup_symbol(func_def->scope, func_def->params[i]->name, SYMBOL_PARAMETER, false)->type;
        if (!conforms(arg_type, expected_type)) {
            report_semantic_error((ASTNode*)node, "Type error in argument %d of function '%s': expected '%s', got '%s'",
                i, node->name, expected_type->type_name, arg_type->type_name);
            node->base.return_type = error_type;
            return error_type;
        }
    }
    node->base.return_type = func_symbol->type;
    return node->base.return_type;
}

TypeDescriptor* check_semantic_type_definition_node(TypeDefinitionNode* node, TypeTable* table) {
    TypeDescriptor* error_type = type_table_lookup(table, "_Error");
    TypeDescriptor* parent_node = type_table_lookup(table, node->parent_name);
    if (!parent_node) {
        report_semantic_error((ASTNode*)node, "Undefined type \"%s\"", node->parent_name);
        return error_type;
    }
    if (cmp_type(parent_node, type_table_lookup(table, "Object"))) {
        if (node->parent_arg_count != 0) {
            report_semantic_error((ASTNode*)node, "Type 'Object' does not accept parameters in its constructor, but %d were provided.", node->parent_arg_count);
            return error_type;
        }
    } else {
        if (node->parent_arg_count != parent_node->info->param_count) {
            report_semantic_error((ASTNode*)node, "Invalid type init, type \"%s\" constructor expects %d parameters but received %d",
                node->parent_name, parent_node->info->param_count, node->parent_arg_count);
            return error_type;
        }
        for (int i = 0; i < node->parent_arg_count; i++) {
            TypeDescriptor* arg_type = node->parent_args[i]->return_type;
            char* param_name = parent_node->info->type_def->params[i]->name;
            TypeDescriptor* expected_type = lookup_symbol(parent_node->info->scope, param_name, SYMBOL_PARAMETER, false)->type;
            if (!conforms(arg_type, expected_type)) {
                report_semantic_error((ASTNode*)node, "Type error, \"%s\" type was expecting \"%s\" type for \"%s\", but received \"%s\"",
                    node->parent_name, expected_type->type_name, param_name, arg_type->type_name);
                return error_type;
            }
        }
    }
    return type_table_lookup(table, "Null");
}

TypeDescriptor* check_semantic_new_node(NewNode* node, TypeTable* type_table) {
    TypeDescriptor* error_type = type_table_lookup(type_table, "_Error");
    TypeDescriptor* type_desc = type_table_lookup(type_table, node->type_name);
    if (!type_desc) {
        report_semantic_error((ASTNode*)node, "Type '%s' not found.", node->type_name);
        return error_type;
    }
    if (!type_desc->info) {
        report_semantic_error((ASTNode*)node, "Type '%s' is not instantiable (no type_info defined).", node->type_name);
        return error_type;
    }
    int param_count = type_desc->info->param_count;
    char** params_name = type_desc->info->params_name;
    if (node->arg_count != param_count) {
        report_semantic_error((ASTNode*)node, "Type '%s' expects %d arguments for the constructor, but %d were provided.",
            node->type_name, param_count, node->arg_count);
        return error_type;
    }
    for (int i = 0; i < node->arg_count; i++) {
        TypeDescriptor* arg_type = node->args[i]->return_type;
        if (!arg_type || arg_type == error_type) {
            report_semantic_error((ASTNode*)node, "Could not determine the type of argument %d.", i);
            return error_type;
        }
        Symbol* param_symbol = lookup_symbol(type_desc->info->scope, params_name[i], SYMBOL_PARAMETER, false);
        if (!param_symbol) {
            report_semantic_error((ASTNode*)node, "Internal error: Parameter '%s' not found in type '%s'.", params_name[i], node->type_name);
            return error_type;
        }
        TypeDescriptor* expected_type = param_symbol->type;
        if (!expected_type) {
            report_semantic_error((ASTNode*)node, "Internal error: Parameter '%s' has no defined type.", params_name[i]);
            return error_type;
        }
        if (!conforms(arg_type, expected_type)) {
            report_semantic_error((ASTNode*)node, "Argument %d ('%s') has incompatible type (expected '%s', got '%s').",
                i, params_name[i], expected_type->type_name, arg_type->type_name);
            return error_type;
        }
    }
    node->base.return_type = type_desc;
    return type_desc;
}

TypeDescriptor* check_semantic_attribute_access_node(AttributeAccessNode* node, TypeTable* table) {
    TypeDescriptor* error_type = type_table_lookup(table, "_Error");
    if (!node || !node->object) {
        report_semantic_error((ASTNode*)node, "Internal Error: Null AttributeAccessNode or object.");
        return error_type;
    }
    TypeDescriptor* obj_type = node->object->return_type;
    if (!obj_type || obj_type->tag != HULK_Type_UserDefined || !obj_type->info) {
        report_semantic_error((ASTNode*)node, "Cannot access '%s' on non-instantiable type.", node->attribute_name);
        return error_type;
    }
    SymbolTable* type_scope = obj_type->info->scope;
    if (!node->is_method_call) {
        Symbol* attr_symbol = lookup_symbol(type_scope, node->attribute_name, SYMBOL_TYPE_FIELD, false);
        if (!attr_symbol) {
            report_semantic_error((ASTNode*)node, "'%s' is not defined as an attribute in type '%s'.", node->attribute_name, obj_type->type_name);
            return error_type;
        }
        bool is_self = node->object->type == AST_Node_Variable && strcmp(((VariableNode*)node->object)->name, "self") == 0;
        if (!is_self) {
            report_semantic_error((ASTNode*)node, "Field '%s' of type '%s' is private and can only be accessed from within the type.", node->attribute_name, obj_type->type_name);
            return error_type;
        }
        node->base.return_type = attr_symbol->type;
        return node->base.return_type;
    } else {
        Symbol* method_symbol = NULL;
        TypeDescriptor* current = obj_type;
        while (!method_symbol) {
            method_symbol = lookup_symbol(current->info->scope, node->attribute_name, SYMBOL_TYPE_METHOD, false);
            if (current->parent->tag != HULK_Type_UserDefined)
                break;
            current = current->parent;
        }
        if (!method_symbol || !method_symbol->value) {
            report_semantic_error((ASTNode*)node, "Method '%s' not found in type '%s'.", node->attribute_name, obj_type->type_name);
            return error_type;
        }
        FunctionDefinitionNode* method = (FunctionDefinitionNode*)method_symbol->value;
        if (method->param_count != node->arg_count) {
            report_semantic_error((ASTNode*)node, "Method '%s' expects %d arguments but got %d.", node->attribute_name, method->param_count, node->arg_count);
            return error_type;
        }
        for (int i = 0; i < node->arg_count; i++) {
            TypeDescriptor* expected = lookup_symbol(method->scope, method->params[i]->name, SYMBOL_PARAMETER, false)->type;
            TypeDescriptor* actual = node->args[i]->return_type;
            if (!expected || !actual || actual == error_type) {
                report_semantic_error((ASTNode*)node, "Argument %d for method '%s' has undefined type.", i + 1, node->attribute_name);
                return error_type;
            }
            if (!conforms(actual, expected)) {
                report_semantic_error((ASTNode*)node, "Argument %d for method '%s' expects type '%s' but got '%s'.",
                    i + 1, node->attribute_name, expected->type_name, actual->type_name);
                return error_type;
            }
        }
        node->base.return_type = method_symbol->type;
        return node->base.return_type;
    }
}
