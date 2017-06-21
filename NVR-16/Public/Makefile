include  ../Makefile.param

CCILINUX_eti_CC    := $(CCARCH)gcc
CCILINUX_eti_AR    := $(CCARCH)ar

LIB_TARGET := libpublic.so
INSTALL_LIB_PATH := ../common/lib/debug

INC_DIR := ./inc ../common/inc ../common/inc/mod ../common/inc/tlsdk /opt/hisi-linux/x86-arm/gcc-3.4.3-uClibc-0.9.28/usr/include/
SRC_DIR := ./src
OBJ_DIR := ./obj
TEST_DIR := ./test

#CFLAGS := $(foreach inc,$(INC_DIR),-I$(inc))
CFLAGS += -O2 -Wall -fno-strict-aliasing
CFLAGS += $(foreach inc,$(INC_DIR),-I$(inc))

include ../tmp.mak
ifdef MMUCHIP
	CFLAGS += -D$(MMUCHIP)
endif

LDFLAGS := -lpublic -lpthread -lcustommp4 -lapi


OBJS := $(OBJ_DIR)/msgcmd	\
	$(OBJ_DIR)/public	\
	$(OBJ_DIR)/kdvsys \
	$(OBJ_DIR)/iniread \
	$(OBJ_DIR)/iniwrite \

OBJS := $(foreach obj,$(OBJS),$(obj).o)


$(OBJ_DIR)/%.o : $(SRC_DIR)/%.c
	#$(CCILINUX_eti_CC) $(CFLAGS) -c $< -o $@ -fPIC 
	$(CCILINUX_eti_CC) $(CFLAGS) -fPIC -c $< -o $@

all:$(LIB_TARGET)

$(TEST_DIR)/test.o: $(TEST_DIR)/test.c
	$(CCILINUX_eti_CC) $(CFLAGS) -c $< -o $@

test:$(TEST_DIR)/test.o
	$(CCILINUX_eti_CC) $(CFLAGS) $< -o $(TEST_DIR)/$@ -L$(INSTALL_LIB_PATH) -L../common/lib $(LDFLAGS)
	#$(CCILINUX_eti_STRIP) $(TEST_DIR)/$@
	\cp $(TEST_DIR)/test /tftpboot
	
$(LIB_TARGET): $(OBJS)
	$(CCILINUX_eti_CC) $(CFLAGS) -fPIC -shared -o $@ $(OBJS)
	install -D -m 644 $@ $(INSTALL_LIB_PATH)/$@
	\cp $@ /tftpboot
	\cp $@ /mnt/nfs

clean :
	rm -rf $(OBJ_DIR)/*
	rm -rf $(TEST_DIR)/test.o
	rm -rf $(TEST_DIR)/test

	
	

	
	
	


	


