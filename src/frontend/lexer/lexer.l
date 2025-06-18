%{
#include "../common/common.h"
#include "../ast/ast.h"
#include "parser.tab.h"
#include <string.h>
#include <stdlib.h>

// for error messages
int line_num = 1;
char current_line[1024];

#define APPEND_TO_LINE() \
    do { \
        size_t len = strlen(current_line); \
        size_t tlen = strlen(yytext); \
        if (len + tlen + 1 < sizeof(current_line) - 1) { \
            strcat(current_line, yytext); \
            strcat(current_line, " "); \
        } \
    } while(0)
void yyerror(const char*);
%}

%option noinput nounput

delim       [ \t]
ws          {delim}+
digit       [0-9]
letter      [a-zA-Z]
number      {digit}+(\.{digit}+)?
string      \"([^"\\]|\\.)*\"
id          {letter}({letter}|{digit}|_)*

%%
\n                  { line_num++; current_line[0] = '\0'; }
{ws}                { /* Ignorar espacios */ }
"true"|"false"      { APPEND_TO_LINE(); yylval.ival = strcmp(yytext, "true") == 0; return BOOLEAN; }
"if"                { APPEND_TO_LINE(); return IF; }
"elif"              { APPEND_TO_LINE(); return ELIF; }
"else"              { APPEND_TO_LINE(); return ELSE; }
"while"             { APPEND_TO_LINE(); return WHILE; }
"let"               { APPEND_TO_LINE(); return LET; }
"inherits"          { APPEND_TO_LINE(); return INHERITS; }
"in"                { APPEND_TO_LINE(); return IN; }
"type"              { APPEND_TO_LINE(); return TYPE; }
"new"               { APPEND_TO_LINE(); return NEW; }
"function"          { APPEND_TO_LINE(); return FUNCTION; }
{number}            { APPEND_TO_LINE(); yylval.dval = atof(yytext); return NUMBER; }
{id}                { APPEND_TO_LINE(); yylval.sval = strdup(yytext); return ID; }
";"                 { APPEND_TO_LINE(); return SEMICOLON; }
"+"                 { APPEND_TO_LINE(); return ADD; }
"-"                 { APPEND_TO_LINE(); return SUB; }
"*"                 { APPEND_TO_LINE(); return MUL; }
"/"                 { APPEND_TO_LINE(); return DIV; }
"%"                 { APPEND_TO_LINE(); return MOD; }
"^"|"**"            { APPEND_TO_LINE(); return POW; }
"&"                 { APPEND_TO_LINE(); return AND; }
"|"                 { APPEND_TO_LINE(); return OR; }
"!"                 { APPEND_TO_LINE(); return NOT; }
">"                 { APPEND_TO_LINE(); yylval.token = GT_TK; return COMP; }
">="                { APPEND_TO_LINE(); yylval.token = GE_TK; return COMP; }
"<"                 { APPEND_TO_LINE(); yylval.token = LT_TK; return COMP; }
"<="                { APPEND_TO_LINE(); yylval.token = LE_TK; return COMP; }
"=="                { APPEND_TO_LINE(); yylval.token = EQ_TK; return COMP; }
"!="                { APPEND_TO_LINE(); yylval.token = NE_TK; return COMP; }
"=>"                { APPEND_TO_LINE(); return ARROW;}
"="                 { APPEND_TO_LINE(); return ASSIGN; }
"@"                 { APPEND_TO_LINE(); return CONCAT; }
"@@"                { APPEND_TO_LINE(); return DCONCAT; }
"("                 { APPEND_TO_LINE(); return LPAREN; }
")"                 { APPEND_TO_LINE(); return RPAREN; }
"{"                 { APPEND_TO_LINE(); return LBRACKET; }
"}"                 { APPEND_TO_LINE(); return RBRACKET; }
","                 { APPEND_TO_LINE(); return COMMA; }
":="                { APPEND_TO_LINE(); return REASSIGN; }
":"                 { APPEND_TO_LINE(); return COLON; }
"."                 { APPEND_TO_LINE(); return DOT; }
{string}            {   
                        APPEND_TO_LINE(); 
                        const char* src = yytext + 1; // skip opening quote
                        size_t len = strlen(src);
                        char* dst = malloc(len); // máx posible sin la comilla final
                        char* out = dst;
                        while (*src && *src != '"') {
                            if (*src == '\\') {
                                src++;
                                switch (*src) {
                                    case 'n': *out++ = '\n'; break;
                                    case 't': *out++ = '\t'; break;
                                    case '\\': *out++ = '\\'; break;
                                    case '"': *out++ = '"'; break;
                                    default: *out++ = *src; break; // copia tal cual si no es escape conocido
                                }
                                src++;
                            } else {
                                *out++ = *src++;
                            }
                        }
                        *out = '\0';
                        yylval.sval = dst;
                        return STRING;
                    }

%%
int yywrap() {
    return 1; // Indica que no hay más entradas
}