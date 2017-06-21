#ifndef _PTZ_H_
#define _PTZ_H_

#define MAX_PROTOCOL_LEN 50

typedef struct
{
	u8 nProtocolName[20];
	u8 nCamAddrInfo[3];
	u8 nPanSpeedInfo[4];
	u8 nTileSpeedInfo[4];
	u8 nPresetInfo[4];
	u8 nTourInfo[2];
	u8 nPatternInfo[2];
	u8 nAuxInfo[4];//u8 nAuxInfo[3];//csp modify
	u16 nInternal;
	u8 nProtocolData[MAX_PROTOCOL_LEN];
}SPtzRealCmd;

typedef enum
{
	EM_PTZ_MSG_START_TILEUP = 0,
	EM_PTZ_MSG_START_TILEDOWN,
	EM_PTZ_MSG_START_PANLEFT,
	EM_PTZ_MSG_START_PANRIGHT,
	EM_PTZ_MSG_START_LEFTUP,
	EM_PTZ_MSG_START_LEFTDOWN,
	EM_PTZ_MSG_START_RIGHTUP,
	EM_PTZ_MSG_START_RIGHTDOWN,
	EM_PTZ_MSG_START_ZOOMWIDE,
	EM_PTZ_MSG_START_ZOOMTELE,
	EM_PTZ_MSG_START_FOCUSNEAR,
	EM_PTZ_MSG_START_FOCUSFAR,
	EM_PTZ_MSG_START_IRISSMALL,
	EM_PTZ_MSG_START_IRISLARGE,
	//
	EM_PTZ_MSG_STOP_TILEUP = 30,
	EM_PTZ_MSG_STOP_TILEDOWN,
	EM_PTZ_MSG_STOP_PANLEFT,
	EM_PTZ_MSG_STOP_PANRIGHT,
	EM_PTZ_MSG_STOP_LEFTUP,
	EM_PTZ_MSG_STOP_LEFTDOWN,
	EM_PTZ_MSG_STOP_RIGHTUP,
	EM_PTZ_MSG_STOP_RIGHTDOWN,
	EM_PTZ_MSG_STOP_ZOOMWIDE,
	EM_PTZ_MSG_STOP_ZOOMTELE,
	EM_PTZ_MSG_STOP_FOCUSNEAR,
	EM_PTZ_MSG_STOP_FOCUSFAR,
	EM_PTZ_MSG_STOP_IRISSMALL,
	EM_PTZ_MSG_STOP_IRISLARGE,
	//
	EM_PTZ_MSG_PRESET_SET = 100,
	EM_PTZ_MSG_PRESET_CLEAR,
	EM_PTZ_MSG_PRESET_GOTO,
	//
	EM_PTZ_MSG_AUTOSCAN_ON = 110,
	EM_PTZ_MSG_AUTOSCAN_OFF,
	EM_PTZ_MSG_SETLIMIT_LEFT,
	EM_PTZ_MSG_SETLIMIT_RIGHT,
	//
	EM_PTZ_MSG_AUTOPAN_ON = 120,
	EM_PTZ_MSG_AUTOPAN_OFF,
	//
	EM_PTZ_MSG_START_TOUR = 130,
	EM_PTZ_MSG_STOP_TOUR,
	EM_PTZ_MSG_ADD_TOUR,
	EM_PTZ_MSG_DELETE_TOUR,
	EM_PTZ_MSG_START_TOUR_AUTO,
	EM_PTZ_MSG_STOP_TOUR_AUTO,
	//
	EM_PTZ_MSG_START_PATTERN = 140,
	EM_PTZ_MSG_STOP_PATTERN,
	EM_PTZ_MSG_SET_START,
	EM_PTZ_MSG_SET_STOP,
	//
	EM_PTZ_MSG_AUX_ON = 150,
	EM_PTZ_MSG_AUX_OFF,
	//
	EM_PTZ_MSG_LIGHT_ON = 160,
	EM_PTZ_MSG_LIGHT_OFF,
	//
	EM_PTZ_MSG_WIPER_ON = 170,//xdc modify 0715
	EM_PTZ_MSG_WIPER_OFF,
	//
	EM_PTZ_MSG_SETSPEED = 253,
	EM_PTZ_MSG_NULL = 254,
	//
	EM_PTZ_MSG_SET_PARAM = 255,
	//
}EMPTZMSGTYPE;

typedef struct
{
	u8 nChn;
	EMPTZMSGTYPE emType;
	void* pData;
}SPtzMsgHeader;

typedef struct
{
	u32 nNum;
	char** pNameOfProtocol;
}SPtzPtotocolList;

s32 PtzCreateMsgQueue(void);
s32 PtzDestroyMsgQueue(void);
s32 PtzWriteMsgQueue(SPtzMsgHeader* pHeader);
s32 PtzReadMsgQueue(SPtzMsgHeader* pHeader);
s32 PtzGetProtocolSupport(void);
s32 PtzCheckSupportProtocol(char* name);
s32 PtzGetCmdSupport(void);
s32 PtzGetOneKeyInfo(lua_State* L, u8* firstsrc, u8* secondsrc, u8* thirdsrc, u8* fourthsrc, u8* fifthsrc, u8* firstdes, u8* seconddes, u16* thirddes);
s32 PtzLoadProtocolInfo(u8 nChn);
void PtzTouringFxn(SPtzMsgHeader* pHeader);
s32 PtzDealTourCmd(SPtzMsgHeader* pHeader);
s32 PtzGetCmdFromFile(u8 nChn, u8* firstsrc, u8* secondsrc, u8* des);
s32 PtzCheckByte(u8 nChn, u8* data, u8* elem);
s32 PtzCmdDeal(SPtzMsgHeader* pHeader);
s32 PtzSendDataToDevice(s32 fd, u8 nChn, u8* data, u8 length);
s32 PtzSetMoveSpeed(SPtzMsgHeader* pHeader);
void PtzManagerFxn(void);
s32 PtzCheckTouring(u8 nChn);

#endif //_PTZ_H_

