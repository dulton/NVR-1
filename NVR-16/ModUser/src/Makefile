include  ../../Makefile.param

AR    := $(CCARCH)ar
CC    := $(CCARCH)gcc

all:libuser.so copy
libuser.so:user.o
	$(CC) $(CFLAGS) -O2 -Wall -fno-strict-aliasing -fPIC -shared user.o -o libuser.so
user.o:
	$(CC) $(CFLAGS) -O2 -Wall -fno-strict-aliasing -fPIC -I../inc -I../../Common/inc/mod -c user.c 
copy:
	cp *.o ../obj; cp *.so ../obj
	
clean:
	rm -f *.o; rm -f *.so
