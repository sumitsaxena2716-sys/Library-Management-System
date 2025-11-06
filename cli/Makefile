# Simple build helper for the C CLI (lms.c)
CC ?= gcc
CFLAGS ?= -std=c11 -O2 -Wall -Wextra

SRC := lms.c
BIN := lms

all: $(BIN)

$(BIN): $(SRC)
	$(CC) $(CFLAGS) $< -o $@

run: $(BIN)
	./$(BIN)

debug: CFLAGS := -std=c11 -g -O0 -Wall -Wextra
debug: clean $(BIN)

clean:
	rm -f $(BIN)

.PHONY: all run debug clean
