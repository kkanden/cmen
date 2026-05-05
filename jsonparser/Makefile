CC = gcc
CFLAGS = -Wall -Wextra -Isrc -Iinclude -fPIC
LDLIBS = -lm

ifdef RELEASE
  CFLAGS += -O2
else
  CFLAGS += -g
endif

BUILD_DIR = build

SRCS = $(wildcard src/*.c)
OBJS = $(patsubst src/%.c, $(BUILD_DIR)/%.o, $(SRCS))

LIB = $(BUILD_DIR)/libjson.a
SHAREDLIB = $(BUILD_DIR)/libjson.so
BIN = $(BUILD_DIR)/main

.PHONY: all lib solib clean

all: $(BIN)
lib: $(LIB)
solib: $(SHAREDLIB)

$(BIN): $(BUILD_DIR)/main.o $(LIB)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

$(SHAREDLIB): $(OBJS)
	$(CC) $(CFLAGS) -shared -o $@ $^ $(LDLIBS)

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
