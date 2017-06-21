#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/hdreg.h>
#include <scsi/scsi.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <dirent.h>
#include "Hddcmd.h"
#include "format.h"
#include "update.h"
#include <sys/mount.h>
#include <sys/vfs.h>

#include "diskmanage.h"
#include "hddcmd.h"
//#include "common.h"
#include "mod_config.h"
#include "public.h"

//yaogang modify 20141118
//以硬盘逻辑编号按位排列
//disk logic index	 8765 4321
//eg disk_status = 0b0000 0100 表示逻辑SATA口3上的硬盘丢失
typedef struct 
{
	int init_status;
	int disk_status;
	pthread_mutex_t lock;
}check_disk_lost_info;

typedef struct 
{
	int init_status;
	int bcheck;//本模块已经在开机检测过硬盘
	int disk_status;
	pthread_mutex_t lock;
}check_disk_exist_info;

static check_disk_lost_info check_disk_lost;
static check_disk_exist_info check_disk_exist;//开机检测一次，无硬盘报警


//disk info struct
typedef struct 
{
	int			nDevIdx;	// <STR_DEV_MAX valid dev, -1 empty
	u8			nState;		// 0 checking, 1 valid, 2 empty
	SDevInfo	sDskInfo;
} SDiskInfo;

//yg modify
static time_t idle_time[STR_DEV_MAX] = {0};

//yg modify
//extern int tl_power_atx_check();

//static pthread_mutex_t mtxFreshDskInf = PTHREAD_MUTEX_INITIALIZER;
//static pthread_mutex_t mtxQueryDskMark = PTHREAD_MUTEX_INITIALIZER;
static char mtxQueryDskMark = 0;

static pthread_mutex_t updateLock = PTHREAD_MUTEX_INITIALIZER;

static u8 nValidDskCnr = 0;

static u8 nValidSataDiskCount = 0;
static u8 nValidUsbDiskCount = 0;

#define MAX_UDISK_NUM	1

static SDiskInfo sSataDiskInfoIns[MAX_HDD_NUM];
static SDiskInfo sUsbDiskInfoIns[MAX_UDISK_NUM];
static SDiskInfo sAllDiskInfoIns[MAX_HDD_NUM+MAX_UDISK_NUM];
unsigned int nMaxSupportDiskNum = sizeof(sAllDiskInfoIns)/sizeof(SDiskInfo);

extern SModSyscplx sModSysIns;

// function 
extern void modsys_SetDskAct(u8 op, BOOL state);
extern int get_disk_info_ex(HddInfo *hdd_manager, int  disk_index, u32* nPrtnNum);
//extern int GetStorageInfo(char *MountPoint, int *Capacity, int type);
extern s32 modSysComplex_MountUsbDev(u8 nIdx);

extern HddInfo *get_hddinfo_by_sn(const char *pdisksn);//yaogang

SDevInfo* modsys_GetDiskInfo(u8 nIdx)
{
	SDiskInfo* pDiskInfo = sAllDiskInfoIns;
	
	pthread_mutex_lock(&updateLock);
	printf("%s nIdx: %d, nValidDskCnr: %d\n", __func__, nIdx, nValidDskCnr);
	if(nIdx >= nValidDskCnr)
	{
		pthread_mutex_unlock(&updateLock);
		return NULL;
	}
	
	//csp modify 20121018
	//if(pDiskInfo[nIdx].nState == 2)
	
	if(pDiskInfo[nIdx].nDevIdx == -1 || pDiskInfo[nIdx].nState == 2)
	{
		pthread_mutex_unlock(&updateLock);
		return NULL;
	}
	/*
	printf("modsys_GetDiskInfo: nIdx = %d, nDevIdx = %d, nType: %d, sDskInfo.nStatus = %d, nTotal = %d, nFree = %d, strDevPath = %s\n", 
		nIdx, pDiskInfo[nIdx].nDevIdx, pDiskInfo[nIdx].sDskInfo.nType, pDiskInfo[nIdx].sDskInfo.nStatus,
		pDiskInfo[nIdx].sDskInfo.nTotal, pDiskInfo[nIdx].sDskInfo.nFree, 
		pDiskInfo[nIdx].sDskInfo.strDevPath);
	*/
	//yaogang
	if(pDiskInfo[nIdx].sDskInfo.nType == 0)
	{
		//printf("%s yg 1\n", __func__);
		HddInfo *phddinfo = get_hddinfo_by_sn(pDiskInfo[nIdx].sDskInfo.disk_sn);//capacity
		//printf("%s yg 2\n", __func__);
		if(phddinfo != NULL)
		{
			//printf("%s yg 3\n", __func__);
			pDiskInfo[nIdx].sDskInfo.nTotal = phddinfo->disk_total;
			pDiskInfo[nIdx].sDskInfo.nFree = phddinfo->disk_free;
			//printf("%s yg 4\n", __func__);
		}
	}
	
	pthread_mutex_unlock(&updateLock);
	
	return &pDiskInfo[nIdx].sDskInfo;
}

s32 modsys_GetDevIdx(u8 nIdx)
{
	SDiskInfo* pDiskInfo = sAllDiskInfoIns;
	
	pthread_mutex_lock(&updateLock);
	
	if(nIdx >= nValidDskCnr)
	{
		pthread_mutex_unlock(&updateLock);
		return -1;
	}
	
	s32 nDevIdx = pDiskInfo[nIdx].nDevIdx;
	
	pthread_mutex_unlock(&updateLock);
	
	return nDevIdx;
}

//不检查条件: 
//备份进行中
//csp modify 20121018
//格式化中
//正在升级中
BOOL modsys_CheckDskInfo()
{
	BOOL bRslt = TRUE;
	
	if(sModSysIns.bBackuping
		|| sModSysIns.bFormating
		|| sModSysIns.bUpdate)
	{
		bRslt = FALSE;
	}
	
	return bRslt;
}

u8 modsys_GetInsertDiskStatue()
{
	return 0;
}

// 1:失败0: 成功
static int set_hdd_sleep(char *pstrDevPath)
{
	#define ATA_OP_SLEEPNOW1	(0xe6)
	
	unsigned char args1[4] = {ATA_OP_SLEEPNOW1,0,0,0};
	
	int fd = open(pstrDevPath, O_RDONLY);
	if(fd < 0) 
	{
		perror(pstrDevPath);
		return 1;
	}
	
	int ret = ioctl(fd, HDIO_DRIVE_CMD, args1);
	if(ret < 0)
	{
		perror("ioctl HDD sleep");
		close(fd);//
		return 1;
	}
	
	close(fd);//
	
	return 0;
}

int find_disk_index(disk_manager* pDskMgr, int nDevIdx)
{
	int i = 0;
	for(i = 0; i < MAX_HDD_NUM; i++)
	{
		if(pDskMgr->hinfo[i].is_disk_exist && pDskMgr->hinfo[i].disk_system_idx == nDevIdx)
		{
			return i;
		}
	}
	return -1;
}
	
u8 is_system_disk_status_changed(SDiskInfo *pLast, u8 nLastNum, SDiskInfo *pCur, u8 nCurNum)
{
	u8 bChanged = 0;
	
	int i, j;
	
	time_t cur_time;
	time(&cur_time);
	
	//printf("%s\n", __func__);
	
	for(j=0; j<nLastNum; j++)
	{
		for(i=0; i<nCurNum; i++)
		{
			if(strcmp(pLast[j].sDskInfo.disk_sn, pCur[i].sDskInfo.disk_sn) == 0)
			{
				if(strcmp(pLast[j].sDskInfo.strDevPath, pCur[i].sDskInfo.strDevPath) != 0 \
					|| pLast[j].sDskInfo.disk_physical_idx != pCur[i].sDskInfo.disk_physical_idx)
				{//硬盘序列号一致但SATA序号或系统序号不同，热插拔发生
					if(pCur[i].sDskInfo.nPartitionNo == MAX_PARTITION_NUM)
					{
						pCur[i].sDskInfo.nStatus = HDD_IDLE;
						idle_time[pCur[i].sDskInfo.disk_logic_idx] = cur_time;
						printf("%s 1, cur: %d, last: %d\n", __func__, pCur[i].sDskInfo.nStatus, pLast[j].sDskInfo.nStatus);
					}
					else
					{
						pCur[i].sDskInfo.nStatus = HDD_UNFORMAT;
						printf("%s 2, cur: %d, last: %d\n", __func__, pCur[i].sDskInfo.nStatus, pLast[j].sDskInfo.nStatus);
					}
					bChanged = 1;
					//if(strcmp(pCur[i].sDskInfo.strDevPath,"/dev/sdd") == 0) printf("hehe-1,disk:%s,status:%d\n",pCur[i].sDskInfo.strDevPath,pCur[i].sDskInfo.nStatus);
				}
				else//该硬盘没有热插拔，如果状态有改变。
				//nState/sDskInfo.nTotal/sDskInfo.nFree/sDskInfo.nPartitionNo 延用之前的值
				{
					if(pCur[i].sDskInfo.nStatus != pLast[j].sDskInfo.nStatus)
					{
						if(pCur[i].sDskInfo.nStatus == HDD_INUSE)
						{
							printf("%s 3, cur: %d, last: %d\n", __func__, pCur[i].sDskInfo.nStatus, pLast[j].sDskInfo.nStatus);
							pCur[i].nState = pLast[j].nState;
							pCur[i].sDskInfo.nTotal = pLast[j].sDskInfo.nTotal;
							pCur[i].sDskInfo.nFree = pLast[j].sDskInfo.nFree;
							pCur[i].sDskInfo.nPartitionNo = pLast[j].sDskInfo.nPartitionNo;
							
							bChanged = 1;
						}
						else//if(pCur[i].sDskInfo.nStatus == HDD_IDLE)//由查询得来，只能表明当前无模块操作该硬盘
						{
							if((pLast[j].sDskInfo.nStatus == HDD_SLEEP) \
								|| (pLast[j].sDskInfo.nStatus == HDD_ERROR))
							{
								printf("%s 4, cur: %d, last: %d\n", __func__, pCur[i].sDskInfo.nStatus, pLast[j].sDskInfo.nStatus);
								pCur[i].sDskInfo.nStatus = pLast[j].sDskInfo.nStatus;
								pCur[i].nState = pLast[j].nState;
								pCur[i].sDskInfo.nTotal = pLast[j].sDskInfo.nTotal;
								pCur[i].sDskInfo.nFree = pLast[j].sDskInfo.nFree;
								pCur[i].sDskInfo.nPartitionNo = pLast[j].sDskInfo.nPartitionNo;
								//printf("nTotal: %d\n", pCur[i].sDskInfo.nTotal);
							}
							else if(pLast[j].sDskInfo.nStatus == HDD_UNFORMAT)
							{
								printf("%s 5, cur: %d, last: %d\n", __func__, pCur[i].sDskInfo.nStatus, pLast[j].sDskInfo.nStatus);
								disk_manager* pDskMgr = sModSysIns.pDiskManager;
								int x = find_disk_index(pDskMgr, pCur[i].nDevIdx);
								if(x != -1)
								{
									u8 nPrtNum = 0;
									int k = 0;
									for(k = 0; k < MAX_PARTITION_NUM; k++)
									{
										if(pDskMgr->hinfo[x].is_partition_exist[k])
										{
											nPrtNum++;
										}
									}
									pCur[i].sDskInfo.nStatus = (nPrtNum == MAX_PARTITION_NUM) ? HDD_IDLE: HDD_UNFORMAT;
									pCur[i].sDskInfo.nPartitionNo = nPrtNum;
								}
								else
								{
									pCur[i].sDskInfo.nStatus = HDD_UNFORMAT;
									pCur[i].sDskInfo.nPartitionNo = pLast[j].sDskInfo.nPartitionNo;
								}
								pCur[i].nState = pLast[j].nState;
								pCur[i].sDskInfo.nTotal = pLast[j].sDskInfo.nTotal;
								pCur[i].sDskInfo.nFree = pLast[j].sDskInfo.nFree;
								if(pCur[i].sDskInfo.nStatus != HDD_UNFORMAT)
								{
									bChanged = 1;
								}
							}
							else//if(cur_time - idle_time[pCur[i].sDskInfo.disk_logic_idx] > 5)//状态确为空闲，但之前可能没有对时间赋值
							{
								printf("%s 6, cur: %d, last: %d\n", __func__, pCur[i].sDskInfo.nStatus, pLast[j].sDskInfo.nStatus);
								idle_time[pCur[i].sDskInfo.disk_logic_idx] = cur_time;
								
								pCur[i].nState = pLast[j].nState;
								pCur[i].sDskInfo.nTotal = pLast[j].sDskInfo.nTotal;
								pCur[i].sDskInfo.nFree = pLast[j].sDskInfo.nFree;
								pCur[i].sDskInfo.nPartitionNo = pLast[j].sDskInfo.nPartitionNo;
								
								bChanged = 1;

								if (HDD_ERROR == pCur[i].sDskInfo.nStatus)
								{
									PublicDealBadDisk(pCur[i].sDskInfo.disk_logic_idx);
								}
							}
						}
						//pLast[j].sDskInfo.nStatus = pCur[i].sDskInfo.nStatus;
						//没有热插拔，就不会memcpy(sSataDiskInfoIns, sTmpSataDiskInfoIns, sizeof(sSataDiskInfoIns));
						//此时要更新sSataDiskInfoIns 中硬盘状态
						//bChanged = 1;
						//if(strcmp(pCur[i].sDskInfo.strDevPath,"/dev/sdd") == 0) printf("hehe-2,disk:%s,status:%d\n",pCur[i].sDskInfo.strDevPath,pCur[i].sDskInfo.nStatus);
					}
					else//状态没有发生改变
					{
						//if(strcmp(pCur[i].sDskInfo.strDevPath,"/dev/sdd") == 0) printf("hehe-3.0,disk:%s,status:%d\n",pCur[i].sDskInfo.strDevPath,pCur[i].sDskInfo.nStatus);
						if((pCur[i].sDskInfo.nStatus == HDD_IDLE)/* && (cur_time - idle_time[pCur[i].sDskInfo.disk_logic_idx] > 2*60)*/)
						{
							//超时休眠
							//pCur[i].sDskInfo.nStatus = HDD_IDLE;//HDD_SLEEP;
							//printf("%s set %s sleep\n", __func__, pCur[i].sDskInfo.strDevPath);
							//set_hdd_sleep(pCur[i].sDskInfo.strDevPath);
							//printf("%s 7, cur: %d, last: %d\n", __func__, pCur[i].sDskInfo.nStatus, pLast[j].sDskInfo.nStatus);
							disk_manager* pDskMgr = sModSysIns.pDiskManager;
							int x = find_disk_index(pDskMgr, pCur[i].nDevIdx);
							if(x != -1)
							{
								u8 nPrtNum = 0;
								int k = 0;
								for(k = 0; k < MAX_PARTITION_NUM; k++)
								{
									if(pDskMgr->hinfo[x].is_partition_exist[k])
									{
										nPrtNum++;
									}
								}
								pCur[i].sDskInfo.nStatus = (nPrtNum == MAX_PARTITION_NUM) ? HDD_IDLE: HDD_UNFORMAT;
								pCur[i].sDskInfo.nPartitionNo = nPrtNum;
								
								//if(strcmp(pCur[i].sDskInfo.strDevPath,"/dev/sdd") == 0) printf("hehe-3.1,disk:%s,status:%d,nPrtNum:%d\n",pCur[i].sDskInfo.strDevPath,pCur[i].sDskInfo.nStatus,nPrtNum);
							}
							else
							{
								pCur[i].sDskInfo.nStatus = HDD_UNFORMAT;
								pCur[i].sDskInfo.nPartitionNo = pLast[j].sDskInfo.nPartitionNo;
								
								//if(strcmp(pCur[i].sDskInfo.strDevPath,"/dev/sdd") == 0) printf("hehe-3.2,disk:%s,status:%d\n",pCur[i].sDskInfo.strDevPath,pCur[i].sDskInfo.nStatus);
							}
							
							pCur[i].nState = pLast[j].nState;
							pCur[i].sDskInfo.nTotal = pLast[j].sDskInfo.nTotal;
							pCur[i].sDskInfo.nFree = pLast[j].sDskInfo.nFree;
							pCur[i].sDskInfo.nPartitionNo = pLast[j].sDskInfo.nPartitionNo;
							
							//bChanged = 1;
						}
						//if(strcmp(pCur[i].sDskInfo.strDevPath,"/dev/sdd") == 0) printf("hehe-3.3,disk:%s,status:%d\n",pCur[i].sDskInfo.strDevPath,pCur[i].sDskInfo.nStatus);
					}	
				}
				break;
			}
		}
		if(i == nCurNum)//没找到，说明已经拔下，预留在此，现在无动作
		{
			printf("unplug\n");
			bChanged = 1;
		}
	}
	
	for(i=0; i<nCurNum; i++)
	{
		for(j=0; j<nLastNum; j++)
		{
			if(strcmp(pCur[i].sDskInfo.disk_sn, pLast[j].sDskInfo.disk_sn) == 0)
			{
				break;
			}
		}
		if(j == nLastNum)//没找到，说明此硬盘刚刚接入系统
		{
			disk_manager* pDskMgr = sModSysIns.pDiskManager;
			int x = find_disk_index(pDskMgr, pCur[i].nDevIdx);
			if(x != -1)
			{
				u8 nPrtNum = 0;
				int k = 0;
				for(k = 0; k < MAX_PARTITION_NUM; k++)
				{
					if(pDskMgr->hinfo[x].is_partition_exist[k])
					{
						nPrtNum++;
					}
				}
				pCur[i].sDskInfo.nPartitionNo = nPrtNum;
			}
			if(pCur[i].sDskInfo.nPartitionNo == MAX_PARTITION_NUM)
			{
				pCur[i].sDskInfo.nStatus = HDD_IDLE;
				idle_time[pCur[i].sDskInfo.disk_logic_idx] = cur_time;
				printf("%s 7\n", __func__);
			}
			else
			{
				pCur[i].sDskInfo.nStatus = HDD_UNFORMAT;
				printf("%s 8\n", __func__);
			}
			bChanged = 1;
		}
		//if(strcmp(pCur[i].sDskInfo.strDevPath,"/dev/sdd") == 0) printf("hehe-4,disk:%s,status:%d\n",pCur[i].sDskInfo.strDevPath,pCur[i].sDskInfo.nStatus);
	}
	
	return bChanged;
}
void upgrade_disk_lost(SDiskInfo *pLast, SDiskInfo *pCur, u8 nDiskNum)
{
	//yaogang modify 20141118
	//check disk lost
	int last, cur , i;
	last = cur = 0;
	
	for(i = 0; i < nDiskNum; i++)
	{
		if (pCur[i].nDevIdx != -1)//存在&& HDD
		{
			cur |= (1<< (pCur[i].sDskInfo.disk_logic_idx -1));
			
		}
		if (pLast[i].nDevIdx != -1)//存在
		{
			last |= (1<< (pLast[i].sDskInfo.disk_logic_idx - 1));
			
		}
	}

	
	printf("yg upgrade_disk_lost cur: 0x%x, last: 0x%x\n", cur, last);
	pthread_mutex_lock(&check_disk_lost.lock);
	for(i = 0; i < MAX_HDD_NUM; i++)
	{
		//上一次有，当前无，则硬盘丢失
		if ( (last & (1<< i)) && ((cur & (1<< i)) == 0) )
		{
			check_disk_lost.disk_status |= 1 << i;
		}
	}
	pthread_mutex_unlock(&check_disk_lost.lock);
	
}

BOOL is_system_disk_changed(SDiskInfo *pLast, SDiskInfo *pCur, u8 nDiskNum)
{
	BOOL bChanged = FALSE;
	
	int i = 0;
	for(i = 0; i < nDiskNum; i++)
	{
		if(pLast[i].nDevIdx != pCur[i].nDevIdx)
		{
			printf("%s i: %d, pLast[i].nDevIdx: %d, pCur[i].nDevIdx: %d\n",
				__func__, i, pLast[i].nDevIdx, pCur[i].nDevIdx);
			
			bChanged = TRUE;
			break;
		}
		else
		{
			//disk不存在
			if(pCur[i].nDevIdx == -1)
			{
				continue;
			}
		}
		
		if(strcmp(pLast[i].sDskInfo.strDevPath, pCur[i].sDskInfo.strDevPath) != 0)
		{
			//
			char buf[32];
			buf[31] = 0;
			strncpy(buf, pLast[i].sDskInfo.strDevPath, 31);
			printf("%s i: %d, pLast[i].sDskInfo.strDevPath: %s\n",
				__func__, i, pLast[i].sDskInfo.strDevPath);
			
			strncpy(buf, pCur[i].sDskInfo.strDevPath, 31);
			printf("%s i: %d, pCur[i].sDskInfo.strDevPath: %s\n",
				__func__, i, pCur[i].sDskInfo.strDevPath);
			//
			
			bChanged = TRUE;
			break;
		}
		
		//前后两次检测的磁盘类型不一样，U盘变成了硬盘或者硬盘变为了U盘
		if(pLast[i].sDskInfo.nType != pCur[i].sDskInfo.nType)
		{
			printf("%s i: %d, pLast[i].sDskInfo.nType: %d, pCur[i].sDskInfo.nType: %d\n",
				__func__, i, pLast[i].sDskInfo.nType, pCur[i].sDskInfo.nType);
			
			bChanged = TRUE;
			break;
		}
		
		if(pCur[i].sDskInfo.nType == 0)
		{
			if(pLast[i].sDskInfo.disk_physical_idx != pCur[i].sDskInfo.disk_physical_idx)
			{
				printf("%s i: %d, pLast[i].sDskInfo.disk_physical_idx: %d, pCur[i].sDskInfo.disk_physical_idx: %d\n",
					__func__, i, pLast[i].sDskInfo.disk_physical_idx, pCur[i].sDskInfo.disk_physical_idx);
				
				bChanged = TRUE;
				break;
			}
			
			if(strcmp(pLast[i].sDskInfo.disk_sn, pCur[i].sDskInfo.disk_sn) != 0)
			{
				printf("%s i: %d, sn different\n", __func__, i);
				
				bChanged = TRUE;
				break;
			}
		}
	}
	
	if(bChanged)
	{
		printf("disk is changed\n");
	}
	else
	{
		//printf("disk is not changed\n");
	}
	
	return bChanged;
}
int get_disk_lost_statue()
{
	int ret = 0;

	if (check_disk_lost.init_status)
	{
		pthread_mutex_lock(&check_disk_lost.lock);
		
		ret = check_disk_lost.disk_status;
		check_disk_lost.disk_status = 0;
		
		pthread_mutex_unlock(&check_disk_lost.lock);
	}
	return ret;
}

int get_disk_exist_statue()
{
	int ret = 0;

	if (check_disk_exist.init_status)
	{
		pthread_mutex_lock(&check_disk_exist.lock);
		
		if (check_disk_exist.bcheck)//模块开机后已经检测过硬盘
		{
			ret |= 1<<1;//告诉调用者，已经检测过
			if (check_disk_exist.disk_status)//无硬盘，报警
			{
				ret |= 1; 
			}
			check_disk_exist.disk_status = 0;
		}
		
		pthread_mutex_unlock(&check_disk_exist.lock);
	}
	return ret;
}


int GetAllDiskInfo(SDiskInfo *pSataInfoIns, u8 nSataCount, SDiskInfo *pUsbInfoIns, u8 nUsbCount)
{
	if(nSataCount + nUsbCount > nMaxSupportDiskNum)
	{
		return -1;
	}
	
	pthread_mutex_lock(&updateLock);
	u8 nCount = nSataCount + nUsbCount;
	u8 i = 0, j = 0, k = 0;
	//i sata  j usb
	for(k = 0; k < nCount; k++)
	{
		if(j >= nUsbCount)
		{
			memcpy(&sAllDiskInfoIns[k], &pSataInfoIns[i++], sizeof(SDiskInfo));
		}
		else if(i >= nSataCount)
		{
			memcpy(&sAllDiskInfoIns[k], &pUsbInfoIns[j++], sizeof(SDiskInfo));
		}
		else
		{
			if(pSataInfoIns[i].nDevIdx < pUsbInfoIns[j].nDevIdx)
			{
				memcpy(&sAllDiskInfoIns[k], &pSataInfoIns[i++], sizeof(SDiskInfo));
			}
			else
			{
				memcpy(&sAllDiskInfoIns[k], &pUsbInfoIns[j++], sizeof(SDiskInfo));
			}
		}
	}
	
	if(nMaxSupportDiskNum > nCount)
	{
		memset(&sAllDiskInfoIns[k], 0, sizeof(SDiskInfo)*(nMaxSupportDiskNum-nCount));
		for( ; k < nMaxSupportDiskNum; k++)
		{
			sAllDiskInfoIns[k].nDevIdx = -1;
			sAllDiskInfoIns[k].nState = 2;
			sAllDiskInfoIns[k].sDskInfo.nType = 0;
		}
	}
	
	nValidDskCnr = nCount;
	pthread_mutex_unlock(&updateLock);
	return 0;
}

//yaogang modify for bad disk
/*
录像时如果遇到硬盘坏道，LINUX会将该硬盘断开，从上层来看相当于硬盘丢失
现去除硬盘热插拔功能
*/
#define disable_disk_hotplug

//yg modify
/*
统计录像、日志、回放模块中正在使用的硬盘
结果要么在使用；要么暂定为空闲，具体状态后面处理
*/
extern void Record_get_disk_use_info(u8 pdiskuse[16], u32 *prec_disk_err_bitmask);

#ifndef disable_disk_hotplug
void get_disk_status(SDiskInfo* pSataDiskInfo, u8 nSataDiskFound)
{
	int i, j;
	u8 rec_disk_use[16] = {0};
	
	Record_get_disk_use_info(rec_disk_use);
	
	for(i=0; i<16; i++)
	{
		if(rec_disk_use[i] > 0)
		{
			//printf("%s rec_disk_use[%d]: %d\n", __func__, i, rec_disk_use[i]);
			for(j=0; j<nSataDiskFound; j++)
			{
				if(pSataDiskInfo[j].nDevIdx == rec_disk_use[i]-1)
				{
					pSataDiskInfo[j].sDskInfo.nStatus = HDD_INUSE;
				}
			}
		}
	}
	for(j=0; j<nSataDiskFound; j++)
	{
		if(pSataDiskInfo[j].sDskInfo.nStatus != HDD_INUSE)
		{
			pSataDiskInfo[j].sDskInfo.nStatus = HDD_IDLE;//非在使用状态下，暂定为空闲
		}
	}
}

#else
void get_disk_status(SDiskInfo* pSataDiskInfo, u8 nSataDiskFound)
{
	int i, j;
	u8 rec_disk_use[16] = {0};
	u32 rec_disk_err_bitmask = 0;

	#if 1 //debug
	static int bitmask = 0;
	
	Record_get_disk_use_info(rec_disk_use, &rec_disk_err_bitmask);

	if (bitmask != rec_disk_err_bitmask)
	{
		printf("%s rec_disk_err_bitmask: 0x%x\n", __func__, rec_disk_err_bitmask);

		bitmask = rec_disk_err_bitmask;
	}
	#endif

	for (j=0; j<nSataDiskFound; ++j)
	{
		if (pSataDiskInfo[j].sDskInfo.nStatus != HDD_ERROR)
		{
			if (rec_disk_err_bitmask & (1 << (pSataDiskInfo[j].sDskInfo.disk_logic_idx-1)))
			{
				pSataDiskInfo[j].sDskInfo.nStatus = HDD_ERROR;

				printf("%s bad disk logic index: %d\n", __func__, pSataDiskInfo[j].sDskInfo.disk_logic_idx);
			}
		}
		
		if (pSataDiskInfo[j].sDskInfo.nStatus == HDD_ERROR)
		{
			continue;
		}
	#if 0
		if (pSataDiskInfo[j].sDskInfo.nStatus == HDD_UNFORMAT)
		{
			continue;
		}

		if (pSataDiskInfo[j].sDskInfo.nStatus == HDD_FORMATTING)
		{
			continue;
		}
	#endif
	
		//暂定为空闲为的是在is_system_disk_status_changed() 检查是否格式化完成
		pSataDiskInfo[j].sDskInfo.nStatus = HDD_IDLE;//非错误状态下，暂定为空闲
		
		for (i=0; i<16; ++i)
		{
			if (rec_disk_use[i] 
				&& (pSataDiskInfo[j].nDevIdx == rec_disk_use[i]-1))
			{
				//printf("%s rec_disk_use[%d]: %d\n", __func__, i, rec_disk_use[i]);
				pSataDiskInfo[j].sDskInfo.nStatus = HDD_INUSE;				
			}
		}
	}
}

#endif

void* modsys_UpdateDiskInfoFxn(void* p)
{
	int			i = 0;
	
	u8			bSata = FALSE;
	u8			bUdsk = FALSE;
	
	u8			nSataDiskFound = 0;
	u8			nUsbDiskFound = 0;
	
	u8			nSataChanged = 0;
	u8			nSataStatusChanged = 0;
	u8			nUsbChanged = 0;
	
	pthread_mutex_init(&check_disk_lost.lock, NULL);
	check_disk_lost.disk_status = 0;
	check_disk_lost.init_status = 1;

	pthread_mutex_init(&check_disk_exist.lock, NULL);
	check_disk_exist.bcheck = 0;
	check_disk_exist.disk_status = 0;
	check_disk_exist.init_status = 1;
	
	disk_manager* pDskMgr = sModSysIns.pDiskManager;
	
	printf("$$$$$$$$$$$$$$$$$$modsys_UpdateDiskInfoFxn id:%d\n",getpid());
	
	//modify
	time_t curtime;
	time(&curtime);
	for(i = 0; i < STR_DEV_MAX; i++)
	{
		idle_time[i] = curtime;
	}
	
	memset(sSataDiskInfoIns, 0, sizeof(sSataDiskInfoIns));
	for(i = 0; i < MAX_HDD_NUM; i++)
	{
		sSataDiskInfoIns[i].nDevIdx = -1;
		sSataDiskInfoIns[i].nState = 2;
		sSataDiskInfoIns[i].sDskInfo.nType = 0;
		
		if(pDskMgr->hinfo[i].is_disk_exist)
		{
			SDevInfo *pInfo = &sSataDiskInfoIns[i].sDskInfo;
			pInfo->disk_physical_idx = pDskMgr->hinfo[i].disk_physical_idx;
			pInfo->disk_logic_idx = pDskMgr->hinfo[i].disk_logic_idx;
			pInfo->disk_system_idx = pDskMgr->hinfo[i].disk_system_idx;
			strcpy(pInfo->disk_sn, pDskMgr->hinfo[i].disk_sn);
			strcpy(pInfo->strDevPath, pDskMgr->hinfo[i].disk_name);
			pInfo->nFree = pDskMgr->hinfo[i].disk_free;
			pInfo->nTotal = pDskMgr->hinfo[i].disk_total;
			pInfo->nType = (pDskMgr->hinfo[i].storage_type == 's') ? 0 : 1;
			//printf("start nTotal: %u\n", pInfo->nTotal);
			
			u8 nPrtNum = 0;
			int j = 0;
			for(j = 0; j < MAX_PARTITION_NUM; j++)
			{
				if(pDskMgr->hinfo[i].is_partition_exist[j])
				{
					nPrtNum++;
				}
			}
			pInfo->nPartitionNo = nPrtNum;
			
			//pInfo->nStatus = (nPrtNum == MAX_PARTITION_NUM) ? 0 : 1;
			//yaogang modify for bad disk
			if (pDskMgr->hinfo[i].is_bad_disk)
			{
				pInfo->nStatus = HDD_ERROR;
			}
			else if(nPrtNum != MAX_PARTITION_NUM)
			{
				pInfo->nStatus = HDD_UNFORMAT;
			}
			else
			{
				pInfo->nStatus = HDD_IDLE;//初始化空闲
				idle_time[pInfo->disk_logic_idx] = curtime;//记录空闲起始时间
			}
			
			sSataDiskInfoIns[i].nDevIdx = pDskMgr->hinfo[i].disk_system_idx;
			sSataDiskInfoIns[i].nState = 1;
			
			nSataDiskFound++;
		}
	}
	nValidSataDiskCount = nSataDiskFound;
	
	memset(sUsbDiskInfoIns, 0, sizeof(sUsbDiskInfoIns));
	for(i = 0; i < MAX_UDISK_NUM; i++)
	{
		sUsbDiskInfoIns[i].nDevIdx = -1;
		sUsbDiskInfoIns[i].nState = 2;
		sUsbDiskInfoIns[i].sDskInfo.nType = 1;
	}
	nValidUsbDiskCount = 0;
	
	GetAllDiskInfo(sSataDiskInfoIns, nValidSataDiskCount, sUsbDiskInfoIns, nValidUsbDiskCount);
	
	//yg modify 20140729
	//unsigned char uncheck_disk_flag = 0;//ATX掉电电池供电时，不检查硬盘,值为1，否则为0
	
	while(1)
	{
		//time(&curtime);
		
		int n = 0;
		for(n = 0; n < 300; n++)
		{
			//yg modify 20140729
			/*
			unsigned char cur_atx_flag = tl_power_atx_check();
			if(cur_atx_flag == 0)//电池供电
			{
				if(uncheck_disk_flag == 0)
				{
					//设置相关变量
					nValidSataDiskCount = 0;
					memset(sSataDiskInfoIns, 0, sizeof(sSataDiskInfoIns));
					for(i = 0; i < MAX_HDD_NUM; i++)
					{
						sSataDiskInfoIns[i].nDevIdx = -1;
						sSataDiskInfoIns[i].nState = 2;
						sSataDiskInfoIns[i].sDskInfo.nType = 0;
					}
					
					nValidUsbDiskCount = 0;
					memset(sUsbDiskInfoIns, 0, sizeof(sUsbDiskInfoIns));
					for(i = 0; i < MAX_UDISK_NUM; i++)
					{
						sUsbDiskInfoIns[i].nDevIdx = -1;
						sUsbDiskInfoIns[i].nState = 2;
						sUsbDiskInfoIns[i].sDskInfo.nType = 1;
					}
					
					GetAllDiskInfo(sSataDiskInfoIns, nValidSataDiskCount, sUsbDiskInfoIns, nValidUsbDiskCount);	
					
					//卸载所有硬盘
					SDevInfo sTmpDevInfoIns[nMaxSupportDiskNum];
					memset(sTmpDevInfoIns, 0, sizeof(sTmpDevInfoIns));
					
					SStoreDevManage sStoreMgr;
					sStoreMgr.nDiskNum = 0;
					sStoreMgr.psDevList = sTmpDevInfoIns;
					
					printf("yg ATX Power Down modSysCmplx_StorgeNotify\n");
					modSysCmplx_StorgeNotify(&sStoreMgr, EM_SYSEVENT_SATARELOAD, 0);
					modSysCmplx_StorgeNotify(&sStoreMgr, EM_SYSEVENT_DISKCHANGED, 0);
					
					uncheck_disk_flag = 1;
				}
				
				//刷新磁盘信息后将格式化，备份标记置假
				modsys_SetDskAct(0, FALSE);
				modsys_SetDskAct(2, FALSE);
				modsys_SetDskAct(4, FALSE);//sModSysIns.bRefreshSata//清除标记
				
				break;
			}
			else //ATX 供电
			{
				uncheck_disk_flag = 0;
			}
			//yg modify 20140729 end
			*/
			usleep(10*1000);
			
			if(sModSysIns.bRefreshSata || sModSysIns.bFormatCheck || sModSysIns.bBackupCheck)
			{
				break;
			}
		}
		
		//yg modify 20140729
		/*
		if(uncheck_disk_flag == 1)
		{
			continue;
		}
		*/
		nSataDiskFound = 0;
		nUsbDiskFound = 0;
		
		nSataChanged = 0;
		nUsbChanged = 0;
		
		SDiskInfo sTmpSataDiskInfoIns[MAX_HDD_NUM];
		
	#ifndef disable_disk_hotplug
	
		memset(sTmpSataDiskInfoIns, 0, sizeof(sTmpSataDiskInfoIns));
		for(i = 0; i < MAX_HDD_NUM; i++)
		{
			sTmpSataDiskInfoIns[i].nDevIdx = -1;
			sTmpSataDiskInfoIns[i].nState = 2;
			sTmpSataDiskInfoIns[i].sDskInfo.nType = 0;
		}
		
	#endif
		
		SDiskInfo sTmpUsbDiskInfoIns[MAX_UDISK_NUM];
		memset(sTmpUsbDiskInfoIns, 0, sizeof(sTmpUsbDiskInfoIns));
		for(i = 0; i < MAX_UDISK_NUM; i++)
		{
			sTmpUsbDiskInfoIns[i].nDevIdx = -1;
			sTmpUsbDiskInfoIns[i].nState = 2;
			sTmpUsbDiskInfoIns[i].sDskInfo.nType = 1;
		}
		
		BOOL bIgnore = FALSE;
		
		int j = 0;
		for(j = 0; j < STR_DEV_MAX; j++)
		{
			//不检查条件: 
			//备份进行中
			//csp modify 20121018
			//格式化中
			//正在升级中
			if(!modsys_CheckDskInfo())
			{
				bIgnore = TRUE;
				break;
			}
			
			char diskname[64] = {0};
			sprintf(diskname, "/dev/sd%c", 'a'+j);
			int fd = open(diskname, O_RDONLY);
			if(fd == -1)
			{
				continue;
			}
			
			bSata = is_sata_disk(fd);
			bUdsk = is_usb_disk(fd);
			
			if(!bSata && !bUdsk)
			{
				close(fd);
				continue;
			}

		#ifndef disable_disk_hotplug
		
			if(bSata && nSataDiskFound < MAX_HDD_NUM)//update sata disk info
			{
				SDiskInfo* pSataDiskInfo = sTmpSataDiskInfoIns;
				
				pSataDiskInfo[nSataDiskFound].nState = 1;//0;
				pSataDiskInfo[nSataDiskFound].sDskInfo.disk_physical_idx = GetDiskPhysicalIndex(fd);
				pSataDiskInfo[nSataDiskFound].sDskInfo.disk_logic_idx = GetDiskLogicIndex(pSataDiskInfo[nSataDiskFound].sDskInfo.disk_physical_idx);
				
				char DiskSN[64];
				memset(DiskSN, 0, sizeof(DiskSN));

				//yaogang modify 20170425
				//需要判断返回值，因为会有下面的情况发生，所以必须检查返回值
				//   /dev目录下有对应的块设备，但ioctl 得到 HDIO_GET_IDENTITY 信息时失败，此时硬盘
				// 可能处于不正常状态，不应该挂载进系统
				if (!GetDiskSN(fd, DiskSN, sizeof(DiskSN)-1))
				{
					printf("%s: %s GetDiskSN failed\n", __func__, diskname);
					close(fd);
					continue;
				}
				
				strcpy(pSataDiskInfo[nSataDiskFound].sDskInfo.disk_sn, DiskSN);
				strcpy(pSataDiskInfo[nSataDiskFound].sDskInfo.strDevPath, diskname);
				pSataDiskInfo[nSataDiskFound].sDskInfo.nType = 0;//sata
				pSataDiskInfo[nSataDiskFound].sDskInfo.nTotal = 0;
				pSataDiskInfo[nSataDiskFound].sDskInfo.nFree = 0;
				pSataDiskInfo[nSataDiskFound].sDskInfo.nPartitionNo = 0;
				pSataDiskInfo[nSataDiskFound].sDskInfo.nStatus = HDD_IDLE;//HDD_UNFORMAT;//1;
				pSataDiskInfo[nSataDiskFound].sDskInfo.disk_system_idx = j;
				pSataDiskInfo[nSataDiskFound].nDevIdx = j;
				
				nSataDiskFound++;
			}

		#endif
		
			if(bUdsk && nUsbDiskFound < MAX_UDISK_NUM)//update usb disk info
			{
				if(nUsbDiskFound == 0)//
				{
					SDiskInfo* pUsbDiskInfo = sTmpUsbDiskInfoIns;
					
					pUsbDiskInfo[nUsbDiskFound].nState = 1;//0;
					pUsbDiskInfo[nUsbDiskFound].sDskInfo.disk_physical_idx = 0;
					pUsbDiskInfo[nUsbDiskFound].sDskInfo.disk_logic_idx = 0;
					memset(pUsbDiskInfo[nUsbDiskFound].sDskInfo.disk_sn, 0, sizeof(pUsbDiskInfo[nUsbDiskFound].sDskInfo.disk_sn));
					strcpy(pUsbDiskInfo[nUsbDiskFound].sDskInfo.strDevPath, diskname);
					pUsbDiskInfo[nUsbDiskFound].sDskInfo.nType = 1;//usb
					pUsbDiskInfo[nUsbDiskFound].sDskInfo.nTotal = 0;
					pUsbDiskInfo[nUsbDiskFound].sDskInfo.nFree = 0;
					pUsbDiskInfo[nUsbDiskFound].sDskInfo.nPartitionNo = 0;
					pUsbDiskInfo[nUsbDiskFound].sDskInfo.nStatus = HDD_IDLE;//HDD_INUSE;
					pUsbDiskInfo[nUsbDiskFound].sDskInfo.disk_system_idx = j;
					pUsbDiskInfo[nUsbDiskFound].nDevIdx = j;
					
					nUsbDiskFound++;
				}
			}
			
			close(fd);
			
			if(nSataDiskFound >= MAX_HDD_NUM && nUsbDiskFound >= MAX_UDISK_NUM)
			{
				printf("Warning: ""%s : nSataDiskFound(%d) >= MAX_HDD_NUM && nUsbDiskFound(%d) >= MAX_UDISK_NUM\n",
					__func__, nSataDiskFound, nUsbDiskFound);
				
				break;
			}
		}
		
		if(bIgnore || !modsys_CheckDskInfo())//再次检查条件
		{
			
		}
		else
		{
		#ifdef disable_disk_hotplug
		
			memcpy(sTmpSataDiskInfoIns, sSataDiskInfoIns, sizeof(sSataDiskInfoIns));
			nSataDiskFound = nValidSataDiskCount;
			
		#endif
			
			//yaogang modiy 20141118 检测开机无硬盘
			pthread_mutex_lock(&check_disk_exist.lock);
			if (check_disk_exist.bcheck == 0)
			{
				if (nSataDiskFound)
				{
					check_disk_exist.disk_status = 0;//有硬盘，不报警
				}
				else
				{
					check_disk_exist.disk_status = 1;
				}
				check_disk_exist.bcheck = 1;
			}
			pthread_mutex_unlock(&check_disk_exist.lock);

		#ifndef disable_disk_hotplug
		
			if(nSataDiskFound != nValidSataDiskCount || is_system_disk_changed(sSataDiskInfoIns, sTmpSataDiskInfoIns, max(nSataDiskFound, nValidSataDiskCount)))
			{
				//为了传递硬盘丢失
				printf("%s nSataDiskFound %d != nValidSataDiskCount %d\n", __func__, nSataDiskFound, nValidSataDiskCount);
				
				upgrade_disk_lost(sSataDiskInfoIns, sTmpSataDiskInfoIns, max(nSataDiskFound, nValidSataDiskCount));
				
				
				SDiskInfo* pSataDiskInfo = sTmpSataDiskInfoIns;
				
				SDevInfo sTmpDevInfoIns[nMaxSupportDiskNum];
				memset(sTmpDevInfoIns, 0, sizeof(sTmpDevInfoIns));
				
				int k = 0;
				for(k = 0; k < nSataDiskFound; k++)
				{
					memcpy(&sTmpDevInfoIns[k], &pSataDiskInfo[k].sDskInfo, sizeof(SDevInfo));
				}
				
				SStoreDevManage sStoreMgr;
				sStoreMgr.nDiskNum = nSataDiskFound;
				sStoreMgr.psDevList = sTmpDevInfoIns;
				
				printf("%s EM_SYSEVENT_SATARELOAD\n", __func__);
				//yaogang modify for bad disk
				modSysCmplx_StorgeNotify(&sStoreMgr, EM_SYSEVENT_SATARELOAD, 0);
				
				for(k = 0; k < nSataDiskFound; k++)
				{
					int x = find_disk_index(pDskMgr, pSataDiskInfo[k].nDevIdx);
					if(x != -1)
					{
						HddInfo *phddinfo = get_hddinfo_by_sn(pSataDiskInfo[k].sDskInfo.disk_sn);//capacity//yaogang
						if(phddinfo != NULL)
						{
							pSataDiskInfo[k].sDskInfo.nTotal = phddinfo->disk_total;
							pSataDiskInfo[k].sDskInfo.nFree = phddinfo->disk_free;
						}
						else
						{
							pSataDiskInfo[k].sDskInfo.nTotal = pDskMgr->hinfo[x].disk_total;
							pSataDiskInfo[k].sDskInfo.nFree = pDskMgr->hinfo[x].disk_free;
						}
						
						u8 nPrtNum = 0;
						int i = 0;
						for(i = 0; i < MAX_PARTITION_NUM; i++)
						{
							if(pDskMgr->hinfo[x].is_partition_exist[i])
							{
								nPrtNum++;
							}
						}
						pSataDiskInfo[k].sDskInfo.nPartitionNo = nPrtNum;
						pSataDiskInfo[k].sDskInfo.nStatus = (nPrtNum == MAX_PARTITION_NUM) ? HDD_IDLE: HDD_UNFORMAT;//0 : 1;

						printf("%s 1, nDevIdx: %d, phddinfo %s NULL, nTotal: %uKb, nFree: %uKb, nPartitionNo: %d, nStatus: %d\n",
							__func__, pSataDiskInfo[k].nDevIdx, phddinfo != NULL ? "!=" : "==",
							pSataDiskInfo[k].sDskInfo.nTotal,
							pSataDiskInfo[k].sDskInfo.nFree,
							pSataDiskInfo[k].sDskInfo.nPartitionNo,
							pSataDiskInfo[k].sDskInfo.nStatus);
					}
					
					pSataDiskInfo[k].nState = 1;
				}
				
				//yg modify
				//memcpy(sSataDiskInfoIns, sTmpSataDiskInfoIns, sizeof(sSataDiskInfoIns));
				//nValidSataDiskCount = nSataDiskFound;
				
				nSataChanged = 1;
			}
		
		#endif
		
			get_disk_status(sTmpSataDiskInfoIns, nSataDiskFound);
			nSataStatusChanged = is_system_disk_status_changed(sSataDiskInfoIns, nValidSataDiskCount, sTmpSataDiskInfoIns, nSataDiskFound);
			//改变sSataDiskInfoIns中硬盘状态值
			if(nSataChanged || nSataStatusChanged)
			//if(nSataChanged)
			{
				printf("%s: nSataChanged: %d\n", __func__, nSataChanged);
				
				nSataChanged = 1;
				memcpy(sSataDiskInfoIns, sTmpSataDiskInfoIns, sizeof(sSataDiskInfoIns));
				nValidSataDiskCount = nSataDiskFound;
			}
			
			if(nUsbDiskFound != nValidUsbDiskCount || is_system_disk_changed(sUsbDiskInfoIns, sTmpUsbDiskInfoIns, max(nUsbDiskFound, nValidUsbDiskCount)))
			{
				umount_user("myusb");
				
				if(nUsbDiskFound)
				{
					SDiskInfo* pUsbDiskInfo = sTmpUsbDiskInfoIns;
					
					if(0 == modSysComplex_MountUsbDev(pUsbDiskInfo[0].nDevIdx))
					{
						printf("###usb disk mount success!!!\n");
						
						struct statfs s;
						int rtn = statfs("myusb", &s);
						
						long long totalspace = 0;
						long long freespace = 0;
						if(rtn >= 0)
						{
							freespace = (((long long)s.f_bsize * (long long)s.f_bfree) / (long long)KB);
							totalspace = (((long long)s.f_bsize * (long long)s.f_blocks) /(long long)KB);
							
							pUsbDiskInfo[0].sDskInfo.nTotal = totalspace;
							pUsbDiskInfo[0].sDskInfo.nFree = freespace;
							pUsbDiskInfo[0].sDskInfo.nStatus = HDD_IDLE;//0;
							pUsbDiskInfo[0].sDskInfo.nPartitionNo = 1;
							
							pUsbDiskInfo[0].nState = 1;
						}
						else
						{
							pUsbDiskInfo[0].sDskInfo.nTotal = 0;
							pUsbDiskInfo[0].sDskInfo.nFree = 0;
							pUsbDiskInfo[0].sDskInfo.nStatus = HDD_ERROR;//2;
							pUsbDiskInfo[0].sDskInfo.nPartitionNo = 0;
						}
					}
				}
				
				memcpy(sUsbDiskInfoIns, sTmpUsbDiskInfoIns, sizeof(sUsbDiskInfoIns));
				nValidUsbDiskCount = nUsbDiskFound;
				
				nUsbChanged = 1;
			}
			
			if(nSataChanged || nUsbChanged || sModSysIns.bFormatCheck)
			{
				printf("%s: nSataChanged: %d, nUsbChanged: %d, bFormatCheck: %d\n", \
					__func__, nSataChanged, nUsbChanged, sModSysIns.bFormatCheck);

				int k = 0;
			
				//yg modify 20140730
				//计算空间大小
				SDiskInfo* pSataDiskInfo = sSataDiskInfoIns;
				
				for(k = 0; k < nValidSataDiskCount; k++)
				{
					int x = find_disk_index(pDskMgr, pSataDiskInfo[k].nDevIdx);
					if(x != -1)
					{
						HddInfo *phddinfo = get_hddinfo_by_sn(pSataDiskInfo[k].sDskInfo.disk_sn);//capacity//yaogang
						if(phddinfo != NULL)
						{
							pSataDiskInfo[k].sDskInfo.nTotal = phddinfo->disk_total;
							pSataDiskInfo[k].sDskInfo.nFree = phddinfo->disk_free;
							
							//printf("disktest 1, devpath: %s, total: %uKb, free: %uKb\n", 
							//	pSataDiskInfo[k].sDskInfo.strDevPath,
							//	pSataDiskInfo[k].sDskInfo.nTotal, pSataDiskInfo[k].sDskInfo.nFree);
						}
						else
						{
							pSataDiskInfo[k].sDskInfo.nTotal = pDskMgr->hinfo[x].disk_total;
							pSataDiskInfo[k].sDskInfo.nFree = pDskMgr->hinfo[x].disk_free;

							//printf("disktest 2, devpath: %s, total: %uKb, free: %uKb\n", 
							//	pSataDiskInfo[k].sDskInfo.strDevPath,
							//	pSataDiskInfo[k].sDskInfo.nTotal, pSataDiskInfo[k].sDskInfo.nFree);
						}

						printf("%s 2, nDevIdx: %d, phddinfo %s NULL, nTotal: %uKb, nFree: %uKb, nPartitionNo: %d, nStatus: %d\n",
							__func__, pSataDiskInfo[k].nDevIdx, phddinfo != NULL ? "!=" : "==",
							pSataDiskInfo[k].sDskInfo.nTotal,
							pSataDiskInfo[k].sDskInfo.nFree,
							pSataDiskInfo[k].sDskInfo.nPartitionNo,
							pSataDiskInfo[k].sDskInfo.nStatus);
					}
				}
			
				GetAllDiskInfo(sSataDiskInfoIns, nValidSataDiskCount, sUsbDiskInfoIns, nValidUsbDiskCount);
				
				SDiskInfo* pAllDiskInfo = sAllDiskInfoIns;
				
				SDevInfo sTmpDevInfoIns[nMaxSupportDiskNum];
				memset(sTmpDevInfoIns, 0, sizeof(sTmpDevInfoIns));
				
				for(k = 0; k < nValidDskCnr; k++)
				{
					memcpy(&sTmpDevInfoIns[k], &pAllDiskInfo[k].sDskInfo, sizeof(SDevInfo));
				}
				
				SStoreDevManage sStoreMgr;
				sStoreMgr.nDiskNum = nValidDskCnr;
				sStoreMgr.psDevList = sTmpDevInfoIns;
				printf("%s EM_SYSEVENT_DISKCHANGED\n", __func__);
				modSysCmplx_StorgeNotify(&sStoreMgr, EM_SYSEVENT_DISKCHANGED, 0);
				
				nSataChanged = 0;
				nUsbChanged = 0;
			}
		}
		
		//刷新磁盘信息后将格式化，备份标记置假
		modsys_SetDskAct(0, FALSE);//pModSys->bFormatCheck
		modsys_SetDskAct(2, FALSE);//pModSys->bBackupCheck
		modsys_SetDskAct(4, FALSE);//pModSys->bRefreshSata
	}
	
	return NULL;
}

s32 modsys_DskInfoUpdateInit()
{
	pthread_t id;
	pthread_create(&id, NULL, modsys_UpdateDiskInfoFxn, NULL);
	
	printf("modsys_DskInfoUpdateInit\n");
	
	return 0;
}

// 用于设置和清除用于控制磁盘查询线程的几个标记
// 主要是格式化和备份完成之后需要重新查询磁盘信息
// 格式化和备份中则暂停查询
// op - 0 format 1 formating 2 backup 3 backuping 4 bRefreshSata 5 bUpdate
// state - TRUE active, FALSE clear
//
void modsys_SetDskAct( u8 op, BOOL state )
{
	SModSyscplx *pModSys = &sModSysIns;
	
	if(mtxQueryDskMark) return;
	
	mtxQueryDskMark = 1;
	//pthread_mutex_trylock(&mtxQueryDskMark);
	
	switch(op)
	{
		case 0:
		{
			pModSys->bFormatCheck = state;
		} break;
		case 1:
		{
			pModSys->bFormating = state;
		} break;
		case 2:
		{
			pModSys->bBackupCheck = state;
		} break;
		case 3:
		{
			pModSys->bBackuping = state;
		} break;
		case 4:
		{
			pModSys->bRefreshSata = state;
		} break;
		case 5:
		{
			pModSys->bUpdate = state;			
		}
	}
	
	mtxQueryDskMark = 0;
	//pthread_mutex_unlock(&mtxQueryDskMark);
}

