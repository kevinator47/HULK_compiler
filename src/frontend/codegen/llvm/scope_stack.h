#ifndef SCOPE_STACK_H
#define SCOPE_STACK_H

#include <stddef.h>
#include <llvm-c/Core.h>

typedef struct ScopeStack {
    LLVMValueRef* values;
    size_t capacity;
    size_t size;
} ScopeStack;

ScopeStack* create_scope_stack();
void destroy_scope_stack(ScopeStack* stack);
// Puedes añadir más funciones aquí (push, pop, top, etc.)

#endif // SCOPE_STACK_H