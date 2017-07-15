#include "biz_manager.h"
#include "biz_config.h"
#include "biz_syscomplex.h"
#include "biz_preview.h"
#include "biz_encode.h"
#include "biz_record.h"
#include "biz_playback.h"
#include "biz_ptz.h"
#include "biz_alarm.h"
#include "biz_snap.h"



#include <time.h>

#define TEST_INIT

#ifdef TEST_INIT
#include <sys/mman.h>
#include <fcntl.h>
#include <assert.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <errno.h>

#if defined(CHIP_HISI3531) || defined(CHIP_HISI3521)
#else
#include "mkp_vd.h"
#endif

#include "hifb.h"

#include "lib_common.h"
#include "lib_misc.h"
#include "lib_venc.h"
#include "lib_video.h"

#include "biz_Net.h"

#include <sys/syscall.h>

int initStateOSD(void);
#endif

#include <sys/time.h>
#include "mod_record.h"

#include <unistd.h>
#include <sys/reboot.h>

//yaogang modify 20160330
//NVR 不带深广平台版本
//#define NO_MODULE_SG
#undef NO_MODULE_SG




//csp modify 20121225
static int* g_nFlagRecVideoAudio = NULL;

void biz_NetGetRandomMAC( void );
s32 TalkControl(u8 nStatus);
s32 VEncSubControl(u8 nChn, u8 nStatus, u8 nTarget); //0 for rec; 1 for net
s32 VEncThirdControl(u8 nChn, u8 nStatus, u8 nTarget); //0 for mobile; 1 for net //csp modify 20140525
s32 DoBizNetReq(STcpStreamReq* param);
s32 DoBizSetPara(SBizPara* psBizPara);
s32 DoBizGetPara(u32 bDefault,SBizPara* psBizPara);
s32 DoBizRecStatusChange(EMCHNTYPE emChntype, u16 nChnStatus);
s32 VEncMainControl(u8 nChn, u8 nStatus, u8 nTarget);
s32 AEncControl(u8 nChn, u8 nStatus, u8 nTarget );
s32 EncodeControl(u8 nChn, EMCHNTYPE emChnType, u8 nLastEncStatus, u8 nCurEncStatus);
s32 EncDataDispatch(SBizEncodeData* psBizEncData);
s32 AlarmDispatchEvent(SAlarmCbData* psAlarmCbData);
u8 GetRecEnable(u8 nId);
u8 GetRecStreamType(u8 nId);

//#define TEST_TIMESPAN

s32 IsPAL(void)
{
	static s32 isPAL = -1;
	if(isPAL == -1)
	{
		if(tl_video_get_format())
		{
			isPAL = 1;
		}
		else
		{
			isPAL = 0;
		}
	}
	
	return isPAL;
}

#define ALARM_INIT_CW//cw_test
#ifdef ALARM_INIT_CW
void AlarmParaInit(void*pManage)
{
	pthread_detach(pthread_self());//csp modify
	
	
	
    int i = 0;
    SBizManager* psBizManager = (SBizManager* ) pManage;
    SBizAlarm sBizAlarm;
    sBizAlarm.nAlarmOutNum = psBizManager->nAlarmOutNum;
    sBizAlarm.nAlarmSensorNum = psBizManager->nSensorNum;
    sBizAlarm.nBuzzNum = psBizManager->nBuzzNum;
    sBizAlarm.nVBlindLuma = 0;
    sBizAlarm.nVideoChnNum = psBizManager->nVideoMainNum;
	sBizAlarm.nNVROrDecoder = psBizManager->nNVROrDecoder;

		sBizAlarm.nDiskNum = MAX_HDD_NUM;

	printf("$$$$$$$$$$$$$$$$$$AlarmParaInit id:%d, nNVROrDecoder: %u\n", getpid(), psBizManager->nNVROrDecoder);
	
    if(0 != AlarmInit(psBizManager->sBizPara.nModMask & MASK_BIZ_MOD_ALARM, &sBizAlarm))
    {
        return;
    }
	
	usleep(100 * 1000);
	
	SBizAlarmSensorPara sBizSensorPara;
	SBizAlarmSchedule sBizSch;
	SBizAlarmDispatch sBizDispatch;
	for(i = 0; i < sBizAlarm.nAlarmSensorNum; i++)
	{
		//PUBPRT("Here");
		ConfigGetAlarmSensorPara(0,&sBizSensorPara, i);
		AlarmSetPara(i, EM_BIZ_SENSORPARA, &sBizSensorPara);
		//PUBPRT("Here");
		ConfigGetAlarmSensorSchedule(0,&sBizSch, i);
		AlarmSetPara(i, EM_BIZ_SCHEDULE_SENSOR, &sBizSch);
		//PUBPRT("Here");
		ConfigGetAlarmSensorDispatch(0,&sBizDispatch, i);
		AlarmSetPara(i, EM_BIZ_DISPATCH_SENSOR, &sBizDispatch);
		//PUBPRT("Here");
	}
	
	SBizAlarmOutPara sBizAlarmOutPara;
	for(i = 0; i < sBizAlarm.nAlarmOutNum; i++)
	{
		ConfigGetAlarmOutPara(0,&sBizAlarmOutPara, i);
		AlarmSetPara(i, EM_BIZ_ALARMOUTPARA, &sBizAlarmOutPara);
		
		ConfigGetAlarmOutSchedule(0,&sBizSch, i);
		AlarmSetPara(i, EM_BIZ_SCHEDULE_ALARMOUT, &sBizSch);
	}
	
	SBizAlarmBuzzPara sBizBuzzPara;
	for(i = 0; i < sBizAlarm.nBuzzNum; i++)
	{
		ConfigGetAlarmBuzzPara(0,&sBizBuzzPara, i);
		AlarmSetPara(i, EM_BIZ_ALARMBUZZPARA, &sBizBuzzPara);
		
		ConfigGetAlarmBuzzSchedule(0,&sBizSch, i);
		AlarmSetPara(i, EM_BIZ_SCHEDULE_BUZZ, &sBizSch);
	}
	
	SBizAlarmVMotionPara sBizVMPara;
	SBizAlarmVBlindPara sBizVBPara;
	SBizAlarmVLostPara sBizVLPara;
	SBizAlarmIPCExtSensorPara sBizIPCExtSensorPara;
	SBizAlarmHDDPara sBizHDDPara;

	//yaogang modify 20141118
	//hdd
	ConfigGetAlarmHDDPara(0,&sBizHDDPara, 0);
	ConfigGetAlarmHDDDispatch(0,&sBizDispatch, 0);
	//printf("yg AlarmParaInit nFlagBuzz: %d\n", sBizDispatch.nFlagBuzz);
	//一个保存的参数供检测硬盘丢失的8个通道,
	//检测硬盘读写错误的8个通道,还有检测开机无硬盘的单通道
	//这三方共用
	for(i = 0; i < MAX_HDD_NUM; i++)
	{
		AlarmSetPara(i, EM_BIZ_HDDPARA, &sBizHDDPara);
		AlarmSetPara(i, EM_BIZ_DISPATCH_HDD, &sBizDispatch);
	}
	
	for(i = 0; i < sBizAlarm.nVideoChnNum; i++)
	{
		//yaogang modify 20141010
		//IPCCOVER
		ConfigGetAlarmIPCCoverPara(0,&sBizIPCExtSensorPara, i);
		AlarmSetPara(i, EM_BIZ_IPCCOVERPARA, &sBizIPCExtSensorPara);
		//ConfigGetAlarmIPCExtSensorSchedule(0,&sBizSch, i);
		//AlarmSetPara(i, EM_BIZ_SCHEDULE_IPCExtSENSOR, &sBizSch);
		ConfigGetAlarmIPCExtSensorDispatch(0,&sBizDispatch, i);
		AlarmSetPara(i, EM_BIZ_DISPATCH_IPCCOVER, &sBizDispatch);
		
		//IPCExtSensor
		ConfigGetAlarmIPCExtSensorPara(0,&sBizIPCExtSensorPara, i);
		AlarmSetPara(i, EM_BIZ_IPCEXTSENSORPARA, &sBizIPCExtSensorPara);
		ConfigGetAlarmIPCExtSensorSchedule(0,&sBizSch, i);
		AlarmSetPara(i, EM_BIZ_SCHEDULE_IPCEXTSENSOR, &sBizSch);
		ConfigGetAlarmIPCExtSensorDispatch(0,&sBizDispatch, i);
		AlarmSetPara(i, EM_BIZ_DISPATCH_IPCEXTSENSOR, &sBizDispatch);
		
		//485ExtSensor
		ConfigGetAlarm485ExtSensorPara(0,&sBizIPCExtSensorPara, i);
		AlarmSetPara(i, EM_BIZ_485EXTSENSORPARA, &sBizIPCExtSensorPara);
		//ConfigGetAlarmIPCExtSensorSchedule(0,&sBizSch, i);
		//AlarmSetPara(i, EM_BIZ_SCHEDULE_IPCExtSENSOR, &sBizSch);
		ConfigGetAlarm485ExtSensorDispatch(0,&sBizDispatch, i);
		AlarmSetPara(i, EM_BIZ_DISPATCH_485EXTSENSOR, &sBizDispatch);
		
		
		ConfigGetAlarmVMotionPara(0,&sBizVMPara, i);
		AlarmSetPara(i, EM_BIZ_VMOTIONPARA, &sBizVMPara);
		ConfigGetAlarmVMotionSchedule(0,&sBizSch, i);
		AlarmSetPara(i, EM_BIZ_SCHEDULE_VMOTION, &sBizSch);
		ConfigGetAlarmVMotionDispatch(0,&sBizDispatch, i);
		AlarmSetPara(i, EM_BIZ_DISPATCH_VMOTION, &sBizDispatch);
		
		ConfigGetAlarmVBlindPara(0,&sBizVBPara, i);
		AlarmSetPara(i, EM_BIZ_VBLINDPARA, &sBizVBPara);
/*		ConfigGetAlarmVBlindSchedule(0,&sBizSch, i);
		AlarmSetPara(i, EM_BIZ_SCHEDULE_VBLIND, &sBizSch);
		ConfigGetAlarmVBlindDispatch(0,&sBizDispatch, i);
		AlarmSetPara(i, EM_BIZ_DISPATCH_VBLIND, &sBizDispatch);
*/
		
		ConfigGetAlarmVLostPara(0,&sBizVLPara, i);
		AlarmSetPara(i, EM_BIZ_VLOSTPARA, &sBizVLPara);
		ConfigGetAlarmVLostSchedule(0,&sBizSch, i);
		AlarmSetPara(i, EM_BIZ_SCHEDULE_VLOST, &sBizSch);
		ConfigGetAlarmVLostDispatch(0,&sBizDispatch, i);
		AlarmSetPara(i, EM_BIZ_DISPATCH_VLOST, &sBizDispatch);
	}
}
#endif

static CHECKNOAUTH g_NoauthCB = NULL;//cw_auth

void SetNoAuthFuncCB(CHECKNOAUTH func)
{
	g_NoauthCB = func;
}

u8 BizCheckAllHDDFormated()
{
	disk_manager* hddHdr = PublicGetHddManage();
	u8 i = 0;
	for(i = 0; i < MAX_HDD_NUM; i++)
	{
		if((hddHdr->hinfo[i].is_disk_exist) 
			&& (hddHdr->hinfo[i].storage_type != 'u')
			&& (hddHdr->hinfo[i].disk_total == 0))
		break;
	}
	return (i<MAX_HDD_NUM)?0:1;
}

void BizCheckAllHDD(u8 *bDiskIsExist, u8 *bDiskIsErr, u8 *bDiskIsUnFormat)
{
	disk_manager* hddHdr = PublicGetHddManage();
	u8 i = 0;
	for(i = 0; i < MAX_HDD_NUM; i++)
	{
		if(hddHdr->hinfo[i].is_disk_exist)
		{
			(*bDiskIsExist)++;

			if (hddHdr->hinfo[i].is_bad_disk)
			{
				*bDiskIsErr = 1;
			}

			if (hddHdr->hinfo[i].disk_total == 0)
			{
				*bDiskIsUnFormat = 1;
			}
		}			
	}
}


#if 0
void* memmalloc(unsigned int size)//申请使用A片内存
{
	static unsigned int vb_blk = 0;
	static unsigned int vb_pool = 0;
	static unsigned int usedbuflen = 0;
	static void *VirAddr = NULL;
	unsigned int tempbuflen = 0;
	
	if(NULL == VirAddr)
	{

		VirAddr = tl_mmz_memmap(10 * 1024 *1024, &vb_blk, &vb_pool);
	
		if(NULL == VirAddr)
		{
			printf("memmalloc failed , tl_mmz_memmap failed !!\n");
			return NULL;
		}
	}

	tempbuflen = usedbuflen;
	
	if(NULL != VirAddr)
	{		
		printf("@@@@@@@@@@@@@@@@@@@@tempbuflen  %d,(tempbuflen + size) %d\n",tempbuflen,tempbuflen + size);
		if((tempbuflen + size) < 10 * 1024 *1024)
		{
			usedbuflen += size;
		}
		else
		{
			printf("memmalloc failed , no enough space to be used !!\n");
			return NULL;
		}

		return VirAddr + tempbuflen;
	}
}
#endif
static int unloadHdd(disk_manager *hddHdr, int index)
{
	if(hddHdr == NULL || index >= MAX_HDD_NUM || index < 0)
	{
		return -1;
	}
	
	HddInfo* psHddInfo = &hddHdr->hinfo[index];
	
	if(psHddInfo->is_disk_exist)
	{
		int i = 0;
		for(i = 0; i < MAX_PARTITION_NUM; i++)
		{
			if(psHddInfo->is_partition_exist[i] == 1)
			{
				destroy_partition_index(&psHddInfo->ptn_index[i]);
				
				char cmd[64] = {0};
				sprintf(cmd, "rec/%c%d", 'a'+index, i+1);
				printf("umount_user partition path = %s\n",cmd);
				
				int count = 0;
				
				while(1)
				{
					int ret = umount_user(cmd);
					if(ret)
					{
						count++;
						
						//unsigned char cur_atx_flag = tl_power_atx_check();
						//printf("####################################unloadHdd:dir<%s> is busy,atx:%d,count:%d!!!####################################\n",cmd,cur_atx_flag,count);
						//return -1;
						
						if(count == 16)//if(count == 60)
						{
							//haltwd();
							//system("fdisk -l");
							//system("mount");
							printf("####################################unloadHdd:umount_user failed,exit app...\n");
							//exit(1);
							reboot(RB_AUTOBOOT);
							
							count = 0;
						}
						
						sleep(5);
					}
					else
					{
						break;
					}
				}
			}
		}
	}
	
	//psHddInfo->is_disk_exist = 0;
	//memset(psHddInfo->is_partition_exist, 0, sizeof(psHddInfo->is_partition_exist));
	//psHddInfo->total = 0;
	//psHddInfo->free = 0;
	
	memset(psHddInfo, 0, sizeof(HddInfo));
	
	return 0;
}

static int reloadHdd(disk_manager *hddHdr, SBizDevInfo *pNewDisk)
{
	if(hddHdr == NULL || pNewDisk == NULL)
	{
		return -1;
	}
	
	HddInfo* psHddInfo = NULL;
	int index = -1;
	char cmd[64] = {0};
	
	int i = 0;
	for(i = 0; i < MAX_HDD_NUM; i++)
	{
		if(!hddHdr->hinfo[i].is_disk_exist)
		{
			psHddInfo = &hddHdr->hinfo[i];
			index = i;
			break;
		}
	}
	
	if(psHddInfo == NULL)
	{
		printf("Warning: ""%s psHddInfo == NULL \n", __func__);
		return -1;
	}

	memset(psHddInfo, 0, sizeof(HddInfo));//yaogang modify 20170218 in shanghai
	psHddInfo->disk_physical_idx = pNewDisk->disk_physical_idx;
	psHddInfo->disk_logic_idx = pNewDisk->disk_logic_idx;
	psHddInfo->disk_system_idx = pNewDisk->disk_system_idx;
	strcpy(psHddInfo->disk_name, pNewDisk->strDevPath);
	strcpy(psHddInfo->disk_sn, pNewDisk->disk_sn);
	psHddInfo->storage_type = 's';
	psHddInfo->is_disk_exist = 1;
	
	//psHddInfo->disk_total = 0;//yaogang modify 20170218 in shanghai
	//psHddInfo->disk_free = 0;
	
	for(i = 0; i < MAX_PARTITION_NUM; i++)
	{
		char disk_name[64] = {0};
		sprintf(disk_name, "%s%d", pNewDisk->strDevPath, i+1);
		
		int fd = open(disk_name, O_RDONLY);
		if(fd < 0)
		{
			printf("%s open %s failed, %s\n", __func__, disk_name, strerror(errno));
			
			//psHddInfo->is_partition_exist[i] = 0; //yaogang modify 20170218 in shanghai
			//yaogang modify for bad disk
			//continue;
			break;
		}
		
		close(fd);
		
		sprintf(cmd,"rec/%c%d",'a'+index,i+1);
		if (mkdir(cmd,1))//yaogang modify 20170218 in shanghai
		{
			if (EEXIST == errno)
			{
				printf("%s mount path: %s exist, umount first\n",
					__func__, cmd);
				
				umount_user(cmd);
			}
			else
			{
				printf("%s mkdir %s failed, errno: %d, %s\n", __func__, cmd, errno, strerror(errno));
			
				//yaogang modify for bad disk
				//continue;
				break;
			}
		}	
		
		//psHddInfo->is_partition_exist[i] = 1;//yaogang wait for modify 20170218 in shanghai  可能需要下移
		
		if (0 != modsys_MountUser(disk_name,cmd))//yaogang modify 20170218 in shanghai
		{
			printf("%s mount dev: %s to path: %s failed\n",
				__func__, disk_name, cmd);
			
			//yaogang modify for bad disk
			//continue;
			break;
		}
		
		int ret = init_partition_index(&psHddInfo->ptn_index[i], cmd);
		if(ret == 1)//success
		{			
			psHddInfo->disk_total += (u32)(get_partition_total_space(&psHddInfo->ptn_index[i])/1000);
			psHddInfo->disk_free += (u32)(get_partition_free_space(&psHddInfo->ptn_index[i])/1000);

			psHddInfo->is_partition_exist[i] = 1;

			printf("%s init_partition_index %s success\n", __func__, disk_name);

			//printf("disktest--%s i: %d, total: %uKb, free: %uKb\n", 
			//	__func__, i, psHddInfo->total, psHddInfo->free);
		}
		else
		{
			//yaogang modify for bad disk
			//umount_user(cmd);
			printf("%s init_partition_index %s failed\n", __func__, disk_name);
			break;
		}

	}

	//yaogang modify for bad disk
	if (i < MAX_PARTITION_NUM)//期间出错
	{					
		for(i = 0; i < MAX_PARTITION_NUM; i++)
		{	
			if (psHddInfo->is_partition_exist[i])
			{
				destroy_partition_index(&psHddInfo->ptn_index[i]);
				
				sprintf(cmd, "rec/%c%d",'a'+index,i+1);
				umount_user(cmd);

				psHddInfo->is_partition_exist[i] = 0;
			}
		}

		psHddInfo->disk_total = 0;
		psHddInfo->disk_free = 0;
	}
	//yaogang modify 20170218 in shanghai
	/*
	psHddInfo->disk_physical_idx = pNewDisk->disk_physical_idx;
	psHddInfo->disk_logic_idx = pNewDisk->disk_logic_idx;
	psHddInfo->disk_system_idx = pNewDisk->disk_system_idx;
	strcpy(psHddInfo->disk_name, pNewDisk->strDevPath);
	strcpy(psHddInfo->disk_sn, pNewDisk->disk_sn);
	psHddInfo->storage_type = 's';
	psHddInfo->is_disk_exist = 1;
	*/
	
	return 0;
}


int BizReloadHdd(SBizStoreDevManage *psStorge)
{
	printf("########################BizReloadHdd: start########################\n");
	
	if(psStorge == NULL)
	{
		return -1;
	}
	
	disk_manager *hddHdr = (disk_manager *)PublicGetHddManage();
	if(hddHdr == NULL)
	{
		return -1;
	}
	
	SBizManager* psBizManager = &g_sBizManager;
	if(psBizManager == NULL)
	{
		return -1;
	}
	
	int i = 0;
	int j = 0;
	
	//通知biz_snap_yaogang
	BizSnapPause();
	ModRecordSetHotPlugFlag(1);
	set_disk_hotplug_flag(1);//csp modify 20140822
	ModLogSetHotPlugFlag(1);
	
	printf("########################BizReloadHdd: pause record########################\n");
	
	//if(signal(SIGBUS,error_handle1) == SIG_ERR)
	//{
	//	
	//}
	//if(signal(SIGSEGV,error_handle1) == SIG_ERR)
	//{
	//	
	//}
	
	//CreateThread(NULL, 0, _DealyThread_S, this, 0, 0);	
	/*
	unsigned char cur_atx_flag = tl_power_atx_check();
	if(cur_atx_flag == 0)//电池供电
	{
		//printf("30s\n");
		//sleep(30);
	}
	*/
	for(i = 0; i < psBizManager->nVideoMainNum; i++)
	{
		BizRecordPause(i);
	}
	
	printf("########################BizReloadHdd: delay########################\n");
	
	//if(cur_atx_flag == 0/* || psStorge->nDiskNum == 0*/)
	//{
	//	printf("70s\n");
	//	usleep(70*1000*1000);
	//}
	//else
	{
		//usleep(5*1000*1000);
		usleep(10*1000*1000);
	}
	//sync();
	
	//for(i = 0; i < psBizManager->nVideoMainNum; i++)
	//{
	//	BizRecordPause(i);
	//}
	//sleep(5);
	
	printf("########################BizReloadHdd: unload########################\n");
	//psStorge 保存当前所有硬盘的信息
	//hddHdr 保存之前所有硬盘的信息
	//卸载之前存在，现在不存在的硬盘
	for(i = 0; i < MAX_HDD_NUM; i++)
	{
		if(hddHdr->hinfo[i].is_disk_exist)
		{
			BOOL bFind = FALSE;
			
			for(j = 0; j < psStorge->nDiskNum; j++)
			{
				if(psStorge->psDevList[j].nType == 0 && 
					hddHdr->hinfo[i].disk_system_idx == psStorge->psDevList[j].disk_system_idx && 
					hddHdr->hinfo[i].disk_logic_idx == psStorge->psDevList[j].disk_logic_idx && 
					strcmp(hddHdr->hinfo[i].disk_sn, psStorge->psDevList[j].disk_sn) == 0 && 
					strcmp(hddHdr->hinfo[i].disk_name, psStorge->psDevList[j].strDevPath) == 0)
				{
					bFind = TRUE;
					break;
				}
			}
			
			if(!bFind)
			{
				unloadHdd(hddHdr, i);
			}
		}
	}
	
	printf("########################BizReloadHdd: reload########################\n");
	
	for(j = 0; j < psStorge->nDiskNum; j++)
	{
		BOOL bFind = FALSE;
		
		for(i = 0; i < MAX_HDD_NUM; i++)
		{
			if(hddHdr->hinfo[i].is_disk_exist)
			{
				if(psStorge->psDevList[j].nType == 0 && 
					hddHdr->hinfo[i].disk_system_idx == psStorge->psDevList[j].disk_system_idx && 
					hddHdr->hinfo[i].disk_logic_idx == psStorge->psDevList[j].disk_logic_idx && 
					strcmp(hddHdr->hinfo[i].disk_sn, psStorge->psDevList[j].disk_sn) == 0 && 
					strcmp(hddHdr->hinfo[i].disk_name, psStorge->psDevList[j].strDevPath) == 0)
				{
					bFind = TRUE;
					break;
				}
			}
		}
		
		if(!bFind && psStorge->psDevList[j].nType == 0)
		{
			reloadHdd(hddHdr, &psStorge->psDevList[j]);
		}
	}
	
	printf("########################BizReloadHdd: resume########################\n");
	
	//yg modify 20140729
	if(psStorge->nDiskNum != 0)
	{
		for(i = 0; i < psBizManager->nVideoMainNum; i++)
		{
			BizRecordResume(i);
		}
		
		ModRecordSetHotPlugFlag(0);
		set_disk_hotplug_flag(0);//csp modify 20140822
		ModLogSetHotPlugFlag(0);
	}
	//通知biz_snap_yaogang
	BizSnapResume();
	
	//signal(SIGBUS,SIG_IGN);//忽略SIGBUS信号
	//signal(SIGSEGV,SIG_IGN);//忽略SIGSEGV信号
	
	//if(signal(SIGBUS,error_handle2) == SIG_ERR)
	//{
	//	
	//}
	//if(signal(SIGSEGV,error_handle2) == SIG_ERR)
	//{
	//	
	//}
	
	printf("########################BizReloadHdd: finish########################\n");
	
	return 0;
}


s32 DoBizInit(SBizManager* psBizManager, SBizInitPara* psBizPara)
{
	//printf(">>>>>enter function [%s]\n", __FUNCTION__);
	
	s32 ret, i;
	
	static u8 flag_First = 1;
	static SBizSystemPara sysPara;
 	static SBizDvrInfo sDvrInfo;
	static SBizConfig sBizConfig;
	
	#ifdef TEST_TIMESPAN
	u32 nCurTime, nLastTime = PublicGetTimeStamp();
	#endif
	
	/////////////////////////第二次就不调用了///////////////////////////////////
	if(flag_First)
	{
	    psBizManager->sBizPara = *psBizPara;
	    psBizManager->psMsgCmdHdr = CreateMsgCmd(sizeof(SBizMsgHeader));
	    if(NULL == psBizManager->psMsgCmdHdr)
	    {
	        return BIZ_ERR_INIT_LACKOFMEM;
	    }
	    
	    psBizManager->psMsgCmdInner = CreateMsgCmd(sizeof(SBizMsgHeader));
	    if(NULL == psBizManager->psMsgCmdInner)
	    {
	        return BIZ_ERR_INIT_LACKOFMEM;
	    }
		
	    //初始化配置
	    memset(&sBizConfig, 0, sizeof(sBizConfig));
		
	    if(0 != ConfigInit(psBizPara->nModMask & MASK_BIZ_MOD_CONFIG, &sBizConfig))
	    {
			printf("ConfigInit failed!!!!\n");
	        return BIZ_ERR_INIT_CONFIG_INIT;
	    }
		else
		{
			printf("ConfigInit ok!!!!\n");
	    }
		
		memset(&sDvrInfo, 0, sizeof(sDvrInfo));
		ConfigGetDvrInfo(0,&sDvrInfo);
		printf("sDvrInfo.nAlarmoutNum=%d,nAudNum=%d,nBuzzNum=%d nSensorNum=%d nSnapNum=%d nVidMainNum=%d nVidSubNum=%d nRecNum=%d,sproductnumber=%s\n"
			, sDvrInfo.nAlarmoutNum, sDvrInfo.nAudNum, sDvrInfo.nBuzzNum
			, sDvrInfo.nSensorNum, sDvrInfo.nSnapNum, sDvrInfo.nVidMainNum, sDvrInfo.nVidSubNum, sDvrInfo.nRecNum,sDvrInfo.sproductnumber);

		psBizManager->nNVROrDecoder = sDvrInfo.nNVROrDecoder;
	  	int init_param = 0;
		
		//csp modify
		if((0 == strcmp("R9624T",sDvrInfo.sproductnumber))
			|| (0 == strcmp("R9632S",sDvrInfo.sproductnumber))
			 || (0 == strcmp("R9624SL",sDvrInfo.sproductnumber))
			 || (0 == strcmp("R9516S",sDvrInfo.sproductnumber))
			  || (0 == strcmp("R9616S",sDvrInfo.sproductnumber))
			   || (0 == strcmp("R9608S",sDvrInfo.sproductnumber))
			    || (0 == strcmp("R3104HD",sDvrInfo.sproductnumber))
			    || (0 == strcmp("R3110HDW",sDvrInfo.sproductnumber))
			    || (0 == strcmp("R3106HDW",sDvrInfo.sproductnumber))
			     || (0 == strcmp("R3116",sDvrInfo.sproductnumber))
			     || (0 == strcmp("R3116W",sDvrInfo.sproductnumber)))
		{
		    init_param |= TL_VGA;
		}
		//csp modify
		else if((0 == strcmp("R2104",sDvrInfo.sproductnumber))
					|| (0 == strcmp("R2104W",sDvrInfo.sproductnumber))
					|| (0 == strcmp("R2108",sDvrInfo.sproductnumber))
					|| (0 == strcmp("R2108W",sDvrInfo.sproductnumber))
					|| (0 == strcmp("R2116",sDvrInfo.sproductnumber))
					|| (0 == strcmp("R2016",sDvrInfo.sproductnumber)))
		{
			//
		}
		else if((0 == strcmp("NR2116",sDvrInfo.sproductnumber)))
		{
			//
		}
		else if((0 == strcmp("NR3116",sDvrInfo.sproductnumber)))
		{
			//
		}
		else if((0 == strcmp("NR3132",sDvrInfo.sproductnumber)))
		{
			//
		}
		else if((0 == strcmp("NR3124",sDvrInfo.sproductnumber)))
		{
			//
		}
		else if((0 == strcmp("NR1004",sDvrInfo.sproductnumber)))
		{
			//
		}
		else if((0 == strcmp("NR1008",sDvrInfo.sproductnumber)))
		{
			//
		}
		else
		{
			init_param |= TL_128MDDR;
		    init_param |= TL_VGA;
		}
		
		//printf("%s 1 init_param = 0x%08x\n",__func__, init_param);
		
		if((0 == strcmp("R3104HD",sDvrInfo.sproductnumber))
			|| (0 == strcmp("R3110HDW",sDvrInfo.sproductnumber))
			|| (0 == strcmp("R3106HDW",sDvrInfo.sproductnumber)))
		{
			SBizCfgPreview sPreviewPara;
			memset(&sPreviewPara, 0, sizeof(sPreviewPara));		
			ConfigGetCfgPreview(0, &sPreviewPara);
			for(i = 0; i < sDvrInfo.nPreviewNum; i++)
			{
				//csp modify 20130407
				//init_param |= (((sPreviewPara.nVideoSrcType[i] == 2) ? 1 : 0) << (31 - i));
				if(sPreviewPara.nVideoSrcType[i] == 1)//Video720P
				{
					init_param |= (1 << (30 - i*2));
				}
				else if(sPreviewPara.nVideoSrcType[i] == 3)//Video1080I
				{
					init_param |= (3 << (30 - i*2));
				}
				else if(sPreviewPara.nVideoSrcType[i] == 2)//Video1080P
				{
					init_param |= (2 << (30 - i*2));
				}
				else
				{
					init_param |= (2 << (30 - i*2));
				}
				
				if((0 == strcmp("R3110HDW",sDvrInfo.sproductnumber)) && (i == 1))
				{
					break;
				}
				else if((0 == strcmp("R3106HDW",sDvrInfo.sproductnumber)) && (i == 1))
				{
					break;
				}
			}
			
			printf("%s : init_param=0x%08x\n",sDvrInfo.sproductnumber,init_param);
		}
		
		//printf("%s 2 init_param = 0x%08x\n",__func__, init_param);
		
		memset(&sysPara, 0, sizeof(SBizSystemPara));
		if(0 != ConfigGetSystemPara(0, &sysPara))
		{
			printf("ConfigGetSystemPara failed!\n");
			return -1;
		}
		
		s8 bManual = sysPara.nManual;
		//printf("video format manual set:%d########################################\n",bManual);
		
		if(!bManual)
		{
			//自动
			if(IsPAL())
			{
				sysPara.nVideoStandard = EM_BIZ_PAL;
				printf("video format AUTO PAL########################################\n");
			}
			else
			{
				sysPara.nVideoStandard = EM_BIZ_NTSC;
				printf("video format AUTO NTSC########################################\n");
			}
			
			ConfigSetSystemPara(&sysPara);
		}
		
		{
			if(bManual)
			{
				if(EM_BIZ_PAL == sysPara.nVideoStandard)
				{
					//PAL
					init_param |= TL_PAL;	//0x02
					printf("video format manual set:PAL########################################\n");
				}
				else
				{
					//10 , NTSC
					init_param |= TL_NTSC;	//0x04
					printf("video format manual set:NTSC########################################\n");
				}
			}
			
			//printf("device product=%s\n",sDvrInfo.sproductnumber);//csp modify
			
			// debug by lanston
			if(0 == strcasecmp("R9624T",sDvrInfo.sproductnumber))
			{
				tl_hslib_init_c(TL_BOARD_TYPE_R9624T, init_param);
			}
			else if(0 == strcasecmp("R9632S",sDvrInfo.sproductnumber))
			{
				tl_hslib_init_c(TL_BOARD_TYPE_R9632SL, init_param);
			}
			else if(0 == strcasecmp("R9624SL",sDvrInfo.sproductnumber))
			{
				tl_hslib_init_c(TL_BOARD_TYPE_R9624SL, init_param);
			}
			else if(0 == strcasecmp("R9616S",sDvrInfo.sproductnumber))
			{
				tl_hslib_init_c(TL_BOARD_TYPE_R9616S, init_param);
			}
			else if(0 == strcasecmp("R9608S",sDvrInfo.sproductnumber))
			{
				tl_hslib_init_c(TL_BOARD_TYPE_R9608S, init_param);
			}
			else if(0 == strcasecmp("R9504S",sDvrInfo.sproductnumber))
			{
				#if defined(_JMV_)//csp modify
				tl_hslib_init_c(TL_BOARD_TYPE_R9504S_V22_JMS, init_param);
				#elif defined(_JUAN_)//csp modify
				tl_hslib_init_c(TL_BOARD_TYPE_R2004MX, init_param);
				#else
				//csp modify
				tl_hslib_init_c(TL_BOARD_TYPE_R9504S, init_param);
				//printf("init TL_BOARD_TYPE_R9504S......\n");
				//tl_hslib_init_c(TL_BOARD_TYPE_R9504_4D1, init_param);
				//printf("init TL_BOARD_TYPE_R9504_4D1......\n");
				#endif
			}
			else if(0 == strcasecmp("R9508S",sDvrInfo.sproductnumber))
			{
				#if defined(_JMV_)//csp modify
				tl_hslib_init_c(TL_BOARD_TYPE_R9508S_V22_JMS, init_param);
				#elif defined(_JUAN_)//csp modify
				tl_hslib_init_c(TL_BOARD_TYPE_R2008MX, init_param);
				#else
				tl_hslib_init_c(TL_BOARD_TYPE_R9508S, init_param);
				#endif
			}
			else if(0 == strcmp("R9516S",sDvrInfo.sproductnumber))
			{
				#if defined(_R9516S_CX25838_CX25828_)//csp modify
				tl_hslib_init_c(TL_BOARD_TYPE_R9516_CX25838_CX25828, init_param);
				#elif defined(longse)//csp modify
				tl_hslib_init_c(TL_BOARD_TYPE_R9516, init_param);
				#else
				tl_hslib_init_c(TL_BOARD_TYPE_R9516_TW2826, init_param);
				#endif
			}
			#ifdef CHIP_HISI3531
			else if(0 == strcmp("R3104HD",sDvrInfo.sproductnumber))
			{
				tl_hslib_init_c(TL_BOARD_TYPE_R9800_MA, init_param);
			}
			else if(0 == strcmp("R3110HDW",sDvrInfo.sproductnumber))
			{
				tl_hslib_init_c(TL_BOARD_TYPE_R9800_DA, init_param);
			}
			else if(0 == strcmp("R3106HDW",sDvrInfo.sproductnumber))
			{
				init_param |= (0x01<<16);
				tl_hslib_init_c(TL_BOARD_TYPE_R9800_DA, init_param);
			}
			else if(0 == strcmp("R3116",sDvrInfo.sproductnumber))
			{
				tl_hslib_init_c(TL_BOARD_TYPE_R9816_SA, init_param);
			}
			else if(0 == strcmp("R3116W",sDvrInfo.sproductnumber))
			{
				init_param |= (0x01<<17);
				tl_hslib_init_c(TL_BOARD_TYPE_R9816_SA, init_param);
			}
			else if(0 == strcmp("NR3116",sDvrInfo.sproductnumber))
			{
				tl_hslib_init_c(TL_BOARD_TYPE_NR3116, init_param);
			}
			else if(0 == strcmp("NR3132",sDvrInfo.sproductnumber))
			{
				tl_hslib_init_c(TL_BOARD_TYPE_NR3132, init_param);
			}
			else if(0 == strcmp("NR3124",sDvrInfo.sproductnumber))
			{
				tl_hslib_init_c(TL_BOARD_TYPE_NR3124, init_param);
			}
			#endif
			#ifdef CHIP_HISI3521
			else if(0 == strcmp("R2104",sDvrInfo.sproductnumber))
			{
				tl_hslib_init_c(TL_BOARD_TYPE_R9704S, init_param);
			}
			else if(0 == strcmp("R2104W",sDvrInfo.sproductnumber))
			{
				init_param |= (0x03<<17);
				tl_hslib_init_c(TL_BOARD_TYPE_R9704S, init_param);
			}
			else if(0 == strcmp("R2108",sDvrInfo.sproductnumber))
			{
				tl_hslib_init_c(TL_BOARD_TYPE_R9708S, init_param);
			}
			else if(0 == strcmp("R2108W",sDvrInfo.sproductnumber))
			{
				init_param |= (0x03<<17);
				tl_hslib_init_c(TL_BOARD_TYPE_R9708S, init_param);
			}
			else if(0 == strcmp("R2116",sDvrInfo.sproductnumber))
			{
				//csp modify 20121205
				//init_param |= (0x01<<17);
				//tl_hslib_init_c(TL_BOARD_TYPE_R9716S, init_param);
				tl_hslib_init_c(TL_BOARD_TYPE_R9716D, init_param);
			}
			else if(0 == strcmp("R2016",sDvrInfo.sproductnumber))
			{
				//csp modify 20130506
				//init_param |= (0x01<<17);
				tl_hslib_init_c(TL_BOARD_TYPE_R9716S, init_param);
			}
			else if(0 == strcmp("NR2116",sDvrInfo.sproductnumber))
			{
				//printf("%s 3 init_param = 0x%08x\n",__func__, init_param);
				tl_hslib_init_c(TL_BOARD_TYPE_NR2116, init_param);
			}
			else if(0 == strcmp("NR1004",sDvrInfo.sproductnumber))
			{
				tl_hslib_init_c(TL_BOARD_TYPE_NR1004, init_param);
			}
			else if(0 == strcmp("NR1008",sDvrInfo.sproductnumber))
			{
				tl_hslib_init_c(TL_BOARD_TYPE_NR1008, init_param);
			}
			#endif
			else
			{
				switch(sDvrInfo.nVidMainNum)
				{
					case 4:
					{
			    		tl_hslib_init_c(TL_BOARD_TYPE_R9504_4D1, init_param);
						//printf("here,init TL_BOARD_TYPE_R9504_4D1......\n");
					} break;
					case 8:
					{
			    		tl_hslib_init_c(TL_BOARD_TYPE_R9508, init_param);
						//printf("here,init TL_BOARD_TYPE_R9508......\n");
						//tl_hslib_init_c(TL_BOARD_TYPE_R9508S, init_param);
						//printf("here,init TL_BOARD_TYPE_R9508S......\n");
			    	} break;
					case 16:
					{
						
					} break;
					default:
					{
			    		tl_hslib_init_c(TL_BOARD_TYPE_R9504_4D1, init_param);
					} break;
				}
			}
		}
		
		if(EM_BIZ_NTSC == sysPara.nVideoStandard)
		{
			//NTSC
			switch(sysPara.nOutput)
			{
				case EM_BIZ_OUTPUT_VGA_800X600:
					tl_vga_resolution(VGA_RESOL_N_800_60);
					break;
				case EM_BIZ_OUTPUT_VGA_1024X768:
					tl_vga_resolution(VGA_RESOL_N_1024_60);
					break;
				case EM_BIZ_OUTPUT_VGA_1280X1024:
					tl_vga_resolution(VGA_RESOL_N_1280_60);
					break;
#if defined(CHIP_HISI3531) || defined(CHIP_HISI3521)
				case EM_BIZ_OUTPUT_HDMI_1280X720:
					tl_vga_resolution(HDMI_RESOL_720P60);
					break;
				case EM_BIZ_OUTPUT_HDMI_1920X1080:
					tl_vga_resolution(HDMI_RESOL_1080P60);
					break;
#endif
				default:
					//csp modify
					//tl_vga_resolution(VGA_RESOL_N_1024_60);
					tl_vga_resolution(VGA_RESOL_N_800_60);
					break;
			}
		}
		else
		{
			//PAL
			switch(sysPara.nOutput)
			{
				case EM_BIZ_OUTPUT_VGA_800X600:
					tl_vga_resolution(VGA_RESOL_P_800_60);
					break;
				case EM_BIZ_OUTPUT_VGA_1024X768:
					tl_vga_resolution(VGA_RESOL_P_1024_60);
					break;
				case EM_BIZ_OUTPUT_VGA_1280X1024:
					tl_vga_resolution(VGA_RESOL_P_1280_60);
					break;
#if defined(CHIP_HISI3531) || defined(CHIP_HISI3521)
				case EM_BIZ_OUTPUT_HDMI_1280X720:
					//csp modify 20130101
					tl_vga_resolution(HDMI_RESOL_720P50);
					//tl_vga_resolution(HDMI_RESOL_720P60);//csp modify 20140423
					break;
				case EM_BIZ_OUTPUT_HDMI_1920X1080:
					//csp modify 20130101
					tl_vga_resolution(HDMI_RESOL_1080P50);
					//tl_vga_resolution(HDMI_RESOL_1080P60);//csp modify 20140423
					break;
#endif
				default:
					//csp modify
					//tl_vga_resolution(VGA_RESOL_P_1024_60);
					tl_vga_resolution(VGA_RESOL_P_800_60);
					break;
			}
		}
		
		initStateOSD();//why???
		
		//读取设备性能参数
		psBizManager->nAlarmOutNum = sDvrInfo.nAlarmoutNum;
		psBizManager->nAudioNum = sDvrInfo.nAudNum;
		psBizManager->nBuzzNum = sDvrInfo.nBuzzNum;
		psBizManager->nSensorNum = sDvrInfo.nSensorNum;
		psBizManager->nSnapNum = sDvrInfo.nSnapNum;
		psBizManager->nVideoMainNum = sDvrInfo.nVidMainNum;
		psBizManager->nVideoSubnum = sDvrInfo.nVidSubNum;
		psBizManager->nTalkNum = 1;//sDvrInfo.nVidSubNum;//why???
		
		psBizManager->pnVideoMainNetStatus = malloc(psBizManager->nVideoMainNum);
		memset(psBizManager->pnVideoMainNetStatus, 0, psBizManager->nVideoMainNum);
		
		psBizManager->pnVideoSubNetStatus = malloc(psBizManager->nVideoSubnum);
		memset(psBizManager->pnVideoSubNetStatus, 0, psBizManager->nVideoSubnum);
		
		psBizManager->pnVideoSubNetMbStatus = malloc(psBizManager->nVideoSubnum);
		memset(psBizManager->pnVideoSubNetMbStatus, 0, psBizManager->nVideoSubnum);
		
		//csp modify 20140525
		psBizManager->pnVideoThirdNetStatus = malloc(psBizManager->nVideoMainNum);
		memset(psBizManager->pnVideoThirdNetStatus, 0, psBizManager->nVideoMainNum);
		psBizManager->pnVideoThirdNetMbStatus = malloc(psBizManager->nVideoMainNum);
		memset(psBizManager->pnVideoThirdNetMbStatus, 0, psBizManager->nVideoMainNum);
		
		psBizManager->pnVideoMainRecStatus = malloc(psBizManager->nVideoMainNum);
		memset(psBizManager->pnVideoMainRecStatus, 0, psBizManager->nVideoMainNum);
		
		psBizManager->pnTalkStatus = malloc(psBizManager->nTalkNum);
		memset(psBizManager->pnTalkStatus, 0, psBizManager->nTalkNum);
		
		psBizManager->pnAudioRecStatus = malloc(psBizManager->nAudioNum);
		memset(psBizManager->pnAudioRecStatus, 0, psBizManager->nAudioNum);
		
		psBizManager->pnAudioNetStatus = malloc(psBizManager->nAudioNum);
		memset(psBizManager->pnAudioNetStatus, 0, psBizManager->nAudioNum);
		
		psBizManager->pnSensorStatus = malloc(psBizManager->nSensorNum);
		memset(psBizManager->pnSensorStatus, 0, psBizManager->nSensorNum);
		
		psBizManager->pnVMotionStatus = malloc(psBizManager->nVideoMainNum);
		memset(psBizManager->pnVMotionStatus, 0, psBizManager->nVideoMainNum);
		
		psBizManager->pnVLostStatus = malloc(psBizManager->nVideoMainNum);
		memset(psBizManager->pnVLostStatus, 0, psBizManager->nVideoMainNum);
		
		psBizManager->pnVBlindStatus = malloc(psBizManager->nVideoMainNum);
		memset(psBizManager->pnVBlindStatus, 0, psBizManager->nVideoMainNum);
		
		//csp modify 20121225
		g_nFlagRecVideoAudio = (int*)calloc(sizeof(int), psBizManager->nVideoMainNum);
		memset(g_nFlagRecVideoAudio, 0, sizeof(int)*psBizManager->nVideoMainNum);
		
#if 1//csp modify 20130303//csp test
		printf("PTZ init start......\n");
		
		//初始化云台并设置
		SBizPtz sBizPtz;
		sBizPtz.nChnNum = psBizManager->nVideoMainNum;
		
		#if defined(CHIP_HISI3531)// || defined(CHIP_HISI3521)
		sBizPtz.pDevPath =	"/dev/ttyAMA2";
		#else
		sBizPtz.pDevPath =	"/dev/ttyAMA1";
		#endif
		psBizPara->nModMask |= MASK_BIZ_MOD_PTZ;
		ret = PtzInit(psBizPara->nModMask & MASK_BIZ_MOD_PTZ, &sBizPtz);
		printf("yg PtzInit %s ret: %d\n", sBizPtz.pDevPath, ret);
		if(0 != ret)
		{
			return BIZ_ERR_INIT_PTZ_INIT;
		}
		usleep(100 * 1000);
		
		SBizPtzPara sBizPtzPara;
		memset(&sBizPtzPara, 0, sizeof(sBizPtzPara));
		
		for(i = 0; i < psBizManager->nVideoMainNum; i++)
		{
			ConfigGetPtzPara(0, &sBizPtzPara, i);
			
			//csp modify 20130323
			//PtzSetPara(i, &sBizPtzPara);
			PtzInitPara(i, &sBizPtzPara);
		}
		
		tl_rs485_ctl(1);
		
		printf("PTZ init over......\n");
#endif
		
#if 1//csp modify 20130407
		//初始化用户管理
		printf("start init user mod......\n");
		
		SBizUserPara sBizUserPara;
		memset(&sBizUserPara, 0, sizeof(sBizUserPara));
		ConfigGetUserPara(0,&sBizUserPara);
		
		if(0 != UserInit(psBizPara->nModMask & MASK_BIZ_MOD_USER, &sBizUserPara))
		{
			return BIZ_ERR_INIT_USER_INIT;
		}
		//usleep(100 * 1000);
		
		printf("user init OK!\n");
#endif
		
		///////////////////////////第一次到此为止/////////////////////////////////////
		flag_First = 0;
		return BIZ_SUCCESS;
	}
	//////////////////////////////////////////////////////////////////////////////
	
//#ifdef RELEASE_VERSION//csp modify
	srand((unsigned int)time(NULL));
	int status_auth = rand();
	
	char cmdline_auth[20] = {0};
	sprintf(cmdline_auth,"%08d", status_auth);
	
	if(1)// (!(0 == strcasecmp("R9632S",sDvrInfo.sproductnumber)))
	{
		//status_auth = tl_license_check(cmdline_auth, 8);
		//printf("check result:%d\n",status_auth);
		
		//csp modify 20130704
		if(0 == access("./longsedvr.txt", F_OK))
		{
			printf("###longse dvr###\n");
			
			status_auth = 0;
		}
		else if(0 == access("/root/longsedvr.txt", F_OK))
		{
			printf("***longse dvr***\n");
			
			status_auth = 0;
		}
		else
		{
			printf("^^^whrz dvr^^^\n");
			
			//status_auth = tl_license_check(cmdline_auth, 8);
			//printf("^^^check result:%d^^^\n",status_auth);
			status_auth = 0;
		}
		
		//if(0 == strcasecmp("R9504S",sDvrInfo.sproductnumber) || 0 == strcasecmp("R9508S",sDvrInfo.sproductnumber) || 0 == strcasecmp("R9516S",sDvrInfo.sproductnumber))
		if(0 == strcasecmp("R9504S",sDvrInfo.sproductnumber) || 0 == strcasecmp("R9508S",sDvrInfo.sproductnumber))
		{
			#ifdef longse
			//printf("No check\n");
			status_auth = 0;//csp modify
			#endif
		}
		else
		{
			//printf("Need check\n");
		}
		
		if(0 == strcasecmp("NR2116",sDvrInfo.sproductnumber) || 
			0 == strcasecmp("NR3116",sDvrInfo.sproductnumber) ||
			0 == strcasecmp("NR3132",sDvrInfo.sproductnumber) ||
			0 == strcasecmp("NR3124",sDvrInfo.sproductnumber) ||
			0 == strcasecmp("NR1004",sDvrInfo.sproductnumber) ||
			0 == strcasecmp("NR1008",sDvrInfo.sproductnumber))
		{
			if(status_auth != 0)
			{
				#if 1//#ifndef _JMV_//csp modify
				status_auth = 0;//csp modify
				//u32 app_start_time = (u32)time(NULL);
				u32 app_start_time = read_rtc(1);
				struct tm currtm;
				currtm.tm_year = 2013 - 1900;
				currtm.tm_mon  = 12 - 1;
				currtm.tm_mday = 1;
				currtm.tm_hour = 0;
				currtm.tm_min = 0;
				currtm.tm_sec = 0;
				time_t overdue = mktime(&currtm);
				if(app_start_time >= (u32)overdue && (int)app_start_time > 0)
				{
					printf("Date too large:0x%x\n",app_start_time);
					status_auth = -1;//for nvr debug
				}
				currtm.tm_year = 2012 - 1900;
				currtm.tm_mon  = 6 - 1;
				currtm.tm_mday = 1;
				currtm.tm_hour = 0;
				currtm.tm_min = 0;
				currtm.tm_sec = 0;
				overdue = mktime(&currtm);
				if(app_start_time < (u32)overdue)
				{
					printf("Date too small\n");
					//status_auth = -1;
				}
				printf("##########################################################nvr app_start_time=%s",ctime((time_t *)&app_start_time));
				#endif
			}
		}
		
		//csp modify 20130704
		//#define APP_TIME_LIMIT
		#ifdef APP_TIME_LIMIT
		//if(0 == strcasecmp("R9504S",sDvrInfo.sproductnumber) || 0 == strcasecmp("R9508S",sDvrInfo.sproductnumber) || 0 == strcasecmp("R9516S",sDvrInfo.sproductnumber))
		if(0 == strcasecmp("R9504S",sDvrInfo.sproductnumber) || 0 == strcasecmp("R9508S",sDvrInfo.sproductnumber))
		{
			#ifdef longse
			status_auth = 0;//csp modify
			#else
			if(status_auth != 0)
			{
				#if 1//#ifndef _JMV_//csp modify
				status_auth = 0;//csp modify
				//u32 app_start_time = (u32)time(NULL);
				u32 app_start_time = read_rtc(1);
				struct tm currtm;
				currtm.tm_year = 2013 - 1900;
				currtm.tm_mon  = 3 - 1;
				currtm.tm_mday = 1;
				currtm.tm_hour = 0;
				currtm.tm_min = 0;
				currtm.tm_sec = 0;
				time_t overdue = mktime(&currtm);
				if(app_start_time >= (u32)overdue && (int)app_start_time > 0)
				{
					printf("Date too large\n");
					status_auth = -1;//for dvr debug
				}
				currtm.tm_year = 2012 - 1900;
				currtm.tm_mon  = 6 - 1;
				currtm.tm_mday = 1;
				currtm.tm_hour = 0;
				currtm.tm_min = 0;
				currtm.tm_sec = 0;
				overdue = mktime(&currtm);
				if(app_start_time < (u32)overdue)
				{
					printf("Date too small\n");
					//status_auth = -1;
				}
				printf("##########################################################app_start_time=%s",ctime((time_t *)&app_start_time));
				#endif
			}
			#endif
		}
		else
		{
			#if 1//csp modify
			if(status_auth != 0)
			{
				status_auth = 0;//csp modify
				//u32 app_start_time = (u32)time(NULL);
				u32 app_start_time = read_rtc(1);
				struct tm currtm;
				currtm.tm_year = 2013 - 1900;
				currtm.tm_mon  = 6 - 1;
				currtm.tm_mday = 1;
				currtm.tm_hour = 0;
				currtm.tm_min = 0;
				currtm.tm_sec = 0;
				time_t overdue = mktime(&currtm);
				if(app_start_time >= (u32)overdue && (int)app_start_time > 0)
				{
					printf("Date too large\n");
					status_auth = -1;//for dvr debug
				}
				currtm.tm_year = 2012 - 1900;
				currtm.tm_mon  = 6 - 1;
				currtm.tm_mday = 1;
				currtm.tm_hour = 0;
				currtm.tm_min = 0;
				currtm.tm_sec = 0;
				overdue = mktime(&currtm);
				if(app_start_time < (u32)overdue)
				{
					printf("Date too small\n");
					//status_auth = -1;
				}
				printf("##########################################################app_start_time=%s",ctime((time_t *)&app_start_time));
			}
			#endif
		}
		#endif
		
		if(status_auth != 0)
		{
			printf("check failed\n");
			
			if(g_NoauthCB != NULL)
			{
				g_NoauthCB();
			}
			
			while(1)
			{
				sleep(10);
			}
		}
	}
	//printf("this is a release version!\n");
//#endif
	
	//初始化公共库及设备管理
	PublicInit();
	HDDHDR hddHdr = PublicGetHddManage();
	
	//初始化预览并设置
	SBizPreview sBizPreview;
	sBizPreview.nAudioNum = psBizManager->nAudioNum;
	sBizPreview.nVideoNum = psBizManager->nVideoMainNum;
	sBizPreview.nMaxStrOsdNum = 0;
	sBizPreview.nMaxImgOsdNum = 0;
	sBizPreview.nMaxRectOsdNum = sDvrInfo.nRectOsdNum;
	
	if(0 != PreviewInit(psBizPara->nModMask & MASK_BIZ_MOD_PREVIEW, &sBizPreview))
	{
		return BIZ_ERR_INIT_PREVIEW_INIT;
	}
	
    SBizCfgPreview sPreviewPara;
	memset(&sPreviewPara, 0, sizeof(sPreviewPara));
	ConfigGetCfgPreview(0, &sPreviewPara);
	
	BizPreviewAudioOutChn(sPreviewPara.nAudioOutChn);
	BizPreviewMute(sPreviewPara.nMute);
	BizPreviewVolume(sPreviewPara.nVolume);
	
	SBizCfgPatrol sPatrolPara;
	ConfigGetCfgPatrol(0, &sPatrolPara);
	ConfigSetCfgPatrol(&sPatrolPara);
	
	SBizPreviewImagePara psPara;
	SBizCfgRectOsd sRectOsd;
	
	memset(&psPara, 0, sizeof(SBizPreviewImagePara));
	memset(&sRectOsd, 0, sizeof(SBizCfgRectOsd));
	
	int nRectOsdNumPerChn = 0;
	
	if(sBizPreview.nVideoNum)
	{
		nRectOsdNumPerChn = sDvrInfo.nRectOsdNum / sBizPreview.nVideoNum;
	}
	
	SRect sRectTmp[nRectOsdNumPerChn];//why???
	
	sRectOsd.psRectOsd = sRectTmp;
	sRectOsd.nMaxNum = nRectOsdNumPerChn;
	
	for(i = 0; i < sBizPreview.nVideoNum; i++)
	{
		//获取配置图像参数
		ConfigGetPreviewImagePara(0,&psPara, i);
		
		//按配置数据设置图像参数
		PreviewSetImage(i, (SPreviewImagePara*)&psPara);
		
		memset(sRectTmp, 0, sizeof(sRectTmp));
		ConfigGetCfgRectOsd(0, &sRectOsd, i);
		
		int j = 0;
		for (j = 0; j < sRectOsd.nRealNum; j++)
		{
			if((0 == strcmp("R9624T",sDvrInfo.sproductnumber))
				|| (0 == strcmp("R9632S",sDvrInfo.sproductnumber))
				 || (0 == strcmp("R9624SL",sDvrInfo.sproductnumber))
				 || (0 == strcmp("R9516S",sDvrInfo.sproductnumber))
				 || (0 == strcmp("R9608S",sDvrInfo.sproductnumber)))
			{
				sRectTmp[j].x = (sRectTmp[j].x / 2);
				sRectTmp[j].y = (sRectTmp[j].y / 2);
				sRectTmp[j].w = (sRectTmp[j].w / 2);
				sRectTmp[j].h = (sRectTmp[j].h / 2);
				if(sRectTmp[j].x == 8)//why???
				{
					sRectTmp[j].x  = 0;
				}
			}
			else if((0 == strcasecmp("R3104HD",sDvrInfo.sproductnumber))
					|| (0 == strcasecmp("R3110HDW",sDvrInfo.sproductnumber))
					|| (0 == strcasecmp("R3106HDW",sDvrInfo.sproductnumber)))
			{
				sRectTmp[j].x = (sRectTmp[j].x * 1920 / 704);
				sRectTmp[j].y = (sRectTmp[j].y * 1080 / (sysPara.nVideoStandard == EM_BIZ_PAL ? 576 : 480));
				sRectTmp[j].w = (sRectTmp[j].w * 1920 / 704);
				sRectTmp[j].h = (sRectTmp[j].h * 1080 / (sysPara.nVideoStandard == EM_BIZ_PAL ? 576 : 480));
			}
			//printf("sRectTmp[%d]:%d %d %d %d\n", j, sRectTmp[j].x, sRectTmp[j].y, sRectTmp[j].w, sRectTmp[j].h);
			PreviewRectOsdShow(i, j, sRectOsd.nShowRectOsd, &sRectTmp[j]);
		}
	}
	
	PUBPRT("PreviewInit");
	
	#ifdef TEST_TIMESPAN
	nCurTime = PublicGetTimeStamp();
	printf("$$$$$$$$DoBizInit line:%d last:%u cur:%u span:%u\n", __LINE__, nLastTime, nCurTime, nCurTime - nLastTime);
	nLastTime = nCurTime;
	#endif
	
	#if 1//csp modify 20130101
	//初始化杂项库并设置
	SBizSysComplex sBizSysComplex;
	sBizSysComplex.hddHdr = hddHdr;
    if(0 != SysComplexInit(psBizPara->nModMask & MASK_BIZ_MOD_SYSCOMPLEX, &sBizSysComplex))
    {
        return BIZ_ERR_INIT_SYSCOMPLEX_INIT;
    }
	#endif
	
	//初始化编码并设置
	SBizEncode sBizEncode;
	sBizEncode.nAudioMaxFrameSize = sDvrInfo.nMaxAudFrameSize;//1024;
	sBizEncode.nAudioNum = psBizManager->nAudioNum;
	sBizEncode.nMainMaxFrameSize = sDvrInfo.nMaxMainFrameSize;
	sBizEncode.nMainNum = psBizManager->nVideoMainNum;
	sBizEncode.nSnapMaxFrameSize = sDvrInfo.nMaxSnapFrameSize;
	sBizEncode.nSnapNum = psBizManager->nSnapNum;
	sBizEncode.nSubMaxFrameSize = sDvrInfo.nMaxSubFrameSize;
	sBizEncode.nSubNum = psBizManager->nVideoSubnum;
	if(0 == strcasecmp("R9616S",sDvrInfo.sproductnumber))
	{
		sBizEncode.emChipType = EM_BIZ_ENCODE_BOARD_HISI3520_16D1;
	}
	else
	{
		sBizEncode.emChipType = EM_BIZ_ENCODE_BOARD_COMMON;
	}
	
	if(0 != EncodeInit(psBizPara->nModMask & MASK_BIZ_MOD_ENCODE, &sBizEncode))
	{
		return BIZ_ERR_INIT_ENCODE_INIT;
	}
	
    PUBPRT("EncodeInit");
	
	usleep(100 * 1000);
	
	SBizEncodePara sBizEncPara;
	for(i = 0; i < sBizEncode.nMainNum; i++)
	{
		ConfigGetVMainEncodePara(0, &sBizEncPara, i);
		//printf("DoBizInit:chn%d VideoResolution:%d\n",i,sBizEncPara.nVideoResolution);//csp modify
		
		#if 1//csp modify 20130407
		u8 flag = 0;
		switch(sPreviewPara.nVideoSrcType[i])
		{
		#if 1//csp modify 20130407
			case 1://720P
				if(sBizEncPara.nVideoResolution == EM_BIZENC_RESOLUTION_1080P)
				{
					sBizEncPara.nVideoResolution = EM_BIZENC_RESOLUTION_720P;
					flag = 1;
				}
				break;
		#else
			case 0:
				if(sBizEncPara.nVideoResolution != EM_BIZENC_RESOLUTION_960H/*EM_BIZENC_RESOLUTION_D1*/)
				{
					sBizEncPara.nVideoResolution = EM_BIZENC_RESOLUTION_960H/*EM_BIZENC_RESOLUTION_D1*/;
					flag = 1;
				}
				break;
			case 1:
				if(sBizEncPara.nVideoResolution != EM_BIZENC_RESOLUTION_720P)
				{
					sBizEncPara.nVideoResolution = EM_BIZENC_RESOLUTION_720P;
					flag = 1;
				}
				break;
			case 2:
				if(sBizEncPara.nVideoResolution != EM_BIZENC_RESOLUTION_1080P)
				{
					sBizEncPara.nVideoResolution = EM_BIZENC_RESOLUTION_1080P;
					flag = 1;
				}
				break;
		#endif
			default:
				break;
		}
		if(flag)
		{
			SModConfigVideoParam para;
			ModConfigGetParam(EM_CONFIG_PARA_ENC_MAIN, &para, i);
			para.nResolution = sBizEncPara.nVideoResolution;
			ModConfigSetParam(EM_CONFIG_PARA_ENC_MAIN, &para, i);
		}
		#endif
		
		VEncMainSetParam(i, &sBizEncPara);
	}
	//PUBPRT("HERE");
	
	for(i = 0; i < sBizEncode.nSubNum; i++)
	{
		ConfigGetVSubEncodePara(0,&sBizEncPara, i);
		VEncSubSetParam(i, &sBizEncPara);
	}
	
	if(0 != EncodeOsdInit())
	{
		printf("EncodeOsdInit failed\n");
		return BIZ_ERR_INIT_ENCODE_INIT;
	}
	//PUBPRT("HERE");
	
	#ifdef TEST_TIMESPAN
	nCurTime = PublicGetTimeStamp();
	printf("$$$$$$$$DoBizInit line:%d last:%u cur:%u span:%u\n", __LINE__, nLastTime, nCurTime, nCurTime - nLastTime);
	nLastTime = nCurTime;
	#endif
	
	//初始化录像并设置
    SBizRecord sBizRecord;
    sBizRecord.nChnNum = sDvrInfo.nRecNum;
    sBizRecord.hddinfo = (RecHddInfo)hddHdr;
    sBizRecord.sModRecPara = calloc(sizeof(SModRecordRecPara), sDvrInfo.nRecNum);
	for(i = 0; i < sBizRecord.nChnNum; i++)
	{
		if((0 == strcmp("R9624T",sDvrInfo.sproductnumber))
			|| (0 == strcmp("R9632S",sDvrInfo.sproductnumber))
			|| (0 == strcmp("R9516S",sDvrInfo.sproductnumber))
			 || (0 == strcmp("R9624SL",sDvrInfo.sproductnumber)))
		{
			sBizRecord.sModRecPara[i].emResolution = EM_REC_ENC_CIF; 
		}
		else
		{
	    	sBizRecord.sModRecPara[i].emResolution = EM_REC_ENC_D1; 
		}
		
		sBizRecord.sModRecPara[i].nFrameRate = 25;
		sBizRecord.sModRecPara[i].nBitRate = sDvrInfo.nMaxMainFrameSize * 8 / 1024;
		
		if((0 == strcmp("R9508S",sDvrInfo.sproductnumber)) && (i > 1))
		{
			sBizRecord.sModRecPara[i].nBitRate = ((sDvrInfo.nMaxSubFrameSize > 0x14000)?sDvrInfo.nMaxSubFrameSize:0x14000) * 8 /1024;//why???
		}
		
		//csp modify
		if((0 == strcmp("R9508",sDvrInfo.sproductnumber)) && (i > 1))
		{
			sBizRecord.sModRecPara[i].nBitRate = ((sDvrInfo.nMaxSubFrameSize > 0x14000)?sDvrInfo.nMaxSubFrameSize:0x14000) * 8 /1024;//why???
		}
	}
	
    if(0 != RecordInit(psBizPara->nModMask & MASK_BIZ_MOD_RECORD, &sBizRecord))
    {
        return BIZ_ERR_INIT_RECORD_INIT;
    }
	else
	{
		RecordSetDealHardDiskFull(sysPara.nCyclingRecord);
		
		i = MAX_HDD_NUM;
		if(hddHdr)
		{
			for (i = 0; i < MAX_HDD_NUM; ++i) 
			{
				if ((((disk_manager *)hddHdr)->hinfo[i].is_disk_exist) &&
		            (((disk_manager *)hddHdr)->hinfo[i].storage_type != 'u')) 
		        {
					break;
				}
			}
		}
		
		if(i < MAX_HDD_NUM)
		{
			SBizRecPara sBizRecPara;
			SModRecSchPara sRecSch;
			SBizSchedule sBizSch;
			
			for(i = 0; i < sDvrInfo.nRecNum; i++)
			{
				if((0 == GetRecEnable(i)))// && (0 == GetRecStreamType(i)))
				{
					RecordStop(i);//yzw add  9624//why???
				}
				
				ConfigGetRecPara(0, &sBizRecPara, i);
				RecordSetPreTime(i, (u16)sBizRecPara.nPreTime);
				RecordSetDelayTime(i, sBizRecPara.nRecDelay);
				
				ConfigGetSchRecSchedule(0, &sBizSch, i);
				
				sRecSch.nChn = i;
				sRecSch.nValue = 0x7f;
				sRecSch.nRecType = EM_REC_SCH_REC_TIMER;
				sRecSch.nTimeType = sBizSch.nSchType;
				memcpy(&sRecSch.nSchTime[0][0], &sBizSch.sSchTime[0][0], sizeof(SModRecSchTime)*31*MAX_TIME_SEGMENTS);
				
				RecordSetSchedulePara(i, &sRecSch);
				
				ConfigGetVMotionAlaRecSchedule(0, &sBizSch, i);
				
				sRecSch.nChn = i;
				sRecSch.nValue = 0x7f;
				sRecSch.nRecType = EM_REC_SCH_REC_VMOTION;
				sRecSch.nTimeType = sBizSch.nSchType;
				memcpy(&sRecSch.nSchTime[0][0], &sBizSch.sSchTime[0][0], sizeof(SModRecSchTime)*31*MAX_TIME_SEGMENTS);
				
				RecordSetSchedulePara(i, &sRecSch);
				
				ConfigGetSensorAlaRecSchedule(0, &sBizSch, i);
				
				sRecSch.nChn = i;
				sRecSch.nValue = 0x7f;
				sRecSch.nRecType = EM_REC_SCH_REC_ALARM;
				sRecSch.nTimeType = sBizSch.nSchType;
				memcpy(&sRecSch.nSchTime[0][0], &sBizSch.sSchTime[0][0], sizeof(SModRecSchTime)*31*MAX_TIME_SEGMENTS);
				
				RecordSetSchedulePara(i, &sRecSch);
			}
		}
	}
	
	#ifdef TEST_TIMESPAN
	nCurTime = PublicGetTimeStamp();
	printf("$$$$$$$$DoBizInit line:%d last:%u cur:%u span:%u\n", __LINE__, nLastTime, nCurTime, nCurTime - nLastTime);
	nLastTime = nCurTime;
	#endif
 	
    //初始化回放并设置
    SBizPlayback sBizPlayback;
    memset(&sBizPlayback, 0, sizeof(SBizPlayback));
	
    sBizPlayback.nMaxChn = psBizManager->nVideoMainNum;
    sBizPlayback.hDisk = hddHdr;
	
	if((0 == strcmp("R9624T",sDvrInfo.sproductnumber))
		|| (0 == strcmp("R9632S",sDvrInfo.sproductnumber))
		 || (0 == strcmp("R9624SL",sDvrInfo.sproductnumber))
		  || (0 == strcmp("R9516S",sDvrInfo.sproductnumber)))
	{
		sBizPlayback.nChnMaskOfD1 = 0;
	}
	else if (0 == strcmp("R9508S",sDvrInfo.sproductnumber))
	{
		sBizPlayback.nChnMaskOfD1 = 0x3;
	}
	else if (0 == strcmp("R9508",sDvrInfo.sproductnumber))//csp modify
	{
		sBizPlayback.nChnMaskOfD1 = 0x3;
	}
	else if (0 == strcmp("R3116",sDvrInfo.sproductnumber))//csp modify
	{
		sBizPlayback.nChnMaskOfD1 = 0;
	}
	else if (0 == strcmp("R3116W",sDvrInfo.sproductnumber))//csp modify
	{
		sBizPlayback.nChnMaskOfD1 = 0;
	}
	else if (0 == strcmp("R3104HD",sDvrInfo.sproductnumber))//csp modify
	{
		sBizPlayback.nChnMaskOfD1 = 0xffffffff;
	}
	else if (0 == strcmp("R2104",sDvrInfo.sproductnumber))//csp modify
	{
		sBizPlayback.nChnMaskOfD1 = 0xffffffff;
	}
	else if (0 == strcmp("R2104W",sDvrInfo.sproductnumber))//csp modify
	{
		sBizPlayback.nChnMaskOfD1 = 0xffffffff;
	}
	else if (0 == strcmp("R2108",sDvrInfo.sproductnumber))//csp modify
	{
		sBizPlayback.nChnMaskOfD1 = 0xffffffff;
	}
	else if (0 == strcmp("R2108W",sDvrInfo.sproductnumber))//csp modify
	{
		sBizPlayback.nChnMaskOfD1 = 0xffffffff;
	}
	else if (0 == strcmp("R2116",sDvrInfo.sproductnumber))//csp modify
	{
		sBizPlayback.nChnMaskOfD1 = 0xffffffff;//???
	}
	else if (0 == strcmp("R2016",sDvrInfo.sproductnumber))//csp modify
	{
		sBizPlayback.nChnMaskOfD1 = 0;
	}
	else if (0 == strcmp("NR2116",sDvrInfo.sproductnumber))//csp modify
	{
		sBizPlayback.nChnMaskOfD1 = 0xffffffff;//???
	}
	else if (0 == strcmp("NR3116",sDvrInfo.sproductnumber))//csp modify
	{
		sBizPlayback.nChnMaskOfD1 = 0xffffffff;//???
	}
	else if (0 == strcmp("NR3132",sDvrInfo.sproductnumber))//csp modify
	{
		sBizPlayback.nChnMaskOfD1 = 0xffffffff;//???
	}
	else if (0 == strcmp("NR3124",sDvrInfo.sproductnumber))//csp modify
	{
		sBizPlayback.nChnMaskOfD1 = 0xffffffff;//???
	}
	else if (0 == strcmp("NR1004",sDvrInfo.sproductnumber))//csp modify
	{
		sBizPlayback.nChnMaskOfD1 = 0xffffffff;//???
	}
	else if (0 == strcmp("NR1008",sDvrInfo.sproductnumber))//csp modify
	{
		sBizPlayback.nChnMaskOfD1 = 0xffffffff;//???
	}
	else
	{
    	sBizPlayback.nChnMaskOfD1 = 0xffffffff;
	}
	
    if(0 != PlaybackInit(psBizPara->nModMask & MASK_BIZ_MOD_PLAYBACK, &sBizPlayback))
    {
        return BIZ_ERR_INIT_PLAYBACK_INIT;
    }
	
	//初始化日志并设置
	SBizLogInitPara logPara;
	memset(&logPara, 0, sizeof(logPara));
	
	logPara.nMaxLogs = 0x4ff;
	logPara.pHddMgr = (void*)hddHdr;
	//yaogang modify 20170218 in shanghai
	//psBizPara->nModMask &= ~(MASK_BIZ_MOD_LOG);
	if(0 != LogInit(psBizPara->nModMask & MASK_BIZ_MOD_LOG, &logPara))
	{
		return BIZ_ERR_INIT_LOG_INIT;
	}
	
#if 0//csp modify 20130303
	//初始化云台并设置
	SBizPtz sBizPtz;
	sBizPtz.nChnNum = psBizManager->nVideoMainNum;
	
	#if defined(CHIP_HISI3531)// || defined(CHIP_HISI3521)
	sBizPtz.pDevPath =  "/dev/ttyAMA2";
	#else
	sBizPtz.pDevPath =  "/dev/ttyAMA1";
	#endif
	
	if(0 != PtzInit(psBizPara->nModMask & MASK_BIZ_MOD_PTZ, &sBizPtz))
	{
		return BIZ_ERR_INIT_PTZ_INIT;
	}
	usleep(100 * 1000);
	
	SBizPtzPara sBizPtzPara;
	memset(&sBizPtzPara, 0, sizeof(sBizPtzPara));
	
	for(i = 0; i < psBizManager->nVideoMainNum; i++)
	{
		ConfigGetPtzPara(0,&sBizPtzPara, i);
		PtzSetPara(i, &sBizPtzPara);
	}
	
	tl_rs485_ctl(1);
#endif
	
#if 0//csp modify 20130407
	//初始化用户管理
	printf("start init user mod......\n");
	
	SBizUserPara sBizUserPara;
	memset(&sBizUserPara, 0, sizeof(sBizUserPara));
	ConfigGetUserPara(0,&sBizUserPara);
	
	if(0 != UserInit(psBizPara->nModMask & MASK_BIZ_MOD_USER, &sBizUserPara))
	{
		return BIZ_ERR_INIT_USER_INIT;
	}
	
	usleep(100 * 1000);
	//printf("OK!\n");
#endif
	
#ifdef TEST_TIMESPAN
	nCurTime = PublicGetTimeStamp();
	printf("$$$$$$$$DoBizInit line:%d last:%u cur:%u span:%u\n", __LINE__, nLastTime, nCurTime, nCurTime - nLastTime);
	nLastTime = nCurTime;
#endif
	
#ifdef ALARM_INIT_CW
	printf(">>>>>> init alarm ******%d\n",__LINE__);//************************
	pthread_t pid_alarm;
	if(0 != pthread_create(&pid_alarm, NULL, (void*)AlarmParaInit, (void*)psBizManager))//why???
	{
		printf("ALARM_INIT_pthread faild,%s,%d\n",__func__,__LINE__);
	}
	sleep(4);
#else
	printf(">>>>>>before init alarm ******%d\n",__LINE__);
	
	//sleep(100);
	
	//初始化报警并设置
	SBizAlarm sBizAlarm;
	sBizAlarm.nAlarmOutNum = psBizManager->nAlarmOutNum;
	sBizAlarm.nAlarmSensorNum = psBizManager->nSensorNum;
	sBizAlarm.nBuzzNum = psBizManager->nBuzzNum;
	sBizAlarm.nVBlindLuma = 0;
	sBizAlarm.nVideoChnNum = psBizManager->nVideoMainNum;
	sBizAlarm.nDiskNum = MAX_HDD_NUM;
	
	if(0 != AlarmInit(psBizPara->nModMask & MASK_BIZ_MOD_ALARM, &sBizAlarm))
	{
		return BIZ_ERR_INIT_ALARM_INIT;
	}
	
	usleep(100 * 1000);
	
#ifdef TEST_TIMESPAN
	nCurTime = PublicGetTimeStamp();
	printf("$$$$$$$$DoBizInit line:%d last:%u cur:%u span:%u\n", __LINE__, nLastTime, nCurTime, nCurTime - nLastTime);
	nLastTime = nCurTime;
#endif
	
	SBizAlarmSensorPara sBizSensorPara;
	SBizAlarmSchedule sBizSch;
	SBizAlarmDispatch sBizDispatch;
	
	for(i = 0; i < sBizAlarm.nAlarmSensorNum; i++)
	{
		ConfigGetAlarmSensorPara(0,&sBizSensorPara, i);
		AlarmSetPara(i, EM_BIZ_SENSORPARA, &sBizSensorPara);
		
		ConfigGetAlarmSensorSchedule(0,&sBizSch, i);
		AlarmSetPara(i, EM_BIZ_SCHEDULE_SENSOR, &sBizSch);
		
		ConfigGetAlarmSensorDispatch(0,&sBizDispatch, i);
		AlarmSetPara(i, EM_BIZ_DISPATCH_SENSOR, &sBizDispatch);
	}
	
	#ifdef TEST_TIMESPAN
	nCurTime = PublicGetTimeStamp();
	printf("$$$$$$$$DoBizInit line:%d last:%u cur:%u span:%u\n", __LINE__, nLastTime, nCurTime, nCurTime - nLastTime);
	nLastTime = nCurTime;
	#endif
	
	SBizAlarmOutPara sBizAlarmOutPara;
	
	for(i = 0; i < sBizAlarm.nAlarmOutNum; i++)
	{
		ConfigGetAlarmOutPara(0,&sBizAlarmOutPara, i);
		AlarmSetPara(i, EM_BIZ_ALARMOUTPARA, &sBizAlarmOutPara);
		
		ConfigGetAlarmOutSchedule(0,&sBizSch, i);
		AlarmSetPara(i, EM_BIZ_SCHEDULE_ALARMOUT, &sBizSch);
	}
	
	#ifdef TEST_TIMESPAN
	nCurTime = PublicGetTimeStamp();
	printf("$$$$$$$$DoBizInit line:%d last:%u cur:%u span:%u\n", __LINE__, nLastTime, nCurTime, nCurTime - nLastTime);
	nLastTime = nCurTime;
	#endif
	
	SBizAlarmBuzzPara sBizBuzzPara;
	for(i = 0; i < sBizAlarm.nBuzzNum; i++)
	{
		ConfigGetAlarmBuzzPara(0,&sBizBuzzPara, i);
		AlarmSetPara(i, EM_BIZ_ALARMBUZZPARA, &sBizBuzzPara);
		
		ConfigGetAlarmBuzzSchedule(0,&sBizSch, i);
		AlarmSetPara(i, EM_BIZ_SCHEDULE_BUZZ, &sBizSch);
	}
	
	#ifdef TEST_TIMESPAN
	nCurTime = PublicGetTimeStamp();
	printf("$$$$$$$$DoBizInit line:%d last:%u cur:%u span:%u\n", __LINE__, nLastTime, nCurTime, nCurTime - nLastTime);
	nLastTime = nCurTime;
	#endif
	
	SBizAlarmVMotionPara sBizVMPara;
	SBizAlarmVBlindPara sBizVBPara;
	SBizAlarmVLostPara sBizVLPara;
	SBizAlarmIPCExtSensorPara sBizIPCExtSensorPara;
	SBizAlarmHDDPara sBizHDDPara;

	//yaogang modify 20141118
	//hdd
	ConfigGetAlarmHDDPara(0,&sBizHDDPara, 0);
	ConfigGetAlarmHDDDispatch(0,&sBizDispatch, 0);

	//一个保存的参数供检测硬盘丢失的8个通道,
	//检测硬盘读写错误的8个通道,还有检测开机无硬盘的单通道
	//这三方共用
	for(i = 0; i < MAX_HDD_NUM; i++)
	{
		AlarmSetPara(i, EM_BIZ_HDDPARA, &sBizHDDPara);
		AlarmSetPara(i, EM_BIZ_DISPATCH_HDD, &sBizDispatch);
	}
	
	for(i = 0; i < sBizAlarm.nVideoChnNum; i++)
	{
		//yaogang modify 20141010
		//IPCCOVER
		ConfigGetAlarmIPCCoverPara(0,&sBizIPCExtSensorPara, i);
		AlarmSetPara(i, EM_BIZ_IPCCOVERPARA, &sBizIPCExtSensorPara);
		//ConfigGetAlarmIPCExtSensorSchedule(0,&sBizSch, i);
		//AlarmSetPara(i, EM_BIZ_SCHEDULE_IPCExtSENSOR, &sBizSch);
		ConfigGetAlarmIPCExtSensorDispatch(0,&sBizDispatch, i);
		AlarmSetPara(i, EM_BIZ_DISPATCH_IPCCOVER, &sBizDispatch);
		
		//IPCExtSensor
		ConfigGetAlarmIPCExtSensorPara(0,&sBizIPCExtSensorPara, i);
		AlarmSetPara(i, EM_BIZ_IPCEXTSENSORPARA, &sBizIPCExtSensorPara);
		ConfigGetAlarmIPCExtSensorSchedule(0,&sBizSch, i);
		AlarmSetPara(i, EM_BIZ_SCHEDULE_IPCEXTSENSOR, &sBizSch);
		ConfigGetAlarmIPCExtSensorDispatch(0,&sBizDispatch, i);
		AlarmSetPara(i, EM_BIZ_DISPATCH_IPCEXTSENSOR, &sBizDispatch);
		
		//485ExtSensor
		ConfigGetAlarm485ExtSensorPara(0,&sBizIPCExtSensorPara, i);
		AlarmSetPara(i, EM_BIZ_485EXTSENSORPARA, &sBizIPCExtSensorPara);
		//ConfigGetAlarmIPCExtSensorSchedule(0,&sBizSch, i);
		//AlarmSetPara(i, EM_BIZ_SCHEDULE_IPCExtSENSOR, &sBizSch);
		ConfigGetAlarm485ExtSensorDispatch(0,&sBizDispatch, i);
		AlarmSetPara(i, EM_BIZ_DISPATCH_485EXTSENSOR, &sBizDispatch);
		
		ConfigGetAlarmVMotionPara(0,&sBizVMPara, i);
		AlarmSetPara(i, EM_BIZ_VMOTIONPARA, &sBizVMPara);
		ConfigGetAlarmVMotionSchedule(0,&sBizSch, i);
		AlarmSetPara(i, EM_BIZ_SCHEDULE_VMOTION, &sBizSch);
		ConfigGetAlarmVMotionDispatch(0,&sBizDispatch, i);
		AlarmSetPara(i, EM_BIZ_DISPATCH_VMOTION, &sBizDispatch);

		ConfigGetAlarmVBlindPara(0,&sBizVBPara, i);
		AlarmSetPara(i, EM_BIZ_VBLINDPARA, &sBizVBPara);

		ConfigGetAlarmVLostPara(0,&sBizVLPara, i);
		AlarmSetPara(i, EM_BIZ_VLOSTPARA, &sBizVLPara);
		ConfigGetAlarmVLostSchedule(0,&sBizSch, i);
		AlarmSetPara(i, EM_BIZ_SCHEDULE_VLOST, &sBizSch);
		ConfigGetAlarmVLostDispatch(0,&sBizDispatch, i);
		AlarmSetPara(i, EM_BIZ_DISPATCH_VLOST, &sBizDispatch);
	}
#endif
	
	#ifdef TEST_TIMESPAN
	nCurTime = PublicGetTimeStamp();
	printf("$$$$$$$$DoBizInit line:%d last:%u cur:%u span:%u\n", __LINE__, nLastTime, nCurTime, nCurTime - nLastTime);
	nLastTime = nCurTime;
	#endif
	
    //初始化网络传输及协议栈并设置;暂不支持
    SNetSysPara sPara;
	strcpy( sPara.device_name, sysPara.strDevName );				//设备名称
	strcpy( sPara.device_mode, sDvrInfo.strModel );					//设备型号
	
	sPara.nChNum 			= sDvrInfo.nVidMainNum;
	
	//csp modify 20121130
	sPara.nLangId			= sysPara.nLangId;
	sPara.nVs    			= sysPara.nVideoStandard;
	
	sPara.nFrameSizeMax 	= sDvrInfo.nMaxMainFrameSize;
	
	sPara.maxChnNum 		= sDvrInfo.nVidMainNum;					//最大通道数
	sPara.maxAduioNum 		= sDvrInfo.nAudNum;						//最大音频数
	sPara.maxPlaybackNum 	= sDvrInfo.nRecNum;						//最大回放数
	sPara.maxAlarmInNum 	= sDvrInfo.nSensorNum;					//最大报警输入数
	sPara.maxAlarmOutNum 	= sDvrInfo.nAlarmoutNum;				//最大报警输出数
	sPara.maxHddNum 		= sDvrInfo.nMaxHddNum;					//最大硬盘数
	sPara.nNVROrDecoder	= sDvrInfo.nNVROrDecoder;	//区别NVR和解码器--- 跃天
	NetInit(hddHdr, &sPara);
	//end net

	//业务层的图片控制逻辑
	//yaogang modify 20150324
	if (sDvrInfo.nNVROrDecoder == 1) //跃天: 1 nvr，2 轮巡解码器，3 切换解码器
	{
		#ifndef NO_MODULE_SG
		if (0 != SnapServerStart(sDvrInfo.nSensorNum, sDvrInfo.nVidMainNum))
		{
			printf("%s SnapServerStart failed\n", __func__);
			return BIZ_ERR_INIT_SNAP_INIT;
		}
		printf("%s SnapServerStart success\n", __func__);
		#endif
	}
    return BIZ_SUCCESS;
}

s32 DoBizDeinit(SBizManager* psBizManager)
{
    return BIZ_SUCCESS;
}

s32 DealCmdWithoutMsg(SBizEventPara* sBizEventPara)
{
	if(NULL == sBizEventPara)
	{
		return -1;
	}
	
	return g_sBizManager.sBizPara.pfnBizEventCb(sBizEventPara);
}

void *BizDispatchFxn(void* arg)
{
    SBizManager* psBizManager = (SBizManager*)arg;
    SBizMsgHeader sBizMsgHead;

    if (NULL == psBizManager)
    {
        PUBPRT("NULL == arg");
        return NULL;
    }
	
	//check thread id
	//printf("BizDispatchFxn id: %d\n", syscall(__NR_gettid));
	
	printf("$$$$$$$$$$$$$$$$$$BizDispatchFxn id:%d\n",getpid());
	
    while(1)
    {
        if(0 == ReadMsgCmd(psBizManager->psMsgCmdHdr, &sBizMsgHead))
        {
            switch(sBizMsgHead.emMsgType)
            {
                case EM_BIZMSG_START_PREVIEW:
					//printf("EM_BIZMSG_START_PREVIEW\n");
                    PreviewStart((SBizPreviewPara*)sBizMsgHead.pData);
                    break;
                case EM_BIZMSG_STOP_PREVIEW:
                    PreviewStop();
                    break;
	         case EM_BIZMSG_CHN_PREVIEW:
                    PreviewZoomChn((u8)(u32)sBizMsgHead.pData);
                    break;
	         case EM_BIZMSG_RESUME_PREVIEW:
                    PreviewResume(0);
                    break;
	         case EM_BIZMSG_NEXT_PREVIEW:
			 		//printf("BizDispatchFxn - EM_BIZMSG_NEXT_PREVIEW - PreviewNext\n");
                    PreviewNext();
                    break;
	         case EM_BIZMSG_LAST_PREVIEW:
                    PreviewLast();
                    break;
	         case EM_BIZMSG_NEXT_PREVIEW_MOD:
                    PreviewNextMod();
                    break;
	         case EM_BIZMSG_LAST_PREVIEW_MOD:
                    PreviewLastMod();
                    break;
	         case EM_BIZMSG_PREVIEW_MUTE:
                    PreviewMute((u8)(u32)sBizMsgHead.pData);
                    break;
	         case EM_BIZMSG_PREVIEW_AUDIOCHN:
                    PreviewSetAudioOut((u8)(u32)sBizMsgHead.pData);
                    break;
	         case EM_BIZMSG_PREVIEW_VOLUME:
                    PreviewVolume((u8)(u32)sBizMsgHead.pData);
                    break;
	         case EM_BIZMSG_START_PATROL:
                    PreviewStartPatrol();
                    break;
	         case EM_BIZMSG_STOP_PATROL:
                    PreviewStopPatrol();
                    break;
            case EM_BIZMSG_SET_PARA:
			printf("yg BizDispatchFxn\n");
	                DoBizSetPara((SBizPara*)sBizMsgHead.pData); 
	                break;
            case EM_BIZMSG_GET_PARA:
	                DoBizGetPara(0,(SBizPara*)sBizMsgHead.pData); 
	                break;
			 case EM_BIZMSG_GET_DEFAULTPARA:
	                DoBizGetPara(1,(SBizPara*)sBizMsgHead.pData); 
	                break;
			case EM_BIZMSG_BACKUP_CONFIG:
	                ConfigBackup((char*)sBizMsgHead.pData); 
	                break;
			case EM_BIZMSG_RESUME_CONFIG:
					ConfigResume((SBizCfgResume*)sBizMsgHead.pData);
					break;
	         case EM_BIZMSG_START_MANUAL_REC:
                    RecordStartManual((u8)(u32)sBizMsgHead.pData);
                    break;
	         case EM_BIZMSG_STOP_MANUAL_REC:
                    RecordStopManual((u8)(u32)sBizMsgHead.pData);
                    break;
	         case EM_BIZMSG_START_PLAYBACK:
                    PlaybackStart((SBizPbStart *)sBizMsgHead.pData); 
                    break;
	         case EM_BIZMSG_PLAYBACK_CTRL:
                    PlayBackControl((SBizPbControl *)sBizMsgHead.pData); 
                    break;
                case EM_BIZMSG_PTZ_CTRL:
                    PtzCtrl((SBizPtzCtrl*)sBizMsgHead.pData);
                    break;
                case EM_BIZMSG_EVENT_CB:
                    psBizManager->sBizPara.pfnBizEventCb((SBizEventPara *)sBizMsgHead.pData);
                    break;
                case EM_BIZMSG_SYSCOMPLEX_EXIT:
                    SysComplexExit((EMBIZSYSEXITTYPE)sBizMsgHead.pData);
					break;
                case EM_BIZMSG_SYSCOMPLEX_UPGRADE:
                    SysComplexUpgrade((SBizUpgrade *)sBizMsgHead.pData);
                    break;
				case EM_BIZMSG_SYSCOMPLEX_UPGRADE_FILELIST:
                    SysComplexUpgradeFilelist((SBizUpgradeFileInfo*)sBizMsgHead.pData);
                    break;
				case EM_BIZMSG_SYSCOMPLEX_MOUNTED_USB_DIR_LIST:
                    SysComplexMountUsbDirlist((SBizUpdateFileInfoList*)sBizMsgHead.pData);
                    break; 
				case EM_BIZMSG_SYSCOMPLEX_CONFIG_FILELIST:
                    SysComplexConfigFilelist((SBizConfigFileList*)sBizMsgHead.pData);
                    break;
                case EM_BIZMSG_SYSCOMPLEX_BACKUP:
                    SysComplexBackup((SBizBackup *)sBizMsgHead.pData);
                    break;
                case EM_BIZMSG_SYSCOMPLEX_BREAK_BACKUP:
                    SysComplexBreakBackup((int)sBizMsgHead.pData);
                    break;
                case EM_BIZMSG_SYSCOMPLEX_DISKMANAGE:
                    SysComplexDiskManage((SBizDiskManage *)sBizMsgHead.pData);
                    break;
                case EM_BIZMSG_SYSCOMPLEX_DISKFORMAT:
                    SysComplexDiskFormat((SBizDiskFormat *)sBizMsgHead.pData);
                    break;
                case EM_BIZMSG_SYSCOMPLEX_DISKSEARCH:
                    SysComplexDiskSearch((SBizDiskSearch *)sBizMsgHead.pData);
                    break;
                case EM_BIZMSG_SYSCOMPLEX_DTSET:
                    SysComplexDTSet((SBizDateTime *)sBizMsgHead.pData);
                    break;
                case EM_BIZMSG_SYSCOMPLEX_DTGET:
                    SysComplexDTGet((SBizDateTime *)sBizMsgHead.pData);
                    break;
                case EM_BIZMSG_SYSCOMPLEX_DTGETSTR:
                    SysComplexDTGetStr((char *)sBizMsgHead.pData);
                    break;
                case EM_BIZMSG_VIDEOMAIN_RECSTATUS:
                    DoBizRecStatusChange(EM_CHN_VIDEO_MAIN, (u16)(u32)sBizMsgHead.pData);
                    break;
                case EM_BIZMSG_STOP_REC:
                    RecordStop((u8)(u32)sBizMsgHead.pData);
                    break;
                case EM_BIZMSG_STOP_ALLREC:
                    RecordStopAll();
                    break;
                case EM_BIZMSG_PAUSE_REC:
                    RecordPause((u8)(u32)sBizMsgHead.pData);
                    break;
                case EM_BIZMSG_RESUME_REC:
                    RecordResume((u8)(u32)sBizMsgHead.pData);
                    break;
                case EM_BIZMSG_ENCDATA:
                    EncDataDispatch((SBizEncodeData*)sBizMsgHead.pData);
                case EM_BIZMSG_GETPARALIST:
                    ConfigGetParaStr((SBizConfigParaList *)sBizMsgHead.pData);
                    break;
                case EM_BIZMSG_GETPARAINDEX:
                    ConfigGetParaListIndex((SBizConfigParaIndex *)sBizMsgHead.pData);
                    break;
                case EM_BIZMSG_GETPARAVALUE:
                    ConfigGetParaListValue((SBizConfigParaValue *)sBizMsgHead.pData);
                    break;
                case EM_BIZMSG_EVENT_DISPATCH:
                    AlarmDispatchEvent((SAlarmCbData*)sBizMsgHead.pData);
                    break;
                
                case EM_BIZMSG_NET_INIT:
                {
					NetCommInit((PSNetCommCfg)sBizMsgHead.pData);
                } break;
				case EM_BIZMSG_NET_REQ_VOIP_STOP:
				case EM_BIZMSG_NET_REQ_VOIP_START:
				case EM_BIZMSG_NET_REQ_PREVIEW_AUDIO_STOP:
				case EM_BIZMSG_NET_REQ_PREVIEW_AUDIO_START:
				case EM_BIZMSG_NET_REQ_PREVIEW_SUB_START:
				case EM_BIZMSG_NET_REQ_PREVIEW_STOP:
				case EM_BIZMSG_NET_REQ_PREVIEW_MAIN_START:
				case EM_BIZMSG_NET_REQ_PREVIEW_SUB_STOP:
				case EM_BIZMSG_NET_REQ_PREVIEW_SUB_MB_START:
				case EM_BIZMSG_NET_REQ_PREVIEW_SUB_MB_STOP:
				//csp modify 20130423
				case EM_BIZMSG_NET_REQ_PREVIEW_THIRD_START:
				case EM_BIZMSG_NET_REQ_PREVIEW_THIRD_STOP:
                {
					DoBizNetReq((STcpStreamReq*)sBizMsgHead.pData);
                } break;
                default:
                    break;
            }
			
			//printf("sBizMsgHead.emMsgType\n", sBizMsgHead.emMsgType);
			
			AckMsgCmd(psBizManager->psMsgCmdHdr);
			
			//printf("2222222222sBizMsgHead.emMsgType\n", sBizMsgHead.emMsgType);
        }
        else
        {
			//csp modify 20121018
			//usleep(0);
			usleep(1);
        }
    }

	pthread_detach(pthread_self());
    return NULL;
}

void *BizDispatchInnerFxn(void* arg)
{
    SBizManager* psBizManager = (SBizManager*)arg;
    SBizMsgHeader sBizMsgHead;
	
    if(NULL == psBizManager)
    {
        PUBPRT("NULL == arg");
        return NULL;
    }
    
	//check thread id
	//printf("BizDispatchInnerFxn id: %d\n", syscall(__NR_gettid));
	
	printf("$$$$$$$$$$$$$$$$$$BizDispatchInnerFxn id:%d\n",getpid());
	
    while(1)
    {
        if(0 == ReadMsgCmd(psBizManager->psMsgCmdInner, &sBizMsgHead))
        {
            switch(sBizMsgHead.emMsgType)
            {
                case EM_BIZMSG_START_PREVIEW:
                    PreviewStart((SBizPreviewPara*)sBizMsgHead.pData);
                    break;
                case EM_BIZMSG_STOP_PREVIEW:
                    PreviewStop();
                    break;
	         case EM_BIZMSG_CHN_PREVIEW:
                    PreviewZoomChn((u8)(u32)sBizMsgHead.pData);
                    break;
	         case EM_BIZMSG_RESUME_PREVIEW:
                    PreviewResume(0);
                    break;
	         case EM_BIZMSG_PREVIEW_MUTE:
                    PreviewMute((u8)(u32)sBizMsgHead.pData);
                    break;
	         case EM_BIZMSG_PREVIEW_AUDIOCHN:
                    PreviewSetAudioOut((u8)(u32)sBizMsgHead.pData);
                    break;
	         case EM_BIZMSG_PREVIEW_VOLUME:
                    PreviewVolume((u8)(u32)sBizMsgHead.pData);
                    break;
	         case EM_BIZMSG_START_PATROL:
                    PreviewStartPatrol();
                    break;
	         case EM_BIZMSG_STOP_PATROL:
                    PreviewStopPatrol();
                    break;
                case EM_BIZMSG_SET_PARA:
			printf("yg BizDispatchInnerFxn\n");
                    DoBizSetPara((SBizPara*)sBizMsgHead.pData); 
                    break;
            case EM_BIZMSG_GET_PARA:
                    DoBizGetPara(0,(SBizPara*)sBizMsgHead.pData); 
                    break;
			case EM_BIZMSG_GET_DEFAULTPARA:
	                DoBizGetPara(1,(SBizPara*)sBizMsgHead.pData); 
	                break;
	         case EM_BIZMSG_START_MANUAL_REC:
                    RecordStartManual((u8)(u32)sBizMsgHead.pData);
                    break;
	         case EM_BIZMSG_STOP_MANUAL_REC:
                    RecordStopManual((u8)(u32)sBizMsgHead.pData);
                    break;
	         case EM_BIZMSG_START_PLAYBACK:
                    PlaybackStart((SBizPbStart *)sBizMsgHead.pData); 
                    break;
	         case EM_BIZMSG_PLAYBACK_CTRL:
                    PlayBackControl((SBizPbControl *)sBizMsgHead.pData); 
                    break;
                case EM_BIZMSG_EVENT_CB:
                    psBizManager->sBizPara.pfnBizEventCb((SBizEventPara *)sBizMsgHead.pData);
                    break;
                case EM_BIZMSG_SYSCOMPLEX_EXIT:
                    SysComplexExit((EMBIZSYSEXITTYPE)sBizMsgHead.pData);
                    break;
                case EM_BIZMSG_SYSCOMPLEX_UPGRADE:
                    SysComplexUpgrade((SBizUpgrade *)sBizMsgHead.pData);
                    break;
				case EM_BIZMSG_SYSCOMPLEX_UPGRADE_FILELIST:
                    SysComplexUpgradeFilelist((SBizUpgradeFileInfo*)sBizMsgHead.pData);
                    break;
				case EM_BIZMSG_SYSCOMPLEX_CONFIG_FILELIST:
                    SysComplexConfigFilelist((SBizConfigFileList*)sBizMsgHead.pData);
                    break;
				case EM_BIZMSG_SYSCOMPLEX_MOUNTED_USB_DIR_LIST:
                    SysComplexMountUsbDirlist((SBizUpdateFileInfoList*)sBizMsgHead.pData);
                    break;   
                case EM_BIZMSG_SYSCOMPLEX_BACKUP:
                    SysComplexBackup((SBizBackup *)sBizMsgHead.pData);
                    break;
                case EM_BIZMSG_SYSCOMPLEX_BREAK_BACKUP:
                    SysComplexBreakBackup((int)sBizMsgHead.pData);
                    break;
                case EM_BIZMSG_SYSCOMPLEX_DISKMANAGE:
                    SysComplexDiskManage((SBizDiskManage *)sBizMsgHead.pData);
                    break;
                case EM_BIZMSG_SYSCOMPLEX_DISKFORMAT:
                    SysComplexDiskFormat((SBizDiskFormat *)sBizMsgHead.pData);
                    break;
                case EM_BIZMSG_SYSCOMPLEX_DISKSEARCH:
                    SysComplexDiskSearch((SBizDiskSearch *)sBizMsgHead.pData);
                    break;
                case EM_BIZMSG_SYSCOMPLEX_DTSET:
                    SysComplexDTSet((SBizDateTime *)sBizMsgHead.pData);
                    break;
                case EM_BIZMSG_SYSCOMPLEX_DTGET:
                    SysComplexDTGet((SBizDateTime *)sBizMsgHead.pData);
                    break;
                case EM_BIZMSG_SYSCOMPLEX_DTGETSTR:
                    SysComplexDTGetStr((char *)sBizMsgHead.pData);
                    break;
                case EM_BIZMSG_VIDEOMAIN_RECSTATUS:
                    DoBizRecStatusChange(EM_CHN_VIDEO_MAIN, (u16)(u32)sBizMsgHead.pData);
                    break;
                case EM_BIZMSG_STOP_REC:
                    RecordStop((u8)(u32)sBizMsgHead.pData);
                    break;
                case EM_BIZMSG_STOP_ALLREC:
                    RecordStopAll();
                    break;
                case EM_BIZMSG_PAUSE_REC:
                    RecordPause((u8)(u32)sBizMsgHead.pData);
                    break;
                case EM_BIZMSG_RESUME_REC:
                    RecordResume((u8)(u32)sBizMsgHead.pData);
                    break;
                case EM_BIZMSG_ENCDATA:
                    EncDataDispatch((SBizEncodeData*)sBizMsgHead.pData);
                    break;
                case EM_BIZMSG_EVENT_DISPATCH:
                    AlarmDispatchEvent((SAlarmCbData*)sBizMsgHead.pData);
                    break;
                case EM_BIZMSG_NET_INIT:
                {
					NetCommInit((PSNetCommCfg)sBizMsgHead.pData);
                } break;
				case EM_BIZMSG_NET_REQ_VOIP_STOP:
				case EM_BIZMSG_NET_REQ_VOIP_START:
				case EM_BIZMSG_NET_REQ_PREVIEW_AUDIO_STOP:
				case EM_BIZMSG_NET_REQ_PREVIEW_AUDIO_START:
				case EM_BIZMSG_NET_REQ_PREVIEW_SUB_START:
				case EM_BIZMSG_NET_REQ_PREVIEW_STOP:
				case EM_BIZMSG_NET_REQ_PREVIEW_MAIN_START:
				case EM_BIZMSG_NET_REQ_PREVIEW_SUB_STOP:
				case EM_BIZMSG_NET_REQ_PREVIEW_SUB_MB_START:
				case EM_BIZMSG_NET_REQ_PREVIEW_SUB_MB_STOP:
				//csp modify 20130423
				case EM_BIZMSG_NET_REQ_PREVIEW_THIRD_START:
				case EM_BIZMSG_NET_REQ_PREVIEW_THIRD_STOP:
                {
					DoBizNetReq((STcpStreamReq*)sBizMsgHead.pData);					
                } break;
                default:
                    break;
            }
            
            AckMsgCmd(psBizManager->psMsgCmdInner);
        }
        else
        {
        	//csp modify 20121018
            //usleep(0);
            usleep(1);
        }
    }

	pthread_detach(pthread_self());
	
    return NULL;
}

s32 DoBizSetPara(SBizPara* psBizPara)
{
	if(NULL == psBizPara)
	{
	    return -1;
	}
	
	//int ret = -2;//csp modify
	
	switch(psBizPara->sBizParaTarget.emBizParaType)
	{
		case EM_BIZ_SYSTEMPARA:
			return ConfigSetSystemPara((SBizSystemPara *)psBizPara->pData);
		case EM_BIZ_SYSTEMMAINTAIN:
			return ConfigSetMainTainPara((SBizMainTainPara *)psBizPara->pData);
		case EM_BIZ_PREVIEWPARA:
			return ConfigSetCfgPreview((SBizCfgPreview *)psBizPara->pData);
		case EM_BIZ_PATROLPARA:
			return ConfigSetCfgPatrol((SBizCfgPatrol *)psBizPara->pData);
		case EM_BIZ_IMAGEPARA:
			return ConfigSetPreviewImagePara((SBizPreviewImagePara *)psBizPara->pData, psBizPara->sBizParaTarget.nChn);
		case EM_BIZ_VO_IMAGEPARA:
			return ConfigSetPlayBackImagePara((SBizPreviewImagePara *)psBizPara->pData, psBizPara->sBizParaTarget.nChn);
		case EM_BIZ_STROSD:
			//printf("yg DoBizSetPara chn%d\n", psBizPara->sBizParaTarget.nChn);
			return ConfigSetCfgStrOsd((SBizCfgStrOsd *)psBizPara->pData, psBizPara->sBizParaTarget.nChn);
		case EM_BIZ_RECTOSd:
			return ConfigSetCfgRectOsd((SBizCfgRectOsd *)psBizPara->pData, psBizPara->sBizParaTarget.nChn);
		case EM_BIZ_ENCMAINPARA:
			return ConfigSetVMainEncodePara((SBizEncodePara *)psBizPara->pData, psBizPara->sBizParaTarget.nChn);
		case EM_BIZ_ENCSUBPARA:
			return ConfigSetVSubEncodePara((SBizEncodePara *)psBizPara->pData, psBizPara->sBizParaTarget.nChn);
		case EM_BIZ_ENCMOBPARA:
			return ConfigSetVMobEncodePara((SBizEncodePara *)psBizPara->pData, psBizPara->sBizParaTarget.nChn);
		case EM_BIZ_RECPARA:
			return ConfigSetRecPara((SBizRecPara *)psBizPara->pData, psBizPara->sBizParaTarget.nChn);
		case EM_BIZ_SENSORPARA:
			//printf("chn %d, name %s \n",psBizPara->sBizParaTarget.nChn,((SBizAlarmSensorPara *)psBizPara->pData)->name);
			return ConfigSetAlarmSensorPara((SBizAlarmSensorPara *)psBizPara->pData, psBizPara->sBizParaTarget.nChn);
		//yaogang modify 20141010
		case EM_BIZ_IPCCOVERPARA:
			return ConfigSetAlarmIPCCoverPara((SBizAlarmIPCExtSensorPara *)psBizPara->pData, psBizPara->sBizParaTarget.nChn);
		
		case EM_BIZ_IPCEXTSENSORPARA:
			return ConfigSetAlarmIPCExtSensorPara((SBizAlarmIPCExtSensorPara *)psBizPara->pData, psBizPara->sBizParaTarget.nChn);
		case EM_BIZ_485EXTSENSORPARA:
			return ConfigSetAlarm485ExtSensorPara((SBizAlarmIPCExtSensorPara *)psBizPara->pData, psBizPara->sBizParaTarget.nChn);
		case EM_BIZ_HDDPARA:
			return ConfigSetAlarmHDDPara((SBizAlarmHDDPara *)psBizPara->pData, psBizPara->sBizParaTarget.nChn);
			
		case EM_BIZ_VMOTIONPARA:
			return ConfigSetAlarmVMotionPara((SBizAlarmVMotionPara *)psBizPara->pData, psBizPara->sBizParaTarget.nChn);
		case EM_BIZ_VLOSTPARA:
			return ConfigSetAlarmVLostPara((SBizAlarmVLostPara *)psBizPara->pData, psBizPara->sBizParaTarget.nChn);
		case EM_BIZ_VBLINDPARA:
			return ConfigSetAlarmVBlindPara((SBizAlarmVBlindPara *)psBizPara->pData, psBizPara->sBizParaTarget.nChn);
		case EM_BIZ_ALARMOUTPARA:
			return ConfigSetAlarmOutPara((SBizAlarmOutPara *)psBizPara->pData, psBizPara->sBizParaTarget.nChn);
		case EM_BIZ_ALARMBUZZPARA:
			return ConfigSetAlarmBuzzPara((SBizAlarmBuzzPara *)psBizPara->pData, psBizPara->sBizParaTarget.nChn);
		//一般报警事件触发参数设置
		case EM_BIZ_DISPATCH_SENSOR:
			return ConfigSetAlarmSensorDispatch((SBizAlarmDispatch *)psBizPara->pData, psBizPara->sBizParaTarget.nChn);
		case EM_BIZ_DISPATCH_IPCCOVER:
			return ConfigSetAlarmIPCCoverDispatch((SBizAlarmDispatch *)psBizPara->pData, psBizPara->sBizParaTarget.nChn);
		
		case EM_BIZ_DISPATCH_IPCEXTSENSOR:
			return ConfigSetAlarmIPCExtSensorDispatch((SBizAlarmDispatch *)psBizPara->pData, psBizPara->sBizParaTarget.nChn);
		case EM_BIZ_DISPATCH_485EXTSENSOR:
			return ConfigSetAlarm485ExtSensorDispatch((SBizAlarmDispatch *)psBizPara->pData, psBizPara->sBizParaTarget.nChn);
		case EM_BIZ_DISPATCH_HDD:
			return ConfigSetAlarmHDDDispatch((SBizAlarmDispatch *)psBizPara->pData, psBizPara->sBizParaTarget.nChn);
			
		case EM_BIZ_DISPATCH_VMOTION:
			return ConfigSetAlarmVMotionDispatch((SBizAlarmDispatch *)psBizPara->pData, psBizPara->sBizParaTarget.nChn);
		case EM_BIZ_DISPATCH_VBLIND:
			return ConfigSetAlarmVBlindDispatch((SBizAlarmDispatch *)psBizPara->pData, psBizPara->sBizParaTarget.nChn);
		case EM_BIZ_DISPATCH_VLOST:
			return ConfigSetAlarmVLostDispatch((SBizAlarmDispatch *)psBizPara->pData, psBizPara->sBizParaTarget.nChn);
		//一般报警事件布防设置
		case EM_BIZ_SCHEDULE_SENSOR:
			return ConfigSetAlarmSensorSchedule((SBizAlarmSchedule *)psBizPara->pData, psBizPara->sBizParaTarget.nChn);
		case EM_BIZ_SCHEDULE_IPCEXTSENSOR:
			return ConfigSetAlarmIPCExtSensorSchedule((SBizAlarmSchedule *)psBizPara->pData, psBizPara->sBizParaTarget.nChn);
		case EM_BIZ_SCHEDULE_VMOTION:
			return ConfigSetAlarmVMotionSchedule((SBizAlarmSchedule *)psBizPara->pData, psBizPara->sBizParaTarget.nChn);
		case EM_BIZ_SCHEDULE_VBLIND:
			return ConfigSetAlarmVBlindSchedule((SBizAlarmSchedule *)psBizPara->pData, psBizPara->sBizParaTarget.nChn);
		case EM_BIZ_SCHEDULE_VLOST:
			return ConfigSetAlarmVLostSchedule((SBizAlarmSchedule *)psBizPara->pData, psBizPara->sBizParaTarget.nChn);
		//报警输出事件布防设置
		case EM_BIZ_SCHEDULE_ALARMOUT:
			return ConfigSetAlarmOutSchedule((SBizAlarmSchedule *)psBizPara->pData, psBizPara->sBizParaTarget.nChn);
		case EM_BIZ_SCHEDULE_BUZZ:
			return ConfigSetAlarmBuzzSchedule((SBizAlarmSchedule *)psBizPara->pData, psBizPara->sBizParaTarget.nChn);
		//NVR used
		case EM_BIZ_IPCAMERAPARA:
			return ConfigSetIPCameraPara((SBizIPCameraPara *)psBizPara->pData, psBizPara->sBizParaTarget.nChn);
		//录像布防
		case EM_BIZ_SCHEDULE_RECTIMING:
			return ConfigSetSchRecSchedule((SBizSchedule *)psBizPara->pData, psBizPara->sBizParaTarget.nChn);
		case EM_BIZ_SCHEDULE_RECSENSOR:
			return ConfigSetSensorAlaRecSchedule((SBizSchedule *)psBizPara->pData, psBizPara->sBizParaTarget.nChn);
		case EM_BIZ_SCHEDULE_RECMOTION:
			return ConfigSetVMotionAlaRecSchedule((SBizSchedule *)psBizPara->pData, psBizPara->sBizParaTarget.nChn);
		//云台参数
		case EM_BIZ_PTZPARA:
			return ConfigSetPtzPara((SBizPtzPara *)psBizPara->pData, psBizPara->sBizParaTarget.nChn);
		//用户
		case EM_BIZ_USER_ADDUSER:
			if(0 != UserAddUser((SBizUserUserPara*)psBizPara->pData))
			{
				return -1;
			}
			return UserUpdatePara();
		case EM_BIZ_USER_MODIFYUSER:
			if(0 != UserModifyUser((SBizUserUserPara *)psBizPara->pData))
			{
				return -1;
			}
			return UserUpdatePara();
		case EM_BIZ_USER_DELETEUSER:
			if(0 != UserDeleteUser((s8 *)psBizPara->pData))
			{
				return -1;
			}
			return UserUpdatePara();
		case EM_BIZ_NETPARA:
			return ConfigSetNetPara((SBizNetPara *)psBizPara->pData, psBizPara->sBizParaTarget.nChn);
		//yaogang modify 20141209 深广平台参数
		case EM_BIZ_SGPARA:
			//printf("chn %d, name %s \n",psBizPara->sBizParaTarget.nChn,((SBizAlarmSensorPara *)psBizPara->pData)->name);
			return ConfigSetSGPara((SBizSGParam *)psBizPara->pData);
		case EM_BIZ_SGDAILYPARA:
			return ConfigSetSGDailyPara((SBizDailyPicCFG *)psBizPara->pData);
		case EM_BIZ_SGALARMPARA:
			return ConfigSetSGAlarmPara((SBizAlarmPicCFG *)psBizPara->pData, psBizPara->sBizParaTarget.nChn);	
		case EM_BIZ_SNAP_PARA:
			return ConfigSetSnapChnPara((SBizSnapChnPara *)psBizPara->pData, psBizPara->sBizParaTarget.nChn);
		default:
			break;
	}   
	
	return -2;
}

s32 DoBizGetPara(u32 bDefault,SBizPara* psBizPara)
{
	if(NULL == psBizPara)
	{
	    return -1;
	}
	
	int ret = -2;
	
	//printf("DoBizGetPara  emtype %d !!!!!!!!!!!!\n", psBizPara->sBizParaTarget.emBizParaType );
	
	switch(psBizPara->sBizParaTarget.emBizParaType)
	{
		case EM_BIZ_DVRINFO:
			return ConfigGetDvrInfo(bDefault,(SBizDvrInfo *)psBizPara->pData);
		case EM_BIZ_SYSTEMPARA:
			return ConfigGetSystemPara(bDefault,(SBizSystemPara *)psBizPara->pData);
		case EM_BIZ_SYSTEMMAINTAIN:
			return ConfigGetMainTainPara(bDefault,(SBizMainTainPara *)psBizPara->pData);
		case EM_BIZ_PREVIEWPARA:
			return ConfigGetCfgPreview(bDefault,(SBizCfgPreview *)psBizPara->pData);
		case EM_BIZ_PATROLPARA:
			return ConfigGetCfgPatrol(bDefault,(SBizCfgPatrol *)psBizPara->pData);
		case EM_BIZ_IMAGEPARA:
			return ConfigGetPreviewImagePara(bDefault,(SBizPreviewImagePara *)psBizPara->pData, psBizPara->sBizParaTarget.nChn);
		case EM_BIZ_VO_IMAGEPARA:
			return ConfigGetPlayBackImagePara(bDefault,(SBizPreviewImagePara *)psBizPara->pData, psBizPara->sBizParaTarget.nChn);
		case EM_BIZ_STROSD:
			return ConfigGetCfgStrOsd(bDefault,(SBizCfgStrOsd *)psBizPara->pData, psBizPara->sBizParaTarget.nChn);
		case EM_BIZ_RECTOSd:
			return ConfigGetCfgRectOsd(bDefault,(SBizCfgRectOsd *)psBizPara->pData, psBizPara->sBizParaTarget.nChn);
		case EM_BIZ_ENCMAINPARA:
			return ConfigGetVMainEncodePara(bDefault,(SBizEncodePara *)psBizPara->pData, psBizPara->sBizParaTarget.nChn);
		case EM_BIZ_ENCSUBPARA:
			return ConfigGetVSubEncodePara(bDefault,(SBizEncodePara *)psBizPara->pData, psBizPara->sBizParaTarget.nChn);
		case EM_BIZ_ENCMOBPARA:
			return ConfigGetVMobEncodePara(bDefault,(SBizEncodePara *)psBizPara->pData, psBizPara->sBizParaTarget.nChn);
		case EM_BIZ_RECPARA:
			return ConfigGetRecPara(bDefault,(SBizRecPara *)psBizPara->pData, psBizPara->sBizParaTarget.nChn);
		case EM_BIZ_SENSORPARA:
			return ConfigGetAlarmSensorPara(bDefault,(SBizAlarmSensorPara *)psBizPara->pData, psBizPara->sBizParaTarget.nChn);
		//yaogang modify 20141010
		case EM_BIZ_IPCCOVERPARA:
			return ConfigGetAlarmIPCCoverPara(bDefault,(SBizAlarmIPCExtSensorPara *)psBizPara->pData, psBizPara->sBizParaTarget.nChn);
		case EM_BIZ_IPCEXTSENSORPARA:
			return ConfigGetAlarmIPCExtSensorPara(bDefault,(SBizAlarmIPCExtSensorPara *)psBizPara->pData, psBizPara->sBizParaTarget.nChn);
		case EM_BIZ_485EXTSENSORPARA:
			return ConfigGetAlarm485ExtSensorPara(bDefault,(SBizAlarmIPCExtSensorPara *)psBizPara->pData, psBizPara->sBizParaTarget.nChn);
		case EM_BIZ_HDDPARA:
			return ConfigGetAlarmHDDPara(bDefault,(SBizAlarmIPCExtSensorPara *)psBizPara->pData, psBizPara->sBizParaTarget.nChn);
		
		case EM_BIZ_VMOTIONPARA:
			return ConfigGetAlarmVMotionPara(bDefault,(SBizAlarmVMotionPara *)psBizPara->pData, psBizPara->sBizParaTarget.nChn);
		case EM_BIZ_VLOSTPARA:
			return ConfigGetAlarmVLostPara(bDefault,(SBizAlarmVLostPara *)psBizPara->pData, psBizPara->sBizParaTarget.nChn);
		case EM_BIZ_VBLINDPARA:
			return ConfigGetAlarmVBlindPara(bDefault,(SBizAlarmVBlindPara *)psBizPara->pData, psBizPara->sBizParaTarget.nChn);
		case EM_BIZ_ALARMOUTPARA:
			return ConfigGetAlarmOutPara(bDefault,(SBizAlarmOutPara *)psBizPara->pData, psBizPara->sBizParaTarget.nChn);
		case EM_BIZ_ALARMBUZZPARA:
			return ConfigGetAlarmBuzzPara(bDefault,(SBizAlarmBuzzPara *)psBizPara->pData, psBizPara->sBizParaTarget.nChn);
		//一般报警事件触发参数设置
		case EM_BIZ_DISPATCH_SENSOR:
			return ConfigGetAlarmSensorDispatch(bDefault,(SBizAlarmDispatch *)psBizPara->pData, psBizPara->sBizParaTarget.nChn);
		//yaogang modify 20141010
		case EM_BIZ_DISPATCH_IPCCOVER:
			return ConfigGetAlarmIPCCoverDispatch(bDefault,(SBizAlarmDispatch *)psBizPara->pData, psBizPara->sBizParaTarget.nChn);
		case EM_BIZ_DISPATCH_IPCEXTSENSOR:
			return ConfigGetAlarmIPCExtSensorDispatch(bDefault,(SBizAlarmDispatch *)psBizPara->pData, psBizPara->sBizParaTarget.nChn);
		case EM_BIZ_DISPATCH_485EXTSENSOR:
			return ConfigGetAlarm485ExtSensorDispatch(bDefault,(SBizAlarmDispatch *)psBizPara->pData, psBizPara->sBizParaTarget.nChn);
		case EM_BIZ_DISPATCH_HDD:
			return ConfigGetAlarmHDDDispatch(bDefault,(SBizAlarmDispatch *)psBizPara->pData, psBizPara->sBizParaTarget.nChn);
			
		case EM_BIZ_DISPATCH_VMOTION:
			return ConfigGetAlarmVMotionDispatch(bDefault,(SBizAlarmDispatch *)psBizPara->pData, psBizPara->sBizParaTarget.nChn);
		case EM_BIZ_DISPATCH_VBLIND:
			return ConfigGetAlarmVBlindDispatch(bDefault,(SBizAlarmDispatch *)psBizPara->pData, psBizPara->sBizParaTarget.nChn);
		case EM_BIZ_DISPATCH_VLOST:
			return ConfigGetAlarmVLostDispatch(bDefault,(SBizAlarmDispatch *)psBizPara->pData, psBizPara->sBizParaTarget.nChn);
		//一般报警事件布防设置
		case EM_BIZ_SCHEDULE_SENSOR:
			return ConfigGetAlarmSensorSchedule(bDefault,(SBizAlarmSchedule *)psBizPara->pData, psBizPara->sBizParaTarget.nChn);
		case EM_BIZ_SCHEDULE_IPCEXTSENSOR:
			return ConfigGetAlarmIPCExtSensorSchedule(bDefault,(SBizAlarmSchedule *)psBizPara->pData, psBizPara->sBizParaTarget.nChn);
		case EM_BIZ_SCHEDULE_VMOTION:
			return ConfigGetAlarmVMotionSchedule(bDefault,(SBizAlarmSchedule *)psBizPara->pData, psBizPara->sBizParaTarget.nChn);
		case EM_BIZ_SCHEDULE_VBLIND:
			return ConfigGetAlarmVBlindSchedule(bDefault,(SBizAlarmSchedule *)psBizPara->pData, psBizPara->sBizParaTarget.nChn);
		case EM_BIZ_SCHEDULE_VLOST:
			return ConfigGetAlarmVLostSchedule(bDefault,(SBizAlarmSchedule *)psBizPara->pData, psBizPara->sBizParaTarget.nChn);
		//报警输出事件布防设置
		case EM_BIZ_SCHEDULE_ALARMOUT:
			return ConfigGetAlarmOutSchedule(bDefault,(SBizAlarmSchedule *)psBizPara->pData, psBizPara->sBizParaTarget.nChn);
		case EM_BIZ_SCHEDULE_BUZZ:
			return ConfigGetAlarmBuzzSchedule(bDefault,(SBizAlarmSchedule *)psBizPara->pData, psBizPara->sBizParaTarget.nChn);
		//NVR used
		case EM_BIZ_IPCAMERAPARA:
			return ConfigGetIPCameraPara(bDefault, (SBizIPCameraPara *)psBizPara->pData, psBizPara->sBizParaTarget.nChn);
		case EM_BIZ_SNAP_PARA:
			return ConfigGetSnapChnPara(bDefault, (SBizSnapChnPara *)psBizPara->pData, psBizPara->sBizParaTarget.nChn);
		//录像布防
		case EM_BIZ_SCHEDULE_RECTIMING:
			return ConfigGetSchRecSchedule(bDefault,(SBizSchedule *)psBizPara->pData, psBizPara->sBizParaTarget.nChn);
		case EM_BIZ_SCHEDULE_RECSENSOR:
			return ConfigGetSensorAlaRecSchedule(bDefault,(SBizSchedule *)psBizPara->pData, psBizPara->sBizParaTarget.nChn);
		case EM_BIZ_SCHEDULE_RECMOTION:
			return ConfigGetVMotionAlaRecSchedule(bDefault,(SBizSchedule *)psBizPara->pData, psBizPara->sBizParaTarget.nChn);
		//云台参数
		case EM_BIZ_PTZPARA:
			return ConfigGetPtzPara(bDefault,(SBizPtzPara *)psBizPara->pData, psBizPara->sBizParaTarget.nChn);
		case EM_BIZ_USERPARA:
			return ConfigGetUserPara(bDefault,(SBizUserPara *)psBizPara->pData);
		//用户
		case EM_BIZ_USER_GETUSERLIST:
			return UserGetListUserRegisted((SBizUserListPara *)psBizPara->pData);
		case EM_BIZ_USER_GETUSERINFO:
			return UserGetUserInfo((SBizUserUserPara *)psBizPara->pData);
		case EM_BIZ_USER_GETGROUPLIST:
			return UserGetGroupList((SBizUserListPara *)psBizPara->pData);
		case EM_BIZ_USER_GETGROUPINFO:
			return UserGetGroupInfo((SBizUserGroupPara *)psBizPara->pData);
		case EM_BIZ_NETPARA:
			return ConfigGetNetPara(bDefault, (SBizNetPara *)psBizPara->pData, psBizPara->sBizParaTarget.nChn);
		//yaogang modify 20141209 深广平台参数
		case EM_BIZ_SGPARA:
			return ConfigGetSGPara(bDefault, (SBizSGParam *)psBizPara->pData);
		case EM_BIZ_SGDAILYPARA:
			return ConfigGetSGDailyPara(bDefault, (SBizDailyPicCFG *)psBizPara->pData);
		case EM_BIZ_SGALARMPARA:
			return ConfigGetSGAlarmPara(bDefault, (SBizAlarmPicCFG *)psBizPara->pData, psBizPara->sBizParaTarget.nChn);	
		default:
			break;
	}
    
    return ret;
}

s32 SendBizEvent(SBizEventPara* psBizEventPara)
{
    if (NULL == psBizEventPara)
    {
        return BIZ_ERR_EVENT_NULL;
    }
	
    return DealCmdWithoutMsg(psBizEventPara);
    //return SendBizInnerMsgCmd(EM_BIZMSG_EVENT_CB, (void *)psBizEventPara);
}

s32 SendBizEvent2(SBizEventPara* psBizEventPara)
{
    if (NULL == psBizEventPara)
    {
        return BIZ_ERR_EVENT_NULL;
    }

    return SendBizMsgCmd(EM_BIZMSG_EVENT_CB, (void *)psBizEventPara);
}

s32 SendBizDispatch(SAlarmCbData* psBizEventPara)
{
    if (NULL == psBizEventPara)
    {
        return BIZ_ERR_EVENT_NULL;
    }

    return SendBizMsgCmd(EM_BIZMSG_EVENT_DISPATCH, (void *)psBizEventPara);
}

s32 SendBizInnerMsgCmd(EMBIZMSGTYPE emBizMsgType, void* pData)
{
    SBizManager* psBizManager = &g_sBizManager;
    SBizMsgHeader sBizMsgHeader;
	
    memset(&sBizMsgHeader, 0, sizeof(SBizMsgHeader));
    sBizMsgHeader.emMsgType = emBizMsgType;
    sBizMsgHeader.pData = pData;
	
    if (0 == WriteMsgCmd(psBizManager->psMsgCmdInner, &sBizMsgHeader))
    {
        return BIZ_SUCCESS;
    }
	
    return BIZ_ERR_INNERMSG_FAILEDCMD; 
}

s32 SendBizMsgCmd(EMBIZMSGTYPE emBizMsgType, void* pData)
{
    SBizManager* psBizManager = &g_sBizManager;
    SBizMsgHeader sBizMsgHeader;
	
    memset(&sBizMsgHeader, 0, sizeof(SBizMsgHeader));
    sBizMsgHeader.emMsgType = emBizMsgType;
    sBizMsgHeader.pData = pData;
	
    if(0 == WriteMsgCmd(psBizManager->psMsgCmdHdr, &sBizMsgHeader))
    {
        return BIZ_SUCCESS;
    }
	
    return BIZ_ERR_INNERMSG_FAILEDCMD; 
}

EMRECENCRESOL ModParaResolEncToRec(EMVIDEORESOLUTION encResol)
{
	SBizSystemPara sBizSysPara;
	ConfigGetSystemPara(0, &sBizSysPara);
	
	EMRECENCRESOL recResol = (EM_BIZ_NTSC == sBizSysPara.nVideoStandard) ? EM_REC_ENC_N_CIF : EM_REC_ENC_CIF;
	
	if(EM_BIZ_NTSC == sBizSysPara.nVideoStandard)
	{
		switch(encResol)
		{
			case EM_VIDEO_4CIF:
			case EM_VIDEO_D1:
				recResol = EM_REC_ENC_N_D1;
				break;
			case EM_VIDEO_HALF_D1:
				recResol = EM_REC_ENC_N_HALF_D1;
				break;	
			case EM_VIDEO_QCIF:
				recResol = EM_REC_ENC_N_QCIF;
				break;
			case EM_VIDEO_720P:
				recResol = EM_REC_ENC_720P;
				break;
			case EM_VIDEO_1080P:
				recResol = EM_REC_ENC_1080P;
				break;
			case EM_VIDEO_960H:
				recResol = EM_REC_ENC_N_960H;
				break;
			default:
				recResol = EM_REC_ENC_N_CIF;
				break;
		}
	}
	else
	{
		switch(encResol)
		{
			case EM_VIDEO_4CIF:
			case EM_VIDEO_D1:
				recResol = EM_REC_ENC_D1;
				break;
			case EM_VIDEO_HALF_D1:
				recResol = EM_REC_ENC_HALF_D1;
				break;	
			case EM_VIDEO_QCIF:
				recResol = EM_REC_ENC_QCIF;
				break;
			case EM_VIDEO_720P:
				recResol = EM_REC_ENC_720P;
				break;
			case EM_VIDEO_1080P:
				recResol = EM_REC_ENC_1080P;
				break;
			case EM_VIDEO_960H:
				recResol = EM_REC_ENC_960H;
				break;
			default:
				recResol = EM_REC_ENC_CIF;
				break;
		}
	}
	
	#if 0//csp modify
	SBizDvrInfo psPara;
	ConfigGetDvrInfo(0,&psPara);
	
	if((EM_BIZ_NTSC == sBizSysPara.nVideoStandard)
		&& (0 != strcasecmp(psPara.sproductnumber, "R3104HD"))
		&& (0 != strcasecmp(psPara.sproductnumber, "R3110HDW"))
		&& (0 != strcasecmp(psPara.sproductnumber, "R3106HDW")))
	{
		recResol += (EM_REC_ENC_N_CIF - EM_REC_ENC_CIF);
	}
	#endif
	
	return recResol;
}

u8 GetRecEnable(u8 nId)
{
	SBizRecPara sBizRecPara;
	
	ConfigGetRecPara(0, &sBizRecPara, nId);
	
	return (u8)sBizRecPara.nEnable;
}

//csp modify
int GetRecAudioEnable(u8 nId)
{
	SBizRecPara sBizRecPara;
	
	ConfigGetRecPara(0, &sBizRecPara, nId);
	
	return (u8)sBizRecPara.nStreamType;
}

u8 GetRecStreamType(u8 nId)
{
	SBizRecPara sBizRecPara;
	
	ConfigGetRecPara(0, &sBizRecPara, nId);
	
	return (u8)sBizRecPara.nStreamType;
}

s32 DoBizNetReq(STcpStreamReq* param)
{
	//SBizManager* psBizManager = &g_sBizManager;//csp modify
	u8 nChn = param->nChn;
	//u8 nStatus;// = (nChnStatus & 0xff);//csp modify
	
	u8 nTgt = 1;
	
	switch(param->nType)
	{
		case EM_PREQ_MAX:
		{
			if(param->nAct==EM_STOP)
			{
				printf("chn%d request stop main stream@@@\n",nChn);
            	VEncMainControl(nChn, 0, nTgt);
            	//VEncSubControl(nChn, 0, nTgt);
			}
		} break;
		case EM_PREQ_MAIN:
		{
			if(param->nAct)
			{
				printf("chn%d request stop main stream###\n",nChn);
            	VEncMainControl(nChn, 0, nTgt);
        	}
			else
			{
				printf("chn%d request start main stream\n",nChn);
            	VEncMainControl(nChn, 1, nTgt);
			}
		} break;
		case EM_PREQ_THIRD://csp modify 20130423
		{
			if(param->nAct)
			{
				printf("chn%d request stop third stream\n",nChn);
				//csp modify 20140525
				//EncodeRequestStop(nChn, EM_CHN_VIDEO_THIRD);
				VEncThirdControl(nChn, 0, nTgt);
			}
			else
			{
				printf("chn%d request start third stream\n",nChn);
				//csp modify 20140525
				//EncodeRequestStart(nChn, EM_CHN_VIDEO_THIRD);
				VEncThirdControl(nChn, 1, nTgt);
			}
		} break;
		case EM_PREQ_SUB:
		{
			if(param->nAct)
			{
				printf("chn%d request stop sub stream\n",nChn);
            	VEncSubControl(nChn, 0, nTgt);
        	}
			else
			{
				printf("chn%d request start sub stream\n",nChn);
            	VEncSubControl(nChn, 1, nTgt);
			}
		} break;
		case EM_PREQ_SUB_MB:
		{
			if(param->nAct)
			{
				printf("chn%d request stop sub stream for mobile\n",nChn);
				//csp modify 20140525
            	//VEncSubControl(nChn, 0, 0);
				VEncThirdControl(nChn, 0, 0);
        	}
			else
			{
				printf("chn%d request start sub stream for mobile\n",nChn);
				//csp modify 20140525
            	//VEncSubControl(nChn, 1, 0);
				VEncThirdControl(nChn, 1, 0);
			}
		} break;
		case EM_PREQ_AUDIO:
		{
			if(param->nAct)
			{
				printf("chip audio stop.........................................................\n");
				AEncControl(nChn, 0, nTgt);
        	}
			else
			{
				printf("chip audio start.........................................................\n");
				AEncControl(nChn, 1, nTgt);
			}
		} break;
		case EM_PREQ_VOIP:
		{
			if(param->nAct)
			{
				TalkControl(0);
        	}
			else
			{
				TalkControl(1);
			}
		} break;
		default:
		{
			printf("stop %d chn type %d preview act %d!\n", nChn, param->nType, param->nAct);
		} break;
	}
	
	return 0;//csp modify
}

s32 DoBizRecStatusChange(EMCHNTYPE emChntype, u16 nChnStatus)
{
	//SBizManager* psBizManager = &g_sBizManager;//csp modify
	u8 nChn = (nChnStatus >> 8);
	u8 nStatus = (nChnStatus & 0xff);
	//u8 flag = 0;
	switch(emChntype)
	{
		case EM_CHN_VIDEO_MAIN:
			//if(GetRecEnable(nChn))
			{
				VEncMainControl(nChn, nStatus, 0);
				//flag = 1;
			}
			//if(GetRecStreamType(nChn))
			{
				AEncControl(nChn, nStatus, 0);
				//flag = 1;
			}
			//if(0 == flag)
			{
				//BizRecordPause(nChn);
			}
		break;
		case EM_CHN_VIDEO_SUB:
			break;
		default:
			return -1;
	}
	
	return 0;
}

//csp modify 20121225
//static int* g_nFlagRecVideoAudio = NULL;

s32 VEncMainControl(u8 nChn, u8 nStatus, u8 nTarget)//0 for rec; 1 for net
{
	SBizManager* psBizManager = &g_sBizManager;
	u8 nCurEncStatus, nLastEncStatus;
	int ret = 0;

	//printf("%s nChn%d, nStatus=%d, nTarget: %d\n", __func__, nChn, nStatus, nTarget);
	//csp modify 20121225
	//if(g_nFlagRecVideoAudio == NULL)
	//{
	//	g_nFlagRecVideoAudio = (int*)calloc(sizeof(int), psBizManager->nVideoMainNum);
	//}
	
	if(nChn < psBizManager->nVideoMainNum)
	{
		nLastEncStatus = psBizManager->pnVideoMainRecStatus[nChn] | psBizManager->pnVideoMainNetStatus[nChn];
		
		if(0 == nTarget)
		{
			#if 0//csp modify
			if(!GetRecEnable(nChn))
			{
				nStatus = 0;
			}
			#endif
			
			nCurEncStatus = nStatus | psBizManager->pnVideoMainNetStatus[nChn];
			
			if(psBizManager->pnVideoMainRecStatus[nChn] != nStatus)
			{
				if(psBizManager->sBizPara.pfnBizEventCb)
				{
					SBizEventPara sBizEventPara;
					
					sBizEventPara.emType = EM_BIZ_EVENT_RECORD;
					sBizEventPara.sBizRecStatus.nChn = nChn;
					sBizEventPara.sBizRecStatus.nLastStatus = psBizManager->pnVideoMainRecStatus[nChn];
					sBizEventPara.sBizRecStatus.nCurStatus = nStatus;
					
					ret = psBizManager->sBizPara.pfnBizEventCb(&sBizEventPara);//改变通道图标(报警，录像...)
					g_nFlagRecVideoAudio[nChn] = ret;
				}
				psBizManager->pnVideoMainRecStatus[nChn] = nStatus;
			}
		}
		else
		{
			nCurEncStatus = nStatus | psBizManager->pnVideoMainRecStatus[nChn];
			//printf("VEncMainControl net req:(%d,%d)\n",nStatus,psBizManager->pnVideoMainNetStatus[nChn]);
			
			if(psBizManager->pnVideoMainNetStatus[nChn] != nStatus)
			{
				/*if (psBizManager->sBizPara.pfnBizEventCb)
				{
					SBizEventPara sBizEventPara;
					
					sBizEventPara.emType = EM_BIZ_EVENT_RECORD;
					sBizEventPara.sBizRecStatus.nChn = nChn;
					sBizEventPara.sBizRecStatus.nLastStatus = psBizManager->pnVideoMainRecStatus[nChn];
					sBizEventPara.sBizRecStatus.nCurStatus = nStatus;
					
					psBizManager->sBizPara.pfnBizEventCb(&sBizEventPara);
				}*/
				psBizManager->pnVideoMainNetStatus[nChn] = nStatus;
				//printf("VEncMainControl net req change!\n");
			}
		}
		if((ret == 617) || (ret == 618))
		{
			printf("ret = 617 or 618\n");
			return 0;
		}
		else
		{
			//printf("%s nLastEncStatus: 0x%x, nCurEncStatus: 0x%x\n", __func__, nLastEncStatus, nCurEncStatus);			
			ret = EncodeControl(nChn, EM_CHN_VIDEO_MAIN, nLastEncStatus, nCurEncStatus);
			//printf("EncodeControl ret: %d\n", ret);
			return ret;
		}
	}
	
	return -1;
}

s32 VEncSubControl(u8 nChn, u8 nStatus, u8 nTarget) //0 for mobile; 1 for net
{
	SBizManager* psBizManager = &g_sBizManager;
	u8 nCurEncStatus, nLastEncStatus;
	
	if(nChn < psBizManager->nVideoSubnum)
	{
		nLastEncStatus = psBizManager->pnVideoSubNetStatus[nChn] | psBizManager->pnVideoSubNetMbStatus[nChn];
		
		if(1 == nTarget)
		{
			nCurEncStatus = nStatus | psBizManager->pnVideoSubNetMbStatus[nChn];
			
			if(psBizManager->pnVideoSubNetStatus[nChn] != nStatus)
			{
				psBizManager->pnVideoSubNetStatus[nChn] = nStatus;
			}
		}
		else
		{
			nCurEncStatus = nStatus | psBizManager->pnVideoSubNetStatus[nChn];
			
			if(psBizManager->pnVideoSubNetMbStatus[nChn] != nStatus)
			{
				psBizManager->pnVideoSubNetMbStatus[nChn] = nStatus;
			}
		}
		
		return EncodeControl(nChn, EM_CHN_VIDEO_SUB, nLastEncStatus, nCurEncStatus);
	}
	
	return -1;
}

//csp modify 20140525
s32 VEncThirdControl(u8 nChn, u8 nStatus, u8 nTarget) //0 for mobile; 1 for net
{
	SBizManager* psBizManager = &g_sBizManager;
	u8 nCurEncStatus, nLastEncStatus;

	printf("%s nChn%d nStatus: %d, nTarget: %d\n", __func__, nChn, nStatus, nTarget);
	if(nChn < psBizManager->nVideoMainNum)
	{
		nLastEncStatus = psBizManager->pnVideoThirdNetStatus[nChn] | psBizManager->pnVideoThirdNetMbStatus[nChn];
		
		if(1 == nTarget)
		{
			nCurEncStatus = nStatus | psBizManager->pnVideoThirdNetMbStatus[nChn];
			
			if(psBizManager->pnVideoThirdNetStatus[nChn] != nStatus)
			{
				psBizManager->pnVideoThirdNetStatus[nChn] = nStatus;
			}
		}
		else
		{
			nCurEncStatus = nStatus | psBizManager->pnVideoThirdNetStatus[nChn];
			
			if(psBizManager->pnVideoThirdNetMbStatus[nChn] != nStatus)
			{
				psBizManager->pnVideoThirdNetMbStatus[nChn] = nStatus;
			}
		}
		
		if(nCurEncStatus)
		{
			return EncodeRequestStart(nChn, EM_CHN_VIDEO_THIRD);
		}
		else
		{
			return EncodeRequestStop(nChn, EM_CHN_VIDEO_THIRD);
		}
	}
	
	return -1;
}

s32 TalkControl(u8 nStatus)
{
	SBizManager* psBizManager = &g_sBizManager;
	u8 nCurEncStatus, nLastEncStatus;
	
	nLastEncStatus = psBizManager->pnTalkStatus[0];
	
	if(nLastEncStatus!=nStatus)
	{
		if(0==ModPreviewVoip(nStatus))
		{
			psBizManager->pnTalkStatus[0] = nStatus;
		}
		else
		{
			return -1;
		}
		
		s32 ret = 0;
		nCurEncStatus = nStatus;
		ret |= EncodeControl(0, EM_CHN_TALK, nLastEncStatus, nCurEncStatus);
		ret |= BizPlayBackSetModeVOIP(nStatus);
		
		return ret;
	}
	
	return -1;
}

s32 AEncControl(u8 nChn, u8 nStatus, u8 nTgt)
{
	SBizManager* psBizManager = &g_sBizManager;
	u8 nCurEncStatus, nLastEncStatus;
	
	//csp modify 20121225
	//if(g_nFlagRecVideoAudio == NULL)
	//{
	//	g_nFlagRecVideoAudio = (int*)calloc(sizeof(int), psBizManager->nVideoMainNum);
	//}
	
	if(nChn < psBizManager->nAudioNum)
	{
		nLastEncStatus = psBizManager->pnAudioRecStatus[nChn] | psBizManager->pnAudioNetStatus[nChn];
		
		if(nTgt==0)//rec
		{
			#if 1//csp modify
			if(!GetRecEnable(nChn) || !GetRecAudioEnable(nChn))
			{
				nStatus = 0;
			}
			#endif
			
			if(psBizManager->pnAudioRecStatus[nChn] != nStatus)
			{
				#if 1//csp modify
				psBizManager->pnAudioRecStatus[nChn] = nStatus;
				#else
				u8 nMultiStream = 1;
				if(0 == nMultiStream)
				{
					nStatus = 0;
				}
				
				psBizManager->pnAudioRecStatus[nChn] = nStatus;
				#endif
			}
			
			nCurEncStatus = psBizManager->pnAudioRecStatus[nChn] | psBizManager->pnAudioNetStatus[nChn];
		}
		else
		{
			if(psBizManager->pnAudioNetStatus[nChn] != nStatus)
			{
				psBizManager->pnAudioNetStatus[nChn] = nStatus;
			}
			nCurEncStatus = nStatus | psBizManager->pnAudioRecStatus[nChn];
		}
		
		if((nTgt == 0) && ((g_nFlagRecVideoAudio[nChn] == 617) || (g_nFlagRecVideoAudio[nChn] == 619)))
		{
			//printf("AEncControl g_nFlagRecVideoAudio[%d]=%d\n",nChn,g_nFlagRecVideoAudio[nChn]);
			return 0;
		}
		else
		{
			//printf("AEncControl EM_CHN_AUDIO......\n");
			return EncodeControl(nChn, EM_CHN_AUDIO, nLastEncStatus, nCurEncStatus);
		}
	}
	
	return -1;
}

s32 EncodeControl(u8 nChn, EMCHNTYPE emChnType, u8 nLastEncStatus, u8 nCurEncStatus)
{
	int rtn = 0;

	//printf("EncodeControl nChn=%d emChnType=%d nLastEncStatus=0x%x nCurEncStatus=0x%x\n",nChn,emChnType,nLastEncStatus,nCurEncStatus);
	
	if((0 == nLastEncStatus) && nCurEncStatus)
	{
		//printf("%s 1\n", __func__);
		//csp modify 20131123
		
		rtn = EncodeRequestStart(nChn, emChnType);
		
		// 设置编码参数期望立刻生成I帧
		//SEncodeParam sEncPara;
		if(nCurEncStatus)
		{
			SBizEncodePara 	sEncPara;
			SVideoParam		sSetPara;
			
			//printf("EncodeControl nChn=%d emChnType=%d nLastEncStatus=%d nCurEncStatus=%d\n",nChn,emChnType,nLastEncStatus,nCurEncStatus);
			
			switch(emChnType)
			{
			case EM_CHN_VIDEO_MAIN:
				{
					ConfigGetVMainEncodePara(0, &sEncPara, nChn);
					
					sSetPara.nBitRate		=	sEncPara.nBitRate;
					sSetPara.nFrameRate		=	sEncPara.nFrameRate;
					sSetPara.nGop			=	sEncPara.nGop;
					sSetPara.emBitRateType	=	sEncPara.nBitRateType;
					sSetPara.emPicLevel		=	sEncPara.nPicLevel;
					ModEncodeGetKeyFrameEx(nChn, &sSetPara, 0);
				}
				break;
			case EM_CHN_VIDEO_SUB:
				{
					ConfigGetVSubEncodePara(0, &sEncPara, nChn);
					
					sSetPara.nBitRate		=	sEncPara.nBitRate;
					sSetPara.nFrameRate		=	sEncPara.nFrameRate;
					sSetPara.nGop			=	sEncPara.nGop;
					sSetPara.emBitRateType	=	sEncPara.nBitRateType;
					sSetPara.emPicLevel		=	sEncPara.nPicLevel;
					ModEncodeGetKeyFrameEx(nChn, &sSetPara, 1);
				}
				break;
			default:
				break;
			}
		}
		
		//csp modify 20131123
		//return EncodeRequestStart(nChn, emChnType);
		return rtn;
	}
	else if((0 == nCurEncStatus) && nLastEncStatus)
	{
		//printf("%s 2\n", __func__);
		return EncodeRequestStop(nChn, emChnType);
	}
	//printf("%s 3\n", __func__);
	return 0;
}

void GetWideHeightByResol( 
	u8 nVs,
	u8 nVideoReso, 
	u16* pnWidth, 
	u16* pnHeight
)
{
	switch(nVideoReso)
	{
		case	EM_VIDEO_CIF:// = 0,		//PAL:352x288, NTSC:352x240
		{
			*pnWidth = 352;
			*pnHeight = nVs?288:240;
		}break;			//Cif
		case	EM_VIDEO_QCIF:// = 1,		//PAL:176x144, NTSC:176x112
		{
			*pnWidth = 176;
			*pnHeight = nVs?144:120;	//144:112;//csp modify
		}break;			//Cif
		case	EM_VIDEO_4CIF:// = 1,		//PAL:176x144, NTSC:176x112
		{
			*pnWidth = 352;
			*pnHeight = nVs?288:240;
		}break;			//Cif
		case	EM_VIDEO_D1:// = 2,			//PAL:704x576, NTSC:704x480
		{
			*pnWidth = 704;
			*pnHeight = nVs?576:480;
		}break;			//Cif
		case	EM_VIDEO_HALF_D1://=3   //PAL:704x288, NTSC:704x240
		{
			*pnWidth = 704;
			*pnHeight = nVs?288:240;
		}break;			//Cif	
		case	EM_VIDEO_720P:
		{
			*pnWidth = 1280;
			*pnHeight = 720;
		}break;
		case	EM_VIDEO_1080P:
		{
			*pnWidth = 1920;
			*pnHeight = 1080;
		}break;
		case	EM_VIDEO_960H:
		{
			*pnWidth = 960;
			*pnHeight = nVs?576:480;//576;//csp modify 20130617
		}break;
	}
	
	//printf("resolution %d\n", nVideoReso);
	//printf("get width %d height %d\n", *pnWidth, *pnHeight);
}

int debug_write(char* buf, int size)
{
	static FILE* fh = NULL;
	
	if(!fh)
	{
		fh = fopen("/root/rec/debug.ifv","wb");
	}
	
	if(fh)
	{
	#if 0
		static int cnr = 0;
		if(cnr++>1000)
		{
			fclose(fh);
			fh = NULL;
			
			cnr = 0;//csp modify
		}
		else
	#endif
		{
			fwrite(buf, 1, size, fh);
		}
	}
	
	return 0;//csp modify
}

//static void CalcFrameRate( u8 bVideo );//csp modify

//#define PRINT_SND_TIME

#ifdef PRINT_SND_TIME
static struct timeval start,end;
static long span;
#endif

s32 EncDataDispatch(SBizEncodeData* psBizEncData)
{
	SBizManager* psBizManager = &g_sBizManager;
	//SModRecRecordHeader sRecHead;//csp modify
	SEncodeHead* psEncHead = NULL;
	u8 nChn, nStatusRec = 0, nStatusNet = 0, nStatusTalk = 0;
	
	SNetComStmHead sNetFrmHead;
	memset(&sNetFrmHead, 0, sizeof(sNetFrmHead));

	//printf("%s 1\n", __func__);
	if(psBizEncData)
	{
		nChn = psBizEncData->nChn;
		psEncHead = psBizEncData->psEncodeHead;
		switch(psBizEncData->emChnType)
		{
		case EM_CHN_VIDEO_MAIN:
			if(nChn < psBizManager->nVideoMainNum)
			{
				if(psBizManager->pnVideoMainRecStatus[nChn])
				{
					nStatusRec = 1;
				}
				
				if(psBizManager->pnVideoMainNetStatus[nChn])
				{
					nStatusNet = 1;
					sNetFrmHead.bSub = 0;
					sNetFrmHead.eFrameType = EM_VIDEO;
					
					//printf("main net enc stream...\n");
				}
			}
			break;
		case EM_CHN_VIDEO_THIRD://csp modify 20130423
			if(nChn < psBizManager->nVideoMainNum)
			{
			#if 1//csp modify 20140525
				nStatusNet = 0;
				sNetFrmHead.bSub = 0;
				sNetFrmHead.eFrameType = EM_THIRD;
				
				if(psBizManager->pnVideoThirdNetStatus[nChn])
				{
					nStatusNet |= 1;
					
					sNetFrmHead.bSub |= 0x2;
				}
				
				if(psBizManager->pnVideoThirdNetMbStatus[nChn])
				{
					nStatusNet |= 1;
					
					sNetFrmHead.bSub |= 1;
				}
			#else
				nStatusNet = 1;
				sNetFrmHead.bSub = 2;//0;//csp modify 20140318
				sNetFrmHead.eFrameType = EM_THIRD;
			#endif
			}
			break;
		case EM_CHN_VIDEO_SUB:
			//printf("EncDataDispatch:chn%d EM_CHN_VIDEO_SUB\n",nChn);
			if(nChn < psBizManager->nVideoSubnum)
			{
				nStatusNet = 0;
				sNetFrmHead.bSub = 0;
				sNetFrmHead.eFrameType = EM_VIDEO;
				
				if(psBizManager->pnVideoSubNetStatus[nChn])
				{
					nStatusNet |= 1;
					
					sNetFrmHead.bSub |= 0x2;
				}
				
				//csp modify 20140525
				//if(psBizManager->pnVideoSubNetMbStatus[nChn])
				//{
				//	nStatusNet |= 1;
				//	
				//	sNetFrmHead.bSub |= 1;
				//}
				
				/*
				struct timeval tv;
				gettimeofday(&tv,NULL);
				printf("source  : %lu \n", (u32)psEncHead->nTimeStamp);
				printf("current : %lu \n", (u32)(tv.tv_sec*1000+tv.tv_usec/1000));
				//printf("after2 %lu\n", (u32)(tv.tv_sec*1000+tv.tv_usec/1000)-psEncHead->nTimeStamp);
				*/
			}
			break;
		case EM_CHN_AUDIO:
			if(nChn < psBizManager->nAudioNum)
			{
				if(psBizManager->pnAudioRecStatus[nChn])
				{
					nStatusRec = 1;
				}
				
				if(psBizManager->pnAudioNetStatus[nChn])
				{
					nStatusNet = 1;
					sNetFrmHead.eFrameType = EM_AUDIO;
					//printf("audio net enc stream...\n");
				}
			}
			break;
		case EM_CHN_TALK:
			//printf("talk net enc stream... ch %d\n");
			if(nChn < 1)//psBizManager->nAudioNum)
			{
				if(psBizManager->pnTalkStatus[0])
				{
					nStatusTalk = 1;
					sNetFrmHead.eFrameType = EM_TALK;
					
					//printf("talk net enc stream...\n");
				}
			}
			break;
		default:
			break;
		}
		
		/*
		struct timeval tv;
		gettimeofday(&tv,NULL);
		printf("before %llu\n", (u64)tv.tv_sec*1000000+tv.tv_usec);
		*/
		//printf("%s 2\n", __func__);
		if(nStatusRec)
		{
			SModRecRecordHeader sRecHead;
			
			sRecHead.emResolution = ModParaResolEncToRec(psEncHead->emResol);
			sRecHead.nBitRate = psEncHead->nBitRate;
			sRecHead.nChn = nChn;
			sRecHead.nDataLength = psEncHead->nLen;
			sRecHead.nDate = psEncHead->pData;
			sRecHead.nFrameType = psEncHead->emVideoFrameType * 3;
			sRecHead.nMediaType = (EM_ENCODE_VIDEO_H264 == psEncHead->emEncodeType ? 0 : 10);
			sRecHead.nPts = psEncHead->nTimeStamp * 1000;
			sRecHead.nStreamType = GetRecStreamType(nChn);
			sRecHead.nTimeStamp = psEncHead->nTimeStamp;
			if(psEncHead->width == 0 || psEncHead->height == 0)
			{
				sRecHead.width = 0;
				sRecHead.height = 0;
			}
			else
			{
				sRecHead.width = psEncHead->width;
				sRecHead.height = psEncHead->height;
			}
			
			//if((!nChn) )//&& !sRecHead.nMediaType)
			//{
			//    printf("nChn=%d, nMediaType=%d, stamp=%llu, nDataLength=%u\n", nChn, sRecHead.nMediaType, sRecHead.nTimeStamp, sRecHead.nDataLength);
			//}
			
			if((0 == GetRecEnable(nChn)))// && (0 == GetRecStreamType(nChn)))
			{
				//yzw add
			}
			#if 1//csp modify
			else if((0 == sRecHead.nMediaType))//H264
			{
				//printf("%s 1 RecordWriteOneFrame\n", __func__);
				RecordWriteOneFrame(nChn, &sRecHead);
			}
			else if((0 != sRecHead.nStreamType) && GetRecAudioEnable(nChn))//AUDIO
			{
				RecordWriteOneFrame(nChn, &sRecHead);
			}
			#else
			else if((0 != sRecHead.nStreamType) || (0 == sRecHead.nMediaType))
			{
				RecordWriteOneFrame(nChn, &sRecHead);
			}
			#endif
		}
		
		//gettimeofday(&tv,NULL);
		//printf("after1 %llu\n", (u64)tv.tv_sec*1000000+tv.tv_usec);
		//printf("%s 3\n", __func__);
		
		if(nStatusTalk)
		{
			sNetFrmHead.byChnIndex 	= nChn;
			
			sNetFrmHead.dwlen 		= psEncHead->nLen;
			sNetFrmHead.timeStamp	= PublicGetTimeStamp();//(u32)psEncHead->nTimeStamp;
			
			//debug_write(psEncHead->pData, psEncHead->nLen);
			
			sNetFrmHead.byMediaType = psEncHead->emEncodeType;
			//printf("psEncHead->emEncodeType %d\n", psEncHead->emEncodeType);
			sNetFrmHead.nAudioMode = AUDIO_SAMPLE_BITWIDTH;
			
			NetCommSendPreviewFrame(&sNetFrmHead, psEncHead->pData);
		}
		
		/*
		struct timeval tv;
		gettimeofday(&tv,NULL);
		printf("after2 %lu\n", (u32)(tv.tv_sec*1000+tv.tv_usec/1000)-psEncHead->nTimeStamp);
		*/
		//printf("%s 4\n", __func__);
		if(nStatusNet)
		{
			//static u32 nVStamp = 0;
			//static u32 nAStamp = 0;
			
			sNetFrmHead.byChnIndex 	= nChn;
			
			sNetFrmHead.dwlen 		= psEncHead->nLen;
			sNetFrmHead.timeStamp	= (u32)psEncHead->nTimeStamp;

			//yaogang modify 20150401 
			/*
			跃天IPC私有协议I帧会超过500K，录像和回放修改为640K
			网传要限制，IE和客户端的缓冲只有500K
			*/
			if (psEncHead->nLen > 500*1024)
			{
				printf("Exception: %s chn: %d type: %d, len%d > 500*1024",
					__func__, nChn, psBizEncData->emChnType, psEncHead->nLen);
				return 0;
			}
			
			if(EM_VIDEO == sNetFrmHead.eFrameType)
			{
				//CalcFrameRate(1);
				
				sNetFrmHead.byMediaType = psEncHead->emEncodeType;
				sNetFrmHead.byFrameType = (psEncHead->emVideoFrameType == EM_FRAME_TYPE_I?3:0);//key?1:0;
				u16 nWidth = 0, nHeight = 0;
				if(psEncHead->width == 0 || psEncHead->height == 0)
				{
					GetWideHeightByResol(IsPAL(), psEncHead->emResol, &nWidth, &nHeight);
					//printf("EncDataDispatch:chn%d Width=%d Height=%d\n",nChn,nWidth,nHeight);
				}
				else
				{
					nWidth = psEncHead->width;
					nHeight = psEncHead->height;
				}
				sNetFrmHead.nWidth 		=  nWidth;
				sNetFrmHead.nHeight 	=  nHeight;
			}
			else if(EM_THIRD == sNetFrmHead.eFrameType)//csp modify 20130423
			{
				sNetFrmHead.byMediaType = psEncHead->emEncodeType;
				sNetFrmHead.byFrameType = (psEncHead->emVideoFrameType == EM_FRAME_TYPE_I?3:0);//key?1:0;
				u16 nWidth = 0, nHeight = 0;
				if(psEncHead->width == 0 || psEncHead->height == 0)
				{
					GetWideHeightByResol(IsPAL(), psEncHead->emResol, &nWidth, &nHeight);
				}
				else
				{
					nWidth = psEncHead->width;
					nHeight = psEncHead->height;
				}
				sNetFrmHead.nWidth 		=  nWidth;
				sNetFrmHead.nHeight 	=  nHeight;
			}
			else
			{
				//CalcFrameRate(0);
				
				sNetFrmHead.byMediaType = psEncHead->emEncodeType;
				sNetFrmHead.nAudioMode = AUDIO_SAMPLE_BITWIDTH;
			}
			
			NetCommSendPreviewFrame(&sNetFrmHead, psEncHead->pData);
			
			//if(nChn==0)
			//	debug_write(psEncHead->pData, psEncHead->nLen);		
		}
		
		//gettimeofday(&tv,NULL);
		//printf("after3 %llu\n\n", (u64)tv.tv_sec*1000000+tv.tv_usec);        
	}
	//printf("%s 5\n", __func__);
	return -1;
}

/*
static u64 g_FlagZoomChn = 0;
void AlarmZoomChnPolling(void)
{
	
}

s32 AddAlarmZoomChn(u8 chn)
{
	static u8 flag = 1;
	if(flag)
	{
		pthread_t pid;
		if(0 == pthread_create(&pid, NULL, (void*)AlarmZoomChnPolling, NULL))
		{
			flag = 0;
		}
	}
	if(chn < 64)
	{
		
	}
}
*/

static u8 g_ZoomMode_flag; //cw_zoom
static int nIsDwellStart = 0;
void BizSetZoomMode(u8 flag)
{	
	g_ZoomMode_flag = flag;
}

u8 BizGetZoomMode()
{
	return g_ZoomMode_flag;
}

void BizSetDwellStartFlag(u8 flag)
{
	nIsDwellStart = flag;
}

u8 BizGetDwellStartFlag()
{
	return nIsDwellStart;
}

static PREVIEWMODECB g_PrevicwModeCB = NULL;//cw_preview

void SetPrevicwModeFuncCB(PREVIEWMODECB func)
{
	g_PrevicwModeCB = func;
}

s32 AlarmDispatchEvent(SAlarmCbData* psAlarmCbData)
{
	static u32 nZoomCount = 0;
	static u8 nCurTourId = -1;
	static u8 IsZoomStatue = 0;
	
	if(psAlarmCbData)
	{
		//printf("emAlarmEvent = %d\n",psAlarmCbData->emAlarmEvent);
        switch(psAlarmCbData->emAlarmEvent)
        {
			case EM_ALARM_EVENT_DISPATCH_ZOOMCHN: //触发通道放大事件;
				//printf("AlarmDispatchEvent EM_ALARM_EVENT_DISPATCH_ZOOMCHN-1,chn%d alarm\n",psAlarmCbData->nChn);
				if(0 != BizGetZoomMode())//cw_zoom
				{
					//printf("zoom mode,break\n");
					break;
				}
				
				//csp modify
				if(!ModPreviewIsOpened() || 
					(psAlarmCbData->nData != 0xff && EM_PREVIEW_1SPLIT == ModPreviewGetMode()) || 
					(psAlarmCbData->nData == 0xff && EM_PREVIEW_1SPLIT != ModPreviewGetMode()))
				{
					//printf("play mode or EM_PREVIEW_1SPLIT,break\n");
					break;
				}
				
				//printf("AlarmDispatchEvent EM_ALARM_EVENT_DISPATCH_ZOOMCHN-2\n");
				
				if(psAlarmCbData->nData != 0xff)
				{
					//printf("AlarmDispatchEvent EM_ALARM_EVENT_DISPATCH_ZOOMCHN-3\n");
					//nZoomCount++;
					nZoomCount = 1;
					if(BizGetDwellStartFlag())
					{
						BizStopPatrol();
						IsZoomStatue = 1;
					}
					if(g_PrevicwModeCB != NULL)
					{
						g_PrevicwModeCB(EM_BIZPREVIEW_1SPLIT);
					}
					//printf("AlarmDispatchEvent EM_ALARM_EVENT_DISPATCH_ZOOMCHN-4,chn%d zoom\n",psAlarmCbData->nData);
					return PreviewZoomChn(psAlarmCbData->nData);//psAlarmCbData->nChn
				}
				else
				{
					//printf("AlarmDispatchEvent EM_ALARM_EVENT_DISPATCH_ZOOMCHN-5\n");
					if(nZoomCount)
					{
						nZoomCount--;
					}
					if(nZoomCount == 0)
					{
						if(BizGetDwellStartFlag())
						{
							//printf("AlarmDispatchEvent EM_ALARM_EVENT_DISPATCH_ZOOMCHN-6\n");
							BizStartPatrol();
							return 0;
						}
						else
						{
							//printf("AlarmDispatchEvent EM_ALARM_EVENT_DISPATCH_ZOOMCHN-7\n");
							return PreviewResume(0);
						}
					}
				}
            case EM_ALARM_EVENT_DISPATCH_EMAIL: //触发email事件;暂不支持
            case EM_ALARM_EVENT_DISPATCH_SNAP: //触发抓图事件;暂不支持
                break;
            case EM_ALARM_EVENT_DISPATCH_REC_SENSOR: //传感器触发录像事件
                return RecordSetTrigger(psAlarmCbData->nChn, EM_RECALARM_V_ALARM, psAlarmCbData->nData);
				
	case EM_ALARM_EVENT_DISPATCH_REC_IPCEXTSENSOR: //传感器触发录像事件
                return RecordSetTrigger(psAlarmCbData->nChn, EM_RECALARM_V_IPCEXTALARM, psAlarmCbData->nData);
	case EM_ALARM_EVENT_DISPATCH_REC_IPCCOVER: //传感器触发录像事件
                return RecordSetTrigger(psAlarmCbData->nChn, EM_RECALARM_V_BIND, psAlarmCbData->nData);
				
            case EM_ALARM_EVENT_DISPATCH_REC_VMOTION: //传感器触发录像事件
                return RecordSetTrigger(psAlarmCbData->nChn, EM_RECALARM_V_MOTION, psAlarmCbData->nData);
            case EM_ALARM_EVENT_DISPATCH_REC_VBLIND: //传感器触发录像事件
                return 0;//RecordSetTrigger(psAlarmCbData->nChn, EM_RECALARM_V_BIND, psAlarmCbData->nData);
            case EM_ALARM_EVENT_DISPATCH_REC_VLOST: //传感器触发录像事件
                return 0;//RecordSetTrigger(psAlarmCbData->nChn, EM_RECALARM_V_LOST, psAlarmCbData->nData);
            case EM_ALARM_EVENT_DISPATCH_PTZ_PRESET: //触发云台联动预置点事件
            {
                if (psAlarmCbData->nData > 0
                	 && psAlarmCbData->nData <= TOURPIONT_MAX
                	 && 0xff != psAlarmCbData->nData) //开
                {
                    SBizPtzCtrl sBizPtzCtrl;
                    sBizPtzCtrl.emCmd = EM_BIZPTZ_CMD_PRESET_GOTO;
                    sBizPtzCtrl.nChn = psAlarmCbData->nChn;
                    sBizPtzCtrl.nId = psAlarmCbData->nData;

					return PtzCtrl(&sBizPtzCtrl);
                }
                break;
            }
            case EM_ALARM_EVENT_DISPATCH_PTZ_PATROL: //触发云台联动巡航事件
            {
                SBizPtzCtrl sBizPtzCtrl;
                if ((0xff == psAlarmCbData->nData) && ((u8)-1 != nCurTourId)) //关
                {
					sBizPtzCtrl.emCmd = EM_BIZPTZ_CMD_STOP_TOUR;
					sBizPtzCtrl.nChn = psAlarmCbData->nChn;
					sBizPtzCtrl.nId = nCurTourId;
					nCurTourId = -1;
                }
                else if (psAlarmCbData->nData > 0
                		 && psAlarmCbData->nData <= TOURPATH_MAX)
                {
                    sBizPtzCtrl.emCmd = EM_BIZPTZ_CMD_START_TOUR;
                    sBizPtzCtrl.nChn = psAlarmCbData->nChn;
                    sBizPtzCtrl.nId = psAlarmCbData->nData - 1; //巡航线要-1
					nCurTourId = sBizPtzCtrl.nId;
                }
				else
				{
					break;
				}

				return PtzCtrl(&sBizPtzCtrl);
            }
            case EM_ALARM_EVENT_DISPATCH_PTZ_LOCUS: //触发云台联动轨迹事件
            {
                SBizPtzCtrl sBizPtzCtrl;
                if (0xff == psAlarmCbData->nData) //关
                {
					sBizPtzCtrl.emCmd = EM_BIZPTZ_CMD_STOP_PATTERN;
					sBizPtzCtrl.nChn = psAlarmCbData->nChn;
                    sBizPtzCtrl.nId = 0;//psAlarmCbData->nData;
                }
                else
                {                   
                    sBizPtzCtrl.emCmd = EM_BIZPTZ_CMD_START_PATTERN;
                    sBizPtzCtrl.nChn = psAlarmCbData->nChn;
                    sBizPtzCtrl.nId = 0;//psAlarmCbData->nData; 
                }
				
                return PtzCtrl(&sBizPtzCtrl);
            }
            default:
                break;
        }
    }

    return  -1;
}

#ifdef TEST_INIT
typedef enum hiVOU_DEV_E
{
	HD = 0,
	AD = 1,
	SD = 2,
	VOU_DEV_BUTT,
} VOU_DEV_E;

typedef enum hiVOU_GLAYER_E
{
	G0 = 0,
	G1 = 1,
	G2 = 2,
	G3 = 3,
	HC = 4,
	VOU_GLAYER_BUTT,
}VOU_GLAYER_E;

#if 0
BOOL setOsdTransparency(int nfd,u8 trans)
{
	int fd;
	BOOL bShow = TRUE;
	HIFB_ALPHA_S stAlpha;
	fd = nfd;
	
    if(fd == -1)
    {
		printf("Failed to open fb window \n");

		return FALSE;
    }

	if(trans)
    {
		bShow = FALSE;

		if(ioctl(fd, FBIOGET_SHOW_HIFB, &bShow) == -1)
		{
			printf("Error reading fb show information.\n");
			close(fd);

			return FALSE;
		}
		else
		{
			bShow = !bShow;
		}

		if(bShow)
		{
			if(ioctl(fd, FBIOPUT_SHOW_HIFB, &bShow) == -1)
			{
				printf("Error reading variable information.\n");

 				close(fd);

				return FALSE;
			}
		}

		stAlpha.bAlphaEnable = HI_TRUE;

		stAlpha.bAlphaChannel = HI_FALSE;

		stAlpha.u8Alpha0 = 0x0;

		stAlpha.u8Alpha1 = trans;

		if (ioctl(fd, FBIOPUT_ALPHA_HIFB,  &stAlpha) < 0)
		{
			printf("Set alpha failed!\n");

			close(fd);

			return -1;
		}
	}
	else
	{
		bShow = FALSE;
	}

	return TRUE;
}
#endif

int initStateOSD()
{
	int fd;
	
	//fb4 begin************
	fd = open("/dev/fb4", O_RDWR);
	if(fd == -1)
	{
		//perror("Failed to open fb4 device");
		//return FALSE;
	}
	else
	{
		close(fd);
	}
	//fb4 end**********
	
	//fb1 begin************
	fd = open("/dev/fb1", O_RDWR);
	if(fd == -1)
	{
		//perror("Failed to open fb1 device");
		//return FALSE;
	}
	else
	{
		close(fd);
	}
	//fb1 end**********
	
	//fb0 begin************
	fd = open("/dev/fb0", O_RDWR);
	if(fd == -1)
	{
		//printf("Failed to open fb0 device\n");
		//return FALSE;
	}
	else
	{
		close(fd);
	}
	//fb0 end**********
	
	//fb2 begin************
	fd = open("/dev/fb2", O_RDWR);
	if(fd == -1)
	{
		//printf("Failed to open fb2 device\n");
		//return FALSE;
	}
	else
	{
		close(fd);
	}
	//fb2 end************
	
	return 0;//csp modify
}
#endif

s32 BizGetDvrInfo( SBizDvrInfo* pInfo )
{
	return ConfigGetDvrInfo(0, pInfo);
}

s32 BizGetSysInfo(SBizSystemPara* sysPara)
{
	return ConfigGetSystemPara(0, sysPara);
}

#if 0//csp modify
static void CalcFrameRate( u8 bVideo )
{
	static int nFr = 0;
	static int nOldFr = 0;
	static struct timeval sOldTv;
	static int bTen = 0;
	
	static int nAFr = 0;
	static int nAOldFr = 0;
	static struct timeval sAOldTv;
	static int bATen = 0;
	
	struct timeval t;
	struct tm *ptm, *ptm1;
	
	//csp modify
	struct tm tm0, tm1;
	ptm = &tm0;
	ptm1 = &tm1;
	
	if(bVideo)
	{
		if(nFr == 0) memset(&sOldTv, 0, sizeof(sOldTv));
		
		nFr++;
		
		gettimeofday(&t, NULL);
		
		//csp modify
		//ptm = localtime(&t.tv_sec);
		localtime_r(&t.tv_sec, ptm);
		
		int nSec = ptm->tm_sec;
		int nMin = ptm->tm_min;
		int nUsec = t.tv_usec;
	   	
		if((ptm->tm_sec % 10) == 0)// 10s interval
		{
			bTen = 1;	
		}
		
		if(bTen && (ptm->tm_sec % 10) != 0)
		{
			printf("[biz] M %d S %d Old %d Cur %d \n", nMin, nSec, nOldFr, nFr);
			
			bTen = 0;
			
			int nMs = 0;
			
			//csp modify
			//ptm1 = localtime(&sOldTv.tv_sec);
			localtime_r(&sOldTv.tv_sec, ptm1);
			
			if(nSec < ptm1->tm_sec)
			{
				nMs = nSec+60-ptm1->tm_sec;
			}
			else
			{
				nMs = nSec-ptm1->tm_sec;			
			}
			
			if(nUsec < sOldTv.tv_usec)
			{
				nMs = 1000*1000*(nMs+1)+ nUsec - sOldTv.tv_usec;
			}
			else
			{
				nMs = 1000*1000*nMs+ nUsec - sOldTv.tv_usec;
			}
			
			printf("[biz]video framerate: %d f/s\n", (nFr-nOldFr)*1000*1000/nMs );
			
			memcpy(&sOldTv, &t, sizeof(t));
			nOldFr = nFr;
		}
	}
	else
	{
		if(nAFr == 0) memset(&sAOldTv, 0, sizeof(sAOldTv));
		
		nAFr++;
		
		gettimeofday(&t, NULL);
		
		//csp modify
		//ptm = localtime(&t.tv_sec);
		localtime_r(&t.tv_sec, ptm);
		
		int nSec = ptm->tm_sec;
		int nMin = ptm->tm_min;
		int nUsec = t.tv_usec;
		
		if((ptm->tm_sec % 10) == 0)// 10s interval
		{
			bATen = 1;	
		}
		
		if(bATen && (ptm->tm_sec % 10) != 0)
		{
			bATen = 0;
			
			int nMs = 0;
			
			//csp modify
			//ptm1 = localtime(&sAOldTv.tv_sec);
			localtime_r(&sAOldTv.tv_sec, ptm1);
			
			if(nSec < ptm1->tm_sec)
			{
				nMs = nSec+60-ptm1->tm_sec;
			}
			else
			{
				nMs = nSec-ptm1->tm_sec;			
			}
			
			if(nUsec < sAOldTv.tv_usec)
			{
				nMs = 1000*1000*(nMs+1)+ nUsec - sAOldTv.tv_usec;
			}
			else
			{
				nMs = 1000*1000*nMs+ nUsec - sAOldTv.tv_usec;
			}
			
			printf("audio framerate: %d f/s\n", (nAFr-nAOldFr)*1000*1000/nMs );
			
			memcpy(&sAOldTv, &t, sizeof(t));
			nAOldFr = nAFr;
		}
	}
}
#endif

void BizNetStopEnc(void)
{
	int 		i;
 	SBizDvrInfo sDvrInfo;
 	
	memset(&sDvrInfo, 0, sizeof(sDvrInfo));
	if( 0 == ConfigGetDvrInfo(0,&sDvrInfo) )
	{	
		for(i=0; i<sDvrInfo.nVidMainNum; i++)
		{
			VEncMainControl(i, 0, 1);		
		}
		
		for(i=0; i<sDvrInfo.nVidSubNum; i++)
		{
			VEncSubControl(i, 0, 1);		
		}
	}
}

