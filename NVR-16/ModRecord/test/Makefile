include  ../../Makefile.param

AR    := $(CCARCH)ar
CC    := $(CCARCH)gcc

test:test.o
	$(CC) -O2 -Wall -fno-strict-aliasing -fPIC -L../obj -lcirclebuf -lrecord -L../../common/lib -L../../common/lib/debug -lcustommp4 -lapi -lpthread test.o -o test
test.o:
	$(CC) -O2 -Wall -fno-strict-aliasing -fPIC -I../inc -I../../common/inc -I../../common/inc/tlsdk  -I../../common/inc/mod -c test.c

clean:
	rm test; rm test.o
