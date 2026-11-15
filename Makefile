
CC = gcc
CFLAGS = -Wall -Wextra -Iinclude

SRC = src/main.c src/shell.c src/execute.c src/builtins.c
TARGET = bin/myshell

all: $(TARGET)

$(TARGET): $(SRC)
	mkdir -p bin
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all clean
