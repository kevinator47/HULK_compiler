%{
#include "ast.h"
#include "parser.tab.h"
%}

%option noyywrap

digit   [0-9]
integer {digit}+
real    {integer}(\.{integer})?
var     [a-zA-Z_][a-zA-Z0-9_]*

%%

{real}      { yylval.double_val = atof(yytext); return NUMBER; }
{var}       { yylval.string_val = strdup(yytext); return VARIABLE; }
"+"         { return ADD; }
"-"         { return SUB; }
"*"         { return MUL; }
"/"         { return DIV; }
"("         { return LPAREN; }
")"         { return RPAREN; }
[ \t\n]     ; // Ignorar espacios
.           { fprintf(stderr, "Error: Carácter inválido '%c'\n", yytext[0]); }

%%