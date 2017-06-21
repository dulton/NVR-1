#CCARCH := arm-hismall-linux-
#CCARCH := arm-hisiv100nptl-linux-

include  ../../Makefile.param

CCILINUX_eti_CC    := $(CCARCH)gcc
CCILINUX_eti_CPP   := $(CCARCH)g++
CCILINUX_eti_LD    := $(CCARCH)ld
CCILINUX_eti_AR    := $(CCARCH)ar
CCILINUX_eti_STRIP := $(CCARCH)strip

INC_DIR := ../include
SRC_DIR := ../source
V2DOT0_LIB_DIR := ../../Common/lib

LIB_TARGET := libMobileServerSdk.a
APP_TEST := test_app

#CFLAGS += -fpack-struct

CFLAGS += -O2 -Wall -fno-strict-aliasing

INC_PATH +=  $(INC_DIR)

CFLAGS += $(foreach dir,$(INC_PATH),-I$(dir))

OBJS :=	$(SRC_DIR)/debug/MobileServerSdk				\
		$(SRC_DIR)/debug/Huashi_OWSP

OBJS := $(foreach obj,$(OBJS),$(obj).o)

$(SRC_DIR)/debug/%.o: $(SRC_DIR)/%.c
	$(CCILINUX_eti_CC) $(CFLAGS) -c $< -o $@  

$(SRC_DIR)/debug/%.o: $(SRC_DIR)/%.cpp
	$(CCILINUX_eti_CPP) $(CFLAGS) -c $< -o $@
	
all:$(LIB_TARGET)

test:$(APP_TEST)

$(LIB_TARGET): $(OBJS)
	$(CCILINUX_eti_AR) crus $(LIB_TARGET) $(OBJS)
	\cp $(LIB_TARGET) $(V2DOT0_LIB_DIR)/

$(APP_TEST): $(SRC_DIR)/debug/test.o
	$(CCILINUX_eti_CC) -o $(APP_TEST) $(SRC_DIR)/debug/test.o -lMobileServerSdk -llocalsdk -lctrlprotocol  -lpthread -L.
	$(CCILINUX_eti_STRIP) $(APP_TEST)
	install -m 755 $(APP_TEST) /tftpboot/

clean:
	rm -f $(OBJS) *.pdb *.map
	rm -f $(LIB_TARGET)

print: $(SRC_DIR)/*.c
	lpr -p $?
	touch print

