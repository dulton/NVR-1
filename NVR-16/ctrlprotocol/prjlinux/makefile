#CCARCH := powerpc-linux-
#CCARCH := arm-hismall-linux-
#CCARCH := arm-hisiv100nptl-linux-

include  ../../Makefile.param

CCILINUX_eti_CC    := $(CCARCH)gcc
CCILINUX_eti_CPP   := $(CCARCH)g++
CCILINUX_eti_LD    := $(CCARCH)ld
CCILINUX_eti_AR    := $(CCARCH)ar
CCILINUX_eti_STRIP := $(CCARCH)strip

INC_DIR := ../include ../../Common/inc ../../Common/inc/mod
SRC_DIR := ../source

LIB_TARGET := libctrlprotocol.a
INSTALL_LIB_PATH := ../../common/lib/debug

#CFLAGS += -fpack-struct

CFLAGS += -O2 -Wall -fno-strict-aliasing

INC_PATH += $(INC_DIR) ../../common/include

CFLAGS += $(foreach dir,$(INC_PATH),-I$(dir))

OBJS :=$(SRC_DIR)/debug/ctrlprotocol			\
		$(SRC_DIR)/debug/vsip					\

OBJS := $(foreach obj,$(OBJS),$(obj).o)

$(SRC_DIR)/debug/%.o: $(SRC_DIR)/%.c
	$(CCILINUX_eti_CC) $(CFLAGS) -c $< -o $@  
$(SRC_DIR)/debug/%.o: $(SRC_DIR)/%.cpp
	$(CCILINUX_eti_CPP) $(CFLAGS) -c $< -o $@
	
all:$(LIB_TARGET)
$(LIB_TARGET) : $(OBJS)
	$(CCILINUX_eti_AR) crus $(LIB_TARGET) $(OBJS)
#	install -D -m 644 $(LIB_TARGET) $(INSTALL_LIB_PATH)/$(LIB_TARGET)
#	install -D -m 644 $(LIB_TARGET) /root/src/9108/common/lib/debug/$(LIB_TARGET)
	\cp $(LIB_TARGET) ../../Common/lib/.
#	cp $(LIB_TARGET) /home/chen/prj/prj1-hs3511/app/common/lib/debug -f

clean:
	rm -f $(OBJS) *.pdb *.map
	rm -f $(LIB_TARGET)

print: $(SRC_DIR)/*.c
	lpr -p $?
	touch print
