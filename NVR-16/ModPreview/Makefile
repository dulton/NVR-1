include  ../Makefile.param

AR    := $(CCARCH)ar
CC    := $(CCARCH)gcc
CCILINUX_eti_CC    := $(CCARCH)gcc
CCILINUX_eti_AR    := $(CCARCH)ar

LIB_TARGET := libpreview.so
INSTALL_LIB_PATH := ../common/lib/debug

INC_DIR := ./inc  ../common/inc		../common/inc/mod  ../common/inc/tlsdk
INC_DIR += $(TL_APP_MPP_PATH)/include
SRC_DIR := ./src
OBJ_DIR := ./obj
TEST_DIR := ./test
LIB_DIR := ./lib ./


LDFLAGS += -lpublic -lcustommp4 -lapi -lpthread
CFLAGS += -O2 -Wall -fno-strict-aliasing 
LDFLAGS += -lz -lm
LDFLAGS += -lpreview
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

OBJS := $(OBJ_DIR)/mod_preview		        \

OBJS := $(foreach obj,$(OBJS),$(obj).o)


$(OBJ_DIR)/%.o : $(SRC_DIR)/%.c
	#$(CC) $(CFLAGS) -c $< -o $@ -fPIC
	$(CC) $(CFLAGS) -fPIC -c $< -o $@

all:$(LIB_TARGET)


$(TEST_DIR)/test.o: $(TEST_DIR)/test.c
	$(CC) $(CFLAGS) -c $< -o $@

test:$(TEST_DIR)/test.o
	$(CC) $(CFLAGS) $(TEST_DIR)/test.o -o $(TEST_DIR)/test -L$(INSTALL_LIB_PATH) -L../common/lib $(LDFLAGS) 
	cp test/test /tftpboot
	

$(LIB_TARGET): $(OBJS)
	$(CC) $(CFLAGS) -fPIC -shared -o $@ $(OBJS)
	install -D -m 644 $@ $(INSTALL_LIB_PATH)/$@
	\cp libpreview.so /tftpboot	
	\cp libpreview.so /mnt/nfs

clean :
	rm -rf $(OBJ_DIR)/*
	rm -rf $(TEST_DIR)/test.o
	rm -rf $(TEST_DIR)/test

	
	

	
	
	


	


