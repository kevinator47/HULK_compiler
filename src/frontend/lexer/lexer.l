%{
#include "../common/common.h"
#include "../ast/ast.h"
#include "parser.tab.h"
#include <string.h>
#include <stdlib.h>


void yyerror(const char*);
%}

%option noinput nounput

delim       [ \t\n]
ws          {delim}+
digit       [0-9]
letter      [a-zA-Z]
number      {digit}+(\.{digit}+)?
string      \"([^"\\]|\\.)*\"
id          {letter}({letter}|{digit}|_)*

%%
{ws}                { /* Ignorar espacios */ }
"true"|"false"      { yylval.ival = strcmp(yytext, "true") == 0; return BOOLEAN; }
"if"                { return IF; }
"elif"              { return ELIF; }
"else"              { return ELSE; }
"while"             { return WHILE; }
"let"               { return LET; }
"in"                { return IN; }
"function"          { return FUNCTION; }
{number}            { yylval.dval = atof(yytext); return NUMBER; }
{id}                { yylval.sval = strdup(yytext); return ID; }
";"                 { return SEMICOLON; }
"+"                 { return ADD; }
"-"                 { return SUB; }
"*"                 { return MUL; }
"/"                 { return DIV; }
"%"                 { return MOD; }
"^"|"**"            { return POW; }
"&"                 { return AND; }
"|"                 { return OR; }
"!"                 { return NOT; }
">"                 { yylval.token = GT_TK; return COMP; }
">="                { yylval.token = GE_TK; return COMP; }
"<"                 { yylval.token = LT_TK; return COMP; }
"<="                { yylval.token = LE_TK; return COMP; }
"=="                { yylval.token = EQ_TK; return COMP; }
"!="                { yylval.token = NE_TK; return COMP; }
"=>"                { return ARROW;}
"="                 { return ASSIGN; }
"@"                 { return CONCAT; }
"@@"                { return DCONCAT; }
"("                 { return LPAREN; }
")"                 { return RPAREN; }
"{"                 { return LBRACKET; }
"}"                 { return RBRACKET; }
","                 { return COMMA; }
":="                { return REASSIGN; }
":"                 { return COLON; }
{string}            {
                        char* src = yytext + 1;           // skip starting quote
                        yytext[strlen(yytext)-1] = '\0';  // remove ending quote
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
int yywrap() {
    return 1; // Indica que no hay más entradas
}