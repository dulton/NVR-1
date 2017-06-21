include  ../../../Makefile.param

include ../../../tmp.mak
ifdef MMUCHIP
	CFLAGS += -D$(MMUCHIP)
endif

AR    := $(CCARCH)ar
CC    := $(CCARCH)gcc

all:libpanel.so copy
libpanel.so:panel.o
	$(CC) $(CFLAGS) -O2 -Wall -fno-strict-aliasing -fPIC -shared panel.o -o libpanel.so
panel.o:Frontboard.c
	$(CC) $(CFLAGS) -O2 -Wall -fno-strict-aliasing -fPIC -I../inc -I../../../Common/inc -c Frontboard.c -o panel.o
copy:
	cp libpanel.so /mnt/nfs -rf
	cp libpanel.so ../obj -rf
	cp libpanel.so ../../../Common/lib/debug -rf
clean:
	rm *.o; rm *.so
