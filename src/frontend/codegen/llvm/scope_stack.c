#include "scope_stack.h"
#include <stdlib.h>

ScopeStack* create_scope_stack() {
    ScopeStack* stack = (ScopeStack*)malloc(sizeof(ScopeStack));
    if (!stack) return NULL;
    stack->capacity = 8;
    stack->size = 0;
    stack->values = (LLVMValueRef*)malloc(sizeof(LLVMValueRef) * stack->capacity);
    if (!stack->values) {
        free(stack);
        return NULL;
    }
    return stack;
}

void destroy_scope_stack(ScopeStack* stack) {
    if (!stack) return;
    free(stack->values);
    free(stack);
}