include  ../../Makefile.param

AR    := $(CCARCH)ar
CC    := $(CCARCH)gcc

test:test.o
	$(CC) -O2 -Wall -fno-strict-aliasing -fPIC -L../obj -luser test.o -lpthread -o test
test.o:
	$(CC) -O2 -Wall -fno-strict-aliasing -fPIC -I../inc -c test.c

clean:
	rm test; rm test.o
