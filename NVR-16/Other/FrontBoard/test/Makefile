include  ../../../Makefile.param

AR    := $(CCARCH)ar
CC    := $(CCARCH)gcc

all:test copy
test:test.o
	$(CC) -O2 -Wall -fno-strict-aliasing -fPIC test.o ../obj/ini*.o ../obj/kdvsys.o -L ../obj -L ../../../common/lib/debug -lconfig -lz -lcirclebuf -lpanel -lpthread -o test
test.o:test.c
	$(CC) -O2 -Wall -fno-strict-aliasing -fPIC -I../inc  -c test.c
copy:
	cp test /mnt/nfs -rf
clean:
	rm test; rm *.o
