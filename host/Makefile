CCOBJ=clang -c -Wall -ansi -std=c99 -Werror -pedantic -DDEBUG
CCLINK=clang  -DDEBUG
CCLINKLIBS=-lpthread

all: bin/libsimplei2c.a

bin/libsimplei2c.a: tmp/i2c.o

	ar -crs bin/libsimplei2c.a tmp/i2c.o

tmp/i2c.o: src/i2c.c src/i2c.h

	$(CCOBJ) -o tmp/i2c.o src/i2c.c
