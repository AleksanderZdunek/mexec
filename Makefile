# Build mexec
# Author: Aleksander Zdunek
# Date: 2026-11-01
TARGET = mexec
OBJ = 	mexec.o parse_input.o

CC = gcc
CFLAGS = -g -std=gnu11 -Werror -Wall -Wextra -Wpedantic -Wmissing-declarations \
	-Wmissing-prototypes -Wold-style-definition

all: $(TARGET)

$(OBJ): parse_input.h debug.h
%.o: %.c Makefile
	$(CC) $(CFLAGS) -c -o $@ $<

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

.PHONY: clean
clean:
	rm -f $(OBJ) $(TARGET)
