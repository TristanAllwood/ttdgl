CC=gcc
CFLAGS=-Wall -Werror -pedantic-errors -std=c99

.SUFFIXES: .c .o .h

all: ttdgl

clean:
	rm -rf ttdgl
	rm -f *.o

.PHONY: all clean
