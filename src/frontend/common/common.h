#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Foward Declaration
typedef struct ASTNode ASTNode;

// Variables globales del compilador
extern const char *Hulk_Op_Names[] ;
extern int line_num;
extern char current_line[];
extern int semantic_error_count;
typedef enum {
    HULK_Type_Undefined, HULK_Type_Null,
    HULK_Type_Number, HULK_Type_String, HULK_Type_Boolean,
    HULK_Type_UserDefined, HULK_Type_Object, HULK_Type_Error,
} HULK_Type;

typedef enum {
    AND_TK, OR_TK, NOT_TK,                                  // Operadores lógicos
    PLUS_TK, MINUS_TK, MULT_TK, DIV_TK, MOD_TK, EXP_TK,     // Operadores aritméticos
    CONCAT_TK, D_CONCAT_TK,                                 // Operadores de concatenación
    GT_TK, GE_TK, LT_TK, LE_TK, EQ_TK, NE_TK,               // Operadores de comparación
} HULK_Op;

typedef enum {
    SYMBOL_VARIABLE,
    SYMBOL_FUNCTION,
    SYMBOL_PARAMETER,
    SYMBOL_TYPE_FIELD,
    SYMBOL_TYPE_METHOD,
    SYMBOL_ANY,
} SymbolKind;

void report_semantic_error(ASTNode* node, const char* fmt, ...);

#define DIE(msg) do { fprintf(stderr, "Error: %s\n", msg); exit(EXIT_FAILURE); } while (0)

#endif
