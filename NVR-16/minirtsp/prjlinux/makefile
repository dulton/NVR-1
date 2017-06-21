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

LIB_TARGET := libminirtsp.a
INSTALL_LIB_PATH := ../../Common/lib/debug

CFLAGS += -O2 -Wall -fno-strict-aliasing
#CFLAGS += -DUSE_PIPE

INC_PATH += $(INC_DIR)
INC_PATH += ../../Common/inc/minirtsp
INC_PATH += ../../Common/inc/tlsdk
INC_PATH += ../../Common/inc/mod
INC_PATH += ../../Common/inc

CFLAGS += $(foreach dir,$(INC_PATH),-I$(dir))

OBJS :=	$(SRC_DIR)/debug/_base64		\
	$(SRC_DIR)/debug/authentication		\
	$(SRC_DIR)/debug/md5							\
	$(SRC_DIR)/debug/netstream				\
	$(SRC_DIR)/debug/portmanage				\
	$(SRC_DIR)/debug/rtcplib					\
	$(SRC_DIR)/debug/rtpbuf						\
	$(SRC_DIR)/debug/rtplib						\
	$(SRC_DIR)/debug/rtplink					\
	$(SRC_DIR)/debug/rtsplib					\
	$(SRC_DIR)/debug/rtspsession			\
	$(SRC_DIR)/debug/sdplib						\
	$(SRC_DIR)/debug/sock							\
	$(SRC_DIR)/debug/vlog							\

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
