#include <sys/ioctl.h>
#include <unistd.h>
#include "zlib.h"
#include <linux/types.h>
#include <sys/types.h>
#include <mtd/mtd-user.h>
#include <string.h>

#include "common_basetypes.h"
#include "common_debug.h"
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include "circlebuf.h"

#include "mod_config.h"
#include "config.h"
#include "public.h"
#include "devcustom.h"
#include "userconfig.h"
#include "ToolCustom.h"

//#define CONFIG_TEST
#define DEBUG_PRINTF //printf("File:%s, Function:%s, Line:%d\n", __FILE__,__FUNCTION__,__LINE__);

static u8 g_Inited = 0;
static sem_t g_FileSem;
static SConfigDevInfo g_sConfigDevInfo;
static SModConfigDvrProperty g_sConfigDvrProperty;
static s8* gp_SysinfoFilePath = "/tmp/data/cfg/DvrCustom.ini";
static SConfigAll g_ConfigAllPara;
static SConfigAll g_ConfigDefaultAllPara;
static SConfigAllList g_ConfigAllList[EM_CONFIG_PARALIST_NUM];
static StrList g_ConfigStrList[EM_CONFIG_PARALIST_NUM];

static u8 g_BeAbleToDefault = 0;
static SCircleBufInfo g_s_MsgQueueInfo;
static u8 g_IsDefaulting = 0;
static u8 g_IsResuming = 0;

static char tmpMac[18] = {0};
static u8 bCustomMac = 1;

//static u8 g_IsSyncFileToFlash = 0;

s32 ConfigCreateMsgQueue(void)
{
	g_s_MsgQueueInfo.nBufId = 0;
	g_s_MsgQueueInfo.nLength = 1024 * sizeof(SConfigMsgHeader);
	
	s32 ret = 0;
	ret = CreateCircleBuf(&g_s_MsgQueueInfo);
	
	if(0 != ret)
	{
		fprintf(stderr, "function: %s\n", __FUNCTION__);
		return ret;
	}
	
	//printf("msg queue created succesful!!\n");
	
	return ret;
}

s32 ConfigDestroyMsgQueue(void)
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

s32 ConfigWriteMsgQueue(SConfigMsgHeader* pHeader)
{
	s32 ret = 0;
	ret = WriteDataToBuf(&g_s_MsgQueueInfo, (u8 *)pHeader, sizeof(SConfigMsgHeader));
	if(0 != ret)
	{
		fprintf(stderr, "function: %s\n", __FUNCTION__);
		return ret;
	}
	
	return ret;
}

s32 ConfigReadMsgQueue(SConfigMsgHeader* pHeader)
{
	s32 ret = 0;
	ret = ReadDataFromBuf(&g_s_MsgQueueInfo, (u8 *)pHeader, sizeof(SConfigMsgHeader));
	if(0 != ret)
	{
		fprintf(stderr, "function: %s\n", __FUNCTION__);
		return ret;
	}
	
	return ret;
}

s32 IniGetString(s8* strFilePath, s8* strSection, s8* strItem, s8* strOutValue, s32 nMaxStrLen)
{
	if(0 != PublicIniGetString(strFilePath, strSection, strItem, strOutValue, nMaxStrLen))
	{
		s8 tmp[100] = {0};
		strcpy(tmp, g_sConfigDevInfo.nDefaultConfPath);
		if(0 != PublicIniGetString(tmp, strSection, strItem, strOutValue, nMaxStrLen))
		{
			return -1;
		}
	}
	return 0;
}

s32 IniGetValue(s8* strFilePath, s8* strSection, s8* strItem, s32* nOutValue)
{
	if(0 != PublicIniGetValue(strFilePath, strSection, strItem, nOutValue))
	{
		s8 tmp[100] = {0};
		strcpy(tmp, g_sConfigDevInfo.nDefaultConfPath);
		if(0 != PublicIniGetValue(tmp, strSection, strItem, nOutValue))
		{
			return -1;
		}
	}
	return 0;
}

s32 IniGetValueU64(s8* strFilePath, s8* strSection, s8* strItem, u64* nOutValue)
{
	if(0 != PublicIniGetValueU64(strFilePath, strSection, strItem, nOutValue))
	{
		s8 tmp[100] = {0};
		strcpy(tmp, g_sConfigDevInfo.nDefaultConfPath);
		if(0 != PublicIniGetValueU64(tmp, strSection, strItem, nOutValue))
		{
			return -1;
		}
	}
	return 0;
}

s32 IniGetArrayString(s8* strFilePath, s8* strSection, s8* strItem, u16 nIndex, s8* strOutValue, s32 nMaxStrLen)
{
	if(0 != PublicIniGetArrayString(strFilePath, strSection, strItem, nIndex, strOutValue, nMaxStrLen))
	{
		s8 tmp[100] = {0};
		strcpy(tmp, g_sConfigDevInfo.nDefaultConfPath);
		if(0 != PublicIniGetArrayString(tmp, strSection, strItem, nIndex, strOutValue, nMaxStrLen))
		{
			return -1;
		}
	}
	return 0;
}

s32 IniGetArrayValue(s8* strFilePath, s8* strSection, s8* strItem, u16 nIndex, s32* nOutValue)
{
	if(0 != PublicIniGetArrayValue(strFilePath, strSection, strItem, nIndex, nOutValue))
	{
		s8 tmp[100] = {0};
		strcpy(tmp, g_sConfigDevInfo.nDefaultConfPath);
		if(0 != PublicIniGetArrayValue(tmp, strSection, strItem, nIndex, nOutValue))
		{
			return -1;
		}
	}
	return 0;
}

s32 IniGetArrayValueU64(s8* strFilePath, s8* strSection, s8* strItem, u16 nIndex, u64* nOutValue)
{
	if(0 != PublicIniGetArrayValueU64(strFilePath, strSection, strItem, nIndex, nOutValue))
	{
		s8 tmp[100] = {0};
		strcpy(tmp, g_sConfigDevInfo.nDefaultConfPath);
		if(0 != PublicIniGetArrayValueU64(tmp, strSection, strItem, nIndex, nOutValue))
		{
			return -1;
		}
	}
	return 0;
}

s32 GetStringFromIni(s8* strFilePath, s8* strSection, s8* strItem, s8* strOutValue, s32 nMaxStrLen)
{
	if((NULL == strFilePath) || (NULL == strSection) || (NULL == strItem) || (NULL == strOutValue) || (nMaxStrLen <= 0))
	{
		fprintf(stderr, "Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}

	int ret = IniGetString(strFilePath, strSection, strItem, strOutValue, nMaxStrLen);
	if(ret != 0)
	{
		return -1;
	}
	for(ret = 0; ret < nMaxStrLen; ret++)
	{
		if(*(strOutValue + ret) == '^')
		{
			*(strOutValue + ret) = ' ';
		}
	}
	if(0 == strcmp(strOutValue, "*****"))
	{
		strcpy(strOutValue, "");
	}
	return 0;
}

s32 GetArrayStringFromIni(s8* strFilePath, s8* strSection, s8* strItem, u16 nIndex, s8* strOutValue, s32 nMaxStrLen)
{
	if((NULL == strFilePath) || (NULL == strSection) || (NULL == strItem) || (NULL == strOutValue) || (nMaxStrLen <= 0))
	{
		fprintf(stderr, "Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}
	int ret = IniGetArrayString(strFilePath, strSection, strItem, nIndex, strOutValue, nMaxStrLen);
	if(ret != 0)
	{
		return -1;
	}
	for(ret = 0; ret < nMaxStrLen; ret++)
	{
		if(*(strOutValue + ret) == '^')
		{
			*(strOutValue + ret) = ' ';
		}
	}
	if(0 == strcmp(strOutValue, "*****"))
	{
		strcpy(strOutValue, "");
	}
	return 0;
}

s32 SetStringToIni(s8* strFilePath, s8* strSection, s8* strItem, s8* strNewValue)
{
	if((NULL == strFilePath) || (NULL == strSection) || (NULL == strItem) || (NULL == strNewValue))
	{
		fprintf(stderr, "Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}
	
	s8 tmp[100] = {0};
	memset(tmp, 0, sizeof(tmp));
	strcpy(tmp, strNewValue);
	int i;
	for(i = 0; i < sizeof(tmp); i++)
	{
		if(tmp[i] == ' ')
		{
			tmp[i] = '^';
		}
	}
	if(0 == strcmp(tmp, ""))
	{
		strcpy(tmp, "*****");
	}
	i = PublicIniSetString(strFilePath, strSection, strItem, tmp);
	return i;
}

s32 SetArrayStringToIni(s8* strFilePath, s8* strSection, s8* strItem, u16 nIndex, s8* strNewtValue)
{
	if((NULL == strFilePath) || (NULL == strSection) || (NULL == strItem) || (NULL == strNewtValue))
	{
		fprintf(stderr, "Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}
	
	s8 tmp[100] = {0};
	memset(tmp, 0, sizeof(tmp));
	strcpy(tmp, strNewtValue);
	int i;
	for(i = 0; i < sizeof(tmp); i++)
	{
		if(tmp[i] == ' ')
		{
			tmp[i] = '^';
		}
	}
	if(0 == strcmp(tmp, ""))
	{
		strcpy(tmp, "*****");
	}
	i = PublicIniSetArrayString(strFilePath, strSection, strItem, nIndex, tmp);
	return i;
}

s32 InitConfigInfo(void)
{
	memset(&g_sConfigDevInfo, 0, sizeof(g_sConfigDevInfo));
	memset(g_ConfigAllList, 0, sizeof(SConfigAllList));
	
	s32 tmp = 0;
	if(0 != PublicIniGetValue(gp_SysinfoFilePath, DVR_CONFIG_INFO, DVR_CONFIG_INFO_SAVETYPE, &tmp))
	{
		return -1;
	}
	g_sConfigDevInfo.nConfSaveType = tmp;

	if(0 != PublicIniGetValue(gp_SysinfoFilePath, DVR_CONFIG_INFO, DVR_CONFIG_INFO_FORMAT, &tmp))
	{
		return -1;
	}
	g_sConfigDevInfo.nConfFormat = tmp;

	if(0 != GetStringFromIni(gp_SysinfoFilePath, DVR_CONFIG_INFO, DVR_CONFIG_INFO_FILEPATH, g_sConfigDevInfo.nFilePath, sizeof(g_sConfigDevInfo.nFilePath)))
	{
		return -1;
	}
	
	if(0 != GetStringFromIni(gp_SysinfoFilePath, DVR_CONFIG_INFO, DVR_CONFIG_INFO_DEFAULTPATH, g_sConfigDevInfo.nDefaultConfPath, sizeof(g_sConfigDevInfo.nDefaultConfPath)))
	{
		return -1;
	}
	
	u8 flag_default = 0;
	if(0 == g_sConfigDevInfo.nConfSaveType)
	{
		if(0 != GetStringFromIni(gp_SysinfoFilePath, DVR_CONFIG_INFO, DVR_CONFIG_INFO_FLASHDEV, g_sConfigDevInfo.nDevPath, sizeof(g_sConfigDevInfo.nDevPath)))
		{
			return -1;
		}
		
		if(0 != PublicIniGetValue(gp_SysinfoFilePath, DVR_CONFIG_INFO, DVR_CONFIG_INFO_ZIP, &tmp))
		{
			return -1;
		}
		g_sConfigDevInfo.nConfZip = tmp;
		
		if(0 != PublicIniGetValue(gp_SysinfoFilePath, DVR_CONFIG_INFO, DVR_CONFIG_INFO_OFFSET, &g_sConfigDevInfo.nFlashOffset))
		{
			return -1;
		}
		
		if(0 != PublicIniGetValue(gp_SysinfoFilePath, DVR_CONFIG_INFO, DVR_CONFIG_INFO_LENGTH, &g_sConfigDevInfo.nFlashSizeLimit))
		{
			return -1;
		}
		
		tmp = open(g_sConfigDevInfo.nDevPath, O_SYNC | O_RDWR);
		if(tmp == -1)
		{
			return -1;
		}
		g_sConfigDevInfo.nDevFd = tmp;
		
		if(0 != ReadFlashToFile(g_sConfigDevInfo.nFilePath))
		{
			printf("ReadFlashToFile failed,use default config\n");
			flag_default = 1;
		}
	}
	else if(1 == g_sConfigDevInfo.nConfSaveType)
	{
		if(0 != access(g_sConfigDevInfo.nFilePath, F_OK))
		{
			flag_default = 1;
		}
	}
	else
	{
		printf("Unknown config save type!\n");
		return -1;
	}
	
	//flag_default = 1;//debug by lanston
	
	printf("use default config file:%s\n",flag_default?"yes":"no");
	
	if(flag_default)
	{	
		FILE* fp1 = NULL;
		FILE* fp2 = NULL;
		fp1 = fopen(g_sConfigDevInfo.nDefaultConfPath, "r");
		fp2 = fopen(g_sConfigDevInfo.nFilePath, "w");
		if((NULL == fp1) || (NULL == fp2))
		{
			printf("open file[%s]/[%s] error! function:%s\n", g_sConfigDevInfo.nDefaultConfPath, g_sConfigDevInfo.nFilePath, __FUNCTION__);
			
			//csp modify
			if(fp1 != NULL)
			{
				fclose(fp1);
			}
			if(fp2 != NULL)
			{
				fclose(fp2);
			}
			
			return -1;
		}
		int c;
		while((c = fgetc(fp1)) != EOF)
		{
			fputc(c, fp2);	
		}
		fclose(fp1);
		fclose(fp2);
		
		g_sConfigDevInfo.nReadInfoSuccess = 2;
		
		if(0 == g_sConfigDevInfo.nConfSaveType)
		{
			if(0 != WriteFileToFlash(g_sConfigDevInfo.nFilePath))
			{
				printf("InitConfigInfo:write config file to flash error! function:%s\n", __FUNCTION__);
				g_sConfigDevInfo.nReadInfoSuccess = 0;
				return -1;
			}
		}
	}
	else
	{
		g_sConfigDevInfo.nReadInfoSuccess = 1;
	}
	
	if(0 != sem_init(&g_sConfigDevInfo.sem, 0, 0))
	{
		g_sConfigDevInfo.nReadInfoSuccess = 0;
		return -1;
	}
	
	printf("Init Config Info OK!\n");
	
	return 0;
}

s32 GetCustomModel(u8* strModel, u8 nMaxLen)
{
	s32 i, nCount;
	SCustomInfo sCustomInfo;
	SCustomData sCustomData;
	
	if (0 == CustomOpen(NULL))
	{
		nCount = CustomGetCount();
		for (i = 0; i < nCount; i++)
		{
			CustomGetInfo(i, &sCustomInfo);

			if (sCustomInfo.emType == EM_CUSTOM_DEVICE_MODEL)
			{
				if (0 == CustomGetData(i, &sCustomData)) //保存型号
				{
					if (strlen(sCustomData.strData) < nMaxLen)
					{
						strcpy(strModel, sCustomData.strData);
						break;
					}
				}
			}
		}
		CustomClose(0);
	}

	return 0;
}

s32 InitDvrProperty(void)
{
	memset(&g_sConfigDvrProperty, 0, sizeof(g_sConfigDvrProperty));
	
	s32 tmp = 0;
	
	if(0 != GetStringFromIni(gp_SysinfoFilePath, DVR_PROPERTY, DVR_PROPERTY_VERSION, g_sConfigDvrProperty.nVersion, sizeof(g_sConfigDvrProperty.nVersion)))
	{
		PUBPRT("HERE ERR");
		return -1;
	}
	
	if(0 != GetStringFromIni(gp_SysinfoFilePath, DVR_PROPERTY, DVR_PROPERTY_MODEL, g_sConfigDvrProperty.nModel, sizeof(g_sConfigDvrProperty.nModel)))
	{
		PUBPRT("HERE ERR");
		return -1;
	}
	
	GetCustomModel(g_sConfigDvrProperty.nModel, sizeof(g_sConfigDvrProperty.nModel));
	
	//9624
	if(0 != GetStringFromIni(gp_SysinfoFilePath, DVR_PROPERTY, DVR_PROPERTY_PRODUCTNUMBER, g_sConfigDvrProperty.nproductnumber, sizeof(g_sConfigDvrProperty.nproductnumber)))
	{
		PUBPRT("HERE ERR");
		return -1;
	}
	else
	{
		printf("get product number is ok@!\n");
		printf("product number is %s\n",g_sConfigDvrProperty.nproductnumber);
	}
	
	if(0 != PublicIniGetValue(gp_SysinfoFilePath, DVR_PROPERTY, DVR_PROPERTY_PREVIEWNUM, &tmp))
	{
		PUBPRT("HERE ERR");
		return -1;
	}
	g_sConfigDvrProperty.nPreviewNum = tmp;
	
	if(0 != PublicIniGetValue(gp_SysinfoFilePath, DVR_PROPERTY, DVR_PROPERTY_VIDEOMAINNUM, &tmp))
	{
		PUBPRT("HERE ERR");
		return -1;
	}
	g_sConfigDvrProperty.nVidMainNum = tmp;
	
	if(0 != PublicIniGetValue(gp_SysinfoFilePath, DVR_PROPERTY, DVR_PROPERTY_VIDEOSUBNUM, &tmp))
	{
		PUBPRT("HERE ERR");
		return -1;
	}
	g_sConfigDvrProperty.nVidSubNum = tmp;
	
	if(0 != PublicIniGetValue(gp_SysinfoFilePath, DVR_PROPERTY, DVR_PROPERTY_AUDIONUM, &tmp))
	{
		PUBPRT("HERE ERR");
		return -1;
	}
	g_sConfigDvrProperty.nAudNum = tmp;
	
	if(0 != PublicIniGetValue(gp_SysinfoFilePath, DVR_PROPERTY, DVR_PROPERTY_SNAPNUM, &tmp))
	{
		PUBPRT("HERE ERR");
		return -1;
	}
	g_sConfigDvrProperty.nSnapNum = tmp;
	
	if(0 != PublicIniGetValue(gp_SysinfoFilePath, DVR_PROPERTY, DVR_PROPERTY_VOIPNUM, &tmp))
	{
		PUBPRT("HERE ERR");
		return -1;
	}
	g_sConfigDvrProperty.nVoipNum = tmp;
	
	if(0 != PublicIniGetValue(gp_SysinfoFilePath, DVR_PROPERTY, DVR_PROPERTY_RECNUM, &tmp))
	{
		PUBPRT("HERE ERR");
		return -1;
	}
	g_sConfigDvrProperty.nRecNum = tmp;
	
	if(0 != PublicIniGetValue(gp_SysinfoFilePath, DVR_PROPERTY, DVR_PROPERTY_SENSORNUM, &tmp))
	{
		PUBPRT("HERE ERR");
		return -1;
	}
	g_sConfigDvrProperty.nSensorNum = tmp;
	
	if(0 != PublicIniGetValue(gp_SysinfoFilePath, DVR_PROPERTY, DVR_PROPERTY_ALARMOUTNUM, &tmp))
	{
		PUBPRT("HERE ERR");
		return -1;
	}
	g_sConfigDvrProperty.nAlarmoutNum = tmp;
	
	if(0 != PublicIniGetValue(gp_SysinfoFilePath, DVR_PROPERTY, DVR_PROPERTY_BUZZNUM, &tmp))
	{
		PUBPRT("HERE ERR");
		return -1;
	}
	g_sConfigDvrProperty.nBuzzNum = tmp;
	
	if(0 != PublicIniGetValue(gp_SysinfoFilePath, DVR_PROPERTY, DVR_PROPERTY_MAXHDDNUM, &tmp))
	{
		PUBPRT("HERE ERR");
		return -1;
	}
	g_sConfigDvrProperty.nMaxHddNum = tmp;
	
	if(0 != PublicIniGetValue(gp_SysinfoFilePath, DVR_PROPERTY, DVR_PROPERTY_MAINFRAMESIZE, &tmp))
	{
		PUBPRT("HERE ERR");
		return -1;
	}
	g_sConfigDvrProperty.nMaxMainFrameSize = tmp;
	
	if(0 != PublicIniGetValue(gp_SysinfoFilePath, DVR_PROPERTY, DVR_PROPERTY_SUBFRAMESIZE, &tmp))
	{
		PUBPRT("HERE ERR");
		return -1;
	}
	g_sConfigDvrProperty.nMaxSubFrameSize = tmp;
	
	if(0 != PublicIniGetValue(gp_SysinfoFilePath, DVR_PROPERTY, DVR_PROPERTY_AUDIOFRAMESIZE, &tmp))
	{
		PUBPRT("HERE ERR");
		return -1;
	}
	g_sConfigDvrProperty.nMaxAudFrameSize = tmp;
	
	if(0 != PublicIniGetValue(gp_SysinfoFilePath, DVR_PROPERTY, DVR_PROPERTY_SNAPFRAMESIZE, &tmp))
	{
		PUBPRT("HERE ERR");
		return -1;
	}
	g_sConfigDvrProperty.nMaxSnapFrameSize = tmp;
	
	if(0 != PublicIniGetValue(gp_SysinfoFilePath, DVR_PROPERTY, DVR_PROPERTY_OSDOPERATOR, &tmp))
	{
		PUBPRT("HERE ERR");
		return -1;
	}
	g_sConfigDvrProperty.nOsdOperator = tmp;
	
	if(0 != PublicIniGetValue(gp_SysinfoFilePath, DVR_PROPERTY, DVR_PROPERTY_STROSDNUM, &tmp))
	{
		PUBPRT("HERE ERR");
		return -1;
	}
	g_sConfigDvrProperty.nStrOsdNum = tmp;
	
	if(0 != PublicIniGetValue(gp_SysinfoFilePath, DVR_PROPERTY, DVR_PROPERTY_IMGOSDNUM, &tmp))
	{
		PUBPRT("HERE ERR");
		return -1;
	}
	g_sConfigDvrProperty.nImgOsdNum = tmp;
	
	if(0 != PublicIniGetValue(gp_SysinfoFilePath, DVR_PROPERTY, DVR_PROPERTY_RECTOSDNUM, &tmp))
	{
		PUBPRT("HERE ERR");
		return -1;
	}
	g_sConfigDvrProperty.nRectOsdNum = tmp;
	
	u8 i = 0;
	for(i = 0; i < sizeof(g_sConfigDvrProperty.nRectOsdRgb); i++)
	{		
		if(0 != PublicIniGetArrayValue(gp_SysinfoFilePath, DVR_PROPERTY, DVR_PROPERTY_RECOSDRGB, i, &tmp))
		{
			PUBPRT("HERE ERR");
			return -1;
		}
		g_sConfigDvrProperty.nRectOsdRgb[i] = tmp;
	}
	
	if(0 != PublicIniGetValue(gp_SysinfoFilePath, DVR_PROPERTY, DVR_PROPERTY_LINEOSDNUM, &tmp))
	{
		PUBPRT("HERE ERR");
		return -1;
	}
	g_sConfigDvrProperty.nLineOsdNum = tmp;
	
	if(0 != PublicIniGetValue(gp_SysinfoFilePath, DVR_PROPERTY, DVR_PROPERTY_REFWIDTH, &tmp))
	{
		PUBPRT("HERE ERR");
		return -1;
	}
	g_sConfigDvrProperty.nRefWidth = tmp;
	
	if(0 != PublicIniGetValue(gp_SysinfoFilePath, DVR_PROPERTY, DVR_PROPERTY_REFHEIGHT, &tmp))
	{
		PUBPRT("HERE ERR");
		return -1;
	}
	g_sConfigDvrProperty.nRefHeight = tmp;
	
	if(0 != PublicIniGetValue(gp_SysinfoFilePath, DVR_PROPERTY, DVR_PROPERTY_AUDIO_ENCTYPE, &tmp))
	{
		PUBPRT("HERE ERR");
		return -1;
	}
	g_sConfigDvrProperty.nAudioEncType = tmp;
	
	if(0 != PublicIniGetValue(gp_SysinfoFilePath, DVR_PROPERTY, DVR_PROPERTY_AUDIO_SAMPLERATE, &tmp))
	{
		PUBPRT("HERE ERR");
		return -1;
	}
	g_sConfigDvrProperty.nAudioSampleRate = tmp;
	
	if(0 != PublicIniGetValue(gp_SysinfoFilePath, DVR_PROPERTY, DVR_PROPERTY_AUDIO_SAMPLEDURAT, &tmp))
	{
		PUBPRT("HERE ERR");
		return -1;
	}
	g_sConfigDvrProperty.nAudioSampleDuration = tmp;
	
	if(0 != PublicIniGetValue(gp_SysinfoFilePath, DVR_PROPERTY, DVR_PROPERTY_AUDIO_SAMPLESIZE, &tmp))
	{
		PUBPRT("HERE ERR");
		return -1;
	}
	g_sConfigDvrProperty.nAudioSampleSize = tmp;
	
	if(0 != PublicIniGetValue(gp_SysinfoFilePath, DVR_PROPERTY, DVR_PROPERTY_AUDIO_BITWIDTH, &tmp))
	{
		PUBPRT("HERE ERR");
		return -1;
	}
	g_sConfigDvrProperty.nAudioBitWidth = tmp;
	
	if(0 != PublicIniGetValue(gp_SysinfoFilePath, DVR_PROPERTY, DVR_PROPERTY_VOIP_ENCTYPE, &tmp))
	{
		PUBPRT("HERE ERR");
		return -1;
	}
	g_sConfigDvrProperty.nVoipEncType = tmp;
	
	if(0 != PublicIniGetValue(gp_SysinfoFilePath, DVR_PROPERTY, DVR_PROPERTY_VOIP_SAMPLERATE, &tmp))
	{
		PUBPRT("HERE ERR");
		return -1;
	}
	g_sConfigDvrProperty.nVoipSampleRate = tmp;
	
	if(0 != PublicIniGetValue(gp_SysinfoFilePath, DVR_PROPERTY, DVR_PROPERTY_VOIP_SAMPLEDURAT, &tmp))
	{
		PUBPRT("HERE ERR");
		return -1;
	}
	g_sConfigDvrProperty.nVoipSampleDuration = tmp;
	
	if(0 != PublicIniGetValue(gp_SysinfoFilePath, DVR_PROPERTY, DVR_PROPERTY_VOIP_SAMPLESIZE, &tmp))
	{
		PUBPRT("HERE ERR");
		return -1;
	}
	g_sConfigDvrProperty.nVoipSampleSize = tmp;
	
	if(0 != PublicIniGetValue(gp_SysinfoFilePath, DVR_PROPERTY, DVR_PROPERTY_VOIP_BITWIDTH, &tmp))
	{
		PUBPRT("HERE ERR");
		return -1;
	}
	g_sConfigDvrProperty.nVoipBitWidth = tmp;
	
	if(0 != PublicIniGetValue(gp_SysinfoFilePath, DVR_PROPERTY, DVR_PROPERTY_MD_AREA_ROWS, &tmp))
	{
		PUBPRT("HERE ERR");
		return -1;
	}
	g_sConfigDvrProperty.nMdAreaRows = tmp;
	
	if(0 != PublicIniGetValue(gp_SysinfoFilePath, DVR_PROPERTY, DVR_PROPERTY_MD_AREA_COLS, &tmp))
	{
		PUBPRT("HERE ERR");
		return -1;
	}
	g_sConfigDvrProperty.nMdAreaCols = tmp;
	
	if(0 != PublicIniGetValue(gp_SysinfoFilePath, DVR_PROPERTY, DVR_PROPERTY_MD_SENSE_MIN, &tmp))
	{
		PUBPRT("HERE ERR");
		return -1;
	}
	g_sConfigDvrProperty.nMdSenseMin = tmp;
	
	if(0 != PublicIniGetValue(gp_SysinfoFilePath, DVR_PROPERTY, DVR_PROPERTY_MD_SENSE_MAX, &tmp))
	{
		PUBPRT("HERE ERR");
		return -1;
	}
	g_sConfigDvrProperty.nMdSenseMax = tmp;
	
	if(0 != PublicIniGetValue(gp_SysinfoFilePath, DVR_PROPERTY, DVR_PROPERTY_SCH_SEGMENTS, &tmp))
	{
		PUBPRT("HERE ERR");
		return -1;
	}
	g_sConfigDvrProperty.nSchSegments = tmp;
	
	if(0 != PublicIniGetValue(gp_SysinfoFilePath, DVR_PROPERTY, DVR_PROPERTY_OUTPUT_SYNC, &tmp))
	{
		PUBPRT("HERE ERR");
		return -1;
	}
	g_sConfigDvrProperty.nOutputSync = tmp;

	//区别NVR和解码器--- 跃天
	if(0 != PublicIniGetValue(gp_SysinfoFilePath, DVR_PROPERTY, DVR_PROPERTY_DEVICE_TYPE, &tmp))
	{
		PUBPRT("HERE ERR");
		return -1;
	}
	g_sConfigDvrProperty.nNVROrDecoder = tmp;
	printf("%s NVROrDecoder: %d\n", __func__, tmp);
	printf("Init DVR property OK!\n");
	
	return 0;
}


s32 WriteToEEPROM(int fd,unsigned int offset,char *buffer,unsigned int len)
{
	s32 ret = 0;
	if(fd == -1 || buffer == NULL)
	{
		return -1;
	}
	if(offset < (g_sConfigDevInfo.nFlashSizeLimit>>1))
	{
		if((offset + len) > (g_sConfigDevInfo.nFlashSizeLimit>>1))
		{
			printf("Warnning: will have no space to backup config!\n");
			ret = -2;
		}
	}
	if(offset + len > g_sConfigDevInfo.nFlashSizeLimit)
	{
		return -1;
	}
	
	offset += g_sConfigDevInfo.nFlashOffset;
	
	off_t rett = lseek(fd,offset,SEEK_SET);
	if(rett == -1)
	{
		return -1;
	}
	ssize_t size = write(fd,buffer,len);
	if(size != len)
	{
		return -1;
	}
	
	//fdatasync(fd);
	fsync(fd);//csp modify
	//sync();//csp modify
	
	usleep(1);
	
	return ret;
}

s32 SetZipConf(u32 offset, s8* pData, u32 len)
{
	SConfigFlashHead head;
	head.nFileSize = len;
	char zip_head_buf[ZIP_BUF_LEN];
	u32 size_buf = ZIP_BUF_LEN;

	printf("%s offset: %u, zip_head_buf: %p, size_buf: %u, pdata: %p, head.nFileSize: %u\n", __func__, offset, zip_head_buf, size_buf, pData, (unsigned long)head.nFileSize);
	int rec = compress(zip_head_buf,(unsigned long*)&size_buf,(char *)pData,(unsigned long)head.nFileSize);
	if(rec!= Z_OK)
	{
		printf("compress Error! rec = %d, errno: %d, %s\n", rec, errno, strerror(errno));
		printf("%s offset: %u, zip_head_buf: %p, size_buf: %u, pdata: %p, head.nFileSize: %u\n", __func__, offset, zip_head_buf, size_buf, pData, (unsigned long)head.nFileSize);
		return -1;
	}
	head.nConfSize = CT_LE_L((u32)size_buf);
	head.nCheckSum = head.nFileSize - head.nConfSize;
	
	u32 tmp = 0;
	u32 nNumOfZero = 0;
	u32 i;
	for(i = 0; i < len; i++)
	{
		tmp += pData[i];
		if((pData[i] == 0) || (pData[i] == '0'))
		{
			nNumOfZero++;
		}
	}
	head.nCheckSum += tmp;
	head.nCheckSum |= (nNumOfZero << 16);
	
	s32 ret = 0;
	ret = WriteToEEPROM(g_sConfigDevInfo.nDevFd,offset+64,zip_head_buf,size_buf);
	WriteToEEPROM(g_sConfigDevInfo.nDevFd,offset,(char *)&head,sizeof(head));
	
	return ret;
}

s32 WriteFileToFlash(s8* pFilePath)
{
	if(NULL == pFilePath)
	{
		printf("Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}
	
	if(0 == g_sConfigDevInfo.nReadInfoSuccess)
	{
		printf("Get config device info failed! function:%s\n", __FUNCTION__);
		return -1;
	}
	
	FILE* fp = fopen(pFilePath, "r");
	if(NULL == fp)
	{
		printf("open file[%s] error! function:%s\n", pFilePath, __FUNCTION__);
		return -1;
	}
	
	int rtn;
	
	//csp modify
	struct mtd_info_user mtd;
	memset(&mtd, 0, sizeof(struct mtd_info_user));
	rtn = ioctl(g_sConfigDevInfo.nDevFd, MEMGETINFO, &mtd);
	//printf("MEMGETINFO=%d,mtdsize=%d,erasesize=%d\n",rtn,mtd.size,mtd.erasesize);
	
	struct erase_info_user erase;
	erase.start = g_sConfigDevInfo.nFlashOffset;
	erase.length = g_sConfigDevInfo.nFlashSizeLimit >> 1;//g_sConfigDevInfo.nFlashSizeLimit;//g_sConfigDevInfo.nFlashSizeLimit >> 1;//csp modify
	rtn = ioctl(g_sConfigDevInfo.nDevFd, MEMERASE, (char*)&erase);
	if(rtn < 0)
	{
		perror("MEMERASE");
		printf("MEMERASE-1 failed <fd=%d,start=%d,length=%d> in function:%s\n", g_sConfigDevInfo.nDevFd, erase.start, erase.length, __FUNCTION__);
		fclose(fp);
		return -1;
	}
	
	#if 1
	fseek(fp,0,SEEK_END);
	s32 len = ftell(fp);
	if (len < 0)
	{
		printf("%s ftell failed, len: %d, errno: %d, %s\n", __func__, len, errno, strerror(errno));
		fclose(fp);
		return -1;
	}
	
	fseek(fp,0,SEEK_SET);
	s8* pData = malloc(len);
	fread(pData,len,1,fp);
	#else
	s8* pData = calloc(1024, 50);
	u32 len = 0;
	int c;
	while((c = fgetc(fp)) != EOF)
	{
		if((len) && ((len%(1024*50)) == 0))
		{
			pData = realloc(pData, 1024*50*(len/(1024*50)+1));
		}
		*(pData+len) = c;
		len++;		
	}
	#endif
	
	s32 ret = SetZipConf(0, pData, len);
	if (-1 == ret)
	{
		printf("SetZipConf-1 failed! function:%s\n", __FUNCTION__);
		free(pData);
		fclose(fp);
		return -1;
	}
	if(-2 == ret)
	{
		printf("Warning: backup config to flash [No Space]\n");
	}
	else
	{
		#if 1//csp modify 20140812
		erase.start = g_sConfigDevInfo.nFlashOffset + (g_sConfigDevInfo.nFlashSizeLimit >> 1);
		erase.length = g_sConfigDevInfo.nFlashSizeLimit >> 1;
		rtn = ioctl(g_sConfigDevInfo.nDevFd, MEMERASE, (char*)&erase);
		if(rtn < 0)
		{
			printf("MEMERASE-2 failed! function:%s\n", __FUNCTION__);
			fclose(fp);
			free(pData);
			return -1;
		}
		#endif
		
		if(0 != SetZipConf(g_sConfigDevInfo.nFlashSizeLimit>>1, pData, len))
		{
			printf("Warning: backup config to flash error!\n");
		}
	}
	
	free(pData);
	fclose(fp);
	
	return 0;
}

void WriteConfigFileToFlashFxn(void)
{
	printf("$$$$$$$$$$$$$$$$$$WriteConfigFileToFlashFxn id:%d\n",getpid());
	
	time_t start = time(NULL);
	time_t now = 0;
	int value = -1;
	
	printf("pthread WriteConfigFileToFlashFxn going......\n");
	
	while(1)
	{
		if(g_Inited == 0)
		{
			break;
		}
		
		time(&now);
		if(now >= start && now - start < 2)//csp modify
		{
			usleep(500*1000);
			continue;
		}
		else//csp modify
		{
			usleep(1);
		}
		start = now;
		
		sem_getvalue(&g_sConfigDevInfo.sem, &value);
		if(value == 0)
		{
			continue;
		}
		sem_wait(&g_sConfigDevInfo.sem);
		
		if(g_IsDefaulting || g_IsResuming)
		{
			continue;
		}
		
		sem_wait(&g_FileSem);
		
		if(0 != WriteFileToFlash(g_sConfigDevInfo.nFilePath))
		{
			printf("write config file to flash error! function:%s\n", __FUNCTION__);
		}
		
		sem_post(&g_FileSem);
		
		//printf("WriteFileToFlash OK! function: %s\n", __FUNCTION__);
		
		//if(g_IsSyncFileToFlash)
		//{
		//	g_IsSyncFileToFlash = 0;
		//}
	}
}

s32 ReadFromEEPROM(int fd,unsigned int offset,char *buffer,unsigned int len)
{
	if(fd == -1 || buffer == NULL)
	{
		return -1;
	}
	if(offset + len > g_sConfigDevInfo.nFlashSizeLimit)
	{
		return -1;
	}

	offset += g_sConfigDevInfo.nFlashOffset;

	off_t ret1 = lseek(fd,0,SEEK_END);
	if(ret1 == -1 || offset + len > ret1)
	{
		return -1;
	}
	off_t ret2 = lseek(fd,offset,SEEK_SET);
	if(ret2 == -1)
	{
		return -1;
	}
	ssize_t size = read(fd,buffer,len);
	if(size != len)
	{
		return -1;
	}
	return 0;
}

s32 ReadZipConf(u32 offset, s8** pData, u32* length)
{
	if(NULL == length)
	{
		printf("Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}
	
	SConfigFlashHead head;
	if(0 != ReadFromEEPROM(g_sConfigDevInfo.nDevFd,offset,(char *)&head,sizeof(head)))
	{
		return -1;
	}
	head.nConfSize = CF_LE_L(head.nConfSize);
	head.nFileSize = CF_LE_L(head.nFileSize);
	head.nCheckSum= CF_LE_L(head.nCheckSum);
	printf("config head info: size[%u][%u], checksum[0x%08x]\n", head.nConfSize, head.nFileSize, head.nCheckSum);
	
	if((0!=head.nConfSize)&&(0!=head.nFileSize)&&(head.nConfSize<=ZIP_BUF_LEN))
	{
		char zip_head_buf[ZIP_BUF_LEN];
		u32 unc_size = head.nFileSize;
		if(0 != ReadFromEEPROM(g_sConfigDevInfo.nDevFd,offset+64,zip_head_buf,head.nConfSize))
		{
			return -1;
		}
		*length = head.nFileSize;
		*pData = (char*)calloc(head.nFileSize, 1);
		int rec = uncompress((char *)*pData,(unsigned long *)&unc_size,zip_head_buf,(unsigned long)(head.nConfSize));
		if(rec!= Z_OK)
		{
			printf("uncompress Error! \n");
			return -1;
		}
		
		u32 tmp = 0;
		u32 nNumOfZero = 0;
		u32 i;
		for(i = 0; i < head.nFileSize; i++)
		{
			tmp += (*(*pData + i));
			if((*(*pData + i) == 0) || (*(*pData + i) == '0'))
			{
				nNumOfZero++;
			}
		}
		tmp += (head.nFileSize - head.nConfSize);
		tmp |= (nNumOfZero << 16);
		
		if(tmp != head.nCheckSum)
		{
			printf("flash checksum failed!\n");
			return -1;
		}
		printf("flash checksum OK! [0x%08x]\n", tmp);
		return 0;
	}
	else
	{
		//printf("head.nConfSize Error!\n");
	}
	
	return -1;
}

s32 ReadFlashToFile(s8* pFilePath)
{
	if(NULL == pFilePath)
	{
		printf("Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}

	s8* pData = NULL;
	u32 len = 0;
	if(0 != ReadZipConf(0, &pData, &len))
	{
		printf("Warnning: system will have to use config backuped!\n");
		if(0 != ReadZipConf(g_sConfigDevInfo.nFlashSizeLimit>>1, &pData, &len))
		{
			printf("Warnning: read config backuped failed!\n");
			return -1;
		}
	}
	if(NULL == pData)
	{
		return -1;
	}
	
	FILE* fp;
	fp = fopen(pFilePath, "w");
	if(NULL == fp)
	{
		printf("open file[%s] error! function:%s\n", pFilePath, __FUNCTION__);
		return -1;
	}
	u32 i;
	for(i = 0; i < len; i++)
	{
		fputc(*(pData+i), fp);
	}
	fclose(fp);
	free(pData);

	return 0;
}

s32 GetParaListStrFlag(EMCONFIGPARALISTTYPE emParaType, s8* strItem)
{
	if(NULL == strItem)
	{
		return -1;
	}
	
    switch(emParaType)
    {
        case EM_CONFIG_PARALIST_LANGSUP: // 支持的语言列表
            sprintf(strItem, "%s", DVR_CFGPATTERN_LANGSUP);
            break;
        case EM_CONFIG_PARALIST_LANGSRC: // 支持的语言资源文件列表 stringonly
            sprintf(strItem, "%s", DVR_CFGPATTERN_LANGSRC);
            break;
        case EM_CONFIG_PARALIST_PREVIEWMODE: // 支持的预览模式
            sprintf(strItem, "%s", DVR_CFGPATTERN_PREVIEWMODE);
            break;
        case EM_CONFIG_PARALIST_BITRATE: // 支持的位率列表
            sprintf(strItem, "%s", DVR_CFGPATTERN_BITRATE);
            break;
        case EM_CONFIG_PARALIST_BITTYPE: // 位率类型列表
             sprintf(strItem, "%s", DVR_CFGPATTERN_BITTYPE);
            break;
        case EM_CONFIG_PARALIST_PICLEVEL: // 图像质量列表
            sprintf(strItem, "%s", DVR_CFGPATTERN_PICLEVEL);
            break;
        case EM_CONFIG_PARALIST_VMAINRESOLH_NUM: // 主码流通用支持的分辨率列表//cw_9508S
            sprintf(strItem, "%s", DVR_CFGPATTERN_VMAINRESOLH_NUM);
            break;
        case EM_CONFIG_PARALIST_VMAINRESOL: // 主码流通用支持的分辨率列表
            sprintf(strItem, "%s", DVR_CFGPATTERN_VMAINRESOL);
            break;
        case EM_CONFIG_PARALIST_VMAINRESOL2: // 主码流通用支持的分辨率列表
            sprintf(strItem, "%s", DVR_CFGPATTERN_VMAINRESOL2);
            break;
        case EM_CONFIG_PARALIST_VSUBRESOL: // 子码流通用支持的分辨率列表
            sprintf(strItem, "%s", DVR_CFGPATTERN_VSUBRESOL);
            break;
	 case EM_CONFIG_PARALIST_VMOBRESOL: // 手机码流通用支持的分辨率列表
            sprintf(strItem, "%s", DVR_CFGPATTERN_VMOBRESOL);
            break;
        case EM_CONFIG_PARALIST_FPSPAL: // P制常用下支持的帧率列表
            sprintf(strItem, "%s", DVR_CFGPATTERN_FPSPAL);
            break;
        case EM_CONFIG_PARALIST_FPSNTSC: // N制常用下支持的帧率列表
            sprintf(strItem, "%s", DVR_CFGPATTERN_FPSNTSC);
            break;
        case EM_CONFIG_PARALIST_FPSPALD1: // P制D1下支持的帧率列表
            sprintf(strItem, "%s", DVR_CFGPATTERN_FPSPALD1);
            break;
        case EM_CONFIG_PARALIST_FPSNTSCD1: // N制D1下支持的帧率列表
            sprintf(strItem, "%s", DVR_CFGPATTERN_FPSNTSCD1);
            break;
        case EM_CONFIG_PARALIST_FPSPALCIF: // P制Cif下支持的帧率列表
            sprintf(strItem, "%s", DVR_CFGPATTERN_FPSPALCIF);
            break;
        case EM_CONFIG_PARALIST_FPSNTSCCIF: // N制Cif下支持的帧率列表
            sprintf(strItem, "%s", DVR_CFGPATTERN_FPSNTSCCIF);
            break;
        case EM_CONFIG_PARALIST_STREAMTYPE: // 录像码流类型列表
            sprintf(strItem, "%s", DVR_CFGPATTERN_STREAMTYPE);
            break;
        case EM_CONFIG_PARALIST_VIDEOSTANDARD: // 视频制式列表
            sprintf(strItem, "%s", DVR_CFGPATTERN_VIDEOSTANDARD);
            break;
        case EM_CONFIG_PARALIST_OUTPUT: // 视频输出列表
            sprintf(strItem, "%s", DVR_CFGPATTERN_OUTPUT);
            break;
			
	case EM_CONFIG_PARALIST_SWITCHPICTURE: //轮巡画面选择 
            sprintf(strItem, "%s", DVR_CFGPATTERN_SWITCHPICTURE);
            break;
			
        case EM_CONFIG_PARALIST_LOCKTIME: // 锁定超时间选项列表
            sprintf(strItem, "%s", DVR_CFGPATTERN_LOCKTIME);
            break;
        case EM_CONFIG_PARALIST_DATEFORMAT: // 日期格式列表
            sprintf(strItem, "%s", DVR_CFGPATTERN_DATEFORMAT);
            break;
        case EM_CONFIG_PARALIST_TIMESTANDARD: // 时间制式列表
            sprintf(strItem, "%s", DVR_CFGPATTERN_TIMESTANDARD);
            break;
        case EM_CONFIG_PARALIST_SITEMAINOUT: // 现场主输出列表
            sprintf(strItem, "%s", DVR_CFGPATTERN_SITEMAINOUT);
            break;
        case EM_CONFIG_PARALIST_INTERVAL: // 时间间隔列表
            sprintf(strItem, "%s", DVR_CFGPATTERN_INTERVAL);
            break;
        case EM_CONFIG_PARALIST_SENSORTYPE: // 传感器类型列表
            sprintf(strItem, "%s", DVR_CFGPATTERN_SENSORTYPE);
            break;
        case EM_CONFIG_PARALIST_ALARMOUTTYPE: // 报警输出类型列表
            sprintf(strItem, "%s", DVR_CFGPATTERN_ALARMOUTTYPE);
            break;
        case EM_CONFIG_PARALIST_DELAY: // 延时时间选项列表
            sprintf(strItem, "%s", DVR_CFGPATTERN_DELAY);
            break;
        case EM_CONFIG_PARALIST_PRETIME: // 预录时间选项列表
            sprintf(strItem, "%s", DVR_CFGPATTERN_PRETIME);
            break;
        case EM_CONFIG_PARALIST_RECDELAY: // 录像延时时间选项列表
            sprintf(strItem, "%s", DVR_CFGPATTERN_RECDELAY);
            break;
        case EM_CONFIG_PARALIST_RECEXPIRE: // 录像过期天数选项列表
            sprintf(strItem, "%s", DVR_CFGPATTERN_RECEXPIRE);
            break;
        case EM_CONFIG_PARALIST_SCHEDULETYP: // 布防类型选项列表
            sprintf(strItem, "%s", DVR_CFGPATTERN_SCHEDULETYP);
            break;
        case EM_CONFIG_PARALIST_PTZLINKTYPE: // 云台联动类型选项列表
            sprintf(strItem, "%s", DVR_CFGPATTERN_PTZLINKTYPE);
            break;
        case EM_CONFIG_PARALIST_BAUDRATE: // 波特率选项列表
            sprintf(strItem, "%s", DVR_CFGPATTERN_BAUDRATE);
            break;
        case EM_CONFIG_PARALIST_DATABIT: // 数据位类型选项列表
            sprintf(strItem, "%s", DVR_CFGPATTERN_DATABIT);
            break;
        case EM_CONFIG_PARALIST_STOPBIT: // 停止位类型选项列表
            sprintf(strItem, "%s", DVR_CFGPATTERN_STOPBIT);
            break;
        case EM_CONFIG_PARALIST_CHECKTYPE: // 校验类型类型选项列表
            sprintf(strItem, "%s", DVR_CFGPATTERN_CHECKTYPE);
            break;
        case EM_CONFIG_PARALIST_FLOWCTRLTYPE: // 流控方式类型选项列表
            sprintf(strItem, "%s", DVR_CFGPATTERN_FLOWCTRLTYPE);
            break;
        case EM_CONFIG_PARALIST_DDNSDOMAIN: // DDNSDomain选项列表
            sprintf(strItem, "%s", DVR_CFGPATTERN_DDNSDOMAIN);
            break;
        case EM_CONFIG_PARALIST_DDNSIPUPDATEITVL: // DDNS IP更新间隔选项列表
            sprintf(strItem, "%s", DVR_CFGPATTERN_DDNSUPINTVL);
            break;
        case EM_CONFIG_PARALIST_VIDEOSRCTYPE: // 视频源类型选项列表
            sprintf(strItem, "%s", DVR_CFGPATTERN_VIDEOSRCTYPE);
            break;
			
	case EM_CONFIG_PARALIST_RECORD_SNAP_RES: //抓图分辨率
		sprintf(strItem, "%s", DVR_CFGPATTERN_SNAP_RES);
            	break;
	case EM_CONFIG_PARALIST_RECORD_SNAP_QUALITY: //抓图质量
		sprintf(strItem, "%s", DVR_CFGPATTERN_SNAP_QUALITY);
            	break;
	case EM_CONFIG_PARALIST_RECORD_SNAP_INTERVAL: //抓图时间间隔	
		sprintf(strItem, "%s", DVR_CFGPATTERN_SNAP_INTERVAL);
            	break;
	case EM_CONFIG_PARALIST_RepairName: //报修系统名称
		sprintf(strItem, "%s", DVR_CFGPATTERN_RepairName);
            	break;
	case EM_CONFIG_PARALIST_RepairType: //报修系统类型
		sprintf(strItem, "%s", DVR_CFGPATTERN_RepairType);
            	break;
	case EM_CONFIG_PARALIST_Maintain: //维护维修
		sprintf(strItem, "%s", DVR_CFGPATTERN_Maintain);
            	break;
	case EM_CONFIG_PARALIST_Test: //测试
		sprintf(strItem, "%s", DVR_CFGPATTERN_Test);
            	break;
	case EM_CONFIG_PARALIST_AlarmInput: //报警图片配置: 输入端子
		sprintf(strItem, "%s", DVR_CFGPATTERN_AlarmInput);
            	break;
        default:
            return -1;
    }
	
    return 0;
}

s32 ConfigLoadAllParaToMem(SConfigAll* para, s8* pFilePath)
{
	if((para == NULL) || (pFilePath == NULL))
	{
		printf("Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}
	memset(para, 0, sizeof(SConfigAll));
	
	u8 chnnum = g_sConfigDvrProperty.nPreviewNum;
	//
	para->psSystemPara = calloc(1, sizeof(SModConfigSystemParam));
	para->psMainTainPara= calloc(1, sizeof(SModConfigMainTainParam));
	para->psPreviewPara = calloc(1, sizeof(SModConfigPreviewParam));
	para->psImagePara = calloc(chnnum, sizeof(SModConfigImagePara));
	para->psVoImagePara = calloc(2, sizeof(SModConfigImagePara));
	para->psMainOsdPara = calloc(1, sizeof(SModConfigMainOsdParam));
	para->psStrOsdPara = calloc(chnnum, sizeof(SModConfigStrOsdpara));
	para->psRectOsdPara = calloc(chnnum, sizeof(SModConfigRectOsdpara));
	para->psVideoMainPara = calloc(g_sConfigDvrProperty.nVidMainNum, sizeof(SModConfigVideoParam));
	para->psVideoSubPara = calloc(g_sConfigDvrProperty.nVidSubNum, sizeof(SModConfigVideoParam));
	para->psVideoMobPara = calloc(g_sConfigDvrProperty.nVidSubNum, sizeof(SModConfigVideoParam));
	
	para->psRecordPara = calloc(g_sConfigDvrProperty.nRecNum, sizeof(SModConfigRecordParam));
	para->psRecTimerSchPara = calloc(g_sConfigDvrProperty.nRecNum, sizeof(SModConfigRecTimerSch));
	para->psRecVMSchPara = calloc(g_sConfigDvrProperty.nRecNum, sizeof(SModConfigRecVMSch));
	para->psRecSensorSchPara = calloc(g_sConfigDvrProperty.nRecNum, sizeof(SModConfigRecAlarmInSch));
	para->psSensorPara = calloc(g_sConfigDvrProperty.nSensorNum, sizeof(SModConfigSensorParam));
	//yaogang modify 20141010
	para->psIPCCoverPara = calloc(g_sConfigDvrProperty.nVidMainNum, sizeof(SModConfigIPCExtSensorParam));
	para->psIPCExtSensorPara = calloc(g_sConfigDvrProperty.nVidMainNum, sizeof(SModConfigIPCExtSensorParam));
	para->ps485ExtSensorPara = calloc(g_sConfigDvrProperty.nVidMainNum, sizeof(SModConfigIPCExtSensorParam));
	para->psHDDPara = calloc(1, sizeof(SModConfigHDDParam));
	
	para->psVMotionPara = calloc(chnnum, sizeof(SModConfigVMotionParam));
	para->psVLostPara = calloc(chnnum, sizeof(SModConfigVLostParam));
	para->psVBlindPara = calloc(chnnum, sizeof(SModConfigVBlindParam));
	para->psAlarmOutPara = calloc(g_sConfigDvrProperty.nAlarmoutNum, sizeof(SModConfigAlarmOutParam));
	para->psBuzzPara = calloc(g_sConfigDvrProperty.nBuzzNum, sizeof(SModConfigBuzzParam));
	para->psSensorSchPara = calloc(g_sConfigDvrProperty.nSensorNum, sizeof(SModConfigSensorSch));
	para->psIPCExtSensorSchPara = calloc(g_sConfigDvrProperty.nVidMainNum, sizeof(SModConfigSensorSch));
	para->psVMotionSchPara = calloc(chnnum, sizeof(SModConfigVMotionSch));
	para->psAlarmOutSchPara = calloc(g_sConfigDvrProperty.nAlarmoutNum, sizeof(SModConfigAlarmOutSch));
	para->psPtzPara = calloc(chnnum, sizeof(SModConfigPtzParam));
	para->psUserPara = calloc(1, sizeof(SModConfigUserPara));
	para->psNetPara = calloc(1, sizeof(SModConfigNetParam));
	para->psIPCameraPara = calloc(chnnum, sizeof(SModConfigIPCameraParam));//NVR used

	para->psSnapPara = calloc(chnnum, sizeof(SModConfigSnapChnPara));
	//yaogang modify 20141209 深广平台参数
	para->psSGPara = calloc(1, sizeof(SModConfigSGParam));
	para->psSGDailyPara = calloc(1, sizeof(SModDailyPicCFG));
	para->psSGAlarmPara = calloc(g_sConfigDvrProperty.nSensorNum + g_sConfigDvrProperty.nVidMainNum,
								sizeof(SModAlarmPicCFG));

	para->psBadDiskListPara = calloc(g_sConfigDvrProperty.nMaxHddNum, sizeof(SModConfigBadDisk));
	
	s32 ret = 0;
	s32 i = 0;
	s8 file[100];
	memset(file, 0, sizeof(file));
	strcpy(file, pFilePath);
	//printf("time:%d, line:%d\n", time(NULL), __LINE__);
	
	if(0 != ConfigGetSystemParam(para->psSystemPara, file))
	{		
		ret = -1;
	}
	if(ret==-1)
	{
		printf("time:%ld, line:%d\n", time(NULL), __LINE__);
	}
	if(0 != ConfigGetMainTainParam(para->psMainTainPara,file))
	{
		ret = -1;
	}
	if(ret==-1)
	{
		printf("time:%ld, line:%d\n", time(NULL), __LINE__);
	}
	if(0 != ConfigGetPreviewParam(para->psPreviewPara, file))
	{		
		ret = -1;
	}
	if(ret==-1)
	{
		printf("time:%ld, line:%d\n", time(NULL), __LINE__);
	}
	for(i = 0; i < chnnum; i++)
	{
		if(0 != ConfigGetImageParam(&para->psImagePara[i],i,file))
		{
			ret = -1;
		}
	}
	if(ret==-1)
	{
		printf("time:%ld, line:%d\n", time(NULL), __LINE__);
	}
	if(0 != ConfigGetVoImageParam(&para->psVoImagePara[0],0,file))
	{
		ret = -1;
	}
	if(ret==-1)
	{
		printf("time:%ld, line:%d\n", time(NULL), __LINE__);
	}
	if(0 != ConfigGetMainOsdParam(para->psMainOsdPara, file))
	{		
		ret = -1;
	}
	if(ret==-1)
	{
		printf("time:%ld, line:%d\n", time(NULL), __LINE__);
	}
	for(i = 0; i < chnnum; i++)
	{
		if(0 != ConfigGetStrOsdParam(&para->psStrOsdPara[i],i,file))
		{
			ret = -1;
		}
	}	
	if(ret==-1)
	{
		printf("time:%ld, line:%d\n", time(NULL), __LINE__);
	}
	for(i = 0; i < chnnum; i++)
	{
		if(0 != ConfigGetRectOsdParam(&para->psRectOsdPara[i],i,file))
		{
			ret = -1;
		}
	}
	if(ret==-1)
	{
		printf("time:%ld, line:%d\n", time(NULL), __LINE__);
	}
	for(i = 0; i < g_sConfigDvrProperty.nVidMainNum; i++)
	{
		if(0 != ConfigGetMainEncodeParam(&para->psVideoMainPara[i],i,file))
		{
			ret = -1;
		}
	}
	if(ret==-1)
	{
		printf("time:%ld, line:%d\n", time(NULL), __LINE__);
	}
	for(i = 0; i < g_sConfigDvrProperty.nVidSubNum; i++)
	{
		if(0 != ConfigGetSubEncodeParam(&para->psVideoSubPara[i],i,file))
		{
			ret = -1;
		}
	}
	for(i = 0; i < g_sConfigDvrProperty.nVidSubNum; i++)
	{
		if(0 != ConfigGetMobEncodeParam(&para->psVideoMobPara[i],i,file))
		{
			ret = -1;
		}
	}
	if(ret==-1)
	{
		printf("time:%ld, line:%d\n", time(NULL), __LINE__);
	}
	for(i = 0; i < g_sConfigDvrProperty.nRecNum; i++)
	{
		if(0 != ConfigGetRecordParam(&para->psRecordPara[i],i,file))
		{
			ret = -1;
		}
	}
	if(ret==-1)
	{
		printf("time:%ld, line:%d\n", time(NULL), __LINE__);
	}
	for(i = 0; i < g_sConfigDvrProperty.nRecNum; i++)
	{
		if(0 != ConfigGetTimerRecSchParam(&para->psRecTimerSchPara[i],i,file))
		{
			ret = -1;
		}
	}
	if(ret==-1)
	{
		printf("time:%ld, line:%d\n", time(NULL), __LINE__);
	}
	for(i = 0; i < g_sConfigDvrProperty.nRecNum; i++)
	{
		if(0 != ConfigGetVMotionRecSchParam(&para->psRecVMSchPara[i],i,file))
		{
			ret = -1;
		}
	}
	
	if(ret==-1)
	{
		printf("time:%ld, line:%d\n", time(NULL), __LINE__);
	}
	
	//printf("time:%ld, line:%d\n", time(NULL), __LINE__);
	for(i = 0; i < g_sConfigDvrProperty.nRecNum; i++)
	{
		if(0 != ConfigGetSensorRecSchParam(&para->psRecSensorSchPara[i],i,file))
		{
			ret = -1;
		}
	}
	
	if(ret==-1)
	{
		printf("time:%ld, line:%d\n", time(NULL), __LINE__);
	}
	
	//printf("time:%ld, line:%d\n", time(NULL), __LINE__);
	for(i = 0; i < g_sConfigDvrProperty.nSensorNum; i++)
	{
		if(0 != ConfigGetSensorParam(&para->psSensorPara[i],i,file))
		{
			ret = -1;
		}
	}
	
	if(ret==-1)
	{
		printf("time:%ld, line:%d\n", time(NULL), __LINE__);
	}
	
	//yaogang modify 20141209 深广平台参数
	if(0 != ConfigGetSGParam(para->psSGPara, file))
	{
		ret = -1;
	}
	
	if(ret==-1)
	{
		printf("ConfigGetSGParam() failed time:%ld, line:%d\n", time(NULL), __LINE__);
	}

	if(0 != ConfigGetSGDailyParam(para->psSGDailyPara, file))
	{
		ret = -1;
	}
	
	if(ret==-1)
	{
		printf("ConfigGetSGDailyParam() failed time:%ld, line:%d\n", time(NULL), __LINE__);
	}

	for(i = 0; i < g_sConfigDvrProperty.nVidMainNum+g_sConfigDvrProperty.nSensorNum; i++)
	{
		if(0 != ConfigGetSGAlarmParam(&para->psSGAlarmPara[i],i,file))
		{
			printf("ConfigGetSGAlarmParam() failed nId: %d time:%ld, line:%d\n", i, time(NULL), __LINE__);
		}
	}
	
	//yaogang modify 20141010
	//printf("time:%ld, line:%d\n", time(NULL), __LINE__);
	for(i = 0; i < g_sConfigDvrProperty.nVidMainNum; i++)
	{
		if(0 != ConfigGetIPCCoverParam(&para->psIPCCoverPara[i],i,file))
		{
			ret = -1;
		}
	}
	if(ret==-1)
	{
		printf("time:%ld, line:%d\n", time(NULL), __LINE__);
	}
	
	for(i = 0; i < g_sConfigDvrProperty.nVidMainNum; i++)
	{
		if(0 != ConfigGetIPCExtSensorParam(&para->psIPCExtSensorPara[i],i,file))
		{
			ret = -1;
		}
	}
	if(ret==-1)
	{
		printf("time:%ld, line:%d\n", time(NULL), __LINE__);
	}
	
	for(i = 0; i < g_sConfigDvrProperty.nRecNum; i++)
	{
		if(0 != ConfigGet485ExtSensorParam(&para->ps485ExtSensorPara[i],i,file))
		{
			ret = -1;
		}
	}
	if(ret==-1)
	{
		printf("time:%ld, line:%d\n", time(NULL), __LINE__);
	}

	if(0 != ConfigGetHDDParam(para->psHDDPara, 0, file))
	{
		ret = -1;
	}
	
	if(ret==-1)
	{
		printf("time:%ld, line:%d\n", time(NULL), __LINE__);
	}
	
	//printf("time:%ld, line:%d\n", time(NULL), __LINE__);
	for(i = 0; i < chnnum; i++)
	{
		if(0 != ConfigGetVMotionParam(&para->psVMotionPara[i],i,file))
		{
			ret = -1;
		}
	}
	
	if(ret==-1)
	{
		printf("time:%ld, line:%d\n", time(NULL), __LINE__);
	}
	
	//printf("time:%ld, line:%d\n", time(NULL), __LINE__);
	for(i = 0; i < chnnum; i++)
	{
		if(0 != ConfigGetVLostParam(&para->psVLostPara[i],i,file))
		{
			ret = -1;
		}
	}
	
	if(ret==-1)
	{
		printf("time:%ld, line:%d\n", time(NULL), __LINE__);
	}
	
	//printf("time:%ld, line:%d\n", time(NULL), __LINE__);
	for(i = 0; i < chnnum; i++)
	{
		if(0 != ConfigGetVBlindParam(&para->psVBlindPara[i],i,file))
		{
			ret = -1;
		}
	}
	
	if(ret==-1)
	{
		printf("time:%ld, line:%d\n", time(NULL), __LINE__);
	}
	
	//printf("time:%ld, line:%d\n", time(NULL), __LINE__);
	for(i = 0; i < g_sConfigDvrProperty.nAlarmoutNum; i++)
	{
		if(0 != ConfigGetAlarmOutParam(&para->psAlarmOutPara[i],i,file))
		{
			ret = -1;
		}
	}
	
	if(ret==-1)
	{
		printf("time:%ld, line:%d\n", time(NULL), __LINE__);
	}
	
	//printf("time:%ld, line:%d\n", time(NULL), __LINE__);
	for(i = 0; i < g_sConfigDvrProperty.nBuzzNum; i++)
	{
		if(0 != ConfigGetBuzzParam(&para->psBuzzPara[i],i,file))
		{
			ret = -1;
		}
	}
	
	if(ret==-1)
	{
		printf("time:%ld, line:%d\n", time(NULL), __LINE__);
	}
	
	//printf("time:%ld, line:%d\n", time(NULL), __LINE__);
	for(i = 0; i < g_sConfigDvrProperty.nSensorNum; i++)
	{
		if(0 != ConfigGetSensorSchParam(&para->psSensorSchPara[i],i,file))
		{
			ret = -1;
		}
	}
	
	if(ret==-1)
	{
		printf("time:%ld, line:%d\n", time(NULL), __LINE__);
	}
	//yaogang modify 20141020
	for(i = 0; i < g_sConfigDvrProperty.nVidMainNum; i++)
	{
		if(0 != ConfigGetIPCExtSensorSchParam(&para->psIPCExtSensorSchPara[i],i,file))
		{
			ret = -1;
		}
	}
	
	if(ret==-1)
	{
		printf("time:%ld, line:%d\n", time(NULL), __LINE__);
	}
	//printf("time:%ld, line:%d\n", time(NULL), __LINE__);
	for(i = 0; i < chnnum; i++)
	{
		if(0 != ConfigGetVMotionSchParam(&para->psVMotionSchPara[i],i,file))
		{
			ret = -1;
		}
	}
	
	if(ret==-1)
	{
		printf("time:%ld, line:%d\n", time(NULL), __LINE__);
	}
	
	//printf("time:%ld, line:%d\n", time(NULL), __LINE__);
	for(i = 0; i < g_sConfigDvrProperty.nAlarmoutNum; i++)
	{
		if(0 != ConfigGetAlarmOutSchParam(&para->psAlarmOutSchPara[i],i,file))
		{
			ret = -1;
		}
	}
	
	if(ret==-1)
	{
		printf("time:%ld, line:%d\n", time(NULL), __LINE__);
	}
	
	for(i = 0; i < chnnum; i++)
	{
		if(0 != ConfigGetPtzParam(&para->psPtzPara[i],i,file))
		{
			ret = -1;
		}
	}
	
	if(ret==-1)
	{
		printf("time:%ld, line:%d\n", time(NULL), __LINE__);
	}
	
	#if 1
	//printf("time:%ld, line:%d\n", time(NULL), __LINE__);
	if(0 != ConfigGetUserParam(para->psUserPara,file))
	{
		printf("ConfigGetUserParam error!\n");
		ret = -1;
	}
	#endif
	
	if(0 != ConfigGetNetParam(para->psNetPara,0,file))
	{
		printf("ConfigGetNetParam error!\n");
		ret = -1;
	}
	
	//NVR used
	for(i = 0; i < chnnum; i++)
	{
		if(0 != ConfigGetIPCameraParam(&para->psIPCameraPara[i],i,file))
		{
			ret = -1;
		}
	}
	if(ret==-1)
	{
		printf("time:%ld, line:%d\n", time(NULL), __LINE__);
	}
	//yaogang modify 20150105
	for(i = 0; i < chnnum; i++)
	{
		if(0 != ConfigGetSnapParam(&para->psSnapPara[i],i,file))
		{
			ret = -1;
		}
	}
	if(ret==-1)
	{
		printf("time:%ld, line:%d\n", time(NULL), __LINE__);
	}

	//yaogang modify for bad disk
	for(i = 0; i < g_sConfigDvrProperty.nMaxHddNum; i++)
	{
		if(0 != ConfigGetBadDiskParam(&para->psBadDiskListPara[i],i,file))
		{
			ret = -1;
		}
	}
	if(ret==-1)
	{
		printf("time:%ld, line:%d\n", time(NULL), __LINE__);
	}
	
	//printf("time:%ld, line:%d\n", time(NULL), __LINE__);
	
	if(ret == 0)//csp modify
	{
		if(0 != sem_init(&para->sem, 0, 1))
		{
			printf("sem_init error! function:%s\n", __FUNCTION__);
			ret = -1;
		}
	}
	
	if(ret != 0)
	{
		free(para->psSystemPara);
		free(para->psPreviewPara);
		free(para->psMainTainPara);
		free(para->psImagePara);
		free(para->psVoImagePara);
		free(para->psMainOsdPara);
		free(para->psStrOsdPara);
		free(para->psRectOsdPara);
		free(para->psVideoMainPara);
		free(para->psVideoSubPara);
		free(para->psVideoMobPara);
		free(para->psRecordPara);
		free(para->psRecTimerSchPara);
		free(para->psRecVMSchPara);
		free(para->psRecSensorSchPara);
		free(para->psSensorPara);
		//yaogang modify 20141010
		free(para->psIPCCoverPara);
		free(para->psIPCExtSensorPara);
		free(para->ps485ExtSensorPara);
		free(para->psHDDPara);
		
		free(para->psVMotionPara);
		free(para->psVLostPara);
		free(para->psVBlindPara);
		free(para->psAlarmOutPara);
		free(para->psBuzzPara);
		free(para->psSensorSchPara);
		free(para->psIPCExtSensorSchPara);
		free(para->psVMotionSchPara);
		free(para->psAlarmOutSchPara);
		free(para->psPtzPara);
		free(para->psUserPara);
		free(para->psNetPara);
		free(para->psIPCameraPara);//NVR used
		free(para->psSnapPara);
		//yaogang modify 20141209 深广平台参数
		free(para->psSGPara);
		free(para->psSGDailyPara);
		free(para->psSGAlarmPara);

		//yaogang modify for bad disk
		free(para->psBadDiskListPara);
	}
	else
	{
		printf("Load all Config para OK!\n");		
	}
	
	return ret;
}

void ConfigWriteParamToFile(SConfigParaInfoWriteFile* para)
{
	if(g_Inited == 0)
	{
		printf("Mod config has not been inited!\n");
		return;
	}
	if(para == NULL)
	{
		printf("Invalid para! function:%s\n", __FUNCTION__);
		return;
	}
	
	//PUBPRT("SPL HERE ConfigWriteParamToFile");
	//DEBUG_PRINTF
	
	sem_wait(&g_FileSem);
	
	//PUBPRT("SPL HERE ConfigWriteParamToFile1");
	
	s32 nId = para->nId;
	s32 ret = 0;
	switch(para->emType)
	{
		case EM_CONFIG_PARA_SYSTEM:
			ret = ConfigSetSystemParam((SModConfigSystemParam*)para->pData, g_sConfigDevInfo.nFilePath);
			break;
		case EM_CONFIG_PARA_MAINTAIN:
			ret = ConfigSetMainTainParam((SModConfigMainTainParam*)para->pData,  g_sConfigDevInfo.nFilePath);
			break;
		case EM_CONFIG_PARA_PATROL:
		case EM_CONFIG_PARA_PREVIEW:
			ret = ConfigSetPreviewParam((SModConfigPreviewParam*)para->pData, g_sConfigDevInfo.nFilePath);
			break;
			
		case EM_CONFIG_PARA_IMAGE:
			ret = ConfigSetImageParam((SModConfigImagePara*)para->pData, nId, g_sConfigDevInfo.nFilePath);
			break;

		case EM_CONFIG_PARA_VO_IMAGE:
			ret = ConfigSetVoImageParam((SModConfigImagePara*)para->pData, nId, g_sConfigDevInfo.nFilePath);
			break;
			
		case EM_CONFIG_PARA_MAIN_OSD:
			ret = ConfigSetMainOsdParam((SModConfigMainOsdParam*)para->pData, g_sConfigDevInfo.nFilePath);
			break;
			
		case EM_CONFIG_PARA_STR_OSD:
			ret = ConfigSetStrOsdParam((SModConfigStrOsdpara*)para->pData, nId, g_sConfigDevInfo.nFilePath);
			break;
			
		case EM_CONFIG_PARA_RECT_OSD:
			ret = ConfigSetRectOsdParam((SModConfigRectOsdpara*)para->pData, nId, g_sConfigDevInfo.nFilePath);
			break;
			
		case EM_CONFIG_PARA_ENC_MAIN:
			ret = ConfigSetMainEncodeParam((SModConfigVideoParam*)para->pData, nId, g_sConfigDevInfo.nFilePath);
			break;
			
		case EM_CONFIG_PARA_ENC_SUB:
			ret = ConfigSetSubEncodeParam((SModConfigVideoParam*)para->pData, nId, g_sConfigDevInfo.nFilePath);
			break;
		case EM_CONFIG_PARA_ENC_MOB:
			ret = ConfigSetMobEncodeParam((SModConfigVideoParam*)para->pData, nId, g_sConfigDevInfo.nFilePath);
			break;
			
		case EM_CONFIG_PARA_RECORD:
			ret = ConfigSetRecordParam((SModConfigRecordParam*)para->pData, nId, g_sConfigDevInfo.nFilePath);
			break;
			
		case EM_CONFIG_PARA_TIMER_REC_SCH:
			ret = ConfigSetTimerRecSchParam((SModConfigRecTimerSch*)para->pData, nId, g_sConfigDevInfo.nFilePath);
			break;
			
		case EM_CONFIG_PARA_SENSOR_REC_SCH:
			ret = ConfigSetSensorRecSchParam((SModConfigRecAlarmInSch*)para->pData, nId, g_sConfigDevInfo.nFilePath);
			break;
			
		case EM_CONFIG_PARA_VMOTION_REC_SCH:
			ret = ConfigSetVMotionRecSchParam((SModConfigRecVMSch*)para->pData, nId, g_sConfigDevInfo.nFilePath);
			break;
			
		case EM_CONFIG_PARA_VLOST_REC_SCH:
		case EM_CONFIG_PARA_VBLIND_REC_SCH:
			break;

		case EM_CONFIG_PARA_SENSOR:
			ret = ConfigSetSensorParam((SModConfigSensorParam*)para->pData, nId, g_sConfigDevInfo.nFilePath);
			break;
		//yaogang modify 20141010
		case EM_CONFIG_PARA_IPCCOVER:
			ret = ConfigSetIPCCoverParam((SModConfigIPCExtSensorParam*)para->pData, nId, g_sConfigDevInfo.nFilePath);
			break;
		case EM_CONFIG_PARA_IPCEXTSENSOR:
			ret = ConfigSetIPCExtSensorParam((SModConfigIPCExtSensorParam*)para->pData, nId, g_sConfigDevInfo.nFilePath);
			break;
		case EM_CONFIG_PARA_485EXTSENSOR:
			ret = ConfigSet485ExtSensorParam((SModConfigIPCExtSensorParam*)para->pData, nId, g_sConfigDevInfo.nFilePath);
			break;
		case EM_CONFIG_PARA_HDD:
			ret = ConfigSetHDDParam((SModConfigHDDParam*)para->pData, nId, g_sConfigDevInfo.nFilePath);
			break;
		
		case EM_CONFIG_PARA_VMOTION:
			ret = ConfigSetVMotionParam((SModConfigVMotionParam*)para->pData, nId, g_sConfigDevInfo.nFilePath);
			break;
			
		case EM_CONFIG_PARA_VLOST:
			ret = ConfigSetVLostParam((SModConfigVLostParam*)para->pData, nId, g_sConfigDevInfo.nFilePath);
			break;
			
		case EM_CONFIG_PARA_VBLIND:
			ret = ConfigSetVBlindParam((SModConfigVBlindParam*)para->pData, nId, g_sConfigDevInfo.nFilePath);
			break;
			
		case EM_CONFIG_PARA_ALARMOUT:
			ret = ConfigSetAlarmOutParam((SModConfigAlarmOutParam*)para->pData, nId, g_sConfigDevInfo.nFilePath);
			break;
			
		case EM_CONFIG_PARA_BUZZ:
			ret = ConfigSetBuzzParam((SModConfigBuzzParam*)para->pData, nId, g_sConfigDevInfo.nFilePath);
			break;
			
		case EM_CONFIG_PARA_SENSOR_SCH:
			ret = ConfigSetSensorSchParam((SModConfigSensorSch*)para->pData, nId, g_sConfigDevInfo.nFilePath);
			break;
		case EM_CONFIG_PARA_IPCEXTSENSOR_SCH:
			ret = ConfigSetIPCExtSensorSchParam((SModConfigSensorSch*)para->pData, nId, g_sConfigDevInfo.nFilePath);
			break;	
		case EM_CONFIG_PARA_VMOTION_SCH:
			ret = ConfigSetVMotionSchParam((SModConfigVMotionSch*)para->pData, nId, g_sConfigDevInfo.nFilePath);
			break;
			
		case EM_CONFIG_PARA_ALARMOUT_SCH:
			ret = ConfigSetAlarmOutSchParam((SModConfigAlarmOutSch*)para->pData, nId, g_sConfigDevInfo.nFilePath);
			break;
			
		case EM_CONFIG_PARA_VLOST_SCH:
		case EM_CONFIG_PARA_VBLIND_SCH:
		case EM_CONFIG_PARA_BUZZ_SCH:
			break;
			
		case EM_CONFIG_PARA_PTZ:
			ret = ConfigSetPtzParam((SModConfigPtzParam*)para->pData, nId, g_sConfigDevInfo.nFilePath);
			break;
			
		case EM_CONFIG_PARA_USER:
			ret = ConfigSetUserParam((SModConfigUserPara*)para->pData, g_sConfigDevInfo.nFilePath);
			break;
			
		case EM_CONFIG_PARA_NETWORK:
			ret = ConfigSetNetParam((SModConfigNetParam*)para->pData, nId, g_sConfigDevInfo.nFilePath);
			break;
			
		//NVR used
		case EM_CONFIG_PARA_IPCAMERA:
			ret = ConfigSetIPCameraParam((SModConfigIPCameraParam*)para->pData, nId, g_sConfigDevInfo.nFilePath);
			break;
		//Snap
		case EM_CONFIG_PARA_SNAP_CHN:
			ret = ConfigSetSnapParam((SModConfigSnapChnPara*)para->pData, nId, g_sConfigDevInfo.nFilePath);
			break;
		case EM_CONFIG_PARA_SG:
			ret = ConfigSetSGParam((SModConfigSGParam*)para->pData, g_sConfigDevInfo.nFilePath);
			break;
		case EM_CONFIG_PARA_SG_DAILY:
			ret = ConfigSetSGDailyParam((SModDailyPicCFG*)para->pData, g_sConfigDevInfo.nFilePath);
			break;
		case EM_CONFIG_PARA_SG_ALARM:
			ret = ConfigSetSGAlarmParam((SModAlarmPicCFG*)para->pData, nId, g_sConfigDevInfo.nFilePath);
			break;
		//yaogang modify for bad disk	
		case EM_CONFIG_BAD_DISK:
			ret = ConfigSetBadDiskParam((SModConfigBadDisk*)para->pData, nId, g_sConfigDevInfo.nFilePath);
			break;	
		case EM_CONFIG_PARA_DVR_PROPERTY:
			//no permit
		default:
			printf("%s Error: Invalid para type: %d!\n", __FUNCTION__, para->emType);
			break;
	}
	if(ret != 0)
	{
		printf("error: %s\n", __FUNCTION__);
	}
	
	sem_post(&g_FileSem);
	
	//DEBUG_PRINTF
	
	return;
}

void ConfigManagerFxn(void)
{
	printf("$$$$$$$$$$$$$$$$$$ConfigManagerFxn id:%d\n",getpid());
	
	u8 flag_sendmsg_writeflash = 1;
	
	int errortimes = 0;
	SConfigMsgHeader nMsg;
	memset(&nMsg, 0, sizeof(SConfigMsgHeader));
	SConfigParaInfoWriteFile sCfgPara;
	memset(&sCfgPara, 0, sizeof(SConfigParaInfoWriteFile));
	
	printf("pthread ConfigManagerFxn going......\n");
	
	while(1)
	{
		if(g_Inited == 0)
		{
			break;
		}
		
		if(g_IsResuming)
		{
			sleep(1);
			continue;
		}
		
		if(g_s_MsgQueueInfo.nLenUsed > 0)
		{
			if(0 != ConfigReadMsgQueue(&nMsg))
			{
				printf("ConfigReadMsgQueue error! error times = %d\n", ++errortimes);
				continue;
			}
			sCfgPara.emType = nMsg.emType;
			sCfgPara.nId = nMsg.nId;
			sCfgPara.pData = nMsg.pData;

			//if (EM_CONFIG_PARA_SG == sCfgPara.emType)
			//{
			//	printf("yg write file EM_CONFIG_PARA_SG\n");
			//}
			//printf("yg ConfigWriteParamToFile type: %d, 0x%x\n", sCfgPara.emType, sCfgPara.emType);
			ConfigWriteParamToFile(&sCfgPara);
			//if(nMsg.pData)
			//{
				//free(nMsg.pData);
			//}
			flag_sendmsg_writeflash = 0;
		}
		else if(g_s_MsgQueueInfo.nLenUsed == 0)
		{
			if(flag_sendmsg_writeflash == 0)
			{
				int value = -1;
				sem_getvalue(&g_sConfigDevInfo.sem, &value);
				if(value == 0)
				{
					sem_post(&g_sConfigDevInfo.sem);
				}
				flag_sendmsg_writeflash = 1;
			}
		}
		
		usleep(40*1000);
	}
}		

#if 0//csp modify
void ConfigLoadDefaultToMemFxn(void)
{
	printf("pthread ConfigLoadDefaultToMemFxn going......\n");
	sleep(1);
	if(0 != ConfigLoadAllParaToMem(&g_ConfigDefaultAllPara, g_sConfigDevInfo.nDefaultConfPath))
	{
		printf("Warnning: Load default config to memory failed!\n");
	}
	else
	{
		g_BeAbleToDefault = 1;
		printf("ConfigLoadDefaultToMemFxn:Load default config to memory OK!\n");
	}
}
#endif

s32 ConfigGetMainTainParam(SModConfigMainTainParam* para, s8* pFilepath)
{
	if ((NULL == para) || (NULL == pFilepath))
	{
		printf("Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}
	
	s32 tmp = 0;

	if(0 != IniGetValue(pFilepath, 
		CONFIG_MAINTAIN_PARAM, 
		EM_CONFIG_MAINTAIN_PARAM_ENABLE, 
		&tmp))
	{
		para->nEnable = 0;
		printf("#####get system mantain enable failed!! set 0 !\n");
		//return -1;
	}
	else
	{
		para->nEnable = tmp;
	}

	if(0 != IniGetValue(pFilepath, 
		CONFIG_MAINTAIN_PARAM, 
		EM_CONFIG_MAINTAIN_PARAM_MAINTYPE, 
		&tmp))
	{
		para->nMainType = 0;
		printf("#####get system mantain maintype failed!! set 0 !\n");
		//return -1;
	}
	else
	{
		para->nMainType = tmp;
	}
	
	if(0 != IniGetValue(pFilepath, 
		CONFIG_MAINTAIN_PARAM, 
		EM_CONFIG_MAINTAIN_PARAM_SUBTYPE, 
		&tmp))
	{
		para->nSubType = 0;
		printf("#####get system mantain subtype!! set 0 !\n");
		//return -1;
	}
	else
	{
		para->nSubType = tmp;
	}
	
	if(0 != IniGetValue(pFilepath, 
		CONFIG_MAINTAIN_PARAM, 
		EM_CONFIG_MAINTAIN_PARAM_TIME, 
		&tmp))
	{
		para->ntime = 23;
		printf("#####get system mantain time failed!! set 23 !\n");
		//return -1;
	}
	else
	{
		para->ntime = tmp;
	}
	
	return 0;//csp modify
}

s32 ConfigGetSystemParam(SModConfigSystemParam* para, s8* pFilepath)
{
	if((NULL == para) || (NULL == pFilepath))
	{
		printf("Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}
	
	#ifdef CONFIG_TEST
	memset(para, 0, sizeof(SModConfigSystemParam));
	para->nCyclingRecord = 1;
	para->nDevId = 1;
	strcpy(para->nDevName, "DVR");
	para->nOutput = 1;
	return 0;
	#endif
	
	#if 1//csp modify
	memset(para->reserved, 0, sizeof(para->reserved));
	memset(para->sn, 0, sizeof(para->sn));
	if(0 != GetStringFromIni(pFilepath, 
		CONFIG_SYSTEM_PARAM, 
		CONFIG_SYSTEM_PARAM_SN, 
		para->sn, 
		sizeof(para->sn)))
	{
		strcpy(para->sn,"xxxxx-xxxxx-xxxxx");
	}
	#endif
	
	s32 tmp = 0;
	if(0 != GetStringFromIni(pFilepath, 
		CONFIG_SYSTEM_PARAM, 
		CONFIG_SYSTEM_PARAM_DEVNAME, 
		para->nDevName, 
		sizeof(para->nDevName)))
	{
		return -1;
	}
	
	if(0 != IniGetValue(pFilepath, 
		CONFIG_SYSTEM_PARAM, 
		CONFIG_SYSTEM_PARAM_DEVID, 
		&tmp))
	{
		return -1;
	}
	para->nDevId = tmp;
	
	if(0 != IniGetValue(pFilepath, 
		CONFIG_SYSTEM_PARAM, 
		CONFIG_SYSTEM_PARAM_MANUAL, 
		&tmp))
	{
		return -1;
	}
	para->nManual = tmp;
	
	if(0 != IniGetValue(pFilepath, 
		CONFIG_SYSTEM_PARAM, 
		CONFIG_SYSTEM_PARAM_VIDEOSTAND, 
		&tmp))
	{
		return -1;
	}
	para->nVideoStandard = tmp;
	printf("file:%s,VideoStandard:%d\n",pFilepath,para->nVideoStandard);
	
	if(0 != IniGetValue(pFilepath, 
		CONFIG_SYSTEM_PARAM, 
		CONFIG_SYSTEM_PARAM_AUTHCHECK, 
		&tmp))
	{
		return -1;
	}
	para->nAuthCheck = tmp;

	if(0 != IniGetValue(pFilepath, 
		CONFIG_SYSTEM_PARAM, 
		CONFIG_SYSTEM_PARAM_OUTPUT, 
		&tmp))
	{
		return -1;
	}
	para->nOutput = tmp;
	
	if(0 != IniGetValue(pFilepath, 
			CONFIG_SYSTEM_PARAM, 
			CONFIG_SYSTEM_PARAM_LANGID, 
			&tmp))
	{
		return -1;
	}
	//para->nLangId = tmp;
	para->nLangId = 1;//tmp;//csp modify 20150110

	if(0 != IniGetValue(pFilepath, 
		CONFIG_SYSTEM_PARAM, 
		CONFIG_SYSTEM_PARAM_SHOWGUIDE, 
		&tmp))
	{
		return -1;
	}
	para->nShowGuide = tmp;

	if(0 != IniGetValue(pFilepath, 
			CONFIG_SYSTEM_PARAM, 
			CONFIG_SYSTEM_PARAM_LOCKTIME, 
			&tmp))
	{
		return -1;
	}
	para->nLockTime = tmp;

	if(0 != IniGetValue(pFilepath, 
			CONFIG_SYSTEM_PARAM, 
			CONFIG_SYSTEM_PARAM_DATEFORMAT, 
			&tmp))
	{
		return -1;
	}
	para->nDateFormat = tmp;

	if(0 != IniGetValue(pFilepath, 
			CONFIG_SYSTEM_PARAM, 
			CONFIG_SYSTEM_PARAM_TIMESTAND, 
			&tmp))
	{
		return -1;
	}
	para->nTimeStandard = tmp;

	if(0 != IniGetValue(pFilepath, 
			CONFIG_SYSTEM_PARAM, 
			CONFIG_SYSTEM_PARAM_CIRCLEREC, 
			&tmp))
	{
		return -1;
	}
	para->nCyclingRecord = tmp;

	if(0 != IniGetValue(pFilepath, 
			CONFIG_SYSTEM_PARAM, 
			CONFIG_SYSTEM_PARAM_SHOWFIRSTMENUE, 
			&tmp))
	{
		return -1;
	}
	para->nShowFirstMenue = tmp;
	
	if(0 != IniGetValue(pFilepath, 
		CONFIG_SYSTEM_PARAM, 
		CONFIG_SYSTEM_PARAM_SyscCheck, 
		&tmp))
	{
		tmp = 0;
		//return -1;
	}
	para->nSyscCheck = tmp;//QYJ
	
	if(0 != GetStringFromIni(pFilepath, 
			CONFIG_SYSTEM_PARAM, 
			CONFIG_SYSTEM_PARAM_NtpServerAdress, 
			para->nNtpServerAdress, 
			sizeof(para->nNtpServerAdress)))
	{
		memset(para->nNtpServerAdress,0,sizeof(para->nNtpServerAdress));
		strcpy(para->nNtpServerAdress,"time.windows.com");
		//return -1;
	}//QYJ
	
	if(0 != IniGetValue(pFilepath, 
		CONFIG_SYSTEM_PARAM, 
		CONFIG_SYSTEM_PARAM_TimeZone, 
		&tmp))
	{
		tmp = 27;
		//return -1;
	}
	para->nTimeZone = tmp;//QYJ
	
	if(0 != IniGetValue(pFilepath, 
		CONFIG_SYSTEM_PARAM, 
		CONFIG_SYSTEM_PARAM_SyncTimeToIPC, 
		&tmp))
	{
		tmp = 0;
		//return -1;
	}
	para->nSyncTimeToIPC = tmp;
	
	if(0 != IniGetValue(pFilepath, 
		CONFIG_SYSTEM_PARAM, 
		CONFIG_SYSTEM_PARAM_P2PEnable, 
		&tmp))
	{
		tmp = 0;
		//return -1;
	}
	para->P2PEnable = tmp;
	
	return 0;
}

s32 ConfigGetPreviewParam(SModConfigPreviewParam* para, s8* pFilepath)
{
	if((NULL == para) || (NULL == pFilepath))
	{
		printf("Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}
	
	#ifdef CONFIG_TEST
	memset(para, 0, sizeof(SModConfigPreviewParam));
	para->nStops = 4;
	para->nPatrolMode = 4;
	para->pnStopModePara[0] = 0;
	para->pnStopModePara[1] = 1;
	para->pnStopModePara[2] = 2;
	para->pnStopModePara[3] = 3;
	para->nPreviewMode = 4;
	
	return 0;
	#endif
	
	s32 tmp = 0;
	s32 i = 0;
	if(0 != IniGetValue(pFilepath, 
		CONFIG_PREVIEW_PARAM, 
		CONFIG_PREVIEW_PARAM_MODE, 
		&tmp))
	{
		return -1;
	}
	para->nPreviewMode = tmp;
	
	if(0 != IniGetValue(pFilepath, 
		CONFIG_PREVIEW_PARAM, 
		CONFIG_PREVIEW_PARAM_MODEPARA, 
		&tmp))
	{
		return -1;
	}
	para->nModePara = tmp;
	
	if(0 != IniGetValue(pFilepath, 
		CONFIG_PREVIEW_PARAM, 
		CONFIG_PREVIEW_PARAM_AUDIOOUTCHN, 
		&tmp))
	{
		return -1;
	}
	para->nAdioChn = tmp;
	
	if(0 != IniGetValue(pFilepath, 
		CONFIG_PREVIEW_PARAM, 
		CONFIG_PREVIEW_PARAM_VOLUME, 
		&tmp))
	{
		return -1;
	}
	para->nVolume = tmp;
	
	if(0 != IniGetValue(pFilepath, 
		CONFIG_PREVIEW_PARAM, 
		CONFIG_PREVIEW_PARAM_MUTE, 
		&tmp))
	{
		return -1;
	}
	para->nMute = tmp;
	
	if(0 != IniGetValue(pFilepath, 
		CONFIG_PREVIEW_PARAM, 
		CONFIG_PREVIEW_PARAM_ISPATROL, 
		&tmp))
	{
		return -1;
	}
	para->nIsPatrol = tmp;
	
	if(0 != IniGetValue(pFilepath, 
		CONFIG_PREVIEW_PARAM, 
		CONFIG_PREVIEW_PARAM_INTERVAL, 
		&tmp))
	{
		return -1;
	}
	para->nInterval = tmp;
	
	if(0 != IniGetValue(pFilepath, 
		CONFIG_PREVIEW_PARAM, 
		CONFIG_PREVIEW_PARAM_PATROLMODE, 
		&tmp))
	{
		return -1;
	}
	para->nPatrolMode = tmp;
	
	if(0 != IniGetValue(pFilepath, 
		CONFIG_PREVIEW_PARAM, 
		CONFIG_PREVIEW_PARAM_STOPS, 
		&tmp))
	{
		return -1;
	}
	para->nStops = tmp;
	
	memset(para->pnStopModePara, 0xff, sizeof(para->pnStopModePara));
	for(i = 0; i < para->nStops; i++)
	{
		if(0 != IniGetArrayValue(pFilepath, 
			CONFIG_PREVIEW_PARAM,  
			CONFIG_PREVIEW_PARAM_STOPMODEPARA,
			i,
			&tmp))
		{
			return -1;
		}
		para->pnStopModePara[i] = tmp;		
	}

	for(i = 0; i < g_sConfigDvrProperty.nPreviewNum; i++)
	{
		if(0 != IniGetArrayValue(pFilepath, 
			CONFIG_PREVIEW_PARAM,  
			CONFIG_PREVIEW_PARAM_VIDEOSRCTYPEPARA,
			i,
			&tmp))
		{
			para->nVideoSrcType[i] = 0;
		}
		else
		{
			para->nVideoSrcType[i] = tmp;
		}
	}
	
	return 0;
}

s32 ConfigGetVoImageParam(SModConfigImagePara* para, s32 nId, s8* pFilepath)
{
	if((para == NULL) || (nId < 0) || (nId >= g_sConfigDvrProperty.nPreviewNum) || (NULL == pFilepath))
	{
		printf("Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}
	
	#ifdef CONFIG_TEST
	memset(para, 0, sizeof(SModConfigImagePara));
	para->nBrightness = 100;
	para->nContrast = 100;
	para->nHue = 100;
	para->nSaturation = 100;
	return 0;
	#endif
	
	s32 tmp = 0;
	s32 i = nId;
	s8 strtmp[30];
	
	memset(strtmp, 0, sizeof(strtmp));
	sprintf(strtmp, "%s%d", CONFIG_PREVIEW_PARAM_PLAYBACKCOLOR, i);

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_PREVIEW_PARAM,  
		strtmp,
		EM_CONFIG_PREVIEW_PARAM_PREVIEWCOLOR_VO_HUE,
		&tmp))
	{
		return -1;
	}
	para->nHue = tmp;	

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_PREVIEW_PARAM,  
		strtmp,
		EM_CONFIG_PREVIEW_PARAM_PREVIEWCOLOR_VO_SATURATION,
		&tmp))
	{
		return -1;
	}
	para->nSaturation = tmp;	

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_PREVIEW_PARAM,  
		strtmp,
		EM_CONFIG_PREVIEW_PARAM_PREVIEWCOLOR_VO_CONTRAST,
		&tmp))
	{
		return -1;
	}
	para->nContrast = tmp;	

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_PREVIEW_PARAM,  
		strtmp,
		EM_CONFIG_PREVIEW_PARAM_PREVIEWCOLOR_VO_BRIGHTNESS,
		&tmp))
	{
		return -1;
	}
	para->nBrightness = tmp;		

	return 0;
}

s32 ConfigGetImageParam(SModConfigImagePara* para, s32 nId, s8* pFilepath)
{
	if((para == NULL) || (nId < 0) || (nId >= g_sConfigDvrProperty.nPreviewNum) || (NULL == pFilepath))
	{
		printf("Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}
	
	#ifdef CONFIG_TEST
	memset(para, 0, sizeof(SModConfigImagePara));
	para->nBrightness = 128;
	para->nContrast = 128;
	para->nHue = 128;
	para->nSaturation = 128;
	return 0;
	#endif
	
	s32 tmp = 0;
	s32 i = nId;
	s8 strtmp[30];
	
	memset(strtmp, 0, sizeof(strtmp));
	sprintf(strtmp, "%s%d", CONFIG_PREVIEW_PARAM_PREVIEWCOLOR, i);

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_PREVIEW_PARAM,  
		strtmp,
		EM_CONFIG_PREVIEW_PARAM_PREVIEWCOLOR_HUE,
		&tmp))
	{
		return -1;
	}
	para->nHue = tmp;	

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_PREVIEW_PARAM,  
		strtmp,
		EM_CONFIG_PREVIEW_PARAM_PREVIEWCOLOR_SATURATION,
		&tmp))
	{
		return -1;
	}
	para->nSaturation = tmp;	

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_PREVIEW_PARAM,  
		strtmp,
		EM_CONFIG_PREVIEW_PARAM_PREVIEWCOLOR_CONTRAST,
		&tmp))
	{
		return -1;
	}
	para->nContrast = tmp;	

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_PREVIEW_PARAM,  
		strtmp,
		EM_CONFIG_PREVIEW_PARAM_PREVIEWCOLOR_BRIGHTNESS,
		&tmp))
	{
		return -1;
	}
	para->nBrightness = tmp;		

	return 0;
}

s32 ConfigGetMainOsdParam(SModConfigMainOsdParam* para, s8* pFilepath)
{
	if((NULL == para) || (NULL == pFilepath))
	{
		printf("Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}

	#ifdef CONFIG_TEST
	memset(para, 0, sizeof(SModConfigMainOsdParam));
	para->nShowRecState = 1;
	para->nShowTime = 1;
	para->nTimePosX = 50;
	para->nTimePosY = 50;
	para->nShowChnKbps = 1;
	return 0;
	#endif
	
	s32 tmp = 0;
	if(0 != IniGetValue(pFilepath, 
		CONFIG_OSD_PARAM, 
		CONFIG_OSD_PARAM_SHOWTIME, 
		&tmp))
	{
		return -1;
	}
	para->nShowTime = tmp;

	if(0 != IniGetValue(pFilepath, 
		CONFIG_OSD_PARAM, 
		CONFIG_OSD_PARAM_SHOWCHNKBPS, 
		&tmp))
	{
		return -1;
	}
	para->nShowChnKbps = tmp;
	
	if(0 != IniGetValue(pFilepath, 
		CONFIG_OSD_PARAM, 
		CONFIG_OSD_PARAM_TIME_X, 
		&tmp))
	{
		return -1;
	}
	para->nTimePosX = tmp;
	
	if(0 != IniGetValue(pFilepath, 
		CONFIG_OSD_PARAM, 
		CONFIG_OSD_PARAM_TIME_Y, 
		&tmp))
	{
		return -1;
	}
	para->nTimePosY = tmp;
	
	if(0 != IniGetValue(pFilepath, 
		CONFIG_OSD_PARAM, 
		CONFIG_OSD_PARAM_SHOWRECSTATE, 
		&tmp))
	{
		return -1;
	}
	para->nShowRecState = tmp;
	
	return 0;
}

s32 ConfigGetStrOsdParam(SModConfigStrOsdpara* para, s32 nId, s8* pFilepath)
{
	if((NULL == para) || (nId < 0) || (nId >= g_sConfigDvrProperty.nPreviewNum) || (NULL == pFilepath))
	{
		printf("Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}

	#ifdef CONFIG_TEST
	memset(para, 0, sizeof(SModConfigStrOsdpara));
	sprintf(para->nChnName, "CH %2ld", nId);
	para->nChnNamePosX = 600;
	para->nChnNamePosY = 500;
	para->nShowChnName = 1;
	return 0;
	#endif
	
	s32 tmp = 0;
	s32 i = nId;
	s8 strtmp[30];
	memset(strtmp, 0, sizeof(strtmp));
	sprintf(strtmp, "%s%d", CONFIG_OSD_PARAM_STR_OSD, i);

	if(0 != GetArrayStringFromIni(pFilepath, 
		CONFIG_OSD_PARAM, 
		strtmp, 
		EM_CONFIG_OSD_PARAM_STR_OSD_CHNNAME,
		para->nChnName, 
		sizeof(para->nChnName)))
	{
		return -1;
	}

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_OSD_PARAM, 
		strtmp, 
		EM_CONFIG_OSD_PARAM_STR_OSD_SHOW_CHNNAME,
		&tmp))
	{
		return -1;
	}
	para->nShowChnName = tmp;

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_OSD_PARAM, 
		strtmp, 
		EM_CONFIG_OSD_PARAM_STR_OSD_CHNNAME_X,
		&tmp))
	{
		return -1;
	}
	para->nChnNamePosX = tmp;

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_OSD_PARAM, 
		strtmp, 
		EM_CONFIG_OSD_PARAM_STR_OSD_CHNNAME_Y,
		&tmp))
	{
		return -1;
	}
	para->nChnNamePosY = tmp;

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_OSD_PARAM, 
		strtmp, 
		EM_CONFIG_OSD_PARAM_STR_OSD_ENC_SHOW_CHNNAME,
		&tmp))
	{
		return -1;
	}
	para->nEncShowChnName = tmp;

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_OSD_PARAM, 
		strtmp, 
		EM_CONFIG_OSD_PARAM_STR_OSD_ENC_CHNNAME_X,
		&tmp))
	{
		return -1;
	}
	para->nEncChnNamePosX = tmp;

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_OSD_PARAM, 
		strtmp, 
		EM_CONFIG_OSD_PARAM_STR_OSD_ENC_CHNNAME_Y,
		&tmp))
	{
		return -1;
	}
	para->nEncChnNamePosY = tmp;

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_OSD_PARAM, 
		strtmp, 
		EM_CONFIG_OSD_PARAM_STR_OSD_ENC_SHOWTIME,
		&tmp))
	{
		return -1;
	}
	para->nEncShowTime = tmp;

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_OSD_PARAM, 
		strtmp, 
		EM_CONFIG_OSD_PARAM_STR_OSD_ENC_TIME_X,
		&tmp))
	{
		return -1;
	}
	para->nEncTimePosX = tmp;

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_OSD_PARAM, 
		strtmp, 
		EM_CONFIG_OSD_PARAM_STR_OSD_ENC_TIME_Y,
		&tmp))
	{
		return -1;
	}
	para->nEncTimePosY = tmp;
	
	return 0;
}

s32 ConfigGetRectOsdParam(SModConfigRectOsdpara* para, s32 nId, s8* pFilepath)
{
	if((NULL == para) || (nId < 0) || (nId >= g_sConfigDvrProperty.nPreviewNum) || (NULL == pFilepath))
	{
		printf("Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}

	#ifdef CONFIG_TEST
	memset(para, 0, sizeof(SModConfigRectOsdpara));
	return 0;
	#endif
	
	s32 tmp = 0;
	s32 i = nId;
	s8 strtmp[30];
	memset(strtmp, 0, sizeof(strtmp));
	sprintf(strtmp, "%s%d", CONFIG_OSD_PARAM_RECT_OSD, i);

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_OSD_PARAM, 
		strtmp, 
		EM_CONFIG_OSD_PARAM_RECT_OSD_SHOW,
		&tmp))
	{
		return -1;
	}
	para->nShowRectOsd = tmp;

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_OSD_PARAM, 
		strtmp, 
		EM_CONFIG_OSD_PARAM_RECT_OSD_NUM,
		&tmp))
	{
		return -1;
	}
	para->nRectOsdNum = ((tmp < g_sConfigDvrProperty.nRectOsdNum) ? tmp : g_sConfigDvrProperty.nRectOsdNum);
	

	for(i = 0; i < para->nRectOsdNum; i++)
	{
		if(0 != IniGetArrayValue(pFilepath, 
			CONFIG_OSD_PARAM, 
			strtmp, 
			EM_CONFIG_OSD_PARAM_RECT_OSD_X + (i * 4),
			&tmp))
		{
			return -1;
		}
		para->nRectOsdX[i] = tmp;

		if(0 != IniGetArrayValue(pFilepath, 
			CONFIG_OSD_PARAM, 
			strtmp, 
			EM_CONFIG_OSD_PARAM_RECT_OSD_Y + (i * 4),
			&tmp))
		{
			return -1;
		}
		para->nRectOsdY[i] = tmp;

		if(0 != IniGetArrayValue(pFilepath, 
			CONFIG_OSD_PARAM, 
			strtmp, 
			EM_CONFIG_OSD_PARAM_RECT_OSD_W + (i * 4),
			&tmp))
		{
			return -1;
		}
		para->nRectOsdW[i] = tmp;

		if(0 != IniGetArrayValue(pFilepath, 
			CONFIG_OSD_PARAM, 
			strtmp, 
			EM_CONFIG_OSD_PARAM_RECT_OSD_H + (i * 4),
			&tmp))
		{
			return -1;
		}
		para->nRectOsdH[i] = tmp;
	}
	
	return 0;
}

s32 ConfigGetMainEncodeParam(SModConfigVideoParam* para, s32 nId, s8* pFilepath)
{
	if((NULL == para) || (nId < 0) || (nId >= g_sConfigDvrProperty.nVidMainNum) || (NULL == pFilepath))
	{
		printf("Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}
	
	#ifdef CONFIG_TEST
	memset(para, 0, sizeof(SModConfigVideoParam));
	para->nBitRate = 512;
	para->nEncodeType = 98;
	para->nFrameRate = 25;
	para->nGop = 40;
	para->nResolution = 1;
	return 0;
	#endif
	
	s32 tmp = 0;
	s32 i = nId;
	s8 strtmp[30];
	memset(strtmp, 0, sizeof(strtmp));
	sprintf(strtmp, "%s%d", CONFIG_ENCODE_PARAM_MAIN, i);

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_ENCODE_PARAM, 
		strtmp, 
		EM_CONFIG_ENCODE_PARAM_BITRATE,
		&tmp))
	{
		return -1;
	}
	para->nBitRate = tmp;

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_ENCODE_PARAM, 
		strtmp, 
		EM_CONFIG_ENCODE_PARAM_FRAMERATE,
		&tmp))
	{
		return -1;
	}
	para->nFrameRate = tmp;

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_ENCODE_PARAM, 
		strtmp, 
		EM_CONFIG_ENCODE_PARAM_GOP,
		&tmp))
	{
		return -1;
	}
	para->nGop = tmp;

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_ENCODE_PARAM, 
		strtmp, 
		EM_CONFIG_ENCODE_PARAM_MINQP,
		&tmp))
	{
		return -1;
	}
	para->nMinQP = tmp;

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_ENCODE_PARAM, 
		strtmp, 
		EM_CONFIG_ENCODE_PARAM_MAXQP,
		&tmp))
	{
		return -1;
	}
	para->nMaxQP = tmp;

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_ENCODE_PARAM, 
		strtmp, 
		EM_CONFIG_ENCODE_PARAM_RESULUTION,
		&tmp))
	{
		return -1;
	}
	para->nResolution = tmp;

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_ENCODE_PARAM, 
		strtmp, 
		EM_CONFIG_ENCODE_PARAM_ENCODETYPE,
		&tmp))
	{
		return -1;
	}
	para->nEncodeType = tmp;

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_ENCODE_PARAM, 
		strtmp, 
		EM_CONFIG_ENCODE_PARAM_BITERATETYPE,
		&tmp))
	{
		return -1;
	}
	para->nBitRateType = tmp;

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_ENCODE_PARAM, 
		strtmp, 
		EM_CONFIG_ENCODE_PARAM_PICLEVEL,
		&tmp))
	{
		return -1;
	}
	para->nPicLevel = tmp;

	return 0;
}

//yaogang modify 20150402 mobile stream
s32 ConfigGetMobEncodeParam(SModConfigVideoParam* para, s32 nId, s8* pFilepath)
{
	if((NULL == para) || (nId < 0) || (nId >= g_sConfigDvrProperty.nVidSubNum) || (NULL == pFilepath))
	{
		printf("Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}

	#ifdef CONFIG_TEST
	memset(para, 0, sizeof(SModConfigVideoParam));
	para->nBitRate = 128;
	para->nEncodeType = 98;
	para->nFrameRate = 6;
	para->nGop = 40;
	para->nResolution = 1;
	return 0;
	#endif
	
	s32 tmp = 0;
	s32 i = nId;
	s8 strtmp[30];
	memset(strtmp, 0, sizeof(strtmp));
	sprintf(strtmp, "%s%d", CONFIG_ENCODE_PARAM_MOB, i);

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_ENCODE_PARAM, 
		strtmp, 
		EM_CONFIG_ENCODE_PARAM_BITRATE,
		&tmp))
	{
		return -1;
	}
	para->nBitRate = tmp;

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_ENCODE_PARAM, 
		strtmp, 
		EM_CONFIG_ENCODE_PARAM_FRAMERATE,
		&tmp))
	{
		return -1;
	}
	para->nFrameRate = tmp;

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_ENCODE_PARAM, 
		strtmp, 
		EM_CONFIG_ENCODE_PARAM_GOP,
		&tmp))
	{
		return -1;
	}
	para->nGop = tmp;

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_ENCODE_PARAM, 
		strtmp, 
		EM_CONFIG_ENCODE_PARAM_MINQP,
		&tmp))
	{
		return -1;
	}
	para->nMinQP = tmp;

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_ENCODE_PARAM, 
		strtmp, 
		EM_CONFIG_ENCODE_PARAM_MAXQP,
		&tmp))
	{
		return -1;
	}
	para->nMaxQP = tmp;

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_ENCODE_PARAM, 
		strtmp, 
		EM_CONFIG_ENCODE_PARAM_RESULUTION,
		&tmp))
	{
		return -1;
	}
	para->nResolution = tmp;

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_ENCODE_PARAM, 
		strtmp, 
		EM_CONFIG_ENCODE_PARAM_ENCODETYPE,
		&tmp))
	{
		return -1;
	}
	para->nEncodeType = tmp;

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_ENCODE_PARAM, 
		strtmp, 
		EM_CONFIG_ENCODE_PARAM_BITERATETYPE,
		&tmp))
	{
		return -1;
	}
	para->nBitRateType = tmp;

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_ENCODE_PARAM, 
		strtmp, 
		EM_CONFIG_ENCODE_PARAM_PICLEVEL,
		&tmp))
	{
		return -1;
	}
	para->nPicLevel = tmp;

	return 0;
}

s32 ConfigGetSubEncodeParam(SModConfigVideoParam* para, s32 nId, s8* pFilepath)
{
	if((NULL == para) || (nId < 0) || (nId >= g_sConfigDvrProperty.nVidSubNum) || (NULL == pFilepath))
	{
		printf("Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}

	#ifdef CONFIG_TEST
	memset(para, 0, sizeof(SModConfigVideoParam));
	para->nBitRate = 128;
	para->nEncodeType = 98;
	para->nFrameRate = 6;
	para->nGop = 40;
	para->nResolution = 1;
	return 0;
	#endif
	
	s32 tmp = 0;
	s32 i = nId;
	s8 strtmp[30];
	memset(strtmp, 0, sizeof(strtmp));
	sprintf(strtmp, "%s%d", CONFIG_ENCODE_PARAM_SUB, i);

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_ENCODE_PARAM, 
		strtmp, 
		EM_CONFIG_ENCODE_PARAM_BITRATE,
		&tmp))
	{
		return -1;
	}
	para->nBitRate = tmp;

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_ENCODE_PARAM, 
		strtmp, 
		EM_CONFIG_ENCODE_PARAM_FRAMERATE,
		&tmp))
	{
		return -1;
	}
	para->nFrameRate = tmp;

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_ENCODE_PARAM, 
		strtmp, 
		EM_CONFIG_ENCODE_PARAM_GOP,
		&tmp))
	{
		return -1;
	}
	para->nGop = tmp;

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_ENCODE_PARAM, 
		strtmp, 
		EM_CONFIG_ENCODE_PARAM_MINQP,
		&tmp))
	{
		return -1;
	}
	para->nMinQP = tmp;

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_ENCODE_PARAM, 
		strtmp, 
		EM_CONFIG_ENCODE_PARAM_MAXQP,
		&tmp))
	{
		return -1;
	}
	para->nMaxQP = tmp;

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_ENCODE_PARAM, 
		strtmp, 
		EM_CONFIG_ENCODE_PARAM_RESULUTION,
		&tmp))
	{
		return -1;
	}
	para->nResolution = tmp;

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_ENCODE_PARAM, 
		strtmp, 
		EM_CONFIG_ENCODE_PARAM_ENCODETYPE,
		&tmp))
	{
		return -1;
	}
	para->nEncodeType = tmp;

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_ENCODE_PARAM, 
		strtmp, 
		EM_CONFIG_ENCODE_PARAM_BITERATETYPE,
		&tmp))
	{
		return -1;
	}
	para->nBitRateType = tmp;

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_ENCODE_PARAM, 
		strtmp, 
		EM_CONFIG_ENCODE_PARAM_PICLEVEL,
		&tmp))
	{
		return -1;
	}
	para->nPicLevel = tmp;

	return 0;
}

s32 ConfigGetRecordParam(SModConfigRecordParam* para, s32 nId, s8* pFilepath)
{
	if((NULL == para) || (nId < 0) || (nId >= g_sConfigDvrProperty.nRecNum) || (NULL == pFilepath))
	{
		printf("Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}

	#ifdef CONFIG_TEST
	memset(para, 0, sizeof(SModConfigRecordParam));
	para->nDelayTime = 5;
	para->nEncChn = nId;
	para->nExpireDays = 255;
	para->nPreTime = 5;
	para->nStreamType = 1;
	return 0;
	#endif
	
	s32 tmp = 0;
	s32 i = nId;
	s8 strtmp[30];
	memset(strtmp, 0, sizeof(strtmp));
	sprintf(strtmp, "%s%d", CONFIG_RECORD_PARAM_REC, i);

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_RECORD_PARAM, 
		strtmp, 
		EM_CONFIG_RECORD_PARAM_REC_RECORDING,
		&tmp))
	{
		return -1;
	}
	para->bRecording =tmp;
	
	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_RECORD_PARAM, 
		strtmp, 
		EM_CONFIG_RECORD_PARAM_REC_ENABLE,
		&tmp))
	{
		return -1;
	}
	para->nEnable = tmp;
	
	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_RECORD_PARAM, 
		strtmp, 
		EM_CONFIG_RECORD_PARAM_REC_STREAMTYPE,
		&tmp))
	{
		return -1;
	}
	para->nStreamType = tmp;

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_RECORD_PARAM, 
		strtmp, 
		EM_CONFIG_RECORD_PARAM_REC_ENCCHN,
		&tmp))
	{
		return -1;
	}
	para->nEncChn = tmp;

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_RECORD_PARAM, 
		strtmp, 
		EM_CONFIG_RECORD_PARAM_REC_ENCCHN_TYPE,
		&tmp))
	{
		return -1;
	}
	para->nChnEncType = tmp;

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_RECORD_PARAM, 
		strtmp, 
		EM_CONFIG_RECORD_PARAM_REC_PRETIME,
		&tmp))
	{
		return -1;
	}
	para->nPreTime = tmp;

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_RECORD_PARAM, 
		strtmp, 
		EM_CONFIG_RECORD_PARAM_REC_DELAYTIME,
		&tmp))
	{
		return -1;
	}
	para->nDelayTime = tmp;

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_RECORD_PARAM, 
		strtmp, 
		EM_CONFIG_RECORD_PARAM_REC_EXPIRE,
		&tmp))
	{
		return -1;
	}
	para->nExpireDays = tmp;

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_RECORD_PARAM, 
		strtmp, 
		EM_CONFIG_RECORD_PARAM_REC_SCHTYPE_TIMER,
		&tmp))
	{
		return -1;
	}
	para->nTimerRecSchType = tmp;

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_RECORD_PARAM, 
		strtmp, 
		EM_CONFIG_RECORD_PARAM_REC_SCHTYPE_MOTION,
		&tmp))
	{
		return -1;
	}
	para->nMDRecSchType = tmp;

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_RECORD_PARAM, 
		strtmp, 
		EM_CONFIG_RECORD_PARAM_REC_SCHTYPE_SENSOR,
		&tmp))
	{
		return -1;
	}
	para->nSensorRecSchType = tmp;

	return 0;
}

s32 ConfigGetTimerRecSchParam(SModConfigRecTimerSch* para, s32 nId, s8* pFilepath)
{
	if((NULL == para) || (nId < 0) || (nId >= g_sConfigDvrProperty.nRecNum) || (NULL == pFilepath))
	{
		printf("Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}
	
	#ifdef CONFIG_TEST
	memset(para, 0, sizeof(SModConfigRecTimerSch));
	return 0;
	#endif
	
	s32 tmp = 0;
	s32 i = nId;
	s32 j = 0;
	s8 strtmp[30];
	memset(strtmp, 0, sizeof(strtmp));
	sprintf(strtmp, "%s%d", CONFIG_RECORD_PARAM_REC, nId);
	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_RECORD_PARAM, 
		strtmp, 
		EM_CONFIG_RECORD_PARAM_REC_SCHTYPE_TIMER,
		&tmp))
	{
		return -1;
	}
	para->psRecSch.nSchType = tmp;
	
	u8 daynum = 0;
	if(para->psRecSch.nSchType == 0)
	{
		daynum = 7;
	}
	else if(para->psRecSch.nSchType == 1)
	{
		daynum = 31;
	}
	else if(para->psRecSch.nSchType == 2)
	{
		daynum = 1;
	}
	sprintf(strtmp, "%s%d", CONFIG_SCH_PARAM, nId);
	
	s8 strtmp2[30];
	memset(strtmp2, 0, sizeof(strtmp2));
	
	for(i = 0; i < daynum; i++)
	{
		sprintf(strtmp2, "%s%d", CONFIG_SCH_PARAM_REC_TIMER, i);
		j = 0;
		while(1)
		{
			if(0 != IniGetArrayValue(pFilepath, 
				strtmp, 
				strtmp2, 
				EM_CONFIG_SCH_PARAM_STARTTIME + (2 * j),
				&tmp))
			{
				break;
			}
			para->psRecSch.nSchTime[i][j].nStartTime = tmp;
			
			if(0 != IniGetArrayValue(pFilepath, 
				strtmp, 
				strtmp2, 
				EM_CONFIG_SCH_PARAM_ENDTIME + (2 * j),
				&tmp))
			{
				break;
			}
			para->psRecSch.nSchTime[i][j].nStopTime = tmp;
			
			//printf("day %d seg %d st %d et %d \n", i, j, para->psRecSch.nSchTime[i][j].nStartTime, para->psRecSch.nSchTime[i][j].nStartTime);
			
			j++;
			if((j >= g_sConfigDvrProperty.nSchSegments) || (j >= CONFIG_MAX_SCH_TIME_SEGMENTS))
			{
				break;
			}
		}
	}
	
	return 0;
}

s32 ConfigGetVMotionRecSchParam(SModConfigRecVMSch* para, s32 nId, s8* pFilepath)
{
	if((NULL == para) || (nId < 0) || (nId >= g_sConfigDvrProperty.nRecNum) || (NULL == pFilepath))
	{
		printf("Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}

	#ifdef CONFIG_TEST
	memset(para, 0, sizeof(SModConfigRecVMSch));
	return 0;
	#endif
	
	s32 tmp = 0;
	s32 i = nId;
	s32 j = 0;
	s8 strtmp[30];
	memset(strtmp, 0, sizeof(strtmp));
	sprintf(strtmp, "%s%d", CONFIG_RECORD_PARAM_REC, nId);
	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_RECORD_PARAM, 
		strtmp, 
		EM_CONFIG_RECORD_PARAM_REC_SCHTYPE_MOTION,
		&tmp))
	{
		return -1;
	}
	para->psRecSch.nSchType = tmp;
	
	u8 daynum = 0;
	if(para->psRecSch.nSchType == 0)
	{
		daynum = 7;
	}
	else if(para->psRecSch.nSchType == 1)
	{
		daynum = 31;
	}
	else if(para->psRecSch.nSchType == 2)
	{
		daynum = 1;
	}
	sprintf(strtmp, "%s%d", CONFIG_SCH_PARAM, nId);
	s8 strtmp2[30];
	memset(strtmp2, 0, sizeof(strtmp2));
	
	for(i = 0; i < daynum; i++)
	{		
		sprintf(strtmp2, "%s%d", CONFIG_SCH_PARAM_REC_VMOTION, i);
		j = 0;
		while(1)
		{			
			if(0 != IniGetArrayValue(pFilepath, 
				strtmp, 
				strtmp2, 
				EM_CONFIG_SCH_PARAM_STARTTIME + (2 * j),
				&tmp))
			{
				break;
			}
			para->psRecSch.nSchTime[i][j].nStartTime = tmp;

			if(0 != IniGetArrayValue(pFilepath, 
				strtmp, 
				strtmp2, 
				EM_CONFIG_SCH_PARAM_ENDTIME + (2 * j),
				&tmp))
			{
				break;
			}
			para->psRecSch.nSchTime[i][j].nStopTime = tmp;

			j++;
			if((j >= g_sConfigDvrProperty.nSchSegments) || (j >= CONFIG_MAX_SCH_TIME_SEGMENTS))
			{
				break;
			}
		}
	}
	
	return 0;
}

s32 ConfigGetSensorRecSchParam(SModConfigRecAlarmInSch* para, s32 nId, s8* pFilepath)
{
	if((NULL == para) || (nId < 0) || (nId >= g_sConfigDvrProperty.nRecNum) || (NULL == pFilepath))
	{
		printf("Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}

	#ifdef CONFIG_TEST
	memset(para, 0, sizeof(SModConfigRecAlarmInSch));
	return 0;
	#endif
	
	s32 tmp = 0;
	s32 i = nId;
	s32 j = 0;
	s8 strtmp[30];
	memset(strtmp, 0, sizeof(strtmp));
	sprintf(strtmp, "%s%d", CONFIG_RECORD_PARAM_REC, nId);
	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_RECORD_PARAM, 
		strtmp, 
		EM_CONFIG_RECORD_PARAM_REC_SCHTYPE_SENSOR,
		&tmp))
	{
		return -1;
	}
	para->psRecSch.nSchType = tmp;

	u8 daynum = 0;
	if(para->psRecSch.nSchType == 0)
	{
		daynum = 7;
	}
	else if(para->psRecSch.nSchType == 1)
	{
		daynum = 31;
	}
	else if(para->psRecSch.nSchType == 2)
	{
		daynum = 1;
	}
	sprintf(strtmp, "%s%d", CONFIG_SCH_PARAM, nId);
	s8 strtmp2[30];
	memset(strtmp2, 0, sizeof(strtmp2));
	
	for(i = 0; i < daynum; i++)
	{		
		sprintf(strtmp2, "%s%d", CONFIG_SCH_PARAM_REC_SENSOR, i);
		j = 0;
		while(1)
		{			
			if(0 != IniGetArrayValue(pFilepath, 
				strtmp, 
				strtmp2, 
				EM_CONFIG_SCH_PARAM_STARTTIME + (2 * j),
				&tmp))
			{
				break;
			}
			para->psRecSch.nSchTime[i][j].nStartTime = tmp;

			if(0 != IniGetArrayValue(pFilepath, 
				strtmp, 
				strtmp2, 
				EM_CONFIG_SCH_PARAM_ENDTIME + (2 * j),
				&tmp))
			{
				break;
			}
			para->psRecSch.nSchTime[i][j].nStopTime = tmp;

			j++;
			if((j >= g_sConfigDvrProperty.nSchSegments) || (j >= CONFIG_MAX_SCH_TIME_SEGMENTS))
			{
				break;
			}
		}
	}
	
	return 0;
}
//yaogang modify 20141209 深广平台参数
s32 ConfigGetSGParam(SModConfigSGParam* para, s8* pFilepath)
{
	if((NULL == para) ||(NULL == pFilepath))
	{
		printf("Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}
	
	s32 tmp = 0;
	u64 tmp64 = 0;
	s32 i = 0;
	s8 strtmp[32];
	s8 str[100];
	
	memset(strtmp, 0, sizeof(strtmp));
	sprintf(strtmp, "%s", CONFIG_SG_PARAM_SG);

	if(0 != IniGetArrayString(pFilepath,
		CONFIG_SG_PARAM,
		strtmp,
		EM_CONFIG_SG_PARAM_AgentID,
		str,
		sizeof(str)))
	{
		printf("%s: EM_CONFIG_SG_PARAM_AgentID failed\n", __FUNCTION__);
		return -1;
	}
	//printf("%s: EM_CONFIG_SG_PARAM_AgentID: %s\n", __FUNCTION__, str);
	if(strncmp(str, "******", 6) == 0)
	{
		//memset(str, 0, sizeof(str));
		strcpy(str, "abc");
	}
	
	strcpy(para->AgentID, str);

	if(0 != IniGetArrayString(pFilepath,
		CONFIG_SG_PARAM,
		strtmp,
		EM_CONFIG_SG_PARAM_CenterIP,
		str,
		sizeof(str)))
	{
		printf("%s: EM_CONFIG_SG_PARAM_CenterIP failed\n", __FUNCTION__);
		return -1;
	}
	//printf("%s: EM_CONFIG_SG_PARAM_CenterIP: %s\n", __FUNCTION__, str);
	if(strncmp(str, "******", 6) == 0)
	{
		memset(str, 0, sizeof(str));
	}
	strcpy(para->CenterIP, str);
	
	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_SG_PARAM, 
		strtmp, 
		EM_CONFIG_SG_PARAM_RunEnable,
		&tmp))
	{
		printf("%s: EM_CONFIG_SG_PARAM_RunEnable failed\n", __FUNCTION__);
		return -1;
	}
	//printf("%s: EM_CONFIG_SG_PARAM_RunEnable: %d\n", __FUNCTION__, tmp);
	para->RunEnable = tmp;

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_SG_PARAM, 
		strtmp, 
		EM_CONFIG_SG_PARAM_AlarmEnable,
		&tmp))
	{
		printf("%s: EM_CONFIG_SG_PARAM_AlarmEnable failed\n", __FUNCTION__);
		return -1;
	}
	//printf("%s: EM_CONFIG_SG_PARAM_AlarmEnable: %d\n", __FUNCTION__, tmp);
	para->AlarmEnable = tmp;
	
	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_SG_PARAM, 
		strtmp, 
		EM_CONFIG_SG_PARAM_DailyPicEnable,
		&tmp))
	{
		printf("%s: EM_CONFIG_SG_PARAM_DailyPicEnable failed\n", __FUNCTION__);
		return -1;
	}
	//printf("%s: EM_CONFIG_SG_PARAM_DailyPicEnable: %d\n", __FUNCTION__, tmp);
	para->DailyPicEnable = tmp;

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_SG_PARAM, 
		strtmp, 
		EM_CONFIG_DVR_EXIT_NORMOAL,
		&tmp))
	{
		printf("%s: EM_CONFIG_DVR_EXIT_NORMOAL failed\n", __FUNCTION__);
		return -1;
	}
	//printf("%s: EM_CONFIG_DVR_EXIT_NORMOAL: %d\n", __FUNCTION__, tmp);
	para->normal_exit = tmp;

#if 0
	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_SG_PARAM, 
		strtmp, 
		EM_CONFIG_SG_PARAM_PreTimes,
		&tmp))
	{
		printf("%s: EM_CONFIG_SG_PARAM_PreTimes failed\n", __FUNCTION__);
		return -1;
	}
	//printf("%s: EM_CONFIG_SG_PARAM_PreTimes: %d\n", __FUNCTION__, tmp);
	para->SAlarmPicParam.PreTimes = tmp;

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_SG_PARAM, 
		strtmp, 
		EM_CONFIG_SG_PARAM_Interval,
		&tmp))
	{
		printf("%s: EM_CONFIG_SG_PARAM_Interval failed\n", __FUNCTION__);
		return -1;
	}
	//printf("%s: EM_CONFIG_SG_PARAM_Interval: %d\n", __FUNCTION__, tmp);
	para->SAlarmPicParam.Interval = tmp;

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_SG_PARAM, 
		strtmp, 
		EM_CONFIG_SG_PARAM_StillTimes,
		&tmp))
	{
		printf("%s: EM_CONFIG_SG_PARAM_StillTimes failed\n", __FUNCTION__);
		return -1;
	}
	//printf("%s: EM_CONFIG_SG_PARAM_StillTimes: %d\n", __FUNCTION__, tmp);
	para->SAlarmPicParam.StillTimes = tmp;

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_SG_PARAM, 
		strtmp, 
		EM_CONFIG_SG_PARAM_AlarmInput,
		&tmp))
	{
		printf("%s: EM_CONFIG_SG_PARAM_AlarmInput failed\n", __FUNCTION__);
		return -1;
	}
	//printf("%s: EM_CONFIG_SG_PARAM_AlarmInput: %d\n", __FUNCTION__, tmp);
	para->SAlarmPicParam.AlarmInput = tmp;

	if(0 != IniGetArrayValueU64(pFilepath, 
		CONFIG_SG_PARAM, 
		strtmp, 
		EM_CONFIG_SG_PARAM_IpcChn,
		&tmp64))
	{
		printf("%s: EM_CONFIG_SG_PARAM_IpcChn failed\n", __FUNCTION__);
		return -1;
	}
	//printf("%s: EM_CONFIG_SG_PARAM_IpcChn: 0x%x\n", __FUNCTION__, tmp64);
	para->SAlarmPicParam.IpcChn = tmp64;

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_SG_PARAM, 
		strtmp, 
		EM_CONFIG_SG_PARAM_Time1Enable,
		&tmp))
	{
		printf("%s: EM_CONFIG_SG_PARAM_Time1Enable failed\n", __FUNCTION__);
		return -1;
	}
	//printf("%s: EM_CONFIG_SG_PARAM_Time1Enable: %d\n", __FUNCTION__, tmp);
	para->SDailyPicParam.Time1.TimeEnable = tmp;

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_SG_PARAM, 
		strtmp, 
		EM_CONFIG_SG_PARAM_Time1Hour,
		&tmp))
	{
		printf("%s: EM_CONFIG_SG_PARAM_Time1Hour failed\n", __FUNCTION__);
		return -1;
	}
	//printf("%s: EM_CONFIG_SG_PARAM_Time1Hour: %d\n", __FUNCTION__, tmp);
	para->SDailyPicParam.Time1.hour= tmp;
	
	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_SG_PARAM, 
		strtmp, 
		EM_CONFIG_SG_PARAM_Time1Min,
		&tmp))
	{
		printf("%s: EM_CONFIG_SG_PARAM_Time1Min failed\n", __FUNCTION__);
		return -1;
	}
	//printf("%s: EM_CONFIG_SG_PARAM_Time1Min: %d\n", __FUNCTION__, tmp);
	para->SDailyPicParam.Time1.min= tmp;
	
	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_SG_PARAM, 
		strtmp, 
		EM_CONFIG_SG_PARAM_Time1Sec,
		&tmp))
	{
		printf("%s: EM_CONFIG_SG_PARAM_Time1Sec failed\n", __FUNCTION__);
		return -1;
	}
	//printf("%s: EM_CONFIG_SG_PARAM_Time1Sec: %d\n", __FUNCTION__, tmp);
	para->SDailyPicParam.Time1.sec= tmp;

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_SG_PARAM, 
		strtmp, 
		EM_CONFIG_SG_PARAM_Time2Enable,
		&tmp))
	{
		printf("%s: EM_CONFIG_SG_PARAM_Time2Enable failed\n", __FUNCTION__);
		return -1;
	}
	//printf("%s: EM_CONFIG_SG_PARAM_Time2Enable: %d\n", __FUNCTION__, tmp);
	para->SDailyPicParam.Time2.TimeEnable = tmp;

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_SG_PARAM, 
		strtmp, 
		EM_CONFIG_SG_PARAM_Time2Hour,
		&tmp))
	{
		printf("%s: EM_CONFIG_SG_PARAM_Time2Hour failed\n", __FUNCTION__);
		return -1;
	}
	//printf("%s: EM_CONFIG_SG_PARAM_Time2Hour: %d\n", __FUNCTION__, tmp);
	para->SDailyPicParam.Time2.hour= tmp;
	
	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_SG_PARAM, 
		strtmp, 
		EM_CONFIG_SG_PARAM_Time2Min,
		&tmp))
	{
		printf("%s: EM_CONFIG_SG_PARAM_Time2Min failed\n", __FUNCTION__);
		return -1;
	}
	//printf("%s: EM_CONFIG_SG_PARAM_Time2Min: %d\n", __FUNCTION__, tmp);
	para->SDailyPicParam.Time2.min= tmp;
	
	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_SG_PARAM, 
		strtmp, 
		EM_CONFIG_SG_PARAM_Time2Sec,
		&tmp))
	{
		printf("%s: EM_CONFIG_SG_PARAM_Time2Sec failed\n", __FUNCTION__);
		return -1;
	}
	//printf("%s: EM_CONFIG_SG_PARAM_Time2Sec: %d\n", __FUNCTION__, tmp);
	para->SDailyPicParam.Time2.sec= tmp;

	if(0 != IniGetArrayValueU64(pFilepath, 
		CONFIG_SG_PARAM, 
		strtmp, 
		EM_CONFIG_SG_PARAM_IpcChn1,
		&tmp64))
	{
		printf("%s: EM_CONFIG_SG_PARAM_IpcChn1 failed\n", __FUNCTION__);
		return -1;
	}
	//printf("%s: EM_CONFIG_SG_PARAM_IpcChn1: 0x%x\n", __FUNCTION__, tmp64);
	para->SDailyPicParam.IpcChn1 = tmp64;

	if(0 != IniGetArrayValueU64(pFilepath, 
		CONFIG_SG_PARAM, 
		strtmp, 
		EM_CONFIG_SG_PARAM_IpcChn2,
		&tmp64))
	{
		printf("%s: EM_CONFIG_SG_PARAM_IpcChn2 failed\n", __FUNCTION__);
		return -1;
	}
	//printf("%s: EM_CONFIG_SG_PARAM_IpcChn2: 0x%x\n", __FUNCTION__, tmp64);
	para->SDailyPicParam.IpcChn2 = tmp64;
#endif	
	return 0;
}

s32 ConfigGetSGDailyParam(SModDailyPicCFG* para, s8* pFilepath)
{
	if((NULL == para) ||(NULL == pFilepath))
	{
		printf("Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}
	
	s32 tmp = 0;
	u64 tmp64 = 0;
	s32 i = 0;
	s8 strtmp[32];
	s8 str[100];
	
	memset(strtmp, 0, sizeof(strtmp));
	//sprintf(strtmp, "%s", CONFIG_SG_PARAM_SG);
	sprintf(strtmp, "%s", "Daily");

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_SG_PARAM, 
		strtmp, 
		EM_CONFIG_SG_PARAM_Time1Enable,
		&tmp))
	{
		printf("%s: EM_CONFIG_SG_PARAM_Time1Enable failed\n", __FUNCTION__);
		return -1;
	}
	//printf("%s: EM_CONFIG_SG_PARAM_Time1Enable: %d\n", __FUNCTION__, tmp);
	para->Time1.TimeEnable = tmp;

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_SG_PARAM, 
		strtmp, 
		EM_CONFIG_SG_PARAM_Time1Hour,
		&tmp))
	{
		printf("%s: EM_CONFIG_SG_PARAM_Time1Hour failed\n", __FUNCTION__);
		return -1;
	}
	//printf("%s: EM_CONFIG_SG_PARAM_Time1Hour: %d\n", __FUNCTION__, tmp);
	para->Time1.hour= tmp;
	
	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_SG_PARAM, 
		strtmp, 
		EM_CONFIG_SG_PARAM_Time1Min,
		&tmp))
	{
		printf("%s: EM_CONFIG_SG_PARAM_Time1Min failed\n", __FUNCTION__);
		return -1;
	}
	//printf("%s: EM_CONFIG_SG_PARAM_Time1Min: %d\n", __FUNCTION__, tmp);
	para->Time1.min= tmp;
	
	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_SG_PARAM, 
		strtmp, 
		EM_CONFIG_SG_PARAM_Time1Sec,
		&tmp))
	{
		printf("%s: EM_CONFIG_SG_PARAM_Time1Sec failed\n", __FUNCTION__);
		return -1;
	}
	//printf("%s: EM_CONFIG_SG_PARAM_Time1Sec: %d\n", __FUNCTION__, tmp);
	para->Time1.sec= tmp;

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_SG_PARAM, 
		strtmp, 
		EM_CONFIG_SG_PARAM_Time2Enable,
		&tmp))
	{
		printf("%s: EM_CONFIG_SG_PARAM_Time2Enable failed\n", __FUNCTION__);
		return -1;
	}
	//printf("%s: EM_CONFIG_SG_PARAM_Time2Enable: %d\n", __FUNCTION__, tmp);
	para->Time2.TimeEnable = tmp;

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_SG_PARAM, 
		strtmp, 
		EM_CONFIG_SG_PARAM_Time2Hour,
		&tmp))
	{
		printf("%s: EM_CONFIG_SG_PARAM_Time2Hour failed\n", __FUNCTION__);
		return -1;
	}
	//printf("%s: EM_CONFIG_SG_PARAM_Time2Hour: %d\n", __FUNCTION__, tmp);
	para->Time2.hour= tmp;
	
	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_SG_PARAM, 
		strtmp, 
		EM_CONFIG_SG_PARAM_Time2Min,
		&tmp))
	{
		printf("%s: EM_CONFIG_SG_PARAM_Time2Min failed\n", __FUNCTION__);
		return -1;
	}
	//printf("%s: EM_CONFIG_SG_PARAM_Time2Min: %d\n", __FUNCTION__, tmp);
	para->Time2.min= tmp;
	
	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_SG_PARAM, 
		strtmp, 
		EM_CONFIG_SG_PARAM_Time2Sec,
		&tmp))
	{
		printf("%s: EM_CONFIG_SG_PARAM_Time2Sec failed\n", __FUNCTION__);
		return -1;
	}
	//printf("%s: EM_CONFIG_SG_PARAM_Time2Sec: %d\n", __FUNCTION__, tmp);
	para->Time2.sec= tmp;

	if(0 != IniGetArrayValueU64(pFilepath, 
		CONFIG_SG_PARAM, 
		strtmp, 
		EM_CONFIG_SG_PARAM_IpcChn1,
		&tmp64))
	{
		printf("%s: EM_CONFIG_SG_PARAM_IpcChn1 failed\n", __FUNCTION__);
		return -1;
	}
	//printf("%s: EM_CONFIG_SG_PARAM_IpcChn1: 0x%x\n", __FUNCTION__, tmp64);
	para->IpcChn1 = tmp64;

	if(0 != IniGetArrayValueU64(pFilepath, 
		CONFIG_SG_PARAM, 
		strtmp, 
		EM_CONFIG_SG_PARAM_IpcChn2,
		&tmp64))
	{
		printf("%s: EM_CONFIG_SG_PARAM_IpcChn2 failed\n", __FUNCTION__);
		return -1;
	}
	//printf("%s: EM_CONFIG_SG_PARAM_IpcChn2: 0x%x\n", __FUNCTION__, tmp64);
	para->IpcChn2 = tmp64;

	return 0;
}
s32 ConfigGetSGAlarmParam(SModAlarmPicCFG* para, s32 nId, s8* pFilepath)
{
	int num = g_sConfigDvrProperty.nSensorNum + g_sConfigDvrProperty.nVidMainNum;
	if((NULL == para) || (nId < 0) || (nId >= num) || (NULL == pFilepath))
	{
		printf("Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}
	s32 tmp = 0;
	u64 tmp64 = 0;
	s32 i = 0;
	s8 strtmp[32];
	s8 str[100];
	
	memset(strtmp, 0, sizeof(strtmp));
	//sprintf(strtmp, "%s", CONFIG_SG_PARAM_SG);
	sprintf(strtmp, "Chn%d", nId);

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_SG_PARAM, 
		strtmp, 
		EM_CONFIG_SG_PARAM_PreTimes,
		&tmp))
	{
		printf("%s: EM_CONFIG_SG_PARAM_PreTimes failed\n", __FUNCTION__);
		return -1;
	}
	printf("%s: chn: %d, EM_CONFIG_SG_PARAM_PreTimes: %d\n", __FUNCTION__, nId, tmp);
	para->PreTimes = tmp; 

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_SG_PARAM, 
		strtmp, 
		EM_CONFIG_SG_PARAM_Interval,
		&tmp))
	{
		printf("%s: EM_CONFIG_SG_PARAM_Interval failed\n", __FUNCTION__);
		return -1;
	}
	//printf("%s: EM_CONFIG_SG_PARAM_Interval: %d\n", __FUNCTION__, tmp);
	para->Interval = tmp;

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_SG_PARAM, 
		strtmp, 
		EM_CONFIG_SG_PARAM_StillTimes,
		&tmp))
	{
		printf("%s: EM_CONFIG_SG_PARAM_StillTimes failed\n", __FUNCTION__);
		return -1;
	}
	//printf("%s: EM_CONFIG_SG_PARAM_StillTimes: %d\n", __FUNCTION__, tmp);
	para->StillTimes = tmp;
	
#if 0
	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_SG_PARAM, 
		strtmp, 
		EM_CONFIG_SG_PARAM_AlarmInput,
		&tmp))
	{
		printf("%s: EM_CONFIG_SG_PARAM_AlarmInput failed\n", __FUNCTION__);
		return -1;
	}
	//printf("%s: EM_CONFIG_SG_PARAM_AlarmInput: %d\n", __FUNCTION__, tmp);
	para->SAlarmPicParam.AlarmInput = tmp;
#endif

	if(0 != IniGetArrayValueU64(pFilepath, 
		CONFIG_SG_PARAM, 
		strtmp, 
		EM_CONFIG_SG_PARAM_IpcChn,
		&tmp64))
	{
		printf("%s: EM_CONFIG_SG_PARAM_IpcChn failed\n", __FUNCTION__);
		return -1;
	}
	//printf("%s: EM_CONFIG_SG_PARAM_IpcChn: 0x%x\n", __FUNCTION__, tmp64);
	para->IpcChn = tmp64;
	
	return 0;
}

s32 ConfigGetSensorParam(SModConfigSensorParam* para, s32 nId, s8* pFilepath)
{
	if((NULL == para) || (nId < 0) || (nId >= g_sConfigDvrProperty.nSensorNum) || (NULL == pFilepath))
	{
		printf("Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}

	#ifdef CONFIG_TEST
	memset(para, 0, sizeof(SModConfigSensorParam));
	para->nDelay = 5;
	para->nEnalbe = 1;
	sprintf(para->name, "Sensor %2ld", nId);
	return 0;
	#endif
	
	s32 tmp = 0;
	u64 tmp64 = 0;
	s32 i = nId;
	s8 strtmp[30];
	memset(strtmp, 0, sizeof(strtmp));
	sprintf(strtmp, "%s%d", CONFIG_SENSOR_PARAM_SENSOR, i);
	//printf("ConfigGetSensorParam:%s\n", strtmp);
	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_SENSOR_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_ENABLE,
		&tmp))
	{
		printf("ConfigGetSensorParam error-1\n");
		return -1;
	}
	para->nEnalbe = tmp;
	//printf("line:%d\n", __LINE__);
	
	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_SENSOR_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_TYPE,
		&tmp))
	{
		printf("ConfigGetSensorParam error-2\n");
		return -1;
	}
	para->nSensorType = tmp;
	//printf("line:%d\n", __LINE__);

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_SENSOR_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_DELAY,
		&tmp))
	{
		printf("ConfigGetSensorParam error-3\n");
		return -1;
	}
	para->nDelay = tmp;
	//printf("line:%d\n", __LINE__);

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_SENSOR_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_SCHTYPE,
		&tmp))
	{
		printf("ConfigGetSensorParam error-4\n");
		return -1;
	}
	para->nSchType = tmp;
	//printf("line:%d\n", __LINE__);

	if(0 != GetArrayStringFromIni(pFilepath, 
		CONFIG_SENSOR_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_NAME,
		para->name, 
		sizeof(para->name)))
	{
		printf("ConfigGetSensorParam error-5\n");
		return -1;
	}
	//printf("line:%d\n", __LINE__);

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_SENSOR_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_FLAG_BUZZ,
		&tmp))
	{
		printf("ConfigGetSensorParam error-6\n");
		return -1;
	}
	para->psDispatch.nFlagBuzz = tmp;
	//printf("line:%d\n", __LINE__);

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_SENSOR_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_ZOOMCHN,
		&tmp))
	{
		printf("ConfigGetSensorParam error-7\n");
		return -1;
	}
	para->psDispatch.nZoomChn = tmp;
	//printf("line:%d\n", __LINE__);

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_SENSOR_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_FLAG_MAIL,
		&tmp))
	{
		printf("ConfigGetSensorParam error-8\n");
		return -1;
	}
	para->psDispatch.nFlagEmail = tmp;
	//printf("line:%d\n", __LINE__);

	if(0 != IniGetArrayValueU64(pFilepath, 
		CONFIG_SENSOR_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_SNAPCHN,
		&tmp64))
	{
		printf("ConfigGetSensorParam error-9\n");
		return -1;
	}
	para->psDispatch.nSnapChn = tmp64;
	//printf("line:%d\n", __LINE__);

	if(0 != IniGetArrayValueU64(pFilepath, 
		CONFIG_SENSOR_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_ALARMOUT,
		&tmp64))
	{
		printf("ConfigGetSensorParam error-10\n");
		return -1;
	}
	para->psDispatch.nAlarmOut = tmp64;
	//printf("line:%d\n", __LINE__);

	if(0 != IniGetArrayValueU64(pFilepath, 
		CONFIG_SENSOR_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_RECCHN,
		&tmp64))
	{
		printf("ConfigGetSensorParam error-11\n");
		return -1;
	}
	para->psDispatch.nRecordChn = tmp64;
	//printf("line:%d\n", __LINE__);
	
	for(i = 0; i < g_sConfigDvrProperty.nPreviewNum; i++)
	{
		if(0 != IniGetArrayValue(pFilepath, 
			CONFIG_SENSOR_PARAM, 
			strtmp, 
			EM_CONFIG_SENSOR_PARAM_PTZTYPE + (3 * i),
			&tmp))
		{
			printf("ConfigGetSensorParam error-12\n");
			return -1;
		}
		para->psDispatch.sAlarmPtz[i].nPtzType = tmp;

		if(0 != IniGetArrayValue(pFilepath, 
			CONFIG_SENSOR_PARAM, 
			strtmp, 
			EM_CONFIG_SENSOR_PARAM_PTZCHN + (3 * i),
			&tmp))
		{
			printf("ConfigGetSensorParam error-13\n");
			return -1;
		}
		para->psDispatch.sAlarmPtz[i].nChn = tmp;

		if(0 != IniGetArrayValue(pFilepath, 
			CONFIG_SENSOR_PARAM, 
			strtmp, 
			EM_CONFIG_SENSOR_PARAM_PTZID + (3 * i),
			&tmp))
		{
			printf("ConfigGetSensorParam error-14\n");
			return -1;
		}
		para->psDispatch.sAlarmPtz[i].nId = tmp;
	}
	
	return 0;
}
//yaogang modify 20141010
s32 ConfigGetIPCCoverParam(SModConfigIPCExtSensorParam* para, s32 nId, s8* pFilepath)
{
	if((NULL == para) || (nId < 0) || (nId >= g_sConfigDvrProperty.nVidMainNum) || (NULL == pFilepath))
	{
		printf("Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}

	#ifdef CONFIG_TEST
	memset(para, 0, sizeof(SModConfigIPCExtSensorParam));
	para->nDelay = 5;
	para->nEnalbe = 1;
	//sprintf(para->name, "Sensor %2ld", nId);
	return 0;
	#endif
	
	s32 tmp = 0;
	u64 tmp64 = 0;
	s32 i = nId;
	s8 strtmp[30];
	memset(strtmp, 0, sizeof(strtmp));
	sprintf(strtmp, "%s%d", CONFIG_SENSOR_PARAM_SENSOR, i);
	//printf("ConfigGetSensorParam:%s\n", strtmp);
	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_IPCCOVER_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_ENABLE,
		&tmp))
	{
		printf("CONFIG_IPCCOVER_PARAM error-1\n");
		return -1;
	}
	para->nEnalbe = tmp;
	//printf("line:%d\n", __LINE__);

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_IPCCOVER_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_DELAY,
		&tmp))
	{
		printf("CONFIG_IPCCOVER_PARAM error-3\n");
		return -1;
	}
	para->nDelay = tmp;
	//printf("line:%d\n", __LINE__);

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_IPCCOVER_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_FLAG_BUZZ,
		&tmp))
	{
		printf("CONFIG_IPCCOVER_PARAM error-6\n");
		return -1;
	}
	para->psDispatch.nFlagBuzz = tmp;
	//printf("line:%d\n", __LINE__);

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_IPCCOVER_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_ZOOMCHN,
		&tmp))
	{
		printf("CONFIG_IPCCOVER_PARAM error-7\n");
		return -1;
	}
	para->psDispatch.nZoomChn = tmp;
	//printf("line:%d\n", __LINE__);

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_IPCCOVER_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_FLAG_MAIL,
		&tmp))
	{
		printf("CONFIG_IPCCOVER_PARAM error-8\n");
		return -1;
	}
	para->psDispatch.nFlagEmail = tmp;
	//printf("line:%d\n", __LINE__);

	if(0 != IniGetArrayValueU64(pFilepath, 
		CONFIG_IPCCOVER_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_SNAPCHN,
		&tmp64))
	{
		printf("CONFIG_IPCCOVER_PARAM error-9\n");
		return -1;
	}
	para->psDispatch.nSnapChn = tmp64;
	//printf("line:%d\n", __LINE__);

	if(0 != IniGetArrayValueU64(pFilepath, 
		CONFIG_IPCCOVER_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_ALARMOUT,
		&tmp64))
	{
		printf("CONFIG_IPCCOVER_PARAM error-10\n");
		return -1;
	}
	para->psDispatch.nAlarmOut = tmp64;
	//printf("line:%d\n", __LINE__);

	if(0 != IniGetArrayValueU64(pFilepath, 
		CONFIG_IPCCOVER_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_RECCHN,
		&tmp64))
	{
		printf("CONFIG_IPCCOVER_PARAM error-11\n");
		return -1;
	}
	para->psDispatch.nRecordChn = tmp64;
	//printf("line:%d\n", __LINE__);
	
	for(i = 0; i < g_sConfigDvrProperty.nPreviewNum; i++)
	{
		if(0 != IniGetArrayValue(pFilepath, 
			CONFIG_IPCCOVER_PARAM, 
			strtmp, 
			EM_CONFIG_SENSOR_PARAM_PTZTYPE + (3 * i),
			&tmp))
		{
			printf("CONFIG_IPCCOVER_PARAM error-12\n");
			return -1;
		}
		para->psDispatch.sAlarmPtz[i].nPtzType = tmp;

		if(0 != IniGetArrayValue(pFilepath, 
			CONFIG_IPCCOVER_PARAM, 
			strtmp, 
			EM_CONFIG_SENSOR_PARAM_PTZCHN + (3 * i),
			&tmp))
		{
			printf("CONFIG_IPCCOVER_PARAM error-13\n");
			return -1;
		}
		para->psDispatch.sAlarmPtz[i].nChn = tmp;

		if(0 != IniGetArrayValue(pFilepath, 
			CONFIG_IPCCOVER_PARAM, 
			strtmp, 
			EM_CONFIG_SENSOR_PARAM_PTZID + (3 * i),
			&tmp))
		{
			printf("CONFIG_IPCCOVER_PARAM error-14\n");
			return -1;
		}
		para->psDispatch.sAlarmPtz[i].nId = tmp;
	}
	
	return 0;
}

s32 ConfigGetIPCExtSensorParam(SModConfigIPCExtSensorParam* para, s32 nId, s8* pFilepath)
{
	if((NULL == para) || (nId < 0) || (nId >= g_sConfigDvrProperty.nVidMainNum) || (NULL == pFilepath))
	{
		printf("Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}

	#ifdef CONFIG_TEST
	memset(para, 0, sizeof(SModConfigIPCExtSensorParam));
	para->nDelay = 5;
	para->nEnalbe = 1;
	//sprintf(para->name, "Sensor %2ld", nId);
	return 0;
	#endif
	
	s32 tmp = 0;
	u64 tmp64 = 0;
	s32 i = nId;
	s8 strtmp[30];
	memset(strtmp, 0, sizeof(strtmp));
	sprintf(strtmp, "%s%d", CONFIG_SENSOR_PARAM_SENSOR, i);
	//printf("ConfigGetSensorParam:%s\n", strtmp);
	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_IPCExtSENSOR_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_ENABLE,
		&tmp))
	{
		printf("ConfigGetIPCExtSensorParam error-1\n");
		return -1;
	}
	para->nEnalbe = tmp;
	//printf("line:%d\n", __LINE__);

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_IPCExtSENSOR_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_DELAY,
		&tmp))
	{
		printf("ConfigGetIPCExtSensorParam error-3\n");
		return -1;
	}
	para->nDelay = tmp;
	//printf("line:%d\n", __LINE__);

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_IPCExtSENSOR_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_FLAG_BUZZ,
		&tmp))
	{
		printf("ConfigGetIPCExtSensorParam error-6\n");
		return -1;
	}
	para->psDispatch.nFlagBuzz = tmp;
	//printf("line:%d\n", __LINE__);

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_IPCExtSENSOR_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_ZOOMCHN,
		&tmp))
	{
		printf("ConfigGetIPCExtSensorParam error-7\n");
		return -1;
	}
	para->psDispatch.nZoomChn = tmp;
	//printf("line:%d\n", __LINE__);

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_IPCExtSENSOR_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_FLAG_MAIL,
		&tmp))
	{
		printf("ConfigGetIPCExtSensorParam error-8\n");
		return -1;
	}
	para->psDispatch.nFlagEmail = tmp;
	//printf("line:%d\n", __LINE__);

	if(0 != IniGetArrayValueU64(pFilepath, 
		CONFIG_IPCExtSENSOR_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_SNAPCHN,
		&tmp64))
	{
		printf("ConfigGetIPCExtSensorParam error-9\n");
		return -1;
	}
	para->psDispatch.nSnapChn = tmp64;
	//printf("line:%d\n", __LINE__);

	if(0 != IniGetArrayValueU64(pFilepath, 
		CONFIG_IPCExtSENSOR_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_ALARMOUT,
		&tmp64))
	{
		printf("ConfigGetIPCExtSensorParam error-10\n");
		return -1;
	}
	para->psDispatch.nAlarmOut = tmp64;
	//printf("line:%d\n", __LINE__);

	if(0 != IniGetArrayValueU64(pFilepath, 
		CONFIG_IPCExtSENSOR_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_RECCHN,
		&tmp64))
	{
		printf("ConfigGetIPCExtSensorParam error-11\n");
		return -1;
	}
	para->psDispatch.nRecordChn = tmp64;
	//printf("line:%d\n", __LINE__);
	
	for(i = 0; i < g_sConfigDvrProperty.nPreviewNum; i++)
	{
		if(0 != IniGetArrayValue(pFilepath, 
			CONFIG_IPCExtSENSOR_PARAM, 
			strtmp, 
			EM_CONFIG_SENSOR_PARAM_PTZTYPE + (3 * i),
			&tmp))
		{
			printf("ConfigGetIPCExtSensorParam error-12\n");
			return -1;
		}
		para->psDispatch.sAlarmPtz[i].nPtzType = tmp;

		if(0 != IniGetArrayValue(pFilepath, 
			CONFIG_IPCExtSENSOR_PARAM, 
			strtmp, 
			EM_CONFIG_SENSOR_PARAM_PTZCHN + (3 * i),
			&tmp))
		{
			printf("ConfigGetIPCExtSensorParam error-13\n");
			return -1;
		}
		para->psDispatch.sAlarmPtz[i].nChn = tmp;

		if(0 != IniGetArrayValue(pFilepath, 
			CONFIG_IPCExtSENSOR_PARAM, 
			strtmp, 
			EM_CONFIG_SENSOR_PARAM_PTZID + (3 * i),
			&tmp))
		{
			printf("ConfigGetIPCExtSensorParam error-14\n");
			return -1;
		}
		para->psDispatch.sAlarmPtz[i].nId = tmp;
	}
	
	return 0;
}

s32 ConfigGet485ExtSensorParam(SModConfigIPCExtSensorParam* para, s32 nId, s8* pFilepath)
{
	if((NULL == para) || (nId < 0) || (nId >= g_sConfigDvrProperty.nVidMainNum) || (NULL == pFilepath))
	{
		printf("Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}

	#ifdef CONFIG_TEST
	memset(para, 0, sizeof(SModConfigIPCExtSensorParam));
	para->nDelay = 5;
	para->nEnalbe = 1;
	//sprintf(para->name, "Sensor %2ld", nId);
	return 0;
	#endif
	
	s32 tmp = 0;
	u64 tmp64 = 0;
	s32 i = nId;
	s8 strtmp[30];
	memset(strtmp, 0, sizeof(strtmp));
	sprintf(strtmp, "%s%d", CONFIG_SENSOR_PARAM_SENSOR, i);
	//printf("ConfigGetSensorParam:%s\n", strtmp);
	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_485ExtSENSOR_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_ENABLE,
		&tmp))
	{
		printf("ConfigGet485ExtSensorParam error-1\n");
		return -1;
	}
	para->nEnalbe = tmp;
	//printf("line:%d\n", __LINE__);

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_485ExtSENSOR_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_DELAY,
		&tmp))
	{
		printf("ConfigGet485ExtSensorParam error-3\n");
		return -1;
	}
	para->nDelay = tmp;
	//printf("line:%d\n", __LINE__);

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_485ExtSENSOR_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_FLAG_BUZZ,
		&tmp))
	{
		printf("ConfigGet485ExtSensorParam error-6\n");
		return -1;
	}
	para->psDispatch.nFlagBuzz = tmp;
	//printf("line:%d\n", __LINE__);

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_485ExtSENSOR_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_ZOOMCHN,
		&tmp))
	{
		printf("ConfigGet485ExtSensorParam error-7\n");
		return -1;
	}
	para->psDispatch.nZoomChn = tmp;
	//printf("line:%d\n", __LINE__);

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_485ExtSENSOR_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_FLAG_MAIL,
		&tmp))
	{
		printf("ConfigGet485ExtSensorParam error-8\n");
		return -1;
	}
	para->psDispatch.nFlagEmail = tmp;
	//printf("line:%d\n", __LINE__);

	if(0 != IniGetArrayValueU64(pFilepath, 
		CONFIG_485ExtSENSOR_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_SNAPCHN,
		&tmp64))
	{
		printf("ConfigGet485ExtSensorParam error-9\n");
		return -1;
	}
	para->psDispatch.nSnapChn = tmp64;
	//printf("line:%d\n", __LINE__);

	if(0 != IniGetArrayValueU64(pFilepath, 
		CONFIG_485ExtSENSOR_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_ALARMOUT,
		&tmp64))
	{
		printf("ConfigGet485ExtSensorParam error-10\n");
		return -1;
	}
	para->psDispatch.nAlarmOut = tmp64;
	//printf("line:%d\n", __LINE__);

	if(0 != IniGetArrayValueU64(pFilepath, 
		CONFIG_485ExtSENSOR_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_RECCHN,
		&tmp64))
	{
		printf("ConfigGet485ExtSensorParam error-11\n");
		return -1;
	}
	para->psDispatch.nRecordChn = tmp64;
	//printf("line:%d\n", __LINE__);
	
	for(i = 0; i < g_sConfigDvrProperty.nPreviewNum; i++)
	{
		if(0 != IniGetArrayValue(pFilepath, 
			CONFIG_485ExtSENSOR_PARAM, 
			strtmp, 
			EM_CONFIG_SENSOR_PARAM_PTZTYPE + (3 * i),
			&tmp))
		{
			printf("ConfigGet485ExtSensorParam error-12\n");
			return -1;
		}
		para->psDispatch.sAlarmPtz[i].nPtzType = tmp;

		if(0 != IniGetArrayValue(pFilepath, 
			CONFIG_485ExtSENSOR_PARAM, 
			strtmp, 
			EM_CONFIG_SENSOR_PARAM_PTZCHN + (3 * i),
			&tmp))
		{
			printf("ConfigGet485ExtSensorParam error-13\n");
			return -1;
		}
		para->psDispatch.sAlarmPtz[i].nChn = tmp;

		if(0 != IniGetArrayValue(pFilepath, 
			CONFIG_485ExtSENSOR_PARAM, 
			strtmp, 
			EM_CONFIG_SENSOR_PARAM_PTZID + (3 * i),
			&tmp))
		{
			printf("ConfigGet485ExtSensorParam error-14\n");
			return -1;
		}
		para->psDispatch.sAlarmPtz[i].nId = tmp;
	}
	
	return 0;
}
s32 ConfigGetHDDParam(SModConfigHDDParam* para, s32 nId,s8* pFilepath)
{
	if((NULL == para)  || (NULL == pFilepath))
	{
		printf("Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}

	#ifdef CONFIG_TEST
	memset(para, 0, sizeof(SModConfigHDDParam));
	//para->nDelay = 5;
	para->nEnalbe = 1;
	//sprintf(para->name, "Sensor %2ld", nId);
	return 0;
	#endif
	
	s32 tmp = 0;
	u64 tmp64 = 0;
	s32 i = nId;
	s8 strtmp[30];
	memset(strtmp, 0, sizeof(strtmp));
	sprintf(strtmp, "%s", CONFIG_SENSOR_PARAM_SENSOR);
	//printf("ConfigGetSensorParam:%s\n", strtmp);
	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_HDD_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_ENABLE,
		&tmp))
	{
		printf("ConfigGetHDDParam error-1\n");
		return -1;
	}
	para->nEnalbe = tmp;
	//printf("line:%d\n", __LINE__);

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_HDD_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_FLAG_BUZZ,
		&tmp))
	{
		printf("ConfigGetHDDParam error-6\n");
		return -1;
	}
	para->psDispatch.nFlagBuzz = tmp;
	printf("yg ConfigGetHDDParam nFlagBuzz: %d\n", para->psDispatch.nFlagBuzz);
	//printf("line:%d\n", __LINE__);

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_HDD_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_ZOOMCHN,
		&tmp))
	{
		printf("ConfigGetHDDParam error-7\n");
		return -1;
	}
	para->psDispatch.nZoomChn = tmp;
	//printf("line:%d\n", __LINE__);

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_HDD_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_FLAG_MAIL,
		&tmp))
	{
		printf("ConfigGetHDDParam error-8\n");
		return -1;
	}
	para->psDispatch.nFlagEmail = tmp;
	//printf("line:%d\n", __LINE__);

	if(0 != IniGetArrayValueU64(pFilepath, 
		CONFIG_HDD_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_SNAPCHN,
		&tmp64))
	{
		printf("ConfigGetHDDParam error-9\n");
		return -1;
	}
	para->psDispatch.nSnapChn = tmp64;
	//printf("line:%d\n", __LINE__);

	if(0 != IniGetArrayValueU64(pFilepath, 
		CONFIG_HDD_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_ALARMOUT,
		&tmp64))
	{
		printf("ConfigGetHDDParam error-10\n");
		return -1;
	}
	para->psDispatch.nAlarmOut = tmp64;
	//printf("line:%d\n", __LINE__);

	if(0 != IniGetArrayValueU64(pFilepath, 
		CONFIG_HDD_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_RECCHN,
		&tmp64))
	{
		printf("ConfigGetHDDParam error-11\n");
		return -1;
	}
	para->psDispatch.nRecordChn = tmp64;
	//printf("line:%d\n", __LINE__);
	
	for(i = 0; i < g_sConfigDvrProperty.nPreviewNum; i++)
	{
		if(0 != IniGetArrayValue(pFilepath, 
			CONFIG_HDD_PARAM, 
			strtmp, 
			EM_CONFIG_SENSOR_PARAM_PTZTYPE + (3 * i),
			&tmp))
		{
			printf("ConfigGetHDDParam error-12\n");
			return -1;
		}
		para->psDispatch.sAlarmPtz[i].nPtzType = tmp;

		if(0 != IniGetArrayValue(pFilepath, 
			CONFIG_HDD_PARAM, 
			strtmp, 
			EM_CONFIG_SENSOR_PARAM_PTZCHN + (3 * i),
			&tmp))
		{
			printf("ConfigGetHDDParam error-13\n");
			return -1;
		}
		para->psDispatch.sAlarmPtz[i].nChn = tmp;

		if(0 != IniGetArrayValue(pFilepath, 
			CONFIG_HDD_PARAM, 
			strtmp, 
			EM_CONFIG_SENSOR_PARAM_PTZID + (3 * i),
			&tmp))
		{
			printf("ConfigGetHDDParam error-14\n");
			return -1;
		}
		para->psDispatch.sAlarmPtz[i].nId = tmp;
	}
	
	return 0;
}



s32 ConfigGetVMotionParam(SModConfigVMotionParam* para, s32 nId, s8* pFilepath)
{
	if((NULL == para) || (nId < 0) || (nId >= g_sConfigDvrProperty.nPreviewNum) || (NULL == pFilepath))
	{
		printf("Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}

	#ifdef CONFIG_TEST
	memset(para, 0, sizeof(SModConfigVMotionParam));
	para->nDelay = 5;
	para->nEnalbe = 1;
	para->nSensitivity = 6;
	memset(para->nBlockStatus, 0xff, sizeof(para->nBlockStatus));
	memset(&para->psDispatch.nAlarmOut, 0xff, sizeof(para->psDispatch.nAlarmOut));
	memset(&para->psDispatch.nRecordChn, 0xff, sizeof(para->psDispatch.nRecordChn));
	para->psDispatch.nFlagBuzz = 1;
	para->psDispatch.nFlagEmail = 1;
	para->psDispatch.nZoomChn = nId;
	return 0;
	#endif
	
	s32 tmp = 0;
	u64 tmp64 = 0;
	s32 i = nId;
	s8 strtmp[30];
	memset(strtmp, 0, sizeof(strtmp));
	sprintf(strtmp, "%s%d", CONFIG_VMOTION_PARAM_VMOTION, i);
	
	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_VMOTION_PARAM, 
		strtmp, 
		EM_CONFIG_VMOTION_PARAM_ENABLE,
		&tmp))
	{
		return -1;
	}
	para->nEnalbe = tmp;

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_VMOTION_PARAM, 
		strtmp, 
		EM_CONFIG_VMOTION_PARAM_SENSITIVITY,
		&tmp))
	{
		return -1;
	}
	para->nSensitivity = tmp;

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_VMOTION_PARAM, 
		strtmp, 
		EM_CONFIG_VMOTION_PARAM_DELAY,
		&tmp))
	{
		return -1;
	}
	para->nDelay = tmp;

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_VMOTION_PARAM, 
		strtmp, 
		EM_CONFIG_VMOTION_PARAM_SCHTYPE,
		&tmp))
	{
		return -1;
	}
	para->nSchType = tmp;

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_VMOTION_PARAM, 
		strtmp, 
		EM_CONFIG_VMOTION_PARAM_FLAG_BUZZ,
		&tmp))
	{
		return -1;
	}
	para->psDispatch.nFlagBuzz = tmp;

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_VMOTION_PARAM, 
		strtmp, 
		EM_CONFIG_VMOTION_PARAM_ZOOMCHN,
		&tmp))
	{
		return -1;
	}
	para->psDispatch.nZoomChn = tmp;

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_VMOTION_PARAM, 
		strtmp, 
		EM_CONFIG_VMOTION_PARAM_FLAG_MAIL,
		&tmp))
	{
		return -1;
	}
	para->psDispatch.nFlagEmail = tmp;

	if(0 != IniGetArrayValueU64(pFilepath, 
		CONFIG_VMOTION_PARAM, 
		strtmp, 
		EM_CONFIG_VMOTION_PARAM_SNAPCHN,
		&tmp64))
	{
		return -1;
	}
	para->psDispatch.nSnapChn = tmp64;

	if(0 != IniGetArrayValueU64(pFilepath, 
		CONFIG_VMOTION_PARAM, 
		strtmp, 
		EM_CONFIG_VMOTION_PARAM_ALARMOUT,
		&tmp64))
	{
		return -1;
	}
	para->psDispatch.nAlarmOut = tmp64;

	if(0 != IniGetArrayValueU64(pFilepath, 
		CONFIG_VMOTION_PARAM, 
		strtmp, 
		EM_CONFIG_VMOTION_PARAM_RECCHN,
		&tmp64))
	{
		return -1;
	}
	para->psDispatch.nRecordChn = tmp64;
	
	for(i = 0; i < g_sConfigDvrProperty.nPreviewNum; i++)
	{
		if(0 != IniGetArrayValue(pFilepath, 
			CONFIG_VMOTION_PARAM, 
			strtmp, 
			EM_CONFIG_VMOTION_PARAM_STATUS + g_sConfigDvrProperty.nMdAreaRows + (3 * i),//EM_CONFIG_VMOTION_PARAM_PTZTYPE + (3 * i),
			&tmp))
		{
			return -1;
		}
		para->psDispatch.sAlarmPtz[i].nPtzType = tmp;
		if(0 != IniGetArrayValue(pFilepath, 
			CONFIG_VMOTION_PARAM, 
			strtmp, 
			EM_CONFIG_VMOTION_PARAM_STATUS + g_sConfigDvrProperty.nMdAreaRows + (3 * i) + 1,//EM_CONFIG_VMOTION_PARAM_PTZCHN + (3 * i),
			&tmp))
		{
			return -1;
		}
		para->psDispatch.sAlarmPtz[i].nChn = tmp;
		if(0 != IniGetArrayValue(pFilepath, 
			CONFIG_VMOTION_PARAM, 
			strtmp, 
			EM_CONFIG_VMOTION_PARAM_STATUS + g_sConfigDvrProperty.nMdAreaRows + (3 * i) + 2,//EM_CONFIG_VMOTION_PARAM_PTZID + (3 * i),
			&tmp))
		{
			return -1;
		}
		para->psDispatch.sAlarmPtz[i].nId = tmp;
	}

	for(i = 0; i < g_sConfigDvrProperty.nMdAreaRows; i++)
	{
		if(0 != IniGetArrayValueU64(pFilepath, 
			CONFIG_VMOTION_PARAM, 
			strtmp, 
			EM_CONFIG_VMOTION_PARAM_STATUS  + i,
			//EM_CONFIG_VMOTION_PARAM_PTZTYPE + (3 * g_sConfigDvrProperty.nPreviewNum + i),
			&tmp64))
		{
			return -1;
		}
		para->nBlockStatus[i] = tmp64;
	}
	
	return 0;
}

s32 ConfigGetVLostParam(SModConfigVLostParam* para, s32 nId, s8* pFilepath)
{
	if((NULL == para) || (nId < 0) || (nId >= g_sConfigDvrProperty.nPreviewNum) || (NULL == pFilepath))
	{
		printf("Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}
	
	#ifdef CONFIG_TEST
	memset(para, 0, sizeof(SModConfigVLostParam));
	para->nDelay = 5;
	para->nEnalbe = 1;
	memset(&para->psDispatch.nAlarmOut, 0xff, sizeof(para->psDispatch.nAlarmOut));
	memset(&para->psDispatch.nRecordChn, 0xff, sizeof(para->psDispatch.nRecordChn));
	para->psDispatch.nFlagBuzz = 1;
	para->psDispatch.nFlagEmail = 1;
	para->psDispatch.nZoomChn = nId;
	return 0;
	#endif
	
	s32 tmp = 0;
	u64 tmp64 = 0;
	s32 i = nId;
	s8 strtmp[30];
	memset(strtmp, 0, sizeof(strtmp));
	sprintf(strtmp, "%s%d", CONFIG_VLOST_PARAM_VLOST, i);

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_VLOST_PARAM, 
		strtmp, 
		EM_CONFIG_VLOST_PARAM_ENABLE,
		&tmp))
	{
		return -1;
	}
	para->nEnalbe = tmp;

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_VLOST_PARAM, 
		strtmp, 
		EM_CONFIG_VLOST_PARAM_DELAY,
		&tmp))
	{
		return -1;
	}
	para->nDelay = tmp;

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_VLOST_PARAM, 
		strtmp, 
		EM_CONFIG_VLOST_PARAM_SCHTYPE,
		&tmp))
	{
		return -1;
	}
	para->nSchType = tmp;

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_VLOST_PARAM, 
		strtmp, 
		EM_CONFIG_VLOST_PARAM_FLAG_BUZZ,
		&tmp))
	{
		return -1;
	}
	para->psDispatch.nFlagBuzz = tmp;

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_VLOST_PARAM, 
		strtmp, 
		EM_CONFIG_VLOST_PARAM_ZOOMCHN,
		&tmp))
	{
		return -1;
	}
	para->psDispatch.nZoomChn = tmp;

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_VLOST_PARAM, 
		strtmp, 
		EM_CONFIG_VLOST_PARAM_FLAG_MAIL,
		&tmp))
	{
		return -1;
	}
	para->psDispatch.nFlagEmail = tmp;

	if(0 != IniGetArrayValueU64(pFilepath, 
		CONFIG_VLOST_PARAM, 
		strtmp, 
		EM_CONFIG_VLOST_PARAM_SNAPCHN,
		&tmp64))
	{
		return -1;
	}
	para->psDispatch.nSnapChn = tmp64;

	if(0 != IniGetArrayValueU64(pFilepath, 
		CONFIG_VLOST_PARAM, 
		strtmp, 
		EM_CONFIG_VLOST_PARAM_ALARMOUT,
		&tmp64))
	{
		return -1;
	}
	para->psDispatch.nAlarmOut = tmp64;

	if(0 != IniGetArrayValueU64(pFilepath, 
		CONFIG_VLOST_PARAM, 
		strtmp, 
		EM_CONFIG_VLOST_PARAM_RECCHN,
		&tmp64))
	{
		return -1;
	}
	para->psDispatch.nRecordChn = tmp64;

	for(i = 0; i < g_sConfigDvrProperty.nPreviewNum; i++)
	{
		if(0 != IniGetArrayValue(pFilepath, 
			CONFIG_VLOST_PARAM, 
			strtmp, 
			EM_CONFIG_VLOST_PARAM_PTZTYPE + (3 * i),
			&tmp))
		{
			return -1;
		}
		para->psDispatch.sAlarmPtz[i].nPtzType = tmp;

		if(0 != IniGetArrayValue(pFilepath, 
			CONFIG_VLOST_PARAM, 
			strtmp, 
			EM_CONFIG_VLOST_PARAM_PTZCHN + (3 * i),
			&tmp))
		{
			return -1;
		}
		para->psDispatch.sAlarmPtz[i].nChn = tmp;

		if(0 != IniGetArrayValue(pFilepath, 
			CONFIG_VLOST_PARAM, 
			strtmp, 
			EM_CONFIG_VLOST_PARAM_PTZID + (3 * i),
			&tmp))
		{
			return -1;
		}
		para->psDispatch.sAlarmPtz[i].nId = tmp;
	}

	return 0;
}

s32 ConfigGetVBlindParam(SModConfigVBlindParam* para, s32 nId, s8* pFilepath)
{
	if((NULL == para) || (nId < 0) || (nId >= g_sConfigDvrProperty.nPreviewNum) || (NULL == pFilepath))
	{
		printf("Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}

	#ifdef CONFIG_TEST
	memset(para, 0, sizeof(SModConfigVBlindParam));
	para->nDelay = 5;
	para->nEnalbe = 1;
	memset(&para->psDispatch.nAlarmOut, 0xff, sizeof(para->psDispatch.nAlarmOut));
	memset(&para->psDispatch.nRecordChn, 0xff, sizeof(para->psDispatch.nRecordChn));
	para->psDispatch.nFlagBuzz = 1;
	para->psDispatch.nFlagEmail = 1;
	para->psDispatch.nZoomChn = nId;
	return 0;
	#endif
	
	s32 tmp = 0;
	u64 tmp64 = 0;
	s32 i = nId;
	s8 strtmp[30];
	memset(strtmp, 0, sizeof(strtmp));
	sprintf(strtmp, "%s%d", CONFIG_VBLIND_PARAM_VBLIND, i);

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_VBLIND_PARAM, 
		strtmp, 
		EM_CONFIG_VBLIND_PARAM_ENABLE,
		&tmp))
	{
		return -1;
	}
	para->nEnalbe = tmp;

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_VBLIND_PARAM, 
		strtmp, 
		EM_CONFIG_VBLIND_PARAM_DELAY,
		&tmp))
	{
		return -1;
	}
	para->nDelay = tmp;

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_VBLIND_PARAM, 
		strtmp, 
		EM_CONFIG_VBLIND_PARAM_SCHTYPE,
		&tmp))
	{
		return -1;
	}
	para->nSchType = tmp;

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_VBLIND_PARAM, 
		strtmp, 
		EM_CONFIG_VBLIND_PARAM_FLAG_BUZZ,
		&tmp))
	{
		return -1;
	}
	para->psDispatch.nFlagBuzz = tmp;

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_VBLIND_PARAM, 
		strtmp, 
		EM_CONFIG_VBLIND_PARAM_ZOOMCHN,
		&tmp))
	{
		return -1;
	}
	para->psDispatch.nZoomChn = tmp;

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_VBLIND_PARAM, 
		strtmp, 
		EM_CONFIG_VBLIND_PARAM_FLAG_MAIL,
		&tmp))
	{
		return -1;
	}
	para->psDispatch.nFlagEmail = tmp;

	if(0 != IniGetArrayValueU64(pFilepath, 
		CONFIG_VBLIND_PARAM, 
		strtmp, 
		EM_CONFIG_VBLIND_PARAM_SNAPCHN,
		&tmp64))
	{
		return -1;
	}
	para->psDispatch.nSnapChn = tmp64;

	if(0 != IniGetArrayValueU64(pFilepath, 
		CONFIG_VBLIND_PARAM, 
		strtmp, 
		EM_CONFIG_VBLIND_PARAM_ALARMOUT,
		&tmp64))
	{
		return -1;
	}
	para->psDispatch.nAlarmOut = tmp64;

	if(0 != IniGetArrayValueU64(pFilepath, 
		CONFIG_VBLIND_PARAM, 
		strtmp, 
		EM_CONFIG_VBLIND_PARAM_RECCHN,
		&tmp64))
	{
		return -1;
	}
	para->psDispatch.nRecordChn = tmp64;

	for(i = 0; i < g_sConfigDvrProperty.nPreviewNum; i++)
	{
		if(0 != IniGetArrayValue(pFilepath, 
			CONFIG_VBLIND_PARAM, 
			strtmp, 
			EM_CONFIG_VBLIND_PARAM_PTZTYPE + (3 * i),
			&tmp))
		{
			return -1;
		}
		para->psDispatch.sAlarmPtz[i].nPtzType = tmp;

		if(0 != IniGetArrayValue(pFilepath, 
			CONFIG_VBLIND_PARAM, 
			strtmp, 
			EM_CONFIG_VBLIND_PARAM_PTZCHN + (3 * i),
			&tmp))
		{
			return -1;
		}
		para->psDispatch.sAlarmPtz[i].nChn = tmp;

		if(0 != IniGetArrayValue(pFilepath, 
			CONFIG_VBLIND_PARAM, 
			strtmp, 
			EM_CONFIG_VBLIND_PARAM_PTZID + (3 * i),
			&tmp))
		{
			return -1;
		}
		para->psDispatch.sAlarmPtz[i].nId = tmp;
	}

	return 0;
}

s32 ConfigGetAlarmOutParam(SModConfigAlarmOutParam* para, s32 nId, s8* pFilepath)
{
	if((NULL == para) || (nId < 0) || (nId >= g_sConfigDvrProperty.nAlarmoutNum) || (NULL == pFilepath))
	{
		printf("Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}

	#ifdef CONFIG_TEST
	memset(para, 0, sizeof(SModConfigAlarmOutParam));
	para->nDelay = 5;
	para->nEnalbe = 1;
	sprintf(para->name, "Alarmout %2ld", nId);
	return 0;
	#endif
	
	s32 tmp = 0;
	s32 i = nId;
	s8 strtmp[30];
	memset(strtmp, 0, sizeof(strtmp));
	sprintf(strtmp, "%s%d", CONFIG_ALARMOUT_PARAM_ALARMOUT, i);

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_ALARMOUT_PARAM, 
		strtmp, 
		EM_CONFIG_ALARMOUT_PARAM_ENABLE,
		&tmp))
	{
		return -1;
	}
	para->nEnalbe = tmp;

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_ALARMOUT_PARAM, 
		strtmp, 
		EM_CONFIG_ALARMOUT_PARAM_TYPE,
		&tmp))
	{
		return -1;
	}
	para->nAlarmOutType = tmp;

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_ALARMOUT_PARAM, 
		strtmp, 
		EM_CONFIG_ALARMOUT_PARAM_DELAY,
		&tmp))
	{
		return -1;
	}
	para->nDelay = tmp;

	if(0 != GetArrayStringFromIni(pFilepath, 
		CONFIG_ALARMOUT_PARAM, 
		strtmp, 
		EM_CONFIG_ALARMOUT_PARAM_NAME,
		para->name, 
		sizeof(para->name)))
	{
		return -1;
	}

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_ALARMOUT_PARAM, 
		strtmp, 
		EM_CONFIG_ALARMOUT_PARAM_SCHTYPE,
		&tmp))
	{
		return -1;
	}
	para->nSchType = tmp;

	return 0;
}

s32 ConfigGetBuzzParam(SModConfigBuzzParam* para, s32 nId, s8* pFilepath)
{
	if((NULL == para) || (nId < 0) || (nId >= g_sConfigDvrProperty.nBuzzNum) || (NULL == pFilepath))
	{
		printf("Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}

	#ifdef CONFIG_TEST
	memset(para, 0, sizeof(SModConfigBuzzParam));
	para->nDelay = 5;
	para->nEnalbe = 1;
	para->nInterval = 1;
	para->nDuration = 2;
	return 0;
	#endif
	
	s32 tmp = 0;
	s32 i = nId;
	s8 strtmp[30];
	memset(strtmp, 0, sizeof(strtmp));
	sprintf(strtmp, "%s%d", CONFIG_BUZZ_PARAM_BUZZ, i);

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_BUZZ_PARAM, 
		strtmp, 
		EM_CONFIG_BUZZ_PARAM_ENABLE,
		&tmp))
	{
		return -1;
	}
	para->nEnalbe = tmp;

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_BUZZ_PARAM, 
		strtmp, 
		EM_CONFIG_BUZZ_PARAM_DELAY,
		&tmp))
	{
		return -1;
	}
	para->nDelay = tmp;

	para->nDuration = 0;
	para->nInterval = 0;

	return 0;
}

s32 ConfigGetSensorSchParam(SModConfigSensorSch* para, s32 nId, s8* pFilepath)
{
	if((NULL == para) || (nId < 0) || (nId >= g_sConfigDvrProperty.nSensorNum) || (NULL == pFilepath))
	{
		printf("Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}

	#ifdef CONFIG_TEST
	{
		memset(para, 0, sizeof(SModConfigSensorSch));
		int i = 0;
		for (i = 0; i < 7; i++)
		{
			para->psSensorSch.nSchTime[i][0].nStartTime = 0;
			para->psSensorSch.nSchTime[i][0].nStartTime = 86400;
		}
	}
	return 0;
	#endif
	
	s32 tmp = 0;
	s32 i = nId;
	s32 j = 0;
	s8 strtmp[30];
	memset(strtmp, 0, sizeof(strtmp));
	sprintf(strtmp, "%s%d", CONFIG_SENSOR_PARAM_SENSOR, nId);
	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_SENSOR_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_SCHTYPE,
		&tmp))
	{
		return -1;
	}
	para->psSensorSch.nSchType = tmp;

	u8 daynum = 0;
	if(para->psSensorSch.nSchType == 0)
	{
		daynum = 7;
	}
	else if(para->psSensorSch.nSchType == 1)
	{
		daynum = 31;
	}
	else if(para->psSensorSch.nSchType == 2)
	{
		daynum = 1;
	}
	sprintf(strtmp, "%s%d", CONFIG_SCH_PARAM, nId);
	s8 strtmp2[30];
	memset(strtmp2, 0, sizeof(strtmp2));
	
	for(i = 0; i < daynum; i++)
	{		
		sprintf(strtmp2, "%s%d", CONFIG_SCH_PARAM_SENSOR, i);
		j = 0;
		while(1)
		{			
			if(0 != IniGetArrayValue(pFilepath, 
				strtmp, 
				strtmp2, 
				EM_CONFIG_SCH_PARAM_STARTTIME + (2 * j),
				&tmp))
			{
				break;
			}
			para->psSensorSch.nSchTime[i][j].nStartTime = tmp;

			if(0 != IniGetArrayValue(pFilepath, 
				strtmp, 
				strtmp2, 
				EM_CONFIG_SCH_PARAM_ENDTIME + (2 * j),
				&tmp))
			{
				break;
			}
			para->psSensorSch.nSchTime[i][j].nStopTime = tmp;

			j++;
			if((j >= g_sConfigDvrProperty.nSchSegments) || (j >= CONFIG_MAX_SCH_TIME_SEGMENTS))
			{
				break;
			}
		}
	}
	
	return 0;
}
s32 ConfigGetIPCExtSensorSchParam(SModConfigSensorSch* para, s32 nId, s8* pFilepath)
{
	if((NULL == para) || (nId < 0) || (nId >= g_sConfigDvrProperty.nVidMainNum) || (NULL == pFilepath))
	{
		printf("Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}

	#ifdef CONFIG_TEST
	{
		memset(para, 0, sizeof(SModConfigSensorSch));
		int i = 0;
		for (i = 0; i < 7; i++)
		{
			para->psSensorSch.nSchTime[i][0].nStartTime = 0;
			para->psSensorSch.nSchTime[i][0].nStartTime = 86400;
		}
	}
	return 0;
	#endif
	
	s32 tmp = 0;
	s32 i = nId;
	s32 j = 0;
	s8 strtmp[30];
	memset(strtmp, 0, sizeof(strtmp));
	sprintf(strtmp, "%s%d", CONFIG_SENSOR_PARAM_SENSOR, nId);
	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_IPCExtSENSOR_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_SCHTYPE,
		&tmp))
	{
		return -1;
	}
	para->psSensorSch.nSchType = tmp;

	u8 daynum = 0;
	if(para->psSensorSch.nSchType == 0)
	{
		daynum = 7;
	}
	else if(para->psSensorSch.nSchType == 1)
	{
		daynum = 31;
	}
	else if(para->psSensorSch.nSchType == 2)
	{
		daynum = 1;
	}
	sprintf(strtmp, "%s%d", CONFIG_SCH_PARAM, nId);
	s8 strtmp2[30];
	memset(strtmp2, 0, sizeof(strtmp2));
	
	for(i = 0; i < daynum; i++)
	{		
		sprintf(strtmp2, "%s%d", CONFIG_SCH_PARAM_IPCEXTSENSOR, i);
		j = 0;
		while(1)
		{			
			if(0 != IniGetArrayValue(pFilepath, 
				strtmp, 
				strtmp2, 
				EM_CONFIG_SCH_PARAM_STARTTIME + (2 * j),
				&tmp))
			{
				break;
			}
			para->psSensorSch.nSchTime[i][j].nStartTime = tmp;

			if(0 != IniGetArrayValue(pFilepath, 
				strtmp, 
				strtmp2, 
				EM_CONFIG_SCH_PARAM_ENDTIME + (2 * j),
				&tmp))
			{
				break;
			}
			para->psSensorSch.nSchTime[i][j].nStopTime = tmp;

			j++;
			if((j >= g_sConfigDvrProperty.nSchSegments) || (j >= CONFIG_MAX_SCH_TIME_SEGMENTS))
			{
				break;
			}
		}
	}
	
	return 0;
}


s32 ConfigGetVMotionSchParam(SModConfigVMotionSch* para, s32 nId, s8* pFilepath)
{
	if((NULL == para) || (nId < 0) || (nId >= g_sConfigDvrProperty.nPreviewNum) || (NULL == pFilepath))
	{
		printf("Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}

	#ifdef CONFIG_TEST
		{
	memset(para, 0, sizeof(SModConfigVMotionSch));
	int i = 0;
	for (i = 0; i < 7; i++)
	{
		para->psVMSch.nSchTime[i][0].nStartTime = 0;
		para->psVMSch.nSchTime[i][0].nStartTime = 86400;
	}
		}
	return 0;
	#endif
	
	s32 tmp = 0;
	s32 i = nId;
	s32 j = 0;
	s8 strtmp[30];
	memset(strtmp, 0, sizeof(strtmp));
	sprintf(strtmp, "%s%d", CONFIG_VMOTION_PARAM_VMOTION, nId);
	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_VMOTION_PARAM, 
		strtmp, 
		EM_CONFIG_VMOTION_PARAM_SCHTYPE,
		&tmp))
	{
		return -1;
	}
	para->psVMSch.nSchType = tmp;

	u8 daynum = 0;
	if(para->psVMSch.nSchType == 0)
	{
		daynum = 7;
	}
	else if(para->psVMSch.nSchType == 1)
	{
		daynum = 31;
	}
	else if(para->psVMSch.nSchType == 2)
	{
		daynum = 1;
	}
	sprintf(strtmp, "%s%d", CONFIG_SCH_PARAM, nId);
	s8 strtmp2[30];
	memset(strtmp2, 0, sizeof(strtmp2));
	
	for(i = 0; i < daynum; i++)
	{		
		sprintf(strtmp2, "%s%d", CONFIG_SCH_PARAM_VMOTION, i);
		j = 0;
		while(1)
		{			
			if(0 != IniGetArrayValue(pFilepath, 
				strtmp, 
				strtmp2, 
				EM_CONFIG_SCH_PARAM_STARTTIME + (2 * j),
				&tmp))
			{
				break;
			}
			para->psVMSch.nSchTime[i][j].nStartTime = tmp;

			if(0 != IniGetArrayValue(pFilepath, 
				strtmp, 
				strtmp2, 
				EM_CONFIG_SCH_PARAM_ENDTIME + (2 * j),
				&tmp))
			{
				break;
			}
			para->psVMSch.nSchTime[i][j].nStopTime = tmp;

			j++;
			if((j >= g_sConfigDvrProperty.nSchSegments) || (j >= CONFIG_MAX_SCH_TIME_SEGMENTS))
			{
				break;
			}
		}
	}
	
	return 0;
}

s32 ConfigGetAlarmOutSchParam(SModConfigAlarmOutSch* para, s32 nId, s8* pFilepath)
{
	if((NULL == para) || (nId < 0) || (nId >= g_sConfigDvrProperty.nAlarmoutNum) || (NULL == pFilepath))
	{
		printf("Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}


	#ifdef CONFIG_TEST
		{
	memset(para, 0, sizeof(SModConfigAlarmOutSch));
	int i = 0;
	for (i = 0; i < 7; i++)
	{
		para->psAlarmOutSch.nSchTime[i][0].nStartTime = 0;
		para->psAlarmOutSch.nSchTime[i][0].nStartTime = 86400;
	}
		}
	return 0;
	#endif
	
	s32 tmp = 0;
	s32 i = nId;
	s32 j = 0;
	s8 strtmp[30];
	memset(strtmp, 0, sizeof(strtmp));
	sprintf(strtmp, "%s%d", CONFIG_ALARMOUT_PARAM_ALARMOUT, nId);
	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_ALARMOUT_PARAM, 
		strtmp, 
		EM_CONFIG_ALARMOUT_PARAM_SCHTYPE,
		&tmp))
	{
		return -1;
	}
	para->psAlarmOutSch.nSchType = tmp;

	u8 daynum = 0;
	if(para->psAlarmOutSch.nSchType == 0)
	{
		daynum = 7;
	}
	else if(para->psAlarmOutSch.nSchType == 1)
	{
		daynum = 31;
	}
	else if(para->psAlarmOutSch.nSchType == 2)
	{
		daynum = 1;
	}
	sprintf(strtmp, "%s%d", CONFIG_SCH_PARAM, nId);
	s8 strtmp2[30];
	memset(strtmp2, 0, sizeof(strtmp2));
	
	for(i = 0; i < daynum; i++)
	{		
		sprintf(strtmp2, "%s%d", CONFIG_SCH_PARAM_ALARMOUT, i);
		j = 0;
		while(1)
		{			
			if(0 != IniGetArrayValue(pFilepath, 
				strtmp, 
				strtmp2, 
				EM_CONFIG_SCH_PARAM_STARTTIME + (2 * j),
				&tmp))
			{
				break;
			}
			para->psAlarmOutSch.nSchTime[i][j].nStartTime = tmp;

			if(0 != IniGetArrayValue(pFilepath, 
				strtmp, 
				strtmp2, 
				EM_CONFIG_SCH_PARAM_ENDTIME + (2 * j),
				&tmp))
			{
				break;
			}
			para->psAlarmOutSch.nSchTime[i][j].nStopTime = tmp;

			j++;
			if((j >= g_sConfigDvrProperty.nSchSegments) || (j >= CONFIG_MAX_SCH_TIME_SEGMENTS))
			{
				break;
			}
		}
	}
	
	return 0;
}

s32 ConfigGetPtzParam(SModConfigPtzParam* para, s32 nId, s8* pFilepath)
{
	if((NULL == para) || (nId < 0) || (nId >= g_sConfigDvrProperty.nPreviewNum) || (NULL == pFilepath))
	{
		printf("Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}
	
	s32 tmp = 0;
	u64 tmp64 = 0;
	s32 i = nId;
	s8 strtmp[30];
	memset(strtmp, 0, sizeof(strtmp));
	sprintf(strtmp, "%s%d", CONFIG_PTZ_PARAM_PTZCFG, i);

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_PTZ_PARAM, 
		strtmp, 
		EM_CONFIG_PTZ_PARAM_ENABLE,
		&tmp))
	{
		return -1;
	}
	para->nEnable = tmp;

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_PTZ_PARAM, 
		strtmp, 
		EM_CONFIG_PTZ_PARAM_CAMADDR,
		&tmp))
	{
		return -1;
	}
	para->nCamAddr = tmp;

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_PTZ_PARAM, 
		strtmp, 
		EM_CONFIG_PTZ_PARAM_DATABIT,
		&tmp))
	{
		return -1;
	}
	para->nDataBit = tmp;

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_PTZ_PARAM, 
		strtmp, 
		EM_CONFIG_PTZ_PARAM_STOPBIT,
		&tmp))
	{
		return -1;
	}
	para->nStopBit = tmp;

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_PTZ_PARAM, 
		strtmp, 
		EM_CONFIG_PTZ_PARAM_BAUDRATE,
		&tmp))
	{
		return -1;
	}
	para->nBaudRate = tmp;

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_PTZ_PARAM, 
		strtmp, 
		EM_CONFIG_PTZ_PARAM_CHECKTYPE,
		&tmp))
	{
		return -1;
	}
	para->nCheckType = tmp;

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_PTZ_PARAM, 
		strtmp, 
		EM_CONFIG_PTZ_PARAM_FLOWCTRLTYPE,
		&tmp))
	{
		return -1;
	}
	para->nFlowCtrlType = tmp;

	if(0 != GetArrayStringFromIni(pFilepath, 
		CONFIG_PTZ_PARAM, 
		strtmp, 
		EM_CONFIG_PTZ_PARAM_PROTOCOL,
		para->nProtocolName, 
		sizeof(para->nProtocolName)))
	{
		return -1;
	}

	if(0 != IniGetArrayValueU64(pFilepath, 
		CONFIG_PTZ_PARAM, 
		strtmp, 
		EM_CONFIG_PTZ_PARAM_PRESET,
		&tmp64))
	{
		return -1;
	}
	para->sTourPresetPara.nPresetId[0] = tmp64;

	if(0 != IniGetArrayValueU64(pFilepath, 
		CONFIG_PTZ_PARAM, 
		strtmp, 
		EM_CONFIG_PTZ_PARAM_PRESET2,
		&tmp64))
	{
		return -1;
	}
	para->sTourPresetPara.nPresetId[1] = tmp64;

	if(0 != IniGetArrayValueU64(pFilepath, 
		CONFIG_PTZ_PARAM, 
		strtmp, 
		EM_CONFIG_PTZ_PARAM_TOURPATH,
		&tmp64))
	{
		return -1;
	}
	para->sTourPresetPara.nTourId[0] = tmp64;

	if(0 != IniGetArrayValueU64(pFilepath, 
		CONFIG_PTZ_PARAM, 
		strtmp, 
		EM_CONFIG_PTZ_PARAM_TOURPATH2,
		&tmp64))
	{
		return -1;
	}
	para->sTourPresetPara.nTourId[1] = tmp64;

	s8 strtmp2[30];
	memset(strtmp, 0, sizeof(strtmp));
	memset(strtmp2, 0, sizeof(strtmp2));
	sprintf(strtmp, "%s%d", CONFIG_PTZ_TOUR_PARAM, nId);
	//for(i = 0; i < 2 * 64; i++)
	for(i = 0; i < 32; i++)
	{
		if((para->sTourPresetPara.nTourId[i/64] >> (i%64)) & 1)  //cw_tour
		{
			sprintf(strtmp2, "%s%d", CONFIG_PTZ_TOUR_PARAM_TOURPATH, i);
			int j = 0;
			while(1)
			{
				if(0 != IniGetArrayValue(pFilepath, 
					strtmp, 
					strtmp2, 
					EM_CONFIG_PTZ_TOUR_PARAM_PRESETPOS + (j * 3),
					&tmp))
				{
					break;
				}
				para->sTourPresetPara.nTourPresetId[i][j].nPresetId = tmp;
				//如果是一个无效的置点就跳过其它的加载，防止开机时等待时间过长
				if (0 == tmp) {
					j++;
					if(j >= 128)
					{
						break;
					}
					continue;
				}

				if(0 != IniGetArrayValue(pFilepath, 
					strtmp, 
					strtmp2, 
					EM_CONFIG_PTZ_TOUR_PARAM_DWELLTIME + (j * 3),
					&tmp))
				{
					break;
				}
				para->sTourPresetPara.nTourPresetId[i][j].nStayTime = tmp;

				if(0 != IniGetArrayValue(pFilepath, 
					strtmp, 
					strtmp2, 
					EM_CONFIG_PTZ_TOUR_PARAM_SPEED + (j * 3),
					&tmp))
				{
					break;
				}
				para->sTourPresetPara.nTourPresetId[i][j].nSpeed = tmp;

				#if 0
				if (para->sTourPresetPara.nTourPresetId[i][j].nPresetId>0)
				{
					printf("***j:[%d], pos:[%d], speed:[%d], time:[%d]\n", j,
						para->sTourPresetPara.nTourPresetId[i][j].nPresetId,
						para->sTourPresetPara.nTourPresetId[i][j].nSpeed,
						para->sTourPresetPara.nTourPresetId[i][j].nStayTime
					);
				}
				#endif

				j++;
				if(j >= 128)
				{
					break;
				}
			}
		}
	}

	return 0;
}

s32 ConfigGetUserParam(SModConfigUserPara* para, s8* pFilepath)
{
	if((NULL == para) || (NULL == pFilepath))
	{
		printf("Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}	
	
	u64 tmp64 = 0;
	
	if(0 != IniGetArrayValueU64(pFilepath, 
		CONFIG_USER_PARAM, 
		CONFIG_USER_PARAM_SETTED_USER, 
		0,
		&tmp64))
	{
		return -1;
	}
	para->nFlagUserSetted[0] = tmp64;
	
	if(0 != IniGetArrayValueU64(pFilepath, 
		CONFIG_USER_PARAM, 
		CONFIG_USER_PARAM_SETTED_USER, 
		1,
		&tmp64))
	{
		return -1;
	}
	para->nFlagUserSetted[1] = tmp64;
	//printf("nFlagUserSetted[0] = %08x, [1] = %08x\n", para->nFlagUserSetted[0],para->nFlagUserSetted[1]);
	
	if(0 != IniGetValueU64(pFilepath, 
		CONFIG_USER_PARAM, 
		CONFIG_USER_PARAM_SETTED_GROUP, 
		&tmp64))
	{
		return -1;
	}
	para->nFlagGroupSetted = tmp64;
	
	s32 tmp = 0;
	s32 i = 0;
	s8 strtmp[30];
	s32 j = 0;
	
	for(i = 0; i < MOD_CONFIG_MAX_USER_NUM; i++)
	{
		if((para->nFlagUserSetted[i/64] >> (i%64)) & 1)
		{			
			memset(strtmp, 0, sizeof(strtmp));
			sprintf(strtmp, "%s%d", CONFIG_USER_PARAM_USER, i);
			
			if(0 != GetArrayStringFromIni(pFilepath, 
				CONFIG_USER_PARAM, 
				strtmp,
				EM_CONFIG_USER_PARAM_USER_NAME, 
				para->sUserPara[i].UserName, 
				sizeof(para->sUserPara[i].UserName)))
			{
				return -1;
			}
			
			if(0 != GetArrayStringFromIni(pFilepath, 
				CONFIG_USER_PARAM, 
				strtmp,
				EM_CONFIG_USER_PARAM_USER_PASSWD, 
				para->sUserPara[i].Password, 
				sizeof(para->sUserPara[i].Password)))
			{
				return -1;
			}
			
			if(0 != GetArrayStringFromIni(pFilepath, 
				CONFIG_USER_PARAM, 
				strtmp,
				EM_CONFIG_USER_PARAM_USER_GROUPBELONG, 
				para->sUserPara[i].GroupBelong, 
				sizeof(para->sUserPara[i].GroupBelong)))
			{
				return -1;
			}
			
			if(0 != GetArrayStringFromIni(pFilepath, 
				CONFIG_USER_PARAM, 
				strtmp,
				EM_CONFIG_USER_PARAM_USER_DESCRIPT, 
				para->sUserPara[i].UserDescript, 
				sizeof(para->sUserPara[i].UserDescript)))
			{
				return -1;
			}
			
			if(0 != IniGetArrayValue(pFilepath, 
				CONFIG_USER_PARAM, 
				strtmp, 
				EM_CONFIG_USER_PARAM_USER_LOGINTWINCE,
				&tmp))
			{
				return -1;
			}
			para->sUserPara[i].emLoginTwince = tmp;
			
			if(0 != IniGetArrayValue(pFilepath, 
				CONFIG_USER_PARAM, 
				strtmp, 
				EM_CONFIG_USER_PARAM_USER_BINDPCMAC,
				&tmp))
			{
				return -1;
			}
			para->sUserPara[i].emIsBindPcMac = tmp;
			
			if(0 != IniGetArrayValueU64(pFilepath, 
				CONFIG_USER_PARAM, 
				strtmp, 
				EM_CONFIG_USER_PARAM_USER_MACADDR,
				&tmp64))
			{
				return -1;
			}
			para->sUserPara[i].PcMacAddress = tmp64;
			
			for(j = 0; j < sizeof(para->sUserPara[i].UserAuthor.nAuthor); j++)
			{
				if(0 != IniGetArrayValue(pFilepath, 
					CONFIG_USER_PARAM, 
					strtmp, 
					EM_CONFIG_USER_PARAM_USER_AUTHOR + j,
					&tmp))
				{
					return -1;
				}
				para->sUserPara[i].UserAuthor.nAuthor[j] = tmp;
			}
			
			u8 offset = sizeof(para->sUserPara[i].UserAuthor.nAuthor);
			
			for(j = 0; j < 4; j++)
			{
				if(0 != IniGetArrayValueU64(pFilepath, 
					CONFIG_USER_PARAM, 
					strtmp, 
					EM_CONFIG_USER_PARAM_USER_AUTHOR + offset + j,
					&tmp64))
				{
					return -1;
				}
				para->sUserPara[i].UserAuthor.nLiveView[j] = tmp64;
			}
			
			for(j = 0; j < 4; j++)
			{
				if(0 != IniGetArrayValueU64(pFilepath, 
					CONFIG_USER_PARAM, 
					strtmp, 
					EM_CONFIG_USER_PARAM_USER_AUTHOR + offset + j + 4,
					&tmp64))
				{
					return -1;
				}
				para->sUserPara[i].UserAuthor.nRemoteView[j] = tmp64;
				
				//csp modify 20130519
				if(strcasecmp(para->sUserPara[i].UserName, "admin") == 0)
				{
					para->sUserPara[i].UserAuthor.nRemoteView[j] = (u64)(-1);
				}
			}
			
			for(j = 0; j < 4; j++)
			{
				if(0 != IniGetArrayValueU64(pFilepath, 
					CONFIG_USER_PARAM, 
					strtmp, 
					EM_CONFIG_USER_PARAM_USER_AUTHOR + offset + j + 8,
					&tmp64))
				{
					return -1;
				}
				para->sUserPara[i].UserAuthor.nRecord[j] = tmp64;
			}
			
			for(j = 0; j < 4; j++)
			{
				if(0 != IniGetArrayValueU64(pFilepath, 
					CONFIG_USER_PARAM, 
					strtmp, 
					EM_CONFIG_USER_PARAM_USER_AUTHOR + offset + j + 12,
					&tmp64))
				{
					return -1;
				}
				para->sUserPara[i].UserAuthor.nPlayback[j] = tmp64;
			}
			
			for(j = 0; j < 4; j++)
			{
				if(0 != IniGetArrayValueU64(pFilepath, 
					CONFIG_USER_PARAM, 
					strtmp, 
					EM_CONFIG_USER_PARAM_USER_AUTHOR + offset + j + 16,
					&tmp64))
				{
					return -1;
				}
				para->sUserPara[i].UserAuthor.nBackUp[j] = tmp64;
			}
			
			for(j = 0; j < 4; j++)
			{
				if(0 != IniGetArrayValueU64(pFilepath, 
					CONFIG_USER_PARAM, 
					strtmp, 
					EM_CONFIG_USER_PARAM_USER_AUTHOR + offset + j + 20,
					&tmp64))
				{
					return -1;
				}
				para->sUserPara[i].UserAuthor.nPtzCtrl[j] = tmp64;
			}
		}
	}
	
	for(i = 0; i < MOD_CONFIG_MAX_GROUP_NUM; i++)
	{
		if((para->nFlagGroupSetted >> i) & 1)
		{			
			memset(strtmp, 0, sizeof(strtmp));
			sprintf(strtmp, "%s%d", CONFIG_USER_PARAM_GROUP, i);
			
			if(0 != GetArrayStringFromIni(pFilepath, 
				CONFIG_USER_PARAM, 
				strtmp,
				EM_CONFIG_USER_PARAM_GROUP_NAME, 
				para->sGroupPara[i].GroupName, 
				sizeof(para->sGroupPara[i].GroupName)))
			{
				return -1;
			}
			
			if(0 != GetArrayStringFromIni(pFilepath, 
				CONFIG_USER_PARAM, 
				strtmp,
				EM_CONFIG_USER_PARAM_GROUP_DESCRIPT, 
				para->sGroupPara[i].GroupDescript, 
				sizeof(para->sGroupPara[i].GroupDescript)))
			{
				return -1;
			}
			
			if(0 != IniGetArrayValue(pFilepath, 
				CONFIG_USER_PARAM, 
				strtmp, 
				EM_CONFIG_USER_PARAM_GROUP_DOWITHUSER,
				&tmp))
			{
				return -1;
			}
			para->sGroupPara[i].nDoWithUser = tmp;
			
			for(j = 0; j < sizeof(para->sGroupPara[i].GroupAuthor.nAuthor); j++)
			{
				if(0 != IniGetArrayValue(pFilepath, 
					CONFIG_USER_PARAM, 
					strtmp, 
					EM_CONFIG_USER_PARAM_GROUP_AUTHOR + j,
					&tmp))
				{
					return -1;
				}
				para->sGroupPara[i].GroupAuthor.nAuthor[j] = tmp;
			}
			
			for(j = 0; j < 4; j++)
			{
				if(0 != IniGetArrayValueU64(pFilepath, 
					CONFIG_USER_PARAM, 
					strtmp, 
					EM_CONFIG_USER_PARAM_GROUP_AUTHOR + sizeof(para->sGroupPara[i].GroupAuthor.nAuthor) + j,
					&tmp64))
				{
					return -1;
				}
				para->sGroupPara[i].GroupAuthor.nLiveView[j] = tmp64;
			}
			
			for(j = 0; j < 4; j++)
			{
				if(0 != IniGetArrayValueU64(pFilepath, 
					CONFIG_USER_PARAM, 
					strtmp, 
					EM_CONFIG_USER_PARAM_GROUP_AUTHOR + sizeof(para->sGroupPara[i].GroupAuthor.nAuthor) + j + 4,
					&tmp64))
				{
					return -1;
				}
				para->sGroupPara[i].GroupAuthor.nRemoteView[j] = tmp64;
				
				//csp modify 20130519
				if(strcasecmp(para->sGroupPara[i].GroupName, "Administrator") == 0)
				{
					para->sGroupPara[i].GroupAuthor.nRemoteView[j] = (u64)(-1);
				}
			}
			
			for(j = 0; j < 4; j++)
			{
				if(0 != IniGetArrayValueU64(pFilepath, 
					CONFIG_USER_PARAM, 
					strtmp, 
					EM_CONFIG_USER_PARAM_GROUP_AUTHOR + sizeof(para->sGroupPara[i].GroupAuthor.nAuthor) + j + 8,
					&tmp64))
				{
					return -1;
				}
				para->sGroupPara[i].GroupAuthor.nRecord[j] = tmp64;
			}
			
			for(j = 0; j < 4; j++)
			{
				if(0 != IniGetArrayValueU64(pFilepath, 
					CONFIG_USER_PARAM, 
					strtmp, 
					EM_CONFIG_USER_PARAM_GROUP_AUTHOR + sizeof(para->sGroupPara[i].GroupAuthor.nAuthor) + j + 12,
					&tmp64))
				{
					return -1;
				}
				para->sGroupPara[i].GroupAuthor.nPlayback[j] = tmp64;
			}
			
			for(j = 0; j < 4; j++)
			{
				if(0 != IniGetArrayValueU64(pFilepath, 
					CONFIG_USER_PARAM, 
					strtmp, 
					EM_CONFIG_USER_PARAM_GROUP_AUTHOR + sizeof(para->sGroupPara[i].GroupAuthor.nAuthor) + j + 16,
					&tmp64))
				{
					return -1;
				}
				para->sGroupPara[i].GroupAuthor.nBackUp[j] = tmp64;
			}
			
			for(j = 0; j < 4; j++)
			{
				if(0 != IniGetArrayValueU64(pFilepath, 
					CONFIG_USER_PARAM, 
					strtmp, 
					EM_CONFIG_USER_PARAM_GROUP_AUTHOR + sizeof(para->sGroupPara[i].GroupAuthor.nAuthor) + j + 20,
					&tmp64))
				{
					return -1;
				}
				para->sGroupPara[i].GroupAuthor.nPtzCtrl[j] = tmp64;
			}			
		}
	}
	
	return 0;
}

//
s32 ConfigSetMainTainParam(SModConfigMainTainParam* para, s8* pFilepath)
{
	if((NULL == para) || (NULL == pFilepath))
	{
		printf("Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}
	
	s32 tmp = para->nEnable;
	
	if(0 != PublicIniSetValue(pFilepath, 
		CONFIG_MAINTAIN_PARAM, 
		EM_CONFIG_MAINTAIN_PARAM_ENABLE, 
		tmp))
	{
		return -1;
	}

	tmp = para->nMainType;
	
	if(0 != PublicIniSetValue(pFilepath, 
		CONFIG_MAINTAIN_PARAM, 
		EM_CONFIG_MAINTAIN_PARAM_MAINTYPE, 
		tmp))
	{
		return -1;
	}
	
	tmp = para->nSubType;
	
	if(0 != PublicIniSetValue(pFilepath, 
		CONFIG_MAINTAIN_PARAM, 
		EM_CONFIG_MAINTAIN_PARAM_SUBTYPE, 
		tmp))
	{
		return -1;
	}
	
	tmp = para->ntime;
	
	if(0 != PublicIniSetValue(pFilepath, 
		CONFIG_MAINTAIN_PARAM, 
		EM_CONFIG_MAINTAIN_PARAM_TIME, 
		tmp))
	{
		return -1;
	}
	
	return 0;
}

s32 ConfigSetSystemParam(SModConfigSystemParam* para, s8* pFilepath)
{
	if((NULL == para) || (NULL == pFilepath))
	{
		printf("Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}
	
	#if 1//csp modify
	if(0 != SetStringToIni(pFilepath, 
		CONFIG_SYSTEM_PARAM, 
		CONFIG_SYSTEM_PARAM_SN, 
		para->sn))
	{
		return -1;
	}
	#endif
	
	s32 tmp = 0;
	if(0 != SetStringToIni(pFilepath, 
		CONFIG_SYSTEM_PARAM, 
		CONFIG_SYSTEM_PARAM_DEVNAME, 
		para->nDevName))
	{
		return -1;
	}

	tmp = para->nDevId;
	if(0 != PublicIniSetValue(pFilepath, 
		CONFIG_SYSTEM_PARAM, 
		CONFIG_SYSTEM_PARAM_DEVID, 
		tmp))
	{
		return -1;
	}

	/*
	tmp = para->nManual;
	if(0 != PublicIniSetValue(pFilepath, 
		CONFIG_SYSTEM_PARAM, 
		CONFIG_SYSTEM_PARAM_MANUAL, 
		tmp))
	{
		return -1;
	}
	*/
	
	tmp = para->nVideoStandard;
	if(0 != PublicIniSetValue(pFilepath, 
		CONFIG_SYSTEM_PARAM, 
		CONFIG_SYSTEM_PARAM_VIDEOSTAND, 
		tmp))
	{
		return -1;
	}
	
	tmp = para->nAuthCheck;
	if(0 != PublicIniSetValue(pFilepath, 
		CONFIG_SYSTEM_PARAM, 
		CONFIG_SYSTEM_PARAM_AUTHCHECK, 
		tmp))
	{
		return -1;
	}
	
	tmp = para->nOutput;
	if(0 != PublicIniSetValue(pFilepath, 
		CONFIG_SYSTEM_PARAM, 
		CONFIG_SYSTEM_PARAM_OUTPUT, 
		tmp))
	{
		return -1;
	}

	para->nLangId = 1;//csp modify 20150110
	
	tmp = para->nLangId;
	if(0 != PublicIniSetValue(pFilepath, 
			CONFIG_SYSTEM_PARAM, 
			CONFIG_SYSTEM_PARAM_LANGID, 
			tmp))
	{
		return -1;
	}

	tmp = para->nShowGuide;
	if(0 != PublicIniSetValue(pFilepath, 
		CONFIG_SYSTEM_PARAM, 
		CONFIG_SYSTEM_PARAM_SHOWGUIDE, 
		tmp))
	{
		return -1;
	}
	
	tmp = para->nLockTime;
	if(0 != PublicIniSetValue(pFilepath, 
			CONFIG_SYSTEM_PARAM, 
			CONFIG_SYSTEM_PARAM_LOCKTIME, 
			tmp))
	{
		return -1;
	}
	
	tmp = para->nDateFormat;
	if(0 != PublicIniSetValue(pFilepath, 
			CONFIG_SYSTEM_PARAM, 
			CONFIG_SYSTEM_PARAM_DATEFORMAT, 
			tmp))
	{
		return -1;
	}
	
	tmp = para->nTimeStandard;
	if(0 != PublicIniSetValue(pFilepath, 
			CONFIG_SYSTEM_PARAM, 
			CONFIG_SYSTEM_PARAM_TIMESTAND, 
			tmp))
	{
		return -1;
	}
	
	tmp = para->nCyclingRecord;
	if(0 != PublicIniSetValue(pFilepath, 
			CONFIG_SYSTEM_PARAM, 
			CONFIG_SYSTEM_PARAM_CIRCLEREC, 
			tmp))
	{
		return -1;
	}
	
	tmp = para->nShowFirstMenue;//0;
	if(0 != PublicIniSetValue(pFilepath, 
			CONFIG_SYSTEM_PARAM, 
			CONFIG_SYSTEM_PARAM_SHOWFIRSTMENUE, 
			tmp))
	{
		return -1;
	}
	
	tmp = para->nSyscCheck;
	if(0 != PublicIniSetValue(pFilepath, 
		CONFIG_SYSTEM_PARAM, 
		CONFIG_SYSTEM_PARAM_SyscCheck, 
		tmp))
	{
		return -1;
	}//QYJ
	
	if(0 != SetStringToIni(pFilepath, 
		CONFIG_SYSTEM_PARAM, 
		CONFIG_SYSTEM_PARAM_NtpServerAdress, 
		para->nNtpServerAdress))
	{
		return -1;
	}//QYJ
	
	tmp = para->nTimeZone;
	if(0 != PublicIniSetValue(pFilepath, 
		CONFIG_SYSTEM_PARAM, 
		CONFIG_SYSTEM_PARAM_TimeZone, 
		tmp))
	{
		return -1;
	}//QYJ
	
	tmp = para->nSyncTimeToIPC;
	if(0 != PublicIniSetValue(pFilepath, 
		CONFIG_SYSTEM_PARAM, 
		CONFIG_SYSTEM_PARAM_SyncTimeToIPC, 
		tmp))
	{
		return -1;
	}
	
	tmp = para->P2PEnable;
	if(0 != PublicIniSetValue(pFilepath, 
		CONFIG_SYSTEM_PARAM, 
		CONFIG_SYSTEM_PARAM_P2PEnable, 
		tmp))
	{
		return -1;
	}
	
	return 0;
}

s32 ConfigSetPreviewParam(SModConfigPreviewParam* para, s8* pFilepath)
{
	if((NULL == para) || (NULL == pFilepath))
	{
		printf("Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}
	
	s32 tmp = 0;
	s32 i = 0;

	tmp = para->nPreviewMode;
	if(0 != PublicIniSetValue(pFilepath, 
		CONFIG_PREVIEW_PARAM, 
		CONFIG_PREVIEW_PARAM_MODE, 
		tmp))
	{
		return -1;
	}
	
	tmp = para->nModePara;
	if(0 != PublicIniSetValue(pFilepath, 
		CONFIG_PREVIEW_PARAM, 
		CONFIG_PREVIEW_PARAM_MODEPARA, 
		tmp))
	{
		return -1;
	}
	
	tmp = para->nAdioChn;
	if(0 != PublicIniSetValue(pFilepath, 
		CONFIG_PREVIEW_PARAM, 
		CONFIG_PREVIEW_PARAM_AUDIOOUTCHN, 
		tmp))
	{
		return -1;
	}
	
	tmp = para->nVolume;
	if(0 != PublicIniSetValue(pFilepath, 
		CONFIG_PREVIEW_PARAM, 
		CONFIG_PREVIEW_PARAM_VOLUME, 
		tmp))
	{
		return -1;
	}
	
	tmp = para->nMute;
	if(0 != PublicIniSetValue(pFilepath, 
		CONFIG_PREVIEW_PARAM, 
		CONFIG_PREVIEW_PARAM_MUTE, 
		tmp))
	{
		return -1;
	}
	
	tmp = para->nIsPatrol;
	if(0 != PublicIniSetValue(pFilepath, 
		CONFIG_PREVIEW_PARAM, 
		CONFIG_PREVIEW_PARAM_ISPATROL, 
		tmp))
	{
		return -1;
	}
	
	tmp = para->nInterval;
	if(0 != PublicIniSetValue(pFilepath, 
		CONFIG_PREVIEW_PARAM, 
		CONFIG_PREVIEW_PARAM_INTERVAL, 
		tmp))
	{
		return -1;
	}
	
	tmp = para->nPatrolMode;
	if(0 != PublicIniSetValue(pFilepath, 
		CONFIG_PREVIEW_PARAM, 
		CONFIG_PREVIEW_PARAM_PATROLMODE, 
		tmp))
	{
		return -1;
	}
	
	tmp = para->nStops;
	if(0 != PublicIniSetValue(pFilepath, 
		CONFIG_PREVIEW_PARAM, 
		CONFIG_PREVIEW_PARAM_STOPS, 
		tmp))
	{
		return -1;
	}
	
	for(i = 0; i < para->nStops; i++)
	{
		tmp = para->pnStopModePara[i];
		if(0 != PublicIniSetArrayValue(pFilepath, 
			CONFIG_PREVIEW_PARAM,  
			CONFIG_PREVIEW_PARAM_STOPMODEPARA,
			i,
			tmp))
		{
			return -1;
		}	
	}

	for(i = 0; i < g_sConfigDvrProperty.nPreviewNum; i++)
	{
		tmp = para->nVideoSrcType[i];
		if(0 != PublicIniSetArrayValue(pFilepath, 
			CONFIG_PREVIEW_PARAM,  
			CONFIG_PREVIEW_PARAM_VIDEOSRCTYPEPARA,
			i,
			tmp))
		{
			return -1;
		}
	}

	return 0;
}

s32 ConfigSetImageParam(SModConfigImagePara* para, s32 nId, s8* pFilepath)
{
	if((para == NULL) || (nId < 0) || (nId >= g_sConfigDvrProperty.nPreviewNum) || (NULL == pFilepath))
	{
		printf("Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}
	
	s32 tmp = 0;
	s32 i = nId;
	s8 strtmp[30];
	memset(strtmp, 0, sizeof(strtmp));
	sprintf(strtmp, "%s%d", CONFIG_PREVIEW_PARAM_PREVIEWCOLOR, i);
	tmp = para->nHue;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_PREVIEW_PARAM,  
		strtmp,
		EM_CONFIG_PREVIEW_PARAM_PREVIEWCOLOR_HUE,
		tmp))
	{
		return -1;
	}
	
	tmp = para->nSaturation;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_PREVIEW_PARAM,  
		strtmp,
		EM_CONFIG_PREVIEW_PARAM_PREVIEWCOLOR_SATURATION,
		tmp))
	{
		return -1;
	}
	
	tmp = para->nContrast;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_PREVIEW_PARAM,  
		strtmp,
		EM_CONFIG_PREVIEW_PARAM_PREVIEWCOLOR_CONTRAST,
		tmp))
	{
		return -1;
	}
	
	tmp = para->nBrightness;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_PREVIEW_PARAM,  
		strtmp,
		EM_CONFIG_PREVIEW_PARAM_PREVIEWCOLOR_BRIGHTNESS,
		tmp))
	{
		return -1;
	}
	
	return 0;
}

s32 ConfigSetVoImageParam(SModConfigImagePara* para, s32 nId, s8* pFilepath)
{
	if((para == NULL) || (nId < 0) || (nId >= g_sConfigDvrProperty.nPreviewNum) || (NULL == pFilepath))
	{
		printf("Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}
	
	s32 tmp = 0;
	s32 i = nId;
	s8 strtmp[30];
	memset(strtmp, 0, sizeof(strtmp));
	sprintf(strtmp, "%s%d", CONFIG_PREVIEW_PARAM_PLAYBACKCOLOR, i);

	tmp = para->nHue;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_PREVIEW_PARAM,  
		strtmp,
		EM_CONFIG_PREVIEW_PARAM_PREVIEWCOLOR_VO_HUE,
		tmp))
	{
		return -1;
	}
	
	tmp = para->nSaturation;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_PREVIEW_PARAM,  
		strtmp,
		EM_CONFIG_PREVIEW_PARAM_PREVIEWCOLOR_VO_SATURATION,
		tmp))
	{
		return -1;
	}
	
	tmp = para->nContrast;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_PREVIEW_PARAM,  
		strtmp,
		EM_CONFIG_PREVIEW_PARAM_PREVIEWCOLOR_VO_CONTRAST,
		tmp))
	{
		return -1;
	}
	
	tmp = para->nBrightness;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_PREVIEW_PARAM,  
		strtmp,
		EM_CONFIG_PREVIEW_PARAM_PREVIEWCOLOR_VO_BRIGHTNESS,
		tmp))
	{
		return -1;
	}
	
	return 0;
}

s32 ConfigSetMainOsdParam(SModConfigMainOsdParam* para, s8* pFilepath)
{
	if((NULL == para) || (NULL == pFilepath))
	{
		printf("Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}
	
	s32 tmp = 0;

	tmp = para->nShowTime;
	if(0 != PublicIniSetValue(pFilepath, 
		CONFIG_OSD_PARAM, 
		CONFIG_OSD_PARAM_SHOWTIME, 
		tmp))
	{
		return -1;
	}
	
	tmp = para->nShowChnKbps;
	if(0 != PublicIniSetValue(pFilepath, 
		CONFIG_OSD_PARAM, 
		CONFIG_OSD_PARAM_SHOWCHNKBPS, 
		tmp))
	{
		return -1;
	}
	
	tmp = para->nTimePosX;
	if(0 != PublicIniSetValue(pFilepath, 
		CONFIG_OSD_PARAM, 
		CONFIG_OSD_PARAM_TIME_X, 
		tmp))
	{
		return -1;
	}
	
	tmp = para->nTimePosY;
	if(0 != PublicIniSetValue(pFilepath, 
		CONFIG_OSD_PARAM, 
		CONFIG_OSD_PARAM_TIME_Y, 
		tmp))
	{
		return -1;
	}
	
	tmp = para->nShowRecState;
	if(0 != PublicIniSetValue(pFilepath, 
		CONFIG_OSD_PARAM, 
		CONFIG_OSD_PARAM_SHOWRECSTATE, 
		tmp))
	{
		return -1;
	}
	
	return 0;
}

s32 ConfigSetStrOsdParam(SModConfigStrOsdpara* para, s32 nId, s8* pFilepath)
{
	if((NULL == para) || (nId < 0) || (nId >= g_sConfigDvrProperty.nPreviewNum) || (NULL == pFilepath))
	{
		printf("Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}
	
	s32 tmp = 0;
	s32 i = nId;
	s8 strtmp[30];
	memset(strtmp, 0, sizeof(strtmp));
	sprintf(strtmp, "%s%d", CONFIG_OSD_PARAM_STR_OSD, i);
/*
	printf("ConfigSetStrOsdParam: \n");
	int j;
	for(j=0; j<6;j++)
		printf("0x%x\n", para->nChnName[j]);
*/
	if(0 != SetArrayStringToIni(pFilepath, 
		CONFIG_OSD_PARAM, 
		strtmp, 
		EM_CONFIG_OSD_PARAM_STR_OSD_CHNNAME,
		para->nChnName))
	{
		return -1;
	}

	tmp = para->nShowChnName;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_OSD_PARAM, 
		strtmp, 
		EM_CONFIG_OSD_PARAM_STR_OSD_SHOW_CHNNAME,
		tmp))
	{
		return -1;
	}
	
	tmp = para->nChnNamePosX;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_OSD_PARAM, 
		strtmp, 
		EM_CONFIG_OSD_PARAM_STR_OSD_CHNNAME_X,
		tmp))
	{
		return -1;
	}
	
	tmp = para->nChnNamePosY;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_OSD_PARAM, 
		strtmp, 
		EM_CONFIG_OSD_PARAM_STR_OSD_CHNNAME_Y,
		tmp))
	{
		return -1;
	}
	
	tmp = para->nEncShowChnName;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_OSD_PARAM, 
		strtmp, 
		EM_CONFIG_OSD_PARAM_STR_OSD_ENC_SHOW_CHNNAME,
		tmp))
	{
		return -1;
	}
	
	tmp = para->nEncChnNamePosX;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_OSD_PARAM, 
		strtmp, 
		EM_CONFIG_OSD_PARAM_STR_OSD_ENC_CHNNAME_X,
		tmp))
	{
		return -1;
	}
	
	tmp = para->nEncChnNamePosY;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_OSD_PARAM, 
		strtmp, 
		EM_CONFIG_OSD_PARAM_STR_OSD_ENC_CHNNAME_Y,
		tmp))
	{
		return -1;
	}
	
	tmp = para->nEncShowTime;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_OSD_PARAM, 
		strtmp, 
		EM_CONFIG_OSD_PARAM_STR_OSD_ENC_SHOWTIME,
		tmp))
	{
		return -1;
	}
	
	tmp = para->nEncTimePosX;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_OSD_PARAM, 
		strtmp, 
		EM_CONFIG_OSD_PARAM_STR_OSD_ENC_TIME_X,
		tmp))
	{
		return -1;
	}
	
	tmp = para->nEncTimePosY;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_OSD_PARAM, 
		strtmp, 
		EM_CONFIG_OSD_PARAM_STR_OSD_ENC_TIME_Y,
		tmp))
	{
		return -1;
	}
	
	return 0;
}

s32 ConfigSetRectOsdParam(SModConfigRectOsdpara* para, s32 nId, s8* pFilepath)
{
	if((NULL == para) || (nId < 0) || (nId >= g_sConfigDvrProperty.nPreviewNum) || (NULL == pFilepath))
	{
		printf("Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}
	
	s32 tmp = 0;
	s32 i = nId;
	s8 strtmp[30];
	memset(strtmp, 0, sizeof(strtmp));
	sprintf(strtmp, "%s%d", CONFIG_OSD_PARAM_RECT_OSD, i);

	tmp = para->nShowRectOsd;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_OSD_PARAM, 
		strtmp, 
		EM_CONFIG_OSD_PARAM_RECT_OSD_SHOW,
		tmp))
	{
		return -1;
	}
	
	tmp = para->nRectOsdNum;
	tmp = ((tmp < g_sConfigDvrProperty.nRectOsdNum) ? tmp : g_sConfigDvrProperty.nRectOsdNum);
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_OSD_PARAM, 
		strtmp, 
		EM_CONFIG_OSD_PARAM_RECT_OSD_NUM,
		tmp))
	{
		return -1;
	}

	int j = tmp;
	for(i = 0; i < j; i++)
	{
		tmp = para->nRectOsdX[i];
		if(0 != PublicIniSetArrayValue(pFilepath, 
			CONFIG_OSD_PARAM, 
			strtmp, 
			EM_CONFIG_OSD_PARAM_RECT_OSD_X + (4 * i),
			tmp))
		{
			return -1;
		}
		
		tmp = para->nRectOsdY[i];
		if(0 != PublicIniSetArrayValue(pFilepath, 
			CONFIG_OSD_PARAM, 
			strtmp, 
			EM_CONFIG_OSD_PARAM_RECT_OSD_Y + (4 * i),
			tmp))
		{
			return -1;
		}
		
		tmp = para->nRectOsdW[i];
		if(0 != PublicIniSetArrayValue(pFilepath, 
			CONFIG_OSD_PARAM, 
			strtmp, 
			EM_CONFIG_OSD_PARAM_RECT_OSD_W + (4 * i),
			tmp))
		{
			return -1;
		}
		
		tmp = para->nRectOsdH[i];
		if(0 != PublicIniSetArrayValue(pFilepath, 
			CONFIG_OSD_PARAM, 
			strtmp, 
			EM_CONFIG_OSD_PARAM_RECT_OSD_H + (4 * i),
			tmp))
		{
			return -1;
		}
	}
	
	return 0;
}

s32 ConfigSetMainEncodeParam(SModConfigVideoParam* para, s32 nId, s8* pFilepath)
{
	if((NULL == para) || (nId < 0) || (nId >= g_sConfigDvrProperty.nVidMainNum) || (NULL == pFilepath))
	{
		printf("Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}
	
	s32 tmp = 0;
	s32 i = nId;
	s8 strtmp[30];
	memset(strtmp, 0, sizeof(strtmp));
	sprintf(strtmp, "%s%d", CONFIG_ENCODE_PARAM_MAIN, i);

	tmp = para->nBitRate;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_ENCODE_PARAM, 
		strtmp, 
		EM_CONFIG_ENCODE_PARAM_BITRATE,
		tmp))
	{
		return -1;
	}
	
	tmp = para->nFrameRate;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_ENCODE_PARAM, 
		strtmp, 
		EM_CONFIG_ENCODE_PARAM_FRAMERATE,
		tmp))
	{
		return -1;
	}
	
	tmp = para->nGop;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_ENCODE_PARAM, 
		strtmp, 
		EM_CONFIG_ENCODE_PARAM_GOP,
		tmp))
	{
		return -1;
	}
	
	tmp = para->nMinQP;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_ENCODE_PARAM, 
		strtmp, 
		EM_CONFIG_ENCODE_PARAM_MINQP,
		tmp))
	{
		return -1;
	}
	
	tmp = para->nMaxQP;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_ENCODE_PARAM, 
		strtmp, 
		EM_CONFIG_ENCODE_PARAM_MAXQP,
		tmp))
	{
		return -1;
	}
	
	tmp = para->nResolution;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_ENCODE_PARAM, 
		strtmp, 
		EM_CONFIG_ENCODE_PARAM_RESULUTION,
		tmp))
	{
		return -1;
	}
	
	tmp = para->nEncodeType;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_ENCODE_PARAM, 
		strtmp, 
		EM_CONFIG_ENCODE_PARAM_ENCODETYPE,
		tmp))
	{
		return -1;
	}
	
	tmp = para->nBitRateType;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_ENCODE_PARAM, 
		strtmp, 
		EM_CONFIG_ENCODE_PARAM_BITERATETYPE,
		tmp))
	{
		return -1;
	}
	
	tmp = para->nPicLevel;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_ENCODE_PARAM, 
		strtmp, 
		EM_CONFIG_ENCODE_PARAM_PICLEVEL,
		tmp))
	{
		return -1;
	}
	
	return 0;
}
//yaogang modify 20150402 mobile stream
s32 ConfigSetMobEncodeParam(SModConfigVideoParam* para, s32 nId, s8* pFilepath)
{
	if((NULL == para) || (nId < 0) || (nId >= g_sConfigDvrProperty.nVidSubNum) || (NULL == pFilepath))
	{
		printf("Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}
	
	s32 tmp = 0;
	s32 i = nId;
	s8 strtmp[30];
	memset(strtmp, 0, sizeof(strtmp));
	sprintf(strtmp, "%s%d", CONFIG_ENCODE_PARAM_MOB, i);

	tmp = para->nBitRate;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_ENCODE_PARAM, 
		strtmp, 
		EM_CONFIG_ENCODE_PARAM_BITRATE,
		tmp))
	{
		return -1;
	}
	
	tmp = para->nFrameRate;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_ENCODE_PARAM, 
		strtmp, 
		EM_CONFIG_ENCODE_PARAM_FRAMERATE,
		tmp))
	{
		return -1;
	}
	
	tmp = para->nGop;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_ENCODE_PARAM, 
		strtmp, 
		EM_CONFIG_ENCODE_PARAM_GOP,
		tmp))
	{
		return -1;
	}
	
	tmp = para->nMinQP;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_ENCODE_PARAM, 
		strtmp, 
		EM_CONFIG_ENCODE_PARAM_MINQP,
		tmp))
	{
		return -1;
	}
	
	tmp = para->nMaxQP;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_ENCODE_PARAM, 
		strtmp, 
		EM_CONFIG_ENCODE_PARAM_MAXQP,
		tmp))
	{
		return -1;
	}
	
	tmp = para->nResolution;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_ENCODE_PARAM, 
		strtmp, 
		EM_CONFIG_ENCODE_PARAM_RESULUTION,
		tmp))
	{
		return -1;
	}
	
	tmp = para->nEncodeType;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_ENCODE_PARAM, 
		strtmp, 
		EM_CONFIG_ENCODE_PARAM_ENCODETYPE,
		tmp))
	{
		return -1;
	}
	
	tmp = para->nBitRateType;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_ENCODE_PARAM, 
		strtmp, 
		EM_CONFIG_ENCODE_PARAM_BITERATETYPE,
		tmp))
	{
		return -1;
	}
	
	tmp = para->nPicLevel;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_ENCODE_PARAM, 
		strtmp, 
		EM_CONFIG_ENCODE_PARAM_PICLEVEL,
		tmp))
	{
		return -1;
	}
	
	return 0;
}

s32 ConfigSetSubEncodeParam(SModConfigVideoParam* para, s32 nId, s8* pFilepath)
{
	if((NULL == para) || (nId < 0) || (nId >= g_sConfigDvrProperty.nVidSubNum) || (NULL == pFilepath))
	{
		printf("Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}
	
	s32 tmp = 0;
	s32 i = nId;
	s8 strtmp[30];
	memset(strtmp, 0, sizeof(strtmp));
	sprintf(strtmp, "%s%d", CONFIG_ENCODE_PARAM_SUB, i);

	tmp = para->nBitRate;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_ENCODE_PARAM, 
		strtmp, 
		EM_CONFIG_ENCODE_PARAM_BITRATE,
		tmp))
	{
		return -1;
	}
	
	tmp = para->nFrameRate;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_ENCODE_PARAM, 
		strtmp, 
		EM_CONFIG_ENCODE_PARAM_FRAMERATE,
		tmp))
	{
		return -1;
	}
	
	tmp = para->nGop;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_ENCODE_PARAM, 
		strtmp, 
		EM_CONFIG_ENCODE_PARAM_GOP,
		tmp))
	{
		return -1;
	}
	
	tmp = para->nMinQP;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_ENCODE_PARAM, 
		strtmp, 
		EM_CONFIG_ENCODE_PARAM_MINQP,
		tmp))
	{
		return -1;
	}
	
	tmp = para->nMaxQP;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_ENCODE_PARAM, 
		strtmp, 
		EM_CONFIG_ENCODE_PARAM_MAXQP,
		tmp))
	{
		return -1;
	}
	
	tmp = para->nResolution;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_ENCODE_PARAM, 
		strtmp, 
		EM_CONFIG_ENCODE_PARAM_RESULUTION,
		tmp))
	{
		return -1;
	}
	
	tmp = para->nEncodeType;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_ENCODE_PARAM, 
		strtmp, 
		EM_CONFIG_ENCODE_PARAM_ENCODETYPE,
		tmp))
	{
		return -1;
	}
	
	tmp = para->nBitRateType;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_ENCODE_PARAM, 
		strtmp, 
		EM_CONFIG_ENCODE_PARAM_BITERATETYPE,
		tmp))
	{
		return -1;
	}
	
	tmp = para->nPicLevel;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_ENCODE_PARAM, 
		strtmp, 
		EM_CONFIG_ENCODE_PARAM_PICLEVEL,
		tmp))
	{
		return -1;
	}
	
	return 0;
}

s32 ConfigSetRecordParam(SModConfigRecordParam* para, s32 nId, s8* pFilepath)
{
	if((NULL == para) || (nId < 0) || (nId >= g_sConfigDvrProperty.nRecNum) || (NULL == pFilepath))
	{
		printf("Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}
	
	s32 tmp = 0;
	s32 i = nId;
	s8 strtmp[30];
	memset(strtmp, 0, sizeof(strtmp));
	sprintf(strtmp, "%s%d", CONFIG_RECORD_PARAM_REC, i);

	tmp = para->bRecording;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_RECORD_PARAM, 
		strtmp, 
		EM_CONFIG_RECORD_PARAM_REC_RECORDING,
		tmp))
	{
		return -1;
	}
	
	tmp = para->nEnable;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_RECORD_PARAM, 
		strtmp, 
		EM_CONFIG_RECORD_PARAM_REC_ENABLE,
		tmp))
	{
		return -1;
	}
	
	tmp = para->nStreamType;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_RECORD_PARAM, 
		strtmp, 
		EM_CONFIG_RECORD_PARAM_REC_STREAMTYPE,
		tmp))
	{
		return -1;
	}
	
	tmp = para->nEncChn;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_RECORD_PARAM, 
		strtmp, 
		EM_CONFIG_RECORD_PARAM_REC_ENCCHN,
		tmp))
	{
		return -1;
	}
	
	tmp = para->nChnEncType;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_RECORD_PARAM, 
		strtmp, 
		EM_CONFIG_RECORD_PARAM_REC_ENCCHN_TYPE,
		tmp))
	{
		return -1;
	}
	
	tmp = para->nPreTime;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_RECORD_PARAM, 
		strtmp, 
		EM_CONFIG_RECORD_PARAM_REC_PRETIME,
		tmp))
	{
		return -1;
	}
	
	tmp = para->nDelayTime;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_RECORD_PARAM, 
		strtmp, 
		EM_CONFIG_RECORD_PARAM_REC_DELAYTIME,
		tmp))
	{
		return -1;
	}
	
	tmp = para->nExpireDays;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_RECORD_PARAM, 
		strtmp, 
		EM_CONFIG_RECORD_PARAM_REC_EXPIRE,
		tmp))
	{
		return -1;
	}
/*	
	tmp = para->nTimerRecSchType;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_RECORD_PARAM, 
		strtmp, 
		EM_CONFIG_RECORD_PARAM_REC_SCHTYPE_TIMER,
		tmp))
	{
		return -1;
	}
	
	tmp = para->nMDRecSchType;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_RECORD_PARAM, 
		strtmp, 
		EM_CONFIG_RECORD_PARAM_REC_SCHTYPE_MOTION,
		tmp))
	{
		return -1;
	}
	
	tmp = para->nSensorRecSchType;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_RECORD_PARAM, 
		strtmp, 
		EM_CONFIG_RECORD_PARAM_REC_SCHTYPE_SENSOR,
		tmp))
	{
		return -1;
	}
*/	
	return 0;
}

s32 ConfigSetTimerRecSchParam(SModConfigRecTimerSch* para, s32 nId, s8* pFilepath)
{
	if((NULL == para) || (nId < 0) || (nId >= g_sConfigDvrProperty.nRecNum) || (NULL == pFilepath))
	{
		printf("Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}
	
	s32 tmp = 0;
	s32 i = nId;
	s32 j = 0;
	s8 strtmp[30];
	memset(strtmp, 0, sizeof(strtmp));
	sprintf(strtmp, "%s%d", CONFIG_SCH_PARAM, i);
	s8 strtmp2[30];
	memset(strtmp2, 0, sizeof(strtmp2));
	
	u8 daynum = 0;
	if(para->psRecSch.nSchType == 0)
	{
		daynum = 7;
	}
	else if(para->psRecSch.nSchType == 1)
	{
		daynum = 31;
	}
	else if(para->psRecSch.nSchType == 2)
	{
		daynum = 1;
	}

	for(i = 0; i < daynum; i++)
	{		
		sprintf(strtmp2, "%s%d", CONFIG_SCH_PARAM_REC_TIMER, i);
		j = 0;
		while(1)
		{
			tmp = para->psRecSch.nSchTime[i][j].nStartTime;
			if(0 != PublicIniSetArrayValue(pFilepath, 
				strtmp, 
				strtmp2, 
				EM_CONFIG_SCH_PARAM_STARTTIME + (2 * j),
				tmp))
			{
				break;
			}
			
			tmp = para->psRecSch.nSchTime[i][j].nStopTime;
			if(0 != PublicIniSetArrayValue(pFilepath, 
				strtmp, 
				strtmp2, 
				EM_CONFIG_SCH_PARAM_ENDTIME + (2 * j),
				tmp))
			{
				break;
			}
			
			j++;
			if((j >= g_sConfigDvrProperty.nSchSegments) || (j >= CONFIG_MAX_SCH_TIME_SEGMENTS))
			{
				break;
			}
		}
	}
	
	sprintf(strtmp, "%s%d", CONFIG_RECORD_PARAM_REC, nId);
	tmp = para->psRecSch.nSchType;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_RECORD_PARAM, 
		strtmp, 
		EM_CONFIG_RECORD_PARAM_REC_SCHTYPE_TIMER,
		tmp))
	{
		return -1;
	}
	
	return 0;
}

s32 ConfigSetVMotionRecSchParam(SModConfigRecVMSch* para, s32 nId, s8* pFilepath)
{
	if((NULL == para) || (nId < 0) || (nId >= g_sConfigDvrProperty.nRecNum) || (NULL == pFilepath))
	{
		printf("Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}
	
	s32 tmp = 0;
	s32 i = nId;
	s32 j = 0;
	s8 strtmp[30];
	memset(strtmp, 0, sizeof(strtmp));
	sprintf(strtmp, "%s%d", CONFIG_SCH_PARAM, i);
	s8 strtmp2[30];
	memset(strtmp2, 0, sizeof(strtmp2));
	
	u8 daynum = 0;
	if(para->psRecSch.nSchType == 0)
	{
		daynum = 7;
	}
	else if(para->psRecSch.nSchType == 1)
	{
		daynum = 31;
	}
	else if(para->psRecSch.nSchType == 2)
	{
		daynum = 1;
	}

	for(i = 0; i < daynum; i++)
	{		
		sprintf(strtmp2, "%s%d", CONFIG_SCH_PARAM_REC_VMOTION, i);
		j = 0;
		while(1)
		{
			tmp = para->psRecSch.nSchTime[i][j].nStartTime;
			if(0 != PublicIniSetArrayValue(pFilepath, 
				strtmp, 
				strtmp2, 
				EM_CONFIG_SCH_PARAM_STARTTIME + (2 * j),
				tmp))
			{
				break;
			}
			
			tmp = para->psRecSch.nSchTime[i][j].nStopTime;
			if(0 != PublicIniSetArrayValue(pFilepath, 
				strtmp, 
				strtmp2, 
				EM_CONFIG_SCH_PARAM_ENDTIME + (2 * j),
				tmp))
			{
				break;
			}
			
			j++;
			if((j >= g_sConfigDvrProperty.nSchSegments) || (j >= CONFIG_MAX_SCH_TIME_SEGMENTS))
			{
				break;
			}
		}
	}
	
	sprintf(strtmp, "%s%d", CONFIG_RECORD_PARAM_REC, nId);
	tmp = para->psRecSch.nSchType;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_RECORD_PARAM, 
		strtmp, 
		EM_CONFIG_RECORD_PARAM_REC_SCHTYPE_MOTION,
		tmp))
	{
		return -1;
	}
	
	return 0;
}

s32 ConfigSetSensorRecSchParam(SModConfigRecAlarmInSch* para, s32 nId, s8* pFilepath)
{
	if((NULL == para) || (nId < 0) || (nId >= g_sConfigDvrProperty.nRecNum) || (NULL == pFilepath))
	{
		printf("Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}
	
	s32 tmp = 0;
	s32 i = nId;
	s32 j = 0;
	s8 strtmp[30];
	memset(strtmp, 0, sizeof(strtmp));
	sprintf(strtmp, "%s%d", CONFIG_SCH_PARAM, i);
	s8 strtmp2[30];
	memset(strtmp2, 0, sizeof(strtmp2));
	
	u8 daynum = 0;
	if(para->psRecSch.nSchType == 0)
	{
		daynum = 7;
	}
	else if(para->psRecSch.nSchType == 1)
	{
		daynum = 31;
	}
	else if(para->psRecSch.nSchType == 2)
	{
		daynum = 1;
	}
	
	for(i = 0; i < daynum; i++)
	{
		sprintf(strtmp2, "%s%d", CONFIG_SCH_PARAM_REC_SENSOR, i);
		j = 0;
		while(1)
		{
			tmp = para->psRecSch.nSchTime[i][j].nStartTime;
			if(0 != PublicIniSetArrayValue(pFilepath, 
				strtmp, 
				strtmp2, 
				EM_CONFIG_SCH_PARAM_STARTTIME + (2 * j),
				tmp))
			{
				break;
			}
			
			tmp = para->psRecSch.nSchTime[i][j].nStopTime;
			if(0 != PublicIniSetArrayValue(pFilepath, 
				strtmp, 
				strtmp2, 
				EM_CONFIG_SCH_PARAM_ENDTIME + (2 * j),
				tmp))
			{
				break;
			}
			
			j++;
			if((j >= g_sConfigDvrProperty.nSchSegments) || (j >= CONFIG_MAX_SCH_TIME_SEGMENTS))
			{
				break;
			}
		}
	}
	
	sprintf(strtmp, "%s%d", CONFIG_RECORD_PARAM_REC, nId);
	tmp = para->psRecSch.nSchType;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_RECORD_PARAM, 
		strtmp, 
		EM_CONFIG_RECORD_PARAM_REC_SCHTYPE_SENSOR,
		tmp))
	{
		return -1;
	}
	
	return 0;
}
s32 ConfigSetSGParam(SModConfigSGParam* para, s8* pFilepath)
{
	if((NULL == para) || (NULL == pFilepath))
	{
		printf("Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}
	
	s32 tmp = 0;
	u64 tmp64 = 0;
	s32 i = 0;
	s8 strtmp[30];
	memset(strtmp, 0, sizeof(strtmp));
	sprintf(strtmp, "%s", CONFIG_SG_PARAM_SG);

	char str[100] = {"SSJCXXXXXXXX"};
	char *ps = str;
	
	if (para->AgentID[0] != '\0')//不为空串
	{
		ps = para->AgentID;
	}
	
	if(0 != PublicIniSetArrayString(pFilepath,
		CONFIG_SG_PARAM,
		strtmp,
		EM_CONFIG_SG_PARAM_AgentID,
		ps))
	{
		return -1;
	}
	printf("%s: EM_CONFIG_SG_PARAM_AgentID: %s\n", __FUNCTION__, ps);

	char str1[100] = {"sgdzpic.3322.org"};
	ps = str1;
	if (para->CenterIP[0] != '\0')//不为空串
	{
		ps = para->CenterIP;
	}
	
	if(0 != PublicIniSetArrayString(pFilepath,
		CONFIG_SG_PARAM,
		strtmp,
		EM_CONFIG_SG_PARAM_CenterIP,
		ps))
	{
		return -1;
	}
	printf("%s: EM_CONFIG_SG_PARAM_CenterIP: %s\n", __FUNCTION__, ps);

	tmp = para->RunEnable;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_SG_PARAM, 
		strtmp, 
		EM_CONFIG_SG_PARAM_RunEnable,
		tmp))
	{
		return -1;
	}
	printf("%s: EM_CONFIG_SG_PARAM_RunEnable: %d\n", __FUNCTION__, tmp);

	tmp = para->AlarmEnable;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_SG_PARAM, 
		strtmp, 
		EM_CONFIG_SG_PARAM_AlarmEnable,
		tmp))
	{
		return -1;
	}
	printf("%s: EM_CONFIG_SG_PARAM_AlarmEnable: %d\n", __FUNCTION__, tmp);

	tmp = para->DailyPicEnable;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_SG_PARAM, 
		strtmp, 
		EM_CONFIG_SG_PARAM_DailyPicEnable,
		tmp))
	{
		return -1;
	}
	printf("%s: EM_CONFIG_SG_PARAM_DailyPicEnable: %d\n", __FUNCTION__, tmp);

	tmp = para->normal_exit;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_SG_PARAM, 
		strtmp, 
		EM_CONFIG_DVR_EXIT_NORMOAL,
		tmp))
	{
		return -1;
	}
	printf("%s: EM_CONFIG_DVR_EXIT_NORMOAL: %d\n", __FUNCTION__, tmp);
	
#if 0
	tmp = para->SAlarmPicParam.PreTimes;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_SG_PARAM, 
		strtmp, 
		EM_CONFIG_SG_PARAM_PreTimes,
		tmp))
	{
		return -1;
	}
	printf("%s: EM_CONFIG_SG_PARAM_PreTimes: %d\n", __FUNCTION__, tmp);

	tmp = para->SAlarmPicParam.Interval;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_SG_PARAM, 
		strtmp, 
		EM_CONFIG_SG_PARAM_Interval,
		tmp))
	{
		return -1;
	}
	printf("%s: EM_CONFIG_SG_PARAM_Interval: %d\n", __FUNCTION__, tmp);

	tmp = para->SAlarmPicParam.StillTimes;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_SG_PARAM, 
		strtmp, 
		EM_CONFIG_SG_PARAM_StillTimes,
		tmp))
	{
		return -1;
	}
	printf("%s: EM_CONFIG_SG_PARAM_StillTimes: %d\n", __FUNCTION__, tmp);

	tmp = para->SAlarmPicParam.AlarmInput;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_SG_PARAM, 
		strtmp, 
		EM_CONFIG_SG_PARAM_AlarmInput,
		tmp))
	{
		return -1;
	}
	printf("%s: EM_CONFIG_SG_PARAM_AlarmInput: %d\n", __FUNCTION__, tmp);

	tmp64 = para->SAlarmPicParam.IpcChn;
	if(0 != PublicIniSetArrayValueU64(pFilepath, 
		CONFIG_SG_PARAM, 
		strtmp, 
		EM_CONFIG_SG_PARAM_IpcChn,
		tmp64))
	{
		return -1;
	}
	printf("%s: EM_CONFIG_SG_PARAM_IpcChn: 0x%x\n", __FUNCTION__, tmp64);

	tmp = para->SDailyPicParam.Time1.TimeEnable;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_SG_PARAM, 
		strtmp, 
		EM_CONFIG_SG_PARAM_Time1Enable,
		tmp))
	{
		return -1;
	}
	printf("%s: EM_CONFIG_SG_PARAM_Time1Enable: %d\n", __FUNCTION__, tmp);

	tmp = para->SDailyPicParam.Time1.hour;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_SG_PARAM, 
		strtmp, 
		EM_CONFIG_SG_PARAM_Time1Hour,
		tmp))
	{
		return -1;
	}
	printf("%s: EM_CONFIG_SG_PARAM_Time1Hour: %d\n", __FUNCTION__, tmp);

	tmp = para->SDailyPicParam.Time1.min;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_SG_PARAM, 
		strtmp, 
		EM_CONFIG_SG_PARAM_Time1Min,
		tmp))
	{
		return -1;
	}
	printf("%s: EM_CONFIG_SG_PARAM_Time1Min: %d\n", __FUNCTION__, tmp);

	tmp = para->SDailyPicParam.Time1.sec;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_SG_PARAM, 
		strtmp, 
		EM_CONFIG_SG_PARAM_Time1Sec,
		tmp))
	{
		return -1;
	}
	printf("%s: EM_CONFIG_SG_PARAM_Time1Sec: %d\n", __FUNCTION__, tmp);

	tmp = para->SDailyPicParam.Time2.TimeEnable;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_SG_PARAM, 
		strtmp, 
		EM_CONFIG_SG_PARAM_Time2Enable,
		tmp))
	{
		return -1;
	}
	printf("%s: EM_CONFIG_SG_PARAM_Time2Enable: %d\n", __FUNCTION__, tmp);

	tmp = para->SDailyPicParam.Time2.hour;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_SG_PARAM, 
		strtmp, 
		EM_CONFIG_SG_PARAM_Time2Hour,
		tmp))
	{
		return -1;
	}
	printf("%s: EM_CONFIG_SG_PARAM_Time2Hour: %d\n", __FUNCTION__, tmp);

	tmp = para->SDailyPicParam.Time2.min;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_SG_PARAM, 
		strtmp, 
		EM_CONFIG_SG_PARAM_Time2Min,
		tmp))
	{
		return -1;
	}
	printf("%s: EM_CONFIG_SG_PARAM_Time2Min: %d\n", __FUNCTION__, tmp);

	tmp = para->SDailyPicParam.Time2.sec;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_SG_PARAM, 
		strtmp, 
		EM_CONFIG_SG_PARAM_Time2Sec,
		tmp))
	{
		return -1;
	}
	printf("%s: EM_CONFIG_SG_PARAM_Time2Sec: %d\n", __FUNCTION__, tmp);

	tmp64 = para->SDailyPicParam.IpcChn1;
	if(0 != PublicIniSetArrayValueU64(pFilepath, 
		CONFIG_SG_PARAM, 
		strtmp, 
		EM_CONFIG_SG_PARAM_IpcChn1,
		tmp64))
	{
		return -1;
	}
	printf("%s: EM_CONFIG_SG_PARAM_IpcChn1: 0x%x\n", __FUNCTION__, tmp64);

	tmp64 = para->SDailyPicParam.IpcChn2;
	if(0 != PublicIniSetArrayValueU64(pFilepath, 
		CONFIG_SG_PARAM, 
		strtmp, 
		EM_CONFIG_SG_PARAM_IpcChn2,
		tmp64))
	{
		return -1;
	}
	printf("%s: EM_CONFIG_SG_PARAM_IpcChn2: 0x%x\n", __FUNCTION__, tmp64);
#endif

	return 0;
}
s32 ConfigSetSGDailyParam(SModDailyPicCFG* para, s8* pFilepath)
{
	
	if((NULL == para) || (NULL == pFilepath))
	{
		printf("Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}
	
	s32 tmp = 0;
	u64 tmp64 = 0;
	s32 i = 0;
	s8 strtmp[30];
	memset(strtmp, 0, sizeof(strtmp));
	sprintf(strtmp, "%s", "Daily");

	tmp = para->Time1.TimeEnable;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_SG_PARAM, 
		strtmp, 
		EM_CONFIG_SG_PARAM_Time1Enable,
		tmp))
	{
		return -1;
	}
	printf("%s: EM_CONFIG_SG_PARAM_Time1Enable: %d\n", __FUNCTION__, tmp);

	tmp = para->Time1.hour;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_SG_PARAM, 
		strtmp, 
		EM_CONFIG_SG_PARAM_Time1Hour,
		tmp))
	{
		return -1;
	}
	printf("%s: EM_CONFIG_SG_PARAM_Time1Hour: %d\n", __FUNCTION__, tmp);

	tmp = para->Time1.min;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_SG_PARAM, 
		strtmp, 
		EM_CONFIG_SG_PARAM_Time1Min,
		tmp))
	{
		return -1;
	}
	printf("%s: EM_CONFIG_SG_PARAM_Time1Min: %d\n", __FUNCTION__, tmp);

	tmp = para->Time1.sec;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_SG_PARAM, 
		strtmp, 
		EM_CONFIG_SG_PARAM_Time1Sec,
		tmp))
	{
		return -1;
	}
	printf("%s: EM_CONFIG_SG_PARAM_Time1Sec: %d\n", __FUNCTION__, tmp);

	tmp = para->Time2.TimeEnable;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_SG_PARAM, 
		strtmp, 
		EM_CONFIG_SG_PARAM_Time2Enable,
		tmp))
	{
		return -1;
	}
	printf("%s: EM_CONFIG_SG_PARAM_Time2Enable: %d\n", __FUNCTION__, tmp);

	tmp = para->Time2.hour;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_SG_PARAM, 
		strtmp, 
		EM_CONFIG_SG_PARAM_Time2Hour,
		tmp))
	{
		return -1;
	}
	printf("%s: EM_CONFIG_SG_PARAM_Time2Hour: %d\n", __FUNCTION__, tmp);

	tmp = para->Time2.min;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_SG_PARAM, 
		strtmp, 
		EM_CONFIG_SG_PARAM_Time2Min,
		tmp))
	{
		return -1;
	}
	printf("%s: EM_CONFIG_SG_PARAM_Time2Min: %d\n", __FUNCTION__, tmp);

	tmp = para->Time2.sec;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_SG_PARAM, 
		strtmp, 
		EM_CONFIG_SG_PARAM_Time2Sec,
		tmp))
	{
		return -1;
	}
	printf("%s: EM_CONFIG_SG_PARAM_Time2Sec: %d\n", __FUNCTION__, tmp);

	tmp64 = para->IpcChn1;
	if(0 != PublicIniSetArrayValueU64(pFilepath, 
		CONFIG_SG_PARAM, 
		strtmp, 
		EM_CONFIG_SG_PARAM_IpcChn1,
		tmp64))
	{
		return -1;
	}
	printf("%s: EM_CONFIG_SG_PARAM_IpcChn1: 0x%x\n", __FUNCTION__, tmp64);

	tmp64 = para->IpcChn2;
	if(0 != PublicIniSetArrayValueU64(pFilepath, 
		CONFIG_SG_PARAM, 
		strtmp, 
		EM_CONFIG_SG_PARAM_IpcChn2,
		tmp64))
	{
		return -1;
	}
	printf("%s: EM_CONFIG_SG_PARAM_IpcChn2: 0x%x\n", __FUNCTION__, tmp64);
	
	return 0;
	
}
s32 ConfigSetSGAlarmParam(SModAlarmPicCFG* para, s32 nId, s8* pFilepath)
{
	int num = g_sConfigDvrProperty.nSensorNum + g_sConfigDvrProperty.nVidMainNum;
	if((NULL == para) || (nId < 0) || (nId >= num) || (NULL == pFilepath))
	{
		printf("Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}
	s32 tmp = 0;
	u64 tmp64 = 0;
	s32 i = 0;
	s8 strtmp[32];
	s8 str[100];
	
	memset(strtmp, 0, sizeof(strtmp));
	//sprintf(strtmp, "%s", CONFIG_SG_PARAM_SG);
	sprintf(strtmp, "Chn%d", nId);

	tmp = para->PreTimes;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_SG_PARAM, 
		strtmp, 
		EM_CONFIG_SG_PARAM_PreTimes,
		tmp))
	{
		return -1;
	}
	printf("%s: EM_CONFIG_SG_PARAM_PreTimes: %d\n", __FUNCTION__, tmp);

	tmp = para->Interval;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_SG_PARAM, 
		strtmp, 
		EM_CONFIG_SG_PARAM_Interval,
		tmp))
	{
		return -1;
	}
	printf("%s: EM_CONFIG_SG_PARAM_Interval: %d\n", __FUNCTION__, tmp);

	tmp = para->StillTimes;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_SG_PARAM, 
		strtmp, 
		EM_CONFIG_SG_PARAM_StillTimes,
		tmp))
	{
		return -1;
	}
	printf("%s: EM_CONFIG_SG_PARAM_StillTimes: %d\n", __FUNCTION__, tmp);
#if 0
	tmp = para->AlarmInput;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_SENSOR_PARAM, 
		strtmp, 
		EM_CONFIG_SG_PARAM_AlarmInput,
		tmp))
	{
		return -1;
	}
	printf("%s: EM_CONFIG_SG_PARAM_AlarmInput: %d\n", __FUNCTION__, tmp);
#endif
	tmp64 = para->IpcChn;
	if(0 != PublicIniSetArrayValueU64(pFilepath, 
		CONFIG_SG_PARAM, 
		strtmp, 
		EM_CONFIG_SG_PARAM_IpcChn,
		tmp64))
	{
		return -1;
	}
	printf("%s: EM_CONFIG_SG_PARAM_IpcChn: 0x%x\n", __FUNCTION__, tmp64);
	
	return 0;
}


s32 ConfigSetSensorParam(SModConfigSensorParam* para, s32 nId, s8* pFilepath)
{
	if((NULL == para) || (nId < 0) || (nId >= g_sConfigDvrProperty.nSensorNum) || (NULL == pFilepath))
	{
		printf("Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}
	
	s32 tmp = 0;
	u64 tmp64 = 0;
	s32 i = nId;
	s8 strtmp[30];
	memset(strtmp, 0, sizeof(strtmp));
	sprintf(strtmp, "%s%d", CONFIG_SENSOR_PARAM_SENSOR, i);
	
	tmp = para->nEnalbe;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_SENSOR_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_ENABLE,
		tmp))
	{
		return -1;
	}
	
	tmp = para->nSensorType;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_SENSOR_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_TYPE,
		tmp))
	{
		return -1;
	}
	
	tmp = para->nDelay;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_SENSOR_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_DELAY,
		tmp))
	{
		return -1;
	}
/*	
	tmp = para->nSchType;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_SENSOR_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_SCHTYPE,
		tmp))
	{
		return -1;
	}
*/	
	if(0 != SetArrayStringToIni(pFilepath, 
		CONFIG_SENSOR_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_NAME,
		para->name))
	{
		return -1;
	}

	tmp = para->psDispatch.nFlagBuzz;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_SENSOR_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_FLAG_BUZZ,
		tmp))
	{
		return -1;
	}
	
	tmp = para->psDispatch.nZoomChn;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_SENSOR_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_ZOOMCHN,
		tmp))
	{
		return -1;
	}
	
	tmp = para->psDispatch.nFlagEmail;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_SENSOR_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_FLAG_MAIL,
		tmp))
	{
		return -1;
	}
	
	tmp64 = para->psDispatch.nSnapChn;
	if(0 != PublicIniSetArrayValueU64(pFilepath, 
		CONFIG_SENSOR_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_SNAPCHN,
		tmp64))
	{
		return -1;
	}
	
	tmp64 = para->psDispatch.nAlarmOut;
	if(0 != PublicIniSetArrayValueU64(pFilepath, 
		CONFIG_SENSOR_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_ALARMOUT,
		tmp64))
	{
		return -1;
	}
	
	tmp64 = para->psDispatch.nRecordChn;
	if(0 != PublicIniSetArrayValueU64(pFilepath, 
		CONFIG_SENSOR_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_RECCHN,
		tmp64))
	{
		return -1;
	}
	
	for(i = 0; i < g_sConfigDvrProperty.nPreviewNum; i++)
	{
		tmp = para->psDispatch.sAlarmPtz[i].nPtzType;
		
		if(0 != PublicIniSetArrayValue(pFilepath, 
			CONFIG_SENSOR_PARAM, 
			strtmp, 
			EM_CONFIG_SENSOR_PARAM_PTZTYPE + (3 * i),
			tmp))
		{
			return -1;
		}
		
		tmp = para->psDispatch.sAlarmPtz[i].nChn;
		if(0 != PublicIniSetArrayValue(pFilepath, 
			CONFIG_SENSOR_PARAM, 
			strtmp, 
			EM_CONFIG_SENSOR_PARAM_PTZCHN + (3 * i),
			tmp))
		{
			return -1;
		}
		
		tmp = para->psDispatch.sAlarmPtz[i].nId;
		if(0 != PublicIniSetArrayValue(pFilepath, 
			CONFIG_SENSOR_PARAM, 
			strtmp, 
			EM_CONFIG_SENSOR_PARAM_PTZID + (3 * i),
			tmp))
		{
			return -1;
		}
	}

	return 0;
}
//yaogang modify 20141010
s32 ConfigSetIPCCoverParam(SModConfigIPCExtSensorParam* para, s32 nId, s8* pFilepath)
{
	if((NULL == para) || (nId < 0) || (nId >= g_sConfigDvrProperty.nVidMainNum) || (NULL == pFilepath))
	{
		printf("Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}
	
	s32 tmp = 0;
	u64 tmp64 = 0;
	s32 i = nId;
	s8 strtmp[30];
	memset(strtmp, 0, sizeof(strtmp));
	sprintf(strtmp, "%s%d", CONFIG_SENSOR_PARAM_SENSOR, i);
	
	tmp = para->nEnalbe;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_IPCCOVER_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_ENABLE,
		tmp))
	{
		return -1;
	}
	
	tmp = para->nDelay;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_IPCCOVER_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_DELAY,
		tmp))
	{
		return -1;
	}

	tmp = para->psDispatch.nFlagBuzz;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_IPCCOVER_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_FLAG_BUZZ,
		tmp))
	{
		return -1;
	}
	
	tmp = para->psDispatch.nZoomChn;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_IPCCOVER_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_ZOOMCHN,
		tmp))
	{
		return -1;
	}
	
	tmp = para->psDispatch.nFlagEmail;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_IPCCOVER_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_FLAG_MAIL,
		tmp))
	{
		return -1;
	}
	
	tmp64 = para->psDispatch.nSnapChn;
	if(0 != PublicIniSetArrayValueU64(pFilepath, 
		CONFIG_IPCCOVER_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_SNAPCHN,
		tmp64))
	{
		return -1;
	}
	
	tmp64 = para->psDispatch.nAlarmOut;
	if(0 != PublicIniSetArrayValueU64(pFilepath, 
		CONFIG_IPCCOVER_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_ALARMOUT,
		tmp64))
	{
		return -1;
	}
	
	tmp64 = para->psDispatch.nRecordChn;
	if(0 != PublicIniSetArrayValueU64(pFilepath, 
		CONFIG_IPCCOVER_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_RECCHN,
		tmp64))
	{
		return -1;
	}
	
	for(i = 0; i < g_sConfigDvrProperty.nPreviewNum; i++)
	{
		tmp = para->psDispatch.sAlarmPtz[i].nPtzType;
		
		if(0 != PublicIniSetArrayValue(pFilepath, 
			CONFIG_IPCCOVER_PARAM, 
			strtmp, 
			EM_CONFIG_SENSOR_PARAM_PTZTYPE + (3 * i),
			tmp))
		{
			return -1;
		}
		
		tmp = para->psDispatch.sAlarmPtz[i].nChn;
		if(0 != PublicIniSetArrayValue(pFilepath, 
			CONFIG_IPCCOVER_PARAM, 
			strtmp, 
			EM_CONFIG_SENSOR_PARAM_PTZCHN + (3 * i),
			tmp))
		{
			return -1;
		}
		
		tmp = para->psDispatch.sAlarmPtz[i].nId;
		if(0 != PublicIniSetArrayValue(pFilepath, 
			CONFIG_IPCCOVER_PARAM, 
			strtmp, 
			EM_CONFIG_SENSOR_PARAM_PTZID + (3 * i),
			tmp))
		{
			return -1;
		}
	}

	return 0;
}


s32 ConfigSetIPCExtSensorParam(SModConfigIPCExtSensorParam* para, s32 nId, s8* pFilepath)
{
	if((NULL == para) || (nId < 0) || (nId >= g_sConfigDvrProperty.nVidMainNum) || (NULL == pFilepath))
	{
		printf("Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}
	
	s32 tmp = 0;
	u64 tmp64 = 0;
	s32 i = nId;
	s8 strtmp[30];
	memset(strtmp, 0, sizeof(strtmp));
	sprintf(strtmp, "%s%d", CONFIG_SENSOR_PARAM_SENSOR, i);
	
	tmp = para->nEnalbe;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_IPCExtSENSOR_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_ENABLE,
		tmp))
	{
		return -1;
	}
	
	tmp = para->nDelay;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_IPCExtSENSOR_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_DELAY,
		tmp))
	{
		return -1;
	}

	tmp = para->psDispatch.nFlagBuzz;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_IPCExtSENSOR_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_FLAG_BUZZ,
		tmp))
	{
		return -1;
	}
	
	tmp = para->psDispatch.nZoomChn;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_IPCExtSENSOR_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_ZOOMCHN,
		tmp))
	{
		return -1;
	}
	
	tmp = para->psDispatch.nFlagEmail;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_IPCExtSENSOR_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_FLAG_MAIL,
		tmp))
	{
		return -1;
	}
	
	tmp64 = para->psDispatch.nSnapChn;
	if(0 != PublicIniSetArrayValueU64(pFilepath, 
		CONFIG_IPCExtSENSOR_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_SNAPCHN,
		tmp64))
	{
		return -1;
	}
	
	tmp64 = para->psDispatch.nAlarmOut;
	if(0 != PublicIniSetArrayValueU64(pFilepath, 
		CONFIG_IPCExtSENSOR_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_ALARMOUT,
		tmp64))
	{
		return -1;
	}
	
	tmp64 = para->psDispatch.nRecordChn;
	if(0 != PublicIniSetArrayValueU64(pFilepath, 
		CONFIG_IPCExtSENSOR_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_RECCHN,
		tmp64))
	{
		return -1;
	}
	
	for(i = 0; i < g_sConfigDvrProperty.nPreviewNum; i++)
	{
		tmp = para->psDispatch.sAlarmPtz[i].nPtzType;
		
		if(0 != PublicIniSetArrayValue(pFilepath, 
			CONFIG_IPCExtSENSOR_PARAM, 
			strtmp, 
			EM_CONFIG_SENSOR_PARAM_PTZTYPE + (3 * i),
			tmp))
		{
			return -1;
		}
		
		tmp = para->psDispatch.sAlarmPtz[i].nChn;
		if(0 != PublicIniSetArrayValue(pFilepath, 
			CONFIG_IPCExtSENSOR_PARAM, 
			strtmp, 
			EM_CONFIG_SENSOR_PARAM_PTZCHN + (3 * i),
			tmp))
		{
			return -1;
		}
		
		tmp = para->psDispatch.sAlarmPtz[i].nId;
		if(0 != PublicIniSetArrayValue(pFilepath, 
			CONFIG_IPCExtSENSOR_PARAM, 
			strtmp, 
			EM_CONFIG_SENSOR_PARAM_PTZID + (3 * i),
			tmp))
		{
			return -1;
		}
	}

	return 0;
}
s32 ConfigSet485ExtSensorParam(SModConfigIPCExtSensorParam* para, s32 nId, s8* pFilepath)
{
	if((NULL == para) || (nId < 0) || (nId >= g_sConfigDvrProperty.nVidMainNum) || (NULL == pFilepath))
	{
		printf("Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}
	
	s32 tmp = 0;
	u64 tmp64 = 0;
	s32 i = nId;
	s8 strtmp[30];
	memset(strtmp, 0, sizeof(strtmp));
	sprintf(strtmp, "%s%d", CONFIG_SENSOR_PARAM_SENSOR, i);
	
	tmp = para->nEnalbe;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_485ExtSENSOR_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_ENABLE,
		tmp))
	{
		return -1;
	}
	
	tmp = para->nDelay;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_485ExtSENSOR_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_DELAY,
		tmp))
	{
		return -1;
	}

	tmp = para->psDispatch.nFlagBuzz;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_485ExtSENSOR_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_FLAG_BUZZ,
		tmp))
	{
		return -1;
	}
	
	tmp = para->psDispatch.nZoomChn;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_485ExtSENSOR_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_ZOOMCHN,
		tmp))
	{
		return -1;
	}
	
	tmp = para->psDispatch.nFlagEmail;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_485ExtSENSOR_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_FLAG_MAIL,
		tmp))
	{
		return -1;
	}
	
	tmp64 = para->psDispatch.nSnapChn;
	if(0 != PublicIniSetArrayValueU64(pFilepath, 
		CONFIG_485ExtSENSOR_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_SNAPCHN,
		tmp64))
	{
		return -1;
	}
	
	tmp64 = para->psDispatch.nAlarmOut;
	if(0 != PublicIniSetArrayValueU64(pFilepath, 
		CONFIG_485ExtSENSOR_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_ALARMOUT,
		tmp64))
	{
		return -1;
	}
	
	tmp64 = para->psDispatch.nRecordChn;
	if(0 != PublicIniSetArrayValueU64(pFilepath, 
		CONFIG_485ExtSENSOR_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_RECCHN,
		tmp64))
	{
		return -1;
	}
	
	for(i = 0; i < g_sConfigDvrProperty.nPreviewNum; i++)
	{
		tmp = para->psDispatch.sAlarmPtz[i].nPtzType;
		
		if(0 != PublicIniSetArrayValue(pFilepath, 
			CONFIG_485ExtSENSOR_PARAM, 
			strtmp, 
			EM_CONFIG_SENSOR_PARAM_PTZTYPE + (3 * i),
			tmp))
		{
			return -1;
		}
		
		tmp = para->psDispatch.sAlarmPtz[i].nChn;
		if(0 != PublicIniSetArrayValue(pFilepath, 
			CONFIG_485ExtSENSOR_PARAM, 
			strtmp, 
			EM_CONFIG_SENSOR_PARAM_PTZCHN + (3 * i),
			tmp))
		{
			return -1;
		}
		
		tmp = para->psDispatch.sAlarmPtz[i].nId;
		if(0 != PublicIniSetArrayValue(pFilepath, 
			CONFIG_485ExtSENSOR_PARAM, 
			strtmp, 
			EM_CONFIG_SENSOR_PARAM_PTZID + (3 * i),
			tmp))
		{
			return -1;
		}
	}

	return 0;
}

s32 ConfigSetHDDParam(SModConfigHDDParam* para, s32 nId,s8* pFilepath)
{
	if((NULL == para) || (NULL == pFilepath))
	{
		printf("Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}
	
	s32 tmp = 0;
	u64 tmp64 = 0;
	s32 i = nId;
	s8 strtmp[30];
	memset(strtmp, 0, sizeof(strtmp));
	sprintf(strtmp, "%s", CONFIG_SENSOR_PARAM_SENSOR);
	
	tmp = para->nEnalbe;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_HDD_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_ENABLE,
		tmp))
	{
		return -1;
	}

	tmp = para->psDispatch.nFlagBuzz;
	//printf("yg ConfigSetHDDParam nFlagBuzz: %d\n", tmp);
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_HDD_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_FLAG_BUZZ,
		tmp))
	{
		return -1;
	}
	
	tmp = para->psDispatch.nZoomChn;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_HDD_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_ZOOMCHN,
		tmp))
	{
		return -1;
	}
	
	tmp = para->psDispatch.nFlagEmail;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_HDD_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_FLAG_MAIL,
		tmp))
	{
		return -1;
	}
	
	tmp64 = para->psDispatch.nSnapChn;
	if(0 != PublicIniSetArrayValueU64(pFilepath, 
		CONFIG_HDD_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_SNAPCHN,
		tmp64))
	{
		return -1;
	}
	
	tmp64 = para->psDispatch.nAlarmOut;
	if(0 != PublicIniSetArrayValueU64(pFilepath, 
		CONFIG_HDD_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_ALARMOUT,
		tmp64))
	{
		return -1;
	}
	
	tmp64 = para->psDispatch.nRecordChn;
	if(0 != PublicIniSetArrayValueU64(pFilepath, 
		CONFIG_HDD_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_RECCHN,
		tmp64))
	{
		return -1;
	}
	
	for(i = 0; i < g_sConfigDvrProperty.nPreviewNum; i++)
	{
		tmp = para->psDispatch.sAlarmPtz[i].nPtzType;
		
		if(0 != PublicIniSetArrayValue(pFilepath, 
			CONFIG_HDD_PARAM, 
			strtmp, 
			EM_CONFIG_SENSOR_PARAM_PTZTYPE + (3 * i),
			tmp))
		{
			return -1;
		}
		
		tmp = para->psDispatch.sAlarmPtz[i].nChn;
		if(0 != PublicIniSetArrayValue(pFilepath, 
			CONFIG_HDD_PARAM, 
			strtmp, 
			EM_CONFIG_SENSOR_PARAM_PTZCHN + (3 * i),
			tmp))
		{
			return -1;
		}
		
		tmp = para->psDispatch.sAlarmPtz[i].nId;
		if(0 != PublicIniSetArrayValue(pFilepath, 
			CONFIG_HDD_PARAM, 
			strtmp, 
			EM_CONFIG_SENSOR_PARAM_PTZID + (3 * i),
			tmp))
		{
			return -1;
		}
	}

	return 0;
}


s32 ConfigSetVMotionParam(SModConfigVMotionParam* para, s32 nId, s8* pFilepath)
{
	if((NULL == para) || (nId < 0) || (nId >= g_sConfigDvrProperty.nPreviewNum) || (NULL == pFilepath))
	{
		printf("Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}
	
	s32 tmp = 0;
	u64 tmp64 = 0;
	s32 i = nId;
	s8 strtmp[30];
	memset(strtmp, 0, sizeof(strtmp));
	sprintf(strtmp, "%s%d", CONFIG_VMOTION_PARAM_VMOTION, i);
	
	tmp = para->nEnalbe;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_VMOTION_PARAM, 
		strtmp, 
		EM_CONFIG_VMOTION_PARAM_ENABLE,
		tmp))
	{
		return -1;
	}
	
	tmp = para->nSensitivity;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_VMOTION_PARAM, 
		strtmp, 
		EM_CONFIG_VMOTION_PARAM_SENSITIVITY,
		tmp))
	{
		return -1;
	}
	
	tmp = para->nDelay;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_VMOTION_PARAM, 
		strtmp, 
		EM_CONFIG_VMOTION_PARAM_DELAY,
		tmp))
	{
		return -1;
	}
/*	
	tmp = para->nSchType;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_VMOTION_PARAM, 
		strtmp, 
		EM_CONFIG_VMOTION_PARAM_SCHTYPE,
		tmp))
	{
		return -1;
	}
*/	
	tmp = para->psDispatch.nFlagBuzz;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_VMOTION_PARAM, 
		strtmp, 
		EM_CONFIG_VMOTION_PARAM_FLAG_BUZZ,
		tmp))
	{
		return -1;
	}
	
	tmp = para->psDispatch.nZoomChn;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_VMOTION_PARAM, 
		strtmp, 
		EM_CONFIG_VMOTION_PARAM_ZOOMCHN,
		tmp))
	{
		return -1;
	}
	
	tmp = para->psDispatch.nFlagEmail;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_VMOTION_PARAM, 
		strtmp, 
		EM_CONFIG_VMOTION_PARAM_FLAG_MAIL,
		tmp))
	{
		return -1;
	}
	
	tmp64 = para->psDispatch.nSnapChn;
	if(0 != PublicIniSetArrayValueU64(pFilepath, 
		CONFIG_VMOTION_PARAM, 
		strtmp, 
		EM_CONFIG_VMOTION_PARAM_SNAPCHN,
		tmp64))
	{
		return -1;
	}
	
	tmp64 = para->psDispatch.nAlarmOut;
	if(0 != PublicIniSetArrayValueU64(pFilepath, 
		CONFIG_VMOTION_PARAM, 
		strtmp, 
		EM_CONFIG_VMOTION_PARAM_ALARMOUT,
		tmp64))
	{
		return -1;
	}
	
	tmp64 = para->psDispatch.nRecordChn;
	if(0 != PublicIniSetArrayValueU64(pFilepath, 
		CONFIG_VMOTION_PARAM, 
		strtmp, 
		EM_CONFIG_VMOTION_PARAM_RECCHN,
		tmp64))
	{
		return -1;
	}
	
	for(i = 0; i < g_sConfigDvrProperty.nPreviewNum; i++)
	{
		tmp = para->psDispatch.sAlarmPtz[i].nPtzType;
		if(0 != PublicIniSetArrayValue(pFilepath, 
			CONFIG_VMOTION_PARAM, 
			strtmp, 
			EM_CONFIG_VMOTION_PARAM_STATUS + g_sConfigDvrProperty.nMdAreaRows + (3 * i),//EM_CONFIG_VMOTION_PARAM_PTZTYPE + (3 * i),
			tmp))
		{
			return -1;
		}
		tmp = para->psDispatch.sAlarmPtz[i].nChn;
		if(0 != PublicIniSetArrayValue(pFilepath, 
			CONFIG_VMOTION_PARAM, 
			strtmp, 
			EM_CONFIG_VMOTION_PARAM_STATUS + g_sConfigDvrProperty.nMdAreaRows + (3 * i) + 1,//EM_CONFIG_VMOTION_PARAM_PTZCHN + (3 * i),
			tmp))
		{
			return -1;
		}
		tmp = para->psDispatch.sAlarmPtz[i].nId;
		if(0 != PublicIniSetArrayValue(pFilepath, 
			CONFIG_VMOTION_PARAM, 
			strtmp, 
			EM_CONFIG_VMOTION_PARAM_STATUS + g_sConfigDvrProperty.nMdAreaRows + (3 * i) + 2,//EM_CONFIG_VMOTION_PARAM_PTZID + (3 * i),
			tmp))
		{
			return -1;
		}
	}

	for(i = 0; i < g_sConfigDvrProperty.nMdAreaRows; i++)
	{
		tmp64 = para->nBlockStatus[i];
		if(0 != PublicIniSetArrayValueU64(pFilepath, 
			CONFIG_VMOTION_PARAM, 
			strtmp, 
			EM_CONFIG_VMOTION_PARAM_STATUS  + i,
			//EM_CONFIG_VMOTION_PARAM_PTZTYPE + (3 * g_sConfigDvrProperty.nPreviewNum + i),
			tmp64))
		{
			return -1;
		}
	}

	return 0;
}

s32 ConfigSetVLostParam(SModConfigVLostParam* para, s32 nId, s8* pFilepath)
{
	if((NULL == para) || (nId < 0) || (nId >= g_sConfigDvrProperty.nPreviewNum) || (NULL == pFilepath))
	{
		printf("Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}
	
	s32 tmp = 0;
	u64 tmp64 = 0;
	s32 i = nId;
	s8 strtmp[30];
	memset(strtmp, 0, sizeof(strtmp));
	sprintf(strtmp, "%s%d", CONFIG_VLOST_PARAM_VLOST, i);
	
	tmp = para->nEnalbe;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_VLOST_PARAM, 
		strtmp, 
		EM_CONFIG_VLOST_PARAM_ENABLE,
		tmp))
	{
		return -1;
	}
	
	tmp = para->nDelay;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_VLOST_PARAM, 
		strtmp, 
		EM_CONFIG_VLOST_PARAM_DELAY,
		tmp))
	{
		return -1;
	}
/*	
	tmp = para->nSchType;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_VLOST_PARAM, 
		strtmp, 
		EM_CONFIG_VLOST_PARAM_SCHTYPE,
		tmp))
	{
		return -1;
	}
*/	
	tmp = para->psDispatch.nFlagBuzz;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_VLOST_PARAM, 
		strtmp, 
		EM_CONFIG_VLOST_PARAM_FLAG_BUZZ,
		tmp))
	{
		return -1;
	}
	
	tmp = para->psDispatch.nZoomChn;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_VLOST_PARAM, 
		strtmp, 
		EM_CONFIG_VLOST_PARAM_ZOOMCHN,
		tmp))
	{
		return -1;
	}
	
	tmp = para->psDispatch.nFlagEmail;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_VLOST_PARAM, 
		strtmp, 
		EM_CONFIG_VLOST_PARAM_FLAG_MAIL,
		tmp))
	{
		return -1;
	}
	
	tmp64 = para->psDispatch.nSnapChn;
	if(0 != PublicIniSetArrayValueU64(pFilepath, 
		CONFIG_VLOST_PARAM, 
		strtmp, 
		EM_CONFIG_VLOST_PARAM_SNAPCHN,
		tmp64))
	{
		return -1;
	}
	
	tmp64 = para->psDispatch.nAlarmOut;
	if(0 != PublicIniSetArrayValueU64(pFilepath, 
		CONFIG_VLOST_PARAM, 
		strtmp, 
		EM_CONFIG_VLOST_PARAM_ALARMOUT,
		tmp64))
	{
		return -1;
	}
	
	tmp64 = para->psDispatch.nRecordChn;
	if(0 != PublicIniSetArrayValueU64(pFilepath, 
		CONFIG_VLOST_PARAM, 
		strtmp, 
		EM_CONFIG_VLOST_PARAM_RECCHN,
		tmp64))
	{
		return -1;
	}
	
	for(i = 0; i < g_sConfigDvrProperty.nPreviewNum; i++)
	{
		tmp = para->psDispatch.sAlarmPtz[i].nPtzType;
		if(0 != PublicIniSetArrayValue(pFilepath, 
			CONFIG_VLOST_PARAM, 
			strtmp, 
			EM_CONFIG_VLOST_PARAM_PTZTYPE + (3 * i),
			tmp))
		{
			return -1;
		}
		
		tmp = para->psDispatch.sAlarmPtz[i].nChn;
		if(0 != PublicIniSetArrayValue(pFilepath, 
			CONFIG_VLOST_PARAM, 
			strtmp, 
			EM_CONFIG_VLOST_PARAM_PTZCHN + (3 * i),
			tmp))
		{
			return -1;
		}
		
		tmp = para->psDispatch.sAlarmPtz[i].nId;
		if(0 != PublicIniSetArrayValue(pFilepath, 
			CONFIG_VLOST_PARAM, 
			strtmp, 
			EM_CONFIG_VLOST_PARAM_PTZID + (3 * i),
			tmp))
		{
			return -1;
		}
	}

	return 0;
}

s32 ConfigSetVBlindParam(SModConfigVBlindParam* para, s32 nId, s8* pFilepath)
{
	if((NULL == para) || (nId < 0) || (nId >= g_sConfigDvrProperty.nPreviewNum) || (NULL == pFilepath))
	{
		printf("Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}
	
	s32 tmp = 0;
	u64 tmp64 = 0;
	s32 i = nId;
	s8 strtmp[30];
	memset(strtmp, 0, sizeof(strtmp));
	sprintf(strtmp, "%s%d", CONFIG_VBLIND_PARAM_VBLIND, i);
	
	tmp = para->nEnalbe;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_VBLIND_PARAM, 
		strtmp, 
		EM_CONFIG_VBLIND_PARAM_ENABLE,
		tmp))
	{
		return -1;
	}
	
	tmp = para->nDelay;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_VBLIND_PARAM, 
		strtmp, 
		EM_CONFIG_VBLIND_PARAM_DELAY,
		tmp))
	{
		return -1;
	}
/*	
	tmp = para->nSchType;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_VBLIND_PARAM, 
		strtmp, 
		EM_CONFIG_VBLIND_PARAM_SCHTYPE,
		tmp))
	{
		return -1;
	}
*/	
	tmp = para->psDispatch.nFlagBuzz;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_VBLIND_PARAM, 
		strtmp, 
		EM_CONFIG_VBLIND_PARAM_FLAG_BUZZ,
		tmp))
	{
		return -1;
	}
	
	tmp = para->psDispatch.nZoomChn;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_VBLIND_PARAM, 
		strtmp, 
		EM_CONFIG_VBLIND_PARAM_ZOOMCHN,
		tmp))
	{
		return -1;
	}
	
	tmp = para->psDispatch.nFlagEmail;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_VBLIND_PARAM, 
		strtmp, 
		EM_CONFIG_VBLIND_PARAM_FLAG_MAIL,
		tmp))
	{
		return -1;
	}
	
	tmp64 = para->psDispatch.nSnapChn;
	if(0 != PublicIniSetArrayValueU64(pFilepath, 
		CONFIG_VBLIND_PARAM, 
		strtmp, 
		EM_CONFIG_VBLIND_PARAM_SNAPCHN,
		tmp64))
	{
		return -1;
	}
	
	tmp64 = para->psDispatch.nAlarmOut;
	if(0 != PublicIniSetArrayValueU64(pFilepath, 
		CONFIG_VBLIND_PARAM, 
		strtmp, 
		EM_CONFIG_VBLIND_PARAM_ALARMOUT,
		tmp64))
	{
		return -1;
	}
	
	tmp64 = para->psDispatch.nRecordChn;
	if(0 != PublicIniSetArrayValueU64(pFilepath, 
		CONFIG_VBLIND_PARAM, 
		strtmp, 
		EM_CONFIG_VBLIND_PARAM_RECCHN,
		tmp64))
	{
		return -1;
	}
	
	for(i = 0; i < g_sConfigDvrProperty.nPreviewNum; i++)
	{
		tmp = para->psDispatch.sAlarmPtz[i].nPtzType;
		if(0 != PublicIniSetArrayValue(pFilepath, 
			CONFIG_VBLIND_PARAM, 
			strtmp, 
			EM_CONFIG_VBLIND_PARAM_PTZTYPE + (3 * i),
			tmp))
		{
			return -1;
		}
		
		tmp = para->psDispatch.sAlarmPtz[i].nChn;
		if(0 != PublicIniSetArrayValue(pFilepath, 
			CONFIG_VBLIND_PARAM, 
			strtmp, 
			EM_CONFIG_VBLIND_PARAM_PTZCHN + (3 * i),
			tmp))
		{
			return -1;
		}
		
		tmp = para->psDispatch.sAlarmPtz[i].nId;
		if(0 != PublicIniSetArrayValue(pFilepath, 
			CONFIG_VBLIND_PARAM, 
			strtmp, 
			EM_CONFIG_VBLIND_PARAM_PTZID + (3 * i),
			tmp))
		{
			return -1;
		}
	}

	return 0;
}

s32 ConfigSetAlarmOutParam(SModConfigAlarmOutParam* para, s32 nId, s8* pFilepath)
{
	if((NULL == para) || (nId < 0) || (nId >= g_sConfigDvrProperty.nAlarmoutNum) || (NULL == pFilepath))
	{
		printf("Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}
	
	s32 tmp = 0;
	s32 i = nId;
	s8 strtmp[30];
	memset(strtmp, 0, sizeof(strtmp));
	sprintf(strtmp, "%s%d", CONFIG_ALARMOUT_PARAM_ALARMOUT, i);
	
	tmp = para->nEnalbe;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_ALARMOUT_PARAM, 
		strtmp, 
		EM_CONFIG_ALARMOUT_PARAM_ENABLE,
		tmp))
	{
		return -1;
	}
	
	tmp = para->nAlarmOutType;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_ALARMOUT_PARAM, 
		strtmp, 
		EM_CONFIG_ALARMOUT_PARAM_TYPE,
		tmp))
	{
		return -1;
	}
	
	tmp = para->nDelay;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_ALARMOUT_PARAM, 
		strtmp, 
		EM_CONFIG_ALARMOUT_PARAM_DELAY,
		tmp))
	{
		return -1;
	}
	
	if(0 != SetArrayStringToIni(pFilepath, 
		CONFIG_ALARMOUT_PARAM, 
		strtmp, 
		EM_CONFIG_ALARMOUT_PARAM_NAME,
		para->name))
	{
		return -1;
	}
/*
	tmp = para->nSchType;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_ALARMOUT_PARAM, 
		strtmp, 
		EM_CONFIG_ALARMOUT_PARAM_SCHTYPE,
		tmp))
	{
		return -1;
	}
*/	
	return 0;
}

s32 ConfigSetBuzzParam(SModConfigBuzzParam* para, s32 nId, s8* pFilepath)
{
	if((NULL == para) || (nId < 0) || (nId >= g_sConfigDvrProperty.nBuzzNum) || (NULL == pFilepath))
	{
		printf("Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}
	
	s32 tmp = 0;
	s32 i = nId;
	s8 strtmp[30];
	memset(strtmp, 0, sizeof(strtmp));
	sprintf(strtmp, "%s%d", CONFIG_BUZZ_PARAM_BUZZ, i);
	
	tmp = para->nEnalbe;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_BUZZ_PARAM, 
		strtmp, 
		EM_CONFIG_BUZZ_PARAM_ENABLE,
		tmp))
	{
		return -1;
	}
	
	tmp = para->nDelay;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_BUZZ_PARAM, 
		strtmp, 
		EM_CONFIG_BUZZ_PARAM_DELAY,
		tmp))
	{
		return -1;
	}
	
	//para->nDuration = 0;
	//para->nInterval = 0;

	return 0;
}

s32 ConfigSetSensorSchParam(SModConfigSensorSch* para, s32 nId, s8* pFilepath)
{
	if((NULL == para) || (nId < 0) || (nId >= g_sConfigDvrProperty.nSensorNum) || (NULL == pFilepath))
	{
		printf("Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}
	
	s32 tmp = 0;
	s32 i = nId;
	s32 j = 0;
	s8 strtmp[30];
	memset(strtmp, 0, sizeof(strtmp));
	sprintf(strtmp, "%s%d", CONFIG_SCH_PARAM, i);
	s8 strtmp2[30];
	memset(strtmp2, 0, sizeof(strtmp2));
	
	u8 daynum = 0;
	if(para->psSensorSch.nSchType == 0)
	{
		daynum = 7;
	}
	else if(para->psSensorSch.nSchType == 1)
	{
		daynum = 31;
	}
	else if(para->psSensorSch.nSchType == 2)
	{
		daynum = 1;
	}

	for(i = 0; i < daynum; i++)
	{		
		sprintf(strtmp2, "%s%d", CONFIG_SCH_PARAM_SENSOR, i);
		j = 0;
		while(1)
		{
			tmp = para->psSensorSch.nSchTime[i][j].nStartTime;
			if(0 != PublicIniSetArrayValue(pFilepath, 
				strtmp, 
				strtmp2, 
				EM_CONFIG_SCH_PARAM_STARTTIME + (2 * j),
				tmp))
			{
				break;
			}
			
			tmp = para->psSensorSch.nSchTime[i][j].nStopTime;
			if(0 != PublicIniSetArrayValue(pFilepath, 
				strtmp, 
				strtmp2, 
				EM_CONFIG_SCH_PARAM_ENDTIME + (2 * j),
				tmp))
			{
				break;
			}
			
			j++;
			if((j >= g_sConfigDvrProperty.nSchSegments) || (j >= CONFIG_MAX_SCH_TIME_SEGMENTS))
			{
				break;
			}
		}
	}
	
	sprintf(strtmp, "%s%d", CONFIG_SENSOR_PARAM_SENSOR, nId);
	tmp = para->psSensorSch.nSchType;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_SENSOR_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_SCHTYPE,
		tmp))
	{
		return -1;
	}
	
	return 0;
}
s32 ConfigSetIPCExtSensorSchParam(SModConfigSensorSch* para, s32 nId, s8* pFilepath)
{
	if((NULL == para) || (nId < 0) || (nId >= g_sConfigDvrProperty.nVidMainNum) || (NULL == pFilepath))
	{
		printf("Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}
	
	s32 tmp = 0;
	s32 i = nId;
	s32 j = 0;
	s8 strtmp[30];
	memset(strtmp, 0, sizeof(strtmp));
	sprintf(strtmp, "%s%d", CONFIG_SCH_PARAM, i);
	s8 strtmp2[30];
	memset(strtmp2, 0, sizeof(strtmp2));
	
	u8 daynum = 0;
	if(para->psSensorSch.nSchType == 0)
	{
		daynum = 7;
	}
	else if(para->psSensorSch.nSchType == 1)
	{
		daynum = 31;
	}
	else if(para->psSensorSch.nSchType == 2)
	{
		daynum = 1;
	}

	for(i = 0; i < daynum; i++)
	{		
		sprintf(strtmp2, "%s%d", CONFIG_SCH_PARAM_IPCEXTSENSOR, i);
		j = 0;
		while(1)
		{
			tmp = para->psSensorSch.nSchTime[i][j].nStartTime;
			if(0 != PublicIniSetArrayValue(pFilepath, 
				strtmp, 
				strtmp2, 
				EM_CONFIG_SCH_PARAM_STARTTIME + (2 * j),
				tmp))
			{
				break;
			}
			
			tmp = para->psSensorSch.nSchTime[i][j].nStopTime;
			if(0 != PublicIniSetArrayValue(pFilepath, 
				strtmp, 
				strtmp2, 
				EM_CONFIG_SCH_PARAM_ENDTIME + (2 * j),
				tmp))
			{
				break;
			}
			
			j++;
			if((j >= g_sConfigDvrProperty.nSchSegments) || (j >= CONFIG_MAX_SCH_TIME_SEGMENTS))
			{
				break;
			}
		}
	}
	
	sprintf(strtmp, "%s%d", CONFIG_SENSOR_PARAM_SENSOR, nId);
	tmp = para->psSensorSch.nSchType;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_IPCExtSENSOR_PARAM, 
		strtmp, 
		EM_CONFIG_SENSOR_PARAM_SCHTYPE,
		tmp))
	{
		return -1;
	}
	
	return 0;
}


s32 ConfigSetVMotionSchParam(SModConfigVMotionSch* para, s32 nId, s8* pFilepath)
{
	if((NULL == para) || (nId < 0) || (nId >= g_sConfigDvrProperty.nPreviewNum) || (NULL == pFilepath))
	{
		printf("Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}
	
	s32 tmp = 0;
	s32 i = nId;
	s32 j = 0;
	s8 strtmp[30];
	memset(strtmp, 0, sizeof(strtmp));
	sprintf(strtmp, "%s%d", CONFIG_SCH_PARAM, i);
	s8 strtmp2[30];
	memset(strtmp2, 0, sizeof(strtmp2));
	
	u8 daynum = 0;
	if(para->psVMSch.nSchType == 0)
	{
		daynum = 7;
	}
	else if(para->psVMSch.nSchType == 1)
	{
		daynum = 31;
	}
	else if(para->psVMSch.nSchType == 2)
	{
		daynum = 1;
	}

	for(i = 0; i < daynum; i++)
	{		
		sprintf(strtmp2, "%s%d", CONFIG_SCH_PARAM_VMOTION, i);
		j = 0;
		while(1)
		{
			tmp = para->psVMSch.nSchTime[i][j].nStartTime;
			if(0 != PublicIniSetArrayValue(pFilepath, 
				strtmp, 
				strtmp2, 
				EM_CONFIG_SCH_PARAM_STARTTIME + (2 * j),
				tmp))
			{
				break;
			}
			
			tmp = para->psVMSch.nSchTime[i][j].nStopTime;
			if(0 != PublicIniSetArrayValue(pFilepath, 
				strtmp, 
				strtmp2, 
				EM_CONFIG_SCH_PARAM_ENDTIME + (2 * j),
				tmp))
			{
				break;
			}
			
			j++;
			if((j >= g_sConfigDvrProperty.nSchSegments) || (j >= CONFIG_MAX_SCH_TIME_SEGMENTS))
			{
				break;
			}
		}
	}
	
	sprintf(strtmp, "%s%d", CONFIG_VMOTION_PARAM_VMOTION, nId);
	tmp = para->psVMSch.nSchType;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_VMOTION_PARAM, 
		strtmp, 
		EM_CONFIG_VMOTION_PARAM_SCHTYPE,
		tmp))
	{
		return -1;
	}
	
	return 0;
}

s32 ConfigSetAlarmOutSchParam(SModConfigAlarmOutSch* para, s32 nId, s8* pFilepath)
{
	if((NULL == para) || (nId < 0) || (nId >= g_sConfigDvrProperty.nAlarmoutNum) || (NULL == pFilepath))
	{
		printf("Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}
	
	s32 tmp = 0;
	s32 i = nId;
	s32 j = 0;
	s8 strtmp[30];
	memset(strtmp, 0, sizeof(strtmp));
	sprintf(strtmp, "%s%d", CONFIG_SCH_PARAM, i);
	s8 strtmp2[30];
	memset(strtmp2, 0, sizeof(strtmp2));
	
	u8 daynum = 0;
	if(para->psAlarmOutSch.nSchType == 0)
	{
		daynum = 7;
	}
	else if(para->psAlarmOutSch.nSchType == 1)
	{
		daynum = 31;
	}
	else if(para->psAlarmOutSch.nSchType == 2)
	{
		daynum = 1;
	}

	for(i = 0; i < daynum; i++)
	{		
		sprintf(strtmp2, "%s%d", CONFIG_SCH_PARAM_ALARMOUT, i);
		j = 0;
		while(1)
		{
			tmp = para->psAlarmOutSch.nSchTime[i][j].nStartTime;
			if(0 != PublicIniSetArrayValue(pFilepath, 
				strtmp, 
				strtmp2, 
				EM_CONFIG_SCH_PARAM_STARTTIME + (2 * j),
				tmp))
			{
				break;
			}
			
			tmp = para->psAlarmOutSch.nSchTime[i][j].nStopTime;
			if(0 != PublicIniSetArrayValue(pFilepath, 
				strtmp, 
				strtmp2, 
				EM_CONFIG_SCH_PARAM_ENDTIME + (2 * j),
				tmp))
			{
				break;
			}
			
			j++;
			if((j >= g_sConfigDvrProperty.nSchSegments) || (j >= CONFIG_MAX_SCH_TIME_SEGMENTS))
			{
				break;
			}
		}
	}
	
	sprintf(strtmp, "%s%d", CONFIG_ALARMOUT_PARAM_ALARMOUT, nId);
	tmp = para->psAlarmOutSch.nSchType;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_ALARMOUT_PARAM, 
		strtmp, 
		EM_CONFIG_ALARMOUT_PARAM_SCHTYPE,
		tmp))
	{
		return -1;
	}
	
	return 0;
}

s32 ConfigSetPtzParam(SModConfigPtzParam* para, s32 nId, s8* pFilepath)
{
	if((NULL == para) || (nId < 0) || (nId >= g_sConfigDvrProperty.nPreviewNum) || (NULL == pFilepath))
	{
		printf("Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}
	
	s32 tmp = 0;
	u64 tmp64 = 0;
	s32 i = nId;
	s8 strtmp[30];
	memset(strtmp, 0, sizeof(strtmp));
	sprintf(strtmp, "%s%d", CONFIG_PTZ_PARAM_PTZCFG, i);
	tmp = para->nEnable;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_PTZ_PARAM, 
		strtmp, 
		EM_CONFIG_PTZ_PARAM_ENABLE,
		tmp))
	{
		return -1;
	}
	
	tmp = para->nCamAddr;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_PTZ_PARAM, 
		strtmp, 
		EM_CONFIG_PTZ_PARAM_CAMADDR,
		tmp))
	{
		return -1;
	}
	
	tmp = para->nDataBit;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_PTZ_PARAM, 
		strtmp, 
		EM_CONFIG_PTZ_PARAM_DATABIT,
		tmp))
	{
		return -1;
	}
	
	tmp = para->nStopBit;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_PTZ_PARAM, 
		strtmp, 
		EM_CONFIG_PTZ_PARAM_STOPBIT,
		tmp))
	{
		return -1;
	}
	
	tmp = para->nBaudRate;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_PTZ_PARAM, 
		strtmp, 
		EM_CONFIG_PTZ_PARAM_BAUDRATE,
		tmp))
	{
		return -1;
	}
	
	tmp = para->nCheckType;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_PTZ_PARAM, 
		strtmp, 
		EM_CONFIG_PTZ_PARAM_CHECKTYPE,
		tmp))
	{
		return -1;
	}
	
	tmp = para->nFlowCtrlType;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_PTZ_PARAM, 
		strtmp, 
		EM_CONFIG_PTZ_PARAM_FLOWCTRLTYPE,
		tmp))
	{
		return -1;
	}
	
	if(0 != SetArrayStringToIni(pFilepath, 
		CONFIG_PTZ_PARAM, 
		strtmp, 
		EM_CONFIG_PTZ_PARAM_PROTOCOL,
		para->nProtocolName))
	{
		return -1;
	}
	
	tmp64 = para->sTourPresetPara.nPresetId[0];
	if(0 != PublicIniSetArrayValueU64(pFilepath, 
		CONFIG_PTZ_PARAM, 
		strtmp, 
		EM_CONFIG_PTZ_PARAM_PRESET,
		tmp64))
	{
		return -1;
	}
	
	tmp64 = para->sTourPresetPara.nPresetId[1];
	if(0 != PublicIniSetArrayValueU64(pFilepath, 
		CONFIG_PTZ_PARAM, 
		strtmp, 
		EM_CONFIG_PTZ_PARAM_PRESET2,
		tmp64))
	{
		return -1;
	}
	
	tmp64 = para->sTourPresetPara.nTourId[0];
	if(0 != PublicIniSetArrayValueU64(pFilepath, 
		CONFIG_PTZ_PARAM, 
		strtmp, 
		EM_CONFIG_PTZ_PARAM_TOURPATH,
		tmp64))
	{
		return -1;
	}
	
	tmp64 = para->sTourPresetPara.nTourId[1];
	if(0 != PublicIniSetArrayValueU64(pFilepath, 
		CONFIG_PTZ_PARAM, 
		strtmp, 
		EM_CONFIG_PTZ_PARAM_TOURPATH2,
		tmp64))
	{
		return -1;
	}
	
	s8 strtmp2[30];
	memset(strtmp, 0, sizeof(strtmp));
	memset(strtmp2, 0, sizeof(strtmp2));
	sprintf(strtmp, "%s%d", CONFIG_PTZ_TOUR_PARAM, nId);
	//for(i = 0; i < 2 * 64; i++)
	for(i = 0; i < 32; i++)
	{
		if((para->sTourPresetPara.nTourId[i/64] >> (i%64)) & 1)
		{
			sprintf(strtmp2, "%s%d", CONFIG_PTZ_TOUR_PARAM_TOURPATH, i);
			int j = 0;
			while(1)
			{
				if (1)//(para->sTourPresetPara.nTourPresetId[i][j].nPresetId>0)
				{
					#if 0
					if (1)//(para->sTourPresetPara.nTourPresetId[i][j].nPresetId>0)
					{
						printf("***j:[%d], pos:[%d], speed:[%d], time:[%d]\n", j,
							para->sTourPresetPara.nTourPresetId[i][j].nPresetId,
							para->sTourPresetPara.nTourPresetId[i][j].nSpeed,
							para->sTourPresetPara.nTourPresetId[i][j].nStayTime
						);
					}
					#endif

					tmp = para->sTourPresetPara.nTourPresetId[i][j].nPresetId;
					#if 1//add by Lirl on Nov/30/2011,是0也必须写入文件中
					if(0 == tmp)//cw_ptz
					{
						j++;
						if(j >= 128)
						{
							break;
						}
						continue;
					};
					#endif //end
					
					if(0 != PublicIniSetArrayValue(pFilepath, 
						strtmp, 
						strtmp2, 
						EM_CONFIG_PTZ_TOUR_PARAM_PRESETPOS + (j * 3),
						tmp))
					{
						break;
					}
					
					tmp = para->sTourPresetPara.nTourPresetId[i][j].nStayTime;
					if(0 != PublicIniSetArrayValue(pFilepath, 
						strtmp, 
						strtmp2, 
						EM_CONFIG_PTZ_TOUR_PARAM_DWELLTIME + (j * 3),
						tmp))
					{
						break;
					}
					
					tmp = para->sTourPresetPara.nTourPresetId[i][j].nSpeed;
					if(0 != PublicIniSetArrayValue(pFilepath, 
						strtmp, 
						strtmp2, 
						EM_CONFIG_PTZ_TOUR_PARAM_SPEED + (j * 3),
						tmp))
					{
						break;
					}
				}
				
				j++;
				if(j >= 128)
				{
					break;
				}
			}
		}
	}

	return 0;
}

s32 ConfigSetUserParam(SModConfigUserPara* para, s8* pFilepath)
{
	if((NULL == para) || (NULL == pFilepath))
	{
		printf("Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}	
	
	u64 tmp64 = 0;
	
	tmp64 = para->nFlagUserSetted[0];
	if(0 != PublicIniSetArrayValueU64(pFilepath, 
		CONFIG_USER_PARAM, 
		CONFIG_USER_PARAM_SETTED_USER, 
		0,
		tmp64))
	{
		return -1;
	}
	
	tmp64 = para->nFlagUserSetted[1];
	if(0 != PublicIniSetArrayValueU64(pFilepath, 
		CONFIG_USER_PARAM, 
		CONFIG_USER_PARAM_SETTED_USER, 
		1,
		tmp64))
	{
		return -1;
	}
	
	tmp64 = para->nFlagGroupSetted;
	if(0 != PublicIniSetValueU64(pFilepath, 
		CONFIG_USER_PARAM, 
		CONFIG_USER_PARAM_SETTED_GROUP, 
		tmp64))
	{
		return -1;
	}
	
	s32 tmp = 0;
	s32 i = 0;
	s8 strtmp[30];
	s32 j = 0;
	
	for(i = 0; i < MOD_CONFIG_MAX_USER_NUM; i++)
	{
		if((para->nFlagUserSetted[i/64] >> (i%64)) & 1)
		{			
			memset(strtmp, 0, sizeof(strtmp));
			sprintf(strtmp, "%s%d", CONFIG_USER_PARAM_USER, i);
			
			if(0 != SetArrayStringToIni(pFilepath, 
				CONFIG_USER_PARAM, 
				strtmp,
				EM_CONFIG_USER_PARAM_USER_NAME, 
				para->sUserPara[i].UserName))
			{
				return -1;
			}

			if(0 != SetArrayStringToIni(pFilepath, 
				CONFIG_USER_PARAM, 
				strtmp,
				EM_CONFIG_USER_PARAM_USER_PASSWD, 
				para->sUserPara[i].Password))
			{
				return -1;
			}

			if(0 != SetArrayStringToIni(pFilepath, 
				CONFIG_USER_PARAM, 
				strtmp,
				EM_CONFIG_USER_PARAM_USER_GROUPBELONG, 
				para->sUserPara[i].GroupBelong))
			{
				return -1;
			}

			if(0 != SetArrayStringToIni(pFilepath, 
				CONFIG_USER_PARAM, 
				strtmp,
				EM_CONFIG_USER_PARAM_USER_DESCRIPT, 
				para->sUserPara[i].UserDescript))
			{
				return -1;
			}

			tmp = para->sUserPara[i].emLoginTwince;
			if(0 != PublicIniSetArrayValue(pFilepath, 
				CONFIG_USER_PARAM, 
				strtmp, 
				EM_CONFIG_USER_PARAM_USER_LOGINTWINCE,
				tmp))
			{
				return -1;
			}
			
			tmp = para->sUserPara[i].emIsBindPcMac;
			if(0 != PublicIniSetArrayValue(pFilepath, 
				CONFIG_USER_PARAM, 
				strtmp, 
				EM_CONFIG_USER_PARAM_USER_BINDPCMAC,
				tmp))
			{
				return -1;
			}
			
			tmp64 = para->sUserPara[i].PcMacAddress;
			if(0 != PublicIniSetArrayValueU64(pFilepath, 
				CONFIG_USER_PARAM, 
				strtmp, 
				EM_CONFIG_USER_PARAM_USER_MACADDR,
				tmp64))
			{
				return -1;
			}
			
			for(j = 0; j < sizeof(para->sUserPara[i].UserAuthor.nAuthor); j++)
			{
				tmp = para->sUserPara[i].UserAuthor.nAuthor[j];
				if(0 != PublicIniSetArrayValue(pFilepath, 
					CONFIG_USER_PARAM, 
					strtmp, 
					EM_CONFIG_USER_PARAM_USER_AUTHOR + j,
					tmp))
				{
					return -1;
				}				
			}

			for(j = 0; j < 4; j++)
			{
				tmp64 = para->sUserPara[i].UserAuthor.nLiveView[j];
				if(0 != PublicIniSetArrayValueU64(pFilepath, 
					CONFIG_USER_PARAM, 
					strtmp, 
					EM_CONFIG_USER_PARAM_USER_AUTHOR + sizeof(para->sUserPara[i].UserAuthor.nAuthor) + j,
					tmp64))
				{
					return -1;
				}				
			}

			for(j = 0; j < 4; j++)
			{
				tmp64 = para->sUserPara[i].UserAuthor.nRemoteView[j];
				if(0 != PublicIniSetArrayValueU64(pFilepath, 
					CONFIG_USER_PARAM, 
					strtmp, 
					EM_CONFIG_USER_PARAM_USER_AUTHOR + sizeof(para->sUserPara[i].UserAuthor.nAuthor) + j + 4,
					tmp64))
				{
					return -1;
				}
			}

			for(j = 0; j < 4; j++)
			{
				tmp64 = para->sUserPara[i].UserAuthor.nRecord[j];
				if(0 != PublicIniSetArrayValueU64(pFilepath, 
					CONFIG_USER_PARAM, 
					strtmp, 
					EM_CONFIG_USER_PARAM_USER_AUTHOR + sizeof(para->sUserPara[i].UserAuthor.nAuthor) + j + 8,
					tmp64))
				{
					return -1;
				}
			}

			for(j = 0; j < 4; j++)
			{
				tmp64 = para->sUserPara[i].UserAuthor.nPlayback[j];
				if(0 != PublicIniSetArrayValueU64(pFilepath, 
					CONFIG_USER_PARAM, 
					strtmp, 
					EM_CONFIG_USER_PARAM_USER_AUTHOR + sizeof(para->sUserPara[i].UserAuthor.nAuthor) + j + 12,
					tmp64))
				{
					return -1;
				}
			}

			for(j = 0; j < 4; j++)
			{
				tmp64 = para->sUserPara[i].UserAuthor.nBackUp[j];
				if(0 != PublicIniSetArrayValueU64(pFilepath, 
					CONFIG_USER_PARAM, 
					strtmp, 
					EM_CONFIG_USER_PARAM_USER_AUTHOR + sizeof(para->sUserPara[i].UserAuthor.nAuthor) + j + 16,
					tmp64))
				{
					return -1;
				}		
			}

			for(j = 0; j < 4; j++)
			{
				tmp64 = para->sUserPara[i].UserAuthor.nPtzCtrl[j];
				if(0 != PublicIniSetArrayValueU64(pFilepath, 
					CONFIG_USER_PARAM, 
					strtmp, 
					EM_CONFIG_USER_PARAM_USER_AUTHOR + sizeof(para->sUserPara[i].UserAuthor.nAuthor) + j + 20,
					tmp64))
				{
					return -1;
				}				
			}			
		}
	}
	
	for(i = 0; i < MOD_CONFIG_MAX_GROUP_NUM; i++)
	{
		if((para->nFlagGroupSetted >> i) & 1)
		{
			memset(strtmp, 0, sizeof(strtmp));
			sprintf(strtmp, "%s%d", CONFIG_USER_PARAM_GROUP, i);
			
			if(0 != SetArrayStringToIni(pFilepath, 
				CONFIG_USER_PARAM, 
				strtmp,
				EM_CONFIG_USER_PARAM_GROUP_NAME, 
				para->sGroupPara[i].GroupName))
			{
				return -1;
			}

			if(0 != SetArrayStringToIni(pFilepath, 
				CONFIG_USER_PARAM, 
				strtmp,
				EM_CONFIG_USER_PARAM_GROUP_DESCRIPT, 
				para->sGroupPara[i].GroupDescript))
			{
				return -1;
			}

			tmp = para->sGroupPara[i].nDoWithUser;
			if(0 != PublicIniSetArrayValue(pFilepath, 
				CONFIG_USER_PARAM, 
				strtmp, 
				EM_CONFIG_USER_PARAM_GROUP_DOWITHUSER,
				tmp))
			{
				return -1;
			}
			
			for(j = 0; j < sizeof(para->sGroupPara[i].GroupAuthor.nAuthor); j++)
			{
				tmp = para->sGroupPara[i].GroupAuthor.nAuthor[j];
				if(0 != PublicIniSetArrayValue(pFilepath, 
					CONFIG_USER_PARAM, 
					strtmp, 
					EM_CONFIG_USER_PARAM_GROUP_AUTHOR + j,
					tmp))
				{
					return -1;
				}
			}

			for(j = 0; j < 4; j++)
			{
				tmp64 = para->sGroupPara[i].GroupAuthor.nLiveView[j];
				if(0 != PublicIniSetArrayValueU64(pFilepath, 
					CONFIG_USER_PARAM, 
					strtmp, 
					EM_CONFIG_USER_PARAM_GROUP_AUTHOR + sizeof(para->sGroupPara[i].GroupAuthor.nAuthor) + j,
					tmp64))
				{
					return -1;
				}
			}

			for(j = 0; j < 4; j++)
			{
				tmp64 = para->sGroupPara[i].GroupAuthor.nRemoteView[j];
				if(0 != PublicIniSetArrayValueU64(pFilepath, 
					CONFIG_USER_PARAM, 
					strtmp, 
					EM_CONFIG_USER_PARAM_GROUP_AUTHOR + sizeof(para->sGroupPara[i].GroupAuthor.nAuthor) + j + 4,
					tmp64))
				{
					return -1;
				}
			}

			for(j = 0; j < 4; j++)
			{
				tmp64 = para->sGroupPara[i].GroupAuthor.nRecord[j];
				if(0 != PublicIniSetArrayValueU64(pFilepath, 
					CONFIG_USER_PARAM, 
					strtmp, 
					EM_CONFIG_USER_PARAM_GROUP_AUTHOR + sizeof(para->sGroupPara[i].GroupAuthor.nAuthor) + j + 8,
					tmp64))
				{
					return -1;
				}
			}

			for(j = 0; j < 4; j++)
			{
				tmp64 = para->sGroupPara[i].GroupAuthor.nPlayback[j];
				if(0 != PublicIniSetArrayValueU64(pFilepath, 
					CONFIG_USER_PARAM, 
					strtmp, 
					EM_CONFIG_USER_PARAM_GROUP_AUTHOR + sizeof(para->sGroupPara[i].GroupAuthor.nAuthor) + j + 12,
					tmp64))
				{
					return -1;
				}
			}

			for(j = 0; j < 4; j++)
			{
				tmp64 = para->sGroupPara[i].GroupAuthor.nBackUp[j];
				if(0 != PublicIniSetArrayValueU64(pFilepath, 
					CONFIG_USER_PARAM, 
					strtmp, 
					EM_CONFIG_USER_PARAM_GROUP_AUTHOR + sizeof(para->sGroupPara[i].GroupAuthor.nAuthor) + j + 16,
					tmp64))
				{
					return -1;
				}
			}

			for(j = 0; j < 4; j++)
			{
				tmp64 = para->sGroupPara[i].GroupAuthor.nPtzCtrl[j];
				if(0 != PublicIniSetArrayValueU64(pFilepath, 
					CONFIG_USER_PARAM, 
					strtmp, 
					EM_CONFIG_USER_PARAM_GROUP_AUTHOR + sizeof(para->sGroupPara[i].GroupAuthor.nAuthor) + j + 20,
					tmp64))
				{
					return -1;
				}
			}			
		}
	}
	
	return 0;
}

//
s32 ConfigGetParamFromMem(EMCONFIGPARATYPE emType, void* para, s32 nId, SConfigAll* pAllPara)
{
	if(g_Inited == 0)
	{
		printf("Mod config has not been inited!\n");
		return -1;
	}
	if((para == NULL) || (pAllPara == NULL))
	{
		printf("Invalid para! function:%s\n", __FUNCTION__);
		return -1;
	}
	
	//printf("ConfigGetParamFromMem :%d\n", emType);
	
	switch(emType)
	{
		case EM_CONFIG_PARA_IMAGE:
		case EM_CONFIG_PARA_VO_IMAGE:
			if((nId < 0) || (nId > g_sConfigDvrProperty.nPreviewNum))
			{
				printf("Invalid para [ID]! function:%s\n", __FUNCTION__);
				return -1;
			}
			break;
		case EM_CONFIG_PARA_STR_OSD:
		case EM_CONFIG_PARA_RECT_OSD:
		case EM_CONFIG_PARA_VMOTION:
		case EM_CONFIG_PARA_VLOST:
		case EM_CONFIG_PARA_VBLIND:
		case EM_CONFIG_PARA_VMOTION_SCH:
		case EM_CONFIG_PARA_VLOST_SCH:
		case EM_CONFIG_PARA_VBLIND_SCH:
		case EM_CONFIG_PARA_PTZ:
		//case EM_CONFIG_PARA_NETWORK:
		case EM_CONFIG_PARA_IPCAMERA://NVR used
			if((nId < 0) || (nId >= g_sConfigDvrProperty.nPreviewNum))
			{
				printf("%d,Invalid para [%d]! function:%s,%d\n", emType,nId, __FUNCTION__,__LINE__);
				return -1;
			}	
			break;
		case EM_CONFIG_PARA_ENC_MAIN:
		//yaogang modify 20141010
		case EM_CONFIG_PARA_IPCCOVER:
		case EM_CONFIG_PARA_IPCEXTSENSOR:	
		case EM_CONFIG_PARA_485EXTSENSOR:
		case EM_CONFIG_PARA_IPCEXTSENSOR_SCH:
		//yaogang modify 20150105
		case EM_CONFIG_PARA_SNAP_CHN:
			if((nId < 0) || (nId >= g_sConfigDvrProperty.nVidMainNum))
			{
				printf("%d,Invalid para [%d]! function:%s,%d\n", emType,nId, __FUNCTION__,__LINE__);
				return -1;
			}	
			break;
		case EM_CONFIG_PARA_ENC_SUB:
			if((nId < 0) || (nId >= g_sConfigDvrProperty.nVidSubNum))
			{
				printf("%d,Invalid para [%d]! function:%s,%d\n", emType,nId, __FUNCTION__,__LINE__);
				return -1;
			}	
			break;
		case EM_CONFIG_PARA_ENC_MOB:
			if((nId < 0) || (nId >= g_sConfigDvrProperty.nVidSubNum))
			{
				printf("%d,Invalid para [%d]! function:%s,%d\n", emType,nId, __FUNCTION__,__LINE__);
				return -1;
			}	
			break;
			
		case EM_CONFIG_PARA_PATROL:
		case EM_CONFIG_PARA_RECORD:
		case EM_CONFIG_PARA_TIMER_REC_SCH:
		case EM_CONFIG_PARA_SENSOR_REC_SCH:
		case EM_CONFIG_PARA_VMOTION_REC_SCH:
		case EM_CONFIG_PARA_VLOST_REC_SCH:
		case EM_CONFIG_PARA_VBLIND_REC_SCH:
			if((nId < 0) || (nId >= g_sConfigDvrProperty.nRecNum))
			{
				//printf("nId:%d, RecNum:%d\n", nId, g_sConfigDvrProperty.nVidMainNum);
				printf("%d,Invalid para [%d]! function:%s,%d\n", emType,nId, __FUNCTION__,__LINE__);
				return -1;
			}	
			break;
		case EM_CONFIG_PARA_SENSOR:
		case EM_CONFIG_PARA_SENSOR_SCH:
			if((nId < 0) || (nId >= g_sConfigDvrProperty.nSensorNum))
			{
				printf("%d,Invalid para [%d]! function:%s,%d\n", emType,nId, __FUNCTION__,__LINE__);
				return -1;
			}	
			break;
		case EM_CONFIG_PARA_ALARMOUT:
		case EM_CONFIG_PARA_ALARMOUT_SCH:
			if((nId < 0) || (nId >= g_sConfigDvrProperty.nAlarmoutNum))
			{
				printf("%d,Invalid para [%d]! function:%s,%d\n", emType,nId, __FUNCTION__,__LINE__);
				return -1;
			}	
			break;
		case EM_CONFIG_PARA_BUZZ:
		case EM_CONFIG_PARA_BUZZ_SCH:
			if((nId < 0) || (nId >= g_sConfigDvrProperty.nBuzzNum))
			{
				printf("%d,Invalid para [%d]! function:%s,%d\n", emType,nId, __FUNCTION__,__LINE__);
				return -1;
			}	
			break;
		default:
			break;
	}
	
	//printf("ConfigGetParamFromMem :%d\n", __LINE__);
	
	sem_wait(&pAllPara->sem);
	
	s32 ret = 0;
	
	switch(emType)
	{
		case EM_CONFIG_PARA_DVR_PROPERTY:
			memcpy(para, &g_sConfigDvrProperty, sizeof(SModConfigDvrProperty));
			break;
			
		case EM_CONFIG_PARA_SYSTEM:
			memcpy(para, pAllPara->psSystemPara, sizeof(SModConfigSystemParam));
			break;
		case EM_CONFIG_PARA_MAINTAIN:
			memcpy(para, pAllPara->psMainTainPara, sizeof(SModConfigMainTainParam));
			break;
		case EM_CONFIG_PARA_PATROL:
		case EM_CONFIG_PARA_PREVIEW:
			memcpy(para, pAllPara->psPreviewPara, sizeof(SModConfigPreviewParam));
			break;
			
		case EM_CONFIG_PARA_IMAGE:
			memcpy(para, &pAllPara->psImagePara[nId], sizeof(SModConfigImagePara));
			break;

		case EM_CONFIG_PARA_VO_IMAGE:
			memcpy(para, &pAllPara->psVoImagePara[nId], sizeof(SModConfigImagePara));
			break;
			
		case EM_CONFIG_PARA_MAIN_OSD:
			memcpy(para, pAllPara->psMainOsdPara, sizeof(SModConfigMainOsdParam));
			break;
			
		case EM_CONFIG_PARA_STR_OSD:
			memcpy(para, &pAllPara->psStrOsdPara[nId], sizeof(SModConfigStrOsdpara));
			break;
			
		case EM_CONFIG_PARA_RECT_OSD:
			memcpy(para, &pAllPara->psRectOsdPara[nId], sizeof(SModConfigRectOsdpara));
			break;
			
		case EM_CONFIG_PARA_ENC_MAIN:
			memcpy(para, &pAllPara->psVideoMainPara[nId], sizeof(SModConfigVideoParam));
			break;
			
		case EM_CONFIG_PARA_ENC_SUB:
			memcpy(para, &pAllPara->psVideoSubPara[nId], sizeof(SModConfigVideoParam));
			break;
		case EM_CONFIG_PARA_ENC_MOB:
			memcpy(para, &pAllPara->psVideoMobPara[nId], sizeof(SModConfigVideoParam));
			break;
			
		case EM_CONFIG_PARA_RECORD:
			memcpy(para, &pAllPara->psRecordPara[nId], sizeof(SModConfigRecordParam));
			break;
			
		case EM_CONFIG_PARA_TIMER_REC_SCH:
			memcpy(para, &pAllPara->psRecTimerSchPara[nId], sizeof(SModConfigRecTimerSch));
			break;
			
		case EM_CONFIG_PARA_SENSOR_REC_SCH:
			memcpy(para, &pAllPara->psRecSensorSchPara[nId], sizeof(SModConfigRecAlarmInSch));
			break;
			
		case EM_CONFIG_PARA_VMOTION_REC_SCH:
			memcpy(para, &pAllPara->psRecVMSchPara[nId], sizeof(SModConfigRecVMSch));
			break;
			
		case EM_CONFIG_PARA_VLOST_REC_SCH:
		case EM_CONFIG_PARA_VBLIND_REC_SCH:
			break;

		case EM_CONFIG_PARA_SENSOR:
			memcpy(para, &pAllPara->psSensorPara[nId], sizeof(SModConfigSensorParam));
			break;
		//yaogang modify 20141010
		case EM_CONFIG_PARA_IPCCOVER:
			//if (nId == 0)
				//printf("yg ConfigGetParamFromMem chn0\n");
			memcpy(para, &pAllPara->psIPCCoverPara[nId], sizeof(SModConfigIPCExtSensorParam));
			break;
		case EM_CONFIG_PARA_IPCEXTSENSOR:	
			//if (nId == 0)
				//printf("yg ConfigGetParamFromMem chn0\n");
			memcpy(para, &pAllPara->psIPCExtSensorPara[nId], sizeof(SModConfigIPCExtSensorParam));
			break;
		case EM_CONFIG_PARA_485EXTSENSOR:
			//if (nId == 0)
				//printf("yg ConfigGetParamFromMem chn0\n");
			memcpy(para, &pAllPara->ps485ExtSensorPara[nId], sizeof(SModConfigIPCExtSensorParam));
			break;
		case EM_CONFIG_PARA_HDD:	
			
			memcpy(para, pAllPara->psHDDPara, sizeof(SModConfigHDDParam));
			break;
			
		case EM_CONFIG_PARA_VMOTION:
			memcpy(para, &pAllPara->psVMotionPara[nId], sizeof(SModConfigVMotionParam));
			break;
			
		case EM_CONFIG_PARA_VLOST:
			memcpy(para, &pAllPara->psVLostPara[nId], sizeof(SModConfigVLostParam));
			break;
			
		case EM_CONFIG_PARA_VBLIND:
			memcpy(para, &pAllPara->psVBlindPara[nId], sizeof(SModConfigVBlindParam));
			break;
			
		case EM_CONFIG_PARA_ALARMOUT:
			memcpy(para, &pAllPara->psAlarmOutPara[nId], sizeof(SModConfigAlarmOutParam));
			break;
			
		case EM_CONFIG_PARA_BUZZ:
			memcpy(para, &pAllPara->psBuzzPara[nId], sizeof(SModConfigBuzzParam));
			break;
			
		case EM_CONFIG_PARA_SENSOR_SCH:
			memcpy(para, &pAllPara->psSensorSchPara[nId], sizeof(SModConfigSensorSch));
			break;
		case EM_CONFIG_PARA_IPCEXTSENSOR_SCH:
			memcpy(para, &pAllPara->psIPCExtSensorSchPara[nId], sizeof(SModConfigSensorSch));
			break;
		case EM_CONFIG_PARA_VMOTION_SCH:
			memcpy(para, &pAllPara->psVMotionSchPara[nId], sizeof(SModConfigVMotionSch));
			break;
			
		case EM_CONFIG_PARA_ALARMOUT_SCH:
			memcpy(para, &pAllPara->psAlarmOutSchPara[nId], sizeof(SModConfigAlarmOutSch));
			break;
			
		case EM_CONFIG_PARA_VLOST_SCH:
		case EM_CONFIG_PARA_VBLIND_SCH:
		case EM_CONFIG_PARA_BUZZ_SCH:
			break;
			
		case EM_CONFIG_PARA_PTZ:
			memcpy(para, &pAllPara->psPtzPara[nId], sizeof(SModConfigPtzParam));
			break;
			
		case EM_CONFIG_PARA_USER:
			memcpy(para, pAllPara->psUserPara, sizeof(SModConfigUserPara));
			break;
			
		case EM_CONFIG_PARA_NETWORK:
			memcpy(para, pAllPara->psNetPara, sizeof(SModConfigNetParam));
			//printf("ConfigGetParamFromMem :%d\n", __LINE__);
			break;
			
		//NVR used
		case EM_CONFIG_PARA_IPCAMERA:
			memcpy(para, &pAllPara->psIPCameraPara[nId], sizeof(SModConfigIPCameraParam));
			break;
		//yaogang modify 20150105
		case EM_CONFIG_PARA_SNAP_CHN:
			memcpy(para, &pAllPara->psSnapPara[nId], sizeof(SModConfigSnapChnPara));
			break;
		//yaogang modify 20141209
		//深广平台参数	
		case EM_CONFIG_PARA_SG:
			memcpy(para, pAllPara->psSGPara, sizeof(SModConfigSGParam));
			break;
		case EM_CONFIG_PARA_SG_DAILY:
			memcpy(para, pAllPara->psSGDailyPara, sizeof(SModDailyPicCFG));
			break;
		case EM_CONFIG_PARA_SG_ALARM:
			memcpy(para, &pAllPara->psSGAlarmPara[nId], sizeof(SModAlarmPicCFG));
			break;
		//yaogang modify for bad disk
		case EM_CONFIG_BAD_DISK:
			memcpy(para, &pAllPara->psBadDiskListPara[nId], sizeof(SModConfigBadDisk));
			break;
		default:
			printf("%s Error: Invalid para type: %d!\n", __FUNCTION__, emType);
			ret = -1;
			break;
	}
	
	//printf("ConfigGetParamFromMem :%d\n", __LINE__);
	
	sem_post(&pAllPara->sem);
	
	return ret;
}

u32 inet_addr_user(char *cp)
{
	char chSeps[8] = ".";
	char *pchToken;
	u32 value = 0;
	u32 code = 0;
	
	pchToken = strtok(cp,chSeps);
	if(pchToken == NULL) return 0;
	code = (u8)atoi(pchToken);
	value += code;
	
	pchToken = strtok(NULL,chSeps);
	if(pchToken == NULL) return 0;
	code = (u8)atoi(pchToken);
	value += (code<<8);
	
	pchToken = strtok(NULL,chSeps);
	if(pchToken == NULL) return 0;
	code = (u8)atoi(pchToken);
	value += (code<<16);
	
	pchToken = strtok(NULL,chSeps);
	if(pchToken == NULL) return 0;
	code = (u8)atoi(pchToken);
	value += (code<<24);
	
	//printf("ip value:0x%x\n",value);
	
	return value;
}

static u8 g_LanguageList[30][30] = 
							{
								"&CfgPtn.LangEng",
								"&CfgPtn.LangSch",
								"&CfgPtn.LangTch",
								"&CfgPtn.LangHan",
								"&CfgPtn.LangJap",
								"&CfgPtn.LangGer",
								"&CfgPtn.LangRus",
								"&CfgPtn.LangFre",
								"&CfgPtn.LangPor",
								"&CfgPtn.LangTur",
								"&CfgPtn.LangSpa",
								"&CfgPtn.LangIta",
								"&CfgPtn.LangPol",
								//csp modify 20121224
								"&CfgPtn.LangAra",//"&CfgPtn.LangFarsi",
								"&CfgPtn.LangThai",
								"&CfgPtn.LangHun",
								"&CfgPtn.LangSlovakia",
								"&CfgPtn.LangVietnam",
								"&CfgPtn.LangGreece",
								//lcy add
								"&CfgPtn.LangHeb",
								//
							};  //注意:需与EM_CUSTOM_TRANSLATE_ENG顺序一致

s32 EnableCustomConfig(EMCUSTOMTYPE emType)
{
	s32 i, nCount;
	SCustomInfo sCustomInfo;
	SCustomData sCustomData;

	if (0 == CustomOpen(NULL))
	{
		nCount = CustomGetCount();
		for (i = 0; i < nCount; i++)
		{
			CustomGetInfo(i, &sCustomInfo);

			if (EM_CUSTOM_ALL != emType && emType !=  sCustomInfo.emType)
			{
				continue;
			}

			if(EM_CUSTOM_DEVICE_IP == sCustomInfo.emType)
			{
				if (0 == CustomGetData(i, &sCustomData))
				{
					SModConfigNetParam sPIns;						
					u32 	tmpIP = inet_addr_user(sCustomData.strData);//保存ip
					ModConfigGetParam( EM_CONFIG_PARA_NETWORK, &sPIns, 0);
					sPIns.HostIP = tmpIP;
					ModConfigSetParam( EM_CONFIG_PARA_NETWORK, &sPIns, 0);
					printf("客户定制IP成功!\n");
				}
			}
			else if(EM_CUSTOM_DEVICE_MAC == sCustomInfo.emType)
			{
				if (0 == CustomGetData(i, &sCustomData)) //保存MAC
				{
					SModConfigNetParam sPIns;
					ModConfigGetParam( EM_CONFIG_PARA_NETWORK, &sPIns, 0);
					memcpy(sPIns.MAC, sCustomData.strData, sizeof(sPIns.MAC));
					ModConfigSetParam( EM_CONFIG_PARA_NETWORK, &sPIns, 0);
					printf("客户定制MAC成功! %s 111\n", sPIns.MAC);
					bCustomMac = 1;
				}
			}
			else if(EM_CUSTOM_DEVICE_GATEWAY == sCustomInfo.emType)
			{
				if (0 == CustomGetData(i, &sCustomData))
				{
					SModConfigNetParam sPIns;						
					u32 	tmpIP = inet_addr_user(sCustomData.strData);//保存网关ip
					ModConfigGetParam( EM_CONFIG_PARA_NETWORK, &sPIns, 0);
					sPIns.GateWayIP = tmpIP;
					ModConfigSetParam( EM_CONFIG_PARA_NETWORK, &sPIns, 0);
					printf("客户定制网关成功!\n");
				}
			}
			else if(EM_CUSTOM_DEVICE_VIDEOSTAND == sCustomInfo.emType)
			{
				if (0 == CustomGetData(i, &sCustomData)) //保存视频制式
				{
					SModConfigSystemParam sPIns;
					ModConfigGetParam(EM_CONFIG_PARA_SYSTEM, &sPIns, 0);
					
					//sPIns.nVideoStandard = sCustomData.nVideoStandard;					
					//ModConfigSetParam(EM_CONFIG_PARA_SYSTEM, &sPIns, 0);
					//printf("客户定制视频制式成功!\n");
					
					printf("***UserConf: %d, Custom: %d\n", sPIns.nVideoStandard, sCustomData.nVideoStandard);
					
					//csp modify 20130322
					if(sCustomData.nVideoStandard == EM_PAL || sCustomData.nVideoStandard == EM_NTSC)
					{
						sPIns.nVideoStandard = sCustomData.nVideoStandard;
						ModConfigSetParam(EM_CONFIG_PARA_SYSTEM, &sPIns, 0);
						printf("客户定制视频制式成功!\n");
					}
					
					//Modify by Lirl on Feb/09/2012,恢复出产设置时N/P制的帧率做相应的改变
					if(1)// (sPIns.nVideoStandard != sCustomData.nVideoStandard)
					{
						//csp modify 20130322
						//sPIns.nVideoStandard = sCustomData.nVideoStandard;
						//ModConfigSetParam(EM_CONFIG_PARA_SYSTEM, &sPIns, 0);
						//printf("客户定制视频制式成功!\n");
						
						SModConfigVideoParam sVideoMainParam;
						int i = 0;
						
						//csp modify 20130322
						//if (EM_NTSC == (EMVIDSTANDARD)sCustomData.nVideoStandard)
						if(EM_NTSC == (EMVIDSTANDARD)sPIns.nVideoStandard)
						{
							for(i = 0; i < g_sConfigDvrProperty.nVidMainNum; i++)
							{
								ModConfigGetParam(EM_CONFIG_PARA_ENC_MAIN, &sVideoMainParam, i);
								//if(sVideoMainParam.nFrameRate >= 25)//csp modify 20130416
								{
									sVideoMainParam.nFrameRate = 30;
									ModConfigSetParam(EM_CONFIG_PARA_ENC_MAIN, &sVideoMainParam, i);
								}
							}
						}
						//csp modify 20130322
						//else if (EM_PAL == (EMVIDSTANDARD)sCustomData.nVideoStandard)
						else if(EM_PAL == (EMVIDSTANDARD)sPIns.nVideoStandard)
						{
							for(i = 0; i < g_sConfigDvrProperty.nVidMainNum; i++)
							{
								ModConfigGetParam(EM_CONFIG_PARA_ENC_MAIN, &sVideoMainParam, i);
								//if(sVideoMainParam.nFrameRate > 25)//csp modify 20130416
								{
									sVideoMainParam.nFrameRate = 25;
									ModConfigSetParam(EM_CONFIG_PARA_ENC_MAIN, &sVideoMainParam, i);
								}
							}
						}
					}
					//end
				}
			}
			else if((EM_CUSTOM_VIDENC_MAIN_BASE <= sCustomInfo.emType)
					&& (EM_CUSTOM_VIDENC_MAIN_TOP >= sCustomInfo.emType))
			{
				if (0 == CustomGetData(i, &sCustomData)) //保存主码流编码参数
				{
					SModConfigVideoParam sPIns;
					sPIns.nBitRate = sCustomData.sVidEncParam.nBitRate;
					sPIns.nFrameRate = sCustomData.sVidEncParam.nFrameRate;
					sPIns.nResolution = sCustomData.sVidEncParam.nVideoResolution;
					sPIns.nEncodeType = sCustomData.sVidEncParam.nEncodeType;
					sPIns.nBitRateType = sCustomData.sVidEncParam.nCbrOrVbr;
					sPIns.nPicLevel = sCustomData.sVidEncParam.nPicLevel;
					sPIns.nGop = sCustomData.sVidEncParam.nGop;
					sPIns.nMaxQP = sCustomData.sVidEncParam.nMaxQP;
					sPIns.nMinQP = sCustomData.sVidEncParam.nMinQP;	
					u8 chn = 0;
					for(chn = sCustomData.sVidEncParam.nChnStart-1; chn < sCustomData.sVidEncParam.nChnEnd; chn++)
					{
						if(chn < g_sConfigDvrProperty.nVidMainNum)
						{
							ModConfigSetParam( EM_CONFIG_PARA_ENC_MAIN, &sPIns, chn);
						}
					}
					printf("客户定制主码流编码参数成功!\n");
				}
			}
			else if((EM_CUSTOM_VIDENC_SUB_BASE <= sCustomInfo.emType)
					&& (EM_CUSTOM_VIDENC_SUB_TOP >= sCustomInfo.emType))
			{
				if (0 == CustomGetData(i, &sCustomData)) //保存子码流编码参数
				{
					SModConfigVideoParam sPIns;
					sPIns.nBitRate = sCustomData.sVidEncParam.nBitRate;
					sPIns.nFrameRate = sCustomData.sVidEncParam.nFrameRate;
					sPIns.nResolution = sCustomData.sVidEncParam.nVideoResolution;
					sPIns.nEncodeType = sCustomData.sVidEncParam.nEncodeType;
					sPIns.nBitRateType = sCustomData.sVidEncParam.nCbrOrVbr;
					sPIns.nPicLevel = sCustomData.sVidEncParam.nPicLevel;
					sPIns.nGop = sCustomData.sVidEncParam.nGop;
					sPIns.nMaxQP = sCustomData.sVidEncParam.nMaxQP;
					sPIns.nMinQP = sCustomData.sVidEncParam.nMinQP;	
					u8 chn = 0;
					for(chn = sCustomData.sVidEncParam.nChnStart-1; chn < sCustomData.sVidEncParam.nChnEnd; chn++)
					{
						if(chn < g_sConfigDvrProperty.nVidSubNum)
						{
							ModConfigSetParam( EM_CONFIG_PARA_ENC_SUB, &sPIns, chn);
						}
					}
					printf("客户定制子码流编码参数成功!\n");
				}
			}
			else if((EM_CUSTOM_PREVIEW_COLOR_BASE <= sCustomInfo.emType)
					&& (EM_CUSTOM_PREVIEW_COLOR_TOP >= sCustomInfo.emType))
			{
				if (0 == CustomGetData(i, &sCustomData)) //保存预览色彩参数
				{
					SModConfigImagePara sPIns;
					sPIns.nHue = sCustomData.sVidPreviewColor.nHue;
					sPIns.nSaturation = sCustomData.sVidPreviewColor.nSaturation;
					sPIns.nContrast = sCustomData.sVidPreviewColor.nContrast;
					sPIns.nBrightness = sCustomData.sVidPreviewColor.nBrightness;
					u8 chn = 0;
					for(chn = sCustomData.sVidPreviewColor.nChnStart-1; chn < sCustomData.sVidPreviewColor.nChnEnd; chn++)
					{
						if(chn < g_sConfigDvrProperty.nPreviewNum)
						{
							ModConfigSetParam( EM_CONFIG_PARA_IMAGE, &sPIns, chn);
						}
					}
					printf("客户定制预览图像参数成功!\n");
				}
			}
			else if(EM_CUSTOM_DEVICE_LANGUAGE == sCustomInfo.emType)
			{
				if (0 != CustomGetData(i, &sCustomData)) //保存语言
				{
					continue;
				}
				if(EM_CUSTOM_ALL == emType)
				{
					SModConfigSystemParam sPIns;
					ModConfigGetParam(EM_CONFIG_PARA_SYSTEM, &sPIns, 0);
					
					printf("配置文件语言[%d],客户定制语言[%d]\n",sPIns.nLangId,sCustomData.sLanguageParam.nLanguageIdDefault);
					
					sPIns.nLangId = sCustomData.sLanguageParam.nLanguageIdDefault;
					
					ModConfigSetParam(EM_CONFIG_PARA_SYSTEM, &sPIns, 0);
					
					printf("客户定制语言[%d]成功!\n",sPIns.nLangId);
				}
				else
				{
					g_ConfigStrList[EM_CONFIG_PARALIST_LANGSUP].nItem = sCustomData.sLanguageParam.nLanguageIdNum;
					g_ConfigStrList[EM_CONFIG_PARALIST_LANGSUP].strList = (u8**)realloc(g_ConfigStrList[EM_CONFIG_PARALIST_LANGSUP].strList, sizeof(u8*)*sCustomData.sLanguageParam.nLanguageIdNum);
					g_ConfigAllList[EM_CONFIG_PARALIST_LANGSUP].nNum = sCustomData.sLanguageParam.nLanguageIdNum;
					g_ConfigAllList[EM_CONFIG_PARALIST_LANGSUP].pConfigValList = (s32**)realloc(g_ConfigAllList[EM_CONFIG_PARALIST_LANGSUP].pConfigValList, sizeof(s32*)*sCustomData.sLanguageParam.nLanguageIdNum);
					u8 i = 0;
					for(i = 0; i < sCustomData.sLanguageParam.nLanguageIdNum; i++)
					{
						g_ConfigStrList[EM_CONFIG_PARALIST_LANGSUP].strList[i] = (u8*)calloc(30, sizeof(u8));
						strcpy(g_ConfigStrList[EM_CONFIG_PARALIST_LANGSUP].strList[i], g_LanguageList[sCustomData.sLanguageParam.nLanguageIdList[i]]);
						g_ConfigAllList[EM_CONFIG_PARALIST_LANGSUP].pConfigValList[i] = (s32*)calloc(1, sizeof(s32));
						*(g_ConfigAllList[EM_CONFIG_PARALIST_LANGSUP].pConfigValList[i]) = sCustomData.sLanguageParam.nLanguageIdList[i];
						printf("language:(%d,%s)\n",*(g_ConfigAllList[EM_CONFIG_PARALIST_LANGSUP].pConfigValList[i]), g_ConfigStrList[EM_CONFIG_PARALIST_LANGSUP].strList[i]);
					}
				}
			}
			else
			{
				//ssss
			}
		}
		CustomClose(0);
	}

	return 0;
}

s32 SaveCustomDefaultConfig(EMCUSTOMTYPE emType)
{
	s32 i, nCount;
	SCustomInfo sCustomInfo;
	SCustomData sCustomData;
	
	if (0 == CustomOpen(NULL))
	{
		nCount = CustomGetCount();
		for (i = 0; i < nCount; i++)
		{
			CustomGetInfo(i, &sCustomInfo);

			if (EM_CUSTOM_ALL != emType && emType != sCustomInfo.emType)
			{
				continue;
			}

			if(EM_CUSTOM_DEVICE_IP == sCustomInfo.emType)
			{
				if (0 == CustomGetData(i, &sCustomData))
				{
					u32 	tmpIP = inet_addr_user(sCustomData.strData);//保存ip
					g_ConfigDefaultAllPara.psNetPara->HostIP = tmpIP;
					printf("客户定制IP设置默认值成功!\n");
				}
			}
			else if(EM_CUSTOM_DEVICE_MAC == sCustomInfo.emType)
			{
				if (0 == CustomGetData(i, &sCustomData)) //保存MAC
				{
					strcpy(g_ConfigDefaultAllPara.psNetPara->MAC, sCustomData.strData);
					printf("客户定制MAC设置默认值成功!\n");
				}
			}
			else if(EM_CUSTOM_DEVICE_GATEWAY == sCustomInfo.emType)
			{
				if (0 == CustomGetData(i, &sCustomData))
				{
					u32 	tmpIP = inet_addr_user(sCustomData.strData);//保存网关ip
					g_ConfigDefaultAllPara.psNetPara->GateWayIP = tmpIP;
					printf("客户定制网关设置默认值成功!\n");
				}
			}
			else if(EM_CUSTOM_DEVICE_VIDEOSTAND == sCustomInfo.emType)
			{
				if (0 == CustomGetData(i, &sCustomData)) //保存视频制式
				{
					printf("视频制式: DefaultConf: %d, Custom: %d\n", g_ConfigDefaultAllPara.psSystemPara->nVideoStandard, sCustomData.nVideoStandard);
					
					//csp modify 20130322
					if(sCustomData.nVideoStandard == EM_PAL || sCustomData.nVideoStandard == EM_NTSC)
					{
						g_ConfigDefaultAllPara.psSystemPara->nVideoStandard = sCustomData.nVideoStandard;
						printf("客户定制视频制式设置默认值成功!\n");
					}
				}
			}
			else if((EM_CUSTOM_VIDENC_MAIN_BASE <= sCustomInfo.emType)
					&& (EM_CUSTOM_VIDENC_MAIN_TOP >= sCustomInfo.emType))
			{
				if (0 == CustomGetData(i, &sCustomData)) //保存主码流编码参数
				{
					SModConfigVideoParam sPIns;
					sPIns.nBitRate = sCustomData.sVidEncParam.nBitRate;
					sPIns.nFrameRate = sCustomData.sVidEncParam.nFrameRate;
					sPIns.nResolution = sCustomData.sVidEncParam.nVideoResolution;
					sPIns.nEncodeType = sCustomData.sVidEncParam.nEncodeType;
					sPIns.nBitRateType = sCustomData.sVidEncParam.nCbrOrVbr;
					sPIns.nPicLevel = sCustomData.sVidEncParam.nPicLevel;
					sPIns.nGop = sCustomData.sVidEncParam.nGop;
					sPIns.nMaxQP = sCustomData.sVidEncParam.nMaxQP;
					sPIns.nMinQP = sCustomData.sVidEncParam.nMinQP;	
					u8 chn = 0;
					for(chn = sCustomData.sVidEncParam.nChnStart-1; chn < sCustomData.sVidEncParam.nChnEnd; chn++)
					{
						if(chn < g_sConfigDvrProperty.nPreviewNum)
						{
							memcpy(&g_ConfigDefaultAllPara.psVideoMainPara[chn], &sPIns, sizeof(SModConfigVideoParam));
						}
					}
					printf("客户定制主码流编码参数设置默认值成功!\n");
				}
			}
			else if((EM_CUSTOM_VIDENC_SUB_BASE <= sCustomInfo.emType)
					&& (EM_CUSTOM_VIDENC_SUB_TOP >= sCustomInfo.emType))
			{
				if (0 == CustomGetData(i, &sCustomData)) //保存子码流编码参数
				{
					SModConfigVideoParam sPIns;
					sPIns.nBitRate = sCustomData.sVidEncParam.nBitRate;
					sPIns.nFrameRate = sCustomData.sVidEncParam.nFrameRate;
					sPIns.nResolution = sCustomData.sVidEncParam.nVideoResolution;
					sPIns.nEncodeType = sCustomData.sVidEncParam.nEncodeType;
					sPIns.nBitRateType = sCustomData.sVidEncParam.nCbrOrVbr;
					sPIns.nPicLevel = sCustomData.sVidEncParam.nPicLevel;
					sPIns.nGop = sCustomData.sVidEncParam.nGop;
					sPIns.nMaxQP = sCustomData.sVidEncParam.nMaxQP;
					sPIns.nMinQP = sCustomData.sVidEncParam.nMinQP;	
					u8 chn = 0;
					for(chn = sCustomData.sVidEncParam.nChnStart-1; chn < sCustomData.sVidEncParam.nChnEnd; chn++)
					{
						if(chn < g_sConfigDvrProperty.nPreviewNum)
						{
							memcpy(&g_ConfigDefaultAllPara.psVideoSubPara[chn], &sPIns, sizeof(SModConfigVideoParam));
						}
					}
					printf("客户定制子码流编码参数设置默认值成功!\n");
				}
			}
			else if((EM_CUSTOM_PREVIEW_COLOR_BASE <= sCustomInfo.emType)
					&& (EM_CUSTOM_PREVIEW_COLOR_TOP >= sCustomInfo.emType))
			{
				if (0 == CustomGetData(i, &sCustomData)) //保存预览色彩参数
				{
					SModConfigImagePara sPIns;
					sPIns.nHue = sCustomData.sVidPreviewColor.nHue;
					sPIns.nSaturation = sCustomData.sVidPreviewColor.nSaturation;
					sPIns.nContrast = sCustomData.sVidPreviewColor.nContrast;
					sPIns.nBrightness = sCustomData.sVidPreviewColor.nBrightness;
					u8 chn = 0;
					for(chn = sCustomData.sVidPreviewColor.nChnStart-1; chn < sCustomData.sVidPreviewColor.nChnEnd; chn++)
					{
						if(chn < g_sConfigDvrProperty.nPreviewNum)
						{
							memcpy(&g_ConfigDefaultAllPara.psImagePara[chn], &sPIns, sizeof(SModConfigImagePara));
						}
					}
					printf("客户定制预览图像参数成功!\n");
				}
			}
			else if(EM_CUSTOM_DEVICE_LANGUAGE == sCustomInfo.emType)
			{
				if (0 != CustomGetData(i, &sCustomData)) //保存语言
				{
					continue;
				}
				g_ConfigDefaultAllPara.psSystemPara->nLangId = sCustomData.sLanguageParam.nLanguageIdDefault;
				printf("客户定制语言设置默认值[%d]成功!\n", g_ConfigDefaultAllPara.psSystemPara->nLangId);
			}
			else
			{
				//
			}
		}
		CustomClose(0);
	}
	
	return 0;
}

void GetCustomAppLogo(void)
{
	s32 i, nCount;
	SCustomInfo sCustomInfo;
	//SCustomData sCustomData;
	
	if(0 == CustomOpen(NULL))
	{
		nCount = CustomGetCount();
		//printf("count = %d\n", nCount);
		for(i = 0; i < nCount; i++)
		{
			CustomGetInfo(i, &sCustomInfo);
			
			if(sCustomInfo.emType == EM_CUSTOM_LOGO_APP)
			{
				printf("has logo !!!!\n");
				u8 tmp[sCustomInfo.nLen];
				u32 nRealLen = 0;
				if (0 <  CustomGetLogo(i, sizeof(tmp), tmp, &nRealLen)) //保存logo
				{
					//printf("nRealLen = %d\n",nRealLen);
					FILE* fp = NULL;
					fp = fopen("/tmp2/logo.jpg", "w");
					if(fp)
					{
						fwrite(tmp, nRealLen, 1, fp);
						fclose(fp);
					}
					break;
				}
			}
		}
		
		CustomClose(0);
	}
}

//csp modify 20121224
static u8 g_TranslateFileName[30][20] = 
{
	"English",
	"Chinese",
	"TChinese",
	"Korea",
	"Japanese",
	"German",
	"Russian",
	"France",
	"Portuguese",
	"Turkey",
	"Spanish",
	"Italian",
	"Polish",
	"Persian",
	"Thai",
	"Magyar",
	"Slovakia",
	"Vietnam",
	"Greece",
	"Hebrew",//lcy add   希伯来语
	//
};  //注意:需与EM_CUSTOM_TRANSLATE_ENG顺序一致
void GetCustomFontTransFile(void)
{
	s32 i, nCount;
	SCustomInfo sCustomInfo;
	//SCustomData sCustomData;
	
	if (0 == CustomOpen(NULL))
	{
		nCount = CustomGetCount();
		//printf("CustomGetCount = %d\n", nCount);
		
		u8 pFileName[200] = {0};
		
		for (i = 0; i < nCount; i++)
		{
			CustomGetInfo(i, &sCustomInfo);
			//printf("CustomGetInfo emType=%d\n",sCustomInfo.emType);
			
			if(sCustomInfo.emType == EM_CUSTOM_DEVICE_GETFONT)
			{
				sprintf(pFileName, "/tmp/ui/data/Data/Font.bin");
				//printf("GetCustomFontTransFile:EM_CUSTOM_DEVICE_GETFONT,FileName=%s\n",pFileName);
				//continue;
			}
			else if((sCustomInfo.emType >= EM_CUSTOM_TRANSLATE_BASE)
					&& (sCustomInfo.emType <= EM_CUSTOM_TRANSLATE_TOP))
			{
				sprintf(pFileName, "/tmp/ui/data/Data/Strings%s.txt", g_TranslateFileName[sCustomInfo.emType - EM_CUSTOM_TRANSLATE_BASE]);
				//printf("GetCustomFontTransFile:language=%d,FileName=%s\n",sCustomInfo.emType-EM_CUSTOM_TRANSLATE_BASE,pFileName);
			}
			else
			{
				continue;
			}
			//printf("%d\n",i);
			if(0 <  CustomGetFile(i, pFileName))//保存
			{
				printf("%s-%s\n", __func__, pFileName);
			}
			memset(pFileName, 0, sizeof(pFileName));
		}
		CustomClose(0);
	}
}

void GetCustomPanelFile(void)
{
	s32 i, nCount;
	SCustomInfo sCustomInfo;
	
	if (0 == CustomOpen(NULL))
	{
		nCount = CustomGetCount();
		
		for (i = 0; i < nCount; i++)
		{
			CustomGetInfo(i, &sCustomInfo);
			
			if (sCustomInfo.emType != EM_CUSTOM_PANEL_REMOTE_CTRL)
			{
				continue;
			}
			
			//csp modify
			/*if (0 <  CustomGetFile(i, "/mnt/panel.ini")) //保存
			{
				printf("Debug : Get panel file ok!\n");
				//break;
			}*/
			
			if (0 <  CustomGetFile(i, "/tmp2/panel.ini")) //保存
			{
				printf("Get panel file ok!\n");
				break;
			}
		}
		CustomClose(0);
	}
}

#define _RZ_P2P_

#ifdef _RZ_P2P_
#define MSG_HEAD_LEN	3
#define MSG_DATA_LEN	120
#define MSG_TAIL_LEN	3

//P2P参数储存在flash的最后一个区域
//头部四字节为"XDC",尾部两字节为"P2P"
typedef struct
{
	unsigned char head[MSG_HEAD_LEN];
	unsigned char registered;
	unsigned char enable;
	char mac[18];
	char sn[33];
	unsigned char reserved[68];
	unsigned char verify;
	unsigned char tail[MSG_TAIL_LEN];
}P2PParam;

const unsigned char g_msg_head[] = {'X', 'D', 'C'};
const unsigned char g_msg_tail[] = {'P', '2', 'P'};

static BOOL InitP2PParam(P2PParam *para)
{
	if(para == NULL)
	{
		return FALSE;
	}
	
	memset(para, 0, sizeof(P2PParam));
	
	memcpy(para->head, g_msg_head, sizeof(g_msg_head));
	memcpy(para->tail, g_msg_tail, sizeof(g_msg_tail));
	
	para->registered = 'N';
	para->enable = 'N';
	strcpy(para->sn, "rzxxxx0000000000");
	strcpy(para->mac, "00:00:00:00:00:00");
	
	unsigned int temp = 0;
	unsigned char *p = para->head + sizeof(para->head);
	while(p < &para->verify)
	{
		temp += *p++;
	}
	
	para->verify = (temp % 256);
	
	return TRUE;
}

static BOOL VerifyP2PParam(P2PParam *para)
{
	if(para == NULL)
	{
		return FALSE;
	}
	
	if(memcmp(para->head, g_msg_head, sizeof(g_msg_head)) || memcmp(para->tail, g_msg_tail, sizeof(g_msg_tail)))
	{
		return FALSE;
	}
	
	unsigned int temp = 0;
	unsigned char *p = para->head + sizeof(para->head);
	while(p < &para->verify)
	{
		temp += *p++;
	}
	
	if(para->verify != (temp % 256))
	{
		return FALSE;
	}
	
	return TRUE;
}

static BOOL GetP2PParam(P2PParam *para)
{
	if(para == NULL)
	{
		return FALSE;
	}
	
	int fd = open("/dev/mtd/6", O_RDWR);
	if(fd == -1)
	{
		printf("GetP2PParam:unable to open mtd6\n");
		InitP2PParam(para);
		return FALSE;
	}
	
	lseek(fd, 0, 0);
	read(fd, para, sizeof(P2PParam));
	close(fd);
	
	if(!VerifyP2PParam(para))
	{
		printf("GetP2PParam:verify failed\n");
		InitP2PParam(para);
		return FALSE;
	}
	
	return TRUE;
}
#endif

/*************************以下提供给业务层接口********************/
s32 ModConfigInit(void)
{
	if(g_Inited)
	{
		printf("Mod config has been inited!\n");
		return -1;
	}
	
	//printf("before GetCustomFontTransFile\n");
	GetCustomFontTransFile();//字库和翻译文件
	
	//printf("before GetCustomPanelFile\n");
	GetCustomPanelFile();//前面板键值表
	
	if(0 != InitConfigInfo())
	{
		PUBPRT("HERE ERR");
		printf("HERE ERR\n");
		return -1;
	}
	if(0 != InitDvrProperty())
	{
		PUBPRT("HERE ERR");
		return -1;
	}	
	if(0 != ConfigLoadAllParaToMem(&g_ConfigAllPara, g_sConfigDevInfo.nFilePath))
	{
		PUBPRT("HERE ERR");
		return -1;
	}
	
	if(0 != sem_init(&g_FileSem, 0, 1))
	{
		PUBPRT("HERE ERR");
		return -1;
	}	
	if(0 != ConfigCreateMsgQueue())
	{
		fprintf(stderr, "ConfigCreateMsgQueue error! function: %s\n", __FUNCTION__);
		return -1;
	}
	
	g_Inited = 1;
	
	int i;
	
	s32 nValueList[MAX_LIST_LEN], *pnValueList[MAX_LIST_LEN];
	u8 StrList[MAX_LIST_LEN][MAX_STR_LEN], *pnStrList[MAX_LIST_LEN], len;
	
	memset(g_ConfigAllList, 0, sizeof(g_ConfigAllList));
	memset(g_ConfigStrList, 0, sizeof(g_ConfigStrList));
	
	for (i = 0; i < MAX_LIST_LEN; i++)
	{
		pnValueList[i] = &nValueList[i];
		pnStrList[i] = StrList[i];//pnStrList[i] = &StrList[i];//csp modify
	}
	
	for (i = 0; i < EM_CONFIG_PARALIST_NUM; i++)
	{
		ModConfigGetParaStr(i, pnStrList, &len, MAX_LIST_LEN, MAX_STR_LEN);
		
		ModConfigGetParaListIndex(i, 0, &len);
	}
	
	EnableCustomConfig(EM_CUSTOM_DEVICE_LANGUAGE);
	
#ifdef _RZ_P2P_
	P2PParam p2ppara;
	GetP2PParam(&p2ppara);
#endif
	
	pthread_t pid;
	#if 0
	if(0 != pthread_create(&pid, NULL, (void*)ConfigLoadDefaultToMemFxn, NULL))
	{
		g_Inited = 0;
		return -1;
	}
	#else
	if(0 != ConfigLoadAllParaToMem(&g_ConfigDefaultAllPara, g_sConfigDevInfo.nDefaultConfPath))
	{
		g_Inited = 0;
		return -1;
	}
	else
	{
		g_BeAbleToDefault = 1;
		SaveCustomDefaultConfig(EM_CUSTOM_ALL);
		//printf("ModConfigInit:Load default config to memory OK!\n");
		
		#ifdef _RZ_P2P_
		if(p2ppara.registered == 'Y')
		{
			strcpy(g_ConfigDefaultAllPara.psNetPara->MAC, p2ppara.mac);
		}
		#endif
	}
	#endif
	
	if(0 != pthread_create(&pid, NULL, (void*)ConfigManagerFxn, NULL))
	{
		g_Inited = 0;
		return -1;
	}
	
	//printf("###ConfigManagerFxn\n");
	
	if(0 == g_sConfigDevInfo.nConfSaveType)
	{
		if(0 != pthread_create(&pid, NULL, (void*)WriteConfigFileToFlashFxn, NULL))
		{
			g_Inited = 0;
			return -1;
		}
	}
	
	if(2 == g_sConfigDevInfo.nReadInfoSuccess)
	{
		EnableCustomConfig(EM_CUSTOM_ALL);
		
		#ifdef _RZ_P2P_
		if(p2ppara.registered == 'Y')
		{
			SModConfigNetParam sPIns;
			ModConfigGetParam( EM_CONFIG_PARA_NETWORK, &sPIns, 0);
			memcpy(sPIns.MAC, p2ppara.mac, sizeof(sPIns.MAC));
			ModConfigSetParam( EM_CONFIG_PARA_NETWORK, &sPIns, 0);
			bCustomMac = 1;
		}
		#endif
		
		g_sConfigDevInfo.nReadInfoSuccess = 1;
	}
	
	GetCustomAppLogo();//logo
	
	//GetCustomFontTransFile();//字库和翻译文件
	
	#if 0//#ifdef _RZ_P2P_
	//if(p2ppara.registered == 'Y')
	if(1)
	{
		SModConfigSystemParam sPIns;
		ModConfigGetParam(EM_CONFIG_PARA_SYSTEM, &sPIns, 0);
		//if(sPIns.P2PEnable)
		if(1)
		{
			static u8 p2p_start_flag = 0;
			if(!p2p_start_flag)
			{
				printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$start vv p2p......\n");
				system("/home/killp2p");
				system("/home/p2p");
				p2p_start_flag = 1;
			}
		}
	}
	#endif
	
	printf("^.^ %s OK !\n", __FUNCTION__);
	
	return 0;
}

s32 ModConfigDeinit(void)
{
	if(g_Inited == 0)
	{
		printf("Mod config has not been inited!\n");
		return -1;
	}
	
	free(g_ConfigAllPara.psSystemPara);
	free(g_ConfigAllPara.psPreviewPara);
	free(g_ConfigAllPara.psMainTainPara);//csp modify
	free(g_ConfigAllPara.psImagePara);
	free(g_ConfigAllPara.psVoImagePara);//csp modify
	free(g_ConfigAllPara.psMainOsdPara);
	free(g_ConfigAllPara.psStrOsdPara);
	free(g_ConfigAllPara.psRectOsdPara);
	free(g_ConfigAllPara.psVideoMainPara);
	free(g_ConfigAllPara.psVideoSubPara);
	free(g_ConfigAllPara.psVideoMobPara);
	free(g_ConfigAllPara.psRecordPara);
	free(g_ConfigAllPara.psRecTimerSchPara);
	free(g_ConfigAllPara.psRecVMSchPara);
	free(g_ConfigAllPara.psRecSensorSchPara);
	free(g_ConfigAllPara.psSensorPara);
	//yaogang modify 20141010
	free(g_ConfigAllPara.psIPCCoverPara);
	free(g_ConfigAllPara.psIPCExtSensorPara);
	free(g_ConfigAllPara.ps485ExtSensorPara);
	free(g_ConfigAllPara.psHDDPara);
	
	free(g_ConfigAllPara.psVMotionPara);
	free(g_ConfigAllPara.psVLostPara);
	free(g_ConfigAllPara.psVBlindPara);
	free(g_ConfigAllPara.psAlarmOutPara);
	free(g_ConfigAllPara.psBuzzPara);
	free(g_ConfigAllPara.psSensorSchPara);
	free(g_ConfigAllPara.psIPCExtSensorSchPara);
	free(g_ConfigAllPara.psVMotionSchPara);
	free(g_ConfigAllPara.psAlarmOutSchPara);
	free(g_ConfigAllPara.psPtzPara);
	free(g_ConfigAllPara.psUserPara);//csp modify
	free(g_ConfigAllPara.psNetPara);
	free(g_ConfigAllPara.psIPCameraPara);//NVR used
	free(g_ConfigAllPara.psSnapPara);
	//yaogang modify 20141209 深广平台参数
	free(g_ConfigAllPara.psSGPara);
	free(g_ConfigAllPara.psSGDailyPara);
	free(g_ConfigAllPara.psSGAlarmPara);

	//yaogang modify for bad disk psBadDiskListPara
	free(g_ConfigAllPara.psBadDiskListPara);
	
	sem_destroy(&g_ConfigAllPara.sem);
	
	if(g_BeAbleToDefault)
	{
		free(g_ConfigDefaultAllPara.psSystemPara);
		free(g_ConfigDefaultAllPara.psPreviewPara);
		free(g_ConfigDefaultAllPara.psMainTainPara);//csp modify
		free(g_ConfigDefaultAllPara.psImagePara);
		free(g_ConfigDefaultAllPara.psVoImagePara);//csp modify
		free(g_ConfigDefaultAllPara.psMainOsdPara);
		free(g_ConfigDefaultAllPara.psStrOsdPara);
		free(g_ConfigDefaultAllPara.psRectOsdPara);
		free(g_ConfigDefaultAllPara.psVideoMainPara);
		free(g_ConfigDefaultAllPara.psVideoSubPara);
		free(g_ConfigDefaultAllPara.psVideoMobPara);
		free(g_ConfigDefaultAllPara.psRecordPara);
		free(g_ConfigDefaultAllPara.psRecTimerSchPara);
		free(g_ConfigDefaultAllPara.psRecVMSchPara);
		free(g_ConfigDefaultAllPara.psRecSensorSchPara);
		free(g_ConfigDefaultAllPara.psSensorPara);
		//yaogang modify 20141010
		free(g_ConfigDefaultAllPara.psIPCCoverPara);
		free(g_ConfigDefaultAllPara.psIPCExtSensorPara);
		free(g_ConfigDefaultAllPara.ps485ExtSensorPara);
		free(g_ConfigDefaultAllPara.psHDDPara);
		
		free(g_ConfigDefaultAllPara.psVMotionPara);
		free(g_ConfigDefaultAllPara.psVLostPara);
		free(g_ConfigDefaultAllPara.psVBlindPara);
		free(g_ConfigDefaultAllPara.psAlarmOutPara);
		free(g_ConfigDefaultAllPara.psBuzzPara);
		free(g_ConfigDefaultAllPara.psSensorSchPara);
		free(g_ConfigDefaultAllPara.psIPCExtSensorSchPara);
		free(g_ConfigDefaultAllPara.psVMotionSchPara);
		free(g_ConfigDefaultAllPara.psAlarmOutSchPara);
		free(g_ConfigDefaultAllPara.psPtzPara);
		free(g_ConfigDefaultAllPara.psUserPara);//csp modify
		free(g_ConfigDefaultAllPara.psNetPara);
		free(g_ConfigDefaultAllPara.psIPCameraPara);//NVR used
		free(g_ConfigDefaultAllPara.psSnapPara);
		//yaogang modify 20141209 深广平台参数
		free(g_ConfigDefaultAllPara.psSGPara);
		free(g_ConfigDefaultAllPara.psSGDailyPara);
		free(g_ConfigDefaultAllPara.psSGAlarmPara);
		//yaogang modify for bad disk
		free(g_ConfigDefaultAllPara.psBadDiskListPara);
		
		sem_destroy(&g_ConfigDefaultAllPara.sem);
		
		g_BeAbleToDefault = 0;//csp modify
	}
	
	if(0 != ConfigDestroyMsgQueue())
	{
		fprintf(stderr, "ConfigDestroyMsgQueue error! function: %s\n", __FUNCTION__);
	}
	
	g_Inited = 0;
	
	sleep(2);
	
	printf("^.^ %s OK !\n", __FUNCTION__);
	
	return 0;
}

s32 ModDefaultConfigRelease(void)
{
	if(g_BeAbleToDefault)
	{
		free(g_ConfigDefaultAllPara.psSystemPara);
		free(g_ConfigDefaultAllPara.psPreviewPara);
		free(g_ConfigDefaultAllPara.psMainTainPara);//csp modify
		free(g_ConfigDefaultAllPara.psImagePara);
		free(g_ConfigDefaultAllPara.psVoImagePara);//csp modify
		free(g_ConfigDefaultAllPara.psMainOsdPara);
		free(g_ConfigDefaultAllPara.psStrOsdPara);
		free(g_ConfigDefaultAllPara.psRectOsdPara);
		free(g_ConfigDefaultAllPara.psVideoMainPara);
		free(g_ConfigDefaultAllPara.psVideoSubPara);
		free(g_ConfigDefaultAllPara.psVideoMobPara);
		free(g_ConfigDefaultAllPara.psRecordPara);
		free(g_ConfigDefaultAllPara.psRecTimerSchPara);
		free(g_ConfigDefaultAllPara.psRecVMSchPara);
		free(g_ConfigDefaultAllPara.psRecSensorSchPara);
		free(g_ConfigDefaultAllPara.psSensorPara);
		//yaogang modify 20141010
		free(g_ConfigDefaultAllPara.psIPCCoverPara);
		free(g_ConfigDefaultAllPara.psIPCExtSensorPara);
		free(g_ConfigDefaultAllPara.ps485ExtSensorPara);
		free(g_ConfigDefaultAllPara.psHDDPara);
		
		free(g_ConfigDefaultAllPara.psVMotionPara);
		free(g_ConfigDefaultAllPara.psVLostPara);
		free(g_ConfigDefaultAllPara.psVBlindPara);
		free(g_ConfigDefaultAllPara.psAlarmOutPara);
		free(g_ConfigDefaultAllPara.psBuzzPara);
		free(g_ConfigDefaultAllPara.psSensorSchPara);
		free(g_ConfigDefaultAllPara.psIPCExtSensorSchPara);
		free(g_ConfigDefaultAllPara.psVMotionSchPara);
		free(g_ConfigDefaultAllPara.psAlarmOutSchPara);
		free(g_ConfigDefaultAllPara.psPtzPara);
		free(g_ConfigDefaultAllPara.psUserPara);//csp modify
		free(g_ConfigDefaultAllPara.psNetPara);
		free(g_ConfigDefaultAllPara.psIPCameraPara);//NVR used
		free(g_ConfigDefaultAllPara.psSnapPara);
		//yaogang modify 20141209 深广平台参数
		free(g_ConfigDefaultAllPara.psSGPara);
		free(g_ConfigDefaultAllPara.psSGDailyPara);
		free(g_ConfigDefaultAllPara.psSGAlarmPara);

		//yaogang modify for bad disk
		free(g_ConfigDefaultAllPara.psBadDiskListPara);
		
		sem_destroy(&g_ConfigDefaultAllPara.sem);
		
		g_BeAbleToDefault = 0;//csp modify
	}
	
	sleep(2);
	
	printf("^.^ %s OK!\n", __FUNCTION__);
	
	return 0;
}

s32 ModDefaultConfigResume(void)
{
	if(0 != ConfigLoadAllParaToMem(&g_ConfigDefaultAllPara, g_sConfigDevInfo.nDefaultConfPath))
	{
		g_Inited = 0;
		return -1;
	}
	else
	{
		g_BeAbleToDefault = 1;
		SaveCustomDefaultConfig(EM_CUSTOM_ALL);
		printf("ModDefaultConfigResume:Load default config to memory OK!\n");
	}
	printf("^.^ %s OK !\n", __FUNCTION__);
	return 0;
}

s32 ModConfigGetParam(EMCONFIGPARATYPE emType, void* para, s32 nId)
{
	s32 ret = ConfigGetParamFromMem(emType, para, nId, &g_ConfigAllPara);
	
	return ret;
}

s32 ModConfigSetParam(EMCONFIGPARATYPE emType, void* para, s32 nId)
{
	if(g_Inited == 0)
	{
		printf("Mod config has not been inited!\n");
		return -1;
	}
	if(para == NULL)
	{
		printf("Invalid para! function:%s\n", __FUNCTION__);
		return -1;
	}
	//printf("yg ModConfigSetParam1: %d, 0x%x\n", emType, emType);
	switch(emType)
	{
		case EM_CONFIG_PARA_IMAGE:
		case EM_CONFIG_PARA_VO_IMAGE:
		case EM_CONFIG_PARA_STR_OSD:
		case EM_CONFIG_PARA_RECT_OSD:
		case EM_CONFIG_PARA_VMOTION:
		case EM_CONFIG_PARA_VLOST:
		case EM_CONFIG_PARA_VBLIND:
		case EM_CONFIG_PARA_VMOTION_SCH:
		case EM_CONFIG_PARA_VLOST_SCH:
		case EM_CONFIG_PARA_VBLIND_SCH:
		case EM_CONFIG_PARA_PTZ:
		case EM_CONFIG_PARA_IPCAMERA://NVR used
			if((nId < 0) || (nId >= g_sConfigDvrProperty.nPreviewNum))
			{
				printf("Invalid para [ID]! function:%s\n", __FUNCTION__);
				return -1;
			}	
			break;
			
		case EM_CONFIG_PARA_ENC_MAIN:
		//yaogang modify 20141010
		case EM_CONFIG_PARA_IPCCOVER:
		case EM_CONFIG_PARA_IPCEXTSENSOR:
		case EM_CONFIG_PARA_485EXTSENSOR:
		case EM_CONFIG_PARA_IPCEXTSENSOR_SCH:
		case EM_CONFIG_PARA_SNAP_CHN:
			if((nId < 0) || (nId >= g_sConfigDvrProperty.nVidMainNum))
			{
				printf("Invalid para [ID]! function:%s\n", __FUNCTION__);
				return -1;
			}	
			break;
			
		case EM_CONFIG_PARA_ENC_SUB:
			if((nId < 0) || (nId >= g_sConfigDvrProperty.nVidSubNum))
			{
				printf("Invalid para [ID]! function:%s\n", __FUNCTION__);
				return -1;
			}	
			break;
		case EM_CONFIG_PARA_ENC_MOB:
			if((nId < 0) || (nId >= g_sConfigDvrProperty.nVidSubNum))
			{
				printf("Invalid para [ID]! function:%s\n", __FUNCTION__);
				return -1;
			}	
			break;
			
		case EM_CONFIG_PARA_PATROL:
		case EM_CONFIG_PARA_RECORD:
		case EM_CONFIG_PARA_TIMER_REC_SCH:
		case EM_CONFIG_PARA_SENSOR_REC_SCH:
		case EM_CONFIG_PARA_VMOTION_REC_SCH:
		case EM_CONFIG_PARA_VLOST_REC_SCH:
		case EM_CONFIG_PARA_VBLIND_REC_SCH:
			if((nId < 0) || (nId >= g_sConfigDvrProperty.nRecNum))
			{
				printf("Invalid para [ID]! function:%s\n", __FUNCTION__);
				return -1;
			}	
			break;
			
		case EM_CONFIG_PARA_SENSOR:
		case EM_CONFIG_PARA_SENSOR_SCH:
			if((nId < 0) || (nId >= g_sConfigDvrProperty.nSensorNum))
			{
				printf("Invalid para [ID]! function:%s\n", __FUNCTION__);
				return -1;
			}	
			break;
			
		case EM_CONFIG_PARA_ALARMOUT:
		case EM_CONFIG_PARA_ALARMOUT_SCH:
			if((nId < 0) || (nId >= g_sConfigDvrProperty.nAlarmoutNum))
			{
				printf("Invalid para [ID]! function:%s\n", __FUNCTION__);
				return -1;
			}	
			break;
			
		case EM_CONFIG_PARA_BUZZ:
		case EM_CONFIG_PARA_BUZZ_SCH:
			if((nId < 0) || (nId >= g_sConfigDvrProperty.nBuzzNum))
			{
				printf("Invalid para [ID]! function:%s\n", __FUNCTION__);
				return -1;
			}
			break;
			
		default:
			break;
	}
	
	#ifdef CONFIG_TEST
	return 0;
	#endif
	
	SConfigMsgHeader head;
	memset(&head, 0, sizeof(SConfigMsgHeader));
	head.emType = emType;
	head.nId = nId;
	
	sem_wait(&g_ConfigAllPara.sem);
	
	s32 ret = 0;
	//printf("yg ModConfigSetParam2: %d, 0x%x\n", emType, emType);
	switch(emType)
	{
		case EM_CONFIG_PARA_SYSTEM:
			memcpy(g_ConfigAllPara.psSystemPara, para, sizeof(SModConfigSystemParam));
			head.pData = g_ConfigAllPara.psSystemPara;//calloc(1, sizeof(SModConfigSystemParam));
			//memcpy(head.pData, para, sizeof(SModConfigSystemParam));
			break;
		case EM_CONFIG_PARA_MAINTAIN:
			memcpy(g_ConfigAllPara.psMainTainPara, para, sizeof(SModConfigMainTainParam));
			head.pData = g_ConfigAllPara.psMainTainPara;
			break;
		case EM_CONFIG_PARA_PATROL:	
		case EM_CONFIG_PARA_PREVIEW:
			memcpy(g_ConfigAllPara.psPreviewPara, para, sizeof(SModConfigPreviewParam));
			head.pData = g_ConfigAllPara.psPreviewPara;//calloc(1, sizeof(SModConfigPreviewParam));
			//memcpy(head.pData, para, sizeof(SModConfigPreviewParam));
			break;
		case EM_CONFIG_PARA_IMAGE:
			memcpy(&g_ConfigAllPara.psImagePara[nId], para, sizeof(SModConfigImagePara));
			head.pData = &g_ConfigAllPara.psImagePara[nId];//calloc(1, sizeof(SModConfigImagePara));
			//memcpy(head.pData, para, sizeof(SModConfigImagePara));
			break;
		case EM_CONFIG_PARA_VO_IMAGE:
			memcpy(&g_ConfigAllPara.psVoImagePara[nId], para, sizeof(SModConfigImagePara));
			head.pData = &g_ConfigAllPara.psVoImagePara[nId];//calloc(1, sizeof(SModConfigImagePara));
			//memcpy(head.pData, para, sizeof(SModConfigImagePara));
			break;
		case EM_CONFIG_PARA_MAIN_OSD:
			memcpy(g_ConfigAllPara.psMainOsdPara, para, sizeof(SModConfigMainOsdParam));
			head.pData = g_ConfigAllPara.psMainOsdPara;//calloc(1, sizeof(SModConfigMainOsdParam));
			//memcpy(head.pData, para, sizeof(SModConfigMainOsdParam));
			break;
		case EM_CONFIG_PARA_STR_OSD:
			memcpy(&g_ConfigAllPara.psStrOsdPara[nId], para, sizeof(SModConfigStrOsdpara));
			head.pData = &g_ConfigAllPara.psStrOsdPara[nId];//calloc(1, sizeof(SModConfigStrOsdpara));
			//memcpy(head.pData, para, sizeof(SModConfigStrOsdpara));
			break;
		case EM_CONFIG_PARA_RECT_OSD:
			memcpy(&g_ConfigAllPara.psRectOsdPara[nId], para, sizeof(SModConfigRectOsdpara));
			head.pData = &g_ConfigAllPara.psRectOsdPara[nId];//calloc(1, sizeof(SModConfigRectOsdpara));
			//memcpy(head.pData, para, sizeof(SModConfigRectOsdpara));
			break;
		case EM_CONFIG_PARA_ENC_MAIN:
			memcpy(&g_ConfigAllPara.psVideoMainPara[nId], para, sizeof(SModConfigVideoParam));
			head.pData = &g_ConfigAllPara.psVideoMainPara[nId];//calloc(1, sizeof(SModConfigVideoParam));
			//memcpy(head.pData, para, sizeof(SModConfigVideoParam));
			break;
		case EM_CONFIG_PARA_ENC_SUB:
			memcpy(&g_ConfigAllPara.psVideoSubPara[nId], para, sizeof(SModConfigVideoParam));
			head.pData = &g_ConfigAllPara.psVideoSubPara[nId];//calloc(1, sizeof(SModConfigVideoParam));
			//memcpy(head.pData, para, sizeof(SModConfigVideoParam));
			break;
		case EM_CONFIG_PARA_ENC_MOB:
			memcpy(&g_ConfigAllPara.psVideoMobPara[nId], para, sizeof(SModConfigVideoParam));
			head.pData = &g_ConfigAllPara.psVideoMobPara[nId];//calloc(1, sizeof(SModConfigVideoParam));
			//memcpy(head.pData, para, sizeof(SModConfigVideoParam));
			break;
		case EM_CONFIG_PARA_RECORD:
			//printf("nId = %d ,para->bRecording = %d",nId,((SModConfigRecordParam*)para)->bRecording);
			memcpy(&g_ConfigAllPara.psRecordPara[nId], para, sizeof(SModConfigRecordParam));
			head.pData = &g_ConfigAllPara.psRecordPara[nId];//calloc(1, sizeof(SModConfigRecordParam));
			//memcpy(head.pData, para, sizeof(SModConfigRecordParam));
			break;
		case EM_CONFIG_PARA_TIMER_REC_SCH:
			memcpy(&g_ConfigAllPara.psRecTimerSchPara[nId], para, sizeof(SModConfigRecTimerSch));
			head.pData = &g_ConfigAllPara.psRecTimerSchPara[nId];//calloc(1, sizeof(SModConfigRecTimerSch));
			//memcpy(head.pData, para, sizeof(SModConfigRecTimerSch));
			break;
		case EM_CONFIG_PARA_SENSOR_REC_SCH:
			memcpy(&g_ConfigAllPara.psRecSensorSchPara[nId], para, sizeof(SModConfigRecAlarmInSch));
			head.pData = &g_ConfigAllPara.psRecSensorSchPara[nId];//calloc(1, sizeof(SModConfigRecAlarmInSch));
			//memcpy(head.pData, para, sizeof(SModConfigRecAlarmInSch));
			break;
		case EM_CONFIG_PARA_VMOTION_REC_SCH:
			memcpy(&g_ConfigAllPara.psRecVMSchPara[nId], para, sizeof(SModConfigRecVMSch));
			head.pData = &g_ConfigAllPara.psRecVMSchPara[nId];//calloc(1, sizeof(SModConfigRecVMSch));
			//memcpy(head.pData, para, sizeof(SModConfigRecVMSch));
			break;
		case EM_CONFIG_PARA_VLOST_REC_SCH:
		case EM_CONFIG_PARA_VBLIND_REC_SCH:
			break;
		case EM_CONFIG_PARA_SENSOR:
			memcpy(&g_ConfigAllPara.psSensorPara[nId], para, sizeof(SModConfigSensorParam));
			head.pData = &g_ConfigAllPara.psSensorPara[nId];//calloc(1, sizeof(SModConfigSensorParam));
			//memcpy(head.pData, para, sizeof(SModConfigSensorParam));
			break;
		//yaogang modify 20141010
		case EM_CONFIG_PARA_IPCCOVER:
			memcpy(&g_ConfigAllPara.psIPCCoverPara[nId], para, sizeof(SModConfigIPCExtSensorParam));
			head.pData = &g_ConfigAllPara.psIPCCoverPara[nId];//calloc(1, sizeof(SModConfigSensorParam));
			//memcpy(head.pData, para, sizeof(SModConfigSensorParam));
			break;
		case EM_CONFIG_PARA_IPCEXTSENSOR:
			memcpy(&g_ConfigAllPara.psIPCExtSensorPara[nId], para, sizeof(SModConfigIPCExtSensorParam));
			head.pData = &g_ConfigAllPara.psIPCExtSensorPara[nId];//calloc(1, sizeof(SModConfigSensorParam));
			//memcpy(head.pData, para, sizeof(SModConfigSensorParam));
			break;
		case EM_CONFIG_PARA_485EXTSENSOR:
			memcpy(&g_ConfigAllPara.ps485ExtSensorPara[nId], para, sizeof(SModConfigIPCExtSensorParam));
			head.pData = &g_ConfigAllPara.ps485ExtSensorPara[nId];//calloc(1, sizeof(SModConfigSensorParam));
			//memcpy(head.pData, para, sizeof(SModConfigSensorParam));
			break;
		case EM_CONFIG_PARA_HDD:
			memcpy(g_ConfigAllPara.psHDDPara, para, sizeof(SModConfigHDDParam));
			head.pData = g_ConfigAllPara.psHDDPara;//calloc(1, sizeof(SModConfigSensorParam));
			printf("yg g_ConfigAllPara.psHDDPara enable: %d\n", g_ConfigAllPara.psHDDPara->nEnalbe);
			//memcpy(head.pData, para, sizeof(SModConfigSensorParam));
			break;
				
		case EM_CONFIG_PARA_VMOTION:
			memcpy(&g_ConfigAllPara.psVMotionPara[nId], para, sizeof(SModConfigVMotionParam));
			head.pData = &g_ConfigAllPara.psVMotionPara[nId];//calloc(1, sizeof(SModConfigVMotionParam));
			//memcpy(head.pData, para, sizeof(SModConfigVMotionParam));
			break;
		case EM_CONFIG_PARA_VLOST:
			memcpy(&g_ConfigAllPara.psVLostPara[nId], para, sizeof(SModConfigVLostParam));
			head.pData = &g_ConfigAllPara.psVLostPara[nId];//calloc(1, sizeof(SModConfigVLostParam));
			//memcpy(head.pData, para, sizeof(SModConfigVLostParam));
			break;
		case EM_CONFIG_PARA_VBLIND:
			memcpy(&g_ConfigAllPara.psVBlindPara[nId], para, sizeof(SModConfigVBlindParam));
			head.pData = &g_ConfigAllPara.psVBlindPara[nId];//calloc(1, sizeof(SModConfigVBlindParam));
			//memcpy(head.pData, para, sizeof(SModConfigVBlindParam));
			break;
		case EM_CONFIG_PARA_ALARMOUT:
			memcpy(&g_ConfigAllPara.psAlarmOutPara[nId], para, sizeof(SModConfigAlarmOutParam));
			head.pData = &g_ConfigAllPara.psAlarmOutPara[nId];//calloc(1, sizeof(SModConfigAlarmOutParam));
			//memcpy(head.pData, para, sizeof(SModConfigAlarmOutParam));
			break;
		case EM_CONFIG_PARA_BUZZ:
			memcpy(&g_ConfigAllPara.psBuzzPara[nId], para, sizeof(SModConfigBuzzParam));
			head.pData = &g_ConfigAllPara.psBuzzPara[nId];//calloc(1, sizeof(SModConfigBuzzParam));
			//memcpy(head.pData, para, sizeof(SModConfigBuzzParam));
			break;
		case EM_CONFIG_PARA_SENSOR_SCH:
			memcpy(&g_ConfigAllPara.psSensorSchPara[nId], para, sizeof(SModConfigSensorSch));
			head.pData = &g_ConfigAllPara.psSensorSchPara[nId];//calloc(1, sizeof(SModConfigSensorSch));
			//memcpy(head.pData, para, sizeof(SModConfigSensorSch));
			break;
		case EM_CONFIG_PARA_IPCEXTSENSOR_SCH:
			memcpy(&g_ConfigAllPara.psIPCExtSensorSchPara[nId], para, sizeof(SModConfigSensorSch));
			head.pData = &g_ConfigAllPara.psIPCExtSensorSchPara[nId];//calloc(1, sizeof(SModConfigSensorSch));
			//memcpy(head.pData, para, sizeof(SModConfigSensorSch));
			break;
		case EM_CONFIG_PARA_VMOTION_SCH:
			memcpy(&g_ConfigAllPara.psVMotionSchPara[nId], para, sizeof(SModConfigVMotionSch));
			head.pData = &g_ConfigAllPara.psVMotionSchPara[nId];//calloc(1, sizeof(SModConfigVMotionSch));
			//memcpy(head.pData, para, sizeof(SModConfigVMotionSch));
			break;
		case EM_CONFIG_PARA_ALARMOUT_SCH:
			memcpy(&g_ConfigAllPara.psAlarmOutSchPara[nId], para, sizeof(SModConfigAlarmOutSch));
			head.pData = &g_ConfigAllPara.psAlarmOutSchPara[nId];//calloc(1, sizeof(SModConfigAlarmOutSch));
			//memcpy(head.pData, para, sizeof(SModConfigAlarmOutSch));
			break;
		case EM_CONFIG_PARA_VLOST_SCH:
		case EM_CONFIG_PARA_VBLIND_SCH:
		case EM_CONFIG_PARA_BUZZ_SCH:
			break;
		case EM_CONFIG_PARA_PTZ:
			memcpy(&g_ConfigAllPara.psPtzPara[nId], para, sizeof(SModConfigPtzParam));
			head.pData = &g_ConfigAllPara.psPtzPara[nId];//calloc(1, sizeof(SModConfigPtzParam));
			//memcpy(head.pData, para, sizeof(SModConfigPtzParam));
			break;
		case EM_CONFIG_PARA_USER:
			memcpy(g_ConfigAllPara.psUserPara, para, sizeof(SModConfigUserPara));
			head.pData = g_ConfigAllPara.psUserPara;//calloc(1, sizeof(SModConfigUserPara));
			//memcpy(head.pData, para, sizeof(SModConfigUserPara));
			break;
		case EM_CONFIG_PARA_NETWORK:
			memcpy(g_ConfigAllPara.psNetPara, para, sizeof(SModConfigNetParam));
			head.pData = g_ConfigAllPara.psNetPara;//calloc(1, sizeof(SModConfigPtzParam));
			//memcpy(head.pData, para, sizeof(SModConfigPtzParam));
			break;
		//NVR used
		case EM_CONFIG_PARA_IPCAMERA:
			memcpy(&g_ConfigAllPara.psIPCameraPara[nId], para, sizeof(SModConfigIPCameraParam));
			head.pData = &g_ConfigAllPara.psIPCameraPara[nId];
			break;
		case EM_CONFIG_PARA_SNAP_CHN:
			memcpy(&g_ConfigAllPara.psSnapPara[nId], para, sizeof(SModConfigSnapChnPara));
			head.pData = &g_ConfigAllPara.psSnapPara[nId];
			break;
		//yaogang modify 20141209 深广平台参数
		case EM_CONFIG_PARA_SG:
			memcpy(g_ConfigAllPara.psSGPara, para, sizeof(SModConfigSGParam));
			head.pData = g_ConfigAllPara.psSGPara;//calloc(1, sizeof(SModConfigSensorParam));
			//memcpy(head.pData, para, sizeof(SModConfigSensorParam));
			break;
		case EM_CONFIG_PARA_SG_DAILY:
			memcpy(g_ConfigAllPara.psSGDailyPara, para, sizeof(SModDailyPicCFG));
			head.pData = g_ConfigAllPara.psSGDailyPara;
			break;
		case EM_CONFIG_PARA_SG_ALARM:
			memcpy(&g_ConfigAllPara.psSGAlarmPara[nId], para, sizeof(SModAlarmPicCFG));
			head.pData = &g_ConfigAllPara.psSGAlarmPara[nId];
			break;
		//yaogang modify for bad disk	
		case EM_CONFIG_BAD_DISK:
			memcpy(&g_ConfigAllPara.psBadDiskListPara[nId], para, sizeof(SModConfigBadDisk));
			head.pData = &g_ConfigAllPara.psBadDiskListPara[nId];
			break;
		
		case EM_CONFIG_PARA_DVR_PROPERTY:
			//no permit
		default:
			printf("%s Error: Invalid para type: %d!\n", __FUNCTION__, emType);
			ret = -1;
			break;
	}
	
	sem_post(&g_ConfigAllPara.sem);
	
	if(0 == ret)
	{
		if(0 != ConfigWriteMsgQueue(&head))
		{
			fprintf(stderr, "ConfigWriteMsgQueue error! function: %s\n", __FUNCTION__);
			//free(head.pData);
			ret = -1;
		}
	}
	
	return ret;
}

s32 ModConfigGetDefaultParam(EMCONFIGPARATYPE emType, void* para, s32 nId)
{
	if(g_BeAbleToDefault)
	{
		return ConfigGetParamFromMem(emType, para, nId, &g_ConfigDefaultAllPara);
	}
	else
	{
		return -1;
	}
}

static int fileflush(FILE *fp)
{
#ifndef WIN32
	//pw 2010/10/14
	if(NULL == fp)
	{
		return -1;
	}
	fflush(fp);
	fsync(fileno(fp));
#endif
	return 1;
}

static int filecp(char *src,char *dst)
{
	//printf("*****filecp start*****\n");
	
	int nRet;
	FILE* fp1,*fp2;
	unsigned char byData[1024];
	
	fp1 = fopen(src,"rb");
	if(fp1 == NULL)
	{
		return 0;
	}
	
	fp2 = fopen(dst,"wb");
	if(fp2 == NULL)
	{
		printf("fopen %s failed,errno=%d,str=%s\n",dst,errno,strerror(errno));
		fclose(fp1);
		return 0;
	}
	
	//size_t fread(void *ptr,size_t size,size_t nmemb,FILE *stream);
	//size_t fwrite(const void *ptr,size_t size,size_t nmemb,FILE *stream);
	
	nRet = fread(byData,1,sizeof(byData),fp1);
	while(nRet > 0)
	{
		fwrite(byData,nRet,1,fp2);
		nRet = fread(byData,1,sizeof(byData),fp1);
	}
	fileflush(fp2);
	
	fclose(fp1);
	fclose(fp2);
	
	//printf("*****filecp success*****\n");
	
	return 1;
}

s32 ModConfigDefault(void)
{
	if(g_Inited == 0)
	{
		printf("Mod config has not been inited!\n");
		return -1;
	}
	if(g_IsResuming)
	{
		printf("ModConfigDefault failed! system is busy!\n");
		return -1;
	}
	
	g_IsDefaulting = 1;
	
	sem_trywait(&g_sConfigDevInfo.sem);
	
	usleep(500*1000);
	
	if(0 != WriteFileToFlash(g_sConfigDevInfo.nDefaultConfPath))
	{
		printf("write Default config file to flash error! function:%s\n", __FUNCTION__);
		g_IsDefaulting = 0;
		return -1;
	}
	
	g_IsDefaulting = 0;
	
	#if 1//csp modify
	filecp(g_sConfigDevInfo.nDefaultConfPath, g_sConfigDevInfo.nFilePath);
	#else
	if(0 != rename(g_sConfigDevInfo.nDefaultConfPath, g_sConfigDevInfo.nFilePath))
	{
		printf("rename config file error! function:%s\n", __FUNCTION__);
		return -1;
	}
	#endif
	
	#if 1//csp modify
	sem_wait(&g_ConfigAllPara.sem);
	
	u8 nLangId = g_ConfigAllPara.psSystemPara->nLangId;//csp modify 20130322
	u8 nVideoStandard = g_ConfigAllPara.psSystemPara->nVideoStandard;//csp modify 20130322
	u8 nTimeZone = g_ConfigAllPara.psSystemPara->nTimeZone;//csp modify 20131217
	
	u8 chnnum = g_sConfigDvrProperty.nPreviewNum;//csp modify 20130407
	
	memcpy(tmpMac, g_ConfigAllPara.psNetPara->MAC, sizeof(tmpMac));
	
	memcpy(g_ConfigAllPara.psSystemPara, g_ConfigDefaultAllPara.psSystemPara, sizeof(*g_ConfigAllPara.psSystemPara));
	memcpy(g_ConfigAllPara.psPreviewPara, g_ConfigDefaultAllPara.psPreviewPara, sizeof(*g_ConfigAllPara.psPreviewPara));
	memcpy(g_ConfigAllPara.psMainTainPara, g_ConfigDefaultAllPara.psMainTainPara, sizeof(*g_ConfigAllPara.psMainTainPara));
	
	memcpy(g_ConfigAllPara.psMainOsdPara, g_ConfigDefaultAllPara.psMainOsdPara, sizeof(*g_ConfigAllPara.psMainOsdPara));
	
	//csp modify 20130407
	memcpy(g_ConfigAllPara.psImagePara, g_ConfigDefaultAllPara.psImagePara, chnnum * sizeof(*g_ConfigAllPara.psImagePara));
	memcpy(g_ConfigAllPara.psVoImagePara, g_ConfigDefaultAllPara.psVoImagePara, 2 * sizeof(*g_ConfigAllPara.psVoImagePara));
	memcpy(g_ConfigAllPara.psStrOsdPara, g_ConfigDefaultAllPara.psStrOsdPara, chnnum * sizeof(*g_ConfigAllPara.psStrOsdPara));
	memcpy(g_ConfigAllPara.psRectOsdPara, g_ConfigDefaultAllPara.psRectOsdPara, chnnum * sizeof(*g_ConfigAllPara.psRectOsdPara));
	memcpy(g_ConfigAllPara.psVideoMainPara, g_ConfigDefaultAllPara.psVideoMainPara, g_sConfigDvrProperty.nVidMainNum * sizeof(*g_ConfigAllPara.psVideoMainPara));
	memcpy(g_ConfigAllPara.psVideoSubPara, g_ConfigDefaultAllPara.psVideoSubPara, g_sConfigDvrProperty.nVidSubNum * sizeof(*g_ConfigAllPara.psVideoSubPara));
	memcpy(g_ConfigAllPara.psVideoMobPara, g_ConfigDefaultAllPara.psVideoMobPara, g_sConfigDvrProperty.nVidSubNum * sizeof(*g_ConfigAllPara.psVideoMobPara));
	memcpy(g_ConfigAllPara.psRecordPara, g_ConfigDefaultAllPara.psRecordPara, g_sConfigDvrProperty.nRecNum * sizeof(*g_ConfigAllPara.psRecordPara));
	memcpy(g_ConfigAllPara.psRecTimerSchPara, g_ConfigDefaultAllPara.psRecTimerSchPara, g_sConfigDvrProperty.nRecNum * sizeof(*g_ConfigAllPara.psRecTimerSchPara));
	memcpy(g_ConfigAllPara.psRecVMSchPara, g_ConfigDefaultAllPara.psRecVMSchPara, g_sConfigDvrProperty.nRecNum * sizeof(*g_ConfigAllPara.psRecVMSchPara));
	memcpy(g_ConfigAllPara.psRecSensorSchPara, g_ConfigDefaultAllPara.psRecSensorSchPara, g_sConfigDvrProperty.nRecNum * sizeof(*g_ConfigAllPara.psRecSensorSchPara));
	memcpy(g_ConfigAllPara.psSensorPara, g_ConfigDefaultAllPara.psSensorPara, g_sConfigDvrProperty.nSensorNum * sizeof(*g_ConfigAllPara.psSensorPara));
	//yaogang modify 20141010
	memcpy(g_ConfigAllPara.psIPCCoverPara, g_ConfigDefaultAllPara.psIPCCoverPara, g_sConfigDvrProperty.nVidMainNum * sizeof(*g_ConfigAllPara.psIPCCoverPara));
	memcpy(g_ConfigAllPara.psIPCExtSensorPara, g_ConfigDefaultAllPara.psIPCExtSensorPara, g_sConfigDvrProperty.nVidMainNum * sizeof(*g_ConfigAllPara.psIPCExtSensorPara));
	memcpy(g_ConfigAllPara.ps485ExtSensorPara, g_ConfigDefaultAllPara.ps485ExtSensorPara, g_sConfigDvrProperty.nVidMainNum * sizeof(*g_ConfigAllPara.ps485ExtSensorPara));
	memcpy(g_ConfigAllPara.psHDDPara, g_ConfigDefaultAllPara.psHDDPara,  sizeof(*g_ConfigAllPara.psHDDPara));
	
	memcpy(g_ConfigAllPara.psVMotionPara, g_ConfigDefaultAllPara.psVMotionPara, chnnum * sizeof(*g_ConfigAllPara.psVMotionPara));
	memcpy(g_ConfigAllPara.psVLostPara, g_ConfigDefaultAllPara.psVLostPara, chnnum * sizeof(*g_ConfigAllPara.psVLostPara));
	memcpy(g_ConfigAllPara.psVBlindPara, g_ConfigDefaultAllPara.psVBlindPara, chnnum * sizeof(*g_ConfigAllPara.psVBlindPara));
	memcpy(g_ConfigAllPara.psAlarmOutPara, g_ConfigDefaultAllPara.psAlarmOutPara, g_sConfigDvrProperty.nAlarmoutNum * sizeof(*g_ConfigAllPara.psAlarmOutPara));
	memcpy(g_ConfigAllPara.psBuzzPara, g_ConfigDefaultAllPara.psBuzzPara, g_sConfigDvrProperty.nBuzzNum * sizeof(*g_ConfigAllPara.psBuzzPara));
	memcpy(g_ConfigAllPara.psSensorSchPara, g_ConfigDefaultAllPara.psSensorSchPara, g_sConfigDvrProperty.nSensorNum * sizeof(*g_ConfigAllPara.psSensorSchPara));
	memcpy(g_ConfigAllPara.psIPCExtSensorSchPara, g_ConfigDefaultAllPara.psIPCExtSensorSchPara, g_sConfigDvrProperty.nVidMainNum* sizeof(*g_ConfigAllPara.psSensorSchPara));
	memcpy(g_ConfigAllPara.psVMotionSchPara, g_ConfigDefaultAllPara.psVMotionSchPara, chnnum * sizeof(*g_ConfigAllPara.psVMotionSchPara));
	memcpy(g_ConfigAllPara.psAlarmOutSchPara, g_ConfigDefaultAllPara.psAlarmOutSchPara, g_sConfigDvrProperty.nAlarmoutNum * sizeof(*g_ConfigAllPara.psAlarmOutSchPara));	
	memcpy(g_ConfigAllPara.psPtzPara, g_ConfigDefaultAllPara.psPtzPara, chnnum * sizeof(*g_ConfigAllPara.psPtzPara));
	
	memcpy(g_ConfigAllPara.psUserPara, g_ConfigDefaultAllPara.psUserPara, sizeof(*g_ConfigAllPara.psUserPara));
	memcpy(g_ConfigAllPara.psNetPara, g_ConfigDefaultAllPara.psNetPara, sizeof(*g_ConfigAllPara.psNetPara));
	
	//NVR used
	memcpy(g_ConfigAllPara.psIPCameraPara, g_ConfigDefaultAllPara.psIPCameraPara, chnnum * sizeof(*g_ConfigAllPara.psIPCameraPara));
	
	memcpy(g_ConfigAllPara.psSnapPara, g_ConfigDefaultAllPara.psSnapPara, chnnum * sizeof(*g_ConfigAllPara.psSnapPara));
	//yaogang modify 20141209 深广平台参数
	memcpy(g_ConfigAllPara.psSGPara, g_ConfigDefaultAllPara.psSGPara, sizeof(*g_ConfigAllPara.psSGPara));
	memcpy(g_ConfigAllPara.psSGDailyPara, g_ConfigDefaultAllPara.psSGDailyPara, sizeof(*g_ConfigAllPara.psSGDailyPara));

	int num = g_sConfigDvrProperty.nVidMainNum + g_sConfigDvrProperty.nSensorNum;
	memcpy(g_ConfigAllPara.psSGAlarmPara, g_ConfigDefaultAllPara.psSGAlarmPara, num * sizeof(*g_ConfigAllPara.psSGAlarmPara));

	//yaogang modify for bad disk
	num = g_sConfigDvrProperty.nMaxHddNum;
	memcpy(g_ConfigAllPara.psBadDiskListPara, g_ConfigDefaultAllPara.psBadDiskListPara, num * sizeof(SModConfigBadDisk));
	
	sem_post(&g_ConfigAllPara.sem);
	
	#else
	g_ConfigAllPara.psAlarmOutPara = g_ConfigDefaultAllPara.psAlarmOutPara;
	g_ConfigAllPara.psAlarmOutSchPara = g_ConfigDefaultAllPara.psAlarmOutSchPara;
	g_ConfigAllPara.psBuzzPara = g_ConfigDefaultAllPara.psBuzzPara;
	g_ConfigAllPara.psImagePara = g_ConfigDefaultAllPara.psImagePara;
	g_ConfigAllPara.psMainOsdPara = g_ConfigDefaultAllPara.psMainOsdPara;
	
	memcpy(tmpMac, g_ConfigAllPara.psNetPara->MAC, sizeof(tmpMac));
	g_ConfigAllPara.psNetPara = g_ConfigDefaultAllPara.psNetPara;
	
	g_ConfigAllPara.psPreviewPara = g_ConfigDefaultAllPara.psPreviewPara;
	g_ConfigAllPara.psPtzPara = g_ConfigDefaultAllPara.psPtzPara;
	g_ConfigAllPara.psRecordPara = g_ConfigDefaultAllPara.psRecordPara;
	g_ConfigAllPara.psRecSensorSchPara = g_ConfigDefaultAllPara.psRecSensorSchPara;
	g_ConfigAllPara.psRecTimerSchPara = g_ConfigDefaultAllPara.psRecTimerSchPara;
	g_ConfigAllPara.psRectOsdPara = g_ConfigDefaultAllPara.psRectOsdPara;
	g_ConfigAllPara.psRecVMSchPara = g_ConfigDefaultAllPara.psRecVMSchPara;
	g_ConfigAllPara.psSensorPara = g_ConfigDefaultAllPara.psSensorPara;
	g_ConfigAllPara.psSensorSchPara = g_ConfigDefaultAllPara.psSensorSchPara;
	g_ConfigAllPara.psStrOsdPara = g_ConfigDefaultAllPara.psStrOsdPara;
	g_ConfigAllPara.psSystemPara = g_ConfigDefaultAllPara.psSystemPara;
	g_ConfigAllPara.psUserPara = g_ConfigDefaultAllPara.psUserPara;
	g_ConfigAllPara.psVBlindPara = g_ConfigDefaultAllPara.psVBlindPara;
	g_ConfigAllPara.psVideoMainPara = g_ConfigDefaultAllPara.psVideoMainPara;
	g_ConfigAllPara.psVideoSubPara = g_ConfigDefaultAllPara.psVideoSubPara;
	g_ConfigAllPara.psVLostPara = g_ConfigDefaultAllPara.psVLostPara;
	g_ConfigAllPara.psVMotionPara = g_ConfigDefaultAllPara.psVMotionPara;
	g_ConfigAllPara.psVMotionSchPara = g_ConfigDefaultAllPara.psVMotionSchPara;
	g_ConfigAllPara.psVoImagePara = g_ConfigDefaultAllPara.psVoImagePara;
	#endif
	
	bCustomMac = 0;
	EnableCustomConfig(EM_CUSTOM_ALL);
	
#ifdef _RZ_P2P_
	if(bCustomMac)
	{
		P2PParam p2ppara;
		GetP2PParam(&p2ppara);
		if(p2ppara.registered == 'Y')
		{
			SModConfigNetParam sPIns;
			ModConfigGetParam(EM_CONFIG_PARA_NETWORK, &sPIns, 0);
			memcpy(sPIns.MAC, p2ppara.mac, sizeof(sPIns.MAC));
			ModConfigSetParam(EM_CONFIG_PARA_NETWORK, &sPIns, 0);
			bCustomMac = 1;
		}
	}
#endif
	
	// 恢复MAC至当前配置
	if(!bCustomMac)
	{
		int i;
		SModConfigNetParam sPIns;
		ModConfigGetParam(EM_CONFIG_PARA_NETWORK, &sPIns, 0);
		memcpy(sPIns.MAC, tmpMac, sizeof(sPIns.MAC));
		for(i=0; i<sizeof(sPIns.MAC); i++)
		{
			printf("%x ", sPIns.MAC[i]);
		}
		printf("\n");
		ModConfigSetParam(EM_CONFIG_PARA_NETWORK, &sPIns, 0);
	}
	
	//csp modify 20130322
	u8 byUseCurrValue = 1;
	if(0 == CustomOpen(NULL))
	{
		SCustomData sCustomData;
		if(0 == CustomGetData2(EM_CUSTOM_DEVICE_VIDEOSTAND,&sCustomData))
		{
			if(sCustomData.nVideoStandard == EM_PAL || sCustomData.nVideoStandard == EM_NTSC)
			{
				byUseCurrValue = 0;
			}
		}
		CustomClose(0);
	}
	if(byUseCurrValue)
	{
		SModConfigSystemParam sPIns;
		ModConfigGetParam(EM_CONFIG_PARA_SYSTEM, &sPIns, 0);
		sPIns.nLangId = nLangId;
		sPIns.nVideoStandard = nVideoStandard;
		sPIns.nTimeZone = nTimeZone;
		ModConfigSetParam(EM_CONFIG_PARA_SYSTEM, &sPIns, 0);
	}
	
	/*//显示开机选择输出设备界面
	if(0 != PublicIniSetValue(g_sConfigDevInfo.nFilePath, 
			CONFIG_SYSTEM_PARAM, 
			CONFIG_SYSTEM_PARAM_SHOWFIRSTMENUE, 
			1))
	{
		return -1;
	}*/
	
	printf("Write default param To Flash OK! function: %s\n", __FUNCTION__);
	usleep(2000*1000);
	
	return 0;
}

s32 ModConfigBackup(s8* pFilePath)
{
	if(g_Inited == 0)
	{
		printf("Mod config has not been inited!\n");
		return -1;
	}
	if(pFilePath == NULL)
	{
		printf("Invalid para! function:%s\n", __FUNCTION__);
		return -1;
	}
	
	FILE* fp1 = NULL;
	FILE* fp2 = NULL;
	fp1 = fopen(g_sConfigDevInfo.nFilePath, "r");
	fp2 = fopen(pFilePath, "w");
	if((NULL == fp1) || (NULL == fp2))
	{
		printf("open file[%s]/[%s] error! function:%s\n", pFilePath, g_sConfigDevInfo.nFilePath, __FUNCTION__);
		
		//csp modify
		if(fp1 != NULL)
		{
			fclose(fp1);
			fp1 = NULL;
		}
		if(fp2 != NULL)
		{
			fclose(fp2);
			fp2 = NULL;
		}
		
		return -1;
	}
	
	//sem_wait(&g_FileSem);//csp modify
	
	int c;
	while((c = fgetc(fp1)) != EOF)
	{
		fputc(c, fp2);	
	}
	
	fclose(fp1);
	
	fflush(fp2);
	fsync(fileno(fp2));
	fclose(fp2);
	
	//sem_post(&g_FileSem);//csp modify
	
	//sync();//csp modify
	
	return 0;
}

s32 ModConfigResumeDefaultDo(EMCONFIGPARATYPE emType, s32 nId);

s32 ModConfigResumeDefault(EMCONFIGPARATYPE emType, s32 nId)
{
	int nCnr = 0;
	
	switch(emType)
	{
		case EM_CONFIG_PARA_IMAGE:
		case EM_CONFIG_PARA_STR_OSD:
		case EM_CONFIG_PARA_RECT_OSD:
		case EM_CONFIG_PARA_VMOTION:
		case EM_CONFIG_PARA_VLOST:
		case EM_CONFIG_PARA_VBLIND:
		case EM_CONFIG_PARA_VMOTION_SCH:
		case EM_CONFIG_PARA_VLOST_SCH:
		case EM_CONFIG_PARA_VBLIND_SCH:
		case EM_CONFIG_PARA_PTZ:
		case EM_CONFIG_PARA_PREVIEW:
		case EM_CONFIG_PARA_IPCAMERA://NVR used
			nCnr = g_sConfigDvrProperty.nPreviewNum;
			break;
		case EM_CONFIG_PARA_ENC_MAIN:
		//yaogang modify 20141010
		case EM_CONFIG_PARA_IPCEXTSENSOR:
		case EM_CONFIG_PARA_485EXTSENSOR:
		case EM_CONFIG_PARA_IPCEXTSENSOR_SCH:	
			nCnr = g_sConfigDvrProperty.nVidMainNum;
			break;
		case EM_CONFIG_PARA_HDD:
			nCnr = 1;
			break;
		case EM_CONFIG_PARA_ENC_SUB:
			nCnr = g_sConfigDvrProperty.nVidSubNum;
			break;			
		case EM_CONFIG_PARA_RECORD:
		case EM_CONFIG_PARA_TIMER_REC_SCH:
		case EM_CONFIG_PARA_SENSOR_REC_SCH:
		case EM_CONFIG_PARA_VMOTION_REC_SCH:
		case EM_CONFIG_PARA_VLOST_REC_SCH:
		case EM_CONFIG_PARA_VBLIND_REC_SCH:
			nCnr = g_sConfigDvrProperty.nRecNum;
			break;		
		case EM_CONFIG_PARA_SENSOR:
		case EM_CONFIG_PARA_SENSOR_SCH:
			nCnr = g_sConfigDvrProperty.nSensorNum;
			break;				
		case EM_CONFIG_PARA_ALARMOUT:
		case EM_CONFIG_PARA_ALARMOUT_SCH:
			nCnr = g_sConfigDvrProperty.nAlarmoutNum;
			break;			
		case EM_CONFIG_PARA_BUZZ:
		case EM_CONFIG_PARA_BUZZ_SCH:
			nCnr = g_sConfigDvrProperty.nBuzzNum;
			break;
		case EM_CONFIG_PARA_USER:
		case EM_CONFIG_PARA_NETWORK:	
		case EM_CONFIG_PARA_DVR_PROPERTY:
		case EM_CONFIG_PARA_SYSTEM:
		case EM_CONFIG_PARA_SG:
			nCnr = 1;
			break;
		default:
			return -1;
	}
	
	int i;
	s32 rtn = 0;
	for(i=0; i<nCnr; i++)
	{
		rtn |= ModConfigResumeDefaultDo(emType, i);
	}
	
	return rtn?-1:0;
}

s32 ModConfigResumeDefaultDo(EMCONFIGPARATYPE emType, s32 nId)
{
	if(g_Inited == 0)
	{
		printf("Mod config has not been inited!\n");
		return -1;
	}
	if(g_IsDefaulting)
	{
		printf("ModConfigResumeDefaultDo failed! system is busy!\n");
		return -1;
	}
	
	switch(emType)
	{
		case EM_CONFIG_PARA_IMAGE:
		case EM_CONFIG_PARA_STR_OSD:
		case EM_CONFIG_PARA_RECT_OSD:
		case EM_CONFIG_PARA_VMOTION:
		case EM_CONFIG_PARA_VLOST:
		case EM_CONFIG_PARA_VBLIND:
		case EM_CONFIG_PARA_VMOTION_SCH:
		case EM_CONFIG_PARA_VLOST_SCH:
		case EM_CONFIG_PARA_VBLIND_SCH:
		case EM_CONFIG_PARA_PTZ:
		case EM_CONFIG_PARA_IPCAMERA://NVR used
			if((nId < 0) || (nId >= g_sConfigDvrProperty.nPreviewNum))
			{
				printf("Invalid para [ID]! function:%s\n", __FUNCTION__);
				return -1;
			}	
			break;
			
		case EM_CONFIG_PARA_ENC_MAIN:
		//yaogang modify 20141010
		case EM_CONFIG_PARA_IPCEXTSENSOR:
		case EM_CONFIG_PARA_485EXTSENSOR:
		case EM_CONFIG_PARA_IPCEXTSENSOR_SCH:
			if((nId < 0) || (nId >= g_sConfigDvrProperty.nVidMainNum))
			{
				printf("Invalid para [ID]! function:%s\n", __FUNCTION__);
				return -1;
			}	
			break;
			
		case EM_CONFIG_PARA_ENC_SUB:
			if((nId < 0) || (nId >= g_sConfigDvrProperty.nVidSubNum))
			{
				printf("Invalid para [ID]! function:%s\n", __FUNCTION__);
				return -1;
			}	
			break;
			
		case EM_CONFIG_PARA_RECORD:
		case EM_CONFIG_PARA_TIMER_REC_SCH:
		case EM_CONFIG_PARA_SENSOR_REC_SCH:
		case EM_CONFIG_PARA_VMOTION_REC_SCH:
		case EM_CONFIG_PARA_VLOST_REC_SCH:
		case EM_CONFIG_PARA_VBLIND_REC_SCH:
			if((nId < 0) || (nId >= g_sConfigDvrProperty.nRecNum))
			{
				printf("Invalid para [ID]! function:%s\n", __FUNCTION__);
				return -1;
			}	
			break;
		
		case EM_CONFIG_PARA_SENSOR:
		case EM_CONFIG_PARA_SENSOR_SCH:
			if((nId < 0) || (nId >= g_sConfigDvrProperty.nSensorNum))
			{
				printf("Invalid para [ID]! function:%s\n", __FUNCTION__);
				return -1;
			}	
			break;
				
		case EM_CONFIG_PARA_ALARMOUT:
		case EM_CONFIG_PARA_ALARMOUT_SCH:
			if((nId < 0) || (nId >= g_sConfigDvrProperty.nAlarmoutNum))
			{
				printf("Invalid para [ID]! function:%s\n", __FUNCTION__);
				return -1;
			}	
			break;
			
		case EM_CONFIG_PARA_BUZZ:
		case EM_CONFIG_PARA_BUZZ_SCH:
			if((nId < 0) || (nId >= g_sConfigDvrProperty.nBuzzNum))
			{
				printf("Invalid para [ID]! function:%s\n", __FUNCTION__);
				return -1;
			}	
			break;
			
		default:
			break;
	}
	
	s32 ret = 0;
	
	g_IsResuming = 1;
	
	sem_trywait(&g_sConfigDevInfo.sem);
	
	if(emType == EM_CONFIG_PARA_ALL)
	{
		if(0 != WriteFileToFlash(g_sConfigDevInfo.nDefaultConfPath))
		{
			printf("write resumed config file to flash error! function:%s\n", __FUNCTION__);
			ret = -1;
		}
		//EnableCustomConfig(EM_CUSTOM_ALL);
	}
	else
	{
		sem_wait(&g_FileSem);
		
		SConfigAll *pTmp = &g_ConfigDefaultAllPara;
		switch(emType)
		{
			case EM_CONFIG_PARA_SYSTEM:
				if(0 != ConfigSetSystemParam(pTmp->psSystemPara, g_sConfigDevInfo.nFilePath))
				{
					ret = -1;
					break;
				}
				break;
			case EM_CONFIG_PARA_PATROL:
			case EM_CONFIG_PARA_PREVIEW:
				if(0 != ConfigSetPreviewParam(pTmp->psPreviewPara, g_sConfigDevInfo.nFilePath))
				{
					ret = -1;
					break;
				}
				break;	
			case EM_CONFIG_PARA_MAINTAIN:
				if(0 != ConfigSetMainTainParam(pTmp->psMainTainPara,  g_sConfigDevInfo.nFilePath));
				{
					ret = -1;
					break;
				}
				break;
			case EM_CONFIG_PARA_IMAGE:
				if(0 != ConfigSetImageParam(pTmp->psImagePara, nId, g_sConfigDevInfo.nFilePath))
				{
					ret = -1;
					break;
				}
				break;				
			case EM_CONFIG_PARA_MAIN_OSD:
				if(0 != ConfigSetMainOsdParam(pTmp->psMainOsdPara, g_sConfigDevInfo.nFilePath))
				{
					ret = -1;
					break;
				}
				break;
				
			case EM_CONFIG_PARA_STR_OSD:
				if(0 != ConfigSetStrOsdParam(pTmp->psStrOsdPara, nId, g_sConfigDevInfo.nFilePath))
				{
					ret = -1;
					break;
				}
				break;
				
			case EM_CONFIG_PARA_RECT_OSD:
				if(0 != ConfigSetRectOsdParam(pTmp->psRectOsdPara, nId, g_sConfigDevInfo.nFilePath))
				{
					ret = -1;
					break;
				}
				break;
				
			case EM_CONFIG_PARA_ENC_MAIN:
				if(0 != ConfigSetMainEncodeParam(pTmp->psVideoMainPara, nId, g_sConfigDevInfo.nFilePath))
				{
					ret = -1;
					break;
				}
				break;				
			case EM_CONFIG_PARA_ENC_SUB:
				if(0 != ConfigSetSubEncodeParam(pTmp->psVideoSubPara, nId, g_sConfigDevInfo.nFilePath))
				{
					ret = -1;
					break;
				}
				break;
			case EM_CONFIG_PARA_ENC_MOB:
				if(0 != ConfigSetMobEncodeParam(pTmp->psVideoMobPara, nId, g_sConfigDevInfo.nFilePath))
				{
					ret = -1;
					break;
				}
				break;
			case EM_CONFIG_PARA_RECORD:
				if(0 != ConfigSetRecordParam(pTmp->psRecordPara, nId, g_sConfigDevInfo.nFilePath))
				{
					ret = -1;
					break;
				}
				break;
				
			case EM_CONFIG_PARA_TIMER_REC_SCH:
				if(0 != ConfigSetTimerRecSchParam(pTmp->psRecTimerSchPara, nId, g_sConfigDevInfo.nFilePath))
				{
					ret = -1;
					break;
				}
				break;
				
			case EM_CONFIG_PARA_VMOTION_REC_SCH:
				if(0 != ConfigSetVMotionRecSchParam(pTmp->psRecVMSchPara, nId, g_sConfigDevInfo.nFilePath))
				{
					ret = -1;
					break;
				}
				break;
				
			case EM_CONFIG_PARA_SENSOR_REC_SCH:
				if(0 != ConfigSetSensorRecSchParam(pTmp->psRecSensorSchPara, nId, g_sConfigDevInfo.nFilePath))
				{
					ret = -1;
					break;
				}
				break;
				
			case EM_CONFIG_PARA_VLOST_REC_SCH:
			case EM_CONFIG_PARA_VBLIND_REC_SCH:
				break;

			case EM_CONFIG_PARA_SENSOR:
				if(0 != ConfigSetSensorParam(pTmp->psSensorPara, nId, g_sConfigDevInfo.nFilePath))
				{
					ret = -1;
					break;
				}
				break;
			//yaogang modify 20141010
			case EM_CONFIG_PARA_IPCCOVER:
				if(0 != ConfigSetIPCCoverParam(pTmp->psIPCCoverPara, nId, g_sConfigDevInfo.nFilePath))
				{
					ret = -1;
					break;
				}
				break;
			case EM_CONFIG_PARA_IPCEXTSENSOR:
				if(0 != ConfigSetIPCExtSensorParam(pTmp->psIPCExtSensorPara, nId, g_sConfigDevInfo.nFilePath))
				{
					ret = -1;
					break;
				}
				break;
			case EM_CONFIG_PARA_485EXTSENSOR:
				if(0 != ConfigSet485ExtSensorParam(pTmp->ps485ExtSensorPara, nId, g_sConfigDevInfo.nFilePath))
				{
					ret = -1;
					break;
				}
				break;
			case EM_CONFIG_PARA_HDD:
				if(0 != ConfigSetHDDParam(pTmp->psHDDPara, nId, g_sConfigDevInfo.nFilePath))
				{
					ret = -1;
					break;
				}
				break;
				
			case EM_CONFIG_PARA_VMOTION:
				if(0 != ConfigSetVMotionParam(pTmp->psVMotionPara, nId, g_sConfigDevInfo.nFilePath))
				{
					ret = -1;
					break;
				}
				break;
				
			case EM_CONFIG_PARA_VLOST:
				if(0 != ConfigSetVLostParam(pTmp->psVLostPara, nId, g_sConfigDevInfo.nFilePath))
				{
					ret = -1;
					break;
				}
				break;
				
			case EM_CONFIG_PARA_VBLIND:
				if(0 != ConfigSetVBlindParam(pTmp->psVBlindPara, nId, g_sConfigDevInfo.nFilePath))
				{
					ret = -1;
					break;
				}
				break;
				
			case EM_CONFIG_PARA_ALARMOUT:
				if(0 != ConfigSetAlarmOutParam(pTmp->psAlarmOutPara, nId, g_sConfigDevInfo.nFilePath))
				{
					ret = -1;
					break;
				}
				break;
				
			case EM_CONFIG_PARA_BUZZ:
				if(0 != ConfigSetBuzzParam(pTmp->psBuzzPara, nId, g_sConfigDevInfo.nFilePath))
				{
					ret = -1;
					break;
				}
				break;
				
			case EM_CONFIG_PARA_SENSOR_SCH:
				if(0 != ConfigSetSensorSchParam(pTmp->psSensorSchPara, nId, g_sConfigDevInfo.nFilePath))
				{
					ret = -1;
					break;
				}
				break;
			case EM_CONFIG_PARA_IPCEXTSENSOR_SCH:
				if(0 != ConfigSetIPCExtSensorSchParam(pTmp->psIPCExtSensorSchPara, nId, g_sConfigDevInfo.nFilePath))
				{
					ret = -1;
					break;
				}
				break;	
			case EM_CONFIG_PARA_VMOTION_SCH:
				if(0 != ConfigSetVMotionSchParam(pTmp->psVMotionSchPara, nId, g_sConfigDevInfo.nFilePath))
				{
					ret = -1;
					break;
				}
				break;
				
			case EM_CONFIG_PARA_ALARMOUT_SCH:
				if(0 != ConfigSetAlarmOutSchParam(pTmp->psAlarmOutSchPara, nId, g_sConfigDevInfo.nFilePath))
				{
					ret = -1;
					break;
				}
				break;
				
			case EM_CONFIG_PARA_VLOST_SCH:
			case EM_CONFIG_PARA_VBLIND_SCH:
			case EM_CONFIG_PARA_BUZZ_SCH:
				break;
				
			case EM_CONFIG_PARA_PTZ:
				if(0 != ConfigSetPtzParam(pTmp->psPtzPara, nId, g_sConfigDevInfo.nFilePath))
				{
					ret = -1;
					break;
				}
				break;
				
			case EM_CONFIG_PARA_USER:
				if(0 != ConfigSetUserParam(pTmp->psUserPara, g_sConfigDevInfo.nFilePath))
				{
					ret = -1;
					break;
				}
				break;
				
			case EM_CONFIG_PARA_NETWORK:
				if(0 != ConfigSetNetParam(pTmp->psNetPara, nId, g_sConfigDevInfo.nFilePath))
				{
					ret = -1;
					break;
				}
				
				EnableCustomConfig(EM_CUSTOM_DEVICE_IP);
				EnableCustomConfig(EM_CUSTOM_DEVICE_GATEWAY);
				EnableCustomConfig(EM_CUSTOM_DEVICE_MAC);
				break;
				
			//NVR used
			case EM_CONFIG_PARA_IPCAMERA:
				if(0 != ConfigSetIPCameraParam(pTmp->psIPCameraPara, nId, g_sConfigDevInfo.nFilePath))
				{
					ret = -1;
					break;
				}
				break;
			//Snap
			case EM_CONFIG_PARA_SNAP_CHN:
				if(0 != ConfigSetSnapParam(pTmp->psSnapPara, nId, g_sConfigDevInfo.nFilePath))
				{
					ret = -1;
					break;
				}
				break;
			//yaogang modify 20141209 深广平台参数
			case EM_CONFIG_PARA_SG:
				if(0 != ConfigSetSGParam(pTmp->psSGPara, g_sConfigDevInfo.nFilePath))
				{
					ret = -1;
					break;
				}
				break;
			case EM_CONFIG_PARA_SG_DAILY:
				if(0 != ConfigSetSGDailyParam(pTmp->psSGDailyPara, g_sConfigDevInfo.nFilePath))
				{
					ret = -1;
					break;
				}
				break;
			case EM_CONFIG_PARA_SG_ALARM:
				if(0 != ConfigSetSGAlarmParam(pTmp->psSGAlarmPara, nId, g_sConfigDevInfo.nFilePath))
				{
					ret = -1;
					break;
				}
				break;
			//yaogang modify for bad disk
			case EM_CONFIG_BAD_DISK:
				if(0 != ConfigSetBadDiskParam(pTmp->psBadDiskListPara, nId, g_sConfigDevInfo.nFilePath))
				{
					ret = -1;
					break;
				}
				break;
			case EM_CONFIG_PARA_DVR_PROPERTY:
				//no permit
			default:
				printf("%s Error: Invalid para type: %d!\n", __FUNCTION__, emType);
				ret = -1;
				break;
		}
		
		if((ret == 0) && (0 != WriteFileToFlash(g_sConfigDevInfo.nFilePath)))
		{
			printf("write config file to flash error! function:%s\n", __FUNCTION__);
			ret = -1;
		}
		
		sem_post(&g_FileSem);
	}
	
	g_IsResuming = 0;
	
	return ret;
}

//导入配置文件要用到
s32 ModConfigResume(EMCONFIGPARATYPE emType, s32 nId, s8* pFilePath)
{
	if(g_Inited == 0)
	{
		printf("Mod config has not been inited!\n");
		return -1;
	}
	if(g_IsDefaulting)
	{
		printf("ModConfigResume failed! system is busy!\n");
		return -1;
	}
	if(pFilePath == NULL)
	{
		printf("Invalid para! function:%s\n", __FUNCTION__);
		return -1;
	}
	switch(emType)
	{
		case EM_CONFIG_PARA_IMAGE:
		case EM_CONFIG_PARA_STR_OSD:
		case EM_CONFIG_PARA_RECT_OSD:
		case EM_CONFIG_PARA_VMOTION:
		case EM_CONFIG_PARA_VLOST:
		case EM_CONFIG_PARA_VBLIND:
		case EM_CONFIG_PARA_VMOTION_SCH:
		case EM_CONFIG_PARA_VLOST_SCH:
		case EM_CONFIG_PARA_VBLIND_SCH:
		case EM_CONFIG_PARA_PTZ:
		case EM_CONFIG_PARA_IPCAMERA://NVR used
			if((nId < 0) || (nId >= g_sConfigDvrProperty.nPreviewNum))
			{
				printf("Invalid para [ID]! function:%s\n", __FUNCTION__);
				return -1;
			}	
			break;
			
		case EM_CONFIG_PARA_ENC_MAIN:
		//yaogang modify 20141010
		case EM_CONFIG_PARA_IPCEXTSENSOR:
		case EM_CONFIG_PARA_485EXTSENSOR:
		case EM_CONFIG_PARA_IPCEXTSENSOR_SCH:
			if((nId < 0) || (nId >= g_sConfigDvrProperty.nVidMainNum))
			{
				printf("Invalid para [ID]! function:%s\n", __FUNCTION__);
				return -1;
			}	
			break;
			
		case EM_CONFIG_PARA_ENC_SUB:
			if((nId < 0) || (nId >= g_sConfigDvrProperty.nVidSubNum))
			{
				printf("Invalid para [ID]! function:%s\n", __FUNCTION__);
				return -1;
			}	
			break;
			
		case EM_CONFIG_PARA_RECORD:
		case EM_CONFIG_PARA_TIMER_REC_SCH:
		case EM_CONFIG_PARA_SENSOR_REC_SCH:
		case EM_CONFIG_PARA_VMOTION_REC_SCH:
		case EM_CONFIG_PARA_VLOST_REC_SCH:
		case EM_CONFIG_PARA_VBLIND_REC_SCH:
			if((nId < 0) || (nId >= g_sConfigDvrProperty.nRecNum))
			{
				printf("Invalid para [ID]! function:%s\n", __FUNCTION__);
				return -1;
			}	
			break;
		
		case EM_CONFIG_PARA_SENSOR:
		case EM_CONFIG_PARA_SENSOR_SCH:
			if((nId < 0) || (nId >= g_sConfigDvrProperty.nSensorNum))
			{
				printf("Invalid para [ID]! function:%s\n", __FUNCTION__);
				return -1;
			}	
			break;
				
		case EM_CONFIG_PARA_ALARMOUT:
		case EM_CONFIG_PARA_ALARMOUT_SCH:
			if((nId < 0) || (nId >= g_sConfigDvrProperty.nAlarmoutNum))
			{
				printf("Invalid para [ID]! function:%s\n", __FUNCTION__);
				return -1;
			}	
			break;
			
		case EM_CONFIG_PARA_BUZZ:
		case EM_CONFIG_PARA_BUZZ_SCH:
			if((nId < 0) || (nId >= g_sConfigDvrProperty.nBuzzNum))
			{
				printf("Invalid para [ID]! function:%s\n", __FUNCTION__);
				return -1;
			}	
			break;
			
		default:
			break;
	}
	
	s32 ret = 0;
	
	g_IsResuming = 1;
	
	sem_trywait(&g_sConfigDevInfo.sem);
	
	if(emType == EM_CONFIG_PARA_ALL)
	{
		if(0 != WriteFileToFlash(pFilePath))
		{
			printf("write resumed config file to flash error! function:%s\n", __FUNCTION__);
			ret = -1;
		}
		//EnableCustomConfig(EM_CUSTOM_ALL);
	}
	else
	{
		sem_wait(&g_FileSem);
		
		SConfigAll tmp;
		memset(&tmp, 0, sizeof(SConfigAll));
		switch(emType)
		{
			case EM_CONFIG_PARA_SYSTEM:
				tmp.psSystemPara = calloc(1, sizeof(SModConfigSystemParam));
				if(0 != ConfigGetSystemParam(tmp.psSystemPara, pFilePath))
				{
					ret = -1;
					free(tmp.psSystemPara);
					break;
				}
				if(0 != ConfigSetSystemParam(tmp.psSystemPara, g_sConfigDevInfo.nFilePath))
				{
					ret = -1;
					free(tmp.psSystemPara);
					break;
				}
				free(tmp.psSystemPara);
				break;
			case EM_CONFIG_PARA_MAINTAIN:
				tmp.psMainTainPara= calloc(1, sizeof(SModConfigMainTainParam));
				if(0 != ConfigGetMainTainParam(tmp.psMainTainPara, pFilePath))
				{
					ret = -1;
					free(tmp.psMainTainPara);
					break;
				}
				if(0 != ConfigSetMainTainParam(tmp.psMainTainPara, g_sConfigDevInfo.nFilePath))
				{
					ret = -1;
					free(tmp.psMainTainPara);
					break;
				}
				free(tmp.psMainTainPara);
				break;
			case EM_CONFIG_PARA_PATROL:
			case EM_CONFIG_PARA_PREVIEW:
				tmp.psPreviewPara = calloc(1, sizeof(SModConfigPreviewParam));
				if(0 != ConfigGetPreviewParam(tmp.psPreviewPara, pFilePath))
				{
					ret = -1;
					free(tmp.psPreviewPara);
					break;
				}
				if(0 != ConfigSetPreviewParam(tmp.psPreviewPara, g_sConfigDevInfo.nFilePath))
				{
					ret = -1;
					free(tmp.psPreviewPara);
					break;
				}
				free(tmp.psPreviewPara);
				break;
				
			case EM_CONFIG_PARA_IMAGE:
				tmp.psImagePara = calloc(1, sizeof(SModConfigImagePara));
				if(0 != ConfigGetImageParam(tmp.psImagePara, nId, pFilePath))
				{
					ret = -1;
					free(tmp.psImagePara);
					break;
				}
				if(0 != ConfigSetImageParam(tmp.psImagePara, nId, g_sConfigDevInfo.nFilePath))
				{
					ret = -1;
					free(tmp.psImagePara);
					break;
				}
				free(tmp.psImagePara);
				break;
				
			case EM_CONFIG_PARA_MAIN_OSD:
				tmp.psMainOsdPara = calloc(1, sizeof(SModConfigMainOsdParam));
				if(0 != ConfigGetMainOsdParam(tmp.psMainOsdPara, pFilePath))
				{
					ret = -1;
					free(tmp.psMainOsdPara);
					break;
				}
				if(0 != ConfigSetMainOsdParam(tmp.psMainOsdPara, g_sConfigDevInfo.nFilePath))
				{
					ret = -1;
					free(tmp.psMainOsdPara);
					break;
				}
				free(tmp.psMainOsdPara);
				break;
				
			case EM_CONFIG_PARA_STR_OSD:
				tmp.psStrOsdPara = calloc(1, sizeof(SModConfigStrOsdpara));
				if(0 != ConfigGetStrOsdParam(tmp.psStrOsdPara, nId, pFilePath))
				{
					ret = -1;
					free(tmp.psStrOsdPara);
					break;
				}
				if(0 != ConfigSetStrOsdParam(tmp.psStrOsdPara, nId, g_sConfigDevInfo.nFilePath))
				{
					ret = -1;
					free(tmp.psStrOsdPara);
					break;
				}
				free(tmp.psStrOsdPara);
				break;
				
			case EM_CONFIG_PARA_RECT_OSD:
				tmp.psRectOsdPara = calloc(1, sizeof(SModConfigRectOsdpara));
				if(0 != ConfigGetRectOsdParam(tmp.psRectOsdPara, nId, pFilePath))
				{
					ret = -1;
					free(tmp.psRectOsdPara);
					break;
				}
				if(0 != ConfigSetRectOsdParam(tmp.psRectOsdPara, nId, g_sConfigDevInfo.nFilePath))
				{
					ret = -1;
					free(tmp.psRectOsdPara);
					break;
				}
				free(tmp.psRectOsdPara);
				break;
				
			case EM_CONFIG_PARA_ENC_MAIN:
				tmp.psVideoMainPara = calloc(1, sizeof(SModConfigVideoParam));
				if(0 != ConfigGetMainEncodeParam(tmp.psVideoMainPara, nId, pFilePath))
				{
					ret = -1;
					free(tmp.psVideoMainPara);
					break;
				}
				if(0 != ConfigSetMainEncodeParam(tmp.psVideoMainPara, nId, g_sConfigDevInfo.nFilePath))
				{
					ret = -1;
					free(tmp.psVideoMainPara);
					break;
				}
				free(tmp.psVideoMainPara);
				break;
				
			case EM_CONFIG_PARA_ENC_SUB:
				tmp.psVideoSubPara = calloc(1, sizeof(SModConfigVideoParam));
				if(0 != ConfigGetSubEncodeParam(tmp.psVideoSubPara, nId, pFilePath))
				{
					ret = -1;
					free(tmp.psVideoSubPara);
					break;
				}
				if(0 != ConfigSetSubEncodeParam(tmp.psVideoSubPara, nId, g_sConfigDevInfo.nFilePath))
				{
					ret = -1;
					free(tmp.psVideoSubPara);
					break;
				}
				free(tmp.psVideoSubPara);
				break;
			case EM_CONFIG_PARA_ENC_MOB:
				tmp.psVideoMobPara = calloc(1, sizeof(SModConfigVideoParam));
				if(0 != ConfigGetMobEncodeParam(tmp.psVideoMobPara, nId, pFilePath))
				{
					ret = -1;
					free(tmp.psVideoMobPara);
					break;
				}
				if(0 != ConfigSetMobEncodeParam(tmp.psVideoMobPara, nId, g_sConfigDevInfo.nFilePath))
				{
					ret = -1;
					free(tmp.psVideoMobPara);
					break;
				}
				free(tmp.psVideoMobPara);
				break;
				
			case EM_CONFIG_PARA_RECORD:
				tmp.psRecordPara = calloc(1, sizeof(SModConfigRecordParam));
				if(0 != ConfigGetRecordParam(tmp.psRecordPara, nId, pFilePath))
				{
					ret = -1;
					free(tmp.psRecordPara);
					break;
				}
				if(0 != ConfigSetRecordParam(tmp.psRecordPara, nId, g_sConfigDevInfo.nFilePath))
				{
					ret = -1;
					free(tmp.psRecordPara);
					break;
				}
				free(tmp.psRecordPara);
				break;
				
			case EM_CONFIG_PARA_TIMER_REC_SCH:
				tmp.psRecTimerSchPara = calloc(1, sizeof(SModConfigRecTimerSch));
				if(0 != ConfigGetTimerRecSchParam(tmp.psRecTimerSchPara, nId, pFilePath))
				{
					ret = -1;
					free(tmp.psRecTimerSchPara);
					break;
				}
				if(0 != ConfigSetTimerRecSchParam(tmp.psRecTimerSchPara, nId, g_sConfigDevInfo.nFilePath))
				{
					ret = -1;
					free(tmp.psRecTimerSchPara);
					break;
				}
				free(tmp.psRecTimerSchPara);
				break;
				
			case EM_CONFIG_PARA_VMOTION_REC_SCH:
				tmp.psRecVMSchPara = calloc(1, sizeof(SModConfigRecVMSch));
				if(0 != ConfigGetVMotionRecSchParam(tmp.psRecVMSchPara, nId, pFilePath))
				{
					ret = -1;
					free(tmp.psRecVMSchPara);
					break;
				}
				if(0 != ConfigSetVMotionRecSchParam(tmp.psRecVMSchPara, nId, g_sConfigDevInfo.nFilePath))
				{
					ret = -1;
					free(tmp.psRecVMSchPara);
					break;
				}
				free(tmp.psRecVMSchPara);
				break;
				
			case EM_CONFIG_PARA_SENSOR_REC_SCH:
				tmp.psRecSensorSchPara = calloc(1, sizeof(SModConfigRecAlarmInSch));
				if(0 != ConfigGetSensorRecSchParam(tmp.psRecSensorSchPara, nId, pFilePath))
				{
					ret = -1;
					free(tmp.psRecSensorSchPara);
					break;
				}
				if(0 != ConfigSetSensorRecSchParam(tmp.psRecSensorSchPara, nId, g_sConfigDevInfo.nFilePath))
				{
					ret = -1;
					free(tmp.psRecSensorSchPara);
					break;
				}
				free(tmp.psRecSensorSchPara);
				break;
				
			case EM_CONFIG_PARA_VLOST_REC_SCH:
			case EM_CONFIG_PARA_VBLIND_REC_SCH:
				break;

			case EM_CONFIG_PARA_SENSOR:
				tmp.psSensorPara = calloc(1, sizeof(SModConfigSensorParam));
				if(0 != ConfigGetSensorParam(tmp.psSensorPara, nId, pFilePath))
				{
					ret = -1;
					free(tmp.psSensorPara);
					break;
				}
				if(0 != ConfigSetSensorParam(tmp.psSensorPara, nId, g_sConfigDevInfo.nFilePath))
				{
					ret = -1;
					free(tmp.psSensorPara);
					break;
				}
				free(tmp.psSensorPara);
				break;
			//yaogang modify 20141010
			case EM_CONFIG_PARA_IPCCOVER:
				tmp.psIPCCoverPara = calloc(1, sizeof(SModConfigIPCExtSensorParam));
				if(0 != ConfigGetIPCCoverParam(tmp.psIPCCoverPara, nId, pFilePath))
				{ 
					ret = -1;
					free(tmp.psIPCCoverPara);
					break;
				}
				if(0 != ConfigGetIPCCoverParam(tmp.psIPCCoverPara, nId, g_sConfigDevInfo.nFilePath) )
				{
					ret = -1;
					free(tmp.psIPCCoverPara);
					break;
				}
				free(tmp.psIPCCoverPara);
				break;
			case EM_CONFIG_PARA_IPCEXTSENSOR:
				tmp.psIPCExtSensorPara = calloc(1, sizeof(SModConfigIPCExtSensorParam));
				if(0 != ConfigGetIPCExtSensorParam(tmp.psIPCExtSensorPara, nId, pFilePath))
				{ 
					ret = -1;
					free(tmp.psIPCExtSensorPara);
					break;
				}
				if(0 != ConfigGetIPCExtSensorParam(tmp.psIPCExtSensorPara, nId, g_sConfigDevInfo.nFilePath) )
				{
					ret = -1;
					free(tmp.psIPCExtSensorPara);
					break;
				}
				free(tmp.psIPCExtSensorPara);
				break;
			case EM_CONFIG_PARA_485EXTSENSOR:
				tmp.ps485ExtSensorPara = calloc(1, sizeof(SModConfigIPCExtSensorParam));
				if(0 != ConfigGet485ExtSensorParam(tmp.ps485ExtSensorPara, nId, pFilePath))
				{ 
					ret = -1;
					free(tmp.ps485ExtSensorPara);
					break;
				}
				if(0 != ConfigGet485ExtSensorParam(tmp.ps485ExtSensorPara, nId, g_sConfigDevInfo.nFilePath) )
				{
					ret = -1;
					free(tmp.ps485ExtSensorPara);
					break;
				}
				free(tmp.ps485ExtSensorPara);
				break;
			case EM_CONFIG_PARA_HDD:
				tmp.psHDDPara = calloc(1, sizeof(SModConfigHDDParam));
				if(0 != ConfigGetHDDParam(tmp.psHDDPara, 0, pFilePath))
				{ 
					ret = -1;
					free(tmp.psHDDPara);
					break;
				}
				if(0 != ConfigGetHDDParam(tmp.psHDDPara, 0, g_sConfigDevInfo.nFilePath) )
				{
					ret = -1;
					free(tmp.psHDDPara);
					break;
				}
				free(tmp.psHDDPara);
				break;
				
			case EM_CONFIG_PARA_VMOTION:
				tmp.psVMotionPara = calloc(1, sizeof(SModConfigVMotionParam));
				if(0 != ConfigGetVMotionParam(tmp.psVMotionPara, nId, pFilePath))
				{
					ret = -1;
					free(tmp.psVMotionPara);
					break;
				}
				if(0 != ConfigSetVMotionParam(tmp.psVMotionPara, nId, g_sConfigDevInfo.nFilePath))
				{
					ret = -1;
					free(tmp.psVMotionPara);
					break;
				}
				free(tmp.psVMotionPara);
				break;
				
			case EM_CONFIG_PARA_VLOST:
				tmp.psVLostPara = calloc(1, sizeof(SModConfigVLostParam));
				if(0 != ConfigGetVLostParam(tmp.psVLostPara, nId, pFilePath))
				{
					ret = -1;
					free(tmp.psVLostPara);
					break;
				}
				if(0 != ConfigSetVLostParam(tmp.psVLostPara, nId, g_sConfigDevInfo.nFilePath))
				{
					ret = -1;
					free(tmp.psVLostPara);
					break;
				}
				free(tmp.psVLostPara);
				break;
				
			case EM_CONFIG_PARA_VBLIND:
				tmp.psVBlindPara = calloc(1, sizeof(SModConfigVBlindParam));
				if(0 != ConfigGetVBlindParam(tmp.psVBlindPara, nId, pFilePath))
				{
					ret = -1;
					free(tmp.psVBlindPara);
					break;
				}
				if(0 != ConfigSetVBlindParam(tmp.psVBlindPara, nId, g_sConfigDevInfo.nFilePath))
				{
					ret = -1;
					free(tmp.psVBlindPara);
					break;
				}
				free(tmp.psVBlindPara);
				break;
				
			case EM_CONFIG_PARA_ALARMOUT:
				tmp.psAlarmOutPara = calloc(1, sizeof(SModConfigAlarmOutParam));
				if(0 != ConfigGetAlarmOutParam(tmp.psAlarmOutPara, nId, pFilePath))
				{
					ret = -1;
					free(tmp.psAlarmOutPara);
					break;
				}
				if(0 != ConfigSetAlarmOutParam(tmp.psAlarmOutPara, nId, g_sConfigDevInfo.nFilePath))
				{
					ret = -1;
					free(tmp.psAlarmOutPara);
					break;
				}
				free(tmp.psAlarmOutPara);
				break;
				
			case EM_CONFIG_PARA_BUZZ:
				tmp.psBuzzPara = calloc(1, sizeof(SModConfigBuzzParam));
				if(0 != ConfigGetBuzzParam(tmp.psBuzzPara, nId, pFilePath))
				{
					ret = -1;
					free(tmp.psBuzzPara);
					break;
				}
				if(0 != ConfigSetBuzzParam(tmp.psBuzzPara, nId, g_sConfigDevInfo.nFilePath))
				{
					ret = -1;
					free(tmp.psBuzzPara);
					break;
				}
				free(tmp.psBuzzPara);
				break;
				
			case EM_CONFIG_PARA_SENSOR_SCH:
				tmp.psSensorSchPara = calloc(1, sizeof(SModConfigSensorSch));
				if(0 != ConfigGetSensorSchParam(tmp.psSensorSchPara, nId, pFilePath))
				{
					ret = -1;
					free(tmp.psSensorSchPara);
					break;
				}
				if(0 != ConfigSetSensorSchParam(tmp.psSensorSchPara, nId, g_sConfigDevInfo.nFilePath))
				{
					ret = -1;
					free(tmp.psSensorSchPara);
					break;
				}
				free(tmp.psSensorSchPara);
				break;
			case EM_CONFIG_PARA_IPCEXTSENSOR_SCH:
				tmp.psIPCExtSensorSchPara = calloc(1, sizeof(SModConfigSensorSch));
				if(0 != ConfigGetIPCExtSensorSchParam(tmp.psIPCExtSensorSchPara, nId, pFilePath))
				{
					ret = -1;
					free(tmp.psIPCExtSensorSchPara);
					break;
				}
				if(0 != ConfigSetIPCExtSensorSchParam(tmp.psIPCExtSensorSchPara, nId, g_sConfigDevInfo.nFilePath))
				{
					ret = -1;
					free(tmp.psIPCExtSensorSchPara);
					break;
				}
				free(tmp.psIPCExtSensorSchPara);
				break;	
			case EM_CONFIG_PARA_VMOTION_SCH:
				tmp.psVMotionSchPara = calloc(1, sizeof(SModConfigVMotionSch));
				if(0 != ConfigGetVMotionSchParam(tmp.psVMotionSchPara, nId, pFilePath))
				{
					ret = -1;
					free(tmp.psVMotionSchPara);
					break;
				}
				if(0 != ConfigSetVMotionSchParam(tmp.psVMotionSchPara, nId, g_sConfigDevInfo.nFilePath))
				{
					ret = -1;
					free(tmp.psVMotionSchPara);
					break;
				}
				free(tmp.psVMotionSchPara);
				break;
				
			case EM_CONFIG_PARA_ALARMOUT_SCH:
				tmp.psAlarmOutSchPara = calloc(1, sizeof(SModConfigAlarmOutSch));
				if(0 != ConfigGetAlarmOutSchParam(tmp.psAlarmOutSchPara, nId, pFilePath))
				{
					ret = -1;
					free(tmp.psAlarmOutSchPara);
					break;
				}
				if(0 != ConfigSetAlarmOutSchParam(tmp.psAlarmOutSchPara, nId, g_sConfigDevInfo.nFilePath))
				{
					ret = -1;
					free(tmp.psAlarmOutSchPara);
					break;
				}
				free(tmp.psAlarmOutSchPara);
				break;
				
			case EM_CONFIG_PARA_VLOST_SCH:
			case EM_CONFIG_PARA_VBLIND_SCH:
			case EM_CONFIG_PARA_BUZZ_SCH:
				break;
				
			case EM_CONFIG_PARA_PTZ:
				tmp.psPtzPara = calloc(1, sizeof(SModConfigPtzParam));
				if(0 != ConfigGetPtzParam(tmp.psPtzPara, nId, pFilePath))
				{
					ret = -1;
					free(tmp.psPtzPara);
					break;
				}
				if(0 != ConfigSetPtzParam(tmp.psPtzPara, nId, g_sConfigDevInfo.nFilePath))
				{
					ret = -1;
					free(tmp.psPtzPara);
					break;
				}
				free(tmp.psPtzPara);
				break;
				
			case EM_CONFIG_PARA_USER:
				tmp.psUserPara = calloc(1, sizeof(SModConfigUserPara));
				if(0 != ConfigGetUserParam(tmp.psUserPara, pFilePath))
				{
					ret = -1;
					free(tmp.psUserPara);
					break;
				}
				if(0 != ConfigSetUserParam(tmp.psUserPara, g_sConfigDevInfo.nFilePath))
				{
					ret = -1;
					free(tmp.psUserPara);
					break;
				}
				free(tmp.psUserPara);
				break;
				
			case EM_CONFIG_PARA_NETWORK:		
				tmp.psNetPara = calloc(1, sizeof(SModConfigNetParam));
				if(0 != ConfigGetNetParam(tmp.psNetPara, nId, pFilePath))
				{
					ret = -1;
					free(tmp.psNetPara);
					break;
				}
				if(0 != ConfigSetNetParam(tmp.psNetPara, nId, g_sConfigDevInfo.nFilePath))
				{
					ret = -1;
					free(tmp.psNetPara);
					break;
				}
				free(tmp.psNetPara);
				
				EnableCustomConfig(EM_CUSTOM_DEVICE_IP);
				EnableCustomConfig(EM_CUSTOM_DEVICE_GATEWAY);
				EnableCustomConfig(EM_CUSTOM_DEVICE_MAC);
				break;
				
			//NVR used
			case EM_CONFIG_PARA_IPCAMERA:
				tmp.psIPCameraPara = calloc(1, sizeof(SModConfigIPCameraParam));
				if(0 != ConfigGetIPCameraParam(tmp.psIPCameraPara, nId, pFilePath))
				{
					ret = -1;
					free(tmp.psIPCameraPara);
					break;
				}
				if(0 != ConfigSetIPCameraParam(tmp.psIPCameraPara, nId, g_sConfigDevInfo.nFilePath))
				{
					ret = -1;
					free(tmp.psIPCameraPara);
					break;
				}
				free(tmp.psIPCameraPara);
				break;
			//Snap
			case EM_CONFIG_PARA_SNAP_CHN:
				tmp.psSnapPara = calloc(1, sizeof(SModConfigSnapPara));
				if(0 != ConfigGetSnapParam(tmp.psSnapPara, nId, pFilePath))
				{
					ret = -1;
					free(tmp.psSnapPara);
					break;
				}
				if(0 != ConfigGetSnapParam(tmp.psSnapPara, nId, g_sConfigDevInfo.nFilePath))
				{
					ret = -1;
					free(tmp.psSnapPara);
					break;
				}
				free(tmp.psSnapPara);
				break;
			//yaogang modify 20141209 深广平台参数
			case EM_CONFIG_PARA_SG:
				tmp.psSGPara = calloc(1, sizeof(SModConfigSGParam));
				if(0 != ConfigGetSGParam(tmp.psSGPara, pFilePath))
				{
					ret = -1;
					free(tmp.psSGPara);
					break;
				}
				if(0 != ConfigSetSGParam(tmp.psSGPara, g_sConfigDevInfo.nFilePath))
				{
					ret = -1;
					free(tmp.psSGPara);
					break;
				}
				free(tmp.psSGPara);
				break;	
			case EM_CONFIG_PARA_SG_DAILY:
				tmp.psSGDailyPara = calloc(1, sizeof(SModDailyPicCFG));
				if(0 != ConfigGetSGDailyParam(tmp.psSGDailyPara, pFilePath))
				{
					ret = -1;
					free(tmp.psSGDailyPara);
					break;
				}
				if(0 != ConfigSetSGDailyParam(tmp.psSGDailyPara, g_sConfigDevInfo.nFilePath))
				{
					ret = -1;
					free(tmp.psSGDailyPara);
					break;
				}
				free(tmp.psSGDailyPara);
				break;
			case EM_CONFIG_PARA_SG_ALARM:
				tmp.psSGAlarmPara = calloc(1, sizeof(SModAlarmPicCFG));
				if(0 != ConfigGetSGAlarmParam(tmp.psSGAlarmPara, nId, pFilePath))
				{
					ret = -1;
					free(tmp.psSGAlarmPara);
					break;
				}
				if(0 != ConfigSetSGAlarmParam(tmp.psSGAlarmPara, nId, g_sConfigDevInfo.nFilePath))
				{
					ret = -1;
					free(tmp.psSGAlarmPara);
					break;
				}
				free(tmp.psSGAlarmPara);
				break;
			//yaogang modify for bad disk
			case EM_CONFIG_BAD_DISK:
				tmp.psBadDiskListPara = calloc(1, sizeof(SModConfigBadDisk));
				if(0 != ConfigGetBadDiskParam(tmp.psBadDiskListPara, nId, pFilePath))
				{
					ret = -1;
					free(tmp.psBadDiskListPara);
					break;
				}
				if(0 != ConfigSetBadDiskParam(tmp.psBadDiskListPara, nId, g_sConfigDevInfo.nFilePath))
				{
					ret = -1;
					free(tmp.psBadDiskListPara);
					break;
				}
				free(tmp.psBadDiskListPara);
				break;
			case EM_CONFIG_PARA_DVR_PROPERTY:
				//no permit
			default:
				printf("%s Error: Invalid para type: %d!\n", __FUNCTION__, emType);
				ret = -1;
				break;
		}
		
		//csp modify
		//if((ret == 0) && (0 != WriteFileToFlash(pFilePath)))
		if((ret == 0) && (0 != WriteFileToFlash(g_sConfigDevInfo.nFilePath)))
		{
			printf("write config file to flash error! function:%s\n", __FUNCTION__);
			ret = -1;
		}
		
		sem_post(&g_FileSem);
	}
	
	//if((ret != 0) && (flag_times == 0))
	{
		g_IsResuming = 0;
	}
	//else
	{
		//flag_times = 1;
	}
	
	return ret;
}

s32 ModConfigGetConfigFilePath(s8* pFilePath, u32 nMaxPathLen)
{
	if(g_Inited == 0)
	{
		printf("Mod config has not been inited!\n");
		return -1;
	}
	if(pFilePath == NULL)
	{
		printf("Invalid para! function:%s\n", __FUNCTION__);
		return -1;
	}
	if(nMaxPathLen < (strlen(g_sConfigDevInfo.nFilePath) + 1))
	{
		printf("Warnning: arg[2] you offered is too short!");
		return -1;
	}

	strcpy(pFilePath, g_sConfigDevInfo.nFilePath);

	return 0;
}

s32 ModConfigSyncFileToFlash(u8 nMode)
{
	s32 ret = 0;
	
	if(nMode)
	{
		//sem_trywait(&g_sConfigDevInfo.sem);
		sem_wait(&g_FileSem);
		if(0 != WriteFileToFlash(g_sConfigDevInfo.nFilePath))
		{
			ret = -1;
			printf("write config file to flash error! function:%s\n", __FUNCTION__);
		}
		sem_post(&g_FileSem);
	}
	else
	{
		u8 flag = 0;
		u8 count = 0;
		u8 waitSec = 0;
		int value = -1;
		//g_IsSyncFileToFlash = 1;
		//while(g_IsSyncFileToFlash)
		while(1)
		{
			flag = 1;
			if(g_s_MsgQueueInfo.nLenUsed == 0)
			{
				//printf("hehe1,g_s_MsgQueueInfo.nLenUsed is 0\n");
				
				value = -1;
				sem_getvalue(&g_FileSem, &value);
				if(value >= 1)
				{
					//printf("hehe3,sem_getvalue:%d\n",value);
					
					value = -1;
					sem_getvalue(&g_sConfigDevInfo.sem, &value);
					if(value == 0)
					{
						//printf("hehe5,sem_getvalue:%d\n",value);
						
						flag = 0;
						if(++count > 5)
						{
							break;
						}
						else
						{
							printf("sync file to flash over! confirm for time %d ......\n", count);
							usleep(200*1000);
						}
					}
					else
					{
						//printf("hehe6,sem_getvalue:%d\n",value);
					}
				}
				else
				{
					//printf("hehe4,sem_getvalue:%d\n",value);
				}
			}
			else
			{
				//printf("hehe2,g_s_MsgQueueInfo.nLenUsed=%d\n",g_s_MsgQueueInfo.nLenUsed);
			}
			
			if(flag)
			{
				count = 0;
				printf("Sync File To Flash ! Wait %ds\n", ++waitSec);
				usleep(1000*1000);
			}
			
			if(waitSec > 60)
			{
				printf("Sync File To Flash TimeOut! function: %s\n", __FUNCTION__);
				break;
			}
		}
	}
	
	printf("WriteFileToFlash OK! function: %s\n", __FUNCTION__);
	//g_IsSyncFileToFlash = 0;
	
	return ret;
}

#if 1
//获得字符串取值列表
s32 ModConfigGetParaStr(EMCONFIGPARALISTTYPE emParaType, PARAOUT s8* strList[], PARAOUT u8* pnRealListLen, u8 nMaxListLen, u8 nMaxStrLen)
{
	if(pnRealListLen)
	{
		*pnRealListLen = 0;
	}
	
	if((NULL == strList) || (emParaType > EM_CONFIG_PARALIST_NUM)  || (emParaType < EM_CONFIG_PARALIST_BASE))
	{
	   	return -1;
	}
	
	s32 i, ret = 0;

	//printf("%s 1\n", __FUNCTION__);
	if(NULL == g_ConfigStrList[emParaType].strList)
	{
		s8 strItem[32];
		s32 nItem = 0;
		//printf("%s 2\n", __FUNCTION__);
		ret = GetParaListStrFlag(emParaType, strItem);
		if(0 == ret)
		{
			printf("strItem: %s\n", strItem);
			if(0 == strcasecmp(strItem, DVR_CFGPATTERN_VMAINRESOLH_NUM))//cw_9508S
			{
				g_ConfigStrList[emParaType].nItem = 0;
				sprintf(strItem, "%sNum", strItem);
				ret = PublicIniGetValue(DVR_CUSTOM_PATH, DVR_CFGPATTERN, strItem, &nItem);
				if(ret == 0)
				{
					g_ConfigStrList[emParaType].nItem = nItem;
				}
				return 0;
			}
			
			sprintf(strItem, "%sStr", strItem);
			ret = PublicIniGetArrayStrings(DVR_CUSTOM_PATH, DVR_CFGPATTERN, strItem, strList, &nItem, nMaxListLen, nMaxStrLen);
			if((0 != ret) || (0 == nItem))
			{
				int len = strlen(strItem);
				
				strcpy(strItem + len - 3, "List");
				ret = PublicIniGetArrayStrings(DVR_CUSTOM_PATH, DVR_CFGPATTERN, strItem, strList, &nItem, nMaxListLen, nMaxStrLen);
			}
			
			*pnRealListLen = (u8)nItem;
			
			if(*pnRealListLen > 0)
			{
				//PUBPRT("Here");
				g_ConfigStrList[emParaType].nItem = *pnRealListLen;
				g_ConfigStrList[emParaType].strList = (u8**)malloc(*pnRealListLen * sizeof(u8 *));
				for(i = 0; i < *pnRealListLen; i++)
				{
					u8 size = strlen(strList[i]) + 1;
					g_ConfigStrList[emParaType].strList[i] = malloc(size);
					memcpy(g_ConfigStrList[emParaType].strList[i], strList[i], size);
				}
				//PUBPRT("Here");
			}
		}
	}
	else
	{
		//PUBPRT("Here");
		*pnRealListLen = g_ConfigStrList[emParaType].nItem;
		for(i = 0; i < *pnRealListLen; i++)
		{
			strcpy(strList[i], g_ConfigStrList[emParaType].strList[i]);
		}
		//PUBPRT("Here");
	}
	
	return ret;
}

//根据数字取值获得字符串列表中的index
s32 ModConfigGetParaListIndex(EMCONFIGPARALISTTYPE emParaType, s32 nValue, PARAOUT u8* pnIndex)
{
	char strItem[32];
	s32 ret = 0, nItem = 0, i;
	
	if((NULL == pnIndex) || (emParaType > EM_CONFIG_PARALIST_NUM) || (emParaType < EM_CONFIG_PARALIST_BASE))
	{
	    return -1;
	}
	
	if(emParaType == EM_CONFIG_PARALIST_VMAINRESOLH_NUM)//cw_9508S
	{
		return 0;
	}
	
	if(NULL == g_ConfigAllList[emParaType].pConfigValList)
	{
		ret = GetParaListStrFlag(emParaType, strItem);
		if(0 == ret)
		{
			s32 nValueList[MAX_LIST_LEN], *pnValueList[MAX_LIST_LEN];
			
			for(i = 0; i < MAX_LIST_LEN; i++)
			{
				pnValueList[i] = &nValueList[i];
			}
			
			sprintf(strItem, "%sList", strItem);
			//PUBPRT("Here");
			ret = PublicIniGetArrayValues(DVR_CUSTOM_PATH, DVR_CFGPATTERN, strItem, pnValueList, &nItem, MAX_LIST_LEN);
			//printf("%s  nItem = %d ,ret = %d \n",strItem,nItem,ret);
			if(0 != ret)//数据填充失败，释放空间
			{
				return ret;
			}
			//PUBPRT("Here");
			if(nItem > 0)//还没从文件中读到内存中
			{
				//从文件中读到内存中
				g_ConfigAllList[emParaType].pConfigValList = (u32**)malloc(nItem * sizeof(u32 *));
				for(i = 0; i < nItem; i++)
				{
					g_ConfigAllList[emParaType].pConfigValList[i] = (u32*)malloc(sizeof(u32));
					*g_ConfigAllList[emParaType].pConfigValList[i] = nValueList[i];
				}
				
				g_ConfigAllList[emParaType].nNum = nItem;
		    }
			//PUBPRT("Here");
	   	}
	}
	
	if(0 == ret)
	{
	    ret = -1;
	    for (i = 0; i < g_ConfigAllList[emParaType].nNum; i++)
	    {
	        if((*g_ConfigAllList[emParaType].pConfigValList[i]) == nValue)
	        {
	        	//printf("pConfigValList[%d] = %d \n",i,*g_ConfigAllList[emParaType].pConfigValList[i]);
	            ret = 0;
	            *pnIndex = i;
	            break;
	        }
	    }
	}
	
	return ret;
}

//根据index获得实际参数取值
s32 ModConfigGetParaListValue(EMCONFIGPARALISTTYPE emParaType, u8 nIndex, PARAOUT s32* pnValue)
{
	char strItem[32];
	s32 ret = 0, nItem = 0, i = 0;
	
	if((NULL == pnValue) || (emParaType > EM_CONFIG_PARALIST_NUM)  || (emParaType < EM_CONFIG_PARALIST_BASE))
	{
	   	return -1;
	}
	
	if(emParaType == EM_CONFIG_PARALIST_VMAINRESOLH_NUM)//cw_9508S
	{
		*pnValue = g_ConfigStrList[emParaType].nItem;
		return 0;
	}
	
	if(NULL == g_ConfigAllList[emParaType].pConfigValList)
	{
		ret = GetParaListStrFlag(emParaType, strItem);
		if (0 == ret)
		{
			s32 nValueList[MAX_LIST_LEN], *pnValueList[MAX_LIST_LEN];

			for (i = 0; i < MAX_LIST_LEN; i++)
			{
				pnValueList[i] = &nValueList[i];
			}
			sprintf(strItem, "%sList", strItem);
			//PUBPRT("Here");
			ret = PublicIniGetArrayValues(DVR_CUSTOM_PATH, DVR_CFGPATTERN, strItem, pnValueList, &nItem, MAX_LIST_LEN);
			//printf("%s  nItem = %d ,ret = %d \n",strItem,nItem,ret);
			if(0 != ret)//数据填充失败，释放空间
			{
				return ret;
			}
			//PUBPRT("Here");
			if(nItem > 0)//还没从文件中读到内存中
			{
				//从文件中读到内存中
				g_ConfigAllList[emParaType].pConfigValList = (u32**)malloc(nItem * sizeof(u32 *));
				for(i = 0; i < nItem; i++)
				{
					g_ConfigAllList[emParaType].pConfigValList[i] = (u32*)malloc(sizeof(u32));
					*g_ConfigAllList[emParaType].pConfigValList[i] = nValueList[i];
				}
				
				g_ConfigAllList[emParaType].nNum = nItem;
		    }
			//PUBPRT("Here");
	   	}
	}
	
	if(nIndex < g_ConfigAllList[emParaType].nNum)
	{
		*pnValue = *g_ConfigAllList[emParaType].pConfigValList[nIndex];
		return 0;
	}
	else
	{
		return -1;
	}
}
#else
#define MAX_LIST_LEN 64
typedef struct
{
	u8 strList[MAX_LIST_LEN][MAX_LIST_LEN];
	u8 nItem;
}StrList;

static StrList* aD1Fps = NULL;
static StrList* aFps = NULL;

//获得字符串取值列表
s32 ModConfigGetParaStr(EMCONFIGPARALISTTYPE emParaType, PARAOUT s8* strList[], PARAOUT u8* pnRealListLen, u8 nMaxListLen, u8 nMaxStrLen)
{
	if (pnRealListLen)
	{
		*pnRealListLen = 0;
	}
    if (NULL == strList)
    {
        return -1;
    }

	if(aD1Fps == NULL)
	{
		aD1Fps = (StrList*)malloc(sizeof(StrList));
		if(aD1Fps)
		{
			memset(aD1Fps,0,sizeof(StrList));
		}
	}	

	if(aFps == NULL)
	{
		aFps = (StrList*)malloc(sizeof(StrList));
		if(aFps)
		{
			memset(aFps,0,sizeof(StrList));
		}
	}

    s8 strItem[32];
    s32 ret, nItem;

    ret = GetParaListStrFlag(emParaType, strItem);
    if (0 == ret)
    {
    	if(emParaType == EM_CONFIG_PARALIST_FPSPAL)
		{	
			if(aFps->nItem == 0)
			{
			    sprintf(strItem, "%sStr", strItem);
		        ret = PublicIniGetArrayStrings(DVR_CUSTOM_PATH, DVR_CFGPATTERN, strItem, strList, &nItem, nMaxListLen, nMaxStrLen);
		        if ((0 != ret) || (0 == nItem))
		        //if (0 != ret)
		        {
		            int len = strlen(strItem);

		            strcpy(strItem + len -3, "List");
		            ret = PublicIniGetArrayStrings(DVR_CUSTOM_PATH, DVR_CFGPATTERN, strItem, strList, &nItem, nMaxListLen, nMaxStrLen);
		         }
		        
		        *pnRealListLen = (u8)nItem;

				int i = 0;
				for(i=0; i<nItem; i++)
				{
					strcpy(aFps->strList[i],strList[i]);
				}
				aFps->nItem = nItem;
			}
			else
			{
				int i = 0;
				for(i=0; i<aFps->nItem; i++)
				{
					strcpy(strList[i],aFps->strList[i]);
				}

				*pnRealListLen = aFps->nItem;
			}
		}
		else if(emParaType == EM_CONFIG_PARALIST_FPSPALD1)
		{	
			if(aD1Fps->nItem == 0)
			{
			    sprintf(strItem, "%sStr", strItem);
		        ret = PublicIniGetArrayStrings(DVR_CUSTOM_PATH, DVR_CFGPATTERN, strItem, strList, &nItem, nMaxListLen, nMaxStrLen);
		        if ((0 != ret) || (0 == nItem))
		        //if (0 != ret)
		        {
		            int len = strlen(strItem);

		            strcpy(strItem + len -3, "List");
		            ret = PublicIniGetArrayStrings(DVR_CUSTOM_PATH, DVR_CFGPATTERN, strItem, strList, &nItem, nMaxListLen, nMaxStrLen);
		         }
		        
		        *pnRealListLen = (u8)nItem;

				int i = 0;
				for(i=0; i<nItem; i++)
				{
					strcpy(aD1Fps->strList[i],strList[i]);
				}
				aD1Fps->nItem = nItem;
			}
			else
			{
				int i = 0;
				for(i=0; i<aD1Fps->nItem; i++)
				{
					strcpy(strList[i],aD1Fps->strList[i]);
				}

				*pnRealListLen = aD1Fps->nItem;
			}
		}
		else
		{
			sprintf(strItem, "%sStr", strItem);
	        ret = PublicIniGetArrayStrings(DVR_CUSTOM_PATH, DVR_CFGPATTERN, strItem, strList, &nItem, nMaxListLen, nMaxStrLen);
	        if ((0 != ret) || (0 == nItem))
	        //if (0 != ret)
			{
				int len = strlen(strItem);
				
				strcpy(strItem + len -3, "List");
				ret = PublicIniGetArrayStrings(DVR_CUSTOM_PATH, DVR_CFGPATTERN, strItem, strList, &nItem, nMaxListLen, nMaxStrLen);
			}
	        
	        *pnRealListLen = (u8)nItem;
		}
    }
    
    return ret;
}

//根据数字取值获得字符串列表中的index
s32 ModConfigGetParaListIndex(EMCONFIGPARALISTTYPE emParaType, s32 nValue, PARAOUT u8* pnIndex)
{
    s32 nValueList[MAX_LIST_LEN];
    s32* pnValueList[MAX_LIST_LEN];
    char strItem[32];
    s32 ret, nItem = 0, i;

    if (NULL == pnIndex)
    {
        return -1;
    }

    for (i = 0; i < MAX_LIST_LEN; i++)
    {
        pnValueList[i] = &nValueList[i];
    }
    
    ret = GetParaListStrFlag(emParaType, strItem);
    if (0 == ret)
	{
		sprintf(strItem, "%sList", strItem);
		ret = PublicIniGetArrayValues(DVR_CUSTOM_PATH, DVR_CFGPATTERN, strItem, pnValueList, &nItem, MAX_LIST_LEN);
		if (0 == ret)
		{
			ret = -1;
			for (i = 0; i < nItem; i++)
			{
				if (nValueList[i] == nValue)
				{
					ret = 0;
					*pnIndex = i;
					break;
				}
			}
		}
	}
	
	return ret;
}

//根据index获得实际参数取值
s32 ModConfigGetParaListValue(EMCONFIGPARALISTTYPE emParaType, u8 nIndex, PARAOUT s32* pnValue)
{
    char strItem[32];
    s32 ret;
    
    ret = GetParaListStrFlag(emParaType, strItem);
    if (0 == ret)
    {
        sprintf(strItem, "%sList", strItem);
        ret = PublicIniGetArrayValue(DVR_CUSTOM_PATH, DVR_CFGPATTERN, strItem, nIndex, pnValue);

		/*if(emParaType == EM_CONFIG_PARALIST_FPSPAL)
		{	
			printf("PublicIniGetArrayValue ret = %d \n",ret);
		}
		*/
    }
	
    return ret;
}
#endif

s32 ConfigSetNetParam(SModConfigNetParam* para, s32 nId, s8* pFilepath)
{
	if((NULL == para) || (NULL == pFilepath))
	{
		printf("Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}
	
	s32 tmp = 0;
	
	s8 strtmp[30];
	memset(strtmp, 0, sizeof(strtmp));
	sprintf(strtmp, "%s", CONFIG_NET_PARAM);
	
	tmp = para->HttpPort;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_NET_PARAM, 
		strtmp,
		EM_CONFIG_NET_CONFIG_HTTPPORT,
		tmp))
	{
		return -1;
	}
	
	tmp = para->MobilePort;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_NET_PARAM, 
		strtmp,
		EM_CONFIG_NET_CONFIG_MOBILEPORT,
		tmp))
	{
		return -1;
	}
	
	#if 1
	tmp = para->TCPPort;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_NET_PARAM, 
		strtmp,
		EM_CONFIG_NET_CONFIG_SERVERPORT,
		tmp))
	{
		return -1;
	}
	
	tmp = para->DhcpFlag;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_NET_PARAM, 
		strtmp,
		EM_CONFIG_NET_CONFIG_DHCP_FLAG,
		tmp))
	{
		return -1;
	}
	
	tmp = para->HostIP;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_NET_PARAM, 
		strtmp,
		EM_CONFIG_NET_CONFIG_IP,
		tmp))
	{
		return -1;
	}
	
	tmp = para->Submask;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_NET_PARAM, 
		strtmp,
		EM_CONFIG_NET_CONFIG_SUBMASK,
		tmp))
	{
		return -1;
	}
	
	tmp = para->GateWayIP;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_NET_PARAM, 
		strtmp,
		EM_CONFIG_NET_CONFIG_GATEWAY,
		tmp))
	{
		return -1;
	}

	tmp = para->DNSIP;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_NET_PARAM, 
		strtmp,
		EM_CONFIG_NET_CONFIG_DNSIP,
		tmp))
	{
		return -1;
	}
	
	tmp = para->DNSIPAlt;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_NET_PARAM, 
		strtmp,
		EM_CONFIG_NET_CONFIG_DNSIPALT,
		tmp))
	{
		return -1;
	}
	
	if(0 != SetArrayStringToIni(pFilepath,
		CONFIG_NET_PARAM,
		strtmp,
		EM_CONFIG_NET_CONFIG_MAC,
		para->MAC
	))
	{
		return -1;
	}
	
	tmp = para->PPPOEFlag;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_NET_PARAM, 
		strtmp,
		EM_CONFIG_NET_CONFIG_PPPOE_FLAG,
		tmp))
	{
		return -1;
	}
	
	if(0 != SetArrayStringToIni(pFilepath,
		CONFIG_NET_PARAM,
		strtmp,
		EM_CONFIG_NET_CONFIG_POE_USER,
		para->PPPOEUser
	))
	{
		printf("set pppoe user failed\n");
		return -1;
	}
	else
	{
		printf("set pppoe user : %s\n",para->PPPOEUser);
	}
	
	if(0 != SetArrayStringToIni(pFilepath, 
		CONFIG_NET_PARAM, 
		strtmp,
		EM_CONFIG_NET_CONFIG_POE_PASSWD,
		para->PPPOEPasswd))
	{
		printf("set pppoe passwd failed\n");
		return -1;
	}
	else
	{
		printf("set pppoe passwd : %s\n",para->PPPOEPasswd);
	}
	
	/*
	tmp = para->SMTPServerIP;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_NET_PARAM, 
		strtmp,
		EM_CONFIG_NET_MAIL_SMTP_IP,
		tmp))
	{
		return -1;
	}
	*/
	
	if(0 != SetArrayStringToIni(pFilepath, 
		CONFIG_NET_PARAM, 
		strtmp,
		EM_CONFIG_NET_MAIL_SMTP_IP,
		para->sAdancePara.szSmtpServer))
	{
		return -1;
	}
	
	tmp = para->SMTPServerPort;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_NET_PARAM, 
		strtmp,
		EM_CONFIG_NET_MAIL_SMTPPORT,
		tmp))
	{
		return -1;
	}
	
	tmp = para->sAdancePara.nSSLFlag;
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_NET_PARAM, 
		strtmp,
		EM_CONFIG_NET_MAIL_SSL_FLAG,
		tmp))
	{
		return -1;
	}

	if(0 != SetArrayStringToIni(pFilepath, 
		CONFIG_NET_PARAM, 
		strtmp,
		EM_CONFIG_NET_MAIL_SENDMAILBOX,
		para->sAdancePara.szSendMailBox))
	{
		return -1;
	}

	if(0 != SetArrayStringToIni(pFilepath, 
		CONFIG_NET_PARAM, 
		strtmp,
		EM_CONFIG_NET_MAIL_PASSWD,
		para->sAdancePara.szSMailPasswd))
	{
		return -1;
	}
	
	if(0 != SetArrayStringToIni(pFilepath, 
		CONFIG_NET_PARAM, 
		strtmp,
		EM_CONFIG_NET_MAIL_RECVMAILBOX,
		para->sAdancePara.szReceiveMailBox))
	{
		return -1;
	}

	if(0 != SetArrayStringToIni(pFilepath, 
		CONFIG_NET_PARAM, 
		strtmp,
		EM_CONFIG_NET_MAIL_RECVMAILBOX1,
		para->sAdancePara.szReceiveMailBox2))
	{
		return -1;
	}

	if(0 != SetArrayStringToIni(pFilepath, 
		CONFIG_NET_PARAM, 
		strtmp,
		EM_CONFIG_NET_MAIL_RECVMAILBOX2,
		para->sAdancePara.szReceiveMailBox3))
	{
		return -1;
	}

	if(0 != SetArrayStringToIni(pFilepath, 
		CONFIG_NET_PARAM, 
		strtmp,
		EM_CONFIG_NET_MAIL_SNAPPIC,
		para->sAdancePara.szSnapPic))
	{
		return -1;
	}
	
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_NET_PARAM, 
		strtmp,
		EM_CONFIG_NET_MAIL_SNAPINTERVAL,
		para->sAdancePara.nSnapIntvl))
	{
		return -1;
	}
	
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_NET_PARAM, 
		strtmp,
		EM_CONFIG_NET_CONFIG_DDNS_FLAG,
		para->DDNSFlag))
	{
		return -1;
	}
	//printf("set ddns flag=%d\n",para->DDNSFlag);
	
	if(0 != SetArrayStringToIni(pFilepath, 
		CONFIG_NET_PARAM, 
		strtmp,
		EM_CONFIG_NET_CONFIG_DDNS_DOMAIN,
		para->DDNSDomain))
	{
		return -1;
	}
	
	if(0 != SetArrayStringToIni(pFilepath, 
		CONFIG_NET_PARAM, 
		strtmp,
		EM_CONFIG_NET_CONFIG_DDNS_USER,
		para->DDNSUser))
	{
		return -1;
	}
	
	if(0 != SetArrayStringToIni(pFilepath, 
		CONFIG_NET_PARAM, 
		strtmp,
		EM_CONFIG_NET_CONFIG_DDNS_PASSWD,
		para->DDNSPasswd))
	{
		return -1;
	}
	
	if(0 != SetArrayStringToIni(pFilepath, 
		CONFIG_NET_PARAM, 
		strtmp,
		EM_CONFIG_NET_CONFIG_DDNS_DOMAIN,
		para->DDNSDomain))
	{
		return -1;
	}
	
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_NET_PARAM, 
		strtmp,
		EM_CONFIG_NET_CONFIG_UPDATEINTERVAL,
		para->UpdateIntvl))
	{
		return -1;
	}
	//printf("ConfigSetNetParam UpdateIntvl=%d\n",para->UpdateIntvl);
	
	//csp modify 20130321
	if(0 != PublicIniSetArrayValue(pFilepath, 
		CONFIG_NET_PARAM, 
		strtmp,
		EM_CONFIG_NET_CONFIG_UPNP_FLAG,
		para->UPNPFlag))
	{
		return -1;
	}
	#endif
	
	return 0;
}

s32 ConfigGetNetParam(SModConfigNetParam* para, s32 nId, s8* pFilepath)
{
	if((NULL == para) || (nId < 0) || (nId >= g_sConfigDvrProperty.nPreviewNum) || (NULL == pFilepath))
	{
		printf("Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}
	
	//printf("ConfigGetNetParam: %d\n", __LINE__);
	
	s32 tmp = 0;
	
	s8 strtmp[30];
	memset(strtmp, 0, sizeof(strtmp));
	sprintf(strtmp, "%s", CONFIG_NET_PARAM);
	
	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_NET_PARAM, 
		strtmp,
		EM_CONFIG_NET_CONFIG_HTTPPORT,
		&tmp))
	{
		return -1;
	}
	para->HttpPort = tmp;
	
	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_NET_PARAM, 
		strtmp,
		EM_CONFIG_NET_CONFIG_MOBILEPORT,
		&tmp))
	{
		return -1;
	}
	para->MobilePort = tmp;
	
	//printf("ConfigGetNetParam: %d\n", __LINE__);
	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_NET_PARAM, 
		strtmp,
		EM_CONFIG_NET_CONFIG_SERVERPORT,
		&tmp))
	{
		return -1;
	}
	para->TCPPort = tmp;
	
	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_NET_PARAM, 
		strtmp,
		EM_CONFIG_NET_CONFIG_DHCP_FLAG,
		&tmp))
	{
		return -1;
	}
	para->DhcpFlag = tmp;

	//printf("ConfigGetNetParam: %d\n", __LINE__);
	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_NET_PARAM, 
		strtmp,
		EM_CONFIG_NET_CONFIG_IP,
		&tmp))
	{
		return -1;
	}
	para->HostIP = tmp;
	printf("ConfigGetNetParam:Filepath=%s\n",pFilepath);
	
	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_NET_PARAM, 
		strtmp,
		EM_CONFIG_NET_CONFIG_SUBMASK,
		&tmp))
	{
		return -1;
	}
	para->Submask = tmp;
	
	//printf("ConfigGetNetParam: %d\n", __LINE__);
	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_NET_PARAM, 
		strtmp,
		EM_CONFIG_NET_CONFIG_GATEWAY,
		&tmp))
	{
		return -1;
	}
	para->GateWayIP = tmp;

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_NET_PARAM, 
		strtmp,
		EM_CONFIG_NET_CONFIG_DNSIP,
		&tmp))
	{
		return -1;
	}
	para->DNSIP = tmp;
	
	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_NET_PARAM, 
		strtmp,
		EM_CONFIG_NET_CONFIG_DNSIPALT,
		&tmp))
	{
		return -1;
	}
	//printf("ConfigGetNetParam: %d\n", __LINE__);
	para->DNSIPAlt = tmp;
	
	if(0 != GetArrayStringFromIni(pFilepath, 
		CONFIG_NET_PARAM, 
		strtmp,
		EM_CONFIG_NET_CONFIG_MAC,
		para->MAC,
		sizeof(para->MAC)))
	{
		return -1;
	}
	printf("ConfigGetNetParam: MAC=%s,Filepath=%s,strtmp=%s\n", para->MAC, pFilepath, strtmp);
	
	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_NET_PARAM, 
		strtmp,
		EM_CONFIG_NET_CONFIG_PPPOE_FLAG,
		&tmp))
	{
		return -1;
	}
	para->PPPOEFlag = tmp;
	
	if(0 != GetArrayStringFromIni(pFilepath,
		CONFIG_NET_PARAM,
		strtmp,
		EM_CONFIG_NET_CONFIG_POE_USER,
		para->PPPOEUser,
		sizeof(para->PPPOEUser)
	))
	{
		return -1;
	}

	if(0 != GetArrayStringFromIni(pFilepath, 
		CONFIG_NET_PARAM, 
		strtmp,
		EM_CONFIG_NET_CONFIG_POE_PASSWD,
		para->PPPOEPasswd,
		sizeof(para->PPPOEPasswd)))
	{
		return -1;
	}

	/*
	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_NET_PARAM, 
		strtmp,
		EM_CONFIG_NET_MAIL_SMTP_IP,
		&tmp))
	{
		return -1;
	}
	para->SMTPServerIP = tmp;
	*/

	#if 1
	if(0 != GetArrayStringFromIni(pFilepath, 
		CONFIG_NET_PARAM, 
		strtmp,
		EM_CONFIG_NET_MAIL_SMTP_IP,
		para->sAdancePara.szSmtpServer,
		sizeof(para->sAdancePara.szSmtpServer)))
	{
		return -1;
	}
	#endif
	
	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_NET_PARAM, 
		strtmp,
		EM_CONFIG_NET_MAIL_SMTPPORT,
		&tmp))
	{
		return -1;
	}
	para->SMTPServerPort = tmp;

	// ok 1
	
	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_NET_PARAM, 
		strtmp,
		EM_CONFIG_NET_MAIL_SSL_FLAG,
		&tmp))
	{
		return -1;
	}	
	para->sAdancePara.nSSLFlag = tmp;

#if 1
	if(0 != GetArrayStringFromIni(pFilepath, 
		CONFIG_NET_PARAM, 
		strtmp,
		EM_CONFIG_NET_MAIL_SENDMAILBOX,
		para->sAdancePara.szSendMailBox,
		sizeof(para->sAdancePara.szSendMailBox)
	))
	{
		return -1;
	}

	// bad 2
	
	if(0 != GetArrayStringFromIni(pFilepath, 
		CONFIG_NET_PARAM, 
		strtmp,
		EM_CONFIG_NET_MAIL_PASSWD,
		para->sAdancePara.szSMailPasswd,
		sizeof(para->sAdancePara.szSMailPasswd)))
	{
		return -1;
	}
	
	if(0 != GetArrayStringFromIni(pFilepath, 
		CONFIG_NET_PARAM, 
		strtmp,
		EM_CONFIG_NET_MAIL_RECVMAILBOX,
		para->sAdancePara.szReceiveMailBox,
		sizeof(para->sAdancePara.szReceiveMailBox)))
	{
		return -1;
	}

	if(0 != GetArrayStringFromIni(pFilepath, 
		CONFIG_NET_PARAM, 
		strtmp,
		EM_CONFIG_NET_MAIL_RECVMAILBOX1,
		para->sAdancePara.szReceiveMailBox2,
		sizeof(para->sAdancePara.szReceiveMailBox2)))
	{
		return -1;
	}
	
	// bad 1
	
	if(0 != GetArrayStringFromIni(pFilepath, 
		CONFIG_NET_PARAM, 
		strtmp,
		EM_CONFIG_NET_MAIL_RECVMAILBOX2,
		para->sAdancePara.szReceiveMailBox3,
		sizeof(para->sAdancePara.szReceiveMailBox3)))
	{
		return -1;
	}

	if(0 != GetArrayStringFromIni(pFilepath, 
		CONFIG_NET_PARAM, 
		strtmp,
		EM_CONFIG_NET_MAIL_SNAPPIC,
		para->sAdancePara.szSnapPic,
		sizeof(para->sAdancePara.szSnapPic)))
	{
		return -1;
	}
	
	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_NET_PARAM, 
		strtmp,
		EM_CONFIG_NET_MAIL_SNAPINTERVAL,
		&tmp))
	{
		return -1;
	}
	para->sAdancePara.nSnapIntvl = tmp;
	
	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_NET_PARAM, 
		strtmp,
		EM_CONFIG_NET_CONFIG_DDNS_FLAG,
		&tmp))
	{
		return -1;
	}
	para->DDNSFlag = tmp;
	printf("get ddns flag=%d\n",para->DDNSFlag);
	
	if(0 != GetArrayStringFromIni(pFilepath, 
		CONFIG_NET_PARAM, 
		strtmp,
		EM_CONFIG_NET_CONFIG_DDNS_DOMAIN,
		para->DDNSDomain,
		sizeof(para->DDNSDomain)))
	{
		return -1;
	}
	
	if(0 != GetArrayStringFromIni(pFilepath, 
		CONFIG_NET_PARAM, 
		strtmp,
		EM_CONFIG_NET_CONFIG_DDNS_USER,
		para->DDNSUser,
		sizeof(para->DDNSUser)))
	{
		return -1;
	}
	
	if(0 != GetArrayStringFromIni(pFilepath, 
		CONFIG_NET_PARAM, 
		strtmp,
		EM_CONFIG_NET_CONFIG_DDNS_PASSWD,
		para->DDNSPasswd,
		sizeof(para->DDNSPasswd)))
	{
		return -1;
	}
	
	if(0 != GetArrayStringFromIni(pFilepath, 
		CONFIG_NET_PARAM, 
		strtmp,
		EM_CONFIG_NET_CONFIG_DDNS_DOMAIN,
		para->DDNSDomain,
		sizeof(para->DDNSDomain)))
	{
		return -1;
	}
	
	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_NET_PARAM, 
		strtmp,
		EM_CONFIG_NET_CONFIG_UPDATEINTERVAL,
		&tmp))
	{
		return -1;
	}
	para->UpdateIntvl = tmp;
	printf("ConfigGetNetParam UpdateIntvl=%d\n",para->UpdateIntvl);
	
	//csp modify 20130321
	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_NET_PARAM, 
		strtmp,
		EM_CONFIG_NET_CONFIG_UPNP_FLAG,
		&tmp))
	{
		return -1;
	}
	para->UPNPFlag = tmp;
#endif
	
	//PUBPRT("SPL HERE");
	
	return 0;
}

//NVR used
#define PRO_TYPE_ONVIF		0x01//onvif

#define PRO_TYPE_HAIXIN		0x02//海芯威视
#define PRO_TYPE_JUAN		0x04//九安
#define PRO_TYPE_KLW		0x08//科力维、跃天
#define PRO_TYPE_FULLHAN	0x10//富瀚
#define PRO_TYPE_ANNI		0x20//安尼
#define PRO_TYPE_RIVER		0x40//黄河
#define PRO_TYPE_XM			0x80//雄迈
//yaogang modify 20151222
#define PRO_TYPE_NVR		0X100//搜索并添加NVR

//#define IPC_PROTOCOL_TEST //test record disk error by yaogang 20170222
#ifdef IPC_PROTOCOL_TEST
#define PRO_TYPE_IPC_TEST	0X200//test record disk error by yaogang 20170222
#endif


#define STREAM_TYPE_MAIN	0//主码流
#define STREAM_TYPE_SUB		1//次码流
#define STREAM_TYPE_THIRD	2//子码流

#define TRANS_TYPE_UDP		0
#define TRANS_TYPE_TCP		1

#define IPC_TYPE_D1			0
#define IPC_TYPE_720P		1
#define IPC_TYPE_1080P		2
//NVR used

//NVR used
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

//NVR used
s32 ConfigGetIPCameraParam(SModConfigIPCameraParam* para, s32 nId, s8* pFilepath)
{
	if((para == NULL) || (nId < 0) || (nId >= g_sConfigDvrProperty.nPreviewNum) || (NULL == pFilepath))
	{
		printf("Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}
	
	s32 tmp = 0;
	s32 i = nId;
	s8 strtmp[32];
	
	s8 ipinfo[32];
	s8 strvalue[32];
	
	memset(strtmp, 0, sizeof(strtmp));
	sprintf(strtmp, "%s%d", "IPCameraPara", i);
	
	para->channel_no = nId;
	
	if(0 != IniGetArrayValue(pFilepath,
		"IPCameraPara",
		strtmp,
		0,
		&tmp))
	{
		return -1;
	}
	para->enable = tmp?1:0;
	
	memset(ipinfo, 0, sizeof(ipinfo));
	if(0 != IniGetArrayString(pFilepath,
		"IPCameraPara",
		strtmp,
		1,
		ipinfo,
		sizeof(ipinfo)))
	{
		return -1;
	}
	para->dwIp = inet_addr(ipinfo);
	
	if(0 != IniGetArrayValue(pFilepath,
		"IPCameraPara",
		strtmp,
		2,
		&tmp))
	{
		return -1;
	}
	para->wPort = tmp;
	
	memset(strvalue, 0, sizeof(strvalue));
	if(0 != IniGetArrayString(pFilepath,
		"IPCameraPara",
		strtmp,
		3,
		strvalue,
		sizeof(strvalue)))
	{
		return -1;
	}
	if(strcasecmp(strvalue, "onvif") == 0)
	{
		para->protocol_type = PRO_TYPE_ONVIF;
	}
	else if(strcasecmp(strvalue, "i13") == 0)
	{
		para->protocol_type = PRO_TYPE_HAIXIN;
	}
	else if(strcasecmp(strvalue, "juan") == 0)
	{
		para->protocol_type = PRO_TYPE_JUAN;
	}
	else if(strcasecmp(strvalue, "klw") == 0)
	{
		para->protocol_type = PRO_TYPE_KLW;
	}
	else if(strcasecmp(strvalue, "fh8610") == 0)
	{
		para->protocol_type = PRO_TYPE_FULLHAN;
	}
	else if(strcasecmp(strvalue, "anni") == 0)
	{
		para->protocol_type = PRO_TYPE_ANNI;
	}
	else if(strcasecmp(strvalue, "river") == 0)
	{
		para->protocol_type = PRO_TYPE_RIVER;
	}
	else if(strcasecmp(strvalue, "xm") == 0)
	{
		para->protocol_type = PRO_TYPE_XM;
	}
#ifdef IPC_PROTOCOL_TEST
	else if(strcasecmp(strvalue, "IpcTest") == 0)
	{
		para->protocol_type = PRO_TYPE_IPC_TEST;
	}
#endif
	else
	{
		para->protocol_type = PRO_TYPE_ONVIF;
	}
	
	memset(strvalue, 0, sizeof(strvalue));
	if(0 != IniGetArrayString(pFilepath,
		"IPCameraPara",
		strtmp,
		4,
		strvalue,
		sizeof(strvalue)))
	{
		return -1;
	}
	memset(para->user, 0, sizeof(para->user));
	strcpy(para->user, strvalue);

//yaogang modify 20141207 
//如果IPC密码为空，那么保存5个*到配置文件
//因为如果该字段为空，那么在读取的时候就会
//把下个字段的内容当做密码。
	memset(strvalue, 0, sizeof(strvalue));
	if(0 != IniGetArrayString(pFilepath,
		"IPCameraPara",
		strtmp,
		5,
		strvalue,
		sizeof(strvalue)))
	{
		printf("ipc%d get pwd failed\n", nId);
		return -1;
	}
	else
	{
		if(strcmp(strvalue, "*****") == 0)
		{
			memset(strvalue, 0, sizeof(strvalue));
		}
		//printf("ipc%d get pwd : %s\n", nId, strvalue);
	}
	//memset(para->pwd, 0, sizeof(para->pwd));
	strcpy(para->pwd, strvalue);
	
	//yaogang modify for YueTian private audio sw
	if(0 != IniGetArrayValue(pFilepath,
		"IPCameraPara",
		strtmp,
		6,
		&tmp))
	{
		return -1;
	}
	para->main_audio_sw = tmp?1:0;
	if(0 != IniGetArrayValue(pFilepath,
		"IPCameraPara",
		strtmp,
		7,
		&tmp))
	{
		return -1;
	}
	para->sub_audio_sw = tmp?1:0;

	//printf("yg get main:%d, sub: %d\n", para->main_audio_sw, para->sub_audio_sw);
	//yaogang modify for YueTian private audio sw end
	
	//para->channel_no = nId;
	para->trans_type = TRANS_TYPE_TCP;
	para->stream_type = STREAM_TYPE_MAIN;
	para->ipc_type = IPC_TYPE_720P;
	if(para->protocol_type == PRO_TYPE_ONVIF)
	{
		sprintf(para->address, "http://%s:%d/onvif/device_service", ipinfo, para->wPort);
	}
	
	return 0;
}

//NVR used
s32 ConfigSetIPCameraParam(SModConfigIPCameraParam* para, s32 nId, s8* pFilepath)
{
	if((para == NULL) || (nId < 0) || (nId >= g_sConfigDvrProperty.nPreviewNum) || (NULL == pFilepath))
	{
		printf("Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}
	
	s32 tmp = 0;
	s32 i = nId;
	s8 strtmp[32];
	
	s8 ipinfo[32];
	s8 strvalue[32];
	
	memset(strtmp, 0, sizeof(strtmp));
	sprintf(strtmp, "%s%d", "IPCameraPara", i);
	
	tmp = para->enable;
	if(0 != PublicIniSetArrayValue(pFilepath,
		"IPCameraPara",
		strtmp,
		0,
		tmp))
	{
		return -1;
	}
	
	memset(ipinfo, 0, sizeof(ipinfo));
	struct in_addr host;
	host.s_addr = para->dwIp;
	strcpy(ipinfo, inet_ntoa(host));
	if(0 != PublicIniSetArrayString(pFilepath,
		"IPCameraPara",
		strtmp,
		1,
		ipinfo))
	{
		return -1;
	}
	
	tmp = para->wPort;
	if(0 != PublicIniSetArrayValue(pFilepath,
		"IPCameraPara",
		strtmp,
		2,
		tmp))
	{
		return -1;
	}
	
	memset(strvalue, 0, sizeof(strvalue));
	if(para->protocol_type == PRO_TYPE_ONVIF)
	{
		strcpy(strvalue, "onvif");
	}
	else if(para->protocol_type == PRO_TYPE_HAIXIN)
	{
		strcpy(strvalue, "i13");
	}
	else if(para->protocol_type == PRO_TYPE_JUAN)
	{
		strcpy(strvalue, "juan");
	}
	else if(para->protocol_type == PRO_TYPE_KLW)
	{
		strcpy(strvalue, "klw");
	}
	else if(para->protocol_type == PRO_TYPE_FULLHAN)
	{
		strcpy(strvalue, "fh8610");
	}
	else if(para->protocol_type == PRO_TYPE_ANNI)
	{
		strcpy(strvalue, "anni");
	}
	else if(para->protocol_type == PRO_TYPE_RIVER)
	{
		strcpy(strvalue, "river");
	}
	else if(para->protocol_type == PRO_TYPE_XM)
	{
		strcpy(strvalue, "xm");
	}
#ifdef IPC_PROTOCOL_TEST
	else if(para->protocol_type = PRO_TYPE_IPC_TEST)
	{
		strcpy(strvalue, "IpcTest");
	}
#endif
	else
	{
		strcpy(strvalue, "onvif");
	}
	if(0 != PublicIniSetArrayString(pFilepath,
		"IPCameraPara",
		strtmp,
		3,
		strvalue))
	{
		return -1;
	}
	
	if(0 != PublicIniSetArrayString(pFilepath,
		"IPCameraPara",
		strtmp,
		4,
		para->user))
	{
		return -1;
	}
//yaogang modify 20141207 
//如果IPC密码为空，那么保存5个*到配置文件
//因为如果该字段为空，那么在读取的时候就会
//把下个字段的内容当做密码。
	char tmp_pwd[32] = {"*****"};
	if(strlen(para->pwd) > 0)
	{
		strcpy(tmp_pwd, para->pwd);
	}
	if(0 != PublicIniSetArrayString(pFilepath,
		"IPCameraPara",
		strtmp,
		5,
		tmp_pwd))
	{
		return -1;
	}

	tmp = para->main_audio_sw;
	if(0 != PublicIniSetArrayValue(pFilepath,
		"IPCameraPara",
		strtmp,
		6,
		tmp))
	{
		return -1;
	}

	tmp = para->sub_audio_sw;
	if(0 != PublicIniSetArrayValue(pFilepath,
		"IPCameraPara",
		strtmp,
		7,
		tmp))
	{
		return -1;
	}
	printf("yg set main:%d, sub: %d\n", para->main_audio_sw, para->sub_audio_sw);
	return 0;
}


//Snap
s32 ConfigGetSnapParam(SModConfigSnapChnPara* para, s32 nId, s8* pFilepath)
{
	if((NULL == para) || (nId < 0) || (nId >= g_sConfigDvrProperty.nVidMainNum) || (NULL == pFilepath))
	{
		printf("Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}
	
	s32 tmp = 0;
	s32 i = nId;
	s8 strtmp[30];
	memset(strtmp, 0, sizeof(strtmp));
	sprintf(strtmp, "%s%d", CONFIG_SNAP_PARAM_CHN, i);
	//定时抓图参数
	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_SNAP_PARAM, 
		strtmp, 
		0,
		&tmp))
	{
		printf("%s: CONFIG_SNAP_PARAM  0 failed\n", __FUNCTION__);
		return -1;
	}
	//printf("%s: CONFIG_SNAP_PARAM: %d\n", __FUNCTION__, tmp);
	para->TimeSnap.Enable= tmp;
	
	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_SNAP_PARAM, 
		strtmp, 
		1,
		&tmp))
	{
		printf("%s: CONFIG_SNAP_PARAM 1 failed\n", __FUNCTION__);
		return -1;
	}
	//printf("%s: CONFIG_SNAP_PARAM: %d\n", __FUNCTION__, tmp);
	para->TimeSnap.Res = tmp;

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_SNAP_PARAM, 
		strtmp, 
		2,
		&tmp))
	{
		printf("%s: CONFIG_SNAP_PARAM 2 failed\n", __FUNCTION__);
		return -1;
	}
	//printf("%s: CONFIG_SNAP_PARAM: %d\n", __FUNCTION__, tmp);
	para->TimeSnap.Quality = tmp;

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_SNAP_PARAM, 
		strtmp, 
		3,
		&tmp))
	{
		printf("%s: CONFIG_SNAP_PARAM 3 failed\n", __FUNCTION__);
		return -1;
	}
	//printf("%s: CONFIG_SNAP_PARAM: %d\n", __FUNCTION__, tmp);
	para->TimeSnap.Interval= tmp;

	//事件抓图参数
	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_SNAP_PARAM, 
		strtmp, 
		4,
		&tmp))
	{
		printf("%s: CONFIG_SNAP_PARAM 4 failed\n", __FUNCTION__);
		return -1;
	}
	//printf("%s: CONFIG_SNAP_PARAM: %d\n", __FUNCTION__, tmp);
	para->EventSnap.Enable= tmp;
	
	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_SNAP_PARAM, 
		strtmp, 
		5,
		&tmp))
	{
		printf("%s: CONFIG_SNAP_PARAM 5 failed\n", __FUNCTION__);
		return -1;
	}
	//printf("%s: CONFIG_SNAP_PARAM: %d\n", __FUNCTION__, tmp);
	para->EventSnap.Res = tmp;

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_SNAP_PARAM, 
		strtmp, 
		6,
		&tmp))
	{
		printf("%s: CONFIG_SNAP_PARAM 6 failed\n", __FUNCTION__);
		return -1;
	}
	//printf("%s: CONFIG_SNAP_PARAM: %d\n", __FUNCTION__, tmp);
	para->EventSnap.Quality = tmp;

	if(0 != IniGetArrayValue(pFilepath, 
		CONFIG_SNAP_PARAM, 
		strtmp, 
		7,
		&tmp))
	{
		printf("%s: CONFIG_SNAP_PARAM 7 failed\n", __FUNCTION__);
		return -1;
	}
	//printf("%s: CONFIG_SNAP_PARAM: %d\n", __FUNCTION__, tmp);
	para->EventSnap.Interval= tmp;
	
	return 0;
}
s32 ConfigSetSnapParam(SModConfigSnapChnPara* para, s32 nId, s8* pFilepath)
{
	if((para == NULL) || (nId < 0) || (nId >= g_sConfigDvrProperty.nPreviewNum) || (NULL == pFilepath))
	{
		printf("Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}
	
	s32 tmp = 0;
	s32 i = nId;
	s8 strtmp[32];
	
	memset(strtmp, 0, sizeof(strtmp));
	sprintf(strtmp, "%s%d", CONFIG_SNAP_PARAM_CHN, i);

	tmp = para->TimeSnap.Enable;
	if(0 != PublicIniSetArrayValue(pFilepath,
		CONFIG_SNAP_PARAM,
		strtmp,
		0,
		tmp))
	{
		return -1;
	}
	
	tmp = para->TimeSnap.Res;
	if(0 != PublicIniSetArrayValue(pFilepath,
		CONFIG_SNAP_PARAM,
		strtmp,
		1,
		tmp))
	{
		return -1;
	}
	//printf("%s: CONFIG_SNAP_PARAM: %d\n", __FUNCTION__, tmp);

	tmp = para->TimeSnap.Quality;
	if(0 != PublicIniSetArrayValue(pFilepath,
		CONFIG_SNAP_PARAM,
		strtmp,
		2,
		tmp))
	{
		return -1;
	}
	//printf("%s: CONFIG_SNAP_PARAM: %d\n", __FUNCTION__, tmp);

	tmp = para->TimeSnap.Interval;
	if(0 != PublicIniSetArrayValue(pFilepath,
		CONFIG_SNAP_PARAM,
		strtmp,
		3,
		tmp))
	{
		return -1;
	}
	//printf("%s: CONFIG_SNAP_PARAM: %d\n", __FUNCTION__, tmp);

	tmp = para->EventSnap.Enable;
	if(0 != PublicIniSetArrayValue(pFilepath,
		CONFIG_SNAP_PARAM,
		strtmp,
		4,
		tmp))
	{
		return -1;
	}
	
	tmp = para->EventSnap.Res;
	if(0 != PublicIniSetArrayValue(pFilepath,
		CONFIG_SNAP_PARAM,
		strtmp,
		5,
		tmp))
	{
		return -1;
	}
	//printf("%s: CONFIG_SNAP_PARAM: %d\n", __FUNCTION__, tmp);

	tmp = para->EventSnap.Quality;
	if(0 != PublicIniSetArrayValue(pFilepath,
		CONFIG_SNAP_PARAM,
		strtmp,
		6,
		tmp))
	{
		return -1;
	}
	//printf("%s: CONFIG_SNAP_PARAM: %d\n", __FUNCTION__, tmp);

	tmp = para->EventSnap.Interval;
	if(0 != PublicIniSetArrayValue(pFilepath,
		CONFIG_SNAP_PARAM,
		strtmp,
		7,
		tmp))
	{
		return -1;
	}
	//printf("%s: CONFIG_SNAP_PARAM: %d\n", __FUNCTION__, tmp);

	return 0;
}

//yaogang modify for bad disk
s32 ConfigGetBadDiskParam(SModConfigBadDisk* para, s32 nId, s8* pFilepath)
{
	if((para == NULL) || (nId < 0) || (nId >= g_sConfigDvrProperty.nMaxHddNum) || (NULL == pFilepath))
	{
		printf("Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}
	
	s32 tmp = 0;
	s32 i = nId;
	s8 strtmp[32];
	
	s8 strvalue[64];
	
	memset(strtmp, 0, sizeof(strtmp));
	sprintf(strtmp, "%s%d", CONFIG_BAD_DISK_PARAM_LINE, i);
	
	if(0 != IniGetArrayValue(pFilepath,
		CONFIG_BAD_DISK_PARAM,
		strtmp,
		0,
		&tmp))
	{
		return -1;
	}
	para->time = (u32)tmp;
	
	memset(strvalue, 0, sizeof(strvalue));
	if(0 != IniGetArrayString(pFilepath,
		CONFIG_BAD_DISK_PARAM,
		strtmp,
		1,
		strvalue,
		sizeof(strvalue)))
	{
		return -1;
	}

	strcpy(para->disk_sn, strvalue);
	
	
	return 0;
}

s32 ConfigSetBadDiskParam(SModConfigBadDisk* para, s32 nId, s8* pFilepath)
{
	if((para == NULL) || (nId < 0) || (nId >= g_sConfigDvrProperty.nMaxHddNum) || (NULL == pFilepath))
	{
		printf("Invalid para! function: %s\n", __FUNCTION__);
		return -1;
	}
	
	s32 tmp = 0;
	s32 i = nId;
	s8 strtmp[32];
	
	//s8 strvalue[64];
	
	memset(strtmp, 0, sizeof(strtmp));
	sprintf(strtmp, "%s%d", CONFIG_BAD_DISK_PARAM_LINE, i);
	
	tmp = para->time;
	if(0 != PublicIniSetArrayValue(pFilepath,
		CONFIG_BAD_DISK_PARAM,
		strtmp,
		0,
		tmp))
	{
		return -1;
	}
	
	if(0 != PublicIniSetArrayString(pFilepath,
		CONFIG_BAD_DISK_PARAM,
		strtmp,
		1,
		para->disk_sn))
	{
		return -1;
	}
	
	return 0;
}


/**************************END************************************/

