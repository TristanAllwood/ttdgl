SRC=*.c *.h
CC=gcc
CFLAGS=-Wall -Werror -pedantic-errors -std=c99 -g $(shell sdl-config --cflags)
LDFLAGS=$(shell sdl-config --libs) -lGL

.SUFFIXES: .c .o .h

all: tags ttdgl

child.o: child.c child.h util.h

parent.o: parent.c parent.h util.h ttdgl_state.h render.h

render.o: render.c render.h ttdgl_state.h

ttdgl_state.o: ttdgl_state.c ttdgl_state.h util.h

ttdgl.o: ttdgl.c util.h ttdgl_state.h child.h parent.h

ttdgl: ttdgl.o util.o ttdgl_state.o child.o render.o parent.o

clean:
	rm -rf ttdgl
	rm -f *.o
	rm -f tags

tags: $(SRC)
	ctags $^

.PHONY: all clean
