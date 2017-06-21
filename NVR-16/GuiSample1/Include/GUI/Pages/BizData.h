#ifndef _BIZ_DATA_H_
#define _BIZ_DATA_H_

#include "GUI/Ctrls/Page.h"
#include "APIs/System.h"
#include "common_basetypes.h"
#include "biz.h"
#include "lib_video.h"
#include <semaphore.h>
#include <dirent.h>
#include <arpa/inet.h>


// debug by lanston
//
#define BIZ_DATA_DBG(str)	//printf("line:%d %s\n",__LINE__,str)
#define BIZ_DT_DBG	
#define BIZ_DO_DBG	

#define CHN_MAX	32

#define CUR_SCH_SEG										MAX_SCHEDULE_SEGMENTS
#define ALL_DEFAULT										0xff
#define ALL_SEL											0xff
#define CBX_MAX											33
#define EDIT_MAX										128
#define LINKAGE_INDEX									16

#define DBG_LNKALARM_INDEX_MAX							128

#define MEGA	(1000*1000)
#define TENKB	(10*1000)

//#define DISABLE_ALARM_SCH								// 20111014 by lanston
#define DISABLE_PREREC_TIME								// 20111014 by lanston
//yaogang modify 20141106
#define DISABLE_RECBITRATE
#define DISABLE_STAMP

//yaogang modify 20160330
//NVR 不带深广平台版本
//#define NO_MODULE_SG
#undef NO_MODULE_SG


typedef struct
{
	int nValue;
	char strDisplay[32];
} SValue; 

typedef enum//cw_led
{
    ALARM_LED_CTRL = 0, 
    REC_LED_CTRL,
} LEDCTRLTYPE;

#define GSR_DEFAULT_OFFSET	0x10000

// === config ===
#define GSR_CONFIG_BASE	0
// -------------------------------basic
#define GSR_CONFIG_BASIC_BASE							GSR_CONFIG_BASE+0
//system..
#define GSR_CONFIG_BASIC_SYSTEM_BASE 					GSR_CONFIG_BASIC_BASE+0
#define GSR_CONFIG_BASIC_SYSTEM_DEVICENAME				GSR_CONFIG_BASIC_SYSTEM_BASE+0
#define GSR_CONFIG_BASIC_SYSTEM_DEVICEID				GSR_CONFIG_BASIC_SYSTEM_BASE+1
#define GSR_CONFIG_BASIC_SYSTEM_VIDEOFORMAT			GSR_CONFIG_BASIC_SYSTEM_BASE+2
#define GSR_CONFIG_BASIC_SYSTEM_AUTHCHECK				GSR_CONFIG_BASIC_SYSTEM_BASE+3
#define GSR_CONFIG_BASIC_SYSTEM_TIMEDISPLAY			GSR_CONFIG_BASIC_SYSTEM_BASE+4
#define GSR_CONFIG_BASIC_SYSTEM_MAXNETUSERS			GSR_CONFIG_BASIC_SYSTEM_BASE+5
#define GSR_CONFIG_BASIC_SYSTEM_OUTPUT					GSR_CONFIG_BASIC_SYSTEM_BASE+6
#define GSR_CONFIG_BASIC_SYSTEM_LANGUAGE 				GSR_CONFIG_BASIC_SYSTEM_BASE+7
#define GSR_CONFIG_BASIC_SYSTEM_SHOWGUIDE				GSR_CONFIG_BASIC_SYSTEM_BASE+8
#define GSR_CONFIG_BASIC_SYSTEM_SCRENNSAVER			GSR_CONFIG_BASIC_SYSTEM_BASE+9
//yaogang modify 20140918
#define GSR_CONFIG_BASIC_SYSTEM_SHOWCHNKBPS			GSR_CONFIG_BASIC_SYSTEM_BASE+10



//date&time..
#define GSR_CONFIG_BASIC_DATETIME_BASE 					GSR_CONFIG_BASIC_BASE+0x80
#define GSR_CONFIG_BASIC_DATETIME_DATEFORMAT 			GSR_CONFIG_BASIC_DATETIME_BASE+0
#define GSR_CONFIG_BASIC_DATETIME_TIMEFORMAT 			GSR_CONFIG_BASIC_DATETIME_BASE+1
#define GSR_CONFIG_BASIC_DATETIME_TIMEZONE 				GSR_CONFIG_BASIC_DATETIME_BASE+2
#define GSR_CONFIG_BASIC_DATETIME_NETSYN 				GSR_CONFIG_BASIC_DATETIME_BASE+3
#define GSR_CONFIG_BASIC_DATETIME_NTPSERVER 			GSR_CONFIG_BASIC_DATETIME_BASE+4
#define GSR_CONFIG_BASIC_DATETIME_SYSDATE 				GSR_CONFIG_BASIC_DATETIME_BASE+5
#define GSR_CONFIG_BASIC_DATETIME_SYSTIME 				GSR_CONFIG_BASIC_DATETIME_BASE+6
#define GSR_CONFIG_BASIC_DATETIME_SYNCTIMETOIPC 		GSR_CONFIG_BASIC_DATETIME_BASE+7


//-------------------------------live
#define GSR_CONFIG_LIVE_BASE							GSR_CONFIG_BASE+0x100
#define GSR_CONFIG_LIVE_LIVE							GSR_CONFIG_LIVE_BASE+0
#define GSR_CONFIG_LIVE_ALLCHN							GSR_CONFIG_LIVE_BASE+1
#define GSR_CONFIG_LIVE_CHNNAME							GSR_CONFIG_LIVE_BASE+2
#define GSR_CONFIG_LIVE_DISPLAY							GSR_CONFIG_LIVE_BASE+3
#define GSR_CONFIG_LIVE_LIVEREC							GSR_CONFIG_LIVE_BASE+4
#define GSR_CONFIG_LIVE_COLORSET							GSR_CONFIG_LIVE_BASE+5

#define GSR_CONFIG_LIVE_MAINMONITOR							GSR_CONFIG_LIVE_BASE+0x40
#define GSR_CONFIG_LIVE_MAINMONITOR_SPLITMODE				GSR_CONFIG_LIVE_MAINMONITOR+0
#define GSR_CONFIG_LIVE_MAINMONITOR_CHN				GSR_CONFIG_LIVE_MAINMONITOR+1
#define GSR_CONFIG_LIVE_MAINMONITOR_DWELLTIME				GSR_CONFIG_LIVE_MAINMONITOR+2

#define GSR_CONFIG_LIVE_SPOT							GSR_CONFIG_LIVE_BASE+0x80
#define GSR_CONFIG_LIVE_SPOT_SPLITMODE							GSR_CONFIG_LIVE_SPOT+0
#define GSR_CONFIG_LIVE_SPOT_CHN							GSR_CONFIG_LIVE_SPOT+1
#define GSR_CONFIG_LIVE_SPOT_DWELLTIME							GSR_CONFIG_LIVE_SPOT+2

#define GSR_CONFIG_LIVE_MASK							GSR_CONFIG_LIVE_BASE+0xc0
#define GSR_CONFIG_LIVE_MASK_AREA							GSR_CONFIG_LIVE_MASK+0



//-------------------------------record
#define GSR_CONFIG_RECORD_BASE							GSR_CONFIG_BASE+0x200
#define GSR_CONFIG_RECORD_ENABLE_BASE					GSR_CONFIG_RECORD_BASE+0
#define GSR_CONFIG_RECORD_ENABLE_ALLCHN					GSR_CONFIG_RECORD_ENABLE_BASE+0
#define GSR_CONFIG_RECORD_ENABLE_RECORD					GSR_CONFIG_RECORD_BASE+1
#define GSR_CONFIG_RECORD_ENABLE_AUDIO					GSR_CONFIG_RECORD_BASE+2

#define GSR_CONFIG_RECORD_ENABLE_CODESTREAM_BASE					GSR_CONFIG_RECORD_BASE+0x10
#define GSR_CONFIG_RECORD_ENABLE_CODESTREAM_ALLCHN					GSR_CONFIG_RECORD_ENABLE_CODESTREAM_BASE+0
#define GSR_CONFIG_RECORD_ENABLE_CODESTREAM_RESOLUTION					GSR_CONFIG_RECORD_ENABLE_CODESTREAM_BASE+1
#define GSR_CONFIG_RECORD_ENABLE_CODESTREAM_FRAMERATE					GSR_CONFIG_RECORD_ENABLE_CODESTREAM_BASE+2
#define GSR_CONFIG_RECORD_ENABLE_CODESTREAM_ENCODE					GSR_CONFIG_RECORD_ENABLE_CODESTREAM_BASE+3
#define GSR_CONFIG_RECORD_ENABLE_CODESTREAM_QUALITY					GSR_CONFIG_RECORD_ENABLE_CODESTREAM_BASE+4
#define GSR_CONFIG_RECORD_ENABLE_CODESTREAM_MAXRATE				GSR_CONFIG_RECORD_ENABLE_CODESTREAM_BASE+5

#define GSR_CONFIG_RECORD_ENABLE_RECORDTIME_BASE				GSR_CONFIG_RECORD_BASE+0x20
#define GSR_CONFIG_RECORD_ENABLE_RECORDTIME_ALLCHN				GSR_CONFIG_RECORD_ENABLE_RECORDTIME_BASE+0
#define GSR_CONFIG_RECORD_ENABLE_RECORDTIME_TIMEBEFALARM		GSR_CONFIG_RECORD_ENABLE_RECORDTIME_BASE+1
#define GSR_CONFIG_RECORD_ENABLE_RECORDTIME_TIMEAFTALARM				GSR_CONFIG_RECORD_ENABLE_RECORDTIME_BASE+2
#define GSR_CONFIG_RECORD_ENABLE_RECORDTIME_EXPIRATION				GSR_CONFIG_RECORD_ENABLE_RECORDTIME_BASE+3

#define GSR_CONFIG_RECORD_ENABLE_OSD_BASE					GSR_CONFIG_RECORD_BASE+0x30
#define GSR_CONFIG_RECORD_ENABLE_OSD_ALLCHN					GSR_CONFIG_RECORD_ENABLE_OSD_BASE+0
#define GSR_CONFIG_RECORD_ENABLE_OSD_CHNNAME					GSR_CONFIG_RECORD_ENABLE_OSD_BASE+1
#define GSR_CONFIG_RECORD_ENABLE_OSD_TIMESTAMP					GSR_CONFIG_RECORD_ENABLE_OSD_BASE+2
#define GSR_CONFIG_RECORD_ENABLE_OSD_POSITION					GSR_CONFIG_RECORD_ENABLE_OSD_BASE+3

#define GSR_CONFIG_RECORD_ENABLE_CYCLINGREC_BASE					GSR_CONFIG_RECORD_BASE+0x40
#define GSR_CONFIG_RECORD_ENABLE_CYCLINGREC_REC					GSR_CONFIG_RECORD_ENABLE_CYCLINGREC_BASE+0


#define GSR_CONFIG_RECORD_ENABLE_SNAP_BASE					GSR_CONFIG_RECORD_BASE+0x50
#define GSR_CONFIG_RECORD_SNAP_CHN_CHOICE						GSR_CONFIG_RECORD_ENABLE_SNAP_BASE+0
//Resolution=分辨率
#define GSR_CONFIG_RECORD_SNAP_RES								GSR_CONFIG_RECORD_ENABLE_SNAP_BASE+1
//Quality=图片质量
#define GSR_CONFIG_RECORD_SNAP_QUALITY							GSR_CONFIG_RECORD_ENABLE_SNAP_BASE+2
//Interval=抓图时间间隔
#define GSR_CONFIG_RECORD_SNAP_INTERVAL						GSR_CONFIG_RECORD_ENABLE_SNAP_BASE+3

//-------------------------------schedule
#define GSR_CONFIG_SCHEDULE_BASE		GSR_CONFIG_BASE+0x300
#define GSR_CONFIG_SCHEDULE_TIMEREC		GSR_CONFIG_SCHEDULE_BASE+0
#define GSR_CONFIG_SCHEDULE_TIMEREC_CHN		GSR_CONFIG_SCHEDULE_TIMEREC+0
#define GSR_CONFIG_SCHEDULE_TIMEREC_SCH		GSR_CONFIG_SCHEDULE_TIMEREC+1
#define GSR_CONFIG_SCHEDULE_TIMEREC_COPYTO		GSR_CONFIG_SCHEDULE_TIMEREC+2


#define GSR_CONFIG_SCHEDULE_MOTIONALARMREC		GSR_CONFIG_SCHEDULE_BASE+0x20
#define GSR_CONFIG_SCHEDULE_MOTIONALARMREC_CHN		GSR_CONFIG_SCHEDULE_MOTIONALARMREC+0
#define GSR_CONFIG_SCHEDULE_MOTIONALARMREC_SCH		GSR_CONFIG_SCHEDULE_MOTIONALARMREC+1
#define GSR_CONFIG_SCHEDULE_MOTIONALARMREC_COPYTO		GSR_CONFIG_SCHEDULE_MOTIONALARMREC+2

#define GSR_CONFIG_SCHEDULE_SENSORALARMREC		GSR_CONFIG_SCHEDULE_BASE+0x30
#define GSR_CONFIG_SCHEDULE_SENSORALARMREC_CHN		GSR_CONFIG_SCHEDULE_SENSORALARMREC+0
#define GSR_CONFIG_SCHEDULE_SENSORALARMREC_SCH		GSR_CONFIG_SCHEDULE_SENSORALARMREC+1
#define GSR_CONFIG_SCHEDULE_SENSORALARMREC_COPYTO		GSR_CONFIG_SCHEDULE_SENSORALARMREC+2


//-------------------------------alarm
#define GSR_CONFIG_ALARM_BASE			GSR_CONFIG_BASE+0x400
#define GSR_CONFIG_ALARM_SENSOR			GSR_CONFIG_ALARM_BASE+0
#define GSR_CONFIG_ALARM_SENSOR_BASE			GSR_CONFIG_ALARM_SENSOR+0
#define GSR_CONFIG_ALARM_SENSOR_BASE_ALLCHN			GSR_CONFIG_ALARM_SENSOR_BASE+0
#define GSR_CONFIG_ALARM_SENSOR_BASE_ENABLE			GSR_CONFIG_ALARM_SENSOR_BASE+1
#define GSR_CONFIG_ALARM_SENSOR_BASE_TYPE			GSR_CONFIG_ALARM_SENSOR_BASE+2
#define GSR_CONFIG_ALARM_SENSOR_BASE_NAME			GSR_CONFIG_ALARM_SENSOR_BASE+3


#define GSR_CONFIG_ALARM_SENSOR_ALARMDEAL			GSR_CONFIG_ALARM_SENSOR+0x20
#define GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALLCHN			GSR_CONFIG_ALARM_SENSOR_ALARMDEAL+0
#define GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_DELAYTIME			GSR_CONFIG_ALARM_SENSOR_ALARMDEAL+1

#define GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_DELAYTIME+1
#define GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_SOUND 	GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE+0
#define GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_BIGVIDEO 	GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE+1
#define GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_TIRGGER 	GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE+2
#define GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_MAIL 	GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE+3
#define GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_SNAP 	GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE+4 // bitmap

#define GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_TRIGGER 	GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_SNAP+1

#define GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_LINKAGE 	GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_TRIGGER+1
#define GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_LINKAGE_TYPE 	GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_LINKAGE+0
#define GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_LINKAGE_INDEX 	GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_LINKAGE+1



#define GSR_CONFIG_ALARM_SENSOR_SCH			GSR_CONFIG_ALARM_SENSOR+0x30
#define GSR_CONFIG_ALARM_SENSOR_SCH_CHN		GSR_CONFIG_ALARM_SENSOR_SCH+0
#define GSR_CONFIG_ALARM_SENSOR_SCH_SCH		GSR_CONFIG_ALARM_SENSOR_SCH+1
#define GSR_CONFIG_ALARM_SENSOR_SCH_COPYTO		GSR_CONFIG_ALARM_SENSOR_SCH+2

///

#define GSR_CONFIG_ALARM_MD							GSR_CONFIG_ALARM_BASE+0x40
#define GSR_CONFIG_ALARM_MD_MOTIONDETECT	 		GSR_CONFIG_ALARM_MD+0
#define GSR_CONFIG_ALARM_MD_MOTIONDETECT_ENABLE 	GSR_CONFIG_ALARM_MD_MOTIONDETECT+0
#define GSR_CONFIG_ALARM_MD_MOTIONDETECT_DELAYTIME 	GSR_CONFIG_ALARM_MD_MOTIONDETECT+1
#define GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALLCHN		GSR_CONFIG_ALARM_MD_MOTIONDETECT+2

#define GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL 	GSR_CONFIG_ALARM_MD_MOTIONDETECT+3

#define GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE 	GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL+0
#define GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE_SOUND 	GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE+0
#define GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE_BIGVIDEO 	GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE+1
#define GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE_TIRGGER 	GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE+2
#define GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE_MAIL 	GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE+3
#define GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE_SNAP 	GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE+4 // bitmap

#define GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_TRIGGER 	GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL+0x10

#define GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_LINKAGE 	GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL+0x20
#define GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_LINKAGE_TYPE 	GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_LINKAGE+0
#define GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_LINKAGE_INDEX 	GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_LINKAGE+1

#define GSR_CONFIG_ALARM_MD_MOTIONDETECT_RGNSET 	GSR_CONFIG_ALARM_MD+0x28
#define GSR_CONFIG_ALARM_MD_MOTIONDETECT_RGNSET_SENSITY 	GSR_CONFIG_ALARM_MD+0x29


#define GSR_CONFIG_ALARM_MD_SCH				GSR_CONFIG_ALARM_MD+0x30
#define GSR_CONFIG_ALARM_MD_SCH_SCH_CHN		GSR_CONFIG_ALARM_MD_SCH+0
#define GSR_CONFIG_ALARM_MD_SCH_SCH_SCH		GSR_CONFIG_ALARM_MD_SCH+1
#define GSR_CONFIG_ALARM_MD_SCH_SCH_COPYTO		GSR_CONFIG_ALARM_MD_SCH+2


#define GSR_CONFIG_ALARM_VIDEOLOST			GSR_CONFIG_ALARM_BASE+0x80

#define GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL 	GSR_CONFIG_ALARM_VIDEOLOST+0

#define GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE 	GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL+0
#define GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE_SOUND 	GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE+0
#define GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE_BIGVIDEO 	GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE+1
#define GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE_TIRGGER 	GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE+2
#define GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE_MAIL 	GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE+3
#define GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE_SNAP 	GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE+4 // bitmap

#define GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_TRIGGER 	GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL+0x10

#define GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_LINKAGE 	GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL+0x20
#define GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_LINKAGE_TYPE 	GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_LINKAGE+0
#define GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_LINKAGE_INDEX 	GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_LINKAGE+1

#define GSR_CONFIG_ALARM_OTHER						GSR_CONFIG_ALARM_BASE+0xC0
#define GSR_CONFIG_ALARM_OTHER_BASE					GSR_CONFIG_ALARM_OTHER+0
#define GSR_CONFIG_ALARM_OTHER_ALARMTYPE			GSR_CONFIG_ALARM_OTHER_BASE+1
#define GSR_CONFIG_ALARM_OTHER_SOUNDALARM			GSR_CONFIG_ALARM_OTHER_BASE+2
#define GSR_CONFIG_ALARM_OTHER_MAIL					GSR_CONFIG_ALARM_OTHER_BASE+3
#define GSR_CONFIG_ALARM_OTHER_TRIGGERALARM			GSR_CONFIG_ALARM_OTHER_BASE+4
#define GSR_CONFIG_ALARM_OTHER_DISKALARM			GSR_CONFIG_ALARM_OTHER_BASE+5
#define GSR_CONFIG_ALARM_OTHER_ALLCHN				GSR_CONFIG_ALARM_OTHER_BASE+6


#define GSR_CONFIG_ALARM_ALARMOUT				GSR_CONFIG_ALARM_BASE+0xD0

#define GSR_CONFIG_ALARM_ALARMOUT_ALARMOUT		GSR_CONFIG_ALARM_ALARMOUT+0
#define GSR_CONFIG_ALARM_ALARMOUT_DELAYTIME		GSR_CONFIG_ALARM_ALARMOUT+1
#define GSR_CONFIG_ALARM_ALARMOUT_NAME			GSR_CONFIG_ALARM_ALARMOUT+2
#define GSR_CONFIG_ALARM_ALARMOUT_ALLCHN		GSR_CONFIG_ALARM_ALARMOUT+3


#define GSR_CONFIG_ALARM_ALARMOUT_SCH			GSR_CONFIG_ALARM_ALARMOUT+0x10
#define GSR_CONFIG_ALARM_ALARMOUT_SCH_CHN			GSR_CONFIG_ALARM_ALARMOUT_SCH+0
#define GSR_CONFIG_ALARM_ALARMOUT_SCH_SCH			GSR_CONFIG_ALARM_ALARMOUT_SCH+1
#define GSR_CONFIG_ALARM_ALARMOUT_SCH_COPYTO		GSR_CONFIG_ALARM_ALARMOUT_SCH+2

#define GSR_CONFIG_ALARM_ALARMOUT_SOUNDALARM		GSR_CONFIG_ALARM_ALARMOUT+0x20
#define GSR_CONFIG_ALARM_ALARMOUT_SOUNDALARM_ALARM	GSR_CONFIG_ALARM_ALARMOUT_SOUNDALARM+0
#define GSR_CONFIG_ALARM_ALARMOUT_SOUNDALARM_DELAY	GSR_CONFIG_ALARM_ALARMOUT_SOUNDALARM+1

//yaogang modify 20141010
#define GSR_CONFIG_ALARM_IPCEXTSENSOR_ALARMDEAL_ALARMDEAL_BASE 				GSR_CONFIG_ALARM_BASE+0xB0
#define GSR_CONFIG_ALARM_IPCEXTSENSOR_ALARMDEAL_ALARMDEAL_BASE_SOUND 		GSR_CONFIG_ALARM_IPCEXTSENSOR_ALARMDEAL_ALARMDEAL_BASE+0
#define GSR_CONFIG_ALARM_IPCEXTSENSOR_ALARMDEAL_ALARMDEAL_BASE_BIGVIDEO 	GSR_CONFIG_ALARM_IPCEXTSENSOR_ALARMDEAL_ALARMDEAL_BASE+1
#define GSR_CONFIG_ALARM_IPCEXTSENSOR_ALARMDEAL_ALARMDEAL_BASE_TIRGGER 	GSR_CONFIG_ALARM_IPCEXTSENSOR_ALARMDEAL_ALARMDEAL_BASE+2
#define GSR_CONFIG_ALARM_IPCEXTSENSOR_ALARMDEAL_ALARMDEAL_BASE_MAIL 		GSR_CONFIG_ALARM_IPCEXTSENSOR_ALARMDEAL_ALARMDEAL_BASE+3
#define GSR_CONFIG_ALARM_IPCEXTSENSOR_ALARMDEAL_ALARMDEAL_BASE_SNAP 		GSR_CONFIG_ALARM_IPCEXTSENSOR_ALARMDEAL_ALARMDEAL_BASE+4 // bitmap

#define GSR_CONFIG_ALARM_IPCEXTSENSOR_ALARMDEAL_ALARMDEAL_TRIGGER 			GSR_CONFIG_ALARM_IPCEXTSENSOR_ALARMDEAL_ALARMDEAL_BASE+5

#define GSR_CONFIG_ALARM_IPCEXTSENSOR_ALARMDEAL_ALARMDEAL_LINKAGE 			GSR_CONFIG_ALARM_IPCEXTSENSOR_ALARMDEAL_ALARMDEAL_BASE+6
#define GSR_CONFIG_ALARM_IPCEXTSENSOR_ALARMDEAL_ALARMDEAL_LINKAGE_TYPE 	GSR_CONFIG_ALARM_IPCEXTSENSOR_ALARMDEAL_ALARMDEAL_BASE+7
#define GSR_CONFIG_ALARM_IPCEXTSENSOR_ALARMDEAL_ALARMDEAL_LINKAGE_INDEX 	GSR_CONFIG_ALARM_IPCEXTSENSOR_ALARMDEAL_ALARMDEAL_BASE+8

#define GSR_CONFIG_ALARM_IPCEXTSENSOR_SCH			GSR_CONFIG_ALARM_IPCEXTSENSOR_ALARMDEAL_ALARMDEAL_BASE+9
#define GSR_CONFIG_ALARM_IPCEXTSENSOR_SCH_CHN		GSR_CONFIG_ALARM_IPCEXTSENSOR_ALARMDEAL_ALARMDEAL_BASE+10
#define GSR_CONFIG_ALARM_IPCEXTSENSOR_SCH_SCH		GSR_CONFIG_ALARM_IPCEXTSENSOR_ALARMDEAL_ALARMDEAL_BASE+11
#define GSR_CONFIG_ALARM_IPCEXTSENSOR_SCH_COPYTO	GSR_CONFIG_ALARM_IPCEXTSENSOR_ALARMDEAL_ALARMDEAL_BASE+12


#define GSR_CONFIG_ALARM_EXIT			GSR_CONFIG_ALARM_ALARMOUT+0xF0


//-------------------------------network
#define GSR_CONFIG_NETWORK_BASE			GSR_CONFIG_BASE+0x500
#define GSR_CONFIG_NETWORK_BASICINFO	GSR_CONFIG_NETWORK_BASE+0
#define GSR_CONFIG_NETWORK_HTTPPORT 	GSR_CONFIG_NETWORK_BASE+0
#define GSR_CONFIG_NETWORK_SERVERPORT 	GSR_CONFIG_NETWORK_BASE+1
#define GSR_CONFIG_NETWORK_NULL 		GSR_CONFIG_NETWORK_BASE+2
#define GSR_CONFIG_NETWORK_DHCP 		GSR_CONFIG_NETWORK_BASE+3
#define GSR_CONFIG_NETWORK_IP 			GSR_CONFIG_NETWORK_BASE+4
#define GSR_CONFIG_NETWORK_MASK 		GSR_CONFIG_NETWORK_BASE+5
#define GSR_CONFIG_NETWORK_GATEWAY 		GSR_CONFIG_NETWORK_BASE+6
#define GSR_CONFIG_NETWORK_FIRSTDNS 		GSR_CONFIG_NETWORK_BASE+7
#define GSR_CONFIG_NETWORK_SECONDDNS 		GSR_CONFIG_NETWORK_BASE+8


#define GSR_CONFIG_NETWORK_PPPOE		GSR_CONFIG_NETWORK_BASE+0x10
#define GSR_CONFIG_NETWORK_USER			GSR_CONFIG_NETWORK_PPPOE+1
#define GSR_CONFIG_NETWORK_PASSWD		GSR_CONFIG_NETWORK_PPPOE+2


#define GSR_CONFIG_NETWORK_NETSTREAM			GSR_CONFIG_NETWORK_BASE+0x20
#define GSR_CONFIG_NETWORK_NETSTREAM_RESOLUTION 	GSR_CONFIG_NETWORK_NETSTREAM+0
#define GSR_CONFIG_NETWORK_NETSTREAM_FRAMERATE 		GSR_CONFIG_NETWORK_NETSTREAM+1
#define GSR_CONFIG_NETWORK_NETSTREAM_VBRCBR	 	GSR_CONFIG_NETWORK_NETSTREAM+2
#define GSR_CONFIG_NETWORK_NETSTREAM_QUALITY	 	GSR_CONFIG_NETWORK_NETSTREAM+3
#define GSR_CONFIG_NETWORK_NETSTREAM_BITRATE	 	GSR_CONFIG_NETWORK_NETSTREAM+4
#define GSR_CONFIG_NETWORK_NETSTREAM_CHN	 	GSR_CONFIG_NETWORK_NETSTREAM+6

#define GSR_CONFIG_NETWORK_SMTP	 	GSR_CONFIG_NETWORK_BASE+0x30
#define GSR_CONFIG_NETWORK_SMTP_SMTP		GSR_CONFIG_NETWORK_SMTP+0
#define GSR_CONFIG_NETWORK_SMTP_PORT		GSR_CONFIG_NETWORK_SMTP+1
#define GSR_CONFIG_NETWORK_SMTP_SSL		GSR_CONFIG_NETWORK_SMTP+2
#define GSR_CONFIG_NETWORK_SMTP_SENDADDR		GSR_CONFIG_NETWORK_SMTP+3
#define GSR_CONFIG_NETWORK_SMTP_PASSWD		GSR_CONFIG_NETWORK_SMTP+4
#define GSR_CONFIG_NETWORK_SMTP_RCVADDR1		GSR_CONFIG_NETWORK_SMTP+5
#define GSR_CONFIG_NETWORK_SMTP_RCVADDR2		GSR_CONFIG_NETWORK_SMTP+6
#define GSR_CONFIG_NETWORK_SMTP_RCVADDR3		GSR_CONFIG_NETWORK_SMTP+7


#define GSR_CONFIG_NETWORK_ADVANCE	 	GSR_CONFIG_NETWORK_BASE+0x40
#define GSR_CONFIG_NETWORK_ADVANCE_0	 	GSR_CONFIG_NETWORK_ADVANCE+0
#define GSR_CONFIG_NETWORK_ADVANCE_ATTACHIMGAMOUNT	 	GSR_CONFIG_NETWORK_ADVANCE+1
#define GSR_CONFIG_NETWORK_ADVANCE_SNAPINTVL	 	GSR_CONFIG_NETWORK_ADVANCE+2

#define GSR_CONFIG_NETWORK_ADVANCE_DDNSDOMAIN	 	GSR_CONFIG_NETWORK_BASE+0x60
#define GSR_CONFIG_NETWORK_ADVANCE_DDNSUPINTVL	 	GSR_CONFIG_NETWORK_ADVANCE_DDNSDOMAIN+1


//-------------------------------user
#define GSR_CONFIG_USER_BASE				GSR_CONFIG_BASE+0x600


//-------------------------------ptz
#define GSR_CONFIG_PTZ_BASE					GSR_CONFIG_BASE+0x700
#define GSR_PTZCONFIG_BASE					GSR_CONFIG_PTZ_BASE
#define GSR_PTZCONFIG_SERIAL_BASE			GSR_PTZCONFIG_BASE+0
#define GSR_PTZCONFIG_SERIAL_ENABLE			GSR_PTZCONFIG_BASE+1
#define GSR_PTZCONFIG_SERIAL_ADDRESS			GSR_PTZCONFIG_BASE+2
#define GSR_PTZCONFIG_SERIAL_BPS			GSR_PTZCONFIG_BASE+3
#define GSR_PTZCONFIG_SERIAL_PROTOCOL			GSR_PTZCONFIG_BASE+4
#define GSR_PTZCONFIG_SERIAL_ALLCHECK			GSR_PTZCONFIG_BASE+5

// GSR_PTZCONFIG_ADVANCECONFIG_BASE
#define GSR_PTZCONFIG_ADVANCECONFIG_BASE				GSR_PTZCONFIG_BASE+0x800-1
// GSR_PTZCONFIG_ADVANCECONFIG_PRESET_BASE
#define GSR_PTZCONFIG_ADVANCECONFIG_PRESET_BASE			GSR_PTZCONFIG_ADVANCECONFIG_BASE+0
#define GSR_PTZCONFIG_ADVANCECONFIG_PRESET_ENABLE			GSR_PTZCONFIG_ADVANCECONFIG_PRESET_BASE+1
#define GSR_PTZCONFIG_ADVANCECONFIG_PRESET_ID			GSR_PTZCONFIG_ADVANCECONFIG_PRESET_BASE+2

// GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_BASE
#define GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_BASE		GSR_PTZCONFIG_ADVANCECONFIG_BASE+0x100
#define GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_CRUISEPOINT_ID		GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_BASE+1
#define GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_CRUISEPOINT_NAME		GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_BASE+2
#define GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_CRUISEPOINT_PRESET	GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_BASE+3
#define GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_CRUISEPOINT_RATE		GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_BASE+4
#define GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_CRUISEPOINT_TIME		GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_BASE+5


// GSR_PTZCONFIG_ADVANCECONFIG_TRACK_BASE
#define GSR_PTZCONFIG_ADVANCECONFIG_TRACK_BASE			GSR_PTZCONFIG_ADVANCECONFIG_BASE+0x200


//-------------------------------advance
#define GSR_CONFIG_ADVANCE			GSR_CONFIG_BASE+0x800
#define GSR_ADVANCECONFIG_BASE		GSR_CONFIG_ADVANCE

#define GSR_ADVANCECONFIG_IMPORTEXPORT		GSR_ADVANCECONFIG_BASE+0
#define GSR_ADVANCECONFIG_IMPORTEXPORT_REFRESH		GSR_ADVANCECONFIG_IMPORTEXPORT+1


// === search ===
#define GSR_SEARCH_BASE			0x1000
// === backup ===
#define GSR_BACKUP_BASE			0x2000
// === info ===
#define GSR_INFO_BASE			0x3000
//-------------------------------system
#define GSR_INFO_SYSTEM_BASE		GSR_INFO_BASE+0
#define GSR_INFO_SYSTEM_DEVICENAME		GSR_INFO_SYSTEM_BASE+1
#define GSR_INFO_SYSTEM_DEVICENUM		GSR_INFO_SYSTEM_BASE+2
#define GSR_INFO_SYSTEM_HARDWAREVERSION		GSR_INFO_SYSTEM_BASE+3
#define GSR_INFO_SYSTEM_SCMVERSION		GSR_INFO_SYSTEM_BASE+4
#define GSR_INFO_SYSTEM_KERNELVERSION		GSR_INFO_SYSTEM_BASE+5
#define GSR_INFO_SYSTEM_ROMVERSION		GSR_INFO_SYSTEM_BASE+6
#define GSR_INFO_SYSTEM_RELEASEDATE		GSR_INFO_SYSTEM_BASE+7

#define GSR_INFO_DISK_BASE		GSR_INFO_BASE+0x180
#define GSR_INFO_DISK_NO		GSR_INFO_DISK_BASE+1
#define GSR_INFO_DISK_TYPE		GSR_INFO_DISK_BASE+2
#define GSR_INFO_DISK_CAPACITY		GSR_INFO_DISK_BASE+3
#define GSR_INFO_DISK_FREE		GSR_INFO_DISK_BASE+4
#define GSR_INFO_DISK_STATE		GSR_INFO_DISK_BASE+5
#define GSR_INFO_DISK_ATTRIBUTE		GSR_INFO_DISK_BASE+6
#define GSR_INFO_DISK_SOURCE		GSR_INFO_DISK_BASE+7
#define GSR_INFO_DISK_RSV1		GSR_INFO_DISK_BASE+8
#define GSR_INFO_DISK_RSV2		GSR_INFO_DISK_BASE+9
#define GSR_INFO_DISK_IMAGE		GSR_INFO_DISK_BASE+10
#define GSR_INFO_DISK_TOTAL		GSR_INFO_DISK_BASE+11
#define GSR_INFO_DISK_USED		GSR_INFO_DISK_BASE+12


//-------------------------------event
#define GSR_INFO_EVENT_BASE		GSR_INFO_BASE+0x100

//-------------------------------syslog
#define GSR_INFO_SYSLOG_BASE		GSR_INFO_BASE+0x200

//------------------------------- netinfo
#define GSR_INFO_NETINFO_BASE		GSR_INFO_BASE+0x300

//------------------------------- online
#define GSR_INFO_ONLINE_BASE		GSR_INFO_BASE+0x400
#define GSR_INFO_ONLINE_USERNAME	GSR_INFO_ONLINE_BASE+1
#define GSR_INFO_ONLINE_IP			GSR_INFO_ONLINE_BASE+2
#define GSR_INFO_ONLINE_STATE		GSR_INFO_ONLINE_BASE+3

// exit
#define GSR_INFO_EXIT_BASE			GSR_INFO_BASE+0x500


// === alarm ===
#define GSR_ALARM_BASE				0x4000
#define GSR_MANUALALARM_BASE		GSR_ALARM_BASE
#define GSR_MANUALALARM_ALLCHECK	GSR_MANUALALARM_BASE+1
#define GSR_MANUALALARM_ALARMNAME	GSR_MANUALALARM_BASE+2
#define GSR_MANUALALARM_IP			GSR_MANUALALARM_BASE+3
#define GSR_MANUALALARM_ALARMPROCESS	GSR_MANUALALARM_BASE+4

// === disk ===
#define GSR_DISK_BASE			0x5000


// === resource ===
#define GSR_RESOURCE_BASE		0x6000


// === logoff ===
#define GSR_LOGOFF_BASE			0x7000


// === shutdown ===
#define GSR_SHUTDOWN_BASE		0x8000

// === desktop ===
#define GSR_DESKTOP_BASE		0x9000
#define GSR_DESKTOP_SYSTIME		GSR_DESKTOP_BASE+1
#define GSR_DESKTOP_CHNNAME		GSR_DESKTOP_BASE+2

#define GSR_DESKTOP_VIDEO_ATTRIBUTE		GSR_DESKTOP_BASE+3
#define GSR_DESKTOP_VIDEO_ATTRIBUTE_CHN		GSR_DESKTOP_VIDEO_ATTRIBUTE+0
#define GSR_DESKTOP_VIDEO_ATTRIBUTE_BRIGHTNESS		GSR_DESKTOP_VIDEO_ATTRIBUTE+1
#define GSR_DESKTOP_VIDEO_ATTRIBUTE_CHROMA		GSR_DESKTOP_VIDEO_ATTRIBUTE+2
#define GSR_DESKTOP_VIDEO_ATTRIBUTE_SATURATION		GSR_DESKTOP_VIDEO_ATTRIBUTE+3
#define GSR_DESKTOP_VIDEO_ATTRIBUTE_CONTRAST		GSR_DESKTOP_VIDEO_ATTRIBUTE+4
#define GSR_DESKTOP_VIDEO_ATTRIBUTE_DEFAULT		GSR_DESKTOP_VIDEO_ATTRIBUTE+5

#define GSR_DESKTOP_VIDEO_ATTRIBUTE_VO_BRIGHTNESS		GSR_DESKTOP_VIDEO_ATTRIBUTE+6
#define GSR_DESKTOP_VIDEO_ATTRIBUTE_VO_CHROMA		GSR_DESKTOP_VIDEO_ATTRIBUTE+7
#define GSR_DESKTOP_VIDEO_ATTRIBUTE_VO_SATURATION		GSR_DESKTOP_VIDEO_ATTRIBUTE+8
#define GSR_DESKTOP_VIDEO_ATTRIBUTE_VO_CONTRAST		GSR_DESKTOP_VIDEO_ATTRIBUTE+9



void GetVideoFormatList( SValue* psValueList, int* nRealNum, int nMaxNum);
void GetMaxUserList( SValue* psValueList, int* nRealNum, int nMaxNum);
void GetOutputList( SValue* psValueList, int* nRealNum, int nMaxNum);

void GetLanguageList( SValue* psValueList, int* nRealNum, int nMaxNum);
void GetScreenSaverList( SValue* psValueList, int* nRealNum, int nMaxNum);

//csp modify
void GetLockTimeList(SValue* psValueList, int* nRealNum, int nMaxNum);

#ifdef __cplusplus
extern "C" {
#endif
void GetDWellSwitchTimeList( SValue* psValueList, int* nRealNum, int nMaxNum);
void GetDWellSwitchPictureList( SValue* psValueList, int* nRealNum, int nMaxNum);
//yaogang modify 20141217
void GetSGSysNameList( SValue* psValueList, int* nRealNum, int nMaxNum);
void GetSGSysTypeList( SValue* psValueList, int* nRealNum, int nMaxNum);
void GetSGMaintainTypeList( SValue* psValueList, int* nRealNum, int nMaxNum);
void GetSGTestTypeList( SValue* psValueList, int* nRealNum, int nMaxNum);



#ifdef __cplusplus
}
#endif

int GetVideoFormat();
int GetVideoFormatReal();
int GetMaxUser();
int GetOutput();
void GetVgaResolution(int* w, int* h);
int GetLanguage();
int GetLanguageIndex();
int GetScreenSaver();

//csp modify
int GetLockTimeIndex();
int SetLockTimeIndex(int index);

int GetDWellSwitchTimeIndex();
int SetDWellSwitchTimeIndex(int index);
int GetDWellSwitchPictureIndex();
int SetDWellSwitchPictureIndex(int index);



void GetDevId(char* pID, int nMaxLen);
void GetDevId0(char* pID, int nMaxLen);
void GetDevName(char* pName, int nMaxLen);
s32 GetDvrVersionModel(u32 flag, char *sDvr_info,  char *sDvr_Model);
int GetAuthCheck();
int GetTimeDisplayCheck();
//yaogang modify 20140918
int GetChnKbpsDisplayCheck();

int GetShowGuide();

int GetVideoFormatDefault();
int GetMaxUserDefault();
int GetOutputDefault();
int GetLanguageDefault();
int GetScreenSaverDefault();
int GetShowGuideDefault();

//csp modify
int GetLockTimeIndexDefault();

int GetDWellSwitchTimeDefault();
int GetDWellSwitchPictureDefault();

void GetDevIdDefault(char* pID, int nMaxLen);
int GetDevNameDefault(char* pName, int nMaxLen);
int GetAuthCheckDefault();
int GetTimeDisplayCheckDefault();
//yaogang modify 20140918
int GetChnKbpsDisplayCheckDefault();


void GetCruiseLineList(int id,SValue* psValueList, int* nRealNum, int nMaxNum);
void GetPresetList(int id, int pathNO,SValue* psValueList, int* nRealNum, int nMaxNum);

void GetMainTainTypeList( SValue* psValueList, int* nRealNum, int nMaxNum);
void GetWeekList( SValue* psValueList, int* nRealNum, int nMaxNum);

void GetProductNumber(s8* pProduct);
//int Pthread_Create(pthread_t* pid, void* attr, void* func, void* para);//csp modify 20121222
void SetDevFrontBoard(CDevFrontboard* board);
//void RunThread(u8 id);//csp modify 20121222

void SetSystemLockStatus(u8 flag);  //cw_lock
void GetSystemLockStatus(u8* flag);




/*****   basic config --Date & Time *****/
void GetDateFormatList( SValue* psValueList, int* nRealNum, int nMaxNum);
void GetTimeFormatList( SValue* psValueList, int* nRealNum, int nMaxNum);
void GetTimeZoneList( SValue* psValueList, int* nRealNum, int nMaxNum);

int GetTimeZoneOffset(int index);

int GetDateFormat();
int GetTimeFormat();
int GetTimeZone();

int GetNetworkSyscCheck();
void GetNtpServer(char* pServer, int nMaxLen);

int GetTimeSyncToIPC();

void GetSysDate(SYSTEM_TIME *p);
void GetSysTime(SYSTEM_TIME *p);

void GetSysDate_TZ(SYSTEM_TIME *p);
void GetSysTime_TZ(SYSTEM_TIME *p);

int GetDateFormatDefault();
int GetTimeFormatDefault();
int GetTimeZoneDefault();

int GetNetworkSyscCheckDefault();
void GetNtpServerDefault(char* pServer, int nMaxLen);

int GetTimeSyncToIPCDefault();

int SetMacAddress(char *pBuf);

/*****   record config --Enable *****/
int GetRecordEnable(int nCh);
int GetAudioEnable(int nCh);
int GetEnableAllCheck();

int GetRecordEnableDefault(int nCh);
int GetAudioEnableDefault(int nCh);




/*****   record config --CodeStream*****/
void GetResoList( SValue* psValueList, int* nRealNum, int nMaxNum);
void GetResoList2(SValue* psValueList, int* nRealNum, int nMaxNum);//cw_9508S
void GetResoListH_NUM(int* nRealNum);
void GetFrameRateList(int nResol, SValue* psValueList, int* nRealNum, int nMaxNum);
void GetEncodeList( SValue* psValueList, int* nRealNum, int nMaxNum);
void GetQualityList(SValue* psValueList, int* nRealNum, int nMaxNum);
void GetBitrateList(SValue* psValueList, int* nRealNum, int nMaxNum);
int GetFramerate(int nResol,int nCh);
int GetEncode(int nCh);
int GetQuality(int nCh);
int GetBitrate(int nCh);
char* GetCodeStreamLeft();
int GetCodeStreamAllCheck();

int GetReso(int nCh);

//csp modify 20130504
int VerifyReso(int nCh, int idx);

int GetResoDefault(int nCh);
int GetFramerateDefault(int nResol,int nCh);
int GetEncodeDefault(int nCh);
int GetQualityDefault(int nCh);
int GetBitrateDefault(int nCh);

/*****   record config --RecordTime *****/
void GetPreAlarmTimeList(SValue* psValueList, int* nRealNum, int nMaxNum);
void GetDelayAlarmTimeList(SValue* psValueList, int* nRealNum, int nMaxNum);
void GetExpirationList(SValue* psValueList, int* nRealNum, int nMaxNum);
int GetTimeBeforeAlarm(int nCh);
int GetTimeAfterAlarm(int nCh);
int GetExpiration(int nCh);
int GetRecordTimeAllCheck();

int GetTimeBeforeAlarmDefault(int nCh);
int GetTimeAfterAlarmDefault(int nCh);
int GetExpirationDefault(int nCh);

/*****   record config --Osd *****/
int GetChnNameCheck(int nCh);
int GetTimeStampCheck(int nCh);
int GetOsdAllCheck();

int GetChnNameCheckDefault(int nCh);
int GetTimeStampCheckDefault(int nCh);

/*****   record config --CyclingRec *****/
int GetCyclingRecCheck();

int GetCyclingRecCheckDefault();

/*****   live config --live *****/
void GetChnName(int nCh, char* pName, int nMaxLen);
void GetPtzAddr(int nCh, char* addr, int nMaxLen);
int GetDisplayNameCheck(int nCh);
int GetLiveAllCheck();
int GetLiveRecStateCheck();

int GetChnNameDefault(int nCh, char* pName, int nMaxLen);
int GetDisplayNameCheckDefault(int nCh);
int GetLiveRecStateCheckDefault();
void SaveDefaultPtzConfig();



/*****   live config --MainMonitor & Spot *****/
void GetSplitList(SValue* psValueList, int* nRealNum, int nMaxNum);
void GetChnList(SValue* psValueList, int* nRealNum, int nMaxNum);
void GetDwellTimeList(SValue* psValueList, int* nRealNum, int nMaxNum);
int GetMainSplit();
int GetSpotSplit();
int GetMainDwellTime();
int GetSpotDwellTime();
int GetRectOsd(int nChn,int maxNum,int* realNum,SRect* osdRect);
int SetRectOsd(int nChn,int realNum,SRect* osdRect);

int GetAudioOutInfo(u8* pAudioOutChn, u8* pVolume, u8* pMute);
int SetAudioOutInfo(u8 nAudioOutChn, u8 nVolume, u8 nMute);

s32 GetIsPatrolPara(u32 bDefault,SBizCfgPatrol *psPara, u8 nId);
s32 SetIsPatrolPara(SBizCfgPatrol *psPara, u8 nId);

/*****   record schedule --Time record *****/
void GetCopyChnList(SValue* psValueList, int* nRealNum, int nMaxNum);
void GetTimerSchedule(int nChn, uint* pMask, int nNum);
void GetMotionAlarmSchedule(int nChn, uint* pMask, int nNum);
void GetSensorAlarmSchedule(int nChn, uint* pMask, int nNum);

/*****   Alarm config -- *****/

// Base
void GetSensorAlarmBaseTypeList(SValue* psValueList, int* nRealNum, int nMaxNum);
int GetSensorAlarmBaseEnableCheckCur(int ch); // 0-all, >0-ch
int GetSensorAlarmBaseTypeCur(int ch);
void GetSensorAlarmBaseNameCur(int ch, char* psValue);
int GetSensorAlarmBaseChnAllCheck();

// AlarmDeal
void GetSensorAlarmDealDelayTimeList(SValue* psValueList, int* nRealNum, int nMaxNum);
int GetSensorAlarmDealDelayTimeCur(int ch); // 0-all, >0-ch
int GetSensorAlarmDealChnAllCheck();
// AlarmDeal-set-BaseAlarm
int GetAlarmDealBaseAlarmSoundAlarmCheckCur(int ch);
int GetAlarmDealBaseAlarmBigDealAlarmList(SValue* psValueList, int* nRealNum, int nMaxNum);
int GetAlarmDealBaseAlarmBigDealAlarmCur(char* psValueList, int ch);
int GetAlarmDealBaseAlarmTriggerAlarmCur(int ch);
int GetAlarmDealBaseAlarmMailCur(int ch);
int GetAlarmDealBaseAlarmSnapAllCheckCur(); // ???????????一个通道设置关联所有通道??
int GetAlarmDealBaseAlarmSnapCheckCur(int ch);
// AlarmDeal-set-TiggerRec
int GetAlarmDealTriggerRecordAllCheckCur(); // ???????????一个通道设置关联所有通道??
int GetAlarmDealTriggerRecordCheckCur(int ch);
// AlarmDeal-set-LinkageAlarm
int GetAlarmDealLinkageAlarmTypeList(SValue* psValueList, int* nRealNum, int nMaxNum); 
int GetAlarmDealLinkageAlarmTypeCur(int ch);
int GetAlarmDealLinkageAlarmIndexList(SValue* psValueList, int* nRealNum, int nMaxNum);
int GetAlarmDealLinkageAlarmIndexCur(int ch);

// SCH
int GetSensorAlarmSCHChnList(SValue* psValueList, int* nRealNum, int nMaxNum);
int GetSensorAlarmSCHChnCur(int ch, char* szCur);
int GetSensorAlarmSCHCur(int ch, uint sch[]); // 7days
int GetSensorAlarmApplyToList(SValue* psValueList, int* nRealNum, int nMaxNum);
int GetSensorAlarmApplyToCur(int ch, char* szCur);

// === motiondetect ===
int GetMotionDetectAllChnCheck();
int GetMotionDetectEnableCheckCur(int ch); // 0 all, >0 other
int GetMotionDetectDelayTimeList(SValue* psValueList, int* nRealNum, int nMaxNum); // 0 all, >0 other
int GetMotionDetectDelayTimeCur(int ch, char* szDelayTime); // 0 all, >0 other
// AlarmDeal-set-BaseAlarm
int GetMotionDetectAlarmDealBaseAlarmSoundAlarmCheckCur(int ch); // 0 all, >0 other
int GetMotionDetectAlarmDealBaseAlarmBigVideoAlarmList(SValue* psValueList, int* nRealNum, int nMaxNum); // 0 all, >0 other
int GetMotionDetectAlarmDealBaseAlarmBigVideoAlarmCur(int ch, char* szCur); // 0 all, >0 other
int GetMotionDetectAlarmDealBaseAlarmTriggerAlarmCheckCur(int ch); // 0 all, >0 other
int GetMotionDetectAlarmDealBaseAlarmMailCheckCur(int ch); // 0 all, >0 other

int GetMotionDetectAlarmDealBaseAlarmSnapAllCheckCur(); // ???????????一个通道设置关联所有通道??
int GetMotionDetectAlarmDealBaseAlarmSnapCheckCur(int ch);
// AlarmDeal-set-TiggerRec
int GetMotionDetectAlarmDealTriggerRecordAllCheckCur(); // ???????????一个通道设置关联所有通道??
int GetMotionDetectAlarmDealTriggerRecordCheckCur(int ch);
// AlarmDeal-set-LinkageAlarm
int GetMotionDetectAlarmDealLinkageAlarmTypeList(SValue* psValueList, int* nRealNum, int nMaxNum); 
int GetMotionDetectAlarmDealLinkageAlarmTypeCur(int ch);
int GetMotionDetectAlarmDealLinkageAlarmIndexList(SValue* psValueList, int* nRealNum, int nMaxNum);
int GetMotionDetectAlarmDealLinkageAlarmIndexCur(int ch);
// MotionDetect Motion Detect RgnSet
int GetMotionDetectRgnSetGridCur(int ch, uint64 tmp[]); // uint64 [64]
int GetMotionDetectRgnSetSenseCur(int ch ); // uint64 [64]
// SCH
int GetMotionDetectSCHChnList(SValue* psValueList, int* nRealNum, int nMaxNum);
int GetMotionDetectSCHChnCur(int ch, char* szCur);
int GetMotionDetectSCHCur(int ch, uint sch[]); // 7days
int GetMotionDetectApplyToList(SValue* psValueList, int* nRealNum, int nMaxNum);
int GetMotionDetectApplyToCur(int ch, char* szCur);
// === video loss ===
// video loss AlarmDeal-set-BaseAlarm
int GetVideoLossAlarmDealAllCheckCur(); // 0 all, >0 other

int GetVideoLossAlarmDealBaseAlarmSoundAlarmCheckCur(int ch); // 0 all, >0 other
int GetVideoLossAlarmDealBaseAlarmBigVideoAlarmList(SValue* psValueList, int* nRealNum, int nMaxNum); // 0 all, >0 other
int GetVideoLossAlarmDealBaseAlarmBigVideoAlarmCur(int ch, char* szCur); // 0 all, >0 other
int GetVideoLossAlarmDealBaseAlarmTriggerAlarmCheckCur(int ch); // 0 all, >0 other
int GetVideoLossAlarmDealBaseAlarmMailCheckCur(int ch); // 0 all, >0 other

int GetVideoLossAlarmDealBaseAlarmSnapAllCheckCur(); // ???????????一个通道设置关联所有通道??
int GetVideoLossAlarmDealBaseAlarmSnapCheckCur(int ch);

// AlarmDeal-set-LinkageAlarm
int GetVideoLossAlarmDealLinkageAlarmTypeList(SValue* psValueList, int* nRealNum, int nMaxNum); 
int GetVideoLossAlarmDealLinkageAlarmTypeCur(int ch);
int GetVideoLossAlarmDealLinkageAlarmIndexList(SValue* psValueList, int* nRealNum, int nMaxNum);
int GetVideoLossAlarmDealLinkageAlarmIndexCur(int ch);

// === other alarm ===
int GetOtherAlarmAlarmTypeList(SValue* psValueList, int* nRealNum, int nMaxNum);
int GetOtherAlarmAlarmTypeCur(int ch, char* szCur);  
int GetOtherAlarmDiskAlarmList(SValue* psValueList, int* nRealNum, int nMaxNum);
int GetOtherAlarmDiskAlarmCur(int ch, char* szCur);

int GetOtherAlarmSoundAlarmCheckCur();
int GetOtherAlarmTriggerAlarmCheckCur();
int GetOtherAlarmMailCheckCur(); 

// === AlarmOut ===
// alarm out
int GetAlarmOutAllChnCheckCur(); 
int GetAlarmOutNameList(SValue* psValueList, int* nRealNum, int nMaxNum);
int GetAlarmOutNameCur(int ch, char* szCur); // ch not used now
int GetAlarmOutDelayTimeList(SValue* psValueList, int* nRealNum, int nMaxNum);
int GetAlarmOutDelayTimeCur(int ch); // 0 all, 1 chn

// sch
int GetAlarmOutSCHChnList(SValue* psValueList, int* nRealNum, int nMaxNum);
//int GetAlarmOutSCHChnCur();
int GetAlarmOutSCHCur(int ch, uint sch[]); // 7days
int GetAlarmOutSCHApplyToList(SValue* psValueList, int* nRealNum, int nMaxNum);
int GetAlarmOutSCHApplyToCur();

// soundalarm
int GetAlarmOutSoundAlarmDelayTimeList(SValue* psValueList, int* nRealNum, int nMaxNum);
int GetAlarmOutSoundAlarmDelayTimeCur();
int GetAlarmOutSoundAlarmCheck(); 

int GetSensorAlarmBaseEnableCheckCurDefault(int ch); // 0-all, >0-ch
int GetSensorAlarmBaseTypeCurDefault(int ch);
int GetSensorAlarmBaseNameCurDefault(int ch, char* psValue);
int GetAlarmOutNameCurDefault(int ch, char* szCur); // ch not used now
int GetAlarmOutDelayTimeCurDefault(int ch); // 0 all, 1 chn
int GetAlarmOutSCHCurDefault(int ch, uint sch[]); // 7days
int GetAlarmOutSoundAlarmDelayTimeCurDefault();
int GetAlarmOutSoundAlarmCheckDefault();

int GetFrameRateVal(int nResol, int nIdx);

void GetSubResoList(SValue* psValueList, int* nRealNum, int nMaxNum);
void GetMobResoList(SValue* psValueList, int* nRealNum, int nMaxNum);


void GetVideoSrcTypeList(SValue* psValueList, int* nRealNum, int nMaxNum);
void GetVideoSrcIndexDefault(uchar* pIndex);
void GetVideoSrcIndex(uchar* pIndex);
int SetVideoSrcIndex(uchar* pIndex);

//u8 bizData_IsCbr( char *pItemStr );//csp modify

int GetEPortValue(int sel);//cw_ddns

typedef enum
{
	EM_GSR_CHKORIDX,
	EM_GSR_EDIT,
	EM_GSR_OSD,
	EM_GSR_COMBLIST,
	EM_GSR_SZLIST,
	EM_GSR_SCH,
	
} EM_GSR;

typedef enum
{
	EM_GSR_CTRL_CHK,
	EM_GSR_CTRL_BTN,
	EM_GSR_CTRL_COMB,
	EM_GSR_CTRL_EDIT,
	EM_GSR_CTRL_SLIDE,
	EM_GSR_CTRL_SZLIST,
	EM_GSR_CTRL_SZIPLIST,
	EM_GSR_CTRL_SCH,
	EM_GSR_CTRL_SCH_VAL,
	EM_GSR_CTRL_RGNSET,
	
} EM_GSR_CTRL;

typedef struct _sCombList
{
	s8 		psValueList[CBX_MAX][EDIT_MAX];
	u8	 	nRealNum;
	u8		max;
} SCombList;

typedef enum
{
	EM_STRUCT_SYSINFO,
} EM_STRUCT;

typedef struct _sGetScr
{
	EM_GSR type;
	uchar chn; // 0xff all, >=0 01234...
	union
	{
		SCombList sCombList;

		struct
		{
			uint sch[7];
		} sSCH;

		char* szValue;
		
		uint value; // checkbox 0/1, index >=0
	};
} SGetScr, PSGetScr;

//int GetScreenStrSel( uint nItemID, SGetScr* psGsr );
#if 1
int GetScrStrSelDo(
	u32 	nItemID, 
	EM_GSR 	type, 
	u8 		ch,
	u32 	max,
	s8* psValueList[],
	SGetScr* pGsr
);
#endif

/*
	get combox itemlist from cfg module
	get combox select from cfg module
	

	fill  combox with list
	set current combox select
*/
int GetScrStrInitComboxSelDo(
	void* pCtrl,
	u32 	nItemID, 
	EM_GSR 	type, 
	u8 		ch
);

/*
	get combox select from cfg module
	get edit content from cfg module
	get checkbox select from cfg module

	set current combox/checkbox select
*/
int GetScrStrSetCurSelDo(
	void* pCtrl,
	u32 	nItemID, 
	EM_GSR_CTRL 	type, 
	u8 		ch
);

/*
 get / set region 
 
*/
int GetRgnCurDo(
	u64* 	pRgn,
	u32 	nItemID, 
	EM_GSR_CTRL 	type, 
	u8 		ch
);

/*
	Save Parameters to config module
	 
*/
int SaveScrStr2Cfg(
	void* 			pVoid,
	u32 			nItemID, 
	EM_GSR_CTRL 	type, 
	u8 				ch
);

u8 GetCurRecStatus(u8 nChn);
int EventRecordDeal(SBizRecStatus* recStatus);
int EventMDDeal(SBizAlarmStatus* sBizAlaStatus);
int EventSensorAlarmDeal(SBizAlarmStatus* sBizAlaStatus);
int EventIPCExtSensorAlarmDeal(SBizAlarmStatus* sBizAlaStatus);
int EventIPCCoverAlarmDeal(SBizAlarmStatus* sBizAlaStatus);


u8 GetVideoMainNum(void);
u8 GetSenSorNum(void);
u8 GetAlarmOutNum();
u8 GetAudioNum(void);
//zlb20111117 去掉多余代码
//void InitStrChn();
//void SetChnString(int chn,char* str);
//void SetChnNameEnable(int chn,int enable);

s32 SwitchPreviewEx(EMBIZPREVIEWMODE emMode, u8 nModePara);
void PreviewToNextPic(void);
void PreviewToLastPic(void);
void PreviewToNextMod(void);
void PreviewToLastMod(void);
s32 PreviewElecZoom(int flag, s32 chn, SBizPreviewElecZoom* stCapRect);
s32 PreviewPbElecZoom(int flag, s32 chn, SBizPreviewElecZoom* stCapRect);
s32 PreviewSnapSet(int flag, SBizPreviewElecZoom* stMenuRect);

void ControlPatrol(u8 nEnable);
void ControlMute(u8 nEnable);
void ControlAudioOutChn(u8 nChn);
void ControlVolume(u8 nVolume);
//直接设置预览图像参数而不保存配置
s32 PreviewSetImageColor(u8 nChn, SBizPreviewImagePara* psImagePara);
s32 PlayBackSetImageColor(SBizVoImagePara* psVoImagePara);

int BizEventCB(SBizEventPara* pSBizEventPara);
void StopElecZoom(void);
	
void debug_trace_err(char* str);

u32 MakeTimeForBackup( int y, int m, int d, int h, int min, int s);

void GetTimeForBackup( u32 t, char* szTime );//时间和日期
void GetTimeForBackup2( u32 t, char* szTime );//只有时间
u32 GetLocalTimeForBackup();

s32 FxnKeycodeShut();//cw_shutdown
void RestartSystem();
#ifdef __cplusplus
extern "C" {
#endif
EMBIZPREVIEWMODE GetCurPreviewMode_CW();
void SetCurPreviewMode_CW(EMBIZPREVIEWMODE mode);
void SwitchPreview(EMBIZPREVIEWMODE emMode, u8 nModePara);
EMBIZPREVIEWMODE bizData_GetPreviewMode();
void bizData_GetPreviewPara(EMBIZPREVIEWMODE * PreviewMode, u8* ModePara);

void RefreshAllStatus();
void Biz_CloseGuide();


#ifdef __cplusplus
}
#endif


void SetPlayBakStatus(int flag);
u8 GetPlayBakStatus();

s32 BizGetPlayBackFileFormat();
void BizSetPlayBackFileFormat(u8 flag);

typedef enum {
    EM_PAGE_DESKTOP = 0,
    EM_PAGE_START,
    EM_PAGE_DISKMGR,
    EM_PAGE_UPDATE,
    EM_PAGE_PLAYBACK,
    EM_PAGE_LIVECFG,
    EM_PAGE_BACKUP,
    EM_PAGE_NETCFG,
    EM_PAGE_USERMANAGE,
    EM_PAGE_BASICCFG,
    EM_PAGE_RECORD,
    EM_PAGE_GUIDE,
    EM_PAGE_ALARMLIST,
    EM_PAGE_SNAP,//shixin
    EM_PAGE_SG_PLATFORM,
    EM_PAGE_NUM,
} EM_PAGE;

typedef enum {		 //cw_backup
    EM_BACKUP_FILE= 0,
    EM_BACKUP_TIME,
} EM_BACKUPTYPE;

void SetPage(EM_PAGE emPage, CPage* pPage);
CPage** GetPage();
void BoardGetReUsedKey(u8 key, u8* pKeyNew);

u32 GetGUILoginedUserID();
void SetGUILoginedUserID(u32 ID);
char* GetGUILoginedUserName();
void SetGUILoginedUserName(char* name);
u8 IsGUIUserLogined();
void SetGUIUserIsLogined(u8 logined);

void ResetRecState(int value);

extern BOOL bGUIStart;
//void bizData_CreateCDC();

s32 BizGUiWriteLog(s8 master, s8 slave);
s32 BizGUiWriteLogWithChn(u8 chn, s8 master, s8 slave);

void GetSemForDC(sem_t** sem);


char* bizData_inet_ntoa( u32 nIP );
void BizNetEventCB(SBizNetStateNotify* pNotify);

void WaitForSem();
void ReleaseSem();
int IsOpen();
int SetOpen(int open);
void SetDwellStartFlag(u8 flag);
u8 GetDwellStartFlag();

CDevMouse* GetDevMouse();


//u64 GetTimeTick();

void delDir(char *path);
void rmMemData();

#define SCROLLBAR	25

typedef struct 
{
	u8 nCol;
	u8 nRow;
} STabSize;

void bizData_SetLocales(CLocales* pLocale);
void bizData_GetLocales(CLocales** pLocale);

void RecLedCtrl(int flag,int type);

typedef enum
{
	EM_ALRM_DISK_FULL = 0,//1,
	EM_ALRM_DISK_WRERR,//读写出错
	EM_ALRM_DISK_LOST,//硬盘丢失
	EM_ALRM_DISK_NONE,//开机检测无硬盘
	
	EM_ALRM_NET_DISCONNECT,
	EM_ALRM_IP_CONFLICT,
	EM_ALRM_2G3G_STATUS,
	EM_ALRM_PSTN_STATUS,
	
	EM_ALRM_ALARM_INPUT,
	EM_ALRM_MOTION_DETECTION,
	EM_ALRM_VEDIO_LOST,
	EM_ALRM_ALARM_IPCEXT,
	EM_ALRM_ALARM_IPCCOVER,
	EM_ALRM_ALARM_485EXT,
	
	EM_ALRM_TYPES,//支持的报警种类，必须放在最后一项
}ALARMLISTTYPE;

void AlarmListAdd(u8 chanel,ALARMLISTTYPE cmd,u8 status);
void AlarmListDelete(u8 chanel,ALARMLISTTYPE cmd,u8 status);

void RefreshList(u8 page);
void RefreshAllStatus();


void StartTimer();
void StopTimer();

//yaogang modify 20141209
//深广平台参数

#if 0
typedef struct
{
	u8 TimeEnable;	//定时上传时间使能
	u8 hour;
	u8 min;
	u8 sec;
} SGTime;
typedef struct
{
	u8 PreTimes;		//前置时间
	u8 Interval;		//间隔时间
	u8 StillTimes;		//延录时间
	u8 AlarmInput;	//输入端子
	u32 IpcChn;		//16个IPC通道各占一位，0x1000 0000表示所有通道
} SAlarmPicCFG;
typedef struct
{
	SGTime Time1;		//定时上传时间1 
	SGTime Time2;		//定时上传时间2
	u32 IpcChn1;		//16个IPC通道各占一位，0x1000 0000表示所有通道
	u32 IpcChn2;		//16个IPC通道各占一位，0x1000 0000表示所有通道
} SDailyPicCFG;
typedef struct
{
	char AgentID[100];
	char CenterIP[100];
	u8 RunEnable;//运行状态启用
	u8 AlarmEnable;//报警联动启用
	u8 DailyPicEnable;//日常图像启用
	SAlarmPicCFG SAlarmPicParam;//报警图片配置参数
	SDailyPicCFG SDailyPicParam;//日常图片配置参数
} SGparam;
#endif
//yaogang modify 20141209
//深广平台参数
typedef struct
{
	char AgentID[100];
	char CenterIP[100];
	u8 RunEnable;//运行状态启用
	u8 AlarmEnable;//报警联动启用
	u8 DailyPicEnable;//日常图像启用
	u8 normal_exit;// 1: 系统正常关机0: 异常退出
} SGParam;

//报警图片配置，每个报警通道一个结构
//m路本地，n路IPC外部报警(m 16--n 32)
typedef struct
{
	u8 PreTimes;		//前置时间
	u8 Interval;		//间隔时间
	u8 StillTimes;		//延录时间
	u8 AlarmInput;	//输入端子
	u32 IpcChn;		//每个IPC通道各占一位，0xffff ffff表示所有通道
} SAlarmPicCFG;

//日常图片配置
typedef struct
{
	u8 TimeEnable;	//定时上传时间使能
	u8 hour;
	u8 min;
	u8 sec;
} SGTime;

typedef struct
{
	SBizSGTime Time1;		//定时上传时间1 
	SBizSGTime Time2;		//定时上传时间2
	u32 IpcChn1;		//每个IPC通道各占一位，0xffff ffff表示所有通道
	u32 IpcChn2;		//每个IPC通道各占一位，0xffff ffff表示所有通道
} SDailyPicCFG;

int bizData_GetSGparamDefault(SGParam *para);
int bizData_GetSGparam(SGParam *para);
int bizData_SaveSGparam(SGParam *para);

int bizData_GetSGAlarmParamDefault(int chn, SAlarmPicCFG *para);
int bizData_GetSGAlarmParam(int chn, SAlarmPicCFG *para);
int bizData_SaveSGAlarmParam(int chn, SAlarmPicCFG *para);

int bizData_GetSGDailyParamDefault(SDailyPicCFG *para);
int bizData_GetSGDailyParam(SDailyPicCFG *para);
int bizData_SaveSGDailyParam(SDailyPicCFG *para);


//报修系统类型
void GetRepairNameList( SValue* psValueList, int* nRealNum, int nMaxNum);
void GetRepairTypeList( SValue* psValueList, int* nRealNum, int nMaxNum);
void GetMaintainTypeList( SValue* psValueList, int* nRealNum, int nMaxNum);
void GetTestTypeList( SValue* psValueList, int* nRealNum, int nMaxNum);
void GetAlarmInputList( SValue* psValueList, int* nRealNum, int nMaxNum);

//yaogang modify 20150105
//snap param
typedef struct 
{
	u8 Enable;	//启用
	u8 Res;		//分辨率
	u8 Quality;	//图片质量
	u8 Interval;	//时间间隔
} SGuiSnapPara;
typedef struct 
{
	SGuiSnapPara TimeSnap;		//定时抓图
	SGuiSnapPara EventSnap;	//事件抓图
} SGuiSnapChnPara;

int bizData_SetSnapChnPara(int chn, SGuiSnapChnPara *para);
int bizData_GetSnapChnPara(int chn, SGuiSnapChnPara *para);
int bizData_GetSnapChnParaDefault(int chn, SGuiSnapChnPara *para);



#endif

