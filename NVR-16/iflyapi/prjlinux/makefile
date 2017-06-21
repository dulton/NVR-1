#CROSS_COMPILE = arm-hismall-linux-
#CROSS_COMPILE = arm-hisiv100nptl-linux-

include  ../../Makefile.param

CROSS_COMPILE = $(CCARCH)

CCILINUX_eti_CC    := $(CROSS_COMPILE)gcc
CCILINUX_eti_CPP   := $(CROSS_COMPILE)g++
CCILINUX_eti_LD    := $(CROSS_COMPILE)ld
CCILINUX_eti_AR    := $(CROSS_COMPILE)ar
CCILINUX_eti_STRIP := $(CROSS_COMPILE)strip

INC_DIR := ../include
SRC_DIR := ../source

LIB_TARGET := libapi.a
INSTALL_LIB_PATH := ../../common/lib/debug

#CFLAGS += -fpack-struct

CFLAGS += -O2 -Wall -fno-strict-aliasing
CFLAGS += -DUSE_PIPE

INC_PATH += $(INC_DIR) ../../Common/inc $(INC_DIR)/openssl-0.9.8y/inc32

CFLAGS += $(foreach dir,$(INC_PATH),-I$(dir))

OBJS :=	$(SRC_DIR)/debug/common				\
		$(SRC_DIR)/debug/urat			\
		$(SRC_DIR)/debug/rtc			\
		$(SRC_DIR)/debug/net			\
		$(SRC_DIR)/debug/bbfunc			\
		$(SRC_DIR)/debug/ftp			\
		$(SRC_DIR)/debug/mount			\
		$(SRC_DIR)/debug/umount			\
		$(SRC_DIR)/debug/fdisk			\
		$(SRC_DIR)/debug/mkfsdos            	\
		$(SRC_DIR)/debug/wget           	\
		$(SRC_DIR)/debug/CSmtp          	\
		$(SRC_DIR)/debug/base64           	\
		$(SRC_DIR)/debug/md5          		\
		$(SRC_DIR)/debug/smtp          		\

OBJS := $(foreach obj,$(OBJS),$(obj).o)

$(SRC_DIR)/debug/%.o: $(SRC_DIR)/%.c
	$(CCILINUX_eti_CC) $(CFLAGS) -c $< -o $@  
$(SRC_DIR)/debug/%.o: $(SRC_DIR)/%.cpp
	$(CCILINUX_eti_CPP) $(CFLAGS) -c $< -o $@
	
all:$(LIB_TARGET)
$(LIB_TARGET): $(OBJS)
	$(CCILINUX_eti_AR) crus $(LIB_TARGET) $(OBJS)
#	install -m 0755 $(LIB_TARGET) $(INSTALL_LIB_PATH)
#	install -m 0755 $(LIB_TARGET) /root/src/9108/common/lib/debug
	\cp $(LIB_TARGET) ../../Common/lib/.
#	cp $(LIB_TARGET) /home/chen/prj/prj1-hs3511/app/common/lib/debug -f

clean:
	rm -f $(OBJS) *.pdb *.map
	rm -f $(LIB_TARGET)

print: $(SRC_DIR)/*.c
	lpr -p $?
	touch print
