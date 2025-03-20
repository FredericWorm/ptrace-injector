CC = gcc
CFLAGS = --std=c11 -Wall -Wextra -g
LDFLAGS = -ldl
SOURCES = src/Main.c src/Memory.c
TARGET = InjectorBin
OUTPUT_DIR = out
TEST_OUTPUT_DIR = $(OUTPUT_DIR)/test
TEST_LIB = libtest.so
TEST_BIN = TestBin

all: $(OUTPUT_DIR)/$(TARGET) $(TEST_OUTPUT_DIR)/$(TEST_LIB) $(TEST_OUTPUT_DIR)/$(TEST_BIN)

$(OUTPUT_DIR)/$(TARGET): $(SOURCES)
	mkdir -p $(OUTPUT_DIR)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

$(TEST_OUTPUT_DIR)/$(TEST_LIB): src/Test/TestLib.c
	mkdir -p $(TEST_OUTPUT_DIR)
	$(CC) $(CFLAGS) -fPIC -shared $^ -o $@

$(TEST_OUTPUT_DIR)/$(TEST_BIN): src/Test/TestBin.c
	mkdir -p $(TEST_OUTPUT_DIR)
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -rf $(OUTPUT_DIR)