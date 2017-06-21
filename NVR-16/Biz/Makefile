include  ../Makefile.param

AR    := $(CCARCH)ar
CC    := $(CCARCH)gcc
CPP   := $(CCARCH)g++
CCILINUX_eti_CC    := $(CCARCH)gcc
CCILINUX_eti_CPP   := $(CCARCH)g++
CCILINUX_eti_AR    := $(CCARCH)ar

LIB_TARGET := libbiz.so
INSTALL_LIB_PATH := ../common/lib/debug

INC_DIR := ../GuiSample1/Include ./inc ../Common/inc ../Common/inc/mod ../Common/inc/tlsdk ../encode/inc ../ModRecord/inc 
INC_DIR += $(TL_APP_MPP_PATH)/include
SRC_DIR := ./src
OBJ_DIR := ./obj
TEST_DIR := ./test
LIB_PATH := ./lib
LIB_PATH += ../common/lib/debug ../common/lib/

LDFLAGS += -lpthread
CFLAGS += -O2 -Wall -fno-strict-aliasing
LDFLAGS += -lbiz -lrecord -lencode -lpreview -lpb -lptz -lsyscomplex -lalarm -lconfig -luser -lcirclebuf -lpublic -lcustommp4 -lapi -llua -llualib -lpthread
LDFLAGS += -lz -lm
LDFLAGS += ../common/lib/tl_lib.a
LDFLAGS += $(TL_APP_LIBS)

#CFLAGS := $(foreach inc,$(INC_DIR),-I$(inc))
CFLAGS += $(foreach inc,$(INC_DIR),-I$(inc))
LDFLAGS += $(foreach lib,$(LIB_PATH),-L$(lib))
CFLAGS += $(TL_APP_INC_PATH)

LIB_PATH += $(TL_APP_LIB_PATH)
LDFLAGS += $(TL_APP_LIBS)

include ../tmp.mak
ifdef MYDEFINE
	CFLAGS += -D$(MYDEFINE)
endif
ifdef MMUCHIP
	CFLAGS += -D$(MMUCHIP)
endif


OBJS := $(OBJ_DIR)/biz	\
	$(OBJ_DIR)/biz_manager	\
	$(OBJ_DIR)/biz_config	\
	$(OBJ_DIR)/biz_syscomplex	\
	$(OBJ_DIR)/biz_preview	\
	$(OBJ_DIR)/biz_encode	\
	$(OBJ_DIR)/biz_record	\
	$(OBJ_DIR)/biz_playback	\
	$(OBJ_DIR)/biz_ptz		\
	$(OBJ_DIR)/biz_alarm	\
	$(OBJ_DIR)/biz_net		\
	$(OBJ_DIR)/ExcuteCmd		\
	$(OBJ_DIR)/biz_log	\
	$(OBJ_DIR)/biz_user	\
	$(OBJ_DIR)/biz_snap	\


OBJS := $(foreach obj,$(OBJS),$(obj).o)


$(OBJ_DIR)/%.o : $(SRC_DIR)/%.c
	#$(CC) $(CFLAGS) -c $< -o $@ -fPIC
	$(CC) $(CFLAGS) -fPIC -c $< -o $@
	
$(OBJ_DIR)/%.o : $(SRC_DIR)/%.cpp
	#$(CC) $(CFLAGS) -c $< -o $@ -fPIC
	$(CPP) $(CFLAGS) -fPIC -c $< -o $@

all:$(LIB_TARGET)

$(TEST_DIR)/test.o: $(TEST_DIR)/test.c
	$(CC) $(CFLAGS) -c $< -o $@

test:$(TEST_DIR)/test.o
	$(CC) $(CFLAGS) $(TEST_DIR)/test.o -o $(TEST_DIR)/test $(LDFLAGS)
	\cp test/test /tftpboot


$(LIB_TARGET): $(OBJS)
	$(CPP) $(CFLAGS) -fPIC -shared -o $@ $(OBJS)
	install -D -m 644 $@ $(INSTALL_LIB_PATH)/$@
	\cp $@ /tftpboot
	\cp $@ /mnt/nfs
	
clean :
	rm -rf $(OBJ_DIR)/*
	rm -rf $(TEST_DIR)/test.o
	rm -rf $(TEST_DIR)/test












