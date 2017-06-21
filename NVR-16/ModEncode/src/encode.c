#include <semaphore.h>
#include "encode_manager.h"
#include "lib_audio.h"
#include <sys/syscall.h>
#include <unistd.h>

#include <sys/time.h>
#include "lib_snapshot.h"
#include "mod_config.h"


//csp modify 20130429
#if 0//#if defined(CHIP_HISI3531)
#include "hi_tde_api.h"
typedef struct
{
	u32	u32PhyAddr;
	u32	u32VirAddr;
	u32	u32Len;
}SMmzBuffer;
static SMmzBuffer g_osdbuf1 = {0,0,0};
static SMmzBuffer g_osdbuf2 = {0,0,0};
static pthread_mutex_t g_tde2_lock = PTHREAD_MUTEX_INITIALIZER;
#endif

//csp modify 20130423
extern void tl_venc_update_basetime(void);

//csp modify 20130423
//#if defined(CHIP_HISI3531)
int tl_venc_third_start(int chn, venc_size_e size, venc_parameter_t *para);
int tl_venc_third_stop(int chn);
int tl_venc_third_read(unsigned char *in_buf, unsigned int in_len, venc_stream_s *pout_stream);
//#endif

//csp modify 20130423
static SThirdStreamProperty g_sThirdStreamProperty;

//typedef struct _sAudioCapArg
//{
//	SChnManage* pPreviewAudioManage;
//	SChnManage* pTalkStreamManage;
//}SAudioCapArgp;

//static SAudioCapArgp sPArgIns;

static SEncodeManage g_s_EncodeManager;

//s32 ret_main, ret_sub, ret_audio;

//pthread_t main_str_cap;
//pthread_t sub_str_cap;
//pthread_t audio_str_cap;

//csp modify 20130423
//pthread_t third_str_cap;
//s32 ret_third = -1;

typedef struct
{
	float xScale;
	float yScale;
}SResoScale;

#define QCIF_EQUALS_CIF

#ifdef QCIF_EQUALS_CIF
//csp modify
//curReso/refReso table
//SResoScale scaleTable[7][7] = 
SResoScale scaleTable[8][8] = 
{
#if defined(CHIP_HISI3521)
	//{{1,1},{1/1,1/1},{1/2.0,1/1.0},{1/2.0,1/2.0},{1/2.0,1/2.0},{352/1280.0,288/720.0},{352/1920.0,288/1080.0}, {352/960.0,288/576.0}},
	{{1,1},{1/2.0,1/2.0},{1/4.0,1/2.0},{1/4.0,1/4.0},{1/4.0,1/4.0},{176/1280.0,144/720.0},{176/1920.0,144/1080.0}, {176/960.0,144/576.0}},
	
	//{{1,1},{1,1},{1/2.0,1},{1/2.0,1/2.0},{1/2.0,1/2.0},{352/1280.0,288/720.0},{352/1920.0,288/1080.0}, {352/960.0,288/576.0}},
	{{2,2},{1,1},{1/2.0,1},{1/2.0,1/2.0},{1/2.0,1/2.0},{352/1280.0,288/720.0},{352/1920.0,288/1080.0}, {352/960.0,288/576.0}},
	
	//{{2,1},{2,1},{1,1},{1,1/2.0},{1,1/2.0},{704/1280.0,288/720.0},{704/1920.0,288/1080.0}, {704/960.0,288/576.0}},
	{{4,2},{2,1},{1,1},{1,1/2.0},{1,1/2.0},{704/1280.0,288/720.0},{704/1920.0,288/1080.0}, {704/960.0,288/576.0}},
	
	//{{2,2},{2,2},{1,2},{1,1},{1,1},{704/1280.0,576/720.0},{704/1920.0,576/1080.0}, {704/960.0,576/576.0}},
	{{4,4},{2,2},{1,2},{1,1},{1,1},{704/1280.0,576/720.0},{704/1920.0,576/1080.0}, {704/960.0,576/576.0}},
	
	//{{2,2},{2,2},{1,2},{1,1},{1,1},{704/1280.0,576/720.0},{704/1920.0,576/1080.0}, {704/960.0,576/576.0}},
	{{4,4},{2,2},{1,2},{1,1},{1,1},{704/1280.0,576/720.0},{704/1920.0,576/1080.0}, {704/960.0,576/576.0}},
	
	{{1280/176.0,720/144.0},{1280/352.0,720/288.0},{1280/704.0,720/288.0},{1280/704.0,720/576.0},{1280/704.0,720/576.0},{1,1},{1280/1920.0,720/1080.0}, {1280/960.0,720/576.0}},
	{{1920/176.0,1080/144.0},{1920/352.0,1080/288.0},{1920/704.0,1080/288.0},{1920/704.0,1080/576.0},{1920/704.0,1080/576.0},{1920/1280.0,1080/720.0},{1,1}, {1920/960.0,1080/576.0}},
	
	//csp modify
	{{960/176.0,576/144.0},{960/352.0,576/288.0},{960/704.0,576/288.0},{960/704.0,576/576.0},{960/704.0,576/576.0},{960/1280.0,576/720.0},{960/1920.0,576/1080.0},{1,1}},
#else
	{{1,1},{1/1,1/1},{1/2.0,1/1.0},{1/2.0,1/2.0},{1/2.0,1/2.0},{352/1280.0,288/720.0},{352/1920.0,288/1080.0}, {352/960.0,288/576.0}},	
	{{1,1},{1,1},{1/2.0,1},{1/2.0,1/2.0},{1/2.0,1/2.0},{352/1280.0,288/720.0},{352/1920.0,288/1080.0}, {352/960.0,288/576.0}},
	{{2,1},{2,1},{1,1},{1,1/2.0},{1,1/2.0},{704/1280.0,288/720.0},{704/1920.0,288/1080.0}, {704/960.0,288/576.0}},	
	{{2,2},{2,2},{1,2},{1,1},{1,1},{704/1280.0,576/720.0},{704/1920.0,576/1080.0}, {704/960.0,576/576.0}},
	{{2,2},{2,2},{1,2},{1,1},{1,1},{704/1280.0,576/720.0},{704/1920.0,576/1080.0}, {704/960.0,576/576.0}},
	{{1280/352.0,720/288.0},{1280/352.0,720/288.0},{1280/704.0,720/288.0},{1280/704.0,720/576.0},{1280/704.0,720/576.0},{1,1},{1280/1920.0,720/1080.0}, {1280/960.0,720/576.0}},
	{{1920/352.0,1080/288.0},{1920/352.0,1080/288.0},{1920/704.0,1080/288.0},{1920/704.0,1080/576.0},{1920/704.0,1080/576.0},{1920/1280.0,1080/720.0},{1,1}, {1920/960.0,1080/576.0}},
	//csp modify
	{{960/352.0,576/288.0},{960/352.0,576/288.0},{960/704.0,576/288.0},{960/704.0,576/576.0},{960/704.0,576/576.0},{960/1280.0,576/720.0},{960/1920.0,576/1080.0},{1,1}},
#endif
};
#else
//curReso/refReso table
SResoScale scaleTable[5][5] = 
{
	{{1,1},{1/2.0,1/2.0},{1/4.0,1/2.0},{1/4.0,1/4.0},{1/4.0,1/4.0}},	
	{{2,2},{1,1},{1/2.0,1},{1/2.0,1/2.0},{1/2.0,1/2.0}},
	{{4,2},{2,1},{1,1},{1,1/2.0},{1,1/2.0}},	
	{{4,4},{2,2},{1,2},{1,1},{1,1}},
	{{4,4},{2,2},{1,2},{1,1},{1,1}},
};
#endif

#define PRINTF printf("func:%s, line:%d\n", __FUNCTION__, __LINE__);

s32 GetRealCoord(s32 nChn, s32 x, s32 y, s32* rX, s32* rY, s32* rXSub, s32* rYSub)
{
	EMVIDEORESOLUTION emRefReso
		= g_s_EncodeManager.sOsdManage.emResoRef[nChn];
	
	EMVIDEORESOLUTION emCurReso
		= g_s_EncodeManager.sChnMainManage.psChnObj[nChn].sEncodeParam.sVideoMainParam.emVideoResolution;
	
	EMVIDEORESOLUTION emCurResoSub
		= g_s_EncodeManager.sChnSubManage.psChnObj[nChn].sEncodeParam.sVideoSubParam.emVideoResolution;
	
	SResoScale scale = scaleTable[emCurReso][emRefReso];
	*rX = scale.xScale*x;
	*rY = scale.yScale*y;
	
	SResoScale scaleSub = scaleTable[emCurResoSub][emRefReso];
	*rXSub = scaleSub.xScale*x;
	*rYSub = scaleSub.yScale*y;
	
	//if(nChn == 0)
	//{
	//	printf("GetRealCoord resolution:(%d,%d,%d),scale:(%f,%f),scaleSub:(%f,%f)\n",
	//			emRefReso,emCurReso,emCurResoSub,scale.xScale,scale.yScale,scaleSub.xScale,scaleSub.yScale);
	//}
	
	//#if defined(CHIP_HISI3531)//csp modify 20130429
	//*rX = (*rX + 15) / 16 * 16;
	//*rY = (*rY + 15) / 16 * 16;
	//*rXSub = (*rXSub + 15) / 16 * 16;
	//*rYSub = (*rYSub + 15) / 16 * 16;
	//#endif
	
	return 0;//csp modify
}

//csp modify 20130423
#if defined(CHIP_HISI3531)// || defined(CHIP_HISI3521)
s32 GetThirdRealCoord(s32 nChn, s32 x, s32 y, s32* rXThird, s32* rYThird)
{
	EMVIDEORESOLUTION emRefReso
		= g_s_EncodeManager.sOsdManage.emResoRef[nChn];
	
	EMVIDEORESOLUTION emCurReso
		= g_s_EncodeManager.sChnThirdManage.psChnObj[nChn].sEncodeParam.sVideoMainParam.emVideoResolution;
	
	SResoScale scale = scaleTable[emCurReso][emRefReso];
	*rXThird = scale.xScale*x;
	*rYThird = scale.yScale*y;
	
	//#if defined(CHIP_HISI3531)//csp modify 20130429
	//*rXThird = (*rXThird + 15) / 16 * 16;
	//*rYThird = (*rYThird + 15) / 16 * 16;
	//#endif
	
	return 0;
}
#endif

s32 ModEncodeInit(SEncodeCapbility* psEncodeCap)
{
	//s32 i;
	
	s32 ret_main_crt = -1;
	s32 ret_sub_crt = -1;
	s32 ret_audio_crt = -1;
	s32 ret_snap_crt = -1;
	s32 ret_third_crt = -1;//csp modify 20130423
	
	memset(&g_s_EncodeManager,0,sizeof(SEncodeManage));
	
	g_s_EncodeManager.emBoardType = psEncodeCap->emBoardType;
	
	//csp modify 20130106
	g_s_EncodeManager.EncExceptCB = psEncodeCap->EncExceptCB;
	
	//csp modify 20130316
	g_s_EncodeManager.nVideoStandard = psEncodeCap->nVideoStandard;
	printf("ModEncodeInit VideoStandard=%d\n",g_s_EncodeManager.nVideoStandard);
	
	if(psEncodeCap->nMainNum > 0)
	{
		ret_main_crt = CreateMainStreamObj(&g_s_EncodeManager.sChnMainManage, psEncodeCap->nMainNum, psEncodeCap->nMainMaxFrameSize);
		
		if(-1 == ret_main_crt)
		{
			return -1;
		}
	}
	
	//csp modify 20130423
	if(ModEncodeSupportThirdStream())
	{
		if(psEncodeCap->nMainNum > 0)
		{
			//csp modify 20140318
			//ret_third_crt = CreateThirdStreamObj(&g_s_EncodeManager.sChnThirdManage, psEncodeCap->nMainNum, 256*1024);
			//ret_third_crt = CreateThirdStreamObj(&g_s_EncodeManager.sChnThirdManage, psEncodeCap->nMainNum, 80*1024);
			//csp modify 20140406
			ret_third_crt = CreateThirdStreamObj(&g_s_EncodeManager.sChnThirdManage, psEncodeCap->nMainNum, psEncodeCap->nSubMaxFrameSize);
			
			if(-1 == ret_third_crt)
			{
				return -1;
			}
		}
	}
	
	if(psEncodeCap->nSubNum > 0)
	{
		ret_sub_crt = CreateSubStreamObj(&g_s_EncodeManager.sChnSubManage, psEncodeCap->nSubNum, psEncodeCap->nSubMaxFrameSize);
		
		if (-1 == ret_sub_crt)
		{
			DestroyMainStreamObj(&g_s_EncodeManager.sChnMainManage);
			
			return -1;
		}
	}
	
	//printf("ModEncodeInit nSnapNum=%d,nSnapMaxFrameSize=%d\n",psEncodeCap->nSnapNum,psEncodeCap->nSnapMaxFrameSize);
	
	if(psEncodeCap->nSnapNum > 0)
	{
		ret_snap_crt = CreateSnapObj(&g_s_EncodeManager.sChnSnapManage, psEncodeCap->nSnapNum, psEncodeCap->nSnapMaxFrameSize);
		
		if (-1 == ret_snap_crt)
		{
			DestroyMainStreamObj(&g_s_EncodeManager.sChnMainManage);
			DestroySubStreamObj(&g_s_EncodeManager.sChnSubManage);
			
			return -1;
		}
	}
	
	if(psEncodeCap->nAudioNum > 0)
	{
		ret_audio_crt = CreateAudioObj(&g_s_EncodeManager.sChnAudioManage, psEncodeCap->nAudioNum, psEncodeCap->nAudioMaxFrameSize);
		
		if (-1 == ret_audio_crt)
		{
			DestroyMainStreamObj(&g_s_EncodeManager.sChnMainManage);
			DestroySubStreamObj(&g_s_EncodeManager.sChnSubManage);
			DestroySnapObj(&g_s_EncodeManager.sChnSnapManage);
			
			return -1;
		}
	}
	
	if(psEncodeCap->nTalkNum > 0)
	{
		ret_audio_crt = CreateAudioObj(&g_s_EncodeManager.sChnTalkManage, psEncodeCap->nTalkNum, psEncodeCap->nTalkMaxFrameSize);
		
		if (-1 == ret_audio_crt)
		{
			DestroyMainStreamObj(&g_s_EncodeManager.sChnMainManage);
			DestroySubStreamObj(&g_s_EncodeManager.sChnSubManage);
			DestroySnapObj(&g_s_EncodeManager.sChnSnapManage);
			DestroySnapObj(&g_s_EncodeManager.sChnAudioManage);
			
			return -1;
		}
	}
	
	if(psEncodeCap->nMainNum > 0 || psEncodeCap->nSubNum > 0)
	{
		if(0 != tl_venc_open())
		{
			printf("venc open error!!!!!!\n");
		}
		else
		{
			printf("venc open success......\n");
		}
	}
	else
	{
		printf("psEncodeCap->nMainNum %d psEncodeCap->nSubNum %d\n",
					psEncodeCap->nMainNum,
					psEncodeCap->nSubNum);
	}
	
	//fflush(stdout);
	//sleep(10);
	
	if(psEncodeCap->nAudioNum > 0 || psEncodeCap->nTalkNum > 0)
	{
		//fflush(stdout);
		//printf("audio open start\n");
		//fflush(stdout);
		
		tl_audio_open();
		
		//tl_audio_set_volume(10);/*需要传参*/
		
		//fflush(stdout);
		//printf("audio open finish\n");
		//fflush(stdout);
	}
	
	//fflush(stdout);
	//sleep(10);
	
	if (0 == ret_main_crt)
	{
		InitMainStreamObj(&g_s_EncodeManager.sChnMainManage);
	}
	
	//csp modify 20130423
	if(ModEncodeSupportThirdStream())
	{
		if (0 == ret_third_crt)
		{
			InitThirdStreamObj(&g_s_EncodeManager.sChnThirdManage);
		}
	}
	
	if (0 == ret_sub_crt)
	{
		//printf("InitSubStreamObj......\n");
		InitSubStreamObj(&g_s_EncodeManager.sChnSubManage);
	}
	
	if (0 == ret_snap_crt)
	{
		InitSnapObj(&g_s_EncodeManager.sChnSnapManage);
	}
	
	if (0 == ret_audio_crt)
	{
		InitAudioStreamObj(&g_s_EncodeManager.sChnAudioManage);
	}
	
	if (0 == ret_audio_crt)
	{
		InitTalkStreamObj(&g_s_EncodeManager.sChnTalkManage);
	}
	
	pthread_t status_pid;
	if(0 != pthread_create(&status_pid, NULL, (void*)EncodeStatusDealFxn, &g_s_EncodeManager))
	{
		printf("error: pthread_create,  func:%s\n", __FUNCTION__);		
		return (-1);
	}
	
#if 0//NVR used
	if(0 != (ret_main = pthread_create(&main_str_cap, NULL, MainStreamCap, &g_s_EncodeManager.sChnMainManage)))
	{
		printf("error: %s\n", strerror(ret_main));
		
		return (-1);
	}
	printf("MainStreamCap id : %d\n", (int)main_str_cap);
	
	//csp modify 20130423
	if(ModEncodeSupportThirdStream())
	{
		if(0 != (ret_third = pthread_create(&third_str_cap, NULL, ThirdStreamCap, &g_s_EncodeManager.sChnThirdManage)))
		{
			printf("error: %s\n", strerror(ret_third));
			
			return (-1);
		}
		printf("ThirdStreamCap id : %d\n", (int)third_str_cap);
	}
	
	if(0 != (ret_sub = pthread_create(&sub_str_cap, NULL, SubStreamCap, &g_s_EncodeManager.sChnSubManage)))
	{
		//pthread_exit(&main_str_cap);
		
		printf("error: %s\n", strerror(ret_sub));
		
		return (-1);
	}
	printf("sub_str_cap id : %d\n", (int)sub_str_cap);
#endif
	
#if 0//NVR used
	sPArgIns.pPreviewAudioManage = &g_s_EncodeManager.sChnAudioManage;
	sPArgIns.pTalkStreamManage = &g_s_EncodeManager.sChnTalkManage;
	if(0 != (ret_audio = pthread_create(&audio_str_cap, NULL, AudioStreamCap, &g_s_EncodeManager.sChnAudioManage)))
	{
		//pthread_exit(&main_str_cap);
		//pthread_exit(&sub_str_cap);
		
		printf("error: %s\n", strerror(ret_audio));
		
		return (-1);
	}
#endif
	
#if 0//NVR used
	if(psEncodeCap->emBoardType == EM_ENCODE_BOARD_HISI3520_16D1)
	{
		pthread_t pid;
		if(0 != pthread_create(&pid, NULL, MainStreamCapSlaver, &g_s_EncodeManager.sChnMainManage))
		{
			return -1;
		}
		
		if(0 != pthread_create(&pid, NULL, SubStreamCapSlaver, &g_s_EncodeManager.sChnSubManage))
		{
			return -1;
		}
	}
#endif
	
	if (psEncodeCap->nMainNum > 0)
	{
		ret_main_crt = CreateOsdObj(&g_s_EncodeManager.sOsdManage, psEncodeCap->nMainNum);
		
		if (-1 == ret_main_crt)
		{
			return -1;
		}
	}
	
	tl_venc_update_basetime();
	
	//NVR used
	tl_venc_set_cb(DealMainStream, DealSubStream, DealThirdStream);
	tl_audio_set_cb(DealAudioStream);
	
	printf("encode mod init succesful!!\n");
	
	//fflush(stdout);
	//sleep(10);
	
	//int i = 0;
	//for(i=0;i<g_s_EncodeManager.sChnSubManage.nChnNum;i++)
	//{
	//	printf("ModEncodeInit chn%d gop:%d\n",i,g_s_EncodeManager.sChnSubManage.psChnObj[i].sEncodeParam.sVideoSubParam.nGop);
	//}
	
	return 0;
}

s32 ModEncodeDeinit(void)
{
	DestroyMainStreamObj(&g_s_EncodeManager.sChnMainManage);
	DestroySubStreamObj(&g_s_EncodeManager.sChnSubManage);
	DestroySnapObj(&g_s_EncodeManager.sChnSnapManage);
	DestroyAudioObj(&g_s_EncodeManager.sChnAudioManage);
	DestroyOsdObj(&g_s_EncodeManager.sOsdManage);
	
	//pthread_exit(&main_str_cap);
	//pthread_exit(&sub_str_cap);
	//pthread_exit(&audio_str_cap);
	
	printf("encode mod deinit successful!!\n");
	
	return 0;
}

s32 CreateMainStreamObj(SChnManage* psChnMainManage, u8 nChnNum, u32 nMaxFrameSize)//zlbfix
{
	if(psChnMainManage && nChnNum > 0)
	{
		psChnMainManage->nChnNum = nChnNum;
		
		//csp modify 20121116
		if(nMaxFrameSize > 500 * 1024)
		{
			nMaxFrameSize = 500 * 1024;
		}
		//yaogang modify 20150331
		//psChnMainManage->nMaxFrameSize = nMaxFrameSize;//zlbfix
		psChnMainManage->nMaxFrameSize = 640 * 1024;
		
		psChnMainManage->nIsEncoding = 0;
		psChnMainManage->nMasterWorking = 0;
		psChnMainManage->nSlaverWorking = 0;
		if(0 != sem_init(&psChnMainManage->nSemVencReadOrStart, 0, 1))
		{
			printf("sem_init fail!\n");
			return -1;
		}
		
		if((psChnMainManage->psChnObj = calloc(sizeof(SChnObj), nChnNum)) != NULL)
		{
			//printf("malloc main stream object success!\n");
		}
		else
		{
			printf("malloc main stream object fail!\n");
			return -1;
		}
		printf("create main stream object success!\n");
		return 0;
	}
	else
	{
		printf("create main stream object fail!\n");
		return 1;
	}
}

//csp modify 20130423
s32 CreateThirdStreamObj(SChnManage* psChnThirdManage, u8 nChnNum, u32 nMaxFrameSize)
{
	if(psChnThirdManage && nChnNum > 0)
	{
		psChnThirdManage->nChnNum = nChnNum;
		
		if(nMaxFrameSize > 500 * 1024)
		{
			nMaxFrameSize = 500 * 1024;
		}
		
		psChnThirdManage->nMaxFrameSize = nMaxFrameSize;
		
		psChnThirdManage->nIsEncoding = 0;
		psChnThirdManage->nMasterWorking = 0;
		psChnThirdManage->nSlaverWorking = 0;
		if(0 != sem_init(&psChnThirdManage->nSemVencReadOrStart, 0, 1))
		{
			printf("sem_init fail!\n");
			return -1;
		}
		
		if((psChnThirdManage->psChnObj = calloc(sizeof(SChnObj), nChnNum)) != NULL)
		{
			//printf("malloc third stream object success!\n");
		}
		else
		{
			printf("malloc third stream object fail!\n");
			return -1;
		}
		printf("create third stream object success!\n");
		return 0;
	}
	else
	{
		printf("create third stream object fail!\n");
		return 1;
	}
}

s32 DestroyMainStreamObj(SChnManage* psChnMainManage)
{
	if(psChnMainManage)
	{
	    free(psChnMainManage->psChnObj);
        psChnMainManage->psChnObj = NULL;
        
		if(NULL == psChnMainManage->psChnObj)
		{
			printf("free main stream object success!\n");
		}
		else
		{
			printf("free main stream object fail!\n");
			
			return -1;
		}
		
		printf("destroy main stream object success!\n");
		
		return 0;
	}
	else
	{
		printf("destroy main stream fail!\n");
		
		return 1;
	}
}

s32 CreateSubStreamObj(SChnManage* psChnSubManage, u8 nChnNum, u32 nMaxFrameSize)//zlbfix
{
	if(psChnSubManage && nChnNum> 0)
	{
		psChnSubManage->nChnNum = nChnNum;
		psChnSubManage->nMaxFrameSize = nMaxFrameSize;//zlbfix
		
		if(0 != sem_init(&psChnSubManage->nSemVencReadOrStart, 0, 1))
		{
			printf("sem_init fail!\n");
			return -1;
		}
		
		if((psChnSubManage->psChnObj = calloc(sizeof(SChnObj), nChnNum)) != NULL)
		{
			//printf("malloc sub stream object success!\n");
		}
		else
		{
			printf("malloc sub stream object fail!\n");
			
			return -1;
		}
		
		printf("create sub stream object success!\n");
		
		return 0;
	}
	else
	{
		printf("create sub stream object fail!\n");
		
		return 1;
	}
}

s32 DestroySubStreamObj(SChnManage* psChnSubManage)
{
	if(psChnSubManage)
	{
		free(psChnSubManage->psChnObj);
		psChnSubManage->psChnObj = NULL;
		
		if(NULL == psChnSubManage->psChnObj)
		{
			printf("free sub stream object success!\n");
		}
		else
		{
			printf("free sub stream object fail!\n");
			
			return -1;
		}
		
		printf("destroy sub stream object success!\n");
		
		return 0;
	}
	else
	{
		printf("destroy sub stream object fail!\n");
		
		return 1;
	}
}

s32 CreateSnapObj(SChnManage* psChnSnapManage, u8 nChnNum, u32 nMaxFrameSize)//zlbfix
{
	if(psChnSnapManage && nChnNum> 0)
	{
		psChnSnapManage->nChnNum = nChnNum;
		psChnSnapManage->nMaxFrameSize = nMaxFrameSize;//zlbfix
		
		if((psChnSnapManage->psChnObj = calloc(sizeof(SChnObj), nChnNum)) != NULL)
		{
			//printf("malloc snap object success!\n");
		}
		else
		{
			printf("malloc snap object fail!\n");
			
			return -1;
		}
		
		printf("create snap object success!\n");
		
		return 0;
	}
	else
	{
		printf("create snap object fail!\n");
		
		return 1;
	}
}

s32 DestroySnapObj(SChnManage* psChnSnapManage)
{
	if(psChnSnapManage)
	{
		free(psChnSnapManage->psChnObj);
		psChnSnapManage->psChnObj = NULL;
		
		if(NULL == psChnSnapManage->psChnObj)
		{
			//printf("free snap object success!\n");
		}
		else
		{
			printf("free snap object fail!\n");
			
			return -1;
		}
		
		printf("destroy snap object success!\n");
		
		return 0;
	}
	else
	{
		printf("destroy snap object fail!\n");
		
		return 1;
	}
}

s32 CreateAudioObj(SChnManage* psChnAudioManage, u8 nChnNum, u32 nMaxFrameSize)//zlbfix
{
	if(psChnAudioManage && nChnNum> 0)
	{
		psChnAudioManage->nChnNum = nChnNum;
		psChnAudioManage->nMaxFrameSize = nMaxFrameSize;//zlbfix
		
		if((psChnAudioManage->psChnObj = calloc(sizeof(SChnObj), nChnNum)) != NULL)
		{
			//printf("malloc audio object success!\n");
		}
		else
		{
			printf("malloc audio object fail!\n");
			
			return -1;
		}
		
		printf("create audio object success!\n");
		
		return 0;
	}
	else
	{
		printf("create audio object fail!\n");
		
		return 1;
	}
}

s32 CreateTalkObj(SChnManage* psChnTalkManage, u8 nChnNum, u32 nMaxFrameSize)
{
	if(psChnTalkManage && nChnNum> 0)
	{
		psChnTalkManage->nChnNum = nChnNum;
		psChnTalkManage->nMaxFrameSize = nMaxFrameSize;//zlbfix
		
		if((psChnTalkManage->psChnObj = calloc(sizeof(SChnObj), nChnNum)) != NULL)
		{
			//printf("malloc talk object success!\n");
		}
		else
		{
			printf("malloc talk object fail!\n");
			
			return -1;
		}
		
		printf("create talk object success!\n");
		
		return 0;
	}
	else
	{
		printf("create talk object fail!\n");
		
		return 1;
	}
}

s32 DestroyAudioObj(SChnManage* psChnAudioManage)
{
	if(psChnAudioManage)
	{
		free(psChnAudioManage->psChnObj);
		psChnAudioManage->psChnObj = NULL;
		
		if(NULL == psChnAudioManage->psChnObj)
		{
			printf("free audio object success!\n");
		}
		else
		{
			printf("free audio object fail!\n");
			
			return -1;
		}
		
		printf("destroy audio object success!\n");
		
		return 0;
	}
	else
	{
		printf("destroy audio object fail!\n");
		
		return 1;
	}
}

s32 InitMainStreamObj(SChnManage* psChnMainManage)
{
	int i;
	
	for(i = 0; i < psChnMainManage->nChnNum; i++)
	{
		psChnMainManage->psChnObj[i].sEncodeParam.nForce = 0;
		
		psChnMainManage->psChnObj[i].sEncodeParam.sVideoMainParam.nFrameRate = 25;
		psChnMainManage->psChnObj[i].sEncodeParam.sVideoMainParam.nBitRate = 512;
		psChnMainManage->psChnObj[i].sEncodeParam.sVideoMainParam.nGop = 50;//40;//csp modify
		psChnMainManage->psChnObj[i].sEncodeParam.sVideoMainParam.nMaxQP = 40;
		psChnMainManage->psChnObj[i].sEncodeParam.sVideoMainParam.nMinQP = 1;
		psChnMainManage->psChnObj[i].sEncodeParam.sVideoMainParam.emPicLevel = EM_PIC_LEVEL_BEST;
		psChnMainManage->psChnObj[i].sEncodeParam.sVideoMainParam.emVideoResolution = EM_VIDEO_CIF;
		psChnMainManage->psChnObj[i].sEncodeParam.sVideoMainParam.emBitRateType = EM_BITRATE_CONST;
		psChnMainManage->psChnObj[i].sEncodeParam.sVideoMainParam.emEncodeType = EM_ENCODE_VIDEO_H264;
		memset(psChnMainManage->psChnObj[i].sEncodeParam.sVideoMainParam.nReserve, 0, 
			sizeof(psChnMainManage->psChnObj[i].sEncodeParam.sVideoMainParam.nReserve));
		
		psChnMainManage->psChnObj[i].sChnInfo.nStart = 0;
		psChnMainManage->psChnObj[i].sChnInfo.nRestart = 0;
		psChnMainManage->psChnObj[i].sChnInfo.nPause = 0;
		psChnMainManage->psChnObj[i].sChnInfo.nRun = 0;
		psChnMainManage->psChnObj[i].sChnInfo.nContent = 0;
		psChnMainManage->psChnObj[i].sChnInfo.pfnEncodeProc = NULL;
		memset(psChnMainManage->psChnObj[i].sChnInfo.nReserve, 0, sizeof(psChnMainManage->psChnObj[i].sChnInfo.nReserve));//csp modify
		
		pthread_mutex_init(&psChnMainManage->psChnObj[i].pmLock, NULL);
	}
	
	printf("init main obj successful!!\n");
	
	return 0;
}

//csp modify 20130423
s32 InitThirdStreamObj(SChnManage* psChnThirdManage)
{
	int i, ret;
	SModConfigVideoParam sConfig;
	
	for(i = 0; i < psChnThirdManage->nChnNum; i++)
	{
		psChnThirdManage->psChnObj[i].sEncodeParam.nForce = 0;

		SModConfigVideoParam sConfig;
		ret = ModConfigGetParam(EM_CONFIG_PARA_ENC_MOB, &sConfig, i);
		if (ret == 0)
		{
			printf("%s chn%d nResolution: %d, nBitRate: %d, nFrameRate: %d\n", __func__, i, sConfig.nResolution, sConfig.nBitRate, sConfig.nFrameRate);
			psChnThirdManage->psChnObj[i].sEncodeParam.sVideoMainParam.emVideoResolution = sConfig.nResolution;
			psChnThirdManage->psChnObj[i].sEncodeParam.sVideoMainParam.nBitRate = sConfig.nBitRate;

			psChnThirdManage->psChnObj[i].sEncodeParam.sVideoMainParam.nFrameRate = sConfig.nFrameRate;//(g_s_EncodeManager.nVideoStandard == 10/*EM_BIZ_NTSC*/)?30:25;//csp modify 20140318
			psChnThirdManage->psChnObj[i].sEncodeParam.sVideoMainParam.nGop = 20;//50;//40;//csp modify 20140318
			psChnThirdManage->psChnObj[i].sEncodeParam.sVideoMainParam.nMaxQP = 40;
			psChnThirdManage->psChnObj[i].sEncodeParam.sVideoMainParam.nMinQP = 1;
			psChnThirdManage->psChnObj[i].sEncodeParam.sVideoMainParam.emPicLevel = 0;//1;//EM_PIC_LEVEL_BEST;//csp modify 20140318
			psChnThirdManage->psChnObj[i].sEncodeParam.sVideoMainParam.emBitRateType = EM_BITRATE_CONST;
			psChnThirdManage->psChnObj[i].sEncodeParam.sVideoMainParam.emEncodeType = EM_ENCODE_VIDEO_H264;
		}
		else
		{
			if(g_sThirdStreamProperty.emResolution == EM_VIDEO_D1)
			{
				psChnThirdManage->psChnObj[i].sEncodeParam.sVideoMainParam.emVideoResolution = EM_VIDEO_D1;
				psChnThirdManage->psChnObj[i].sEncodeParam.sVideoMainParam.nBitRate = 2048;
			}
			else
			{
				psChnThirdManage->psChnObj[i].sEncodeParam.sVideoMainParam.emVideoResolution = EM_VIDEO_CIF;
				psChnThirdManage->psChnObj[i].sEncodeParam.sVideoMainParam.nBitRate = 64;//48;//64;//512;//csp modify 20140318
			}
			
			psChnThirdManage->psChnObj[i].sEncodeParam.sVideoMainParam.nFrameRate = 5;//(g_s_EncodeManager.nVideoStandard == 10/*EM_BIZ_NTSC*/)?30:25;//csp modify 20140318
			psChnThirdManage->psChnObj[i].sEncodeParam.sVideoMainParam.nGop = 20;//50;//40;//csp modify 20140318
			psChnThirdManage->psChnObj[i].sEncodeParam.sVideoMainParam.nMaxQP = 40;
			psChnThirdManage->psChnObj[i].sEncodeParam.sVideoMainParam.nMinQP = 1;
			psChnThirdManage->psChnObj[i].sEncodeParam.sVideoMainParam.emPicLevel = 0;//1;//EM_PIC_LEVEL_BEST;//csp modify 20140318
			psChnThirdManage->psChnObj[i].sEncodeParam.sVideoMainParam.emBitRateType = EM_BITRATE_CONST;
			psChnThirdManage->psChnObj[i].sEncodeParam.sVideoMainParam.emEncodeType = EM_ENCODE_VIDEO_H264;
		}
		
		memset(psChnThirdManage->psChnObj[i].sEncodeParam.sVideoMainParam.nReserve, 0, 
			sizeof(psChnThirdManage->psChnObj[i].sEncodeParam.sVideoMainParam.nReserve));
		
		
		psChnThirdManage->psChnObj[i].sChnInfo.nStart = 0;
		psChnThirdManage->psChnObj[i].sChnInfo.nRestart = 0;
		psChnThirdManage->psChnObj[i].sChnInfo.nPause = 0;
		psChnThirdManage->psChnObj[i].sChnInfo.nRun = 0;
		psChnThirdManage->psChnObj[i].sChnInfo.nContent = 0;
		psChnThirdManage->psChnObj[i].sChnInfo.pfnEncodeProc = NULL;
		memset(psChnThirdManage->psChnObj[i].sChnInfo.nReserve, 0, sizeof(psChnThirdManage->psChnObj[i].sChnInfo.nReserve));//csp modify
		
		pthread_mutex_init(&psChnThirdManage->psChnObj[i].pmLock, NULL);
	}
	
	printf("init third obj successful!!\n");
	
	return 0;
}

s32 InitSubStreamObj(SChnManage* psChnSubManage)
{
	int i;
	
	for(i = 0; i < psChnSubManage->nChnNum; i++)
	{
		psChnSubManage->psChnObj[i].sEncodeParam.nForce = 0;
		
		//csp modify
		psChnSubManage->psChnObj[i].sEncodeParam.sVideoSubParam.nFrameRate = 6;
		psChnSubManage->psChnObj[i].sEncodeParam.sVideoSubParam.nBitRate = 64;
		psChnSubManage->psChnObj[i].sEncodeParam.sVideoSubParam.nGop = 18;//25;//csp modify
		psChnSubManage->psChnObj[i].sEncodeParam.sVideoSubParam.nMaxQP = 0;
		psChnSubManage->psChnObj[i].sEncodeParam.sVideoSubParam.nMinQP = 0;
		psChnSubManage->psChnObj[i].sEncodeParam.sVideoSubParam.emPicLevel = EM_PIC_LEVEL_BEST;
		psChnSubManage->psChnObj[i].sEncodeParam.sVideoSubParam.emVideoResolution = EM_VIDEO_QCIF;
		psChnSubManage->psChnObj[i].sEncodeParam.sVideoSubParam.emBitRateType = EM_BITRATE_CONST;
		psChnSubManage->psChnObj[i].sEncodeParam.sVideoSubParam.emEncodeType = EM_ENCODE_VIDEO_H264;
		memset(psChnSubManage->psChnObj[i].sEncodeParam.sVideoSubParam.nReserve, 0, 
			sizeof(psChnSubManage->psChnObj[i].sEncodeParam.sVideoSubParam.nReserve));
		
		psChnSubManage->psChnObj[i].sChnInfo.nStart = 0;
		psChnSubManage->psChnObj[i].sChnInfo.nRestart = 0;
		psChnSubManage->psChnObj[i].sChnInfo.nPause = 0;
		psChnSubManage->psChnObj[i].sChnInfo.nRun = 0;
		psChnSubManage->psChnObj[i].sChnInfo.nContent = 0;
		psChnSubManage->psChnObj[i].sChnInfo.pfnEncodeProc = NULL;
		memset(psChnSubManage->psChnObj[i].sChnInfo.nReserve, 0, sizeof(psChnSubManage->psChnObj[i].sChnInfo.nReserve));//csp modify
		
		pthread_mutex_init(&psChnSubManage->psChnObj[i].pmLock, NULL);
		
		//printf("InitSubStreamObj chn%d gop:%d\n",i,psChnSubManage->psChnObj[i].sEncodeParam.sVideoSubParam.nGop);
	}
	
	printf("init sub obj[%d] successful!!\n",psChnSubManage->nChnNum);
	
	return 0;
}

s32 InitSnapObj(SChnManage* psChnSnapManage)
{
	return 0;
}

s32 InitAudioStreamObj(SChnManage* psChnAudioManage)
{
	int i;
	
	for(i = 0; i < psChnAudioManage->nChnNum; i++)
	{
		psChnAudioManage->psChnObj[i].sEncodeParam.nForce = 0;
		
		psChnAudioManage->psChnObj[i].sEncodeParam.sAudioParam.nSampleRate = 8000;
		psChnAudioManage->psChnObj[i].sEncodeParam.sAudioParam.nSampleDuration = 64;
		//psChnAudioManage->psChnObj[i].sEncodeParam.sAudioParam.nSampleSize = 8000;
		psChnAudioManage->psChnObj[i].sEncodeParam.sAudioParam.nBitWidth = 8;
		psChnAudioManage->psChnObj[i].sEncodeParam.sAudioParam.emEncodeType = EM_ENCODE_AUDIO_RAW;
		psChnAudioManage->psChnObj[i].sEncodeParam.sAudioParam.nSampleSize = psChnAudioManage->psChnObj[i].sEncodeParam.sAudioParam.nSampleRate
				* psChnAudioManage->psChnObj[i].sEncodeParam.sAudioParam.nSampleDuration
				* psChnAudioManage->psChnObj[i].sEncodeParam.sAudioParam.nBitWidth / 8;
		memset(psChnAudioManage->psChnObj[i].sEncodeParam.sAudioParam.nReserve, 0,
			sizeof(psChnAudioManage->psChnObj[i].sEncodeParam.sAudioParam.nReserve));
		
		psChnAudioManage->psChnObj[i].sChnInfo.nStart = 0;
		psChnAudioManage->psChnObj[i].sChnInfo.nPause = 0;
		psChnAudioManage->psChnObj[i].sChnInfo.nRun = 0;
		psChnAudioManage->psChnObj[i].sChnInfo.nContent = 0;
		psChnAudioManage->psChnObj[i].sChnInfo.pfnEncodeProc = NULL;
		memset(psChnAudioManage->psChnObj[i].sChnInfo.nReserve, 0, sizeof(psChnAudioManage->psChnObj[i].sChnInfo.nReserve));
		
		pthread_mutex_init(&psChnAudioManage->psChnObj[i].pmLock, NULL);
	}
	
	printf("init audio obj successful!!\n");
	
	return 0;
}

s32 InitTalkStreamObj(SChnManage* psChnTalkManage)
{
	int i;
	
	//printf("InitTalkStreamObj chnum %d\n", psChnTalkManage->nChnNum);
	
	for(i = 0; i < psChnTalkManage->nChnNum; i++)
	{
		psChnTalkManage->psChnObj[i].sEncodeParam.nForce = 0;
		
		psChnTalkManage->psChnObj[i].sEncodeParam.sAudioParam.nSampleRate = 8000;
		psChnTalkManage->psChnObj[i].sEncodeParam.sAudioParam.nSampleDuration = 64;
		//psChnTalkManage->psChnObj[i].sEncodeParam.sAudioParam.nSampleSize = 8000;
		psChnTalkManage->psChnObj[i].sEncodeParam.sAudioParam.nBitWidth = 8;
		psChnTalkManage->psChnObj[i].sEncodeParam.sAudioParam.emEncodeType = EM_ENCODE_AUDIO_RAW;
		psChnTalkManage->psChnObj[i].sEncodeParam.sAudioParam.nSampleSize = 
				psChnTalkManage->psChnObj[i].sEncodeParam.sAudioParam.nSampleRate
				* psChnTalkManage->psChnObj[i].sEncodeParam.sAudioParam.nSampleDuration
				* psChnTalkManage->psChnObj[i].sEncodeParam.sAudioParam.nBitWidth / 8;
		memset(psChnTalkManage->psChnObj[i].sEncodeParam.sAudioParam.nReserve, 0,
			sizeof(psChnTalkManage->psChnObj[i].sEncodeParam.sAudioParam.nReserve));
		
		psChnTalkManage->psChnObj[i].sChnInfo.nStart = 0;
		psChnTalkManage->psChnObj[i].sChnInfo.nPause = 0;
		psChnTalkManage->psChnObj[i].sChnInfo.nRun = 0;
		psChnTalkManage->psChnObj[i].sChnInfo.nContent = 0;
		psChnTalkManage->psChnObj[i].sChnInfo.pfnEncodeProc = NULL;
		memset(psChnTalkManage->psChnObj[i].sChnInfo.nReserve, 0, sizeof(psChnTalkManage->psChnObj[i].sChnInfo.nReserve));
		
		pthread_mutex_init(&psChnTalkManage->psChnObj[i].pmLock, NULL);
	}
	
	printf("init talk obj successful!!\n");
	
	return 0;
}

//读取编码报错 重新启动编码
void RestartEncoder(u8 bMain, SChnManage* psChnManage)
{
	int ret;
	
	if(!psChnManage) return;
	
	if(!psChnManage->nIsEncoding) return;
	
	int i = 0;
	for(i = 0; i < psChnManage->nChnNum; i++)
	{
		if(!psChnManage->psChnObj[i].sChnInfo.nStart)
		{
			continue;
		}
		
		ret = bMain ? tl_venc_stop(i):tl_venc_minor_stop(i);
		if(ret)
		{
			printf("bMain %d venc read error, stop enc ret %d \n", bMain, ret);
		}
		
		venc_parameter_t venc_param;
		memset(&venc_param, 0, sizeof(venc_param));
		
		venc_param.frame_rate 
			= psChnManage->psChnObj[i].sEncodeParam.sVideoSubParam.nFrameRate;
		venc_param.bit_rate 
			= psChnManage->psChnObj[i].sEncodeParam.sVideoSubParam.nBitRate;
		venc_param.gop 
			= psChnManage->psChnObj[i].sEncodeParam.sVideoSubParam.nGop;
		venc_param.pic_level 
			= psChnManage->psChnObj[i].sEncodeParam.sVideoSubParam.emPicLevel;
		venc_param.is_cbr 
			= (EM_BITRATE_CONST == psChnManage->psChnObj[i].sEncodeParam.sVideoSubParam.emBitRateType) ? 1 : 0;
		
		venc_size_e emResol;
		switch(psChnManage->psChnObj[i].sEncodeParam.sVideoSubParam.emVideoResolution)
		{
			case EM_VIDEO_D1:
			case EM_VIDEO_4CIF:
				emResol = VENC_SIZE_D1;	
				break;
			case EM_VIDEO_HALF_D1:
				emResol = VENC_SIZE_HALF_D1;	
				break;
			case EM_VIDEO_CIF:
				emResol = VENC_SIZE_CIF;							
				break;
			case EM_VIDEO_QCIF:
				emResol = VENC_SIZE_QCIF;	
				break;
			case EM_VIDEO_720P:
				emResol = VENC_SIZE_HD720;	
				break;
			case EM_VIDEO_1080P:
				emResol = VENC_SIZE_HD1080;	
				break;
			case EM_VIDEO_960H:
				emResol = VENC_SIZE_960H;	
				break;
			default:
				emResol = VENC_SIZE_CIF;	
				break;
		}
		
		// 帧率码率不匹配会导致编码器出错
		if(venc_param.frame_rate <= 3)
		{
			venc_param.bit_rate = 256;
		}
		
		if(VENC_SIZE_CIF == emResol)
		{
			venc_param.bit_rate = venc_param.bit_rate > 1024 ? 1024 : venc_param.bit_rate;
			venc_param.bit_rate = venc_param.bit_rate < 64 ? 64 : venc_param.bit_rate;
		}
		
		//printf("RestartEncoder %s stream chn%d gop:%d\n",bMain?"main":"sub",i,venc_param.gop);
		
		ret = bMain ? tl_venc_start(i, emResol, &venc_param) : tl_venc_minor_start(i, emResol, &venc_param);
		if(ret)
		{
			printf("bMain %d venc read error, stop enc ret %d \n", bMain, ret);
		}
	}
}

void EncodeStatusDealFxn(void* arg)
{
	printf("$$$$$$$$$$$$$$$$$$EncodeStatusDealFxn id:%d\n",getpid());
	
	if(NULL == arg)
	{
		printf("Invalid para!!! func:%s\n", __FUNCTION__);
		return;
	}
	
	SEncodeManage* pManager = (SEncodeManage*)arg;
	u8* nMainStatusLast = NULL;
	u8* nSubStatusLast = NULL;
	u8* nAudioStatusLast = NULL;
	u8* nTalkStatusLast = NULL;
	
	nMainStatusLast = (u8*)calloc(sizeof(u8), pManager->sChnMainManage.nChnNum);
	nSubStatusLast = (u8*)calloc(sizeof(u8), pManager->sChnSubManage.nChnNum);
	nAudioStatusLast = (u8*)calloc(sizeof(u8), pManager->sChnAudioManage.nChnNum);	
	nTalkStatusLast = (u8*)calloc(sizeof(u8), pManager->sChnTalkManage.nChnNum);	
	if((NULL == nMainStatusLast) || (NULL == nSubStatusLast) || (NULL == nAudioStatusLast) || (NULL == nTalkStatusLast))
	{
		printf("malloc stream last status fail! func:%s\n", __FUNCTION__);
		return;
	}
	
	//csp modify 20130423
	u8* nThirdStatusLast = NULL;
	if(ModEncodeSupportThirdStream())
	{
		nThirdStatusLast = (u8*)calloc(sizeof(u8), pManager->sChnThirdManage.nChnNum);
		if(nThirdStatusLast == NULL)
		{
			printf("malloc stream last status fail! func:%s\n", __FUNCTION__);
			return;
		}
	}
	
	venc_parameter_t venc_param;
	memset(&venc_param, 0, sizeof(venc_param));
	
	SChnManage* psChnManage = NULL;
	u8 nStatusCur = 0;
	u8 nStartCount = 0;
	s32 ret = 0;
	u8 i = 0;
	
	while(1)
	{
		//子码流
		psChnManage = &(pManager->sChnSubManage);
		nStartCount = 0;
		for(i = 0; i < psChnManage->nChnNum; i++)
		{
			SChnObj* psSubStreamChn = &psChnManage->psChnObj[i];
			
            pthread_mutex_lock(&psSubStreamChn->pmLock);
			
			nStatusCur = (psSubStreamChn->sChnInfo.nStart && 0 == psSubStreamChn->sChnInfo.nPause);
			
			if(1 == nStatusCur)
			{
				nStartCount++;
			}
			
			//printf("sub  chn[%d] restart:%d\n", i, psSubStreamChn->sChnInfo.nRestart);
			
			if((nSubStatusLast[i] != nStatusCur) || (psSubStreamChn->sChnInfo.nRestart && 0==g_s_EncodeManager.sChnMainManage.psChnObj[i].sChnInfo.nRestart))
			{
				if(nStatusCur)
				{
					venc_param.frame_rate 
						= psSubStreamChn->sEncodeParam.sVideoSubParam.nFrameRate;
					venc_param.bit_rate 
						= psSubStreamChn->sEncodeParam.sVideoSubParam.nBitRate;
					venc_param.gop 
						= psSubStreamChn->sEncodeParam.sVideoSubParam.nGop;
					venc_param.pic_level 
						= psSubStreamChn->sEncodeParam.sVideoSubParam.emPicLevel;
					venc_param.is_cbr 
						= (EM_BITRATE_CONST == psSubStreamChn->sEncodeParam.sVideoSubParam.emBitRateType) ? 1 : 0;
					
					venc_size_e emResol;
					switch(psSubStreamChn->sEncodeParam.sVideoSubParam.emVideoResolution)
					{
						case EM_VIDEO_D1:
						case EM_VIDEO_4CIF:
							emResol = VENC_SIZE_D1;	
							break;
						case EM_VIDEO_HALF_D1:
							emResol = VENC_SIZE_HALF_D1;	
							break;
						case EM_VIDEO_CIF:
							emResol = VENC_SIZE_CIF;							
							break;
						case EM_VIDEO_QCIF:
							emResol = VENC_SIZE_QCIF;	
							break;
						case EM_VIDEO_720P:
							emResol = VENC_SIZE_HD720;	
							break;
						case EM_VIDEO_1080P:
							emResol = VENC_SIZE_HD1080;	
							break;
						case EM_VIDEO_960H:
							emResol = VENC_SIZE_960H;	
							break;
						default:
							emResol = VENC_SIZE_CIF;	
							break;
					}
					
					//venc_param.frame_rate = 25;
					//venc_param.bit_rate 	= 512;
					
					// 帧率码率不匹配会导致编码器出错
					if(venc_param.frame_rate <= 3)
					{
						venc_param.bit_rate = venc_param.bit_rate > 64 ? 64 : venc_param.bit_rate;
					}
					if(1)//(VENC_SIZE_CIF == emResol)
					{
						venc_param.bit_rate = venc_param.bit_rate > 1024 ? 1024 : venc_param.bit_rate;
						venc_param.bit_rate = venc_param.bit_rate < 64 ? 64 : venc_param.bit_rate;
					}
					
					//printf("###sub stream chn%d reso %d cbr %d fr %d pic %d gop %d br %d\n",
					//	i,
					//	emResol,
					//	venc_param.is_cbr,
					//	venc_param.frame_rate,
					//	venc_param.pic_level,
					//	venc_param.gop,
					//	venc_param.bit_rate
					//);
					
					//sem_wait(&psChnManage->nSemVencReadOrStart);
					ret = tl_venc_minor_start(i, emResol, &venc_param);
					//sem_post(&psChnManage->nSemVencReadOrStart);
					//printf("tl_venc_minor_start chn[%d]\n", i);
				}
				else
				{
					ret = tl_venc_minor_stop(i);
					//printf("tl_venc_minor_stop chn[%d]\n", i);
				}
				
				if(0 == ret)
				{
					nSubStatusLast[i] = nStatusCur;
					psSubStreamChn->sChnInfo.nRun = nStatusCur;
				}
				else
				{
					if(nStatusCur)//没有开始成功
					{
						tl_venc_minor_stop(i);
						//printf("###########tl_venc_minor_stop chn[%d] after tl_venc_minor_start failed! \n", i);
					}
					nStartCount -= (nStatusCur * 2) - 1;
				}
				
				psSubStreamChn->sChnInfo.nRestart = 0;
			}	
            pthread_mutex_unlock(&psSubStreamChn->pmLock);
		}
		psChnManage->nIsEncoding = nStartCount ? 1 : 0;
		if(g_s_EncodeManager.emBoardType == EM_ENCODE_BOARD_HISI3520_16D1)
		{
			for(i = 0; i < 8; i++)
			{
				if(nSubStatusLast[i])
				{
					g_s_EncodeManager.sChnSubManage.nMasterWorking = 1;
					break;
				}
			}
			if(i >= 8)
			{
				g_s_EncodeManager.sChnSubManage.nMasterWorking = 0;
			}
			
			for(i = 8; i < 16; i++)
			{
				if(nSubStatusLast[i])
				{
					g_s_EncodeManager.sChnSubManage.nSlaverWorking = 1;
					break;
				}
			}
			if(i >= 16)
			{
				g_s_EncodeManager.sChnSubManage.nSlaverWorking = 0;
				//是否需要将从片数据读完
			}
		}
		
		//主码流
		psChnManage = &(pManager->sChnMainManage);		
		nStartCount = 0;		
		for(i = 0; i < psChnManage->nChnNum; i++)
		{
			SChnObj* psMainStreamChn = &psChnManage->psChnObj[i];
			
            pthread_mutex_lock(&psMainStreamChn->pmLock);
			
			nStatusCur = (psMainStreamChn->sChnInfo.nStart && 0 == psMainStreamChn->sChnInfo.nPause);
			
			if(1 == nStatusCur)
			{
				nStartCount++;
			}
			
			//printf("main chn[%d] restart:%d\n", i, psMainStreamChn->sChnInfo.nRestart);
			
			if((nMainStatusLast[i] != nStatusCur) || psMainStreamChn->sChnInfo.nRestart)
			{
				if(nStatusCur)
				{
					venc_param.frame_rate 
						= psMainStreamChn->sEncodeParam.sVideoMainParam.nFrameRate;
					venc_param.bit_rate 
						= psMainStreamChn->sEncodeParam.sVideoMainParam.nBitRate;
					venc_param.gop 
						= psMainStreamChn->sEncodeParam.sVideoMainParam.nGop;
					venc_param.pic_level 
						= psMainStreamChn->sEncodeParam.sVideoMainParam.emPicLevel;
					venc_param.is_cbr 
						= (EM_BITRATE_CONST == psMainStreamChn->sEncodeParam.sVideoMainParam.emBitRateType) ? 1 : 0;
					
					venc_size_e emResol;
					switch(psMainStreamChn->sEncodeParam.sVideoMainParam.emVideoResolution)
					{
						case EM_VIDEO_D1:
						case EM_VIDEO_4CIF:
							emResol = VENC_SIZE_D1;
							break;
						case EM_VIDEO_HALF_D1:
							emResol = VENC_SIZE_HALF_D1;
							break;
						case EM_VIDEO_CIF:
							emResol = VENC_SIZE_CIF;
							break;
						case EM_VIDEO_QCIF:
							emResol = VENC_SIZE_QCIF;
							break;
						case EM_VIDEO_720P:
							emResol = VENC_SIZE_HD720;
							break;
						case EM_VIDEO_1080P:
							emResol = VENC_SIZE_HD1080;
							break;
						case EM_VIDEO_960H:
							emResol = VENC_SIZE_960H;
							break;
						default:
							emResol = VENC_SIZE_CIF;
							break;
					}
					
					/*printf("main stream chn%d reso %d cbr %d fr %d pic %d gop %d br %d\n",
						i,
						emResol,
						venc_param.frame_rate,
						venc_param.is_cbr,
						venc_param.pic_level,
						venc_param.gop,
						venc_param.bit_rate
					);*/
					
					// 帧率码率不匹配会导致编码器出错
					if(venc_param.frame_rate <= 3)
					{
						venc_param.bit_rate = venc_param.bit_rate > 128 ? 128 : venc_param.bit_rate;
					}
					
					if(VENC_SIZE_CIF == emResol)
					{
						venc_param.bit_rate = venc_param.bit_rate > 1024 ? 1024 : venc_param.bit_rate;
						venc_param.bit_rate = venc_param.bit_rate < 64 ? 64 : venc_param.bit_rate;
					}
					else if(VENC_SIZE_D1 == emResol)
					{					
						venc_param.bit_rate = venc_param.bit_rate < 64 ? 64 : venc_param.bit_rate;
					}
					
					if(g_s_EncodeManager.sChnSubManage.psChnObj[i].sChnInfo.nRun)
					{
						tl_venc_minor_stop(i);
					}
					
					//sem_wait(&psChnManage->nSemVencReadOrStart);
					ret = tl_venc_start(i, emResol, &venc_param);
					//printf("tl_venc_start chn[%d],size:%d,result:%d\n", i, emResol, ret);
					//sem_post(&psChnManage->nSemVencReadOrStart);
					
					if(g_s_EncodeManager.sChnSubManage.psChnObj[i].sChnInfo.nRun)
					{
						g_s_EncodeManager.sChnSubManage.psChnObj[i].sChnInfo.nRestart = 1;
					}
				}
				else
				{
					ret = tl_venc_stop(i);
					//printf("tl_venc_stop chn[%d]\n", i);//csp modify
				}
				
				if(0 == ret)
				{
					nMainStatusLast[i] = nStatusCur;
					psMainStreamChn->sChnInfo.nRun = nStatusCur;
				}
				else
				{
					nStartCount -= (nStatusCur * 2) - 1;
				}
				
				psMainStreamChn->sChnInfo.nRestart = 0;
			}	
            pthread_mutex_unlock(&psMainStreamChn->pmLock);
		}
		psChnManage->nIsEncoding = nStartCount ? 1 : 0;
		if(g_s_EncodeManager.emBoardType == EM_ENCODE_BOARD_HISI3520_16D1)
		{
			for(i = 0; i < 8; i++)
			{
				if(nMainStatusLast[i])
				{
					g_s_EncodeManager.sChnMainManage.nMasterWorking = 1;
					break;
				}
			}
			if(i >= 8)
			{
				g_s_EncodeManager.sChnMainManage.nMasterWorking = 0;
			}

			for(i = 8; i < 16; i++)
			{
				if(nMainStatusLast[i])
				{
					g_s_EncodeManager.sChnMainManage.nSlaverWorking = 1;
					break;
				}
			}
			if(i >= 16)
			{
				g_s_EncodeManager.sChnMainManage.nSlaverWorking = 0;
				//是否需要将从片数据读完
			}
		}
		
//#if defined(CHIP_HISI3531)
		//csp modify 20130423
		if(ModEncodeSupportThirdStream())
		{
			//第三路码流
			
			psChnManage = &(pManager->sChnThirdManage);
			nStartCount = 0;
			for(i = 0; i < psChnManage->nChnNum; i++)
			{
				SChnObj* psThirdStreamChn = &psChnManage->psChnObj[i];
				
	            		pthread_mutex_lock(&psThirdStreamChn->pmLock);
				
				nStatusCur = (psThirdStreamChn->sChnInfo.nStart && 0 == psThirdStreamChn->sChnInfo.nPause);
				//printf("%s nStatusCur: %d\n", __func__, nStatusCur);
				if(1 == nStatusCur)
				{
					nStartCount++;
				}
				
				//printf("third chn[%d] restart:%d\n", i, psThirdStreamChn->sChnInfo.nRestart);
				
				if((nThirdStatusLast[i] != nStatusCur) || psThirdStreamChn->sChnInfo.nRestart)
				{
					if(nStatusCur)
					{
						venc_param.frame_rate 
							= psThirdStreamChn->sEncodeParam.sVideoMainParam.nFrameRate;
						venc_param.bit_rate 
							= psThirdStreamChn->sEncodeParam.sVideoMainParam.nBitRate;
						venc_param.gop 
							= psThirdStreamChn->sEncodeParam.sVideoMainParam.nGop;
						venc_param.pic_level 
							= psThirdStreamChn->sEncodeParam.sVideoMainParam.emPicLevel;
						venc_param.is_cbr 
							= (EM_BITRATE_CONST == psThirdStreamChn->sEncodeParam.sVideoMainParam.emBitRateType) ? 1 : 0;
						
						venc_size_e emResol;
						switch(psThirdStreamChn->sEncodeParam.sVideoMainParam.emVideoResolution)
						{
							case EM_VIDEO_D1:
							case EM_VIDEO_4CIF:
								emResol = VENC_SIZE_D1;
								break;
							case EM_VIDEO_HALF_D1:
								emResol = VENC_SIZE_HALF_D1;
								break;
							case EM_VIDEO_CIF:
								emResol = VENC_SIZE_CIF;
								break;
							case EM_VIDEO_VGA:
								emResol = VENC_SIZE_VGA;
								break;
							case EM_VIDEO_QCIF:
								emResol = VENC_SIZE_QCIF;
								break;
							case EM_VIDEO_720P:
								emResol = VENC_SIZE_HD720;
								break;
							case EM_VIDEO_1080P:
								emResol = VENC_SIZE_HD1080;
								break;
							case EM_VIDEO_960H:
								emResol = VENC_SIZE_960H;
								break;
							default:
								emResol = VENC_SIZE_CIF;
								break;
						}
						
						printf("third stream chn%d reso %d cbr %d fr %d pic %d gop %d br %d\n",
							i,
							emResol,
							venc_param.frame_rate,
							venc_param.is_cbr,
							venc_param.pic_level,
							venc_param.gop,
							venc_param.bit_rate
						);
						
						// 帧率码率不匹配会导致编码器出错
						if(venc_param.frame_rate <= 3)
						{
							venc_param.bit_rate = venc_param.bit_rate > 128 ? 128 : venc_param.bit_rate;
						}
						
						if(VENC_SIZE_CIF == emResol)
						{
							venc_param.bit_rate = venc_param.bit_rate > 1024 ? 1024 : venc_param.bit_rate;
							venc_param.bit_rate = venc_param.bit_rate < 64 ? 64 : venc_param.bit_rate;
						}
						else if(VENC_SIZE_D1 == emResol)
						{					
							venc_param.bit_rate = venc_param.bit_rate < 64 ? 64 : venc_param.bit_rate;
						}
						
						if(g_s_EncodeManager.sChnSubManage.psChnObj[i].sChnInfo.nRun)
						{
							tl_venc_minor_stop(i);
						}
						
						//sem_wait(&psChnManage->nSemVencReadOrStart);
						ret = tl_venc_third_start(i, emResol, &venc_param);
						//printf("tl_venc_third_start chn[%d],size:%d,result:%d\n", i, emResol, ret);
						//sem_post(&psChnManage->nSemVencReadOrStart);
						
						if(g_s_EncodeManager.sChnSubManage.psChnObj[i].sChnInfo.nRun)
						{
							g_s_EncodeManager.sChnSubManage.psChnObj[i].sChnInfo.nRestart = 1;
						}
					}
					else
					{
						ret = tl_venc_third_stop(i);
						//printf("tl_venc_third_stop chn[%d]\n", i);//csp modify
					}
					
					if(0 == ret)
					{
						nThirdStatusLast[i] = nStatusCur;
						psThirdStreamChn->sChnInfo.nRun = nStatusCur;
					}
					else
					{
						nStartCount -= (nStatusCur * 2) - 1;
					}
					
					psThirdStreamChn->sChnInfo.nRestart = 0;
				}	
	            pthread_mutex_unlock(&psThirdStreamChn->pmLock);
			}
			psChnManage->nIsEncoding = nStartCount ? 1 : 0;
		}
//#endif
		
		//音频码流
		psChnManage = &(pManager->sChnAudioManage);
		nStartCount = 0;
		for(i = 0; i < psChnManage->nChnNum; i++)
		{
			SChnObj* psAudioChn = &psChnManage->psChnObj[i];
			
			pthread_mutex_lock(&psAudioChn->pmLock);
            
			nStatusCur = (psAudioChn->sChnInfo.nStart && 0 == psAudioChn->sChnInfo.nPause);
			
			if(1 == nStatusCur)
			{
				nStartCount++;
			}
			
			if(nAudioStatusLast[i] != nStatusCur)
			{
				if(nStatusCur)
				{
					ret = tl_audio_enable(i);
					//printf("tl_audio_enable ret = %d ch %d\n", ret, i);
				}
				else
				{
					ret = tl_audio_disable(i);	
					//printf("tl_audio_disable ret = %d ch %d\n", ret, i);
				}				
                if(0 == ret)
                {
					nAudioStatusLast[i] = nStatusCur;
					psAudioChn->sChnInfo.nRun = nStatusCur;
				}
				else
				{
					nStartCount -= (nStatusCur * 2) - 1;
				}
			}	
			
            pthread_mutex_unlock(&psAudioChn->pmLock);
		}
		
		psChnManage = &(pManager->sChnTalkManage);
		i = 0;
		if(psChnManage->nChnNum > 0)
		{
			SChnObj* psAudioChn = &psChnManage->psChnObj[0];
			
            pthread_mutex_lock(&psAudioChn->pmLock);
            
			nStatusCur = (psAudioChn->sChnInfo.nStart && 0 == psAudioChn->sChnInfo.nPause);
			
			if(1 == nStatusCur)
			{
				nStartCount++;
			}
			
			if(nTalkStatusLast[i] != nStatusCur)
			{
				if(nStatusCur)
				{
					ret = tl_audio_enable(TL_AUDIO_CH_TALK_BACK);
					//printf("tl_audio_enable voip ret = %d ch = %d num = %d\n", ret, i, pManager->sChnTalkManage.nChnNum);
				}
				else
				{
					ret = tl_audio_disable(TL_AUDIO_CH_TALK_BACK);
					//printf("tl_audio_disable voip ret = %d ch = %d num = %d\n", ret, i, pManager->sChnTalkManage.nChnNum);
				}
				
                if(0 == ret)
                {
                    nTalkStatusLast[i] = nStatusCur;
					psAudioChn->sChnInfo.nRun = nStatusCur;
                }
				else
				{
					nStartCount -= (nStatusCur * 2) - 1;
				}
			}	
			
            pthread_mutex_unlock(&psAudioChn->pmLock);
		}
		psChnManage = &(pManager->sChnAudioManage);
		psChnManage->nIsEncoding = nStartCount ? 1 : 0;
		
		usleep(50 * 1000);//cw_encode
	}
}

int DealAudioStream(audio_frame_t *stream)
{
	if(stream == NULL)
	{
		return -1;
	}
	
	if(0 == g_s_EncodeManager.sChnAudioManage.nIsEncoding)
	{
		return -1;
	}
	
	int chn = stream->channel;
	if((chn < 0 || chn >= g_s_EncodeManager.sChnAudioManage.nChnNum) && (chn != TL_AUDIO_CH_TALK_BACK))
	{
		return -1;
	}
	
	if(stream->len > g_s_EncodeManager.sChnAudioManage.nMaxFrameSize)
	{
		return -1;
	}
	
	SChnObj* psAudioChn = NULL;
	
	if(chn == TL_AUDIO_CH_TALK_BACK)
	{
		psAudioChn = &g_s_EncodeManager.sChnTalkManage.psChnObj[0];
	}
	else
	{
		psAudioChn = &g_s_EncodeManager.sChnAudioManage.psChnObj[chn];
	}
	
	pthread_mutex_lock(&psAudioChn->pmLock);
	
	if(psAudioChn->sChnInfo.nStart)
	{
		SEncodeHead sEncodeHead;
		sEncodeHead.nLen = stream->len;
		sEncodeHead.nTimeStamp = stream->time_stamp / 1000;
		sEncodeHead.pData = (char *)stream->data;
		sEncodeHead.emEncodeType = EM_ENCODE_AUDIO_RAW;
		sEncodeHead.emVideoFrameType = 0;
		memset(sEncodeHead.nReserve, 0, sizeof(sEncodeHead.nReserve));
		
		PFNEncodeRequest pfnEncodeProc = psAudioChn->sChnInfo.pfnEncodeProc;
		
		pthread_mutex_unlock(&psAudioChn->pmLock);
		
		if(pfnEncodeProc != NULL)
		{
			pfnEncodeProc(&sEncodeHead, psAudioChn->sChnInfo.nContent);
		}
	}
	else
	{
		pthread_mutex_unlock(&psAudioChn->pmLock);
	}
	
	return 0;
}

int DealMainStream(venc_stream_s *stream)
{
	if(stream == NULL)
	{
		return -1;
	}
	
	int chn = stream->chn;
	if(chn < 0 || chn >= g_s_EncodeManager.sChnMainManage.nChnNum)
	{
		return -1;
	}
	//640k
	if(stream->len > g_s_EncodeManager.sChnMainManage.nMaxFrameSize)
	{
		
		//printf("%s 1\n", __func__);
		printf("%s len: %u > nMaxFrameSize: %u\n", __func__, stream->len, g_s_EncodeManager.sChnMainManage.nMaxFrameSize);
		//printf("%s 2\n", __func__);
		
		return -1;
	}
	
	SChnObj* psStreamChn = &g_s_EncodeManager.sChnMainManage.psChnObj[chn];
	
	pthread_mutex_lock(&psStreamChn->pmLock);
	
	if(psStreamChn->sChnInfo.nStart)
	{
		SEncodeHead sEncodeHead;
		sEncodeHead.nLen = stream->len;
		sEncodeHead.nTimeStamp = stream->pts / 1000;
		sEncodeHead.pData = (char *)stream->data;
		sEncodeHead.emVideoFrameType = (FRAME_TYPE_I == stream->type) ? EM_FRAME_TYPE_I : EM_FRAME_TYPE_P;
		sEncodeHead.emEncodeType = EM_ENCODE_VIDEO_H264;
		sEncodeHead.emResol = psStreamChn->sEncodeParam.sVideoMainParam.emVideoResolution;
		sEncodeHead.nBitRate = psStreamChn->sEncodeParam.sVideoMainParam.nBitRate;
		sEncodeHead.width = stream->width;
		sEncodeHead.height = stream->height;
		memset(sEncodeHead.nReserve, 0, sizeof(sEncodeHead.nReserve));
		
		PFNEncodeRequest pfnEncodeProc = psStreamChn->sChnInfo.pfnEncodeProc;
		u32 nContent = psStreamChn->sChnInfo.nContent;
		
		pthread_mutex_unlock(&psStreamChn->pmLock);
		
		if(pfnEncodeProc != NULL)
		{
			//printf("%s nContent: 0x%x\n", __func__, nContent);
			pfnEncodeProc(&sEncodeHead, nContent);
		}
	}
	else
	{
		pthread_mutex_unlock(&psStreamChn->pmLock);
	}
	
	return 0;
}

int DealThirdStream(venc_stream_s *stream)
{
	if(!ModEncodeSupportThirdStream())
	{
		return -1;
	}
	
	if(stream == NULL)
	{
		return -1;
	}
	
	int chn = stream->chn;
	if(chn < 0 || chn >= g_s_EncodeManager.sChnThirdManage.nChnNum)
	{
		return -1;
	}
	
	if(stream->len > g_s_EncodeManager.sChnThirdManage.nMaxFrameSize)
	{
		//printf("warning:DealThirdStream len=%d nMaxFrameSize=%d\n",stream->len,g_s_EncodeManager.sChnThirdManage.nMaxFrameSize);
		return -1;
	}
	
	SChnObj* psStreamChn = &g_s_EncodeManager.sChnThirdManage.psChnObj[chn];
	
	pthread_mutex_lock(&psStreamChn->pmLock);
	
	if(psStreamChn->sChnInfo.nStart)
	{
		SEncodeHead sEncodeHead;
		sEncodeHead.nLen = stream->len;
		sEncodeHead.nTimeStamp = stream->pts / 1000;
		sEncodeHead.pData = (char *)stream->data;
		sEncodeHead.emVideoFrameType = (FRAME_TYPE_I == stream->type) ? EM_FRAME_TYPE_I : EM_FRAME_TYPE_P;
		sEncodeHead.emEncodeType = EM_ENCODE_VIDEO_H264;
		sEncodeHead.emResol = psStreamChn->sEncodeParam.sVideoMainParam.emVideoResolution;
		sEncodeHead.nBitRate = psStreamChn->sEncodeParam.sVideoMainParam.nBitRate;
		sEncodeHead.width = stream->width;
		sEncodeHead.height = stream->height;
		memset(sEncodeHead.nReserve, 0, sizeof(sEncodeHead.nReserve));
		
		PFNEncodeRequest pfnEncodeProc = psStreamChn->sChnInfo.pfnEncodeProc;
		u32 nContent = psStreamChn->sChnInfo.nContent;
		
		pthread_mutex_unlock(&psStreamChn->pmLock);
		
		//printf("chn%d callback third stream\n",chn);
		
		if(pfnEncodeProc != NULL)
		{
			pfnEncodeProc(&sEncodeHead, nContent);
		}
	}
	else
	{
		pthread_mutex_unlock(&psStreamChn->pmLock);
	}
	
	return 0;
}

int DealSubStream(venc_stream_s *stream)
{
	if(stream == NULL)
	{
		return -1;
	}
	
	int chn = stream->chn;
	if(chn < 0 || chn >= g_s_EncodeManager.sChnSubManage.nChnNum)
	{
		return -1;
	}
	
	if(stream->len > g_s_EncodeManager.sChnSubManage.nMaxFrameSize)
	{
		//printf("<warning> DealSubStream chn%d len=%d nMaxFrameSize=%d\n",chn,stream->len,g_s_EncodeManager.sChnSubManage.nMaxFrameSize);
		return -1;
	}
	
	SChnObj* psStreamChn = &g_s_EncodeManager.sChnSubManage.psChnObj[chn];
	
	pthread_mutex_lock(&psStreamChn->pmLock);
	
	if(psStreamChn->sChnInfo.nStart)
	{
		SEncodeHead sEncodeHead;
		sEncodeHead.nLen = stream->len;
		sEncodeHead.nTimeStamp = stream->pts / 1000;
		sEncodeHead.pData = (char *)stream->data;
		sEncodeHead.emVideoFrameType = (FRAME_TYPE_I == stream->type) ? EM_FRAME_TYPE_I : EM_FRAME_TYPE_P;
		sEncodeHead.emEncodeType = EM_ENCODE_VIDEO_H264;
		sEncodeHead.emResol = psStreamChn->sEncodeParam.sVideoSubParam.emVideoResolution;
		sEncodeHead.nBitRate = psStreamChn->sEncodeParam.sVideoSubParam.nBitRate;
		sEncodeHead.width = stream->width;
		sEncodeHead.height = stream->height;
		memset(sEncodeHead.nReserve, 0, sizeof(sEncodeHead.nReserve));
		
		PFNEncodeRequest pfnEncodeProc = psStreamChn->sChnInfo.pfnEncodeProc;
		u32 nContent = psStreamChn->sChnInfo.nContent;
		
		pthread_mutex_unlock(&psStreamChn->pmLock);
		
		//if(stream->chn == 3) printf("######DealSubStream chn%d len=%d nMaxFrameSize=%d,startflag=%d,pfnEncodeProc=0x%08x\n",stream->chn,stream->len,g_s_EncodeManager.sChnSubManage.nMaxFrameSize,psStreamChn->sChnInfo.nStart,(unsigned int)pfnEncodeProc);
		
		if(pfnEncodeProc != NULL)
		{
			pfnEncodeProc(&sEncodeHead, nContent);
		}
	}
	else
	{
		pthread_mutex_unlock(&psStreamChn->pmLock);
	}
	
	return 0;
}

//s32 GetTimeTick()
//{
//	struct timeval time;
//	gettimeofday(&time, NULL);   
//	return time.tv_sec*1000+time.tv_usec/1000;
//}
//csp modify 20140812
s32 ThirdStreamEncodeSetParam(u8 nChn, SEncodeParam* psEncodeParam)
{
	venc_parameter_t venc_param;
	
	venc_param.bit_rate = psEncodeParam->sVideoSubParam.nBitRate;
	venc_param.frame_rate = psEncodeParam->sVideoSubParam.nFrameRate;
	venc_param.gop = psEncodeParam->sVideoSubParam.nGop;
	venc_param.is_cbr = (EM_BITRATE_CONST == psEncodeParam->sVideoSubParam.emBitRateType) ? 1 : 0;
	venc_param.pic_level = (u32)psEncodeParam->sVideoSubParam.emPicLevel;
	/*
	printf("func %s br %d fr %d gop %d cbr %d pic %d \n",
		__FUNCTION__,
		venc_param.bit_rate,
		venc_param.frame_rate,
		venc_param.gop,
		venc_param.is_cbr,
		venc_param.pic_level		
	);
	*/
	tl_venc_third_set_para(nChn, &venc_param);
	
	return 0;
}

s32 ModEncodeSetParam(u8 nChn, EMCHNTYPE emChnType, SEncodeParam* psEncodeParam)
{
	SChnObj* psChnObj = NULL;
	s32 ret;
	u8 byChangeStatus = 0;
	
	ret = GetChnObj(nChn, emChnType, &psChnObj);
	if(ret)
	{
		printf("ModEncodeSetParam GetChnObj failed ret = %d\n", ret);
		return -1;
	}
	
	if(NULL == psChnObj)
	{
		printf("ModEncodeSetParam GetChnObj psChnObj is null\n");
		return -2;
	}
	
	if(NULL == psEncodeParam)
	{
		return -10;
	}
	
	//int start,end;
	
	byChangeStatus = CheckEncodeParamChanged(emChnType, &psChnObj->sEncodeParam, psEncodeParam);
	if(0 == byChangeStatus)
	{
		return 0;
	}
	
	if(255 == byChangeStatus)
	{
		return -100;
	}
	
	u8 flag_main = 0;
	//u8 flag_sub = 0;
	
	switch(emChnType)
	{
		case EM_CHN_VIDEO_MAIN:
		{
			if(2 == byChangeStatus)
			{ 
				//ret = EncodePause(nChn, psChnObj);
				ret = tl_venc_stop(nChn);
				if(ret)
				{
					return -3;
				}
				//printf("tl_venc_stop chn[%d]\n", nChn);//csp modify
				flag_main = 1;
			}
			
			#if 0
			if(2 == byChangeStatus)
			{
				psChnObj = NULL;
				ret = GetChnObj(nChn, EM_CHN_VIDEO_SUB, &psChnObj);
				if((0==ret) && psChnObj && (psChnObj->sChnInfo.nRun))
				{
					ret = tl_venc_minor_stop(nChn);
					if(ret)
					{
						return -3;
					}
					//printf("tl_venc_minor_stop chn[%d]\n", nChn);
					flag_sub = 1;
				}
				
				ret = GetChnObj(nChn, EM_CHN_VIDEO_MAIN, &psChnObj);
				if(ret)
				{
					return -2;
				}
			}
			#endif
			
			pthread_mutex_lock(&psChnObj->pmLock);
			
			//printf("EM_CHN_VIDEO_MAIN-1 chn%d nGop:%d\n",nChn,psEncodeParam->sVideoMainParam.nGop);
			
			//csp modify
			psEncodeParam->sVideoMainParam.nGop = psEncodeParam->sVideoMainParam.nFrameRate * 2;
			psEncodeParam->sVideoMainParam.nGop = psEncodeParam->sVideoMainParam.nGop > 60 ? 60 : psEncodeParam->sVideoMainParam.nGop;
			
			//printf("EM_CHN_VIDEO_MAIN-2 chn%d nGop:%d\n",nChn,psEncodeParam->sVideoMainParam.nGop);
			
			if((psChnObj->sChnInfo.nRun) && !flag_main)
			{
				 MainStreamEncodeSetParam(nChn, psEncodeParam);
			}
			
			psChnObj->sEncodeParam = *psEncodeParam;
			
			pthread_mutex_unlock(&psChnObj->pmLock);
			
			if(2 == byChangeStatus/* && psEncodeParam->nForce*/)
			{
				//ret = EncodeResume(nChn, psChnObj);
				if(flag_main)
				{
					psChnObj->sChnInfo.nRestart = 1;
				}
			}
			
			#if 0
			if(2 == byChangeStatus)
			{
				psChnObj = NULL;
				ret = GetChnObj(nChn, EM_CHN_VIDEO_SUB, &psChnObj);
				if((0==ret) && psChnObj && flag_sub)
				{
					psChnObj->sChnInfo.nRestart = 1;
				}
			}
			#endif
		}
		break;
		case EM_CHN_VIDEO_SUB:
		{
			if(2 == byChangeStatus/* && psEncodeParam->nForce*/)
			{
				
				ret = EncodePause(nChn, psChnObj);
				if(ret)
				{
					return -3;
				}
			}
			
			pthread_mutex_lock(&psChnObj->pmLock);
			
			//printf("EM_CHN_VIDEO_SUB-1 chn%d nGop:%d\n",nChn,psEncodeParam->sVideoSubParam.nGop);
			
			//csp modify
			psEncodeParam->sVideoSubParam.nGop = psEncodeParam->sVideoSubParam.nFrameRate * 3;
			psEncodeParam->sVideoSubParam.nGop = psEncodeParam->sVideoSubParam.nGop > 60 ? 60 : psEncodeParam->sVideoSubParam.nGop;
			
			//printf("EM_CHN_VIDEO_SUB-2 chn%d nGop:%d\n",nChn,psEncodeParam->sVideoSubParam.nGop);
			
			//printf("psChnObj->sChnInfo.nRun %d\n", psChnObj->sChnInfo.nRun);
			if(psChnObj->sChnInfo.nRun)
			{
				 SubStreamEncodeSetParam(nChn, psEncodeParam);
			}
			
			psChnObj->sEncodeParam = *psEncodeParam;
			
			pthread_mutex_unlock(&psChnObj->pmLock);
			
			if(2 == byChangeStatus/* && psEncodeParam->nForce*/)
			{
				ret = EncodeResume(nChn, psChnObj);
				if(ret)
				{
					return -5;
				}
			}
			
			//return 0;
		}
		break;
		case EM_CHN_VIDEO_THIRD:
		{
			if(2 == byChangeStatus/* && psEncodeParam->nForce*/)
			{
				ret = EncodePause(nChn, psChnObj);
				if(ret)
				{
					return -3;
				}
			}
			
			pthread_mutex_lock(&psChnObj->pmLock);
			
			//csp modify
			psEncodeParam->sVideoSubParam.nGop = psEncodeParam->sVideoSubParam.nFrameRate * 3;
			psEncodeParam->sVideoSubParam.nGop = psEncodeParam->sVideoSubParam.nGop > 60 ? 60 : psEncodeParam->sVideoSubParam.nGop;
			
			if(psChnObj->sChnInfo.nRun)
			{
				ThirdStreamEncodeSetParam(nChn, psEncodeParam);
			}
			
			psChnObj->sEncodeParam = *psEncodeParam;
			
			pthread_mutex_unlock(&psChnObj->pmLock);
			
			if(2 == byChangeStatus/* && psEncodeParam->nForce*/)
			{
				ret = EncodeResume(nChn, psChnObj);
				if(ret)
				{
					return -5;
				}
			}
			
			//return 0;
		}
		break;
		case EM_CHN_VIDEO_SNAP:
		{
			if(2 == byChangeStatus && psEncodeParam->nForce)
			{
				ret = EncodePause(nChn, psChnObj);
				if(ret)
				{
					return -3;
				}
			}
			
			pthread_mutex_lock(&psChnObj->pmLock);
			
			if(psChnObj->sChnInfo.nRun)
			{
				SnapSetParam(nChn, psEncodeParam);
			}
			
			psChnObj->sEncodeParam = *psEncodeParam;
			
			pthread_mutex_unlock(&psChnObj->pmLock);
			
			if(2 == byChangeStatus && psEncodeParam->nForce)
			{
				ret = EncodeResume(nChn, psChnObj);
				if(ret)
				{
					return -5;
				}
			}
			
			//return 0;
		}
		break;
		case EM_CHN_AUDIO:
		{
			if(2 == byChangeStatus && psEncodeParam->nForce)
			{
				ret = EncodePause(nChn, psChnObj);
				if(ret)
				{
					return -3;
				}
			}
			
			pthread_mutex_lock(&psChnObj->pmLock);
			
			if(psChnObj->sChnInfo.nRun)
			{
				AudioEncodeSetParam(nChn, psEncodeParam);
			}
			
			psChnObj->sEncodeParam = *psEncodeParam;
			
			pthread_mutex_unlock(&psChnObj->pmLock);
			
			if(2 == byChangeStatus && psEncodeParam->nForce)
			{
				ret = EncodeResume(nChn, psChnObj);
				if(ret)
				{
					return -5;
				}
			}
			
			//return 0;
		}
		break;
		default:
			//return 0;
			
			break;
	}
	
	//gegang edit
	
	SOsdObj* psOsdObj = &g_s_EncodeManager.sOsdManage;
	if(!psOsdObj)
	{
		printf("encode mod has not been init,line is %d\n",__LINE__);
		return -1;
	}
	
	pthread_mutex_lock(&psOsdObj->pmLock);
	
	EMVIDEORESOLUTION emCurReso = psOsdObj->emResoCur[nChn];
	EMVIDEORESOLUTION emNewReso = g_s_EncodeManager.sChnMainManage.psChnObj[nChn].sEncodeParam.sVideoMainParam.emVideoResolution;
	if(emCurReso != emNewReso)//主码流分辨率变化
	{
		s32 ret;
		
		s32 rX,rY,rXSub,rYSub;
		GetRealCoord(nChn,psOsdObj->voaChn[nChn].x,psOsdObj->voaChn[nChn].y,&rX,&rY,&rXSub,&rYSub);
		
		#if defined(CHIP_HISI3521)//csp modify 20130316
		s32 rW,rH,rWSub,rHSub;
		s32 max_w = 352;
		s32 max_h = (g_s_EncodeManager.nVideoStandard == 10/*EM_BIZ_NTSC*/)?240:288;
		GetRealCoord(nChn,max_w,max_h,&rW,&rH,&rWSub,&rHSub);
		rX = (rW << 16) | rX;
		rY = (rH << 16) | rY;
		#endif
		
		if(psOsdObj->fdOsdChn[nChn] >= 0)
		{
			ret = tl_venc_osd_set_postion(psOsdObj->fdOsdChn[nChn], rX, rY);
			if(ret<0)
			{
				//printf("venc_osd set_postion faild\n");
				printf("venc_osd set_bitmap 2 faild,errcode=%d,errstr=%s\n\n\n",errno,strerror(errno));
			}
			
			#if defined(CHIP_HISI3521)//csp modify 20130316
			pthread_mutex_unlock(&psOsdObj->pmLock);
			ModEncodeOsdSet(nChn,EM_ENCODE_OSD_CHNNAME,NULL,0,psOsdObj->ChnName[nChn]);
			pthread_mutex_lock(&psOsdObj->pmLock);
			#endif
		}
		
		GetRealCoord(nChn,psOsdObj->voaTime[nChn].x,psOsdObj->voaTime[nChn].y,&rX,&rY,&rXSub,&rYSub);
		
		#if defined(CHIP_HISI3521)//csp modify 20130316
		rX = (rW << 16) | rX;
		rY = (rH << 16) | rY;
		#endif
		
		if(psOsdObj->fdOsdTime[nChn] >= 0)
		{
			ret = tl_venc_osd_set_postion(psOsdObj->fdOsdTime[nChn], rX, rY);
			if(ret<0)
			{
				//printf("venc_osd set_postion faild\n");
				printf("venc_osd set_bitmap 2 faild,errcode=%d,errstr=%s\n\n\n",errno,strerror(errno));
			}
		}
		
		GetRealCoord(nChn,psOsdObj->voaExt1[nChn].x,psOsdObj->voaExt1[nChn].y,&rX,&rY,&rXSub,&rYSub);
		
		#if defined(CHIP_HISI3521)//csp modify 20130316
		rX = (rW << 16) | rX;
		rY = (rH << 16) | rY;
		#endif
		
		if(psOsdObj->fdOsdExt1[nChn] >= 0)
		{
			ret = tl_venc_osd_set_postion(psOsdObj->fdOsdExt1[nChn], rX, rY);
			if(ret<0)
			{
				//printf("venc_osd set_postion faild\n");
				printf("venc_osd set_bitmap 2 faild,errcode=%d,errstr=%s\n\n\n",errno,strerror(errno));
			}
		}
		
		psOsdObj->emResoCur[nChn] = emNewReso;
	}
	
	#if defined(CHIP_HISI3531) || defined(CHIP_HISI3521)
	
	emCurReso = psOsdObj->emResoCurSub[nChn];
	emNewReso = g_s_EncodeManager.sChnSubManage.psChnObj[nChn].sEncodeParam.sVideoSubParam.emVideoResolution;
	if(emCurReso != emNewReso)//子码流分辨率变化
	{
		s32 ret;
		
		s32 rX,rY,rXSub,rYSub;
		GetRealCoord(nChn,psOsdObj->voaChn[nChn].x,psOsdObj->voaChn[nChn].y,&rX,&rY,&rXSub,&rYSub);
		
		#if defined(CHIP_HISI3521)//csp modify 20130316
		s32 rW,rH,rWSub,rHSub;
		s32 max_w = 352;
		s32 max_h = (g_s_EncodeManager.nVideoStandard == 10/*EM_BIZ_NTSC*/)?240:288;
		GetRealCoord(nChn,max_w,max_h,&rW,&rH,&rWSub,&rHSub);
		rXSub = (rWSub << 16) | rXSub;
		rYSub = (rHSub << 16) | rYSub;
		#endif
		
		if(psOsdObj->fdOsdChnSub[nChn] >= 0)
		{
			ret = tl_venc_osd_set_postion(psOsdObj->fdOsdChnSub[nChn], rXSub, rYSub);
			if(ret<0)
			{
				//printf("venc_osd set_postion faild\n");
				printf("sub venc_osd set_bitmap 2 faild,errcode=%d,errstr=%s\n\n\n",errno,strerror(errno));
			}
			
			#if defined(CHIP_HISI3521)//csp modify 20130316
			pthread_mutex_unlock(&psOsdObj->pmLock);
			ModEncodeOsdSet(nChn,EM_ENCODE_OSD_CHNNAME,NULL,0,psOsdObj->ChnName[nChn]);
			pthread_mutex_lock(&psOsdObj->pmLock);
			#endif
		}
		
		GetRealCoord(nChn,psOsdObj->voaTime[nChn].x,psOsdObj->voaTime[nChn].y,&rX,&rY,&rXSub,&rYSub);
		
		#if defined(CHIP_HISI3521)//csp modify 20130316
		rXSub = (rWSub << 16) | rXSub;
		rYSub = (rHSub << 16) | rYSub;
		#endif
		
		if(psOsdObj->fdOsdTimeSub[nChn] >= 0)
		{
			ret = tl_venc_osd_set_postion(psOsdObj->fdOsdTimeSub[nChn], rXSub, rYSub);
			if(ret<0)
			{
				//printf("venc_osd set_postion faild\n");
				printf("sub venc_osd set_bitmap 2 faild,errcode=%d,errstr=%s\n\n\n",errno,strerror(errno));
			}
		}
		
		psOsdObj->emResoCurSub[nChn] = emNewReso;
	}
	#endif
	
	pthread_mutex_unlock(&psOsdObj->pmLock);
	
	return 0;
}

s32 ModEncodeGetParam(u8 nChn, EMCHNTYPE emChnType, SEncodeParam* psEncodeParam)
{
	SChnObj* psChnObj = NULL;
	s32 ret;
	
	//u8 byChangeStatus = 0;
	
	ret = GetChnObj(nChn, emChnType, &psChnObj);
	if(ret)
	{
		printf("ModEncodeSetParam GetChnObj failed ret = %d\n", ret);
		return -1;
	}
	
	if(NULL == psChnObj)
	{
		printf("ModEncodeSetParam GetChnObj psChnObj is null\n");
		return -2;
	}
	
	if(NULL == psEncodeParam)
	{
		return -10;
	}
	
	*psEncodeParam = psChnObj->sEncodeParam;
	
	return 0;
}

// 0 nochange
// 1 general changed
// 2 key changed
// 255 err
u8 CheckEncodeParamChanged(EMCHNTYPE emChnType, SEncodeParam* psOldEncodeParam, SEncodeParam* psNewEncodeParam)
{
	if (psOldEncodeParam && psNewEncodeParam)
	{
		switch(emChnType)
		{
			case EM_CHN_VIDEO_MAIN:
				/*if (psOldEncodeParam->sVideoMainParam.emVideoResolution 
                    == psNewEncodeParam->sVideoMainParam.emVideoResolution)
				{
					return 0;
				}*/

				if (psOldEncodeParam->sVideoMainParam.emVideoResolution != psNewEncodeParam->sVideoMainParam.emVideoResolution)
				{
					return 2;
				}
				if (psOldEncodeParam->sVideoMainParam.nFrameRate!= psNewEncodeParam->sVideoMainParam.nFrameRate)
				{
					return 2;
				}
				if (psOldEncodeParam->sVideoMainParam.nBitRate!= psNewEncodeParam->sVideoMainParam.nBitRate)
				{
					return 2;
				}
				return 1;
			case EM_CHN_VIDEO_SUB:
			case EM_CHN_VIDEO_THIRD:
				
			/*	if (psOldEncodeParam->sVideoSubParam.emVideoResolution
                    == psNewEncodeParam->sVideoSubParam.emVideoResolution)
				{
					return 0;
				}*/

				if (psOldEncodeParam->sVideoSubParam.emVideoResolution != psNewEncodeParam->sVideoSubParam.emVideoResolution)
				{
					return 2;
				}
				
				return 1;

			case EM_CHN_VIDEO_SNAP:
			/*	if (psOldEncodeParam->sVideoSnapParam == psNewEncodeParam->sVideoSnapParam)
				{
					return 0;
				}*/

				//if (psOldEncodeParam.sVideoSnapParam.emVideoResolution != psNewEncodeParam.sVideoSnapParam.emVideoResolution)
				//{
				//	return 2;
				//}
				
				return 1;

			case EM_CHN_AUDIO:
				/*if (psOldEncodeParam->sAudioParam == psNewEncodeParam->sAudioParam)
				{
					return 0;
				}*/

				//if (psOldEncodeParam.sAudioParam.emVideoResolution != psNewEncodeParam.sAudioParam.emVideoResolution)
				//{
				//	return 2;
				//}
				
				return 1;
			default:
				break;
				
		}

	}

	return 255;
}

s32 ModEncodeGetKeyFrameEx(u8 nChn, SVideoParam* psEncodeParam, u8 bSub)
{
	venc_parameter_t venc_param;
	
	venc_param.bit_rate 		= psEncodeParam->nBitRate;
	venc_param.frame_rate 		= psEncodeParam->nFrameRate;
	venc_param.gop 				= 40;//psEncodeParam->sVideoMainParam.nGop;
	venc_param.is_cbr 			= (EM_BITRATE_CONST == psEncodeParam->emBitRateType) ? 1 : 0;
	venc_param.pic_level 		= (u32)psEncodeParam->emPicLevel;
	
	// I帧间隔保持在5秒左右，避免远程预览长时间停顿等待I帧
	//venc_param.gop = venc_param.frame_rate*5;//csp modify
	
	//csp modify
	if(bSub)
	{
		venc_param.gop = venc_param.frame_rate*3;
	}
	else
	{
		venc_param.gop = venc_param.frame_rate*2;
	}
	
	// 超过60帧间隔则置60，避免I帧过少引起编码问题
	venc_param.gop = venc_param.gop > 60 ? 60 : venc_param.gop;
	
	//printf("ModEncodeGetKeyFrameEx Chn[%d]: BitRate[%u], FrameRate[%u], Gop[%u], IsCbr[%d], PicLevel[%u]\n", nChn,venc_param.bit_rate,venc_param.frame_rate,venc_param.gop,venc_param.is_cbr,venc_param.pic_level);
	//printf("ModEncodeGetKeyFrameEx %s stream chn%d gop:%d\n", bSub?"sub":"main", nChn, venc_param.gop);
	
	//csp modify
	psEncodeParam->nGop = venc_param.gop;//csp modify
	if(bSub)
	{
		g_s_EncodeManager.sChnSubManage.psChnObj[nChn].sEncodeParam.sVideoSubParam.nGop = venc_param.gop;//csp modify
		//printf("ModEncodeGetKeyFrameEx sub stream chn%d nGop:%d\n", nChn, g_s_EncodeManager.sChnSubManage.psChnObj[nChn].sEncodeParam.sVideoSubParam.nGop);
	}
	else
	{
		g_s_EncodeManager.sChnMainManage.psChnObj[nChn].sEncodeParam.sVideoMainParam.nGop = venc_param.gop;//csp modify
		//printf("ModEncodeGetKeyFrameEx main stream chn%d nGop:%d\n", nChn, g_s_EncodeManager.sChnMainManage.psChnObj[nChn].sEncodeParam.sVideoMainParam.nGop);
	}
	
	//printf("ModEncodeGetKeyFrameEx:(%d,%d,%d)\n",bSub,g_s_EncodeManager.sChnSubManage.psChnObj[nChn].sChnInfo.nRun,g_s_EncodeManager.sChnMainManage.psChnObj[nChn].sChnInfo.nRun);
	
	//csp modify 20131123
	//if(bSub && g_s_EncodeManager.sChnSubManage.psChnObj[nChn].sChnInfo.nRun)
	if(bSub)
	{
		//g_s_EncodeManager.sChnSubManage.psChnObj[nChn].sEncodeParam.sVideoSubParam.nGop = venc_param.gop;//csp modify
		
		//tl_venc_minor_set_para(nChn, &venc_param);
		IPC_CMD_RequestIFrame(nChn+g_s_EncodeManager.sChnMainManage.nChnNum);
		//printf("$$$chn%d RequestIFrame$$$\n",nChn+g_s_EncodeManager.sChnMainManage.nChnNum);
	}
	//csp modify 20131123
	//else if((bSub == 0) && g_s_EncodeManager.sChnMainManage.psChnObj[nChn].sChnInfo.nRun)
	else if(bSub == 0)
	{
		//g_s_EncodeManager.sChnMainManage.psChnObj[nChn].sEncodeParam.sVideoMainParam.nGop = venc_param.gop;//csp modify
		
		//tl_venc_set_para(nChn, &venc_param);
		IPC_CMD_RequestIFrame(nChn);
		//printf("###chn%d RequestIFrame###\n",nChn);
	}
	
	return 0;
}

//zlbfix
s32 EncodePause(u8 nChn, SChnObj* psChnObj)
{
	if(psChnObj)
	{
		s32 retry = 0, retry2 = 0;
		
		//pthread_mutex_lock(&psChnObj->pmLock);
		
		for(; retry < 3; retry++)
		{
			if(0 == psChnObj->sChnInfo.nPause)
			{
				psChnObj->sChnInfo.nPause = 1;
				while(retry2 < 100)
				{
					if(0 == psChnObj->sChnInfo.nRun)
					{
						break;
					}
					
					usleep(10 * 1000);
					retry2++;
				}
				if(100 <= retry2)
				{
					psChnObj->sChnInfo.nPause = 0;
					//pthread_mutex_unlock(&psChnObj->pmLock);
					
					return -1;
				}
				
				//pthread_mutex_unlock(&psChnObj->pmLock);
				
				return 0;
			}
			usleep(500 * 1000);
		}
		
		//pthread_mutex_unlock(&psChnObj->pmLock);
		
		return -1;
	}
	
	return -2;
}

//zlbfix
s32 EncodeResume(u8 nChn, SChnObj* psChnObj)
{
	if(psChnObj)
	{
		pthread_mutex_lock(&psChnObj->pmLock);
		
		if(1 == psChnObj->sChnInfo.nPause)
		{
			psChnObj->sChnInfo.nPause = 0;			
		}
		
		pthread_mutex_unlock(&psChnObj->pmLock);
		
		return 0;
	}
	
	return -1;
}

s32 MainStreamEncodeSetParam(u8 nChn, SEncodeParam* psEncodeParam)//zlbfix
{
	venc_parameter_t venc_param;
	
	venc_param.bit_rate = psEncodeParam->sVideoMainParam.nBitRate;
	venc_param.frame_rate = psEncodeParam->sVideoMainParam.nFrameRate;
	venc_param.gop = 40;//psEncodeParam->sVideoMainParam.nGop;
	venc_param.is_cbr = (EM_BITRATE_CONST == psEncodeParam->sVideoMainParam.emBitRateType) ? 1 : 0;
	venc_param.pic_level = (u32)psEncodeParam->sVideoMainParam.emPicLevel;
	
	// I帧间隔保持在5秒左右，避免远程预览长时间停顿等待I帧
	//venc_param.gop = venc_param.frame_rate*5;//csp modify
	
	venc_param.gop = venc_param.frame_rate*2;//csp modify
	
	// 超过60帧间隔则置60，避免I帧过少引起编码问题
	venc_param.gop = venc_param.gop > 60 ? 60 : venc_param.gop;
	
	printf("MainStreamEncodeSetParam Chn[%d]: BitRate[%u], FrameRate[%u], Gop[%u], IsCbr[%d], PicLevel[%u]\n", nChn,venc_param.bit_rate,venc_param.frame_rate,venc_param.gop,venc_param.is_cbr,venc_param.pic_level);
	//printf("MainStreamEncodeSetParam chn%d gop %d\n", nChn, venc_param.gop);
	
	psEncodeParam->sVideoMainParam.nGop = venc_param.gop;//csp modify
	g_s_EncodeManager.sChnMainManage.psChnObj[nChn].sEncodeParam.sVideoMainParam.nGop = venc_param.gop;//csp modify
	
	tl_venc_set_para(nChn, 0, &venc_param);//0-主码流,1-次码流
	
	return 0;
}

s32 SubStreamEncodeSetParam(u8 nChn, SEncodeParam* psEncodeParam)//zlbfix
{
	venc_parameter_t venc_param;
	
	venc_param.bit_rate = psEncodeParam->sVideoSubParam.nBitRate;
	venc_param.frame_rate = psEncodeParam->sVideoSubParam.nFrameRate;
	venc_param.gop = psEncodeParam->sVideoSubParam.nGop;
	venc_param.is_cbr = (EM_BITRATE_CONST == psEncodeParam->sVideoSubParam.emBitRateType) ? 1 : 0;
	venc_param.pic_level = (u32)psEncodeParam->sVideoSubParam.emPicLevel;
	
	/*printf("func %s br %d fr %d gop %d cbr %d pic %d \n",
		__FUNCTION__,
		venc_param.bit_rate,
		venc_param.frame_rate,
		venc_param.gop,
		venc_param.is_cbr,
		venc_param.pic_level		
	);*/
	
	// I帧间隔保持在5秒左右，避免远程预览长时间停顿等待I帧
	//venc_param.gop = venc_param.frame_rate*5;//csp modify
	
	venc_param.gop = venc_param.frame_rate*3;//csp modify
	
	// 超过60帧间隔则置60，避免I帧过少引起编码问题
	venc_param.gop = venc_param.gop > 60 ? 60 : venc_param.gop;
	
	//printf("SubStreamEncodeSetParam chn%d gop %d\n", nChn, venc_param.gop);
	
	psEncodeParam->sVideoSubParam.nGop = venc_param.gop;//csp modify
	g_s_EncodeManager.sChnSubManage.psChnObj[nChn].sEncodeParam.sVideoSubParam.nGop = venc_param.gop;//csp modify
	
	tl_venc_minor_set_para(nChn, &venc_param);
	
	return 0;
}

s32 SnapSetParam(u8 nChn, SEncodeParam* psEncodeParam)
{
	return 0;
}

s32 AudioEncodeSetParam(u8 nChn, SEncodeParam* psEncodeParam)
{
	return 0;
}

s32 MainStreamEncodeGetParam(SChnObj* psMainStreamObj, SEncodeParam* psEncodeParam)
{
	psEncodeParam->sVideoMainParam.nFrameRate
 		= psMainStreamObj->sEncodeParam.sVideoMainParam.nFrameRate;
	psEncodeParam->sVideoMainParam.nBitRate
		= psMainStreamObj->sEncodeParam.sVideoMainParam.nBitRate;
	psEncodeParam->sVideoMainParam.nGop
		= psMainStreamObj->sEncodeParam.sVideoMainParam.nGop;
	psEncodeParam->sVideoMainParam.nMaxQP
		= psMainStreamObj->sEncodeParam.sVideoMainParam.nMaxQP;
	psEncodeParam->sVideoMainParam.nMinQP
		= psMainStreamObj->sEncodeParam.sVideoMainParam.nMinQP;
	psEncodeParam->sVideoMainParam.emEncodeType
		= psMainStreamObj->sEncodeParam.sVideoMainParam.emEncodeType;
	psEncodeParam->sVideoMainParam.emVideoResolution
		= psMainStreamObj->sEncodeParam.sVideoMainParam.emVideoResolution;
	psEncodeParam->sVideoMainParam.emPicLevel
		= psMainStreamObj->sEncodeParam.sVideoMainParam.emPicLevel;
	psEncodeParam->sVideoMainParam.emBitRateType
		= psMainStreamObj->sEncodeParam.sVideoMainParam.emBitRateType;

	return 0;
}

s32 SubStreamEncodeGetParam(SChnObj* psSubStreamObj, SEncodeParam* psEncodeParam)
{
	psEncodeParam->sVideoSubParam.nFrameRate
 		= psSubStreamObj->sEncodeParam.sVideoSubParam.nFrameRate;
	psEncodeParam->sVideoSubParam.nBitRate
		= psSubStreamObj->sEncodeParam.sVideoSubParam.nBitRate;
	psEncodeParam->sVideoSubParam.nGop
		= psSubStreamObj->sEncodeParam.sVideoSubParam.nGop;
	psEncodeParam->sVideoSubParam.nMaxQP
		= psSubStreamObj->sEncodeParam.sVideoSubParam.nMaxQP;
	psEncodeParam->sVideoSubParam.nMinQP
		= psSubStreamObj->sEncodeParam.sVideoSubParam.nMinQP;
	psEncodeParam->sVideoSubParam.emEncodeType
		= psSubStreamObj->sEncodeParam.sVideoSubParam.emEncodeType;
	psEncodeParam->sVideoSubParam.emVideoResolution
		= psSubStreamObj->sEncodeParam.sVideoSubParam.emVideoResolution;
	psEncodeParam->sVideoSubParam.emPicLevel
		= psSubStreamObj->sEncodeParam.sVideoSubParam.emPicLevel;
	psEncodeParam->sVideoSubParam.emBitRateType
		= psSubStreamObj->sEncodeParam.sVideoSubParam.emBitRateType;

	return 0;
}

s32 SnapGetParam(SChnObj* psSnapObj, SEncodeParam* psEncodeParam)
{
	return 0;
}

s32 AudioEncodeGetParam(SChnObj* psAudioObj, SEncodeParam* psEncodeParam)
{
	psEncodeParam->sAudioParam.nBitWidth
		= psAudioObj->sEncodeParam.sAudioParam.nBitWidth;
	psEncodeParam->sAudioParam.nSampleDuration
		= psAudioObj->sEncodeParam.sAudioParam.nSampleDuration;
	psEncodeParam->sAudioParam.nSampleRate
		= psAudioObj->sEncodeParam.sAudioParam.nSampleRate;
	psEncodeParam->sAudioParam.nSampleSize
		= psAudioObj->sEncodeParam.sAudioParam.nSampleSize;
	psEncodeParam->sAudioParam.emEncodeType
		= psAudioObj->sEncodeParam.sAudioParam.emEncodeType;

	return 0;
}

s32 ModEncodeRequestStart(u8 nChn, EMCHNTYPE emChnType, PFNEncodeRequest pfnEncodeCB, u32 nContent) //zlbfix
{
	SChnObj* psChnObj = NULL;
	s32 ret;
	
	ret = GetChnObj(nChn, emChnType, &psChnObj);
	if(ret)
	{
		printf("ModEncodeRequestStart GetChnObj failed ret = %d\n", ret);
		return -1;
	}
	
	if(NULL == psChnObj)
	{
		printf("ModEncodeRequestStart GetChnObj psChnObj is null\n");
		return -2;
	}
	
	switch(emChnType)
	{
		case EM_CHN_VIDEO_MAIN:
		case EM_CHN_VIDEO_SUB:
		case EM_CHN_AUDIO:
		case EM_CHN_TALK:
			pthread_mutex_lock(&psChnObj->pmLock);
			
			if(0 == psChnObj->sChnInfo.nStart)
			{
				//printf("func %s emChnType %d\n", __FUNCTION__, emChnType);
				psChnObj->sChnInfo.nContent = nContent;
				psChnObj->sChnInfo.pfnEncodeProc = pfnEncodeCB;
				psChnObj->sChnInfo.nStart = 1;
			}
			else
			{
				pthread_mutex_unlock(&psChnObj->pmLock);
				
				return -3;
			}
			
			pthread_mutex_unlock(&psChnObj->pmLock);
			
			return 0;
		case EM_CHN_VIDEO_THIRD://csp modify 20130423
		
			printf("%s emChnType %d, start: %d\n", __func__, emChnType, psChnObj->sChnInfo.nStart);
			if(!ModEncodeSupportThirdStream())
			{
				return -1;
			}
			
			pthread_mutex_lock(&psChnObj->pmLock);
			
			if(0 == psChnObj->sChnInfo.nStart)
			{
				//printf("func %s emChnType %d\n", __FUNCTION__, emChnType);
				psChnObj->sChnInfo.nContent = nContent;
				psChnObj->sChnInfo.pfnEncodeProc = pfnEncodeCB;
				psChnObj->sChnInfo.nStart = 1;
			}
			else
			{
				pthread_mutex_unlock(&psChnObj->pmLock);
				
				return -3;
			}
			
			pthread_mutex_unlock(&psChnObj->pmLock);
			
			return 0;
		case EM_CHN_VIDEO_SNAP:
			
			return 0;
			
		default:
			return -5;
	}
}

s32 ModEncodeRequestStop(u8 nChn, EMCHNTYPE emChnType)
{
	SChnObj* psChnObj = NULL;
	s32 ret;
	
	ret = GetChnObj(nChn, emChnType, &psChnObj);
	if (ret)
	{
		printf("ModEncodeRequestStop GetChnObj failed ret = %d\n", ret);
		return -1;
	}
	
	if (NULL == psChnObj)
	{
		printf("ModEncodeRequestStop GetChnObj psChnObj is null\n");
		return -2;
	}
	
	switch (emChnType)
	{
		case EM_CHN_VIDEO_MAIN:
		case EM_CHN_VIDEO_SUB:
		case EM_CHN_AUDIO:
		case EM_CHN_TALK:
			//printf("ModEncodeRequestStop   pthread_mutex_lock line %d\n",__LINE__);
			
			pthread_mutex_lock(&psChnObj->pmLock);
			
			//printf("ModEncodeRequestStop  pthread_mutex_lock line %d\n",__LINE__);
			
			if (1 == psChnObj->sChnInfo.nStart)
			{
				psChnObj->sChnInfo.nStart = 0;
				psChnObj->sChnInfo.nContent = 0;
				psChnObj->sChnInfo.pfnEncodeProc = NULL;
			}
			else
			{
				pthread_mutex_unlock(&psChnObj->pmLock);
				
				return -3;
			}
			
			pthread_mutex_unlock(&psChnObj->pmLock);
			
			//printf(" type [%d] stream stop request successful!!\n", emChnType);
			
			return 0;

		case EM_CHN_VIDEO_THIRD://csp modify 20130423
			if(!ModEncodeSupportThirdStream())
			{
				return -1;
			}
			
			pthread_mutex_lock(&psChnObj->pmLock);
			
			if (1 == psChnObj->sChnInfo.nStart)
			{
				psChnObj->sChnInfo.nStart = 0;
				psChnObj->sChnInfo.nContent = 0;
				psChnObj->sChnInfo.pfnEncodeProc = NULL;
			}
			else
			{
				pthread_mutex_unlock(&psChnObj->pmLock);
				
				return -3;
			}
			
			pthread_mutex_unlock(&psChnObj->pmLock);
			
			return 0;

		case EM_CHN_VIDEO_SNAP:
			
			return 0;

		default:
			return -5;
	}	
}

//zlbfix
//函数功能获得通道对象
s32 GetChnObj(u8 nChn, EMCHNTYPE emChnType, SChnObj** ppsChnObj)
{
	if (NULL == ppsChnObj)
	{
		return -1;
	}
	
	switch(emChnType)
	{
		case EM_CHN_VIDEO_MAIN:
			if (nChn >= g_s_EncodeManager.sChnMainManage.nChnNum)
			{
				*ppsChnObj = NULL;
				return -3;
			}

			*ppsChnObj = &g_s_EncodeManager.sChnMainManage.psChnObj[nChn];
			return 0;
		case EM_CHN_VIDEO_THIRD://csp modify 20130423
			if (!ModEncodeSupportThirdStream())
			{
				*ppsChnObj = NULL;
				return -3;
			}
			if (nChn >= g_s_EncodeManager.sChnThirdManage.nChnNum)
			{
				*ppsChnObj = NULL;
				return -3;
			}

			*ppsChnObj = &g_s_EncodeManager.sChnThirdManage.psChnObj[nChn];
			return 0;
		case EM_CHN_VIDEO_SUB:
			if (nChn >= g_s_EncodeManager.sChnSubManage.nChnNum)
			{
				*ppsChnObj = NULL;
				return -4;
			}

			*ppsChnObj = &g_s_EncodeManager.sChnSubManage.psChnObj[nChn];
			return 0;
		case EM_CHN_VIDEO_SNAP:
			if (nChn >= g_s_EncodeManager.sChnSnapManage.nChnNum)
			{
				*ppsChnObj = NULL;
				return -4;
			}

			*ppsChnObj = &g_s_EncodeManager.sChnSnapManage.psChnObj[nChn];
			return 0;
		case EM_CHN_AUDIO:
			if (nChn >= g_s_EncodeManager.sChnAudioManage.nChnNum)
			{
				*ppsChnObj = NULL;
				return -4;
			}

			*ppsChnObj = &g_s_EncodeManager.sChnAudioManage.psChnObj[nChn];
			return 0;
		case EM_CHN_TALK:
			if (nChn >= g_s_EncodeManager.sChnTalkManage.nChnNum)
			{
				*ppsChnObj = NULL;
				return -4;
			}

			*ppsChnObj = &g_s_EncodeManager.sChnTalkManage.psChnObj[0];
			return 0;
		break;
		default:
			*ppsChnObj = NULL;
			return -2;
	}
}

s32 CreateOsdObj(SOsdObj* psOsdObj, u8 nChnNum)
{
	if(!psOsdObj || nChnNum<=0)
	{
		printf("create Osd object fail!\n");
		return -1;
	}
	
	psOsdObj->nChnNum = nChnNum;
	
	//csp modify
	//s32 i = 0, j = 0;
	s32 i = 0;
	
	if(!(psOsdObj->fdOsdChn = malloc(nChnNum*sizeof(s32))))
	{
		printf("Line:%d OOM!\n",__LINE__);
		return -1;
	}
	
	for(i=0; i<nChnNum; i++)
	{
		psOsdObj->fdOsdChn[i] = -1;
	}
	
	if(!(psOsdObj->voaChn = calloc(nChnNum, sizeof(venc_osd_atr_t))))
	{
		printf("Line:%d OOM!\n",__LINE__);
		return -1;
	}
	
	if(!(psOsdObj->fdOsdTime = malloc(nChnNum*sizeof(s32))))
	{
		printf("Line:%d OOM!\n",__LINE__);
		return -1;
	}
	
	for(i=0; i<nChnNum; i++)
	{
		psOsdObj->fdOsdTime[i] = -1;
	}
	
	if(!(psOsdObj->voaTime = calloc(nChnNum, sizeof(venc_osd_atr_t))))
	{
		printf("Line:%d OOM!\n",__LINE__);
		return -1;
	}
	
	if(!(psOsdObj->fdOsdExt1 = malloc(nChnNum*sizeof(s32))))
	{
		printf("Line:%d OOM!\n",__LINE__);
		return -1;
	}
	
	for(i=0; i<nChnNum; i++)
	{
		psOsdObj->fdOsdExt1[i] = -1;
	}
	
	if(!(psOsdObj->voaExt1 = calloc(nChnNum, sizeof(venc_osd_atr_t))))
	{
		printf("Line:%d OOM!\n",__LINE__);
		return -1;
	}
	
	if(!(psOsdObj->emResoRef = calloc(nChnNum, sizeof(EMVIDEORESOLUTION))))
	{
		printf("Line:%d OOM!\n",__LINE__);
		return -1;
	}
	
	if(!(psOsdObj->emResoCur = calloc(nChnNum, sizeof(EMVIDEORESOLUTION))))
	{
		printf("Line:%d OOM!\n",__LINE__);
		return -1;
	}
	
	#if defined(CHIP_HISI3531) || defined(CHIP_HISI3521)
	if(!(psOsdObj->fdOsdChnSub = malloc(nChnNum*sizeof(s32))))
	{
		printf("Line:%d OOM!\n",__LINE__);
		return -1;
	}
	for(i=0; i<nChnNum; i++)
	{
		psOsdObj->fdOsdChnSub[i] = -1;
	}
	if(!(psOsdObj->fdOsdTimeSub = malloc(nChnNum*sizeof(s32))))
	{
		printf("Line:%d OOM!\n",__LINE__);
		return -1;
	}
	for(i=0; i<nChnNum; i++)
	{
		psOsdObj->fdOsdTimeSub[i] = -1;
	}
	if(!(psOsdObj->emResoCurSub = calloc(nChnNum, sizeof(EMVIDEORESOLUTION))))
	{
		printf("Line:%d OOM!\n",__LINE__);
		return -1;
	}
	#endif
	
	//csp modify 20130423
	#if defined(CHIP_HISI3531)// || defined(CHIP_HISI3521)
	if(ModEncodeSupportThirdStream())
	{
		if(!(psOsdObj->fdOsdChnThird = malloc(nChnNum*sizeof(s32))))
		{
			printf("Line:%d OOM!\n",__LINE__);
			return -1;
		}
		for(i=0; i<nChnNum; i++)
		{
			psOsdObj->fdOsdChnThird[i] = -1;
		}
		if(!(psOsdObj->fdOsdTimeThird = malloc(nChnNum*sizeof(s32))))
		{
			printf("Line:%d OOM!\n",__LINE__);
			return -1;
		}
		for(i=0; i<nChnNum; i++)
		{
			psOsdObj->fdOsdTimeThird[i] = -1;
		}
		//if(!(psOsdObj->emResoCurThird = calloc(nChnNum, sizeof(EMVIDEORESOLUTION))))
		//{
		//	printf("Line:%d OOM!\n",__LINE__);
		//	return -1;
		//}
		//for(i=0; i<nChnNum; i++)
		//{
		//	psOsdObj->emResoCurThird[i] = g_s_EncodeManager.sChnThirdManage.psChnObj[i].sEncodeParam.sVideoMainParam.emVideoResolution;
		//}
	}
	#endif
	
	//csp modify 20130316
	if(!(psOsdObj->ChnName = malloc(nChnNum*sizeof(s8*))))
	{
		printf("Line:%d OOM!\n",__LINE__);
		return -1;
	}
	for(i=0; i<nChnNum; i++)
	{
		if(!(psOsdObj->ChnName[i] = malloc(32*sizeof(s8))))
		{
			printf("Line:%d OOM!\n",__LINE__);
			return -1;
		}
		memset(psOsdObj->ChnName[i],0,32);
	}
	
	//memset(psOsdObj->osdBuf,0,bufSize);
	
	pthread_mutex_init(&psOsdObj->pmLock, NULL);
	
	printf("CreateOsdObj finish\n");
	
	return 0;
}

s32 DestroyOsdObj(SOsdObj* psOsdObj)
{
	if(!psOsdObj)
	{
		printf("OsdObj has been destory \n");
		return 1;
	}

	if(psOsdObj->fdOsdChn)
	{
		free(psOsdObj->fdOsdChn);
		psOsdObj->fdOsdChn = NULL;
	}

	if(psOsdObj->fdOsdTime)
	{
		free(psOsdObj->fdOsdTime);
		psOsdObj->fdOsdTime = NULL;
	}

	if(psOsdObj->fdOsdExt1)
	{
		free(psOsdObj->fdOsdExt1);
		psOsdObj->fdOsdExt1 = NULL;
	}

	if(psOsdObj->voaChn)
	{
		free(psOsdObj->voaChn);
		psOsdObj->voaChn = NULL;
	}

	if(psOsdObj->voaTime)
	{
		free(psOsdObj->voaTime);
		psOsdObj->voaTime = NULL;
	}

	if(psOsdObj->voaExt1)
	{
		free(psOsdObj->voaExt1);
		psOsdObj->voaExt1 = NULL;
	}

	if(psOsdObj->emResoCur)
	{
		free(psOsdObj->emResoCur);
		psOsdObj->emResoCur= NULL;
	}

	if(psOsdObj->emResoRef)
	{
		free(psOsdObj->emResoRef);
		psOsdObj->emResoRef = NULL;
	}
	
	#if defined(CHIP_HISI3531) || defined(CHIP_HISI3521)
	if(psOsdObj->fdOsdChnSub)
	{
		free(psOsdObj->fdOsdChnSub);
		psOsdObj->fdOsdChnSub = NULL;
	}
	if(psOsdObj->fdOsdTimeSub)
	{
		free(psOsdObj->fdOsdTimeSub);
		psOsdObj->fdOsdTimeSub = NULL;
	}
	if(psOsdObj->emResoCurSub)
	{
		free(psOsdObj->emResoCurSub);
		psOsdObj->emResoCurSub = NULL;
	}
	#endif
	
	//csp modify 20130423
	#if defined(CHIP_HISI3531)// || defined(CHIP_HISI3521)
	if(ModEncodeSupportThirdStream())
	{
		if(psOsdObj->fdOsdChnThird)
		{
			free(psOsdObj->fdOsdChnThird);
			psOsdObj->fdOsdChnThird = NULL;
		}
		if(psOsdObj->fdOsdTimeThird)
		{
			free(psOsdObj->fdOsdTimeThird);
			psOsdObj->fdOsdTimeThird = NULL;
		}
		//if(psOsdObj->emResoCurThird)
		//{
		//	free(psOsdObj->emResoCurThird);
		//	psOsdObj->emResoCurThird = NULL;
		//}
	}
	#endif
	
	psOsdObj->pfnOsdCB = NULL;
	
	pthread_mutex_destroy(&psOsdObj->pmLock);
	
	return 0;//csp modify
}


s32 ModEncodeOsdRegistCB(PFNEncodeTextToOsd pfnGetOSDCB)
{	
	if(!pfnGetOSDCB)
	{
		printf("Osd callback error\n");
		return -1;
	}
	
	SOsdObj* psOsdObj = &g_s_EncodeManager.sOsdManage;
	if(0 == psOsdObj->nChnNum)
	{
		printf("encode mod has not been init,line is %d\n",__LINE__);
		return -1;
	}
	
	if(!pfnGetOSDCB)
	{
		printf("callback error\n");
		return -1;
	}
	
	psOsdObj->pfnOsdCB = pfnGetOSDCB;
	
	return 0;
}

s32 ModEncodeOsdResolRef(u8 nChn, EMVIDEORESOLUTION emReSolRef)
{
	SOsdObj* psOsdObj = &g_s_EncodeManager.sOsdManage;
	if(0 == psOsdObj->nChnNum)
	{
		printf("encode mod has not been init\n");
		return -1;
	}
	
	u8 nMaxCh = psOsdObj->nChnNum;
	if(nChn >= nMaxCh)
	{
		printf("chn param error\n");
		return -1;
	}
	
	if(!psOsdObj->emResoRef)
	{
		printf("encode mod has not been init,line is %d\n",__LINE__);
		return -1;
	}
	
//#ifdef CHIP_HISI3531
//	emReSolRef = EM_VIDEO_1080P;
//#endif
	
	pthread_mutex_lock(&psOsdObj->pmLock);
	
	psOsdObj->emResoRef[nChn] = emReSolRef;
	
	pthread_mutex_unlock(&psOsdObj->pmLock);
	
	return 0;
}

s32* GetOsdFd(EMENCODEOSDTYPE emOsdType)
{
	SOsdObj* psOsdObj = &g_s_EncodeManager.sOsdManage;
	
	switch(emOsdType)
	{
		case EM_ENCODE_OSD_CHNNAME:
		{
			return psOsdObj->fdOsdChn;
		}break;
		case EM_ENCODE_OSD_TIME:
		{
			return psOsdObj->fdOsdTime;
		}break;
		case EM_ENCODE_OSD_EXT1:
		{
			return psOsdObj->fdOsdExt1;
		}break;
		default:
			break;
	}
	
	return 0;
}

#if defined(CHIP_HISI3531) || defined(CHIP_HISI3521)
s32* GetOsdFdSub(EMENCODEOSDTYPE emOsdType)
{
	SOsdObj* psOsdObj = &g_s_EncodeManager.sOsdManage;
	
	switch(emOsdType)
	{
		case EM_ENCODE_OSD_CHNNAME:
		{
			return psOsdObj->fdOsdChnSub;
		}break;
		case EM_ENCODE_OSD_TIME:
		{
			return psOsdObj->fdOsdTimeSub;
		}break;
		default:
			break;
	}
	
	return 0;
}
#endif

//csp modify 20130423
#if defined(CHIP_HISI3531)// || defined(CHIP_HISI3521)
s32* GetOsdFdThird(EMENCODEOSDTYPE emOsdType)
{
	SOsdObj* psOsdObj = &g_s_EncodeManager.sOsdManage;
	
	switch(emOsdType)
	{
		case EM_ENCODE_OSD_CHNNAME:
		{
			return psOsdObj->fdOsdChnThird;
		}break;
		case EM_ENCODE_OSD_TIME:
		{
			return psOsdObj->fdOsdTimeThird;
		}break;
		default:
			break;
	}
	
	return 0;
}
#endif

venc_osd_atr_t* GetOsdVoa(EMENCODEOSDTYPE emOsdType)
{
	SOsdObj* psOsdObj = &g_s_EncodeManager.sOsdManage;
	
	switch(emOsdType)
	{
		case EM_ENCODE_OSD_CHNNAME:
		{
			return psOsdObj->voaChn;
		}break;
		case EM_ENCODE_OSD_TIME:
		{
			return psOsdObj->voaTime;
		}break;
		case EM_ENCODE_OSD_EXT1:
		{
			return psOsdObj->voaExt1;
		}break;
		default:
			break;
	}
	
	return 0;
}

/*
u8** GetOsdStr(EMENCODEOSDTYPE emOsdType)
{
	SOsdObj* psOsdObj = &g_s_EncodeManager.sOsdManage;
	
	switch(emOsdType)
	{
		case EM_ENCODE_OSD_CHNNAME:
		{
			return psOsdObj->strChn;
		}break;
		case EM_ENCODE_OSD_TIME:
		{
			return psOsdObj->strTime;
		}break;
		case EM_ENCODE_OSD_EXT1:
		{
			return psOsdObj->strExt1;
		}break;
		default:
			break;
	}
	
	return 0;
}
*/

s32 ModEncodeOsdInit(u8 nChn, EMENCODEOSDTYPE emOsdType, SRect* pRect, SRgb* pBgColor)
{
	//int i = 0;
	//for(i=0;i<g_s_EncodeManager.sChnSubManage.nChnNum;i++)
	//{
	//	printf("ModEncodeOsdInit start chn%d gop:%d\n",i,g_s_EncodeManager.sChnSubManage.psChnObj[i].sEncodeParam.sVideoSubParam.nGop);
	//}
	
	//csp modify for test
	//if(emOsdType == EM_ENCODE_OSD_TIME)
	//{
	//	return 0;
	//}
	
	if(pRect)
	{
		//printf("ModEncodeOsdInit start,Chn:%d,OsdType:%d,nChnNum:%d,rect:(%d,%d,%d,%d)\n",nChn,emOsdType,g_s_EncodeManager.sOsdManage.nChnNum,pRect->x,pRect->y,pRect->w,pRect->h);
		
		//#if defined(CHIP_HISI3531)//csp modify 20130429
		//pRect->x = pRect->x / 16 * 16;
		//pRect->y = pRect->y / 16 * 16;
		//pRect->w = (pRect->w + 15) / 16 * 16;
		//pRect->h = (pRect->h + 15) / 16 * 16;
		//#endif
		
		#if defined(CHIP_HISI3521)//csp modify 20130316
		pRect->x = pRect->x / 8 * 8;
		pRect->y = pRect->y / 8 * 8;
		pRect->w = (pRect->w + 7) / 8 * 8;
		pRect->h = (pRect->h + 7) / 8 * 8;
		#endif
	}
	else
	{
		//printf("ModEncodeOsdInit start,Chn:%d,OsdType:%d,nChnNum:%d\n",nChn,emOsdType,g_s_EncodeManager.sOsdManage.nChnNum);
	}
	
	SOsdObj* psOsdObj = &g_s_EncodeManager.sOsdManage;
	if(0 == psOsdObj->nChnNum)
	{
		printf("encode mod has not been init,line is %d\n",__LINE__);
		return -1;
	}
	
	if(nChn >= psOsdObj->nChnNum)
	{
		printf("ch param error\n");
		return -1;
	}
	
	if(!pRect)
	{
		printf("region param error\n");
		return -1;
	}
	
	pthread_mutex_lock(&psOsdObj->pmLock);
	
	venc_osd_atr_t* pVoa = GetOsdVoa(emOsdType);
	
	pVoa[nChn].x = pRect->x;
	pVoa[nChn].y = pRect->y;
	
#if 0
	printf("width : %d,height : %d,w : %d, h :%d, chn = %d,emOsdType = %d\n",pVoa[nChn].width,pVoa[nChn].height,pRect->w,pRect->h,nChn,emOsdType);
	if(pVoa[nChn].width != pRect->w
		|| pVoa[nChn].height != pRect->h)
	{
		if(psOsdObj->osdBuf)
		{
			printf("@@@@@%d\n",__LINE__);
			free(psOsdObj->osdBuf);
			printf("@@@@@%d\n",__LINE__);
		}
		printf("@@@@@%d\n",__LINE__);
		psOsdObj->osdBuf = malloc(pRect->w*pRect->h*2);
		printf("@@@@@%d\n",__LINE__);
	}
	
	memset(psOsdObj->osdBuf, 0, pRect->w*pRect->h*2);
#endif
	
	pVoa[nChn].width = pRect->w;
	pVoa[nChn].height = pRect->h;
	
#if defined(CHIP_HISI3531)
	//3531的主码流的OSD做过2倍放大,故做下面的处理,否则OSD会显示不全
	pVoa[nChn].width *= 3;
	pVoa[nChn].height *= 3;
#endif
	
	pVoa[nChn].bg_color = 0;
	if(pBgColor)
	{
		u8 r = pBgColor->nR>>3;
		u8 g = pBgColor->nG>>3;
		u8 b = pBgColor->nB>>3;
		//u8 t = pBgColor->nTrans;
		u16 bgColor = r<<10 | g<<5 | b;
		pVoa[nChn].bg_color = bgColor;
	}
	
	pVoa[nChn].bg_alpha = 0;
	pVoa[nChn].fg_alpha = 128;//0~128
	
	s32* pFd = GetOsdFd(emOsdType);
	
	//printf("ModEncodeOsdInit:chn=%d,type=%d,x=%d,y=%d,w=%d,h=%d\n",nChn,emOsdType,pVoa[nChn].x,pVoa[nChn].y,pVoa[nChn].width,pVoa[nChn].height);
	
#if defined(CHIP_HISI3521)
	venc_osd_atr_t vosdatr = pVoa[nChn];
	
	s32 rX,rY,rXSub,rYSub;
	GetRealCoord(nChn,vosdatr.x,vosdatr.y,&rX,&rY,&rXSub,&rYSub);
	
	s32 rW,rH,rWSub,rHSub;
	s32 max_w = 352;
	s32 max_h = (g_s_EncodeManager.nVideoStandard == 10/*EM_BIZ_NTSC*/)?240:288;
	GetRealCoord(nChn,max_w,max_h,&rW,&rH,&rWSub,&rHSub);
	
	vosdatr.x = rX;
	vosdatr.y = rY;
	pFd[nChn] = tl_venc_osd_create(nChn, &vosdatr, ((rW << 16) | rH));
	
	vosdatr.x = rXSub;
	vosdatr.y = rYSub;
#else
	pFd[nChn] = tl_venc_osd_create(nChn, &pVoa[nChn], 0);
#endif
	
	//printf("##################type=%d,pFd[%d] = %d\n",emOsdType,nChn,pFd[nChn]);
	
	if(pFd[nChn]<0)
	{
		printf("creat venc_osd faild, ch %d Line:%d\n",nChn, __LINE__);
		pthread_mutex_unlock(&psOsdObj->pmLock);
		return -1;
	}
	
#if defined(CHIP_HISI3531)
	pFd = GetOsdFdSub(emOsdType);
	pFd[nChn] = tl_venc_osd_create(nChn + g_s_EncodeManager.sOsdManage.nChnNum, &pVoa[nChn], 0);
	if(pFd[nChn]<0)
	{
		printf("creat venc_osd faild, ch %d Line:%d\n", nChn, __LINE__);
		pthread_mutex_unlock(&psOsdObj->pmLock);
		return -1;
	}
#elif defined(CHIP_HISI3521)
	pFd = GetOsdFdSub(emOsdType);
	pFd[nChn] = tl_venc_osd_create(nChn + g_s_EncodeManager.sOsdManage.nChnNum, &vosdatr, ((rWSub << 16) | rHSub));
	if(pFd[nChn]<0)
	{
		printf("creat venc_osd faild, ch %d Line:%d\n", nChn, __LINE__);
		pthread_mutex_unlock(&psOsdObj->pmLock);
		return -1;
	}
#endif
	
	//csp modify 20130423
#if defined(CHIP_HISI3531)// || defined(CHIP_HISI3521)
	if(ModEncodeSupportThirdStream())
	{
		pFd = GetOsdFdThird(emOsdType);
		pFd[nChn] = tl_venc_osd_create(nChn + g_s_EncodeManager.sOsdManage.nChnNum * 2, &pVoa[nChn], 0);
		if(pFd[nChn]<0)
		{
			printf("creat venc_osd faild, ch %d Line:%d\n", nChn, __LINE__);
			pthread_mutex_unlock(&psOsdObj->pmLock);
			return -1;
		}
	}
#endif
	
	pthread_mutex_unlock(&psOsdObj->pmLock);
	
	//printf("ModEncodeOsdInit end,Chn:%d,OsdType:%d\n",nChn,emOsdType);
	
	//int i = 0;
	//for(i=0;i<g_s_EncodeManager.sChnSubManage.nChnNum;i++)
	//{
	//	printf("ModEncodeOsdInit end chn%d gop:%d\n",i,g_s_EncodeManager.sChnSubManage.psChnObj[i].sEncodeParam.sVideoSubParam.nGop);
	//}
	
	return 0;
}

//应该用TDE放大
//放大位图,参数说明:src:位图源数据,w:原始位图的宽(像素), h:高,nBpP:每个像素点由几个字节表示
//des:用于存放放大后的位图数据,x:放大倍数
#if 0//#if defined(CHIP_HISI3531)//csp modify 20130429
void EnlargeBitmap(u32 src, u32 w, u32 h, u8 nBpP, u32 des, u8 x)
#else
void EnlargeBitmap(u8* src, u32 w, u32 h, u8 nBpP, u8* des, u8 x)
#endif
{
#if 0//#if defined(CHIP_HISI3531)//csp modify 20130429
	if((0==src)||(0==w)||(0==h)||(0==nBpP)||(0==des)||(0==x))
	{
		return;
	}
	
	pthread_mutex_lock(&g_tde2_lock);
	
	int s32Ret;
	TDE_HANDLE s32Handle;
	TDE2_SURFACE_S stSrc, stDst;
	TDE2_RECT_S stSrcRect, stDstRect;
	
	memset(&stSrc, 0, sizeof(TDE2_SURFACE_S));
	memset(&stDst, 0, sizeof(TDE2_SURFACE_S));
	memset(&stSrcRect, 0, sizeof(TDE2_RECT_S));
	memset(&stDstRect, 0, sizeof(TDE2_RECT_S));
	
	s32Handle = HI_TDE2_BeginJob();
	if(HI_ERR_TDE_INVALID_HANDLE == s32Handle)
	{
		printf("HI_TDE2_BeginJob failed\n");
		pthread_mutex_unlock(&g_tde2_lock);
		return;
	}
	
	stSrc.enColorFmt = TDE2_COLOR_FMT_ARGB1555;
	stSrc.u32PhyAddr = (unsigned int)src;
	stSrc.u32Width = w;
	stSrc.u32Height = h;
	stSrc.u32Stride = (w << 1);
	stSrc.bAlphaExt1555 = HI_TRUE;
	stSrc.u8Alpha0 = 0xff;//0;//csp modify
	stSrc.u8Alpha1 = 0;//0xff;//csp modify
	
	stSrcRect.s32Xpos = 0;
	stSrcRect.s32Ypos = 0;
	stSrcRect.u32Width = w;
	stSrcRect.u32Height = h;
	
	stDst.enColorFmt = TDE2_COLOR_FMT_ARGB1555;
	stDst.u32PhyAddr = (unsigned int)des;
	stDst.u32Width = w * x;
	stDst.u32Height = h * x;
	stDst.u32Stride = ((w * x) << 1);
	stDst.bAlphaExt1555 = HI_TRUE;
	stDst.u8Alpha0 = 0xff;//0;//csp modify
	stDst.u8Alpha1 = 0;//0xff;//csp modify
	
	stDstRect.s32Xpos = 0;
	stDstRect.s32Ypos = 0;
	stDstRect.u32Width = w * x;
	stDstRect.u32Height = h * x;
	
	s32Ret = HI_TDE2_QuickResize(s32Handle, &stSrc, &stSrcRect, &stDst, &stDstRect);
	if(HI_SUCCESS != s32Ret)
	{
		printf("HI_TDE2_QuickResize command failed s32Ret=0x%08x!\n", s32Ret);
	}
	else
	{
		//printf("HI_TDE2_QuickResize command success\n");
	}
	
	HI_TDE2_EndJob(s32Handle, HI_FALSE, 0, 0);
	HI_TDE2_WaitForDone(s32Handle);
	
	pthread_mutex_unlock(&g_tde2_lock);
#else
	if((NULL==src)||(0==w)||(0==h)||(0==nBpP)||(NULL==des)||(0==x))
	{
		return;
	}
	u32 i = 0, j = 0, k = 0, l = 0, m = 0;
	u8 *tmp = NULL;
	for(i = 0; i < h; i++)
	{
		for(j = 0; j < w; j++)
		{
			tmp = des+i*w*x*nBpP*x+j*nBpP*x;
			for(m=0;m<nBpP;m++)
			{
				*(tmp+m) = *(src+i*w*nBpP+j*nBpP+m);
				for(k=0;k<x;k++)
				{
					*(tmp+k*nBpP+m) = *(tmp+m);
					for(l=1;l<x;l++)
					{
						*(tmp+k*nBpP+m+w*x*nBpP*l) = *(tmp+m);
					}
				}
			}
		}
	}
#endif
}

//应该用TDE缩小
void LessenBitmap(char* src, u32 w, u32 h, u8 nBpP, char* des, u8 x)
{
	if((NULL==src)||(0==w)||(0==h)||(0==nBpP)||(NULL==des)||(0==x))
	{
		return;
	}
	u32 i = 0, j = 0, /*k = 0, l = 0, */m = 0;
	char *tmp = NULL;
	for(i = 0; i < h/x; i++)
	{
		for(j = 0; j < w/x; j++)
		{
			tmp = des+i*(w/x)*nBpP+j*nBpP;
			for(m=0;m<nBpP;m++)
			{
				*(tmp+m) = *(src+(i*x)*w*nBpP+(j*x)*nBpP+m);
			}
		}
	}
}

s32 ModEncodeOsdSet(u8 nChn, EMENCODEOSDTYPE emOsdType, SRect* pRect, EMENCODEFONTSIZE emSize, s8* strOsd)
{
	SOsdObj* psOsdObj = &g_s_EncodeManager.sOsdManage;
	if(0 == psOsdObj->nChnNum)
	{
		printf("encode mod has not been init,line is %d\n",__LINE__);
		return -1;
	}
	
	//csp modify for test
	//if(emOsdType == EM_ENCODE_OSD_TIME)
	//{
	//	return 0;
	//}
	
	//if(emOsdType != EM_ENCODE_OSD_TIME && pRect != NULL) printf("ModEncodeOsdSet Chn=%d,OsdType=%d,osd=%s,rect:(%d,%d,%d,%d)\n",nChn,emOsdType,strOsd,pRect->x,pRect->y,pRect->w,pRect->h);
	
	pthread_mutex_lock(&psOsdObj->pmLock);
	
	u8 nMaxCh = psOsdObj->nChnNum;
	if(nChn >= nMaxCh)
	{
		printf("chn param error\n");
		
		pthread_mutex_unlock(&psOsdObj->pmLock);
		
		return -1;
	}
	
	s32* pFd = GetOsdFd(emOsdType);
	venc_osd_atr_t* pVoa = GetOsdVoa(emOsdType);
	
	if(pFd[nChn] < 0)
	{
		printf("chn%d osd not init, type %d line:%d\n", nChn, emOsdType, __LINE__);
		
		pthread_mutex_unlock(&psOsdObj->pmLock);
		
		return -1;
	}
	
	#if defined(CHIP_HISI3531) || defined(CHIP_HISI3521)
	s32* pFdSub = GetOsdFdSub(emOsdType);
	if(pFdSub[nChn] < 0)
	{
		pthread_mutex_unlock(&psOsdObj->pmLock);
		return -1;
	}
	#endif
	
	//csp modify 20130423
	#if defined(CHIP_HISI3531)// || defined(CHIP_HISI3521)
	s32* pFdThird = NULL;
	if(ModEncodeSupportThirdStream())
	{
		pFdThird = GetOsdFdThird(emOsdType);
		if(pFdThird == NULL || pFdThird[nChn] < 0)
		{
			printf("third stream osd error-1\n");
			pthread_mutex_unlock(&psOsdObj->pmLock);
			return -1;
		}
	}
	#endif
	
	s32 ret = 0;
	if(pRect)
	{
		//#if defined(CHIP_HISI3531)//csp modify 20130429
		//pRect->x = pRect->x / 16 * 16;
		//pRect->y = pRect->y / 16 * 16;
		//pRect->w = (pRect->w + 15) / 16 * 16;
		//pRect->h = (pRect->h + 15) / 16 * 16;
		//#endif
		
		#if defined(CHIP_HISI3521)//csp modify 20130316
		pRect->x = pRect->x / 8 * 8;
		pRect->y = pRect->y / 8 * 8;
		pRect->w = (pRect->w + 7) / 8 * 8;
		pRect->h = (pRect->h + 7) / 8 * 8;
		#endif
		
		s32 rX,rY,rXSub,rYSub;
		GetRealCoord(nChn,pRect->x,pRect->y,&rX,&rY,&rXSub,&rYSub);
		
		#if defined(CHIP_HISI3521)//csp modify 20130316
		s32 rW,rH,rWSub,rHSub;
		s32 max_w = 352;
		s32 max_h = (g_s_EncodeManager.nVideoStandard == 10/*EM_BIZ_NTSC*/)?240:288;
		GetRealCoord(nChn,max_w,max_h,&rW,&rH,&rWSub,&rHSub);
		rX = (rW << 16) | rX;
		rY = (rH << 16) | rY;
		rXSub = (rWSub << 16) | rXSub;
		rYSub = (rHSub << 16) | rYSub;
		#endif
		
		//printf("ModEncodeOsdSet:(%d,%d,%d,%d),(%d,%d),(%d,%d)\n", pRect->x, pRect->y, pRect->w, pRect->h, rX, rY, rXSub, rYSub);//csp modify
		//fflush(stdout);
		
		pVoa[nChn].x = pRect->x;
		pVoa[nChn].y = pRect->y;
		pVoa[nChn].width = pRect->w;
		pVoa[nChn].height = pRect->h;
		
		ret = tl_venc_osd_set_postion(pFd[nChn], rX, rY);
		if(ret<0)
		{
			printf("venc_osd set_postion faild\n");
			//printf("Line: %d ,venc_osd set_bitmap fail,errstr=%s\n",__LINE__,strerror(errno));
			
			pthread_mutex_unlock(&psOsdObj->pmLock);
			
			return ret;
		}
		
		#if defined(CHIP_HISI3531) || defined(CHIP_HISI3521)
		ret = tl_venc_osd_set_postion(pFdSub[nChn], rXSub, rYSub);
		if(ret<0)
		{
			printf("sub venc_osd set_postion faild\n");
			pthread_mutex_unlock(&psOsdObj->pmLock);
			return ret;
		}
		#endif
		
		//csp modify 20130423
		#if defined(CHIP_HISI3531)// || defined(CHIP_HISI3521)
		if(ModEncodeSupportThirdStream())
		{
			s32 rXThird, rYThird;
			GetThirdRealCoord(nChn,pRect->x,pRect->y,&rXThird,&rYThird);
			
			ret = tl_venc_osd_set_postion(pFdThird[nChn], rXThird, rYThird);
			if(ret<0)
			{
				printf("third venc_osd set_postion faild\n");
				pthread_mutex_unlock(&psOsdObj->pmLock);
				return ret;
			}
		}
		#endif
	}
	
	if(strOsd)
	{
		//printf("chn%d strOsd=%s\n",nChn,strOsd);
		//fflush(stdout);
		
		//csp modify 20130316
		if(emOsdType == EM_ENCODE_OSD_CHNNAME)
		{
			strncpy(psOsdObj->ChnName[nChn],strOsd,32);
			psOsdObj->ChnName[nChn][32-1] = '\0';
		}
		
#if 0//#if defined(CHIP_HISI3531)//csp modify 20130429
		u32 alloc_len = pVoa[nChn].width * pVoa[nChn].height * 2;
		if(g_osdbuf1.u32PhyAddr == 0 || g_osdbuf1.u32Len < alloc_len)
		{
			if(g_osdbuf1.u32PhyAddr)
			{
				HI_MPI_SYS_MmzFree((HI_U32)g_osdbuf1.u32PhyAddr, (HI_VOID *)g_osdbuf1.u32VirAddr);
				
				g_osdbuf1.u32PhyAddr = 0;
				g_osdbuf1.u32VirAddr = 0;
				g_osdbuf1.u32Len = 0;
			}
			
			if(HI_FAILURE == HI_MPI_SYS_MmzAlloc((HI_U32 *)&g_osdbuf1.u32PhyAddr, (HI_VOID **)&g_osdbuf1.u32VirAddr, NULL, NULL, alloc_len))
			{
				g_osdbuf1.u32PhyAddr = 0;
				g_osdbuf1.u32VirAddr = 0;
				g_osdbuf1.u32Len = 0;
				
				printf("allocate mmz memory failed-1\n");
				pthread_mutex_unlock(&psOsdObj->pmLock);
				return -1;
			}
			g_osdbuf1.u32Len = alloc_len;
		}
		
		u8 *osdBuf = (u8 *)g_osdbuf1.u32VirAddr;
		memset(osdBuf, 0, alloc_len);
#else
		u8 osdBuf[pVoa[nChn].width * pVoa[nChn].height * 2];
		memset(osdBuf, 0, sizeof(osdBuf));
#endif
		
		if(psOsdObj->pfnOsdCB)
		{
			psOsdObj->pfnOsdCB(osdBuf, 
								pVoa[nChn].width, 
								pVoa[nChn].height, 
								strOsd, 
								strlen(strOsd));
		}
		
#if defined(CHIP_HISI3531)//csp modify 20121201
		u8 nEnlarge = 2;
		
		if(1)//csp modify 20130429
		{
			EMVIDEORESOLUTION emCurReso
				= g_s_EncodeManager.sChnMainManage.psChnObj[nChn].sEncodeParam.sVideoMainParam.emVideoResolution;
			if(emCurReso == EM_VIDEO_1080P)
			{
				nEnlarge = 3;
			}
		}
		
		#if 0//#if 1//csp modify 20130429
		alloc_len = pVoa[nChn].width * pVoa[nChn].height * 2 * nEnlarge * nEnlarge;
		if(g_osdbuf2.u32PhyAddr == 0 || g_osdbuf2.u32Len < alloc_len)
		{
			if(g_osdbuf2.u32PhyAddr)
			{
				HI_MPI_SYS_MmzFree((HI_U32)g_osdbuf2.u32PhyAddr, (HI_VOID *)g_osdbuf2.u32VirAddr);
				
				g_osdbuf2.u32PhyAddr = 0;
				g_osdbuf2.u32VirAddr = 0;
				g_osdbuf2.u32Len = 0;
			}
			
			if(HI_FAILURE == HI_MPI_SYS_MmzAlloc((HI_U32 *)&g_osdbuf2.u32PhyAddr, (HI_VOID **)&g_osdbuf2.u32VirAddr, NULL, NULL, alloc_len))
			{
				g_osdbuf2.u32PhyAddr = 0;
				g_osdbuf2.u32VirAddr = 0;
				g_osdbuf2.u32Len = 0;
				
				printf("allocate mmz memory failed-2\n");
				pthread_mutex_unlock(&psOsdObj->pmLock);
				return -1;
			}
			g_osdbuf2.u32Len = alloc_len;
		}
		
		u8 *osdBuf2 = (u8 *)g_osdbuf2.u32VirAddr;
		memset(osdBuf2, 0, alloc_len);
		
		EnlargeBitmap(g_osdbuf1.u32PhyAddr,pVoa[nChn].width,pVoa[nChn].height,2,g_osdbuf2.u32PhyAddr,nEnlarge);
		#else
		u8 osdBuf2[pVoa[nChn].width * pVoa[nChn].height * 2 * nEnlarge * nEnlarge];
		memset(osdBuf2, 0, sizeof(osdBuf2));
		
		EnlargeBitmap(osdBuf,pVoa[nChn].width,pVoa[nChn].height,2,osdBuf2,nEnlarge);
		#endif
		
		ret = tl_venc_osd_set_bitmap(pFd[nChn], 
										(char *)osdBuf2, 
										pVoa[nChn].width * nEnlarge, 
										pVoa[nChn].height * nEnlarge);
		
		ret |= tl_venc_osd_set_bitmap(pFdSub[nChn], 
										(char *)osdBuf, 
										pVoa[nChn].width, 
										pVoa[nChn].height);
		
		//csp modify 20130423
		#if defined(CHIP_HISI3531)// || defined(CHIP_HISI3521)
		if(ModEncodeSupportThirdStream())
		{
			EMVIDEORESOLUTION emCurReso
				= g_s_EncodeManager.sChnThirdManage.psChnObj[nChn].sEncodeParam.sVideoMainParam.emVideoResolution;
			if(emCurReso == EM_VIDEO_D1)
			{
				if(nEnlarge != 2)
				{
					nEnlarge = 2;
					
					#if 0//#if 1//csp modify 20130429
					alloc_len = pVoa[nChn].width * pVoa[nChn].height * 2 * nEnlarge * nEnlarge;
					memset(osdBuf2, 0, alloc_len);
					EnlargeBitmap(g_osdbuf1.u32PhyAddr,pVoa[nChn].width,pVoa[nChn].height,2,g_osdbuf2.u32PhyAddr,nEnlarge);
					#else
					memset(osdBuf2, 0, sizeof(osdBuf2));
					EnlargeBitmap(osdBuf,pVoa[nChn].width,pVoa[nChn].height,2,osdBuf2,nEnlarge);
					#endif
				}
				
				ret |= tl_venc_osd_set_bitmap(pFdThird[nChn], (char *)osdBuf2, pVoa[nChn].width * nEnlarge, pVoa[nChn].height * nEnlarge);
			}
			else
			{
				ret |= tl_venc_osd_set_bitmap(pFdThird[nChn], (char *)osdBuf, pVoa[nChn].width, pVoa[nChn].height);
			}
		}
		#endif
#elif defined(CHIP_HISI3521)//csp modify 20130316
		u8 nEnlarge = 2;
		
		s32 rW,rH,rWSub,rHSub;
		s32 max_w = 352;
		s32 max_h = (g_s_EncodeManager.nVideoStandard == 10/*EM_BIZ_NTSC*/)?240:288;
		GetRealCoord(nChn,max_w,max_h,&rW,&rH,&rWSub,&rHSub);
		if(rW <= 352)
		{
			nEnlarge = 1;
		}
		
		int main_osd_w = pVoa[nChn].width * nEnlarge;
		int main_osd_h = pVoa[nChn].height * nEnlarge;
		
		u8 osdBuf2[pVoa[nChn].width * pVoa[nChn].height * 2 * nEnlarge * nEnlarge];
		if(nEnlarge > 1)
		{
			EnlargeBitmap(osdBuf,pVoa[nChn].width,pVoa[nChn].height,2,osdBuf2,nEnlarge);
			ret = tl_venc_osd_set_bitmap(pFd[nChn], 
										osdBuf2, 
										main_osd_w, 
										main_osd_h);
		}
		else
		{
			ret = tl_venc_osd_set_bitmap(pFd[nChn], 
										osdBuf, 
										main_osd_w, 
										main_osd_h);
		}
		
		u8 nLessen = 1;
		if(rWSub <= 176)
		{
			//nLessen = 2;
		}
		
		int sub_osd_w = pVoa[nChn].width / nLessen;
		int sub_osd_h = pVoa[nChn].height / nLessen;
		
		if(nLessen > 1)
		{
			LessenBitmap(osdBuf,pVoa[nChn].width,pVoa[nChn].height,2,osdBuf2,nLessen);
			
			sub_osd_h = (sub_osd_h < 16) ? 16 : sub_osd_h;
			
			ret |= tl_venc_osd_set_bitmap(pFdSub[nChn], 
										osdBuf2, 
										sub_osd_w, 
										sub_osd_h);
		}
		else
		{
			ret |= tl_venc_osd_set_bitmap(pFdSub[nChn], 
										osdBuf, 
										sub_osd_w, 
										sub_osd_h);
		}
		
		if(nChn == 0)
		{
			//printf("hehe1-(%d,%d,%d,%d)\n",pVoa[nChn].x,pVoa[nChn].y,pVoa[nChn].width,pVoa[nChn].height);
			//printf("hehe2-(%d,%d,%d,%d)\n",main_osd_w,main_osd_h,sub_osd_w,sub_osd_h);
		}
#else
		ret = tl_venc_osd_set_bitmap(pFd[nChn], 
										osdBuf, 
										pVoa[nChn].width, 
										pVoa[nChn].height);
#endif
		if(ret<0)
		{
			printf("venc_osd set_bitmap fail\n");
			
			pthread_mutex_unlock(&psOsdObj->pmLock);
			
			return ret;
		}
	}
	
	pthread_mutex_unlock(&psOsdObj->pmLock);
	
	//int i = 0;
	//for(i=0;i<g_s_EncodeManager.sChnSubManage.nChnNum;i++)
	//{
	//	printf("ModEncodeOsdSet chn%d gop:%d\n",i,g_s_EncodeManager.sChnSubManage.psChnObj[i].sEncodeParam.sVideoSubParam.nGop);
	//}
	
	return 0;
}

s32 ModEncodeOsdShow(u8 nChn, EMENCODEOSDTYPE emOsdType, u8 nShow)
{	
	SOsdObj* psOsdObj = &g_s_EncodeManager.sOsdManage;
	if(0 == psOsdObj->nChnNum)
	{
		printf("encode mod has not been init,line is %d\n",__LINE__);
		return -1;
	}
	
	//csp modify for test
	//if(emOsdType == EM_ENCODE_OSD_TIME)
	//{
	//	return 0;
	//}
	
	pthread_mutex_lock(&psOsdObj->pmLock);
	
	u8 nMaxCh = psOsdObj->nChnNum;
	if(nChn>=nMaxCh)
	{
		printf("chn param error \n");
		pthread_mutex_unlock(&psOsdObj->pmLock);
		return -1;
	}
	
	s32* pFd = GetOsdFd(emOsdType);
	if(pFd[nChn] < 0)
	{
		printf("osd not init, line:%d \n",__LINE__);
		
		pthread_mutex_unlock(&psOsdObj->pmLock);
		
		return -1;
	}
	
	s32 ret = tl_venc_osd_show(pFd[nChn], nShow);
	if(ret<0)
	{
		printf("venc_osd_show error\n");
		pthread_mutex_unlock(&psOsdObj->pmLock);
		return ret;
	}
	
	#if defined(CHIP_HISI3531) || defined(CHIP_HISI3521)
	pFd = GetOsdFdSub(emOsdType);
	if(pFd[nChn] < 0)
	{
		printf("osd not init, line:%d\n",__LINE__);
		pthread_mutex_unlock(&psOsdObj->pmLock);		
		return -1;
	}
	ret = tl_venc_osd_show(pFd[nChn], nShow);
	if(ret < 0)
	{
		printf("sub venc_osd_show error\n");
		pthread_mutex_unlock(&psOsdObj->pmLock);
		return ret;
	}
	#endif
	
	//csp modify 20130423
	#if defined(CHIP_HISI3531)// || defined(CHIP_HISI3521)
	if(ModEncodeSupportThirdStream())
	{
		pFd = GetOsdFdThird(emOsdType);
		if(pFd == NULL || pFd[nChn] < 0)
		{
			printf("osd not init, line:%d\n",__LINE__);
			pthread_mutex_unlock(&psOsdObj->pmLock);		
			return -1;
		}
		ret = tl_venc_osd_show(pFd[nChn], nShow);
		if(ret < 0)
		{
			printf("third venc_osd_show error\n");
			pthread_mutex_unlock(&psOsdObj->pmLock);
			return ret;
		}
	}
	#endif
	
	pthread_mutex_unlock(&psOsdObj->pmLock);
	
	return 0;
}

//csp modify 20130423
s32 ModEncodeSupportThirdStream()
{
	return g_sThirdStreamProperty.support;
}

//csp modify 20130423
s32 ModEncodeSetThirdStreamProperty(SThirdStreamProperty *psProperty)
{
	if(psProperty == NULL)
	{
		return -1;
	}
	g_sThirdStreamProperty = *psProperty;
	return 0;
}
s32 ModEncodeGetThirdStreamProperty(SThirdStreamProperty *psProperty)
{
	if(psProperty == NULL)
	{
		return -1;
	}
	*psProperty = g_sThirdStreamProperty;
	return 0;
}

//yaogang modify 20150212
s32 ModEncSnapCreateChn(u8 EncChn, int Width, int Height)
{
	return tl_enc_jpeg_create_chn(EncChn, Width, Height);
}
s32 ModEncSnapDestroyChn(u8 EncChn)
{
	return tl_enc_jpeg_destroy_chn(EncChn);
}

/*
s32 ModEncGetSnapshot(u8 StreamChn, u8 EncChn, u8 *pdata, u32 *psize)
{
	return tl_enc_snap(StreamChn, EncChn, pdata, psize);
}
*/
s32 ModEncGetSnapshot(u8 EncChn, u16 *pWidth, u16 *pHeight, u8 *pdata, u32 *psize)
{
	return tl_enc_snap(EncChn, pWidth, pHeight, pdata, psize);
}

