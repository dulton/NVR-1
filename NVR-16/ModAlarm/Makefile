include  ../Makefile.param

CCILINUX_eti_CC    := $(CCARCH)gcc
CCILINUX_eti_AR    := $(CCARCH)ar

LIB_TARGET := libalarm.so
INSTALL_LIB_PATH := ../common/lib/debug

INC_DIR := ./inc ../Common/inc ../Common/inc/mod ../Common/inc/tlsdk
SRC_DIR := ./src
OBJ_DIR := ./obj
TEST_DIR := ./test

#CFLAGS := $(foreach inc,$(INC_DIR),-I$(inc))
CFLAGS += $(foreach inc,$(INC_DIR),-I$(inc))
CFLAGS += -O2 -Wall -fno-strict-aliasing
CFLAGS += $(TL_APP_INC_PATH) 

LDFLAGS := -lalarm -lpublic -lcustommp4 -lapi -lpthread -lm
LDFLAGS += ../common/lib/tl_lib.a $(TL_APP_LIBS)

include ../tmp.mak
ifdef MMUCHIP
        CFLAGS += -D$(MMUCHIP)
endif

OBJS := $(OBJ_DIR)/mod_alarm

OBJS := $(foreach obj,$(OBJS),$(obj).o)


$(OBJ_DIR)/%.o : $(SRC_DIR)/%.c
	$(CCILINUX_eti_CC) $(CFLAGS) -fPIC -c $< -o $@

all:$(LIB_TARGET)

$(TEST_DIR)/test.o: $(TEST_DIR)/test.c
	$(CCILINUX_eti_CC) $(CFLAGS) -c $< -o $@

test:$(TEST_DIR)/test.o
	$(CCILINUX_eti_CC) $(CFLAGS) $< -o $(TEST_DIR)/$@ -L$(INSTALL_LIB_PATH) -L../common/lib -L$(TL_APP_LIB_PATH) $(LDFLAGS)
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

	
	

	
	
	


	


