#include <stdlib.h>
#include "type_scope_stack.h"

TypeScopeStack* create_type_scope_stack() {
    TypeScopeStack* s = malloc(sizeof(TypeScopeStack));
    s->capacity = 8;
    s->top = 0;
    s->stack = malloc(sizeof(TypeDescriptor*) * s->capacity);
    return s;
}

void push_type(TypeScopeStack* s, TypeDescriptor* t) {
    if (s->top == s->capacity) {
        s->capacity *= 2;
        s->stack = realloc(s->stack, sizeof(TypeDescriptor*) * s->capacity);
    }
    s->stack[s->top++] = t;
}

void pop_type(TypeScopeStack* s) {
    if (s->top > 0) s->top--;
}

TypeDescriptor* current_type(TypeScopeStack* s) {
    if (s->top == 0) return NULL;
    return s->stack[s->top - 1];
}