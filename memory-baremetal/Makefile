CC ?= cc
CFLAGS ?= -std=c11 -Wall -Wextra -Iinclude
.RECIPEPREFIX := >

SRC := $(wildcard src/*.c)
OBJ_DIR := build
OBJ := $(patsubst src/%.c,$(OBJ_DIR)/%.o,$(SRC))

TEST_SRC := tests/test_hedged_malloc.c src/hedged_malloc.c src/bio_alloc.c
TEST_OBJ := $(patsubst %.c,$(OBJ_DIR)/%.o,$(TEST_SRC))
TEST_BIN := $(OBJ_DIR)/test_hedged_malloc

.PHONY: build test clean test_run

build: $(OBJ)

$(OBJ_DIR)/%.o: src/%.c | $(OBJ_DIR)
>$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/tests/%.o: tests/%.c | $(OBJ_DIR)/tests
>$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/tests:
>mkdir -p $(OBJ_DIR)/tests

$(OBJ_DIR):
>mkdir -p $(OBJ_DIR)

test: build $(TEST_BIN)

$(TEST_BIN): $(TEST_OBJ) | $(OBJ_DIR)
>$(CC) $(CFLAGS) $^ -o $@

test_run: $(TEST_BIN)
>$(TEST_BIN)

clean:
>rm -rf $(OBJ_DIR)