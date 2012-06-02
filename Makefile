SRC=*.c
CC=gcc
CFLAGS=-Wall -Werror -pedantic-errors -std=c99

.SUFFIXES: .c .o .h

all: tags ttdgl

clean:
	rm -rf ttdgl
	rm -f *.o

tags: $(SRC)
	ctags $^

.PHONY: all clean
