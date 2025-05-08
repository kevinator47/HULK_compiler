CC = gcc
FLEX = flex
BISON = bison
TARGET = hulk-compiler
BUILD_DIR = build
TEST_DIR = tests
SRC_DIR = src/frontend
CFLAGS = -I$(SRC_DIR)

all: build

create_dirs:
	mkdir -p $(BUILD_DIR)

build: create_dirs
	$(BISON) -d -o $(BUILD_DIR)/parser.tab.c $(SRC_DIR)/parser.y
	$(FLEX) -o $(BUILD_DIR)/lex.yy.c $(SRC_DIR)/lexer.l
	$(CC) $(CFLAGS) -o $(BUILD_DIR)/$(TARGET) \
		$(SRC_DIR)/entities/ast.c \
		$(BUILD_DIR)/lex.yy.c \
		$(BUILD_DIR)/parser.tab.c

run: build
	$(BUILD_DIR)/$(TARGET)

clean:
	rm -rf $(BUILD_DIR)/*

test:
	@echo "Running all tests..."
	@find $(TEST_DIR) -name "*.hulk" -type f -exec sh -c 'echo "\nTesting {}" && $(BUILD_DIR)/$(TARGET) < {}' \;

