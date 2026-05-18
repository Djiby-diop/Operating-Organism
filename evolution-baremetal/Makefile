CC ?= cc
CFLAGS ?= -std=c11 -Wall -Wextra -Iinclude
.RECIPEPREFIX := >

SRC := $(wildcard src/*.c)
OBJ_DIR := build
OBJ := $(patsubst src/%.c,$(OBJ_DIR)/%.o,$(SRC))

.PHONY: build test clean

build: $(OBJ)

$(OBJ_DIR)/%.o: src/%.c | $(OBJ_DIR)
>$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR):
>mkdir -p $(OBJ_DIR)

test: build
>@echo "Smoke test passed: objects built ($(words $(OBJ)))"

clean:
>rm -rf $(OBJ_DIR)