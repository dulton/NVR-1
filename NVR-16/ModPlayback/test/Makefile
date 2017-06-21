CC := arm-hismall-linux-gcc

include ./Makefile.param

test:test.o
	$(CC) -O2 -Wall -fno-strict-aliasing -fPIC -L../../common/lib/debug -lpb -L../../common/lib -lcustommp4 -lapi ../../common/lib/tl_hslib_hs3515.a $(TL_APP_LIBS) -lm -lpthread test.o -o test
	\cp test /tftpboot/
	
test.o:
	$(CC) -O2 -Wall -fno-strict-aliasing -fPIC -I../inc -I../../common/inc -I../../common/inc/tlsdk -I../../common/inc/mod  $(TL_APP_INC_PATH) -c test.c

clean:
	rm test; rm test.o
