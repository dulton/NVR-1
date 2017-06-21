include  ../../Makefile.param

AR    := $(CCARCH)ar
CC    := $(CCARCH)gcc

include ../../tmp.mak
ifdef MMUCHIP
	CFLAGS += -D$(MMUCHIP)
endif

all:libcirclebuf.so librecord.so copy
libcirclebuf.so:circlebuf.o
	$(CC) $(CFLAGS) -O2 -Wall -fno-strict-aliasing -fPIC -shared circlebuf.o -o libcirclebuf.so
circlebuf.o:circlebuf.c 
	$(CC) $(CFLAGS) -O2 -Wall -fno-strict-aliasing -fPIC -I../inc -I../../Common/inc -c circlebuf.c 
librecord.so:record.o
	$(CC) $(CFLAGS) -O2 -Wall -fno-strict-aliasing -fPIC -shared record.o -o librecord.so
record.o:record.c
	$(CC) $(CFLAGS) -O2 -Wall -fno-strict-aliasing -fPIC -I../inc -I../../common/inc -I../../common/inc/tlsdk -I../../common/inc/mod -c record.c
#	$(CC) $(CFLAGS) -O2 -Wall -fno-strict-aliasing -fPIC -I../inc -I../../custommp4/inc -c record.c

#install -m 777 *.o ../obj
copy:
	cp *.o ../obj -rf; cp *.so ../obj -rf

clean:
	rm -f *.o; rm -f *.so
