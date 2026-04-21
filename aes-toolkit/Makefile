# Makefile for AES Cryptanalysis Toolkit

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c99 -Isrc
LDFLAGS = -lm

# Debug flags
DEBUG_FLAGS = -g -O0 -DDEBUG
SANITIZE_FLAGS = -fsanitize=address -fsanitize=undefined

# Directories
SRC_DIR = src
BUILD_DIR = build
BIN_DIR = bin
TEST_DIR = tests
EXAMPLES_DIR = examples
BENCH_DIR = benchmarks

# Source files
CORE_SOURCES = $(wildcard $(SRC_DIR)/core/*.c)
ATTACK_SOURCES = $(wildcard $(SRC_DIR)/attacks/*.c)
MAIN_SOURCE = $(SRC_DIR)/main.c

# Object files
CORE_OBJECTS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(CORE_SOURCES))
ATTACK_OBJECTS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(ATTACK_SOURCES))
MAIN_OBJECT = $(BUILD_DIR)/main.o

# Targets
TARGET = $(BIN_DIR)/aes_attack
ALL_OBJECTS = $(CORE_OBJECTS) $(ATTACK_OBJECTS) $(MAIN_OBJECT)

# Default target
.PHONY: all
all: $(TARGET)

# Create directories
$(BUILD_DIR) $(BIN_DIR) $(BUILD_DIR)/core $(BUILD_DIR)/attacks:
	mkdir -p $@

# Compile object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR) $(BUILD_DIR)/core $(BUILD_DIR)/attacks
	$(CC) $(CFLAGS) -c $< -o $@

# Link executable
$(TARGET): $(ALL_OBJECTS) | $(BIN_DIR)
	$(CC) $(ALL_OBJECTS) $(LDFLAGS) -o $@
	@echo ""
	@echo "Build successful! Executable: $(TARGET)"
	@echo ""

# Debug build
.PHONY: debug
debug: CFLAGS += $(DEBUG_FLAGS)
debug: clean all

# Sanitizer build (for memory checking)
.PHONY: sanitize
sanitize: CFLAGS += $(DEBUG_FLAGS) $(SANITIZE_FLAGS)
sanitize: LDFLAGS += $(SANITIZE_FLAGS)
sanitize: clean all

# Run the attack with default settings
.PHONY: run
run: $(TARGET)
	@echo "Running attack with random key..."
	@$(TARGET) -r

# Run with verbose output
.PHONY: run-verbose
run-verbose: $(TARGET)
	@$(TARGET) -r -v

# Clean build artifacts
.PHONY: clean
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)
	@echo "Cleaned build artifacts"

# Clean all generated files
.PHONY: distclean
distclean: clean
	rm -rf data/attack_results/*
	@echo "Cleaned all generated files"

# Install (copy to /usr/local/bin or specified prefix)
PREFIX ?= /usr/local
.PHONY: install
install: $(TARGET)
	install -d $(PREFIX)/bin
	install -m 755 $(TARGET) $(PREFIX)/bin/
	@echo "Installed to $(PREFIX)/bin/"

# Uninstall
.PHONY: uninstall
uninstall:
	rm -f $(PREFIX)/bin/aes_attack
	@echo "Uninstalled from $(PREFIX)/bin/"

# Show variables (for debugging Makefile)
.PHONY: show
show:
	@echo "CC: $(CC)"
	@echo "CFLAGS: $(CFLAGS)"
	@echo "LDFLAGS: $(LDFLAGS)"
	@echo "CORE_SOURCES: $(CORE_SOURCES)"
	@echo "ATTACK_SOURCES: $(ATTACK_SOURCES)"
	@echo "ALL_OBJECTS: $(ALL_OBJECTS)"
	@echo "TARGET: $(TARGET)"

# Help
.PHONY: help
help:
	@echo "AES Cryptanalysis Toolkit - Makefile"
	@echo ""
	@echo "Available targets:"
	@echo "  all          - Build the project (default)"
	@echo "  debug        - Build with debug symbols"
	@echo "  sanitize     - Build with AddressSanitizer"
	@echo "  run          - Build and run with random key"
	@echo "  run-verbose  - Build and run with verbose output"
	@echo "  clean        - Remove build artifacts"
	@echo "  distclean    - Remove all generated files"
	@echo "  install      - Install to PREFIX (default: /usr/local)"
	@echo "  uninstall    - Uninstall from PREFIX"
	@echo "  show         - Show Makefile variables"
	@echo "  help         - Show this help message"
	@echo ""
	@echo "Variables:"
	@echo "  PREFIX       - Installation prefix (default: /usr/local)"
	@echo "  CC           - C compiler (default: gcc)"
	@echo ""
