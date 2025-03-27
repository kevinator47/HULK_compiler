# Compiler and flags
CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Iinclude

# Directories
BUILD_DIR := build
SRC_DIR := src
INCLUDE_DIR := include

# Files
SRCS := $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRCS))
TARGET := $(BUILD_DIR)/hulkc
INPUT_SCRIPT := script.hulk

.PHONY: all build run clean mkdir

all: build

# Rule to create build directory
mkdir:
	@mkdir -p $(BUILD_DIR)

# Compile .cpp files to .o
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | mkdir
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Link all .o files to create executable
$(TARGET): $(OBJS) | mkdir
	$(CXX) $(CXXFLAGS) $^ -o $@

# Main build rule
build: $(TARGET)
	@echo "✅ Build completado"
	@if [ -f "$(INPUT_SCRIPT)" ]; then \
		echo "📜 Ejecutando compilador con $(INPUT_SCRIPT)"; \
		./$(TARGET) "$(INPUT_SCRIPT)"; \
	else \
		echo "❌ Error: No se encontró $(INPUT_SCRIPT)"; \
		exit 1; \
	fi
run: build
	@echo "🚧 Run (pendiente de implementación)"

clean:
	rm -rf $(BUILD_DIR)
	@echo "🧹 Directorio $(BUILD_DIR) eliminado"
