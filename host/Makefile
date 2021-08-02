CCOBJ=clang -c -Wall -ansi -std=c99 -Werror -pedantic -DDEBUG
CCLINK=clang  -DDEBUG
CCLINKLIBS=-lpthread

OBJS=tmp/i2c.o \
	tmp/piezoboard.o \
	tmp/sysuuid.o

all: bin/libsimplei2c.a bin/libpiezoboard.a bin/piezocli

bin/libsimplei2c.a: tmp/i2c.o

	ar -crs bin/libsimplei2c.a tmp/i2c.o

bin/libpiezoboard.a: $(OBJS)

	ar -crs bin/libpiezoboard.a $(OBJS)

bin/piezocli: bin/libpiezoboard.a src/maincli.c

	$(CCOBJ) -o tmp/maincli.o src/maincli.c
	$(CCLINK) -o bin/piezocli -L./bin/ tmp/maincli.o -lpiezoboard

tmp/i2c.o: src/i2c.c src/i2c.h

	$(CCOBJ) -o tmp/i2c.o src/i2c.c

tmp/sysuuid.o: src/sysuuid.c src/sysuuid.h

	$(CCOBJ) -o tmp/sysuuid.o src/sysuuid.c

tmp/piezoboard.o: src/sysuuid.h src/piezoboard.c src/piezoboard.h

	$(CCOBJ) -o tmp/piezoboard.o src/piezoboard.c
