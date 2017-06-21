CC := arm-hismall-linux-gcc

include ./Makefile.param

all:libpb.so copy
libpb.so:playback.o utils.o
	$(CC) -O2 -Wall -fno-strict-aliasing -fPIC -shared playback.o utils.o -o libpb.so
playback.o:
	$(CC) -O2 -Wall -fno-strict-aliasing -fPIC -I../inc -I../lib $(TL_APP_INC_PATH) -c playback.c 
utils.o:
	$(CC) -O2 -Wall -fno-strict-aliasing -fPIC -I../inc -I../lib -c utils.c

copy:
	cp *.o ../obj -rf; cp *.so ../obj -rf

clean:
	rm -f *.o; rm -f *.so
