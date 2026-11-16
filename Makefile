
CC = gcc
CFLAGS = -Wall -Wextra -Iinclude
LDLIBS = -lreadline

SRC = src/main.c src/shell.c src/execute.c src/builtins.c src/jobs.c
TARGET = bin/myshell

all: $(TARGET)

$(TARGET): $(SRC)
	mkdir -p bin
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDLIBS)

clean:
	rm -f $(TARGET)

.PHONY: all clean
