# Compiler and flags
CC = gcc
CFLAGS = -Wall -g

# Directories
SRC_DIR = src
BUILD_DIR = build

# Source files and object files
SRCS = $(SRC_DIR)/execute.c $(SRC_DIR)/builtins.c $(SRC_DIR)/redirect.c $(SRC_DIR)/jobs.c
OBJS = $(BUILD_DIR)/execute.o $(BUILD_DIR)/builtins.o $(BUILD_DIR)/redirect.o $(BUILD_DIR)/jobs.o

# Executable
EXEC = quash

# Default rule: build quash
all: $(EXEC)

# Rule for building quash
$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) -o $(EXEC) $(OBJS)

# Rule for compiling source files into object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Documentation generation
doc: Doxyfile
	@mkdir -p $(DOC_DIR)
	doxygen Doxyfile

# Clean up generated files
clean:
	rm -rf $(BUILD_DIR) $(EXEC)

# Usage instructions
.PHONY: all clean doc
