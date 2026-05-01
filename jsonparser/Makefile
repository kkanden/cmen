CC = gcc
CFLAGS = -Wall -Wextra -g -Isrc -Iinclude -lm

BUILD_DIR = build

SRCS = $(wildcard src/*.c)
OBJS = $(patsubst src/%.c, $(BUILD_DIR)/%.o, $(SRCS))

LIB = $(BUILD_DIR)/libjson.a
BIN = $(BUILD_DIR)/main

.PHONY: all lib clean

all: $(BIN)
lib: $(LIB)

$(BIN): $(BUILD_DIR)/main.o $(LIB)
	$(CC) $(CFLAGS) -o $@ $^

$(LIB): $(OBJS)
	ar rcs $@ $^

$(BUILD_DIR)/main.o: main.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: src/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean: 
	rm -f $(BUILD_DIR)/* 
