include ../Makefile.param

CCILINUX_eti_CC    := $(CCARCH)gcc
CCILINUX_eti_CPP   := $(CCARCH)g++
CCILINUX_eti_LD    := $(CCARCH)ld
CCILINUX_eti_AR    := $(CCARCH)ar
CCILINUX_eti_STRIP := $(CCARCH)strip

INC_DIR := ./inc ../common/inc ../common/inc/tlsdk 
INC_DIR += $(TL_APP_SDK_DIR)/pub/kbuild-FULL_REL/include

SRC_DIR := ./src
BUILD_DIR := ./
OBJ_DIR := ./obj
TEST_DIR := ./test

TARGET := libtools.a
INSTALL_PATH := $(BUILD_DIR)

INC_PATH += $(INC_DIR)

LDFLAGS := -ltools -lz

CFLAGS += -O2 -Wall -fno-strict-aliasing -DFLASHDEV

include ../tmp.mak
ifdef MMUCHIP
	CFLAGS += -D$(MMUCHIP)
endif

CFLAGS += $(foreach dir,$(INC_PATH),-I$(dir))

OBJS :=	$(BUILD_DIR)/obj/CustomGenerate				\
				$(BUILD_DIR)/obj/Upgrade				\
				$(BUILD_DIR)/obj/crc32				\

OBJS := $(foreach obj,$(OBJS),$(obj).o)

$(BUILD_DIR)/obj/%.o: $(SRC_DIR)/%.c
	$(CCILINUX_eti_CC) $(CFLAGS) -c $< -o $@  
	
$(BUILD_DIR)/obj/%.o: $(SRC_DIR)/%.cpp
	$(CCILINUX_eti_CPP) $(CFLAGS) -c $< -o $@
	
all:$(TARGET)

$(TARGET): $(OBJS)
	$(CCILINUX_eti_AR) crus $(TARGET) $(OBJS)
	install -m 0755 $(TARGET) ../Common/lib
	#\cp $(TARGET) /tftpboot
	#\cp $(TARGET) /mnt/nfs
	
$(TEST_DIR)/test.o: $(TEST_DIR)/test.c
	$(CCILINUX_eti_CC) $(CFLAGS) -c $< -o $@

test:$(TEST_DIR)/test.o
	$(CCILINUX_eti_CC) $(CFLAGS) $< -o $(TEST_DIR)/$@ -L../common/lib $(LDFLAGS)
	#$(CCILINUX_eti_STRIP) $(TEST_DIR)/$@
	\cp $(TEST_DIR)/test /tftpboot

clean:
	rm -f $(OBJS) *.pdb *.map
	rm -f $(TARGET)
	rm -rf $(TEST_DIR)/test.o
	rm -rf $(TEST_DIR)/test

print: $(SRC_DIR)/*.c
	lpr -p $?
	touch print


