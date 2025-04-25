CC = gcc
FLEX = flex
BISON = bison
TARGET = hulk-compiler
BUILD_DIR = build
SRC_DIR = src/frontend
INCLUDE_DIR = include

all: create_dirs $(TARGET)

create_dirs:
	mkdir -p $(BUILD_DIR)

$(TARGET):
	$(BISON) -d -o $(BUILD_DIR)/parser.tab.c $(SRC_DIR)/parser.y
	$(FLEX) -o $(BUILD_DIR)/lex.yy.c $(SRC_DIR)/lexer.l
	$(CC) -o $(BUILD_DIR)/$(TARGET) \
		$(SRC_DIR)/ast.c \
		$(BUILD_DIR)/lex.yy.c \
		$(BUILD_DIR)/parser.tab.c \
		-I$(INCLUDE_DIR) -lm

clean:
	rm -rf $(BUILD_DIR)/*

test:
	$(BUILD_DIR)/$(TARGET) < tests/arithmetic/test1.hulk