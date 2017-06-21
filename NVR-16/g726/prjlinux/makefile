#CCARCH := arm_v5t_le-
#CCARCH := powerpc-linux-
#CCARCH := arm-hismall-linux-

include  ../../Makefile.param

CCILINUX_eti_CC    := $(CCARCH)gcc
CCILINUX_eti_CPP   := $(CCARCH)g++
CCILINUX_eti_LD    := $(CCARCH)ld
CCILINUX_eti_AR    := $(CCARCH)ar
CCILINUX_eti_STRIP := $(CCARCH)strip

INC_DIR := ../include
SRC_DIR := ../source

LIB_TARGET := libg726.a
INSTALL_LIB_PATH := ../../Common/lib/

#CFLAGS += -fpack-struct

CFLAGS += -O2 -Wall -fno-strict-aliasing


INC_PATH += $(INC_DIR) ../../common/include

CFLAGS += $(foreach dir,$(INC_PATH),-I$(dir))

OBJS :=$(SRC_DIR)/debug/g72x			\
		$(SRC_DIR)/debug/g726_16			\
		$(SRC_DIR)/debug/g726_24			\
		$(SRC_DIR)/debug/g726_32			\
		$(SRC_DIR)/debug/g726_40			\
		$(SRC_DIR)/debug/g726					\
		
OBJS := $(foreach obj,$(OBJS),$(obj).o)

$(SRC_DIR)/debug/%.o: $(SRC_DIR)/%.c
	$(CCILINUX_eti_CC) $(CFLAGS) -c $< -o $@  
$(SRC_DIR)/debug/%.o: $(SRC_DIR)/%.cpp
	$(CCILINUX_eti_CPP) $(CFLAGS) -c $< -o $@
	
all:$(LIB_TARGET)
$(LIB_TARGET) : $(OBJS)
	$(CCILINUX_eti_AR) crus $(LIB_TARGET) $(OBJS)
	install -D -m 644 $(LIB_TARGET) $(INSTALL_LIB_PATH)/$(LIB_TARGET)
	
clean:
	rm -f $(OBJS) *.pdb *.map
	rm -f $(LIB_TARGET)

print: $(SRC_DIR)/*.c
	lpr -p $?
	touch print
