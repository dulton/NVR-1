include  ../Makefile.param

CCILINUX_eti_CC    := $(CCARCH)gcc
CCILINUX_eti_AR    := $(CCARCH)ar

LIB_TARGET := libconfig.so
INSTALL_LIB_PATH := ../common/lib/debug

INC_DIR := ./inc ../common/inc ../common/inc/mod ../common/inc/tlsdk
SRC_DIR := ./src
OBJ_DIR := ./obj
TEST_DIR := ./test

CFLAGS += -O2 -Wall -fno-strict-aliasing

#CFLAGS := $(foreach inc,$(INC_DIR),-I$(inc))
CFLAGS += $(foreach inc,$(INC_DIR),-I$(inc))
CFLAGS += -O2 -Wall -fno-strict-aliasing
CFLAGS += $(TL_APP_INC_PATH) 

include ../tmp.mak
ifdef MMUCHIP
	CFLAGS += -D$(MMUCHIP)
endif

LDFLAGS := -lconfig -lpublic -lpthread -lm -lz -lcustommp4 -lapi -lcirclebuf
LDFLAGS += ../common/lib/tl_lib.a ../common/lib/tools.a $(TL_APP_LIBS)


OBJS := $(OBJ_DIR)/mod_config

OBJS := $(foreach obj,$(OBJS),$(obj).o)


$(OBJ_DIR)/%.o : $(SRC_DIR)/%.c
	#$(CCILINUX_eti_CC) $(CFLAGS) -c $< -o $@ -fPIC 
	$(CCILINUX_eti_CC) $(CFLAGS) -fPIC -c $< -o $@

all:$(LIB_TARGET)

$(TEST_DIR)/test.o: $(TEST_DIR)/test.c
	$(CCILINUX_eti_CC) $(CFLAGS) -c $< -o $@

test:$(TEST_DIR)/test.o
	$(CCILINUX_eti_CC) $(CFLAGS) $< -o $(TEST_DIR)/$@ -L$(INSTALL_LIB_PATH) -L$(TL_APP_LIB_PATH) -L../common/lib/ $(LDFLAGS)
	#$(CCILINUX_eti_STRIP) $(TEST_DIR)/$@
	\cp $(TEST_DIR)/test /tftpboot
	\cp $(TEST_DIR)/test /mnt/nfs
	
$(LIB_TARGET): $(OBJS)
	$(CCILINUX_eti_CC) $(CFLAGS) -fPIC -shared -o $@ $<
	install -D -m 644 $@ $(INSTALL_LIB_PATH)/$@
	\cp $@ /tftpboot
	\cp $@ /mnt/nfs
	
clean :
	rm -rf $(OBJ_DIR)/*
	rm -rf $(TEST_DIR)/test.o
	rm -rf $(TEST_DIR)/test
	
