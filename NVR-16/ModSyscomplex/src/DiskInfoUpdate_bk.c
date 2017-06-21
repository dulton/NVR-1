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

#ifndef HISI_3515
#define HISI_3515
#endif

// disk info struct
typedef struct 
{
	int	 	 nDevIdx;	// <STR_DEV_MAX valid dev, -1 empty
	u8		 nState; 	// 0 checking, 1 valid, 2 empty
	SDevInfo sDskInfo;
} SDiskInfo;


static pthread_mutex_t mtxFreshDskInf = PTHREAD_MUTEX_INITIALIZER;
//static pthread_mutex_t mtxQueryDskMark = PTHREAD_MUTEX_INITIALIZER;
static char mtxQueryDskMark = 0;

static u8 nValidDskCnr = 0;

static SDiskInfo sDiskInfoIns[MAX_HDD_NUM];

static u8 bInsertDisk = 0xff;

extern SModSyscplx sModSysIns;

// function 
void modsys_SetDskAct( u8 op, BOOL state );

extern int get_disk_info_ex( HddInfo *hdd_manager, int  disk_index, u32* nPrtnNum );
int GetStorageInfo(char * MountPoint, int *Capacity, int type);
extern s32 modSysComplex_MountUsbDev(u8 nIdx);

BOOL is_sata_disk_ex(int fd)
{
	char host[64];
	
	*(int*)host = 63;
	if(ioctl(fd, SCSI_IOCTL_PROBE_HOST, &host) < 0)
	{
		perror("SCSI_IOCTL_PROBE_HOST failed");
		return FALSE;
	}

	//hdd:host=sata_sil hi3515-ahci-device
	//usb:host=SCSI emulation for USB Mass Storage devices
	host[63] = '\0';
	//printf("SCSI_IOCTL_PROBE_HOST:%s\n", host);
	
	if((strstr(host, "ahci")) || (strstr(host, "sata_sil")) || (strstr(host, "hi3515-ahci-device")) || (strstr(host, "atp862x_His")))
	{
		return TRUE;
	}
	
	return FALSE;
}

//s32 nPosHddProg=0;


BOOL is_usb_disk_ex(int fd)
{
	char host[64];
	
	*(int*)host = 63;
	if (ioctl(fd, SCSI_IOCTL_PROBE_HOST, &host) < 0)
	{
		perror("SCSI_IOCTL_PROBE_HOST failed");
		return FALSE;
	}
	
	//hdd:host=sata_sil 
	//usb:host=SCSI emulation for USB Mass Storage devices
	host[63] = '\0';
	
	//printf("SCSI_IOCTL_PROBE_HOST:%s\n", host);
	
	if (strstr(host, "USB") || strstr(host, "usb"))
	{
		return TRUE;
	}
	
	return FALSE;
}

#if 1
s32 modSys_SetDskInfo(SDiskInfo* pDskInfo)
{
	if(!pDskInfo) return -1;
	
	pthread_mutex_trylock(&mtxFreshDskInf);
	
	int j;
	int	nIdleSlot = -1;
	SDiskInfo* pTmpInfo = sDiskInfoIns;
	
	printf("modSys_SetDskInfo %d %d %d \n", pDskInfo->nDevIdx, pDskInfo->sDskInfo.nTotal, pDskInfo->sDskInfo.nFree);
	
	for(j=0; j<MAX_HDD_NUM; j++)
	{
		if( (nIdleSlot < 0) && (pTmpInfo[j].nDevIdx < 0) )
		{
			nIdleSlot = j;
		}
		
		if(pTmpInfo[j].nDevIdx == pDskInfo->nDevIdx)
		{
			memcpy(&pTmpInfo[j], pDskInfo, sizeof(SDiskInfo));

			break;
		}
	}

	if(j >= MAX_HDD_NUM)
	{
		memcpy(&pTmpInfo[j], pDskInfo, sizeof(SDiskInfo));
	}
	
	pthread_mutex_unlock(&mtxFreshDskInf);

	return 0;
}

s32 modsys_ClearDskInfo( u8 nIdx )
{
	SDiskInfo* pDskInfo = sDiskInfoIns;
	
	int j;
	for(j=0; j<MAX_HDD_NUM+1; j++)
	{
		if(pDskInfo[j].nDevIdx == nIdx)
		{
			//printf("modsys_ClearDskInfo %d %d %d \n", 
			//	pDskInfo[j].nDevIdx, pDskInfo[j].sDskInfo.nTotal, pDskInfo[j].sDskInfo.nFree);
			
			pDskInfo[j].nDevIdx = -1;
			pDskInfo[j].nState = 2;
			
			// notify gui, some disk lost
			SDMInfo sInfo;
			sInfo.nType = pDskInfo[j].sDskInfo.nType;
			sInfo.nTotal = -1; // 通知界面清除此磁盘信息
			strcpy(sInfo.szMountPoint, pDskInfo[j].sDskInfo.strDevPath);
			modSysCmplx_DmInfoNotify(&sInfo, EM_SYSEVENT_GETDMINFO, 0);	
			
			bInsertDisk = j;
			
			break;
		}
	}

	return 0;
}

SDevInfo* modsys_GetDiskInfo( u8 nIdx )
{
	SDiskInfo* pDskInfo = sDiskInfoIns;
	
	if(nIdx >= nValidDskCnr)
	{
		return NULL;
	}

	//csp modify 20121018
	//if(pDskInfo[nIdx].nState == 2)
	if(pDskInfo[nIdx].nDevIdx == -1 || pDskInfo[nIdx].nState == 2)
	{
		return NULL;
	}
	
	return &pDskInfo[nIdx].sDskInfo;
}

s32 modsys_GetDevIdx( u8 nIdx )
{
	SDiskInfo* pDskInfo = sDiskInfoIns;

	if(nIdx >= nValidDskCnr)
		return -1;
	
	return pDskInfo[nIdx].nDevIdx;
}

BOOL modsys_CheckDskInfo()
{
	BOOL bRslt = TRUE;
	
	//备份进行中则不检查
	if(sModSysIns.bBackuping)
	{
		bRslt = FALSE;
	}
	
	//csp modify 20121018
	//格式化过程中中或正在升级则不检查
	if(sModSysIns.bFormating || sModSysIns.bUpdate)
	{
		bRslt = FALSE;
	}
	
	return bRslt;
}

u8 modsys_GetInsertDiskStatue()
{
	return 0;
}

void* modsys_UpdateDiskInfoFxn(void* p)
{
	printf("$$$$$$$$$$$$$$$$$$modsys_UpdateDiskInfoFxn id:%d\n",getpid());
	
	int  		ret;
	DiskInfo 	dinfo;
	int  		i,j;
	int  		fd = -1;
	char 		diskname[64];
	//char		devname[64];
	//char 		mountname[64];
	//char 		path2[64]={0};
	u8			bSata = FALSE, bUdsk = FALSE;
	u8			nDskFound = 0;
	u32			nPrtnNum = 0;
	
	memset(sDiskInfoIns, 0, sizeof(sDiskInfoIns));
	//csp modify 20121018
	for(i=0; i<MAX_HDD_NUM; i++)
	{
		sDiskInfoIns[i].nDevIdx = -1;
		sDiskInfoIns[i].nState = 2;
	}
	
	SDiskInfo* pDskInfo = sDiskInfoIns;
	
	disk_manager* pDskMgr = sModSysIns.pDiskManager;
	
	printf("enter modsys_UpdateDiskInfoFxn\n");
	
	while(1)
	{
		#if 1//csp modify 20121018
		//sleep(1);
		int n = 0;
		for(n=0;n<100;n++)
		{
			usleep(10*1000);
			if(sModSysIns.bRefreshSata)
			{
				break;
			}
		}
		#else
		struct timeval val;
		val.tv_sec  = 1;
		val.tv_usec = 0;
		select(0,NULL,NULL,NULL,&val);
		#endif
		
		if(!modsys_CheckDskInfo()) continue; 
		
		nDskFound = 0;
		
		for(j=0; j<STR_DEV_MAX; j++)
		{
			SDMInfo sInfo;
			
			#ifdef HISI_3515
			sprintf(diskname,"/dev/sd%c",'a'+j);
			#else
			sprintf(diskname,"/dev/scsi/host%d/bus0/target0/lun0/disc",j);
			#endif
			
			//printf("disk name check %s\n", diskname);
			
			fd = open(diskname, O_RDONLY);
			if(fd == -1)
			{
				//printf("update_disk_manager %s is not exist\n",diskname);
				
				modsys_ClearDskInfo(j);
				
				//printf("disk name check %s 2\n", diskname);
				
				continue;
			}
			else
			{
				bSata = is_sata_disk_ex(fd);
				bUdsk = is_usb_disk_ex(fd);

				//yaogang modify
				if (bUdsk)
					printf("found udisk %d\n", j);
				
				if( !bSata && !bUdsk )
				{
					//printf("disk name check %s 3\n", diskname);
					close(fd);
					continue;
				}
				printf("yg found 1\n");
				//update hdd info
				for(i=0; i<MAX_HDD_NUM; i++)
				{
					if(pDskInfo[i].nState == 1 && pDskInfo[i].nDevIdx == j)
					{
						break;//info is valid
					}
				}
				printf("yg found 2\n");
				// 找到有效地硬盘，
				// 并且 硬盘格式化标记，数据备份标记为假则认定磁盘信息未变
				// 并且 没有请求非u盘容量信息
				if( ((i < MAX_HDD_NUM) 				&& 
					  !sModSysIns.bFormatCheck 	    && 
					  !sModSysIns.bBackupCheck      && 
					  !sModSysIns.bRefreshSata)     || 
					  sModSysIns.bUpdate)
				{
					nDskFound++;
					//printf("disk name check %s 4\n", diskname);
					close(fd);
					continue;
				}
				printf("yg found 3\n");
				ret = ifly_diskinfo(diskname, &dinfo);
				if(ret != 0)
				{
					//printf("update_disk_manager %s is not exist 3\n",diskname);
					close(fd);
					continue;
				}
				
				//printf("disk %d dev %s is not exist 3\n",bSata, diskname);
				printf("yg found 4\n");
				//refresh
				SDevInfo* 	psDevInfo = &pDskInfo[nDskFound].sDskInfo;
				
				HddInfo*	pHddInfo = NULL;
				if(j<MAX_HDD_NUM)//若程序运行时拔插磁盘，则不再检测此磁盘路径下的信息
				{
					pHddInfo = &pDskMgr->hinfo[j];
				}
				else
				{
					//printf("check disk info line %d \n",__LINE__);
					close(fd);
					continue;
				}
				printf("yg found 5\n");
				sprintf(psDevInfo->strDevPath, "/dev/sd%c", 'a'+j);
				
				pHddInfo->is_disk_exist = 1;
				
				ret = -1;
				if(bSata)
				{
					pDskInfo[nDskFound].nState = 0;
					psDevInfo->nType = 0;
					
					if( 0 == get_disk_info_ex(
						pHddInfo, 
						0, 
						&nPrtnNum
					) )
					{
						psDevInfo->nPartitionNo = nPrtnNum;
						
						psDevInfo->nFree = pHddInfo->free;				
						psDevInfo->nTotal = pHddInfo->total;
						
						if(psDevInfo->nTotal == 0)
						{
							psDevInfo->nStatus = 1;		//0 ok; 1 unformated; 2 err
						}
						else
						{
							psDevInfo->nStatus = 0;
						}
					}
					else
					{
						psDevInfo->nStatus = 2;
					}
					ret = 0;
				}
				else if(bUdsk && !sModSysIns.bUpdate)//update udisk info
				{
					pDskInfo[nDskFound].nState = 0;
					psDevInfo->nType = 1;

					printf("yg found 6\n");
					
					if(bInsertDisk!=0xff || !sModSysIns.bRefreshSata) //主动获取硬盘信息请求到来时，仅刷新sata磁盘信息，u盘信息保持不变
					{
						printf("yg found 7\n");
						psDevInfo->nType 		= 1;
						
						pHddInfo->storage_type 	= 'u';
						pHddInfo->total = dinfo.capability/KB;
						
						sInfo.nType = psDevInfo->nType;
						sInfo.nTotal = pHddInfo->total;
						sInfo.nFree = -1; //剩余空间检测中
						strcpy(sInfo.szMountPoint, psDevInfo->strDevPath);
						modSysCmplx_DmInfoNotify(&sInfo, EM_SYSEVENT_GETDMINFO, 0);
						
						usleep(1000);
						
						umount_user("myusb");
						if( 0 == modSysComplex_MountUsbDev(j) )
						{
							printf("##hard occasion!!\n");
							
							struct statfs s;
							int rtn = statfs("myusb", &s);
							
							long long totalspace = 0;
							long long freespace = 0;
							if(rtn >= 0)
							{
								freespace = (((long long)s.f_bsize * (long long)s.f_bfree) / (long long)KB);
								totalspace = (((long long)s.f_bsize * (long long)s.f_blocks) /(long long)KB);
								
								psDevInfo->nTotal = totalspace;
								psDevInfo->nFree = freespace;
								psDevInfo->nStatus = 0;
								
								ret = 0;
							}
							else
							{
								psDevInfo->nTotal = 0;
								psDevInfo->nFree = 0;
								psDevInfo->nStatus = 0;
							}
						}
						
						bInsertDisk = 0xff;
					}
					else
					{
						ret = 0;
					}
				}
				
				if(ret != 0)
				{
					close(fd);
					continue;
				}
				
				pDskInfo[nDskFound].nDevIdx = j;
				
				printf("found %s type %d total %d free %d\n", psDevInfo->strDevPath, psDevInfo->nType, psDevInfo->nTotal, psDevInfo->nFree);
				
				sInfo.nType = psDevInfo->nType;
				sInfo.nTotal = psDevInfo->nTotal;
				sInfo.nFree = psDevInfo->nFree;
				strcpy(sInfo.szMountPoint, psDevInfo->strDevPath);
				
				//printf("modSysCmplx_DmInfoNotify1\n");
				
				modSysCmplx_DmInfoNotify(&sInfo, EM_SYSEVENT_GETDMINFO, 0);
				usleep(1000);
				
				close(fd);
				
				pDskInfo[nDskFound].nState = 1;
				
				nDskFound++;
				if(nDskFound >= MAX_HDD_NUM)
				{
					break;
				}
			}
		}
		
		nValidDskCnr = nDskFound;
		
		//刷新磁盘信息后将格式化，备份标记置假
		modsys_SetDskAct(0, FALSE);
		modsys_SetDskAct(2, FALSE);
		modsys_SetDskAct(4, FALSE);
	}
	
	return NULL;
}
#endif

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

int GetStorageInfo(char * MountPoint, int *Capacity, int type)
{
	struct statfs statFS;
	u64 usedBytes = 0;
	u64 freeBytes = 0;
	u64 totalBytes = 0;
	u64 endSpace = 0;

	if (statfs(MountPoint, &statFS) == -1){
		printf("statfs failed for path->[%s]\n", MountPoint);
		return(-1);
	}

	totalBytes = (u64)statFS.f_blocks * (u64)statFS.f_bsize;
	freeBytes = (u64)statFS.f_bfree * (u64)statFS.f_bsize;
	usedBytes = (u64)(totalBytes - freeBytes);

	switch( type )
	{
		case 1:
		endSpace = totalBytes/KB;
		break;

		case 2:
		endSpace = usedBytes/KB;
		break;

		case 3:
		endSpace = freeBytes/KB;
		break;

		default:
		return ( -1 );
	}
	
	*Capacity = endSpace;
	return 0;
}
