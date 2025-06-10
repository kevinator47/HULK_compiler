#ifndef IR_SYMBOL_TABLE_H
#define IR_SYMBOL_TABLE_H

#include <stdbool.h>
#include <llvm-c/Core.h>

// Estructura para un símbolo (variable)
typedef struct IrSymbol {
    char* name;
    LLVMValueRef value;         // Dirección de la variable en memoria (alloca)
    struct IrSymbol* next;        // Para colisiones en la tabla hash
} IrSymbol;

// Estructura para la tabla de símbolos (un scope)
typedef struct IrSymbolTable {
    IrSymbol** table;             // Arreglo de listas enlazadas
    int size;
    struct IrSymbolTable* parent; // Scope padre (para anidamiento)
} IrSymbolTable;

// Funciones para manejar la tabla de símbolos
IrSymbolTable* create_ir_symbol_table(int size, IrSymbolTable* parent);
void free_ir_symbol_table(IrSymbolTable* st);

bool insert_ir_symbol(IrSymbolTable* st, const char* name, LLVMValueRef value);
IrSymbol* lookup_ir_symbol(IrSymbolTable* st, const char* name);

#endif // IR_SYMBOL_TABLE_H