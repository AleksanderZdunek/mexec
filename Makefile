#(C) Aleksander Zdunek <redacted>@cs.umu.se
TARGET = mexec
OBJ = 	mexec.o

CC = gcc
CFLAGS = -g -std=gnu11 -Werror -Wall -Wextra -Wpedantic -Wmissing-declarations \
	-Wmissing-prototypes -Wold-style-definition

all: $(TARGET)

%.o: %.c Makefile
	$(CC) $(CFLAGS)   -c -o $@ $<

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

.PHONY: clean
clean:
	rm -f $(OBJ) $(TARGET)
