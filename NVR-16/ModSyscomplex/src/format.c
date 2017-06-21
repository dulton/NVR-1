// file description
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <dirent.h>

#include "format.h"
#include "update.h"
#include "mydaemon.h"
#include "Hddcmd.h"

extern unsigned int nMaxSupportDiskNum;

//** macro

#define USB_FORMAT

#define USB_MAX_PTN_NUM 8
#define DEV_PATH_PFX	"/dev/sd"
#define REC_PATH_PFX	"rec/"

#define PRINT(s) printf("FUNC:%s, LINE:%d, %s\n", __FUNCTION__, __LINE__, s)

//** typedef
typedef struct 
{
	long long curpos;
	long long totalpos;		
} parition_prog;

//** local var
static SProgress sFormat;

unsigned char g_isFormatting = 0;

//** global var
extern SModSyscplx sModSysIns;

//** local functions

s32 modSysComplex_MountUsbDev(u8 nIdx);

//static u64 get_file_size_ex(const char *filename);

//static BOOL get_hdd_isunformated(int index);
//static int get_hdd_isfull(int index);
//static void pw_close(int *socket);

int get_disk_info_ex( HddInfo *hdd_manager, int  disk_index, u32* nPrtnNum );
//static int get_udisk_info_ex( HddInfo *hdd_manager, char* strDevPath, u8 nHddMax, u8* pnStrDevIdx );

//** global functions
//extern int GetStorageInfo(char *MountPoint, int *Capacity, int type);
extern s32 modsys_GetDevIdx( u8 nIdx );
extern SDevInfo* modsys_GetDiskInfo( u8 nIdx );
extern int update_disk_manager(disk_manager *hdd_manager, u8* pnDskCnr);
extern int get_parition_prog(void* prog);
extern void set_parition_prog(parition_prog* prog);

extern s32 modSysCmplx_DmInfoNotify(
	SDMInfo *psInfo,
	s32 	evt,
	s32 	emRslt
);

extern s32 modSysCmplx_UpdateNotify(
	SProgress *sProg,
	s32 	evt,
	s32 	emRslt );

extern int SendMsgToDaemon(my_daemon_param *pDaemon);

#define DISK_PARRITION_PERCENT	90 // 默认分区过程占格式化的90%时间

void* modsys_FormatFxn(void* p)
{
	printf("$$$$$$$$$$$$$$$$$$modsys_FormatFxn id:%d\n",getpid());
	
	pthread_detach(pthread_self());//csp modify 20121222
	
	//parition_prog sProg;
	//long long nLastPos = 0;
	u8 nLastProg = 0;
	
	u32 nWaitLen;
	SProgress sNewProg;
	
	sNewProg.nProgress = 0;
	sNewProg.lTotalSize = 0;
	sNewProg.lCurSize = 0;
	
	while(g_isFormatting)
	{
		struct timeval val;
		val.tv_sec  = 2;
		val.tv_usec = 0;
		select(0,NULL,NULL,NULL,&val);
		
		parition_prog sProg;
		get_parition_prog((void*)&sProg);
		if(sProg.totalpos == 0)
		{
			continue;
		}
		else if(sProg.totalpos != 2)//else if(sProg.totalpos > 5)//3T4T
		{
			printf("modsys_FormatFxn not valid count!\n");
		}
		
		nWaitLen = DISK_PARRITION_PERCENT/sProg.totalpos;
		
		if(sProg.curpos == sProg.totalpos)//条件满足则当做格式化完成
		{
			break;
		}
		
		#if 1//csp modify 20140824
		if(sProg.curpos > nLastProg)
		{
			sNewProg.nProgress = nWaitLen*sProg.curpos;
		}
		else
		{
			if(sNewProg.nProgress < nWaitLen*(sProg.curpos+1))
			{
				sNewProg.nProgress += 1;
			}
		}
		#else
		sNewProg.nProgress += 1;
		
		if(sNewProg.nProgress >= nWaitLen*(sProg.curpos+1))
		{
			sNewProg.nProgress = nWaitLen*(sProg.curpos+1);
		}
		else if(sProg.curpos > nLastProg) // 格式化进行到下一段分区，进度跳一次
		{
			sNewProg.nProgress = nWaitLen*(nLastProg+1);			
		}
		#endif
		
		nLastProg = sProg.curpos;
		
		sNewProg.lTotalSize = 0;
        
		modSysCmplx_UpdateNotify(&sNewProg, EM_SYSEVENT_FORMAT_RUN, 0);//format run
	}
	
	g_isFormatting = 0;
	
	return 0;
}

typedef struct
{
	unsigned int magic;			//幻数
	unsigned int opNums;		//
	unsigned int index;			//第N个索引文件
	unsigned int isAgain;		//是否开始覆盖
	unsigned int reserved1;		//
	unsigned int reserved2;		//
	unsigned int reserved3;		//
	unsigned int verify;		//校验和
}FileHeadInfo;

static int reset_log_index_head(FileHeadInfo *pInfo)
{
	if(pInfo == NULL)
	{
		return -1;
	}
	memset(pInfo, 0, sizeof(*pInfo));
	pInfo->magic = 0x1234ABCD;
	pInfo->index = 0;
	pInfo->isAgain = 0;
	pInfo->verify = pInfo->opNums + pInfo->index + pInfo->isAgain + pInfo->reserved1 + pInfo->reserved2 + pInfo->reserved3;
	return 0;
}

s32 modSysCmplx_DMFormat(char *pstrDevPath, EMFORMATWAY emFormatWay)
{
	int  			ret = 0;
	int  			disk_index = 0;
	int  			i;
	int  			count;
	u8	 			nDevPathLen;
	s32 			evt = 0, rslt = 0;
	SProgress*		pProg = NULL;
	HddInfo*		psHddInfo = NULL;
	disk_manager*	pHddMgr;
	char 			disk_name[64];
	char 			cmd[64];
	
	SStoreDevMgr 		sSdIns;
	SStoreDevManage 	sSDMIns;
	
	SDevInfo sDevList[nMaxSupportDiskNum];//zlb20111117 去掉malloc
	
	MODSYS_ASSERT(NULL!=pstrDevPath);
	
	pHddMgr = sModSysIns.pDiskManager;
	MODSYS_ASSERT(NULL!=pHddMgr);
	
	nDevPathLen = strlen(DEV_PATH_PFX);
	if(0 != strncmp(pstrDevPath, DEV_PATH_PFX, nDevPathLen))
	{
		pProg 	= NULL;
		evt 	= EM_SYSEVENT_FORMAT_INIT;
		rslt 	= -1;
		
		goto END;// invalid path
	}
	
	sSdIns.nMax = nMaxSupportDiskNum;
	sSDMIns.nDiskNum = 0;
	sSDMIns.psDevList = sDevList;
	
	if(!sSDMIns.psDevList)
	{
		pProg 	= NULL;
		evt 	= EM_SYSEVENT_FORMAT_INIT;
		rslt 	= -1;
		
		goto END;// invalid path
	}
	
	sSdIns.pDevMgr = &sSDMIns;
	
	// 根据设备路径找到磁盘信息结构数组中设备对应的结构
	if(0 == modSysCmplx_GetDMInfo(&sSdIns))
	{
		for(i=0; i<sSDMIns.nDiskNum; i++)
		{
			if(strcmp(sSDMIns.psDevList[i].strDevPath, pstrDevPath) == 0)
			{
				disk_index = pstrDevPath[nDevPathLen] - 'a';
				
				if(sSDMIns.psDevList[i].nType)//usb
				{
					my_daemon_param daemon;
					memset(&daemon, 0, sizeof(daemon));
					daemon.magic_no = MY_DAEMON_MAGIC;
					daemon.proc_type = MY_DAEMON_PROCTYPE_FORMAT;
					
					strcpy(daemon.arg1, pstrDevPath);
					SendMsgToDaemon(&daemon);
					
					sleep(5);
					
					if(sSDMIns.psDevList) free(sSDMIns.psDevList);
					
					// found usb device and start format
					pProg 	= NULL;
					evt 	= EM_SYSEVENT_FORMAT_DONE;
					rslt 	= 0;
					
					goto END; // usb device formated
				}			
				else//hdd
				{
					//MR HDD
					disk_index = find_disk_index(pHddMgr, disk_index);
					
					if ((disk_index != -1) && (disk_index < MAX_HDD_NUM))//这里肯定是有bug的
					{												
						psHddInfo = &pHddMgr->hinfo[disk_index];
					}
					else
					{
						printf("error: %s format disk_index=%d, out of range[0, %d)\n",
							__func__, disk_index, MAX_HDD_NUM);
					}
				}
				break;
			}
		}
	}
	
	if(NULL == psHddInfo)
	{
		printf("cannot find valid disk!\n");
		
		pProg 	= NULL;
		evt 	= EM_SYSEVENT_FORMAT_INIT;
		rslt 	= -1;
		
		goto END;
	}
	
	for(i=0; i<MAX_PARTITION_NUM; i++)
	{
		if(psHddInfo->is_partition_exist[i] == 1)
		{
			destroy_partition_index(&psHddInfo->ptn_index[i]);
			sprintf(cmd, "rec/%c%d", 'a'+disk_index, i+1);
			printf("umount_user path = %s\n",cmd);
			ret = umount_user(cmd);
			if(ret)
			{
				printf("device is busy!!\n");
				
				// found usb device and start format
				pProg 	= NULL;
				evt 	= EM_SYSEVENT_FORMAT_INIT;
				rslt 	= -1;
				
				goto END;
			}
		}
	}
	
	memset(psHddInfo->is_partition_exist, 0, sizeof(psHddInfo->is_partition_exist));
	
	psHddInfo->disk_total = 0;
	psHddInfo->disk_free = 0;
	
	count = MAX_PARTITION_NUM;
	
	sFormat.lCurSize = 0;
	sFormat.lTotalSize = count;
	
//#define PRINT_FORMAT_TIME
#ifdef PRINT_FORMAT_TIME
	struct timeval start,end;
	long span;
#endif
	
#ifdef PRINT_FORMAT_TIME
	gettimeofday(&start,NULL);
#endif
	
	//sprintf(disk_name, "/dev/sd%c", 'a' + disk_index);
	strcpy(disk_name, pstrDevPath);
	printf("format disk = %s\n", disk_name);
	
	g_isFormatting = 1;
	
	pthread_t thxdFormat;
	pthread_create(&thxdFormat, NULL, modsys_FormatFxn, NULL);
	
	ret = ifly_partition(disk_name, count, 0xc);//格式化磁盘，分区2个
	if(ret == -1)
	{
		// ifly_partition failed!!
		pProg 	= NULL;
		evt 	= EM_SYSEVENT_FORMAT_INIT;
		rslt 	= -1;
		
		goto END;
	}
	
#ifdef PRINT_FORMAT_TIME
	gettimeofday(&end,NULL);
	span = (end.tv_sec-start.tv_sec)*1000000+(end.tv_usec-start.tv_usec);
	printf("format span:%ld\n",span/1000);
#endif
	
	modSysCmplx_UpdateNotify(&sFormat, EM_SYSEVENT_FORMAT_INIT, 0); 
	for(i=0; i<count; i++)
	{
		//yaogang modify for bad disk
		//psHddInfo->is_partition_exist[i] = 1;
		
		//sprintf(disk_name,"/dev/sd%c%d", 'a' + disk_index,i+1);
		sprintf(disk_name, "%s%d", pstrDevPath, i+1);
		printf("format partition = %s\n", disk_name);
		
		sprintf(cmd,"rec/%c%d",'a'+disk_index,i+1);
		mkdir(cmd,1);
		
		modsys_MountUser(disk_name,cmd);
		
		ret = init_partition_index(&psHddInfo->ptn_index[i], cmd);
		if(ret == 1)
		{
			
			psHddInfo->disk_total += (get_partition_total_space(&psHddInfo->ptn_index[i])/KB);

			//yaogang modify for bad disk
			psHddInfo->is_partition_exist[i] = 1;
			
			if(i == 0)
			{
				char logFile[32] = {0};
				sprintf(logFile, "%s/log.hex", cmd);
				FILE* pFile = fopen(logFile,"wb+"); //在第一分区创建日志文件
				if(pFile != NULL)
				{
					char loginfo[60];
					memset(loginfo, 0, sizeof(loginfo));
					int k;
					for(k=0; k<0x4FF; k++)
					{
						fwrite(loginfo, sizeof(loginfo), 1, pFile);
					}
					fflush(pFile);
					fsync(fileno(pFile));
					
					fclose(pFile);
					pFile = NULL;
					
					printf("modSysCmplx_DMFormat: create %s\n", logFile);
				}

				#if 0//csp modify 20150110
				sprintf(logFile, "%s/log.dat", cmd);
				pFile = fopen(logFile,"wb+");
				if(pFile != NULL)
				{
					FileHeadInfo index_head;
					reset_log_index_head(&index_head);
					fwrite(&index_head, sizeof(index_head), 1, pFile);
					fwrite(&index_head, sizeof(index_head), 1, pFile);
					
					char loginfo[64];
					memset(loginfo, 0, sizeof(loginfo));
					int k;
					for(k=0; k<4096; k++)
					{
						fwrite(loginfo, sizeof(loginfo), 1, pFile);
					}
					fflush(pFile);
					fsync(fileno(pFile));
					
					fclose(pFile);
					pFile = NULL;
					
					printf("modSysCmplx_DMFormat: create %s\n", logFile);
				}
				#endif
			}
		}
		else if(ret == 0)
		{
			printf("\n\n*************error init_partition_index,disk_index=%d,i=%d,path:%s**********\n\n\n",disk_index,i,cmd);
			
			// init_partition_index failed!!!
			pProg 	= NULL;
			evt 	= EM_SYSEVENT_FORMAT_DONE;
			rslt 	= -1;

			//yaogang modify for bad disk
			//goto END;
			break;
		}
		
		sFormat.lTotalSize = 0;
		sFormat.nProgress = DISK_PARRITION_PERCENT + i*(100-DISK_PARRITION_PERCENT)/count;
		
		modSysCmplx_UpdateNotify(
			&sFormat,
			EM_SYSEVENT_FORMAT_RUN,
			0
		);
		//printf("hinfo[%d].total=%ld\n",disk_index,pHddMgr->hinfo[disk_index].total);
	}

	psHddInfo->is_disk_exist = 1;//xdc

	if (i < count)//期间出错
	{
		for(i = 0; i < MAX_PARTITION_NUM; i++)
		{	
			if (psHddInfo->is_partition_exist[i])
			{
				destroy_partition_index(&psHddInfo->ptn_index[i]);
				sprintf(cmd, "rec/%c%d", 'a'+disk_index, i+1);
				printf("umount_user path = %s\n",cmd);
				umount_user(cmd);

				psHddInfo->is_partition_exist[i] = 0;
			}
		}

		psHddInfo->disk_total = 0;
	}
	
	psHddInfo->disk_free = psHddInfo->disk_total;
	
	
END:
	// 格式化完毕将进度参数清零，避免下一次格式化进度线程启动后获取进度出错
	{
		g_isFormatting = 0;
		
		parition_prog sFmtProg;
		memset(&sFmtProg, 0, sizeof(sFmtProg));
		set_parition_prog(&sFmtProg);
	}
	
	if(rslt < 0)
	{
		modSysCmplx_UpdateNotify(pProg, evt, rslt);
	}
	else
	{
		sFormat.nProgress = 100;
		modSysCmplx_UpdateNotify(&sFormat, EM_SYSEVENT_FORMAT_RUN, 0); 
	}
	
	sModSysIns.bFormatCheck = TRUE;
	
	return rslt;
}

s32 modSysCmplx_GetDMInfo( SStoreDevMgr* psStoreDev )
{
	u8		nHddMax;
	u8		j;
	//u8	 disk_index = 0;
	//u32	nPrtnNum;
	//char 	cmd[48];
	disk_manager*	pHddMgr;
	//SDevInfo*		psDevInfo;
	SStoreDevManage* pStoreDev;
	
	MODSYS_ASSERT(NULL!=psStoreDev);
	MODSYS_ASSERT(0<psStoreDev->nMax);
	MODSYS_ASSERT(NULL!=psStoreDev->pDevMgr);
	
	pStoreDev = psStoreDev->pDevMgr;
	
	nHddMax = psStoreDev->nMax;
	pHddMgr = sModSysIns.pDiskManager;
	
	MODSYS_ASSERT(NULL!=pHddMgr);
	
	if(nHddMax > nMaxSupportDiskNum)
	{
		nHddMax = nMaxSupportDiskNum;
	}
	
	//csp modify 20121018
	while(sModSysIns.bRefreshSata)
	{
		usleep(1);
	}
	
	u8 nDiskFound = 0;
	//u8 nStrDevIdx = 0;
	
	//HddInfo* pHddInfo;
	//printf("nHddMax: %d\n", nHddMax);
	for(j=0; j<nHddMax; j++)
	{
		SDevInfo* pDevInfo = modsys_GetDiskInfo(j);
		if(pDevInfo)
		{
			
			memcpy(&pStoreDev->psDevList[nDiskFound++], pDevInfo, sizeof(SDevInfo));
		}
	}
	
	pStoreDev->nDiskNum = nDiskFound;
	
	//printf("##@@find disk %d\n", nDiskFound);
	
	return 0;
}

int get_disk_info_ex(HddInfo *pHddInfo, int disk_index, u32* pPrtnNum)
{
	int i, j;
	
	if(NULL==pPrtnNum)
	{
		return -1;
	}
	
	j = 0;
	*pPrtnNum = 0;
	
	//dbgprint("get_disk_info:i addr:0x%08x,disk_index addr:0x%08x,hdd_manager addr:0x%08x\n",(int)&i,(int)&disk_index,(int)&hdd_manager);
	printf("get_disk_info_ex\n");
	
	HddInfo *phinfo = pHddInfo;
	if(phinfo->is_disk_exist)
	{
		phinfo->disk_total = 0;
		phinfo->disk_free = 0;
		for(i=0;i<MAX_PARTITION_NUM;i++)
		{
			if(phinfo->is_partition_exist[i])
			{
				j++;
				phinfo->disk_total += (u32)(get_partition_total_space(&phinfo->ptn_index[i])/KB);
				phinfo->disk_free += (u32)(get_partition_free_space(&phinfo->ptn_index[i])/KB);
				//printf("HHHHHHHHHHHHHHH disk%d:total=%ld,free=%ld\n",disk_index,phinfo->total,phinfo->free);
			}
		}
		
		*pPrtnNum = j;
	}
	
	return 0;
}

#if 0
static u64 get_file_size_ex( const char *filename) 
{
	if(NULL == filename)
	{
		return -1;
	}
	
	struct stat buf; 
	if(stat(filename, &buf)<0) 
	{ 
		printf("stat check file %s\n", filename);
		perror("stat check file");
		
		return 0; 
	}
	
	u64 size = 0;
	
	if(S_ISDIR(buf.st_mode)) // 只检查第一层文件夹
	{
		DIR * dir;
		struct dirent * ptr;
		dir = opendir(filename);
		if(dir != NULL)
		{
			while((ptr = readdir(dir))!=NULL)
			{
				char szPath[128] = {0};
				if(realpath(filename,szPath))
				{
					size += get_file_size_ex(szPath);
					printf("~~~~~~~~~~~~~~~~~~~dir rest size:%lld\n", size);
				}
			}
		}
	}
	else
	{
		size = (u64)buf.st_size;
	}
	
	return size; 
}
#endif

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX 1024

/*
struct dirent{

ino_t d_ino;

ff_t d_off;

signed short int d_reclen;

unsigned char d_type;

char d_name[256];

};

*/

s32 modSysComplex_GetUsbDevDir(SUsbDirList* pInfo)
{	
	int i;//, j;
	//char devname[32] = {0};
	//char mountname[32] = {0};
	char bMountOk = 0;
	//u8 nDskCnr;
	//int ret;
	//DiskInfo sDskInfo;
	
	MODSYS_ASSERT(NULL!=pInfo);
	
	disk_manager*	pHddMgr;
	pHddMgr = sModSysIns.pDiskManager;
	
	//update_disk_manager(pHddMgr, &nDskCnr);
	
	//memset(pInfo, 0, sizeof(SUsbDirList));
	
	for(i=0; i<nMaxSupportDiskNum; i++)
	{
		SDevInfo* pDevInfo;
		pDevInfo = modsys_GetDiskInfo(i);
		if(pDevInfo)
		{
			if(pDevInfo->nType == 1)
			{
				if(modSysComplex_MountUsbDev(modsys_GetDevIdx(i))==0)
				{
					strcpy( pInfo->sInfo[0].szDir, "/root/myusb");
					//printf("strcpy( pInfo->sInfo[0].szDir end\n");
					
					bMountOk = 1;
					
					break;
				}
			}
		}
	}
	
	return (bMountOk?0:1);
}

s32 modSysComplex_GetUpdateFileInfo(EMUPGRADETARGET emTgt, SUpdateFileInfoList* pLstInfo)
{
	int i, nFileNum, ret;
    DIR *dir;
    struct dirent *ptr;
	SUpdateFileInfo* pUpFileInfo;
	//char mountname[32] = {0};
	char bMountOk = 0;
	//u8 nDskCnr;
	
	MODSYS_ASSERT(NULL!=pLstInfo);
	
	disk_manager* pHddMgr;
	pHddMgr = sModSysIns.pDiskManager;
	
	//update_disk_manager(pHddMgr, &nDskCnr);
	
	//printf("update_disk_manager\n");
	
	for(i=0; i<nMaxSupportDiskNum; i++)
	{
		SDevInfo* pDevInfo;
		pDevInfo = modsys_GetDiskInfo(i);
		if( pDevInfo )
		{
			if(pDevInfo->nType == 1)
			{
				if(modSysComplex_MountUsbDev(modsys_GetDevIdx(i))==0)
				{
					bMountOk = 1;
					break;
				}
			}
		}
	}
	
	//printf("mount finish\n");
	
	nFileNum = 0;
	
	if(bMountOk)
	{
		//printf("opendir start\n");
		
	    dir = opendir ("/root/myusb");//pDevInfo->strDevPath);
	    if (dir == NULL) 
	    {
			printf("fail to open dir\n");
			
			pLstInfo->nFileNum = 0;
	        return -1;
	    }
	    
		//printf("readdir start\n");
		
		struct stat buf;
	    while ( (ptr = readdir (dir)) != NULL) 
	    {
	        if ( strcmp(ptr->d_name, ".") == 0 	|| 
	        	 strcmp(ptr->d_name, "..") == 0	|| 
	        	 strlen(ptr->d_name) >= UPDATE_FILE_NAME_MAX
	        )
	        {
	            continue;
	        }
	        
			char szTmp[UPDATE_FILE_NAME_MAX+12] = { 0 };
			sprintf(szTmp, "/root/myusb/%s", ptr->d_name);
			if( (ret=lstat(szTmp, &buf))==0 )
			{
				if(S_ISDIR(buf.st_mode))
				{
					printf("found dir!!\n");
					continue;
				}
			}
			
			pUpFileInfo = &pLstInfo->pInfo[nFileNum];
			
			u8 		s = 0, t 		= 0;
			u8 		nFileNameLen 	= 0;
			char 	szExt[6]		= {0};
			char    szExtRef[6] 	= {0};
			char    szExtRef1[6] 	= {0};
			char    szNameRef[10]	= {0};
			u32		nLimitSize		= 100*MB;
			switch(emTgt)
			{
				case EM_UPGRADE_MAINBOARD:
				{
					strcpy(szExtRef, "bin");
					strcpy(szExtRef1, "bin");
				} break;
				case EM_UPGRADE_PANEL:
				{
					strcpy(szNameRef, "pannel");
					strcpy(szExtRef1, "bin");
					strcpy(szExtRef, "bin");
				} break;
				case EM_UPGRADE_STARTLOGO:
				case EM_UPGRADE_APPLOGO:
				{
					strcpy(szExtRef, "jpg");
					strcpy(szExtRef1, "jpeg");
					nLimitSize = 128*1024;
				} break;
				default:
				{
				    closedir(dir);
					printf("not support type ! \n");
					return -1;
				}
			}

       		nFileNameLen = strlen(ptr->d_name);
       		
			u8 bIlegalNmae = 0;
       		for(i=0; i<nFileNameLen; i++)
       		{
				if(ptr->d_name[i] == '?')
				{
					bIlegalNmae = 1;
					break;
				}
       		}
       		
       		//printf("file name %s d_reclen %d\n", ptr->d_name, nFileNameLen);
			//if(strncmp (ptr->d_name, "mainboard", 9)==0)
			//文件名长度不能超过30字节,英文为30,中文为10
			if( !bIlegalNmae                        &&
				nFileNameLen < UPDATE_FILE_NAME_MAX && 
				buf.st_size > 0 					&& 	// 0字节文件
				buf.st_size < nLimitSize)				// 超过限定尺寸的升级文件不显示
			{
				// get ext str
				//
				while(s < nFileNameLen)
				{
					if(t!=0) 
						szExt[s-t-1] = ptr->d_name[s];
						
					if(ptr->d_name[s]=='.')
					{
						t=s;
					}

					s++;
				} 
				
				if(strlen(szExt) > UPDATE_FILE_EXT_MAX)
				{
					continue;
				}

				if( strncmp (szExt, szExtRef, strlen(szExtRef))==0   ||
					strncmp (szExt, szExtRef1, strlen(szExtRef1))==0
				)
				{
					strcpy(pUpFileInfo->szName, ptr->d_name);
					strcpy(pUpFileInfo->szExt, szExt);
					
					pUpFileInfo->nSize 			= buf.st_size;
					pUpFileInfo->nTimeStamp 	= buf.st_mtime;

					/*
					printf("[MODSYS] Found type %d update file ...\n", emTgt);
					printf("[MODSYS] file name: %s\n", pUpFileInfo->szName);
					printf("[MODSYS] file ext : %s\n", pUpFileInfo->szExt);
					printf("[MODSYS] file size: %d\n", pUpFileInfo->nSize);
					printf("[MODSYS] file mstamp: %d\n", pUpFileInfo->nTimeStamp);
					*/
					
					nFileNum++;
				}
			}
			else
				continue;
			
	        if(nFileNum>=pLstInfo->nMax)
	        {
				pLstInfo->nFileNum = pLstInfo->nMax;
				
	        	//printf("too many upgrade files!!\n");
	        	
				break;
	        }
	        else
	        {
				pLstInfo->nFileNum = nFileNum;		
	       	}
	    }
	    
	    closedir(dir);
		return 0;
	}
	else
		return -1;
}

s32 modSysComplex_GetConfigFileInfo(char* szDir, SUpdateFileInfoList* pLstInfo)
{
	int nFileNum, ret;
    DIR *dir;
    struct dirent *ptr;
	SUpdateFileInfo* pUpFileInfo;
	//char devname[32] = {0};
	//char mountname[32] = {0};
	//char bMountOk = 0;
	
	MODSYS_ASSERT(NULL!=pLstInfo);
	
	nFileNum = 0;

	//printf("opendir start %s\n", szDir);
		
	struct stat buf;

    dir = opendir (szDir);//pDevInfo->strDevPath);
 
    if (dir == NULL) 
    {
		printf("fail to open dir\n");
		
		pLstInfo->nFileNum = 0;
		
        return -1;
    }
    
	//printf("readdir start\n");
	
    while ( (ptr = readdir (dir)) != NULL) {
        if (strcmp (ptr->d_name, ".") == 0 || strcmp (ptr->d_name, "..") == 0)
            continue;

		#if 1
		char szTmp[64] = {0};
		sprintf(szTmp, "%s/%s", szDir, ptr->d_name);
		if( (ret=lstat(szTmp, &buf))==0 )
		//if( (ret=lstat(ptr->d_name, &buf))==0 )
		{
			if(S_ISDIR(buf.st_mode))
			{
				//printf("found dir!!\n");
				continue;
			}
		}
		else
		{
			//printf("lstat failed err %d, errno %d\n", ret, errno);
			
			//printf("err msg %s\n", strerror(errno));
		}
        #endif
        
		//printf("getfile ...\n");
		
		pUpFileInfo = &pLstInfo->pInfo[nFileNum];
		
		u8 s = 0, t = 0;
		char szExt[8] = {0};//char szExt[4] = {0};//3T4T
		u8 nFileNameLen = 0;
		
   		nFileNameLen = strlen(ptr->d_name);
   		
   		//printf("file name %s d_reclen %d\n", ptr->d_name, nFileNameLen);
		if(strncmp (ptr->d_name, "config", 6)==0)
		{
			// get ext str
			//
			while(s < nFileNameLen)
			{
				if(t!=0) 
					szExt[s-t-1] = ptr->d_name[s];
				
				if(ptr->d_name[s]=='.')
				{
					t=s;
				}
				
				s++;
			}

			if(strncmp (szExt, "ini", 3)==0)
			{
				strcpy(pUpFileInfo->szName, ptr->d_name);
				strcpy(pUpFileInfo->szExt, szExt);
				
				//stat(ptr->d_name, &buf);
				pUpFileInfo->nSize = buf.st_size;
				pUpFileInfo->nTimeStamp = buf.st_mtime;

				/*
				printf("[MODSYS] Found config file ...\n");
				printf("[MODSYS] file name: %s\n", pUpFileInfo->szName);
				printf("[MODSYS] file ext : %s\n", pUpFileInfo->szExt);
				printf("[MODSYS] file size: %d\n", pUpFileInfo->nSize);
				printf("[MODSYS] file mstamp: %d\n", pUpFileInfo->nTimeStamp);
				*/
				
				nFileNum++;
			}
		}
		
        if(nFileNum>=pLstInfo->nMax)
        {
			pLstInfo->nFileNum = pLstInfo->nMax;
			
        	//printf("too many config files!!\n");
        	
			break;
        }
        else
			pLstInfo->nFileNum = nFileNum;
        	
       	
		//printf("file deal part ...\n");
    }
    
    closedir(dir);
	
	return 0;
}

s32 modSysComplex_MountUsbDev(u8 nIdx)
{
	int j;
	char devname[32] = {0};
	
	s32 rtn = 0;
	s32 ret = 0;
	
	if(0==access("myusb", F_OK))
	{
		printf("myusb dir exist!!1\n");
		if( umount_user("myusb") )
		{
			unlink("myusb");
			mkdir("myusb", 1);			
		}
	}
	else
	{
		unlink("myusb");
		mkdir("myusb", 1);
	}
	
	for(j=1; j<=USB_MAX_PTN_NUM; j++)
	{
		sprintf(devname,"/dev/sd%c%d",'a'+nIdx,j);
		ret = modsys_MountUser(devname,"myusb");
		if(ret == 0) break;
		//printf("mount %s failed.\n", devname);
	}

	if(ret == 0)
	{
		rtn = 0;
		goto END;
	}
	
	sprintf(devname,"/dev/sd%c",'a'+nIdx);		
	if( modsys_MountUser(devname, "myusb")==0 )
	{
		rtn = 0;
		goto END;
	}

	rtn = -1;
END:
	//printf("mount %s rtn %d\n", devname, rtn);
	
	return rtn;
}

int modsys_MountUser(char *mounted_path, char *user_path)
{
	if(mounted_path == NULL || user_path == NULL)
	{
		return -1;
	}
	//printf("*****************mount_user: before lstat. path:%s\n", mounted_path);
	
	struct stat st;
	if(lstat(mounted_path, &st))
	{
		//printf("mount_user:%s is not exist\n",mounted_path);
		return -1;
	}
	//printf("*****************mount_user: before realpath\n");
	
	char path[256];
	if(realpath(user_path,path) == NULL)
	{
		//printf("mount_user:realpath failed\n");
		return -1;
	}
	//printf("*****************mount_user: path: %s\n", pathbiz);
	//printf("mount_user:path:%s\n",path);
	int curstat = mount(mounted_path, path, "vfat", 32768, NULL);
	//int curstat = mount(mounted_path, path, "msdos", 32768, NULL);
	if(curstat)
	{
		//if(curstat == 16)//Device or resource busy
		if(errno == 16)//Device or resource busy
		{
			printf("mount_user:mount %s to %s failed1,errcode:%d,errstr:%s\n",mounted_path,path,errno,strerror(errno));
			//return 0;
			return -2;//wrchen 090728
		}
		else
		{
			printf("mount_user:mount %s to %s failed2,errcode:%d,errstr:%s\n",mounted_path,path,errno,strerror(errno));
			return -1;
		}
	}
	//printf("mount_user:mount %s to %s success\n",mounted_path,path);
	
	return 0;
}


