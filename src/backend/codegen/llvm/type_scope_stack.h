#ifndef TYPE_SCOPE_STACK_H
#define TYPE_SCOPE_STACK_H

#include <stdlib.h>
#include "../../../frontend/hulk_type/hulk_type.h"

typedef struct TypeScopeStack {
    TypeDescriptor** stack;
    int top;
    int capacity;
} TypeScopeStack; // Scope en el que viven los tipos mientras se definen sus métodos

TypeScopeStack* create_type_scope_stack();
void push_type(TypeScopeStack* s, TypeDescriptor* t);
void pop_type(TypeScopeStack* s);
TypeDescriptor* current_type(TypeScopeStack* s);

#endif // TYPE_SCOPE_STACK_H