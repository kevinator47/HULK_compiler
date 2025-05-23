CC = gcc
FLEX = flex
BISON = bison
TARGET = hulk-compiler
BUILD_DIR = build
TEST_DIR = tests/functions
SRC_DIR = src/frontend
CFLAGS = -I$(SRC_DIR) -I$(SRC_DIR)/common

LLVM_CONFIG = llvm-config
LLVM_CFLAGS = $(shell $(LLVM_CONFIG) --cflags)
LLVM_LDFLAGS = $(shell $(LLVM_CONFIG) --ldflags --libs core)

all: build

create_dirs:
	mkdir -p $(BUILD_DIR)

build: create_dirs
	$(BISON) -d -o $(BUILD_DIR)/parser.tab.c $(SRC_DIR)/parser.y
	$(FLEX) -o $(BUILD_DIR)/lex.yy.c $(SRC_DIR)/lexer.l
	$(CC) $(CFLAGS) $(LLVM_CFLAGS) -o $(BUILD_DIR)/$(TARGET) \
		$(SRC_DIR)/main.c \
		$(SRC_DIR)/entities/ast.c \
		$(SRC_DIR)/entities/symbol_table.c \
		$(SRC_DIR)/entities/function_table.c \
		$(BUILD_DIR)/lex.yy.c \
		$(BUILD_DIR)/parser.tab.c \
		$(SRC_DIR)/codegen/ast_accept.c \
		$(SRC_DIR)/codegen/llvm/generator.c \
		$(SRC_DIR)/codegen/llvm/visitors.c \
		$(SRC_DIR)/codegen/llvm/scope_stack.c \
		$(LLVM_LDFLAGS)

run: build
	$(BUILD_DIR)/$(TARGET)

clean:
	rm -rf $(BUILD_DIR)/*

llvm: build
	$(BUILD_DIR)/$(TARGET) script.hulk
	@echo "Código LLVM generado en output.ll"
	@cat output.ll

test:
	@echo "Running all tests..."
	@find $(TEST_DIR) -name "*.hulk" -type f -exec sh -c 'echo "\nTesting {}" && $(BUILD_DIR)/$(TARGET) < {}' \;

