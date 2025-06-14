# Compilador y flags
CC = gcc
LLVM_CFLAGS := $(shell llvm-config --cflags)
LLVM_LDFLAGS := $(shell llvm-config --ldflags)
LLVM_LIBS := $(shell llvm-config --libs core)

INCLUDE_DIRS := $(shell find src -type d)
CFLAGS = -Wall -Wextra -g $(addprefix -I, $(INCLUDE_DIRS)) $(LLVM_CFLAGS)

# Archivos fuente
SRC = src/main.c \
	  $(wildcard src/frontend/*.c) \
      $(wildcard src/frontend/ast/*.c) \
      $(wildcard src/frontend/hulk_type/*.c) \
      $(wildcard src/frontend/scope/*.c) \
      $(wildcard src/frontend/semantic_check/*.c) \
      $(wildcard src/backend/codegen/*.c) \
      $(wildcard src/backend/codegen/llvm/*.c) \

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

# Compilar main
build/main.o: src/main.c build/parser.tab.h
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Ejecutar
run: $(BIN)
	./$(BIN)

# Limpiar
clean:
	rm -rf build

llvm: $(BIN)
	@echo "Generando código LLVM para script.hulk..."
	@./$(BIN) script.hulk
	@echo "=== Código LLVM generado en output.ll ==="
	@cat output.ll

llvm_test: $(BIN)
	@echo "=== Ejecutando tests LLVM en la carpeta test ==="
	@find test -name '*.hulk' | while read testfile; do \
	    testdir=$$(dirname $$testfile); \
	    testbase=$$(basename $$testfile .hulk); \
	    echo "Compilando $$testfile ..."; \
	    ./$(BIN) $$testfile > /dev/null 2>&1; \
	    if [ -f output.ll ]; then \
	        mv output.ll "$$testdir/$$testbase.ll"; \
	        echo "  [OK] Generado $$testdir/$$testbase.ll"; \
	    else \
	        echo "  [FAIL] No se generó output.ll para $$testfile"; \
	        exit 1; \
	    fi \
	done
	@echo "=== Fin de tests LLVM ==="

clean_tests:
	@echo "Eliminando archivos .ll generados en los tests..."
	@find test -name '*.ll' -delete
	@echo "Limpieza de tests completada."

# Compila el script.hulk y genera el directorio hulk/ con output.ll
compile: $(BIN)
	@echo "Compilando script.hulk y generando artifacts en ./hulk ..."
	@mkdir -p hulk
	@./$(BIN) script.hulk
	@if [ -f output.ll ]; then \
	    mv output.ll hulk/output.ll; \
	    echo "  [OK] Artifact generado: hulk/output.ll"; \
	else \
	    echo "  [FAIL] No se generó output.ll"; \
	    exit 1; \
	fi	
# E	ecuta el código LLVM generado (requiere clang y un sistema Linux)
execute: compile
	@echo "Compilando a binario nativo y ejecutando..."
	@clang hulk/output.ll -o hulk/hulk_exe -lm -lc
	@./hulk/hulk_exe	
.PHONY: compile execute