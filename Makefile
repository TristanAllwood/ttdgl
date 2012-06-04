OBJS=ttdgl.o util.o ttdgl_state.o child.o render.o parent.o
SRC=*.c *.h
CC=gcc
FREETYPE_INCLUDE="/usr/include/freetype2/"
CFLAGS=-Wall -Werror -pedantic -pedantic-errors -std=c99 -g $(shell sdl-config --cflags) -I$(FREETYPE_INCLUDE)
LDFLAGS=$(shell sdl-config --libs) -lGL -lftgl

.SUFFIXES: .c .o .h

all: tags ttdgl

child.o: child.c child.h util.h

parent.o: parent.c parent.h util.h ttdgl_state.h render.h

render.o: render.c render.h ttdgl_state.h

ttdgl_state.o: ttdgl_state.c ttdgl_state.h util.h

ttdgl.o: ttdgl.c util.h ttdgl_state.h child.h parent.h

ttdgl: $(OBJS)

clean:
	rm -rf ttdgl
	rm -f *.o
	rm -f tags

tags: $(SRC)
	ctags $^

.PHONY: all clean
