CC := arm-hismall-linux-gcc

all:test copy
test:test.o
#	$(CC) -O2 -Wall -fno-strict-aliasing -fPIC test.o -L../../common/lib/debug -lpublic -lconfig -L../../common/lib -lcustommp4 -lz -lpthread -o test
	$(CC) -O2 -Wall -fno-strict-aliasing -fPIC test.o ./obj/ini*.o ./obj/kdvsys.o -L../ -lconfig -L ./obj -lz -lcirclebuf -lpthread -o test
test.o:
	$(CC) -O2 -Wall -fno-strict-aliasing -fPIC -I../../common/inc/mod -I../../common/inc -I../inc -c test.c
copy:
	cp test /mnt/nfs -rf
clean:
	rm test; rm test.o
