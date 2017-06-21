include  ../../Makefile.param

AR	=	/usr/bin/ar
#CC	=	gcc
CC 	= 	arm-hismall-linux-gcc

BUILD_TARGET = modsystest

HS_LIB_PATH := /root/pw/
INC_PATH := ../inc  ../../common/inc
#INC_PATH += $(TL_APP_MPP_PATH)/include
INC_PATH += $(TL_APP_SDK_DIR)/pub/kbuild-FULL_REL/include
SRC_DIR := ./src
OBJ_DIR := ../obj
TEST_DIR := ./test
LIB_DIR := ./lib
LIB_DIR += ../../common/lib

#LDFLAGS += ../common/lib/tl_lib.a 
#LDFLAGS := $(foreach lib,$(LIB_DIR),-L$(lib))
#LDFLAGS += $(TL_APP_LIBS)
LDFLAGS += -lpthread
CFLAGS += -O2 -Wall -fno-strict-aliasing
#LDFLAGS += -lz -lm
LDFLAGS += ../../common/lib/libcustommp4.a 
LDFLAGS += ../../common/lib/libapi.a 
#LDFLAGS += -shared

CFLAGS += $(foreach inc,$(INC_PATH),-I$(inc))
CFLAGS += $(foreach lib,$(LIB_DIR),-L$(lib))
CFLAGS += -L ../../common/lib -lsyscomplex 
CFLAGS += -L ../../common/lib -lpublic 
#CFLAGS += -fPIC

#OBJS := $(OBJ_DIR)/mod_syscomplex                \
#		$(OBJ_DIR)/update                        \


OBJS := $(foreach obj,$(OBJS),$(obj).o)


$(OBJ_DIR)/%.o : $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@  

#arm-hismall-linux-gcc -o test test.c -L.-I.../inc -lalarm

#$(CCILINUX_eti_CC) -fPIC DDNS.cpp -shared -O3 -o ${BUILD_TARGET}

#all:
#	$(CC) $(CFLAGS) $(SRC_DIR)/mod_syscomplex.c $(SRC_DIR)/update.c -o ${BUILD_TARGET} $(LDFLAGS)
#	\cp ${BUILD_TARGET} ../common/lib 

all:
	$(CC) $(CFLAGS) mod_syscomplex_test.c -o modsystest $(LDFLAGS)  
	\cp ${BUILD_TARGET}  /mnt/nfs
	\cp ${BUILD_TARGET}  /tftpboot

#modsystest : $(OBJS) $(TEST_DIR)/mod_syscomplex_test.c
#	$(CC) $(CFLAGS) $(TEST_DIR)/mod_syscomplex_test.c $(OBJS) -o $(TEST_DIR)/modsystest $(LDFLAGS)
	
clean :
	rm -rf modsystest

	
	

	
	
	


	