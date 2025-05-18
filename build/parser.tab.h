/* A Bison parser, made by GNU Bison 3.5.1.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2020 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* Undocumented macros, especially those whose name start with YY_,
   are private implementation details.  Do not rely on them.  */

#ifndef YY_YY_BUILD_PARSER_TAB_H_INCLUDED
# define YY_YY_BUILD_PARSER_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    NUMBER = 258,
    BOOLEAN = 259,
    STRING = 260,
    ID = 261,
    COMP = 262,
    ADD = 263,
    SUB = 264,
    MUL = 265,
    DIV = 266,
    MOD = 267,
    POW = 268,
    CONCAT = 269,
    DCONCAT = 270,
    AND = 271,
    OR = 272,
    NOT = 273,
    LPAREN = 274,
    RPAREN = 275,
    LBRACKET = 276,
    RBRACKET = 277,
    COMMA = 278,
    SEMICOLON = 279,
    LET = 280,
    IN = 281,
    ASSIGN = 282,
    IF = 283,
    ELIF = 284,
    ELSE = 285,
    FUNCTION = 286,
    ARROW = 287
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 19 "src/frontend/parser.y"

    double dval;
    int ival;
    char* sval;
    TokenType token;
    struct ASTNode* node;
    
    struct {
        ASTNode** nodes;
        int count;
    } node_list;
    
    struct {
        char* name;
        ASTNode* value;
        struct SymbolTable* scope; 
    } var_decl;

    struct {
        ASTNode** conditions;
        ASTNode** branches;
        int count;
    } elif_list;

    struct {
        char* name;
        char** parameters;
        int param_count;
    } function_header;

    struct {
        char** names;
        int count;
    } param_list;

#line 126 "build/parser.tab.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_BUILD_PARSER_TAB_H_INCLUDED  */
