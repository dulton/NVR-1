include  ../../Makefile.param

AR    := $(CCARCH)ar
CC    := $(CCARCH)gcc

all:libptz.so copy
libptz.so:ptz.o urat.o
	$(CC) $(CFLAGS) -O2 -Wall -fno-strict-aliasing -fPIC -shared ptz.o urat.o -o libptz.so
ptz.o:
	$(CC) $(CFLAGS) -O2 -Wall -fno-strict-aliasing -fPIC -I../inc -I../../Common/inc/mod -I../../Common/inc/tlsdk -c ptz.c
urat.o:
	$(CC) $(CFLAGS) -O2 -Wall -fno-strict-aliasing -fPIC -c urat.c
copy:
	cp *.o ../obj; cp *.so ../obj
	
clean:
	rm -f *.o; rm -f *.so
