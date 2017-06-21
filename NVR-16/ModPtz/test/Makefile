include  ../../Makefile.param

AR    := $(CCARCH)ar
CC    := $(CCARCH)gcc

test:test.o
	$(CC) -O2 -Wall -fno-strict-aliasing -fPIC -L../obj -L../../common/lib/debug -lcirclebuf -lptz -llua -llualib -lm -lpthread test.o -o test
test.o:
	$(CC) -O2 -Wall -fno-strict-aliasing -fPIC -I../inc -I../../common/inc/mod -I../../Common/inc/tlsdk -c test.c

clean:
	rm -f test; rm -f test.o
