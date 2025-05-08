/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
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
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_YY_BUILD_PARSER_TAB_H_INCLUDED
# define YY_YY_BUILD_PARSER_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    NUMBER = 258,                  /* NUMBER  */
    BOOLEAN = 259,                 /* BOOLEAN  */
    STRING = 260,                  /* STRING  */
    ID = 261,                      /* ID  */
    COMP = 262,                    /* COMP  */
    ADD = 263,                     /* ADD  */
    SUB = 264,                     /* SUB  */
    MUL = 265,                     /* MUL  */
    DIV = 266,                     /* DIV  */
    MOD = 267,                     /* MOD  */
    POW = 268,                     /* POW  */
    CONCAT = 269,                  /* CONCAT  */
    DCONCAT = 270,                 /* DCONCAT  */
    AND = 271,                     /* AND  */
    OR = 272,                      /* OR  */
    NOT = 273,                     /* NOT  */
    LPAREN = 274,                  /* LPAREN  */
    RPAREN = 275,                  /* RPAREN  */
    LBRACKET = 276,                /* LBRACKET  */
    RBRACKET = 277,                /* RBRACKET  */
    COMMA = 278,                   /* COMMA  */
    SEMICOLON = 279,               /* SEMICOLON  */
    LET = 280,                     /* LET  */
    IN = 281,                      /* IN  */
    ASSIGN = 282,                  /* ASSIGN  */
    IF = 283,                      /* IF  */
    ELIF = 284,                    /* ELIF  */
    ELSE = 285,                    /* ELSE  */
    FUNCTION = 286,                /* FUNCTION  */
    ARROW = 287                    /* ARROW  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 17 "src/frontend/parser.y"

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

#line 132 "build/parser.tab.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;


int yyparse (void);


#endif /* !YY_YY_BUILD_PARSER_TAB_H_INCLUDED  */
