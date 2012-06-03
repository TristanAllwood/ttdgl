OBJS=ttdgl.o util.o ttdgl_state.o child.o render.o parent.o commands_parser.o commands_lexer.o
SRC=*.c *.h
CC=gcc
FREETYPE_INCLUDE="/usr/include/freetype2/"
CFLAGS=-Wall -Werror -pedantic -pedantic-errors -std=c99 -g $(shell sdl-config --cflags) -I$(FREETYPE_INCLUDE)
LDFLAGS=$(shell sdl-config --libs) -lGL -lftgl

YACC=/usr/bin/bison
LEX=/usr/bin/flex
LFLAGS=
YFLAGS=

.SUFFIXES: .c .o .h .l .y

all: tags ttdgl

%.c %.h: %.y
	$(YACC) $(YFLAGS) --defines=${<:.y=.h} -o $@ $< 

commands_lexer.o: commands_lexer.c commands_parser.h

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
	rm -f commands_lexer.c commands_lexer.h
	rm -f commands_parser.c commands_parser.h 

tags: $(SRC)
	ctags $^

.PHONY: all clean
