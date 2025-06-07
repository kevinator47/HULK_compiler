# Compilador y flags
CC = gcc
LLVM_CFLAGS := $(shell llvm-config --cflags)
LLVM_LDFLAGS := $(shell llvm-config --ldflags)
LLVM_LIBS := $(shell llvm-config --libs core)

CFLAGS = -Wall -Wextra -g \
	-I./src/frontend \
	-I./src/backend \
	-I./src/frontend/common \
	-I./src/frontend/ast \
	-I./src/frontend/hulk_type \
	-I./src/frontend/scope \
	-I./src/frontend/semantic_check \
	-I./src/frontend/parser \
	-I./src/backend/codegen \
	-I./src/backend/codegen/llvm \
	$(LLVM_CFLAGS)

# Archivos fuente
SRC = $(wildcard src/frontend/*.c) \
      $(wildcard src/frontend/ast/*.c) \
      $(wildcard src/frontend/hulk_type/*.c) \
      $(wildcard src/frontend/scope/*.c) \
      $(wildcard src/frontend/semantic_check/*.c) \
      $(wildcard src/backend/codegen/*.c) \
      $(wildcard src/backend/codegen/llvm/*.c)

# Ejecutable
BIN = build/hulk_compiler

# Flex y Bison
LEX = flex
YACC = bison

LEX_SRC = src/frontend/lexer/lexer.l
YACC_SRC = src/frontend/parser/parser.y

LEX_C = build/lexer.yy.c
YACC_C = build/parser.tab.c
YACC_H = build/parser.tab.h

LEX_O = build/lexer.yy.o
YACC_O = build/parser.tab.o

# Objetos compilados
OBJ = $(patsubst src/%.c, build/%.o, $(SRC)) $(LEX_O) $(YACC_O)

# Regla por defecto
all: $(BIN)

# Crear ejecutable
$(BIN): $(OBJ)
	@mkdir -p build
	$(CC) $(OBJ) -o $@ $(LLVM_LDFLAGS) $(LLVM_LIBS)

# Compilar .c a .o
build/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Generar parser y header
$(YACC_C) $(YACC_H): $(YACC_SRC)
	@mkdir -p build
	$(YACC) -d -o $(YACC_C) $<

# Generar lexer
$(LEX_C): $(LEX_SRC) $(YACC_H)
	@mkdir -p build
	$(LEX) -o $@ $<

# Compilar lexer
$(LEX_O): $(LEX_C)
	$(CC) $(CFLAGS) -c $< -o $@

# Compilar parser
$(YACC_O): $(YACC_C)
	$(CC) $(CFLAGS) -c $< -o $@

# Ejecutar
run: $(BIN)
	./$(BIN)

# Limpiar
clean:
	rm -rf build

