include  ../Makefile.param

CCILINUX_eti_CC    := $(CCARCH)gcc
CCILINUX_eti_CPP   := $(CCARCH)g++
CCILINUX_eti_LD    := $(CCARCH)ld
CCILINUX_eti_AR    := $(CCARCH)ar
CCILINUX_eti_STRIP := $(CCARCH)strip

COMMON_DIR := ../Common

INC_DIR := Include $(COMMON_DIR)/inc $(COMMON_DIR)/inc/mod $(COMMON_DIR)/inc/tlsdk $(COMMON_DIR)/inc/biz
INC_DIR += $(TL_APP_SDK_DIR)/pub/kbuild-FULL_REL/include
SRC_DIR := source
OBJ_DIR := obj


#APP_TARGET := testui
APP_TARGET := myapp
INSTALL_APP_PATH := /mnt/nfs


#INC_PATH += $(INC_DIR) ../../common/include
INC_PATH += $(INC_DIR)
#INC_PATH += /develop/hs/Hi3511_VSSDK_V1.1.2.3/pub/kbuild-LESS_REL/include

LIB_PATH += lib 
LIB_PATH += $(COMMON_DIR)/lib $(COMMON_DIR)/lib/debug
LDFLAGS += $(foreach lib,$(LIB_PATH),-L$(lib))
CFLAGS  += $(foreach dir,$(INC_PATH),-I$(dir))

include ../tmp.mak
ifdef MYDEFINE
	CFLAGS += -D$(MYDEFINE)
endif
ifdef MMUCHIP
	CFLAGS += -D$(MMUCHIP)
endif

#CFLAGS += -O2 -Wall -fno-strict-aliasing
CFLAGS += $(TL_APP_INC_PATH)

LDFLAGS += -lnetcomm -lrecord -lencode -lpreview -lpb -lcustommp4 -lapi -lalarm -lcirclebuf -lptz -lsyscomplex -lpublic -luser -lbiz -lconfig -llog 
LDFLAGS += -lgraphics -lpanel -lmouse -lm -lpthread -lz -ljpeg
LDFLAGS += $(COMMON_DIR)/lib/liblua.a
LDFLAGS += $(COMMON_DIR)/lib/liblualib.a
#LDFLAGS += $(COMMON_DIR)/lib/tl_lib.a
LDFLAGS += $(COMMON_DIR)/lib/tl_hslib_nvr.a
LDFLAGS += $(COMMON_DIR)/lib/libctrlprotocol.a
LDFLAGS += $(COMMON_DIR)/lib/libapi.a
LDFLAGS += $(COMMON_DIR)/lib/libcustommp4.a 
#LDFLAGS += $(TL_APP_SDK_DIR)/mpp_master/lib/libpciv.a

LDFLAGS += -lipcamera
LDFLAGS += -lyzonvif -luuid
LDFLAGS += -lxml2

LDFLAGS += -lFHDEV_Net
LDFLAGS += -lVVVNetsearchc -lVVVNetlivec_print -lSendCgi
LDFLAGS += -lVVVDEVS -lvvvcommon
LDFLAGS += -lNet

LDFLAGS += -lcharset
LDFLAGS += -liconv

LDFLAGS += -lssl -lcrypto
LDFLAGS += -L$(COMMON_DIR)/lib/Hi35xx

LDFLAGS += -lg726

#LDFLAGS += $(COMMON_DIR)/lib/libtestMPEG4VideoStreamer.a
#LDFLAGS += $(COMMON_DIR)/lib/libliveMedia.a
#LDFLAGS += $(COMMON_DIR)/lib/libgroupsock.a
#LDFLAGS += $(COMMON_DIR)/lib/libBasicUsageEnvironment.a
#LDFLAGS += $(COMMON_DIR)/lib/libUsageEnvironment.a

LDFLAGS += $(COMMON_DIR)/lib/libMobileServerSdk.a
LDFLAGS += $(COMMON_DIR)/lib/libtools.a

#LDFLAGS += -lrtspclient
LDFLAGS += -lliveMedia -lgroupsock -lBasicUsageEnvironment -lUsageEnvironment
###LDFLAGS += -lminirtsp
LDFLAGS += -L$(COMMON_DIR)/lib/onvif -L$(COMMON_DIR)/lib/live
###LDFLAGS += -L$(COMMON_DIR)/lib/onvif
#LDFLAGS += -lavformat -lavcodec -lavutil
#LDFLAGS += -L$(COMMON_DIR)/lib/onvif -L$(COMMON_DIR)/lib/ffmpeg
#LDFLAGS += -lrtsp_rev
#LDFLAGS += -L$(COMMON_DIR)/lib/onvif

LDFLAGS += $(TL_APP_LIBS)

OBJS :=	$(OBJ_DIR)/Bitmap	\
		$(OBJ_DIR)/BitmapButton	\
		$(OBJ_DIR)/Button	\
		$(OBJ_DIR)/Calendar	\
		$(OBJ_DIR)/CheckBox	\
		$(OBJ_DIR)/ComboBox	\
		$(OBJ_DIR)/Cursor	\
		$(OBJ_DIR)/DateTimeCtrl		\
		$(OBJ_DIR)/DC				\
		$(OBJ_DIR)/DevFrontboard	\
		$(OBJ_DIR)/DevGraphics	\
		$(OBJ_DIR)/DevMouse	\
		$(OBJ_DIR)/Draw	\
		$(OBJ_DIR)/Edit	\
		$(OBJ_DIR)/GridCtrl	\
		$(OBJ_DIR)/GroupBox	\
		$(OBJ_DIR)/GUI	\
		$(OBJ_DIR)/IMM	\
		$(OBJ_DIR)/IPAddressctrl	\
		$(OBJ_DIR)/Item	\
		$(OBJ_DIR)/ListBox	\
		$(OBJ_DIR)/Menu	\
		$(OBJ_DIR)/MsgQue	\
		$(OBJ_DIR)/mutex	\
		$(OBJ_DIR)/NumberBox	\
		$(OBJ_DIR)/NumberPad	\
		$(OBJ_DIR)/Page	\
		$(OBJ_DIR)/Parse	\
		$(OBJ_DIR)/pool_allocator	\
		$(OBJ_DIR)/ProgressCtrl	\
		$(OBJ_DIR)/Radio	\
		$(OBJ_DIR)/Region	\
		$(OBJ_DIR)/ScrollBar	\
		$(OBJ_DIR)/Timer	\
		$(OBJ_DIR)/TimingCtrl	\
		$(OBJ_DIR)/File	\
		$(OBJ_DIR)/Locales	\
		$(OBJ_DIR)/str_opr	\
		$(OBJ_DIR)/tsk		\
		$(OBJ_DIR)/PageDesktop	\
		$(OBJ_DIR)/PageStart	\
		$(OBJ_DIR)/SliderCtrl	\
		$(OBJ_DIR)/SliderCtrlPartColor \
		$(OBJ_DIR)/Static	\
		$(OBJ_DIR)/sem	\
		$(OBJ_DIR)/sys	\
		$(OBJ_DIR)/PageInputTray  \
		$(OBJ_DIR)/TableBox \
		$(OBJ_DIR)/RecFileTimeBox	\
		$(OBJ_DIR)/Thread	\
		$(OBJ_DIR)/PageFrame	\
		$(OBJ_DIR)/PageMainFrameWork	\
		$(OBJ_DIR)/PageConfigFrameWork \
		$(OBJ_DIR)/PageShenGuangConfig  \
		$(OBJ_DIR)/PageRecordFrameWork \
		$(OBJ_DIR)/PagePlayBackFrameWork \
		$(OBJ_DIR)/PageStartFrameWork \
		$(OBJ_DIR)/PageBasicConfigFrameWork \
		$(OBJ_DIR)/ntp_client \
		$(OBJ_DIR)/PageIPCameraConfigFrameWork \
		$(OBJ_DIR)/PageLiveConfigFrameWork \
		$(OBJ_DIR)/PageAdvanceConfigFrameWork \
		$(OBJ_DIR)/PageImportExportFrameWork \
		$(OBJ_DIR)/PageRecordScheduleFrameWork \
		$(OBJ_DIR)/PageMainTain \
		$(OBJ_DIR)/PageMessageBox  \
		$(OBJ_DIR)/PagePtzCtl \
		$(OBJ_DIR)/PagePtzTrack \
		$(OBJ_DIR)/PagePtzPreset \
		$(OBJ_DIR)/PagePtzConfigFrameWork \
		$(OBJ_DIR)/PageAlarmOut   \
		$(OBJ_DIR)/PageManualAlarm \
		$(OBJ_DIR)/PageUserManager  \
		$(OBJ_DIR)/PageEditPwd \
		$(OBJ_DIR)/PageAddUser \
		$(OBJ_DIR)/PageModifyUser \
		$(OBJ_DIR)/PageSearch	\
		$(OBJ_DIR)/PageFloat	\
		$(OBJ_DIR)/PageChnSelect	\
		$(OBJ_DIR)/PageAudioSelect	\
		$(OBJ_DIR)/PagePlayrateSelect \
		$(OBJ_DIR)/PageCruiseLine  \
		$(OBJ_DIR)/PageCruisePoint  \
		$(OBJ_DIR)/PageColorSetup   \
		$(OBJ_DIR)/PagePlayBackVoColorSetup   \
		$(OBJ_DIR)/PageDataBackup	\
		$(OBJ_DIR)/PageInfoFrameWork \
		$(OBJ_DIR)/PageSysInfo \
		$(OBJ_DIR)/PageDiskMgr \
		$(OBJ_DIR)/PageUpdate \
		$(OBJ_DIR)/PageAlarmCfgFrameWork \
		$(OBJ_DIR)/PageSensorAlarm \
		$(OBJ_DIR)/PageIPCameraExtSensorAlarm \
		$(OBJ_DIR)/PageAlarmDeal \
		$(OBJ_DIR)/PageMDCfg \
		$(OBJ_DIR)/PageVideoLoss \
		$(OBJ_DIR)/PageOtherAlarm \
		$(OBJ_DIR)/PageOSDPosSet \
		$(OBJ_DIR)/PageMaskSet \
		$(OBJ_DIR)/PageNetworkConfig \
		$(OBJ_DIR)/PageMacAddress \
		$(OBJ_DIR)/PageMotionRgnSet \
		$(OBJ_DIR)/PageEventList \
		$(OBJ_DIR)/PageSysLog \
		$(OBJ_DIR)/PageNetwork  \
		$(OBJ_DIR)/PageGuide  \
		$(OBJ_DIR)/PageOnlineUser  \
		$(OBJ_DIR)/PageLogin  \
		$(OBJ_DIR)/PageMenueOutputDev  \
		$(OBJ_DIR)/PageAddCurisePoint \
		$(OBJ_DIR)/PagePresetSetup \
		$(OBJ_DIR)/PageBackupInfo \
		$(OBJ_DIR)/PageInfoBox  \
		$(OBJ_DIR)/PagePIP  \
		$(OBJ_DIR)/PageAlarmList \
		$(OBJ_DIR)/PageSnapPlayBack  \
		$(OBJ_DIR)/BizData  \
		$(OBJ_DIR)/testUI	\
		$(OBJ_DIR)/PageStream	\
		#$(OBJ_DIR)/PageTest \
		#$(OBJ_DIR)/RecFileTimeBox	\
		#$(OBJ_DIR)/PageSplitFrameWork	\
		#$(OBJ_DIR)/Frontboard	\
		



OBJS := $(foreach obj,$(OBJS),$(obj).o)

#$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
#$(OBJ_DIR)/%.o: $(SRC_DIR)/testlocal.c
#	$(CCILINUX_eti_CC) $(CFLAGS) -c $< -o $@ 
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CCILINUX_eti_CC) $(CFLAGS) -c $< -o $@	
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CCILINUX_eti_CPP) $(CFLAGS) -c $< -o $@
	
all:$(APP_TARGET)
$(APP_TARGET): $(OBJS)
#	$(CCILINUX_eti_CC) -o $(APP_TARGET) $(OBJS) $(LDFLAGS)
	$(CCILINUX_eti_CPP) -o $(APP_TARGET)  $(OBJS)  $(LDFLAGS) 
	$(CCILINUX_eti_STRIP) $(APP_TARGET)
	install -m 0755 $(APP_TARGET) $(INSTALL_APP_PATH)
	install -m 0755 $(APP_TARGET) /tftpboot
	cp $(APP_TARGET) /tftpboot -rf
	cp $(APP_TARGET) /mnt/nfs -rf
	
clean:
	rm -f $(OBJS) *.pdb *.map
	rm -f $(APP_TARGET)

print: $(SRC_DIR)/*.c $(SRC_DIR)/*.cpp
	lpr -p $?
	touch print
