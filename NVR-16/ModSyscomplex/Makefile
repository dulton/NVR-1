include ../Makefile.param

AR    := $(CCARCH)ar
CC    := $(CCARCH)gcc

SRC_DIR := ./src
OBJ_DIR := ./obj

INC_LIB = ../common/lib/debug

INC_PATH += ./inc  ../common/inc ../common/inc/mod ../common/inc/tlsdk
INC_PATH += $(TL_APP_SDK_DIR)/pub/kbuild-FULL_REL/include


#CFLAGS := -O3 -Wall -fno-strict-aliasing
CFLAGS += -O2 -Wall -fno-strict-aliasing
CFLAGS += $(foreach inc,$(INC_PATH),-I$(inc))
CFLAGS += $(foreach lib,$(LIB_DIR),-L$(lib))

include ../tmp.mak
ifdef MMUCHIP
	CFLAGS += -D$(MMUCHIP)
endif

LDFLAGS += -lpthread
LDFLAGS += -L$(INC_LIB)/ -L../common/lib -lpublic
LDFLAGS += -L $(INC_LIB)/libcustommp4.a 
LDFLAGS += -L $(INC_LIB)/libapi.a 

OBJS :=$(OBJ_DIR)/mod_syscomplex		\
		$(OBJ_DIR)/update				\
		$(OBJ_DIR)/search				\
		$(OBJ_DIR)/format				\
		$(OBJ_DIR)/backup				\
		$(OBJ_DIR)/avilib				\
		$(OBJ_DIR)/syscomplxtime		\
		$(OBJ_DIR)/DiskInfoUpdate		\
#		$(OBJ_DIR)/crc32		        \
#		$(OBJ_DIR)/demo_v20_wrflash     \
#		$(OBJ_DIR)/bbfunc				\
#		$(OBJ_DIR)/fdisk				\

		
OBJS := $(foreach obj,$(OBJS),$(obj).o)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c 
	$(CC) $(CFLAGS) -fPIC -c $< -o $@

all: $(OBJS) libsyscomplex.so 


libsyscomplex.so: $(OBJS)
	$(CC) $(CFLAGS) -fPIC -shared $(OBJS) -o libsyscomplex.so $(LDFLAGS) -L../Common/lib/tl_lib.a -L../Common/lib/libtools.a
	\cp libsyscomplex.so /tftpboot
	\cp libsyscomplex.so ../common/lib/debug
	\cp libsyscomplex.so /mnt/nfs
	
copy:
	cp *.o ./obj; cp *.so ./obj
	
clean:
	rm -f *.o; rm -f *.so;rm -f $(OBJ_DIR)/*.o;

