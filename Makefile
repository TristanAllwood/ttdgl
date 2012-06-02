SRC=*.c
CC=gcc
CFLAGS=-Wall -Werror -pedantic-errors -std=c99 -g

.SUFFIXES: .c .o .h

all: tags ttdgl

ttdgl.o: ttdgl.c util.h

ttdgl: ttdgl.o util.o

clean:
	rm -rf ttdgl
	rm -f *.o

tags: $(SRC)
	ctags $^

.PHONY: all clean
