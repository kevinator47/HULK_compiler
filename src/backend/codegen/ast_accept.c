
#include "../frontend/ast/ast.h" // Incluye las definiciones de nodos y NodeType
#include "llvm/generator.h" // Incluye la definicion de LLVMCodeGenerator

LLVMValueRef generic_ast_accept(struct ASTNode* self, struct LLVMCodeGenerator* visitor) {
    if (!self || !visitor) {
        fprintf(stderr, "Error critico: ASTNode o Visitor nulo en accept.\n");
        return NULL;
    }
    printf("[generic_ast_accept] self->type: %d\n", self->type);
    // Usamos el campo 'type' para saber que tipo de nodo concreto es 'self'
    // y llamar al metodo 'visit_' correspondiente del visitor.
    switch (self->type) {
        case AST_Node_Literal:
            // Casteamos y llamamos al metodo correspondiente
            if (visitor->visit_Literal)
                return visitor->visit_Literal(visitor, (LiteralNode*)self);
            break;

        case AST_Node_Unary_Operation:
             if (visitor->visit_UnaryOp)
                return visitor->visit_UnaryOp(visitor, (UnaryOperationNode*)self);
            break;

        case AST_Node_Binary_Operation:
             if (visitor->visit_BinaryOp)
                return visitor->visit_BinaryOp(visitor, (BinaryOperationNode*)self);
            break;

        
        case AST_Node_Expression_Block:
            printf("[generic_ast_accept] AST_Node_Expression_Block\n");
             if (visitor->visit_ExpressionBlock)
                return visitor->visit_ExpressionBlock(visitor, (ExpressionBlockNode*)self);
            break;    

         case AST_Node_Conditional:
             if (visitor->visit_Conditional) {
                 return visitor->visit_Conditional(visitor, (ConditionalNode*)self); // Implementacion completa
             }
            break;
        
        case AST_Node_While_Loop:
            printf("[generic_ast_accept] AST_Node_While_Loop\n");
             if (visitor->visit_WhileLoop)
                return visitor->visit_WhileLoop(visitor, (WhileLoopNode*)self);
            break;

        case AST_Node_Let_In:
             if (visitor->visit_Let) {
                return visitor->visit_Let(visitor, (LetInNode*)self); // Implementacion completa
             }
            break;

        case AST_Node_Variable:
             if (visitor->visit_Variable) {
                return visitor->visit_Variable(visitor, (VariableNode*)self); // Implementacion completa
             }
            break;

        case AST_Node_Reassign:
             if (visitor->visit_ReassignNode){
                return visitor->visit_ReassignNode(visitor, (ReassignNode*)self); //Implementación completa
            }
            break;

        case AST_Node_Function_Definition:
             if (visitor->visit_FunctionDefinition) {
                return visitor->visit_FunctionDefinition(visitor, (FunctionDefinitionNode*)self); // Implementacion completa
             }
            break;

         case AST_Node_Function_Call:
             if (visitor->visit_FunctionCall) {
                return visitor->visit_FunctionCall(visitor, (FunctionCallNode*)self); // Implementacion completa
             }
            break;
        
        case AST_Node_New:
             if (visitor->visit_NewNode) {
                return visitor->visit_NewNode(visitor, (NewNode*)self); // Implementacion completa
             }
            break;
        
        case AST_Node_Attribute_Access:
             if (visitor->visit_AttributeAccessNode) {
                return visitor->visit_AttributeAccessNode(visitor, (AttributeAccessNode*)self); // Implementacion completa
             }
            break;

        default:
            fprintf(stderr, "Error: Tipo de nodo AST desconocido o no manejado en accept: %d\n", self->type);
            return NULL;
    }

    // Si llegamos aqui, significa que el puntero visit_ para este tipo era NULL en el visitor
     fprintf(stderr, "Error: Metodo visit_ nulo para tipo %d.\n", self->type);
     return NULL;
}