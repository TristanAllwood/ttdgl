SRC=*.c
CC=gcc
CFLAGS=-Wall -Werror -pedantic-errors -std=c99 -g $(shell sdl-config --cflags)
LDFLAGS=$(shell sdl-config --libs) -lGL

.SUFFIXES: .c .o .h

all: tags ttdgl

ttdgl.o: ttdgl.c util.h

ttdgl: ttdgl.o util.o

clean:
	rm -rf ttdgl
	rm -f *.o
	rm -f tags

tags: $(SRC)
	ctags $^

.PHONY: all clean
