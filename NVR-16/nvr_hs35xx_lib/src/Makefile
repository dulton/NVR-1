include  ../../Makefile.param

LIN_CODE_DIR=..

TARGET=tl_hslib_nvr.a

INC_PATH += -I$(LIN_CODE_DIR)/include
INC_PATH += -I$(LIN_CODE_DIR)/tl_include
INC_PATH += -I../../Common/inc/tlsdk
INC_PATH += -I../../Common/inc
INC_PATH += $(TL_APP_INC_PATH)

#CROSS=arm-hisiv100nptl-linux-
CROSS=$(CCARCH)

CC=$(CROSS)gcc
AR=$(CROSS)ar

CFLAGS += -O2 -Wall $(INC_PATH) -D$(HISIARCH)
#CFLAGS += -Wall $(INC_PATH) -D$(HISIARCH)

all:$(TARGET)

#*************************all source files***********************
SOURCES = lib_preview.c tl_hslib_com.c vio_common.c lib_venc.c lib_vdec.c lib_venc_osd.c lib_video_cover.c lib_md.c lib_misc.c lib_audio.c common.c lib_alarm.c lib_video.c lib_snapshot.c
#****************************************************************

OBJS = $(patsubst %.c,%.o,$(SOURCES)) 

$(OBJS): %.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<
$(TARGET):$(OBJS)
	$(AR) -rc $(TARGET) $(OBJS)
	chmod 777 $(TARGET)
	cp -f $(TARGET) $(LIN_CODE_DIR)/lib/
	cp -f $(TARGET) ../../Common/lib/

clean:
	rm -f *.o $(TARGET) *.a
	rm -f $(LIN_CODE_DIR)/lib/$(TARGET)
	#rm -rf $(LIN_CODE_DIR)/lib/*
