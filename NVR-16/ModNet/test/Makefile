include ./Makefile.param

CC := arm-hismall-linux-g++

SRC_DIR := .
OBJ_DIR := .

BUILD_TARGET := netcommtest


INC_COMMON := ../../common
COMMON_DIR := ../../common


INC_PATH := $(INC_COMMON)/inc $(INC_COMMON)/inc/mod $(INC_COMMON)/inc/tlsdk

INC_PATH += /root/pw/Hi3515/pub/kbuild-FULL_REL/include
INC_PATH += ../inc

LIB_DIR := $(COMMON_DIR)/lib $(COMMON_DIR)/lib/debug

CFLAGS := -O3 -Wall -fno-strict-aliasing
CFLAGS += $(foreach inc,$(INC_PATH),-I$(inc))
CFLAGS += $(foreach lib,$(LIB_DIR),-L$(lib))


LDFLAGS += -lpthread
LDFLAGS += -L ../../common/lib/debug  -lpublic -lsyscomplex -lnetcomm  
LDFLAGS += $(COMMON_DIR)/lib/tl_hslib_hs3515.a
LDFLAGS += $(COMMON_DIR)/lib/libctrlprotocol.a
LDFLAGS += $(COMMON_DIR)/lib/libcustommp4.a 
LDFLAGS += $(COMMON_DIR)/lib/libapi.a
LDFLAGS += $(COMMON_DIR)/lib/libtestMPEG4VideoStreamer.a
LDFLAGS += $(COMMON_DIR)/lib/libliveMedia.a
LDFLAGS += $(COMMON_DIR)/lib/libgroupsock.a
LDFLAGS += $(COMMON_DIR)/lib/libBasicUsageEnvironment.a
LDFLAGS += $(COMMON_DIR)/lib/libUsageEnvironment.a
LDFLAGS += $(COMMON_DIR)/lib/libMobileServerSdk.a

CFLAGS += $(TL_APP_INC_PATH)
LDFLAGS += $(TL_APP_LIBS)

OBJS :=$(OBJ_DIR)/Mod_netcomm_test						\
#	   	 $(OBJ_DIR)/ExcuteCmd  									\

		
OBJS := $(foreach obj,$(OBJS),$(obj).o)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c 
	$(CC) $(CFLAGS) -c $< -o $@

all: $(OBJS) $(BUILD_TARGET) 


$(BUILD_TARGET) : $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(BUILD_TARGET) $(LDFLAGS)
	cp $(BUILD_TARGET)  /tftpboot
		
clean:
	rm -f *.o; rm -f *.so;rm -f $(OBJ_DIR)/*.o;

