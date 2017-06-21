#CCARCH = arm-hisiv100nptl-linux-

include  ../../Makefile.param

CROSS_COMPILE = $(CCARCH)

CCILINUX_eti_CC    := $(CROSS_COMPILE)gcc
CCILINUX_eti_CPP   := $(CROSS_COMPILE)g++
CCILINUX_eti_LD    := $(CROSS_COMPILE)ld
CCILINUX_eti_AR    := $(CROSS_COMPILE)ar
CCILINUX_eti_STRIP := $(CROSS_COMPILE)strip

INC_DIR := ../include
SRC_DIR := ../source

#LIB_TARGET := librtspclient.a
LIB_TARGET := libipcamera.a
INSTALL_LIB_PATH := ../../common/lib/debug

CFLAGS += -O2 -Wall -fno-strict-aliasing
#CFLAGS += -DUSE_PIPE

INC_PATH += $(INC_DIR)
INC_PATH += ../../Common/inc/tlsdk
INC_PATH += ../../Common/inc/onvif
INC_PATH += ../../Common/inc/onvif/xml2
INC_PATH += ../../Common/inc/mod
INC_PATH += ../../Common/inc
INC_PATH += ../live/liveMedia/include
INC_PATH += ../live/groupsock/include
INC_PATH += ../live/UsageEnvironment/include
INC_PATH += ../live/BasicUsageEnvironment/include
#INC_PATH += ../ffmpeg
INC_PATH += ../../Common/inc/minirtsp

CFLAGS += $(foreach dir,$(INC_PATH),-I$(dir))

OBJS :=	$(SRC_DIR)/debug/ipcamera		\
	$(SRC_DIR)/debug/h264			\
	$(SRC_DIR)/debug/openRTSP		\
	$(SRC_DIR)/debug/klw			\
	$(SRC_DIR)/debug/xm				\
	$(SRC_DIR)/debug/IpcTest		\
	#$(SRC_DIR)/debug/I13			\
	#$(SRC_DIR)/debug/fullhan		\
	#$(SRC_DIR)/debug/anni			\
	#$(SRC_DIR)/debug/river		\
	#$(SRC_DIR)/debug/aescrypp		\
	#$(SRC_DIR)/debug/aeskeypp		\
	#$(SRC_DIR)/debug/aestab		\
	#$(SRC_DIR)/debug/CryptAES		\
	#$(SRC_DIR)/debug/miniRTSP		\

OBJS := $(foreach obj,$(OBJS),$(obj).o)

$(SRC_DIR)/debug/%.o: $(SRC_DIR)/%.c
	$(CCILINUX_eti_CC) $(CFLAGS) -c $< -o $@  
$(SRC_DIR)/debug/%.o: $(SRC_DIR)/%.cpp
	$(CCILINUX_eti_CPP) $(CFLAGS) -c $< -o $@
	
all:$(LIB_TARGET)
$(LIB_TARGET): $(OBJS)
	$(CCILINUX_eti_AR) crus $(LIB_TARGET) $(OBJS)
	cp $(LIB_TARGET) ../../Common/lib/

clean:
	rm -f $(OBJS) *.pdb *.map
	rm -f $(LIB_TARGET)

print: $(SRC_DIR)/*.c
	lpr -p $?
	touch print
