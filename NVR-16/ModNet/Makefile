include ../Makefile.param

AR    := $(CCARCH)ar
CC    := $(CCARCH)gcc

SRC_DIR := ./src
OBJ_DIR := ./obj

BUILD_TARGET := libnetcomm.so

INC_LIB = ../common/lib/debug
INC_LIB_STATIC = ../common/lib
 
INC_PATH += ./inc  ../Common/inc ../Common/inc/mod ../Common/inc/tlsdk
INC_PATH += $(TL_APP_SDK_DIR)/pub/kbuild-FULL_REL/include

#CFLAGS := -O3 -Wall -fno-strict-aliasing
CFLAGS += -O2 -Wall -fno-strict-aliasing
CFLAGS += $(foreach inc,$(INC_PATH),-I$(inc))
CFLAGS += $(TL_APP_INC_PATH)
#CFLAGS += $(foreach lib,$(LIB_DIR),-L$(lib))

LDFLAGS += -lpthread
LDFLAGS += -L $(INC_LIB)/ -lpublic -lsyscomplex
LDFLAGS += -L $(INC_LIB_STATIC)/libctrlprotocol.a
LDFLAGS += -L $(INC_LIB_STATIC)/libcustommp4.a 

#LDFLAGS += -L $(INC_LIB_STATIC)/libtestMPEG4VideoStreamer.a
#LDFLAGS += -L $(INC_LIB_STATIC)/libliveMedia.a
#LDFLAGS += -L $(INC_LIB_STATIC)/libgroupsock.a
#LDFLAGS += -L $(INC_LIB_STATIC)/libBasicUsageEnvironment.a
#LDFLAGS += -L $(INC_LIB_STATIC)/libUsageEnvironment.a

include ../tmp.mak
ifdef MMUCHIP
	CFLAGS += -D$(MMUCHIP)
endif

OBJS :=$(OBJ_DIR)/netcomm						\
	   $(OBJ_DIR)/remotepreview					\
	   $(OBJ_DIR)/loopbuf						\
	   $(OBJ_DIR)/CmdProc						\
	   $(OBJ_DIR)/remoteplay					\
	   $(OBJ_DIR)/remoteDown					\
	   $(OBJ_DIR)/remoteUpdate					\
	   $(OBJ_DIR)/ddns_tl						\
	   $(OBJ_DIR)/ddns							\
	   $(OBJ_DIR)/ddns_other					\
	   $(OBJ_DIR)/DDNS_9508						\
	   $(OBJ_DIR)/mobile						\
	   $(OBJ_DIR)/mail   						\
	   $(OBJ_DIR)/sg_platform 					\
	   #$(OBJ_DIR)/ExcuteCmd  					\
	   

		
OBJS := $(foreach obj,$(OBJS),$(obj).o)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c 
	$(CC) $(CFLAGS) -fPIC -c $< -o $@

all: $(OBJS) $(BUILD_TARGET) 


$(BUILD_TARGET) : $(OBJS)
	$(CC) $(CFLAGS) -fPIC -shared $(OBJS) -o $(BUILD_TARGET) $(LDFLAGS)
	cp $(BUILD_TARGET)  /tftpboot
	cp $(BUILD_TARGET)  /mnt/nfs
	cp $(BUILD_TARGET)  ../common/lib/debug
	
copy:
	cp *.o ./obj; cp *.so ./obj
	
clean:
	rm -f *.o; rm -f *.so;rm -f $(OBJ_DIR)/*.o;

