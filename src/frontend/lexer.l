%{
#include "entities/ast.h"
#include "parser.tab.h"
#include <string.h>

void yyerror(const char*);
%}

delim       [ \t\n]
ws          {delim}+
digit       [0-9]
number      {digit}+(\.{digit}+)?
string      \"([^"\\]|\\.)*\"

%%

{ws}                { /* Ignorar */ }            
"true"|"false"      { yylval.ival = strcmp(yytext, "true") == 0; return BOOLEAN; }
{number}            { yylval.dval = atof(yytext); return NUMBER; }
"+"                 { return ADD; }
"-"                 { return SUB; }
"*"                 { return MUL; }
"/"                 { return DIV; }
"%"                 { return MOD; }
"^"|"**"            { return POW; }
"("                 { return LPAREN; }
")"                 { return RPAREN; }
"&"                 { return AND; }
"|"                 { return OR; }
"!"                 { return NOT; }
">"                 { yylval.token = GT_TK; return COMP; }
">="                { yylval.token = GE_TK; return COMP; }
"<"                 { yylval.token = LT_TK; return COMP; }
"<="                { yylval.token = LE_TK; return COMP; }
"=="                { yylval.token = EQ_TK; return COMP; }
"!="                { yylval.token = NE_TK; return COMP; }
"@"                 { return CONCAT; }
"@@"                { return DCONCAT; }
"{"                 { return LBRACKET; }
"}"                 { return RBRACKET; }
";"                 { return SEMICOLON; }
{string}            {
                        yytext[strlen(yytext)-1] = '\0';  // remove ending quote
                        char* src = yytext + 1;           // skip starting quote
                        char* dst = malloc(strlen(src) + 1);
                        char* result = dst;
                        
                        while (*src) {
                            if (*src == '\\' && *(src + 1) == '"') {
                                *dst++ = '"';
                                src += 2;
                            } else {
                                *dst++ = *src++;
                            }
                        }
                        *dst = '\0';
                        
                        yylval.sval = result;
                        return STRING;
                    }
%%

int yywrap(void) {
    return 1;
}