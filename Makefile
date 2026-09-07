TARGET = mexec
OBJ = 	mexec.o

CC = gcc
CFLAGS = -std=gnu11 -g -O0 -Werror -Wall -Wextra -Wpedantic -Wmissing-declarations \
	-Wmissing-declarations -Wmissing-prototypes -Wold-style-definition

all: $(TARGET)

-include *.d
%.o: %.c Makefile
	$(CC) -MMD $(CFLAGS) -c -o $@ $<

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

.PHONY: clean
clean:
	rm -f $(OBJ) $(TARGET) $(OBJ:.o=.d)
