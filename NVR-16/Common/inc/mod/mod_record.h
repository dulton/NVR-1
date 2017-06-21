#ifndef _MOD_RECORD_H_
#define _MOD_RECORD_H_

#ifdef __cplusplus
extern "C" {
#endif
#include <sys/time.h>


/*************Ìá¹©¸øÒµÎñ²ãµÄ*****************/

typedef enum			//·Ö±æÂÊÃ¶¾Ù
{
	EM_REC_ENC_D1 = 0,		//D1
	EM_REC_ENC_4CIF,		//4CIF
	EM_REC_ENC_HALF_D1,		//HD1
	EM_REC_ENC_CIF,			//Cif
	EM_REC_ENC_QCIF,		//Qcif
	EM_REC_ENC_N_D1 = 10,	//NÖÆD1
	EM_REC_ENC_N_4CIF,		//NÖÆ4CIF
	EM_REC_ENC_N_HALF_D1,	//NÖÆHD1
	EM_REC_ENC_N_CIF,		//NÖÆCif
	EM_REC_ENC_N_QCIF,		//NÖÆQcif
	EM_REC_ENC_720P = 20,	//720P
	EM_REC_ENC_1080P,		//1080P
	EM_REC_ENC_960H,		//960H
	EM_REC_ENC_N_960H,		//NÖÆ960H//csp modify
}EMRECENCRESOL;

typedef struct mr_recordpara
{
	//u8 nChn;						//Í¨µÀºÅ
	u32 nBitRate;					//±àÂë±ÈÌØÂÊ(µ¥Î»Îªk)
	EMRECENCRESOL emResolution;		//±àÂë·Ö±æÂÊ
	u8 nFrameRate;					//±àÂëÖ¡ÂÊ
} SModRecordRecPara;

typedef enum				//ÒµÎñ²ã·¢ËÍÏûÏ¢ÀàÐÍÃ¶¾Ù
{
	EM_RECALARM_V_MOTION = 0,			//ÒÆ¶¯Õì²â
	EM_RECALARM_V_ALARM,				//±¨¾¯ÊäÈë
	EM_RECALARM_V_IPCEXTALARM,				//±¨¾¯ÊäÈë	
	EM_RECALARM_V_BIND,					//ÊÓÆµÕÚµ²
	EM_RECALARM_V_LOST,					//ÊÓÆµ¶ªÊ§
	EM_RECALARM_NETLINK_LOST,			//ÍøÂç¶ÏÁ¬
	EM_RECALARM_ABNORMAL,				//Òì³£
} EMRECALARMTYPE;

typedef enum					//Â¼Ïñ²¼·ÀÀàÐÍÃ¶¾Ù
{
	EM_REC_SCH_WEEK_DAY = 0,
	EM_REC_SCH_MONTH_DAY,
	EM_REC_SCH_EVERY_DAY,
	EM_REC_SCH_ONCE_DAY,
}EMRECSCHTYPE;

typedef struct mr_schtime
{
	u32 nStartTime;	//Ã¿¸öÊ±¼ä¶ÎµÄÆðÊ¼Ê±¼ä
	u32 nStopTime;	//½áÊøÊ±¼ä
}SModRecSchTime;

typedef enum
{
    EM_REC_MASK_MANUAL = (1 << 0),
    EM_REC_MASK_TIMING = (1 << 1),
    EM_REC_MASK_MOTION = (1 << 2),
    EM_REC_MASK_SENSORALARM = (1 << 3),
} EMRECTYPEMASK;

#define MAX_TIME_SEGMENTS 12	//Ã¿Ìì¿ÉÉèÖÃµÄÊ±¼ä¶ÎµÄ×î´óÊýÄ¿

typedef enum					//å½•åƒå¸ƒé˜²ç±»åž‹æžšä¸¾
{
	EM_REC_SCH_REC_TIMER = 0,
	EM_REC_SCH_REC_VMOTION,
	EM_REC_SCH_REC_ALARM,
}EMRECSCHRECTYPE;

typedef struct mr_schpara
{
	u8 nChn;										//é€šé“å·
	EMRECSCHTYPE nTimeType;							//å½•åƒå¸ƒé˜²çš„æ—¶é—´ç±»åž‹
	u32 nValue;										//å¯¹åº”ç±»åž‹ä¸‹çš„å–å€¼ï¼Œéœ€è¦æŒ‰ä½æ“ä½œï¼Œï¼ˆå¯ä»¥å®žçŽ°è¡¨ç¤º31å¤©ï¼‰
	EMRECSCHRECTYPE nRecType;					
	SModRecSchTime nSchTime[31][MAX_TIME_SEGMENTS];	//å„æ®µå¸ƒé˜²æ—¶é—´
}SModRecSchPara;

/*
typedef struct mr_schpara
{
	u8 nChn;										//Í¨µÀºÅ
	EMRECSCHTYPE nTimeType;							//Â¼Ïñ²¼·ÀµÄÊ±¼äÀàÐÍ
	u32 nValue;										//¶ÔÓ¦ÀàÐÍÏÂµÄÈ¡Öµ£¬ÐèÒª°´Î»²Ù×÷£¬£¨¿ÉÒÔÊµÏÖ±íÊ¾31Ìì£©
	u8 nRecType[31][MAX_TIME_SEGMENTS];				//Â¼ÏñÀàÐÍ(Î»ÔËËã£¬ÓÉµÍµ½¸ßÎ»·Ö±ð´ú±í
													//¶¨Ê±¡¢ÒÆ¶¯Õì²â¡¢±¨¾¯ÊäÈë¡¢ÕÚµ²¡¢¶ªÊ§¡¢.....¡¢ÆäËü)
	SModRecSchTime nSchTime[31][MAX_TIME_SEGMENTS];	//¸÷¶Î²¼·ÀÊ±¼ä
}SModRecSchPara;
*/

typedef struct rec_header
{
	u8	nChn;					//Ö¡µÄÍ¨µÀºÅ
	u8	nStreamType;				//ÂëÁ÷ÀàÐÍ£¨0:ÊÓÆµÁ÷¡¢1:ÊÓÒôÆµÁ÷¡¢2: snap£©
	u8	nMediaType; 				//±àÂë·½Ê½£¨0:H264¡¢10:PCMU¡¢11:ADPCM£©
	u8	nFrameType; 				//ÊÇ·ñ¹Ø¼üÖ¡(0:·ñ¡¢1:ÊÇ) FRAME_TYPE_P = 0,FRAME_TYPE_I = 1/3/5
	u32 nBitRate;					//±ÈÌØÂÊ
								//nFrameType ÔÚSnapÄ£¿éÓÃ×ö×¥Í¼ÀàÐÍEM_SNAP_TYPE
	EMRECENCRESOL emResolution; 	//±àÂë·Ö±æÂÊ
	u64 nPts;						//Ê±¼ä´Á£¨Î¢Ãî£©
								//nPts ÔÚSnapÄ£¿éÓÃ×östruct timeval {time_t tv_sec; suseconds_t tv_usec}; gettimeofday(); )
	u64 nTimeStamp; 				//Ö¡Ê±¼ä´Á£¨ºÁÃë£©
	u8* nDate;					//Ö¸ÏòÖ¡Êý¾ÝµÄÖ¸Õë
	u32 nDataLength;				//Ö¡Êý¾ÝµÄÕæÊµ³¤¶È
	u16 width;
	u16 height;
}SModRecRecordHeader;



typedef void (*FPMODRECSTATUS)(void* status);
typedef void* RecHddInfo;
typedef void (*FPMODRECEXCEPTION)(u8 nType);

s32 ModRecordInit(u8 nChnNum, SModRecordRecPara* para, FPMODRECSTATUS getrecstatus, RecHddInfo hddinfo, FPMODRECEXCEPTION RecExceptCB);
s32 ModRecordDeInit();
s32 ModRecordStartManual(u8 nChn);
s32 ModRecordStopManual(u8 nChn);
s32 ModRecordStop(u8 nChn);
s32 ModRecordStopAll();
s32 ModRecordPause(u8 nChn);
s32 ModRecordResume(u8 nChn);
s32 ModRecordSetPreTime(u8 nChn, u32 time);
s32 ModRecordSetDelayTime(u8 nChn, u32 time);
//s32 ModRecordSetRecChnTriggered(u8 chn, EMRECALARMTYPE emType, u64 data);
s32 ModRecordSetTrigger(u8 chn, EMRECALARMTYPE emType, u8 key);
s32 ModRecordSetSchedulePara(u8 nChn, SModRecSchPara* para);
s32 ModRecordWriteOneFrame(u8 nChn, SModRecRecordHeader* header);
void ModRecordSetDealHardDiskFull(u8 nCover); //Ó²ÅÌÂúÊ±ÊÇ·ñ¸²¸Ç:0·ñ 1ÊÇ
void ModRecordSetWorkingEnable(u8 nEnable); //ÉèÖÃÄ£¿éÊÇ·ñ¹¤×÷:0·ñ 1ÊÇ
s32 ModRecordBufRelease(void);//ÊÍ·ÅÂ¼Ïñ»º³åÇø£¬½â·ÅÄÚ´æ
s32 ModRecordBufResume(void); //»Ö¸´ÓÉÉÏ¸ö½Ó¿ÚÊÍ·ÅµôµÄ»º³åÇø

s32 ModRecordSetTimeZoneOffset(int nOffset);
s32 ModRecordGetTimeZoneOffset();

//yaogang modify 20141225
/*×¥Í¼ÀàÐÍ	bit_mask	value
¶¨Ê±			0		
ÒÆ¶¯Õì²â	1
±¨¾¯			2 //º£¿µÖ»ÓÐÍâ²¿±¨¾¯ºÍIPCÍâ²¿±¨¾¯²Å×¥Í¼
¶¯²â»ò±¨¾¯			(1<<1) |(1<<2)			ÓÃÓÚ±¸·Ý
¶¯²âÇÒ±¨¾¯			(1<<1) |(1<<2) |(1<<7)	Í¬ÉÏ
ÊÖ¶¯			3
/////////////////ÖÇÄÜÕì²â	4
ÊÖ¶¯½ØÍ¼	4
////////////////////////////////»Ø·Å½ØÍ¼	6
È«²¿ÀàÐÍ			0xff
Ô¤Â¼			5		ÒòÎª±¨¾¯ºóÒªÉÏ´«·¢Éú±¨¾¯Ç°nÃëµÄÍ¼Æ¬
ÈÕ³£			6		(Ã¿ÌìÁ½¸öÊ±¼äµãÉÏ´«Éî¹ã)
È«²¿ÀàÐÍ	0xff
*/

typedef struct
{
	u8 chn;
	u8 pic_type;
	u32 data_size;
	u32 width;
	u32 height;
	time_t tv_sec;
	suseconds_t tv_usec;
}SModRecSnapHeader;

int ModRecordSnapshotToFile(u8 chn, u8 pic_type, time_t tv_sec, suseconds_t tv_usec, void *pdata, u32 data_size, u32 width, u32 height);
int ModRecordSnapshotToBuf(u8 chn, u8 pic_type, time_t tv_sec, suseconds_t tv_usec, void *pdata, u32 data_size, u32 width, u32 height);

//Ô¤Â¼
int ModRecordPreSnapToFile(u8 chn, time_t tv_sec, void *pdata, u32 data_size, u32 width, u32 height);
int ModRecordPreSnapFromFile(u8 chn, time_t tv_sec, void *pdata, u32 * pdata_size);


/********************ÒµÎñ²ãÌá¹©½áÊø***************************/

#ifdef __cplusplus
}
#endif

#endif //_MOD_RECORD_H_

