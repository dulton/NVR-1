#include <signal.h>
#include "mod_common.h"
#include "circlebuf.h"
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "mod_ptz.h"
#include "ptz.h"

#include "mod_common.h"

//#define _NVR_PTZ_

//#ifdef _NVR_PTZ_
//static u8 g_ChnNum = 0;
//static FPMODPTZUPDATE UpdateProtocol = NULL;
//static SPtzPtotocolList g_PtzProtocolList;
//#else
static u8 g_ChnNum = 0;
static SCircleBufInfo g_s_MsgQueueInfo;
static FPMODPTZUPDATE UpdateProtocol = NULL;
static u8 g_Inited = 0;
static u8 g_DeInited = 1;
static u8* g_IsTouring = NULL;

//csp modify 20121018
//static u8* g_ForceStopTouring = NULL;

static pthread_t* g_TourFxnPID = NULL;
static SModPtzParam* g_PtzParam = NULL;
static SPtzRealCmd* g_PtzRealCmd = NULL;
static SPtzPtotocolList g_PtzProtocolList;
static u32 g_CmdNumSupport = 0;
static char** g_PtzCmdList = NULL;
static char* g_FilePath = "/tmp/data/ptz/";
static lua_State* L = NULL;
static s32 PtzFd = -1;

//csp modify 20121018
pthread_mutex_t PtzLock = PTHREAD_MUTEX_INITIALIZER;
//#endif

static s32 g_nResumePreset[32] = {0};
static s32 g_nResumeDelayTime[32] = {0};
static time_t g_nLastCtrlTime[32] = {0};
static u8 g_nResumeFlag[32] = {0};

//#ifndef _NVR_PTZ_
#define TOURPIONT_MAX	128
#define TOURPATH_MAX	32

s32 PtzCreateMsgQueue(void)
{
	g_s_MsgQueueInfo.nBufId = 0;
	g_s_MsgQueueInfo.nLength = 1024 * sizeof(SPtzMsgHeader);
	
	s32 ret = 0;
	ret = CreateCircleBuf(&g_s_MsgQueueInfo);
	
	if(0 != ret)
	{
		fprintf(stderr, "function: %s\n", __FUNCTION__);
		return ret;
	}
	
	printf("msg queue created succesful!!\n");
	
	return ret;
}

s32 PtzDestroyMsgQueue(void)
{
	s32 ret = 0;
	ret = DestroyCircleBuf(&g_s_MsgQueueInfo);
	if(0 != ret)
	{
		fprintf(stderr, "function: %s\n", __FUNCTION__);
		return ret;
	}
	
	return ret;	
}

s32 PtzWriteMsgQueue(SPtzMsgHeader* pHeader)
{
	s32 ret = 0;
	ret = WriteDataToBuf(&g_s_MsgQueueInfo, (u8 *)pHeader, sizeof(SPtzMsgHeader));
	if(0 != ret)
	{
		fprintf(stderr, "function: %s\n", __FUNCTION__);
		return ret;
	}

	return ret;
}

s32 PtzReadMsgQueue(SPtzMsgHeader* pHeader)
{
	s32 ret = 0;
	ret = ReadDataFromBuf(&g_s_MsgQueueInfo, (u8 *)pHeader, sizeof(SPtzMsgHeader));
	if(0 != ret)
	{
		fprintf(stderr, "function: %s\n", __FUNCTION__);
		return ret;
	}
	
	return ret;
}

s32 PtzGetProtocolSupport(void)
{
	lua_pop(L, lua_gettop(L));
	s32 ret = 0;	
	char file[50] = {0};
	sprintf(file, "%s%s", g_FilePath, "info/PTZ_LIST");
	ret = lua_dofile(L, file);
	if(ret != 0)
	{
		printf("file PTZ_LIST is lost !\n");
		return ret;
	}
	lua_getglobal(L, "PtzProtocol");
	/*ret = lua_getglobal(L, "PtzProtocol");
	{
		printf("Read file PTZ_LIST error !\n");
		return ret;
	}*/
	lua_pushstring(L, "List");
	lua_gettable(L, -2);
	
	s32 i = 1;
	while(1)
	{
		lua_pushnumber(L, i);
		lua_gettable(L, -2);
		if(lua_isnil(L, -1))
		{
			g_PtzProtocolList.nNum = i - 1;
			lua_pop(L, 1);
			break;
		}
		lua_pop(L, 1);
		i++;
	}
	g_PtzProtocolList.pNameOfProtocol = (char**)malloc(g_PtzProtocolList.nNum * sizeof(u8*));
	
	const char* name;
	for(i = 0; i < g_PtzProtocolList.nNum; i++)		
	{
		lua_pushnumber(L, i + 1);
		lua_gettable(L, -2);		
		name = lua_tostring(L, -1);
		g_PtzProtocolList.pNameOfProtocol[i] = (char*)malloc(20);
		strcpy(g_PtzProtocolList.pNameOfProtocol[i], name);
		//printf("PTZ protocol [%d]:%s\n",i+1,name);
		lua_pop(L, 1);
	}
	
	lua_pop(L, lua_gettop(L));
	return ret;
}

s32 PtzCheckSupportProtocol(char* name)
{
	if(NULL == name)
	{
		printf("%s failed, Invalid param !\n", __FUNCTION__);
		return -1;
	}
	if(g_PtzProtocolList.nNum == 0)
	{
		return -1;
	}
	s32 i = 0;
	s32 ret = -1;
	for(i = 0; i < g_PtzProtocolList.nNum; i++)
	{
		if(0 == strcasecmp(g_PtzProtocolList.pNameOfProtocol[i], name))
		{
			char file[50] = {0};
			sprintf(file, "%s%s%s", g_FilePath, "pro/", g_PtzProtocolList.pNameOfProtocol[i]);
			if(0 != access(file, F_OK))
			{
				printf("Warning: Protocol file [%s] is lost!\n", g_PtzProtocolList.pNameOfProtocol[i]);
				return -1;
			}
			ret = 0;
			strcpy(name, g_PtzProtocolList.pNameOfProtocol[i]);
			break;
		}
	}
	if(ret != 0)
	{
		printf("The system is not support protocol [%s] !\n", name);
	}
	return ret;
}

s32 PtzGetCmdSupport(void)
{
	lua_pop(L, lua_gettop(L));
	s32 ret = 0;	
	char file[50] = {0};
	sprintf(file, "%s%s", g_FilePath, "info/PTZ_LIST");
	ret = lua_dofile(L, file);
	if(ret != 0)
	{
		printf("file PTZ_LIST is lost !\n");
		return ret;
	}
	lua_getglobal(L, "PtzProtocol");
	/*ret = lua_getglobal(L, "PtzProtocol");
	{
		printf("Read file PTZ_LIST error !\n");
		return ret;
	}*/
	lua_pushstring(L, "Cmd");
	lua_gettable(L, -2);
	
	s32 i = 1;
	u32 cmdnum = 0;
	while(1)
	{
		lua_pushnumber(L, i);
		lua_gettable(L, -2);
		if(lua_isnil(L, -1))
		{
			cmdnum = i - 1;
			lua_pop(L, 1);
			break;
		}
		lua_pop(L, 1);
		i++;
	}
	g_CmdNumSupport = cmdnum;
	g_PtzCmdList = (char**)malloc(cmdnum * sizeof(u8*));
	
	const char* name;
	for(i = 0; i < cmdnum; i++)
	{
		lua_pushnumber(L, i + 1);
		lua_gettable(L, -2);
		name = lua_tostring(L, -1);
		g_PtzCmdList[i] = (char*)malloc(20);
		strcpy(g_PtzCmdList[i], name);
		lua_pop(L, 1);
	}
	
	lua_pop(L, lua_gettop(L));
	return ret;
}

s32 PtzGetOneKeyInfo(lua_State* L, u8* firstsrc, u8* secondsrc, u8* thirdsrc, u8* fourthsrc, u8* fifthsrc, u8* firstdes, u8* seconddes, u16* thirddes)
{
	u32 top1 = lua_gettop(L);
	
	if(NULL != firstsrc)
	{
		lua_getglobal(L, firstsrc);
		if(NULL != secondsrc)
		{
			lua_pushstring(L, secondsrc);
			lua_gettable(L, -2);
			if(NULL != thirdsrc)
			{
				lua_pushstring(L, thirdsrc);
				lua_gettable(L, -2);
				if(NULL != firstdes)
				{
					const char* name = lua_tostring(L, -1);
					strcpy(firstdes, name);
					lua_pop(L, 1);
				}
				else if(NULL != seconddes)
				{
					lua_pushnumber(L, 1);
					lua_gettable(L, -2);	
					*seconddes = lua_tonumber(L, -1);
					lua_pop(L, 1);
					lua_pushnumber(L, 2);
					lua_gettable(L, -2);	
					*(seconddes + 1) = lua_tonumber(L, -1);
					lua_pop(L, 2);					
				}
				else if(NULL != thirddes)
				{
					*thirddes = lua_tonumber(L, -1);
					lua_pop(L, 1);
				}
				else
				{
					printf("warning: no des!\n");
					lua_pop(L, 1);
				}
			}
			lua_pop(L, 1);
		}
		if(NULL != fourthsrc)
		{
			lua_pushstring(L, fourthsrc);
			lua_gettable(L, -2);
			if(NULL != fifthsrc)
			{
				lua_pushstring(L, fifthsrc);
				lua_gettable(L, -2);
				if(NULL != seconddes)
				{
					*(seconddes + 2) = lua_tonumber(L, -1);
				}
				else
				{
					printf("warning: no des too!\n");
				}
				lua_pop(L, 1);	
			}
			lua_pop(L, 1);	
		}		
	}
	
	u32 top2 = lua_gettop(L);
	lua_pop(L, (top2 - top1) > 0 ? (top2 - top1) : 0);
	
	return 0;
}

s32 PtzLoadProtocolInfo(u8 nChn)
{
	lua_pop(L, lua_gettop(L));
	
	s32 ret = 0;
	
	u8 file[50] = {0};
	sprintf(file, "%s%s%s", g_FilePath, "pro/", g_PtzParam[nChn].nProtocolName);
	
	//printf("PtzLoadProtocolInfo before lua_dofile:%s\n",file);
	//fflush(stdout);
	
	lua_dofile(L, file);
	
	//printf("PtzLoadProtocolInfo after lua_dofile:%s\n",file);
	//fflush(stdout);
	
	u8* elem[3] = {"Protocol", "Attr", "CommandAttr"};
	u8* key[15] = {"Name", "Internal", "CamAddrRange", "AddrPos", "MonAddrRange", "PresetRange", "PresetPos", "TourRange", "PatternRange", "TileSpeedRange", "TileSpeedPos", "PanSpeedRange", "PanSpeedPos", "AuxRange", "AuxPos"};
	
	PtzGetOneKeyInfo(L, elem[0], elem[1], key[0], NULL, NULL, g_PtzRealCmd[nChn].nProtocolName, NULL, NULL);
	PtzGetOneKeyInfo(L, elem[0], elem[1], key[1], NULL, NULL, NULL, NULL, &g_PtzRealCmd[nChn].nInternal);
	PtzGetOneKeyInfo(L, elem[0], elem[1], key[2], elem[2], key[3], NULL, g_PtzRealCmd[nChn].nCamAddrInfo, NULL);
	//PtzGetOneKeyInfo(L, elem[0], elem[1], key[4], NULL, NULL, NULL, g_PtzRealCmd[nChn]., NULL);
	PtzGetOneKeyInfo(L, elem[0], elem[1], key[5], elem[2], key[6], NULL, g_PtzRealCmd[nChn].nPresetInfo, NULL);
	PtzGetOneKeyInfo(L, elem[0], elem[1], key[7], NULL, NULL, NULL, g_PtzRealCmd[nChn].nTourInfo, NULL);
	PtzGetOneKeyInfo(L, elem[0], elem[1], key[8], NULL, NULL, NULL, g_PtzRealCmd[nChn].nPatternInfo, NULL);
	PtzGetOneKeyInfo(L, elem[0], elem[1], key[9], elem[2], key[10], NULL, g_PtzRealCmd[nChn].nTileSpeedInfo, NULL);
	PtzGetOneKeyInfo(L, elem[0], elem[1], key[11], elem[2], key[12], NULL, g_PtzRealCmd[nChn].nPanSpeedInfo, NULL);
	PtzGetOneKeyInfo(L, elem[0], elem[1], key[13], elem[2], key[14], NULL, g_PtzRealCmd[nChn].nAuxInfo, NULL);	
	
	g_PtzRealCmd[nChn].nTileSpeedInfo[3] = (((u16)g_PtzRealCmd[nChn].nTileSpeedInfo[0] + (u16)g_PtzRealCmd[nChn].nTileSpeedInfo[1]) >> 1) & 0xff;
	g_PtzRealCmd[nChn].nPanSpeedInfo[3] = (((u16)g_PtzRealCmd[nChn].nPanSpeedInfo[0] + (u16)g_PtzRealCmd[nChn].nPanSpeedInfo[1]) >> 1) & 0xff;
	
	lua_pop(L, lua_gettop(L));
	
	return ret;
}

void stopThread(int sig_no)
{
	printf("PTZ stopThread - 1#################################################################\n");
	if(SIGUSR1 == sig_no)
	{
		printf("PTZ stopThread - 2#################################################################\n");
		pthread_exit((void *)0);
	}
}

void PtzTouringFxn(SPtzMsgHeader* pHeader)
{
	printf("$$$$$$$$$$$$$$$$$$PtzTouringFxn id:%d\n",getpid());
	
	pthread_detach(pthread_self());//csp modify 20121018
	
	if(NULL != pHeader)
	{
		//add by Lirl on Dec/02/2011,注册信号
		signal(SIGUSR1, stopThread);
		//end
		
		SPtzMsgHeader head;
		memcpy(&head, pHeader, sizeof(SPtzMsgHeader));
		
		//memset(pHeader, 0, sizeof(SPtzMsgHeader));//csp modify 20121018
		
		if((head.emType == EM_PTZ_MSG_START_TOUR) || (head.emType == EM_PTZ_MSG_START_TOUR_AUTO))
		{
			SModPtzPresetPara preset[256];
			//csp modify 20121018
			//memset(&preset, 0, sizeof(preset));
			memset(preset, 0, sizeof(preset));
			
			u8 flag_none_preset = 1;
			
			if(head.emType == EM_PTZ_MSG_START_TOUR)
			{
				s32 id = 0;
				memcpy(&id, head.pData, sizeof(s32));
				memcpy(&preset[0], &g_PtzParam[head.nChn].sTourPresetPara.nTourPresetId[id][0], sizeof(preset));
				
				if(head.pData) free(head.pData);//csp modify 20121018
				
				u16 i = 0;
				for(i = 0; i < 0xff; i++)
				{
					if((preset[i].nPresetId) || (preset[i].nStayTime) || (preset[i].nSpeed))
					{
						flag_none_preset = 0;
						break;
					}
				}
			}
			else //自动扫描
			{
				if(head.pData) free(head.pData);//csp modify 20121018
				
				u16 i = 0;
				for(i = 0; i < 0xff; i++)
				{
					if(g_PtzParam[head.nChn].sTourPresetPara.nPresetId[i/64] & (1 << (i%64)))
					{
						preset[i].nPresetId = i + 1; //预置点是从1开始存的
						preset[i].nSpeed = (MOD_PTZ_MOVE_SPEED_MAX + MOD_PTZ_MOVE_SPEED_MIN) >> 1;
						preset[i].nStayTime = 5; //默认停留为5秒
						flag_none_preset = 0;
					}
				}
			}
			
			if(flag_none_preset)
			{
				g_IsTouring[head.nChn] = 0;
			}
			else
			{
				s32 *ppresetpos = NULL;
				//ppresetpos = (s32 *)calloc(sizeof(s32), 1);
				s32 my_preset_pos = 0;
				ppresetpos = &my_preset_pos;
				
				u32 staytime = 5;
				
				u8 i = 0;
				
				#if 0//csp modify 20121018
				u8 count = 0;
				#endif
				
				SPtzMsgHeader msg;
				msg.nChn = head.nChn;
				msg.emType = EM_PTZ_MSG_PRESET_GOTO;
				msg.pData = (void*)ppresetpos;
				
				while(1)
				{
					if(g_DeInited)
					{
						break;
					}
					
					if(g_IsTouring[head.nChn] == 0)
					{
						break;
					}
					
					#if 1//csp modify 20121018
					if((preset[i].nPresetId) || (preset[i].nStayTime) || (preset[i].nSpeed))
					{
						*ppresetpos = preset[i].nPresetId;
						staytime = preset[i].nStayTime;
						i = (i+1)%256;
					}
					else
					{
						i = (i+1)%256;
						continue;
					}
					
					PtzCmdDeal(&msg);
					//printf("PtzCmdDeal is end\n");//xdc modify 0715
					
					if(IPC_CMD_PtzCtrl(msg.nChn, msg.emType, msg.pData) < 0)//xdc modify 0715  485
					{
						//printf("PtzTouringFxn IPC_CMD_PtzCtrl is error!\n");
						//continue;
					}
					
					sleep(staytime);//csp modify 20121209
					#else
					count = 0;
					
					while(1)
					{
						if((preset[i].nPresetId) || (preset[i].nStayTime) || (preset[i].nSpeed))
						{
							*ppresetpos = preset[i].nPresetId;
							staytime = preset[i].nStayTime;
							i++;
							break;
						}
						
						//防止i自增为256时越界发生断错误
 						if (i < 0xff) {//if (i>=0 && i < 0xff) {//csp modify
							i++;
						} else {
							break;
						}
						
						if((++count) == 0xff)
						{
							g_IsTouring[head.nChn] = 0;
							break;
						}
					}
					
					if (255 == i) {
						i = 0;
						continue;
					}
					
					if(0 != PtzWriteMsgQueue(&msg))
					{
						fprintf(stderr, "PtzWriteMsgQueue error! function: %s\n", __FUNCTION__);
						sleep(1);
					}
					else
					{
						sleep(staytime);
					}
					#endif
					
					/*
					printf("g_TourFxnPID[head.nChn] = %d, pid:%d\n", g_TourFxnPID[head.nChn],pthread_self());
					if((g_ForceStopTouring[head.nChn])
						&& (g_TourFxnPID[head.nChn] != pthread_self()))
					{
						g_ForceStopTouring[head.nChn] = 0;
						break;
					}
					*/
				}
				
				//if(ppresetpos) free(ppresetpos);//csp modify 20121018
			}
		}
		else//csp modify 20121018
		{
			if(head.pData) free(head.pData);//csp modify 20121018//按道理逻辑不会走到这里
		}
	}
}

s32 PtzDealTourCmd(SPtzMsgHeader* pHeader)
{
	if((NULL == pHeader) || ((pHeader->emType != EM_PTZ_MSG_START_TOUR) && (pHeader->emType != EM_PTZ_MSG_STOP_TOUR) && (pHeader->emType != EM_PTZ_MSG_START_TOUR_AUTO) && (pHeader->emType != EM_PTZ_MSG_STOP_TOUR_AUTO)))
	{
		printf("Invalid Para! function:%s\n", __FUNCTION__);
		return -1;
	}
	if((pHeader->emType == EM_PTZ_MSG_STOP_TOUR) || (pHeader->emType == EM_PTZ_MSG_STOP_TOUR_AUTO))
	{
		g_IsTouring[pHeader->nChn] = 0;
		return 0;
	}
	if((pHeader->emType == EM_PTZ_MSG_START_TOUR) || (pHeader->emType == EM_PTZ_MSG_START_TOUR_AUTO))
	{
		#if 1//csp modify 20121018
		void* pData = NULL;
		if(pHeader->pData != NULL)
		{
			pData = malloc(sizeof(u32));
			if(pData == NULL)
			{
				printf("%s error!\n", __FUNCTION__);
				return -1;
			}
			memcpy(pData, pHeader->pData, sizeof(u32));
		}
		#endif
		
		g_IsTouring[pHeader->nChn] = 1;
		
		//pthread_t pid = 0;
		
		//add by Lirl on Dec/02/2011,给上一个线程发信号
		if(0 != g_TourFxnPID[pHeader->nChn])
		{
			printf("pthread_kill SIGUSR1\n");
			pthread_kill(g_TourFxnPID[pHeader->nChn], SIGUSR1);
		}
		//end
		
		#if 1//csp modify 20121018
		static SPtzMsgHeader sMsg[16];
		static int index = 0;
		memcpy(&sMsg[index], pHeader, sizeof(SPtzMsgHeader));
		sMsg[index].pData = pData;
		int ret = pthread_create(&g_TourFxnPID[pHeader->nChn], NULL, (void*)PtzTouringFxn, &sMsg[index]);
		index = (index + 1) % 16;
		#else
		int ret = pthread_create(&g_TourFxnPID[pHeader->nChn], NULL, (void*)PtzTouringFxn, pHeader);
		#endif
		
		//g_TourFxnPID[pHeader->nChn] = pid;
		//usleep(5 * 1000);
		
		if(ret != 0)
		{
			printf("creat touring pthread Failed!\n");
			g_IsTouring[pHeader->nChn] = 0;
			
			#if 1//csp modify 20121018
			if(pData != NULL)
			{
				free(pData);
			}
			#endif
		}
		
		return ret;
	}
	
	return 0;
}

s32 PtzGetCmdFromFile(u8 nChn, u8* firstsrc, u8* secondsrc, u8* des)
{
	//csp modify
	if((firstsrc == NULL) || (secondsrc == NULL) || (des == NULL))
	{
		return -1;
	}
	
	//csp modify
	*des = 0;
	
	lua_pop(L, lua_gettop(L));
	
	s32 ret = 0;	
	u8 file[50] = {0};
	sprintf(file, "%s%s%s", g_FilePath, "pro/", g_PtzParam[nChn].nProtocolName);
	ret = lua_dofile(L, file);
	if(ret != 0)
	{
		printf("file %s is lost !\n", g_PtzParam[nChn].nProtocolName);
		return ret;
	}
	
	lua_getglobal(L, "Protocol");
	
	lua_pushstring(L, "Command");
	lua_gettable(L, -2);
	lua_pushstring(L, firstsrc);
	lua_gettable(L, -2);
	lua_pushstring(L, secondsrc);
	lua_gettable(L, -2);
	if(lua_isnil(L, -1))
	{
		printf("Ptz not support this cmd!\n");
		*des = 0;
	}
	else
	{
		int i = 1;
		while(1)
		{
			lua_pushnumber(L, i);
			lua_gettable(L, -2);
			if(lua_isnil(L, -1))
			{
				i--;
				*des = i;
				break;
			}
			*(des + i) = lua_tonumber(L, -1);
			i++;
			lua_pop(L, 1);
		}
	}
	
	lua_pop(L, lua_gettop(L));
	return ret;
}

s32 PtzCheckByte(u8 nChn, u8* data, u8* elem)
{
	lua_pop(L, lua_gettop(L));
	
	//csp modify
	if(data == NULL || *data == 0)
	{
		return -1;
	}
	
	s32 ret = 0;	
	u8 file[50] = {0};
	sprintf(file, "%s%s%s", g_FilePath, "pro/", g_PtzParam[nChn].nProtocolName);
	ret = lua_dofile(L, file);
	if(ret != 0)
	{
		printf("file %s is lost!\n", g_PtzParam[nChn].nProtocolName);
		return ret;
	}
	
	lua_newtable(L);
	
	int i;
	for(i = 0; i < *data; i++)
	{
		lua_pushnumber(L, i + 1);
		lua_pushnumber(L, *(data + i + 1));
		lua_settable(L, -3);		
	}
	
	lua_getglobal(L, "Protocol");
	lua_pushstring(L, elem);
	lua_gettable(L, -2);
	if(lua_isnil(L, -1))
	{
		//printf("Ptz not support %s check!\n", elem);
		if(0 == strcasecmp(elem, "CamAddrProcess"))
		{
			*(data + g_PtzRealCmd[nChn].nCamAddrInfo[2]) = g_PtzParam[nChn].nCamAddr;
		}
		else if(0 == strcasecmp(elem, "PresetProcess"))
		{
			*(data + g_PtzRealCmd[nChn].nPresetInfo[2]) = g_PtzRealCmd[nChn].nPresetInfo[3];
		}
		//csp modify
		else if(0 == strcasecmp(elem, "AuxProcess"))
		{
			if(g_PtzRealCmd[nChn].nAuxInfo[2] <= *data)
			{
				*(data + g_PtzRealCmd[nChn].nAuxInfo[2]) = g_PtzRealCmd[nChn].nAuxInfo[3];
			}
		}
	}
	else
	{
		//printf("PtzCheckByte:%s-1\n",elem);
		//fflush(stdout);
		
		lua_pushvalue(L, -3);
		
		//printf("PtzCheckByte:%s-2\n",elem);
		//fflush(stdout);
		
		if(0 == strcasecmp(elem, "SpeedProcess"))
		{
			lua_pushnumber(L, g_PtzRealCmd[nChn].nTileSpeedInfo[3]);
			lua_pushnumber(L, g_PtzRealCmd[nChn].nPanSpeedInfo[3]);
			lua_pcall(L, 3, 1, 0);
		}
		else if(0 == strcasecmp(elem, "CamAddrProcess"))
		{
			lua_pushnumber(L, g_PtzParam[nChn].nCamAddr);
			lua_pcall(L, 2, 1, 0);
		}
		else if(0 == strcasecmp(elem, "PresetProcess"))
		{
			lua_pushnumber(L, g_PtzRealCmd[nChn].nPresetInfo[3]);
			lua_pcall(L, 2, 1, 0);
		}
		else if(0 == strcasecmp(elem, "Checksum"))
		{
			//printf("PtzCheckByte:%s-3\n",elem);
			//fflush(stdout);
			
			lua_pcall(L, 1, 1, 0);
			
			//printf("PtzCheckByte:%s-4\n",elem);
			//fflush(stdout);
		}
		i = 1;
		while(1)
		{
			//printf("PtzCheckByte:(%s,%d)-5\n",elem,i);
			//fflush(stdout);
			
			lua_pushnumber(L, i);
			
			//printf("PtzCheckByte:%s-5.1\n",elem);
			//fflush(stdout);
			
			lua_gettable(L, -2);
			
			//printf("PtzCheckByte:%s-5.2\n",elem);
			//fflush(stdout);
			
			if(lua_isnil(L, -1))
			{
				//printf("PtzCheckByte:%s-5.3\n",elem);
				//fflush(stdout);
				
				break;
			}
			
			//printf("PtzCheckByte:%s-6\n",elem);
			//fflush(stdout);
			
			*(data + i) = lua_tonumber(L, -1);
			i++;
			
			//printf("PtzCheckByte:%s-7\n",elem);
			//fflush(stdout);
			
			lua_pop(L, 1);
			
			//printf("PtzCheckByte:%s-8\n",elem);
			//fflush(stdout);
		}
		if(*data != (i - 1))
		{
			printf("warning: sth is wrong when checkdata!\n");
		}
		
		//printf("PtzCheckByte:%s-9\n",elem);
		//fflush(stdout);
	}
	
	//printf("PtzCheckByte:%s-10\n",elem);
	//fflush(stdout);
	
	lua_pop(L, lua_gettop(L));
	return ret;
}

s32 PtzCmdDeal(SPtzMsgHeader* pHeader)
{
	if(NULL == pHeader)
	{
		printf("%s failed, Invalid param !\n", __FUNCTION__);
		return -1;
	}
	
	u8* elem = (u8 *)"Start";
	u8* key = NULL;
	if(pHeader->emType < EM_PTZ_MSG_PRESET_SET)
	{
		if(pHeader->emType < EM_PTZ_MSG_STOP_TILEUP)
		{
			key = g_PtzCmdList[pHeader->emType - EM_PTZ_MSG_START_TILEUP];
		}
		else
		{
			elem = (u8 *)"Stop";
			key = g_PtzCmdList[pHeader->emType - EM_PTZ_MSG_STOP_TILEUP];
		}
	}
	else if(pHeader->emType < EM_PTZ_MSG_AUTOSCAN_ON)
	{
		key = g_PtzCmdList[16 + pHeader->emType - EM_PTZ_MSG_PRESET_SET];
	}
	else if(pHeader->emType < EM_PTZ_MSG_AUTOPAN_ON)
	{
		key = g_PtzCmdList[21 + pHeader->emType - EM_PTZ_MSG_AUTOSCAN_ON];
	}
	else if(pHeader->emType < EM_PTZ_MSG_START_TOUR)
	{
		key = g_PtzCmdList[19 + pHeader->emType - EM_PTZ_MSG_AUTOPAN_ON];
	}
	else if(pHeader->emType < EM_PTZ_MSG_START_PATTERN)
	{
		//key = g_PtzCmdList[25 + pHeader->emType - EM_PTZ_MSG_START_TOUR];
		key = NULL;
	}
	else if(pHeader->emType < EM_PTZ_MSG_AUX_ON)
	{
		key = g_PtzCmdList[27 + pHeader->emType - EM_PTZ_MSG_START_PATTERN];
	}
	else if(pHeader->emType < EM_PTZ_MSG_LIGHT_ON)
	{
		key = g_PtzCmdList[31 + pHeader->emType - EM_PTZ_MSG_AUX_ON];
	}
	else if(pHeader->emType < EM_PTZ_MSG_WIPER_ON)//xdc modify 0715
	{
		key = g_PtzCmdList[14 + pHeader->emType - EM_PTZ_MSG_LIGHT_ON];
	}
	else
	{
		key = g_PtzCmdList[35 + pHeader->emType - EM_PTZ_MSG_WIPER_ON];//xdc modify 0715
		if(key == NULL)
		{
			key = g_PtzCmdList[31 + pHeader->emType - EM_PTZ_MSG_WIPER_ON];
			pHeader->emType = EM_PTZ_MSG_AUX_ON + pHeader->emType - EM_PTZ_MSG_WIPER_ON;
			if(pHeader->pData != NULL)
			{
				s32 aux = 1;
				memcpy(pHeader->pData, &aux, sizeof(s32));
			}
		}
	}
	
	if(NULL == key)
	{
		if((pHeader->emType >= EM_PTZ_MSG_START_TOUR) && (pHeader->emType < EM_PTZ_MSG_START_PATTERN))
		{
			/*if(pHeader->emType == EM_PTZ_MSG_ADD_TOUR)
			{
				if(pHeader->pData != NULL)
				{
					SModPtzTourInfo sTourInfo;
					memset(&sTourInfo, 0, sizeof(sTourInfo));
					memcpy(&sTourInfo, pHeader->pData, sizeof(SModPtzTourInfo));
					g_PtzParam[pHeader->nChn].sTourPresetPara.nTourId[sTourInfo.nTourId / 64] |= (1 << (sTourInfo.nTourId % 64));
					memcpy(&g_PtzParam[pHeader->nChn].sTourPresetPara.nTourPresetId[sTourInfo.nTourId][0], &sTourInfo.nPresetSet[0], 4 * sizeof(u64));
				}			
			}
			else if(pHeader->emType == EM_PTZ_MSG_DELETE_TOUR)
			{
				if(pHeader->pData != NULL)
				{
					s32 nTourId = 0;
					memcpy(&nTourId, pHeader->pData, sizeof(s32));
					g_PtzParam[pHeader->nChn].sTourPresetPara.nTourId[nTourId / 64] &= ~(1 << (nTourId % 64));
					memset(&g_PtzParam[pHeader->nChn].sTourPresetPara.nTourPresetId[nTourId][0], 0, 4 * sizeof(u64));
				}
			}
			else*/
			{
				PtzDealTourCmd(pHeader);
			}
		}
		return 0;
	}
	
	PtzGetCmdFromFile(pHeader->nChn, elem, key, g_PtzRealCmd[pHeader->nChn].nProtocolData);
	if(*g_PtzRealCmd[pHeader->nChn].nProtocolData == 0)
	{
		printf("get ptz cmd failed\n");
		//fflush(stdout);
		return 0;
	}
	
	//printf("ptz-1\n");
	//fflush(stdout);
	
	if(pHeader->emType <= EM_PTZ_MSG_START_RIGHTDOWN)
	{
		PtzCheckByte(pHeader->nChn, g_PtzRealCmd[pHeader->nChn].nProtocolData, "SpeedProcess");
	}
	if((pHeader->emType >= EM_PTZ_MSG_PRESET_SET) && (pHeader->emType < EM_PTZ_MSG_AUTOSCAN_ON))
	{
		//printf("ptz-2\n");
		//fflush(stdout);
		
		if(pHeader->pData == NULL)
		{
			return -1;
		}
		
		//printf("ptz-3\n");
		//fflush(stdout);
		
		s32 preset = 0;
		memcpy(&preset, pHeader->pData, sizeof(s32));
		preset &= 0xff;
		g_PtzRealCmd[pHeader->nChn].nPresetInfo[3] = preset;
		if(pHeader->emType == EM_PTZ_MSG_PRESET_SET)
		{
			g_PtzParam[pHeader->nChn].sTourPresetPara.nPresetId[preset / 64] |= (1 << (preset % 64));
		}
		if(pHeader->emType == EM_PTZ_MSG_PRESET_CLEAR)
		{
			g_PtzParam[pHeader->nChn].sTourPresetPara.nPresetId[preset / 64] &= (~(1 << (preset % 64)));
			/*u16 id = 0;
			for(id = 0; id < 256; id++)
			{
				g_PtzParam[pHeader->nChn].sTourPresetPara.nTourPresetId[id][preset / 64] &= (~(1 << (preset % 64)));
			}*/
		}
		
		//printf("ptz-4\n");
		//fflush(stdout);
		
		PtzCheckByte(pHeader->nChn, g_PtzRealCmd[pHeader->nChn].nProtocolData, "PresetProcess");
		
		//printf("ptz-5\n");
		//fflush(stdout);
	}
	//csp modify
	if(pHeader->emType >= EM_PTZ_MSG_AUX_ON && pHeader->emType <= EM_PTZ_MSG_AUX_OFF)
	{
		if(pHeader->pData == NULL)
		{
			return -1;
		}
		
		s32 aux = 0;
		memcpy(&aux, pHeader->pData, sizeof(s32));
		aux &= 0xff;
		
		g_PtzRealCmd[pHeader->nChn].nAuxInfo[3] = aux;
		
		PtzCheckByte(pHeader->nChn, g_PtzRealCmd[pHeader->nChn].nProtocolData, "AuxProcess");
	}
	
	//printf("ptz-6\n");
	//fflush(stdout);
	
	PtzCheckByte(pHeader->nChn, g_PtzRealCmd[pHeader->nChn].nProtocolData, "CamAddrProcess");
	
	//printf("ptz-7\n");
	//fflush(stdout);
	
	PtzCheckByte(pHeader->nChn, g_PtzRealCmd[pHeader->nChn].nProtocolData, "Checksum");
	
	//printf("ptz-8\n");
	//fflush(stdout);
	
	PtzSendDataToDevice(PtzFd, pHeader->nChn, &g_PtzRealCmd[pHeader->nChn].nProtocolData[1], g_PtzRealCmd[pHeader->nChn].nProtocolData[0]);
	
	//printf("ptz-9\n");
	//fflush(stdout);
	
	return 0;
}

s32 PtzSendDataToDevice(s32 fd, u8 nChn, u8* data, u8 length)
{
	return 0;
	
	if((fd < 0) || (nChn >= g_ChnNum) || (NULL == data))
	{
		printf("Invalid param ! function: %s\n", __FUNCTION__);
		return -1;
	}
	
	//csp modify 20121018
	pthread_mutex_lock(&PtzLock);
	
	if(set_speed(fd, g_PtzParam[nChn].emBaudRate) == -1)
	{
		printf("Set speed Error! function:%s\n", __FUNCTION__);
	}
	if(set_Parity(fd, g_PtzParam[nChn].emDataBit, g_PtzParam[nChn].emStopBit, 'N') == -1)
	{
		printf("Set Parity Error! function:%s\n", __FUNCTION__);
	}
	write(fd, data, length);
	
	//csp modify 20121018
	pthread_mutex_unlock(&PtzLock);
	
	printf("Cmd: ");
	int i = 0;
	for(i = 0; i< length; i++)
	{
		printf("0x%02x ", *(data + i));
	}
	printf("\n");
	
	return 0;
}

s32 PtzSetMoveSpeed(SPtzMsgHeader* pHeader)
{
	if((NULL == pHeader) || (NULL == pHeader->pData))
	{
		printf("Invalid para ! function:%s\n", __FUNCTION__);
		return -1;
	}
	
	s32 speed = 0;
	memcpy(&speed, pHeader->pData, sizeof(s32));
	if(speed < MOD_PTZ_MOVE_SPEED_MIN)
	{
		speed = MOD_PTZ_MOVE_SPEED_MIN;
	}
	else if(speed > MOD_PTZ_MOVE_SPEED_MAX)
	{
		speed = MOD_PTZ_MOVE_SPEED_MAX;
	}
	
	u8 minspeed = g_PtzRealCmd[pHeader->nChn].nPanSpeedInfo[0];
	u8 maxspeed = g_PtzRealCmd[pHeader->nChn].nPanSpeedInfo[1];
	u8 everyspeed = 1;
	if((maxspeed - minspeed) < (MOD_PTZ_MOVE_SPEED_MAX - MOD_PTZ_MOVE_SPEED_MIN))
	{
		g_PtzRealCmd[pHeader->nChn].nPanSpeedInfo[3] = ((speed-MOD_PTZ_MOVE_SPEED_MIN)<(maxspeed - minspeed))?(minspeed+speed-MOD_PTZ_MOVE_SPEED_MIN):maxspeed;
	}
	else if((maxspeed - minspeed) < 2 * (MOD_PTZ_MOVE_SPEED_MAX - MOD_PTZ_MOVE_SPEED_MIN))
	{
		if(2 * (speed - MOD_PTZ_MOVE_SPEED_MIN) + (MOD_PTZ_MOVE_SPEED_MAX - speed) <= (maxspeed - minspeed))
		{
			g_PtzRealCmd[pHeader->nChn].nPanSpeedInfo[3] = minspeed + 2 * (speed - MOD_PTZ_MOVE_SPEED_MIN);
		}
		else
		{
			g_PtzRealCmd[pHeader->nChn].nPanSpeedInfo[3] = maxspeed - (MOD_PTZ_MOVE_SPEED_MAX - speed);
		}
	}
	else
	{
		everyspeed = (maxspeed - minspeed) / (MOD_PTZ_MOVE_SPEED_MAX - MOD_PTZ_MOVE_SPEED_MIN);
		g_PtzRealCmd[pHeader->nChn].nPanSpeedInfo[3] = minspeed + (speed - MOD_PTZ_MOVE_SPEED_MIN) * everyspeed;
	}
	
	minspeed = g_PtzRealCmd[pHeader->nChn].nTileSpeedInfo[0];
	maxspeed = g_PtzRealCmd[pHeader->nChn].nTileSpeedInfo[1];
	if((maxspeed - minspeed) < (MOD_PTZ_MOVE_SPEED_MAX - MOD_PTZ_MOVE_SPEED_MIN))
	{
		g_PtzRealCmd[pHeader->nChn].nTileSpeedInfo[3] = ((speed-MOD_PTZ_MOVE_SPEED_MIN)<(maxspeed - minspeed))?(minspeed+speed-MOD_PTZ_MOVE_SPEED_MIN):maxspeed;
	}
	else if((maxspeed - minspeed) < 2 * (MOD_PTZ_MOVE_SPEED_MAX - MOD_PTZ_MOVE_SPEED_MIN))
	{
		if(2 * (speed - MOD_PTZ_MOVE_SPEED_MIN) + (MOD_PTZ_MOVE_SPEED_MAX - speed) <= (maxspeed - minspeed))
		{
			g_PtzRealCmd[pHeader->nChn].nTileSpeedInfo[3] = minspeed + 2 * (speed - MOD_PTZ_MOVE_SPEED_MIN);
		}
		else
		{
			g_PtzRealCmd[pHeader->nChn].nTileSpeedInfo[3] = maxspeed - (MOD_PTZ_MOVE_SPEED_MAX - speed);
		}
	}
	else
	{
		everyspeed = (maxspeed - minspeed) / (MOD_PTZ_MOVE_SPEED_MAX - MOD_PTZ_MOVE_SPEED_MIN);
		g_PtzRealCmd[pHeader->nChn].nTileSpeedInfo[3] = minspeed + (speed - MOD_PTZ_MOVE_SPEED_MIN) * everyspeed;
	}
	
	return 0;
}					

void PtzManagerFxn(void)
{
	u32 errortimes = 0;
	
	SPtzMsgHeader nMsg;
	memset(&nMsg, 0, sizeof(SPtzMsgHeader));
	
	printf("$$$$$$$$$$$$$$$$$$PtzManagerFxn id:%d\n",getpid());
	
	while(1)
	{
		if(g_s_MsgQueueInfo.nLenUsed > 0)
		{
			memset(&nMsg, 0, sizeof(SPtzMsgHeader));//csp modify
			
			if(0 != PtzReadMsgQueue(&nMsg))
			{
				printf("PtzReadMsgQueue error! error times = %d\n", ++errortimes);
				continue;
			}
			
			if(EM_PTZ_MSG_NULL == nMsg.emType)
			{
				printf("a NULL cmd received!\n");
			}
			else if(EM_PTZ_MSG_SET_PARAM == nMsg.emType)
			{
				//printf("chn%d EM_PTZ_MSG_SET_PARAM-1\n",nMsg.nChn);
				
				memcpy(&g_PtzParam[nMsg.nChn], nMsg.pData, sizeof(SModPtzParam));
				
				//printf("chn%d EM_PTZ_MSG_SET_PARAM-2\n",nMsg.nChn);
				
				if(0 != PtzLoadProtocolInfo(nMsg.nChn))
				{
					printf("EM_PTZ_MSG_SET_PARAM PtzLoadProtocolInfo error!\n");
				}
				
				//printf("chn%d EM_PTZ_MSG_SET_PARAM-3\n",nMsg.nChn);
				
				if(nMsg.pData != NULL)
				{
					free(nMsg.pData);
					nMsg.pData = NULL;
				}
				
				//printf("chn%d EM_PTZ_MSG_SET_PARAM-4\n",nMsg.nChn);
			}
			else if(EM_PTZ_MSG_SETSPEED == nMsg.emType)
			{
				if(nMsg.pData != NULL)
				{
					if(0 != PtzSetMoveSpeed(&nMsg))
					{
						printf("PtzSetSpeed error!\n");
					}
				}
			}
			else
			{
				printf("PtzCmdDeal*****\n");
				if(0 != PtzCmdDeal(&nMsg))
				{
					printf("PtzCmdDeal error!\n");
				}
			}
			
			if(nMsg.pData != NULL)
			{
				//if(0 == g_IsTouring[nMsg.nChn]) //zlb20111117
				//只有在云台不在巡航时才能free，在操作巡航时free系统会崩溃
				//if(0 == g_IsTouring[nMsg.nChn])//csp modify 20121018
				{
					free(nMsg.pData);
					nMsg.pData = NULL;
				}
			}
		}
		
		//xdc add
		time_t nCurTime = time(NULL);
		int i = 0;
		for(i = 0; i < g_ChnNum; i++)
		{
			if(g_nResumeDelayTime[i] != 0 && g_nResumePreset[i] != 0)
			{
				if(nCurTime >= g_nLastCtrlTime[i] + g_nResumeDelayTime[i] || nCurTime < g_nLastCtrlTime[i])
				{
					if(!g_nResumeFlag[i])
					{
						g_nResumeFlag[i] = 1;
						
						SPtzMsgHeader nMsg;
						nMsg.nChn = i;
						nMsg.emType = EM_PTZ_MSG_PRESET_GOTO;
						nMsg.pData = &g_nResumePreset[i];
						printf("chn%d ptz resume preset:%d\n", i, g_nResumePreset[i]);
						PtzCmdDeal(&nMsg);
					}
				}
			}
		}
		
		if(g_DeInited)
		{
			break;
		}
		
		usleep(20 * 1000);
	}
}
//#endif

s32 ModPtzInit(SPtzInitPara* para)
{
//#ifdef _NVR_PTZ_
	/*if((NULL == para) || (NULL == para->pfUpdateProtocol))
	{
		printf("%s failed, Invalid param !\n", __FUNCTION__);
		return -1;
	}
	
	g_ChnNum = para->nChnNum;
	UpdateProtocol = para->pfUpdateProtocol;
	
	memset(&g_PtzProtocolList, 0, sizeof(SPtzPtotocolList));
	g_PtzProtocolList.nNum = 2;
	g_PtzProtocolList.pNameOfProtocol = (char**)malloc(g_PtzProtocolList.nNum * sizeof(char*));
	
	char *MyProtocolList[] = 
	{
		"PelcoD",
		"PelcoP"
	};
	
	int i;
	for(i = 0; i < g_PtzProtocolList.nNum; i++)		
	{
		g_PtzProtocolList.pNameOfProtocol[i] = (char*)malloc(20);
		strcpy(g_PtzProtocolList.pNameOfProtocol[i], MyProtocolList[i]);
		printf("PTZ protocol [%d]:%s\n",i+1,MyProtocolList[i]);
	}
	
	UpdateProtocol(&g_PtzProtocolList.nNum, (u8**)g_PtzProtocolList.pNameOfProtocol);
	*/
	//return 0;
//#else
	printf("ModPtzInit-1\n");
	
	if((NULL == para) || (NULL == para->pfUpdateProtocol))
	{
		printf("%s failed, Invalid param !\n", __FUNCTION__);
		return -1;
	}
	if(g_Inited)
	{
		printf("%s failed, Can not init again !\n", __FUNCTION__);
		return -1;
	}
	g_ChnNum = para->nChnNum;
#if 1
	//跃天485接了报警板
	PtzFd = -1;
#else
	printf("ModPtzInit: DevPath: %s\n", (char *)para->pDevPath);
	PtzFd = OpenDev((char *)para->pDevPath);

	if(PtzFd < 0)
	{
		printf("Open PtzDev failed!\n");
		return -1;
	}
#endif

	UpdateProtocol = para->pfUpdateProtocol;
	
	g_IsTouring = (u8*)malloc(g_ChnNum);
	if(g_IsTouring == NULL)
	{
		fprintf(stderr, "function: %s\n", __FUNCTION__);
		return -1;
	}
	memset(g_IsTouring, 0, g_ChnNum);
	
	//csp modify 20121018
	//g_ForceStopTouring = (u8*)malloc(g_ChnNum);
	//if(g_ForceStopTouring == NULL)
	//{
	//	fprintf(stderr, "function: %s\n", __FUNCTION__);
	//	return -1;
	//}
	//memset(g_ForceStopTouring, 0, g_ChnNum);
	
	g_TourFxnPID = (pthread_t*)calloc(g_ChnNum, sizeof(pthread_t));
	if(g_TourFxnPID == NULL)
	{
		fprintf(stderr, "function: %s\n", __FUNCTION__);
		return -1;
	}
	memset(g_TourFxnPID,0,g_ChnNum*sizeof(pthread_t));//csp modify 20121210
	
	g_PtzParam = (SModPtzParam *)malloc(g_ChnNum * sizeof(SModPtzParam));
	if(g_PtzParam == NULL)
	{
		fprintf(stderr, "function: %s\n", __FUNCTION__);
		return -1;
	}
	memset(g_PtzParam, 0, g_ChnNum * sizeof(SModPtzParam));
		
	g_PtzRealCmd = (SPtzRealCmd *)malloc(g_ChnNum * sizeof(SPtzRealCmd));
	if(g_PtzRealCmd == NULL)
	{
		fprintf(stderr, "function: %s\n", __FUNCTION__);
		return -1;
	}
	memset(g_PtzRealCmd, 0, g_ChnNum * sizeof(SPtzRealCmd));
	
	memset(&g_PtzProtocolList, 0, sizeof(SPtzPtotocolList));
	
	//printf("ModPtzInit-2\n");
	
	L = lua_open();
	lua_baselibopen(L);
	lua_mathlibopen(L);
	lua_tablibopen(L);
	lua_strlibopen(L);
	lua_iolibopen(L);
	lua_dblibopen(L);
	
	//printf("ModPtzInit-3\n");
	
	if(0 != PtzGetProtocolSupport())
	{
		fprintf(stderr, "Get Protocol Supportted failed ! function: %s\n", __FUNCTION__);
		return -1;
	}
	
	UpdateProtocol(&g_PtzProtocolList.nNum, g_PtzProtocolList.pNameOfProtocol);
	
	if(0 != PtzGetCmdSupport())
	{
		fprintf(stderr, "Get Cmd Supportted failed ! function: %s\n", __FUNCTION__);
		return -1;
	}
	
	//printf("ModPtzInit-4\n");
	
	if(0 != PtzCreateMsgQueue())
	{
		fprintf(stderr, "PtzCreateMsgQueue error! function: %s\n", __FUNCTION__);
		return -1;
	}
	
	//printf("ModPtzInit-5\n");
	
	int i = 0;
	for(i = 0; i < sizeof(g_nResumePreset)/sizeof(g_nResumePreset[0]); i++)
	{
		g_nResumePreset[i] = 0;
		g_nResumeDelayTime[i] = 0;
		g_nLastCtrlTime[i] = time(NULL);
		g_nResumeFlag[i] = 0;
	}
	
	g_DeInited = 0;
	
	pthread_t id;
	if(0 != pthread_create(&id, NULL, (void*)PtzManagerFxn, NULL))
	{
		fprintf(stderr, "create pthread error!!\n");
		return -1;
	}
	
	g_Inited = 1;
	
	//printf("ModPtzInit-6\n");
	
	return 0;
//#endif
}

s32 ModPtzDeinit(void)
{
//#ifdef _NVR_PTZ_
//	return 0;
//#else
	if(g_DeInited)
	{
		printf("%s failed, mod has not been inited !\n", __FUNCTION__);
		return -1;
	}
	
	g_DeInited = 1;
	g_Inited = 0;
	
	usleep(100 * 1000);	
	
	if(0 != PtzDestroyMsgQueue())
	{
		fprintf(stderr, "PtzDestroyMsgQueue error! function: %s\n", __FUNCTION__);
		return -1;
	}
	
	lua_close(L);
	
	if(NULL != g_IsTouring)
	{
		free(g_IsTouring);
	}
	if(NULL != g_PtzParam)
	{
		free(g_PtzParam);
	}
	if(NULL != g_PtzRealCmd)
	{
		free(g_PtzRealCmd);
	}
	if(NULL != g_PtzCmdList)
	{
		u32 i = 0;
		for(i = 0; i < g_CmdNumSupport; i++)
		{
			if(NULL != g_PtzCmdList[i])
			{
				free(g_PtzCmdList[i]);
			}
		}
		free(g_PtzCmdList);		
	}
	if(NULL != g_PtzProtocolList.pNameOfProtocol)
	{
		u32 i = 0;
		for(i = 0; i < g_PtzProtocolList.nNum; i++)
		{
			if(NULL != g_PtzProtocolList.pNameOfProtocol[i])
			{
				free(g_PtzProtocolList.pNameOfProtocol[i]);
			}
		}
		free(g_PtzProtocolList.pNameOfProtocol);		
	}
	return 0;
//#endif
}

//csp modify 20130323
s32 ModPtzInitParam(SModPtzParam* para)
{
//#ifdef _NVR_PTZ_
//	return 0;
//#else
	if((NULL == para) || (para->nChn >= g_ChnNum))
	{
		printf("%s failed, Invalid param !\n", __FUNCTION__);
		return -1;
	}
	
	if(g_IsTouring[para->nChn] != 0)
	{
		printf("%s failed! The ptz is touring , please stop it first!\n", __FUNCTION__);
		return -1;
	}
	
	if(0 != PtzCheckSupportProtocol((char *)para->nProtocolName))
	{
		printf("%s error! \n", __FUNCTION__);
		return -1;
	}
	
	SPtzMsgHeader nMsg;
	nMsg.nChn = para->nChn;
	nMsg.emType = EM_PTZ_MSG_SET_PARAM;
	nMsg.pData = (void*)para;
	
	//printf("chn%d ModPtzInitParam-1\n",nMsg.nChn);
	
	memcpy(&g_PtzParam[nMsg.nChn], nMsg.pData, sizeof(SModPtzParam));
	
	//printf("chn%d ModPtzInitParam-2\n",nMsg.nChn);
	
	if(0 != PtzLoadProtocolInfo(nMsg.nChn))
	{
		printf("ModPtzInitParam PtzLoadProtocolInfo error!\n");
	}
	
	//printf("chn%d ModPtzInitParam-3\n",nMsg.nChn);
	
	return 0;
//#endif
}

s32 ModPtzSetParam(SModPtzParam* para)
{
//#ifdef _NVR_PTZ_
//	return 0;
//#else
	if((NULL == para) || (para->nChn >= g_ChnNum))
	{
		printf("%s failed, Invalid param !\n", __FUNCTION__);
		return -1;
	}
	
	if(g_IsTouring[para->nChn] != 0)
	{
		printf("%s failed! The ptz is touring , please stop it first!\n", __FUNCTION__);
		return -1;
	}
	
	if(0 != PtzCheckSupportProtocol((char *)para->nProtocolName))
	{
		printf("%s error! \n", __FUNCTION__);
		return -1;
	}
	
	SModPtzParam* sPtzpara = (SModPtzParam*)malloc(sizeof(SModPtzParam));
	if(NULL == sPtzpara)
	{
		printf("%s error!\n", __FUNCTION__);
		return -1;
	}
	memcpy(sPtzpara, para, sizeof(SModPtzParam));
	
	SPtzMsgHeader msg;
	msg.nChn = sPtzpara->nChn;
	msg.emType = EM_PTZ_MSG_SET_PARAM;
	msg.pData = (void*)sPtzpara;
	
	if(0 != PtzWriteMsgQueue(&msg))
	{
		fprintf(stderr, "PtzWriteMsgQueue error! function: %s\n", __FUNCTION__);
		free(sPtzpara);
		return -1;
	}
	
	return 0;
//#endif
}

s32 ModPtzCtrl(u8 nChn, EMPTZCMDTYPE cmd, s32* data)
{
	if(nChn >= g_ChnNum || !(*data>=0 && *data<=TOURPIONT_MAX))
	{
		printf("%s failed, Invalid param !\n", __FUNCTION__);
		return -1;
	}
	
	g_nLastCtrlTime[nChn] = time(NULL);
	
//#ifdef _NVR_PTZ_
	//printf("Ptz-1\n");
	if(IPC_CMD_PtzCtrl(nChn, cmd, data) < 0)
	{
		printf(" IPC_CMD_PtzCtrl is error!\n");
		return -1;
	}
	//printf("Ptz-2\n");
//#else
	//add by Lirl on Nov/09/2011
	if(!g_PtzParam[nChn].nEnable)
	{
		printf("***Chn:%d Enable is not Open***\n", nChn);
		return -1;
	}
	//end
	
	if((g_IsTouring[nChn] != 0) && (cmd != EM_PTZ_CMD_START_TOUR) && (cmd != EM_PTZ_CMD_STOP_TOUR) && (cmd != EM_PTZ_CMD_START_TOUR_AUTO) && (cmd != EM_PTZ_CMD_STOP_TOUR_AUTO))
	{
		printf("%s failed! The ptz is touring, please stop it first!\n", __FUNCTION__);
		return -1;
	}
	
	if((g_IsTouring[nChn] != 0) && ((cmd == EM_PTZ_CMD_START_TOUR) || (cmd == EM_PTZ_CMD_START_TOUR_AUTO)))
	{
		//g_IsTouring[nChn] = 0;
		
		//csp modify 20121018
		//g_ForceStopTouring[nChn] = 1;
	}
	
	#if 1//csp modify 20121018//避免越界
	void* pData = NULL;
	if(data != NULL)
	{
		pData = malloc(sizeof(u32));
		if(pData == NULL)
		{
			printf("%s error!\n", __FUNCTION__);
			return -1;
		}
		memcpy(pData, data, sizeof(u32));
	}
	#else
	void* pData = NULL;
	if(NULL != data)
	{
		pData = malloc(100);
		if(NULL == pData)
		{
			printf("%s error!\n", __FUNCTION__);
			return -1;
		}
		memcpy(pData, data, 100);
	}
	#endif
	
	SPtzMsgHeader msg;
	msg.nChn = nChn;
	msg.emType = cmd;
	msg.pData = pData;

	printf("ModPtzCtrl*********\n");
	if(0 != PtzWriteMsgQueue(&msg))
	{
		fprintf(stderr, "PtzWriteMsgQueue error! function: %s\n", __FUNCTION__);
		if(pData)
		{
			free(pData);
		}
		return -1;
	}
	
	g_nLastCtrlTime[nChn] = time(NULL);
	if(cmd != EM_PTZ_CMD_SETSPEED && cmd != EM_PTZ_CMD_NULL)
	{
		g_nResumeFlag[nChn] = 0;
	}
	
	return 0;
//#endif
}

s32 PtzCheckTouring(u8 nChn)
{
//#ifdef _NVR_PTZ_
//	return 0;
//#else
	if(g_IsTouring[nChn] != 0)
	{
		return -1;
	}
	else
	{
		return 0;
	}
//#endif
}

s32 ModPtzSetResumePreset(u8 nChn, s32 delay, s32 preset)
{
	if(nChn >= g_ChnNum)
	{
		printf("ModPtzSetResumePreset: chn%d error\n", nChn);
		return -1;
	}
	
	g_nResumePreset[nChn] = preset;
	g_nResumeDelayTime[nChn] = delay;
	printf("ModPtzSetResumePreset: chn%d delay=%d preset=%d\n", nChn, delay, preset);
	
	return 0;
}

