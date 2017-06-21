include  ../../Makefile.param

COMMON_DIR := ../../common

LIB_CODE_DIR=..

REL_LIB = $(TL_APP_LIB_PATH)

TL_LIB := $(LIB_CODE_DIR)/src
TL_INCA := $(LIB_CODE_DIR)/include
TL_INCB := $(LIB_CODE_DIR)/tl_include

INC_FLAGS := -I$(TL_INCA) -I$(TL_INCB)
INC_FLAGS += $(TL_APP_INC_PATH)

CFLAGS := -O2 -Wall $(INC_FLAGS) -lpthread -lm

CFLAGS += ../lib/tl_hslib_nvr.a
CFLAGS += -lyzonvif -luuid
CFLAGS += -lrtspclient
CFLAGS += -lliveMedia -lgroupsock -lBasicUsageEnvironment -lUsageEnvironment

CFLAGS += -L../lib -L../live/lib -L$(COMMON_DIR)/lib -L$(COMMON_DIR)/lib/onvif -L$(COMMON_DIR)/lib/live

#CFLAGS += -march=armv7-a -mcpu=cortex-a9

#CROSS:= arm-hisiv100nptl-linux-
CROSS = $(CCARCH)

CC:=$(CROSS)gcc
CPP:=$(CROSS)g++

SRC  := app_test.c test_sys_performance.c
OBJ  := $(SRC:%.c=%.o) 
	
TARGET := app_test
.PHONY : clean all

all: $(TARGET)

MPI_LIBS = $(TL_APP_LIBS)

./%.o: ./%.c
	$(CC) $(CFLAGS) -c $< -o $@  
./%.o: ./%.cpp
	$(CPP) $(CFLAGS) -c $< -o $@

$(TARGET):$(OBJ)
	#$(CPP) $(CFLAGS) -o $@ $^ $(MPI_LIBS)
	$(CPP) -o $@ $^ $(CFLAGS) $(MPI_LIBS)
	cp -f $(TARGET) /mnt/nfs/

clean:
	@rm -f $(TARGET)
	@rm -f $(OBJ)
