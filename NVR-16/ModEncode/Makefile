include  ../Makefile.param

AR    := $(CCARCH)ar
CC    := $(CCARCH)gcc
CCILINUX_eti_CC    := $(CCARCH)gcc
CCILINUX_eti_AR    := $(CCARCH)ar

LIB_TARGET := libencode.so
INSTALL_LIB_PATH := ../common/lib/debug

INC_DIR := ./inc  ../common/inc		../common/inc/mod  ../common/inc/tlsdk
INC_DIR += $(TL_APP_MPP_PATH)/include
INC_DIR += $(TL_APP_SDK_DIR)/pub/kbuild-FULL_REL/include

SRC_DIR := ./src
OBJ_DIR := ./obj
TEST_DIR := ./test
LIB_DIR := ./lib ./


LDFLAGS += -lpthread
CFLAGS += -O2 -Wall -fno-strict-aliasing
LDFLAGS += -lm
LDFLAGS += -lencode
LDFLAGS += ../common/lib/tl_lib.a 
LDFLAGS += $(TL_APP_LIBS)
#CFLAGS := $(foreach inc,$(INC_DIR),-I$(inc))
CFLAGS += $(foreach inc,$(INC_DIR),-I$(inc))
LDFLAGS += $(foreach lib,$(LIB_DIR),-L$(lib))
CFLAGS += $(TL_APP_INC_PATH) 

include ../tmp.mak
ifdef MMUCHIP
	CFLAGS += -D$(MMUCHIP)
endif

OBJS := $(OBJ_DIR)/encode                \

OBJS := $(foreach obj,$(OBJS),$(obj).o)


$(OBJ_DIR)/%.o : $(SRC_DIR)/%.c
	#$(CC) $(CFLAGS) -c $< -o $@ -fPIC
	$(CC) $(CFLAGS) -fPIC -c $< -o $@

all:$(LIB_TARGET)


$(TEST_DIR)/test.o: $(TEST_DIR)/test.c
	$(CC) $(CFLAGS) -c $< -o $@

test:$(TEST_DIR)/test.o
	$(CC) $(CFLAGS) $(TEST_DIR)/test.o -o $(TEST_DIR)/test $(LDFLAGS) $(TL_APP_SDK_DIR)/mpp_master/lib/libpciv.a
	\cp test/test /mnt/nfs
	

$(LIB_TARGET): $(OBJS)
	$(CC) $(CFLAGS) -fPIC -shared -o libencode.so $(OBJS)
	install -D -m 644 libencode.so $(INSTALL_LIB_PATH)/libencode.so
	\cp libencode.so /tftpboot
	\cp libencode.so /mnt/nfs
	
clean :
	rm -rf $(OBJ_DIR)/*
	rm -rf $(TEST_DIR)/test.o
	rm -rf $(TEST_DIR)/test

	
	

	
	
	


	


