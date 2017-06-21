#include "common_msgcmd.h"
#include "update.h"
#include "backup.h"
#include "search.h"
#include "hddcmd.h"
#include "mydaemon.h"


#define BACKUP_AS_AVI
#ifdef BACKUP_AS_AVI
#include "avilib.h"
#include <time.h>
#endif

#include <sys/vfs.h>

#include "custommp4.h"
#include "Mod_syscomplex.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#ifndef USE_AUDIO_PCMU
#define USE_AUDIO_PCMU
#endif

extern unsigned int nMaxSupportDiskNum;

// file description

//** macro

#define SOCKET_ERROR			-1

#define USB_NOERROR 0
#define USB_ERROR_WRITE_ACCESS 1
#define USB_ERROR_WRITE_FULL 2
#define USB_ERROR_WRITE_CANCEL 3

//** typedef
typedef struct
{
	u64 curpos;
	u64 totalpos;
	int cancel;
}PACK_NO_PADDING ifly_copy_progress_t;

//** local var
static ifly_copy_progress_t tCopy;

//** global var
extern SModSyscplx sModSysIns;
extern s32 modSysCmplx_UpdateNotify(
				SProgress *sProg,
				s32 	evt,
				s32 	emRslt );

//** local functions

//** global functions
extern SDevInfo* modsys_GetDiskInfo( u8 nIdx );
extern int ifly_diskinfo(char *diskname,DiskInfo *inf);
int SendMsgToDaemon(my_daemon_param *pDaemon);

static int TimeZoneOffset[] = 
{
	-12*3600,
	-11*3600,
	-10*3600,
	-9*3600,
	-8*3600,
	-7*3600,
	-6*3600,
	-5*3600,
	-4*3600-1800,
	-4*3600,
	-3*3600-1800,
	-3*3600,
	-2*3600,
	-1*3600,
	0,
	1*3600,
	2*3600,
	3*3600,
	3*3600+1800,
	4*3600,
	4*3600+1800,
	5*3600,
	5*3600+1800,
	5*3600+2700,
	6*3600,
	6*3600+1800,
	7*3600,
	8*3600,
	9*3600,
	9*3600+1800,
	10*3600,
	11*3600,
	12*3600,
	13*3600,
};

static int GetTZOffset(int index)
{
	if(index < 0 || index >= sizeof(TimeZoneOffset)/sizeof(TimeZoneOffset[0]))
	{
		return 0;
	}
	
	return TimeZoneOffset[index];
}

//void* wait_backup(void* arg)
#define BAK_BUFSIZE (128 << 10)
#define SNAP_BUFSIZE (10 << 10)


s32 modSysCmplx_Backup_USB_JPEG( u32 nFileNum, SRecSnapInfo* pRecInfoList)
{
	u32 open_offset;
	u8  chn;
	struct tm tm_time;
	time_t t_time;
	int filesize;
	u8  errcode = 0;
	u64 totalsize = 0;
	char usb_device[32];
	FILE *pFile1 = NULL;
	FILE *pFile2 = NULL;
	int index, i;
	unsigned char buf[BAK_BUFSIZE];//zlbfix20110719 backup
	char filename[64];
	char ufilename[128];
	char tmpfile[64];
	BOOL haveerror = FALSE;
	int ret = -1;
	DiskInfo dinfo;
	SEventPara sEventParaIns;
	
	if(pRecInfoList==NULL)
	{
		printf("%s NULL file list!\n", __func__);

		return -1;//
	}
	
	EvNotify(sModSysIns.pFNSysNotifyCB, EM_SYSEVENT_BACKUP_INIT, &sEventParaIns);
	
	mkdir("myusb",1);
	MODSYS_DEBUG_STR("before while.\n");
	
	ret = get_usb_device(usb_device);
	if(ret != 0)
	{
		sEventParaIns.emResult = EM_BACKUP_FAILED_UNKNOW;
		EvNotify(sModSysIns.pFNSysNotifyCB, EM_SYSEVENT_BACKUP_DONE, &sEventParaIns);
		return 0;
	}

	ret = ifly_diskinfo(usb_device,&dinfo);
	if(ret == 0)
	{
		printf("%s found usb disk:capability:%lld B,%dM\n",__func__, dinfo.capability,(int)(dinfo.capability/(1024*1024)));
	}

	for(index=0; index<nFileNum; index++)
	{
		get_snap_file_name((recsnapinfo_t *)&pRecInfoList[index],filename,&open_offset);
		totalsize += (u64)pRecInfoList[index].nSize;
		
		printf("%s filename %s size %d\n", __func__, filename, pRecInfoList[index].nSize);
		
	}
	printf("%s totalsize: %llu\n", __func__, totalsize);

	tCopy.cancel = 0;
	tCopy.curpos = 0;
	tCopy.totalpos = totalsize;

	SDevInfo* pInfo = NULL;
	for(i=0; i<nMaxSupportDiskNum; i++)
	{
		//printf("%s i: %d\n", __func__, i);
		pInfo = modsys_GetDiskInfo(i);
		if(pInfo && pInfo->nStatus==HDD_IDLE && pInfo->nType==1)
		{
			break;
		}
	}
	if (NULL == pInfo || pInfo->nType !=1)
	{
		printf("%s not found USB disk\n", __func__);
		return -1;
	}
	printf("%s: pInfo->nFree: %lld, totalsize: %lld\n", __func__, pInfo->nFree*(u64)KB, totalsize);
	
	if((u64)pInfo->nFree*(u64)KB < totalsize)
	{
		//printf("%s yg 2\n", __func__);
		umount_user("myusb");
		
		sEventParaIns.emResult = EM_BACKUP_FAILED_NOSPACE;
		EvNotify(sModSysIns.pFNSysNotifyCB, EM_SYSEVENT_BACKUP_DONE, &sEventParaIns);
		
		return -1; // too big files
	}

	memset(&sEventParaIns, 0, sizeof(sEventParaIns));

	sEventParaIns.sProgress.lTotalSize = tCopy.totalpos;

	for(index=0; index<nFileNum; index++)
	{
		get_snap_file_name((recsnapinfo_t *)&pRecInfoList[index],filename,&open_offset);
		pFile1 = fopen(filename,"rb");
		if(pFile1 == NULL)
		{
			printf("%s open recfile error\n", __func__);
			haveerror = TRUE;
			errcode = USB_NOERROR;
			goto END;
		}
		
		fseek(pFile1,open_offset,SEEK_SET);

		chn = pRecInfoList[index].nChn;
		
		t_time = pRecInfoList[index].nStartTime;
		
		//csp modify 20131213
		t_time += GetTZOffset(sModSysIns.nTimeZone);
		
		//memcpy(&tm_time,localtime(&t_time),sizeof(tm_time));
		localtime_r(&t_time, &tm_time);
		sprintf(tmpfile,"chn%02d_%04d%02d%02d_%02d%02d%02d",
				chn+1,
				tm_time.tm_year+1900,
				tm_time.tm_mon+1,
				tm_time.tm_mday,
				tm_time.tm_hour,
				tm_time.tm_min,
				tm_time.tm_sec);

		sprintf(ufilename, "myusb/%s.jpg", tmpfile);	//08-01-18
		printf("%s ufilename = %s\n", __func__, ufilename);
		
		pFile2 = fopen(ufilename, "wb");
		if(pFile2 == NULL)
		{
			printf("%s open usb file error\n", __func__);
			haveerror = TRUE;
			errcode = USB_ERROR_WRITE_ACCESS;
			goto END;
		}

		int readlen = 0;
		filesize = pRecInfoList[index].nSize;
		readlen = fread(buf, 1, filesize, pFile1);
		if (readlen != filesize)
		{
			haveerror = TRUE;
			errcode = USB_NOERROR;
			printf( "%s fread readlen: %d, filesize: %d\n", __func__, readlen, filesize);
			printf( "%s fread error,errcode=%d,errstr=%s\n", __func__, errno, strerror(errno) );
			break;
		}

		int writelen = 0;
		writelen = fwrite(buf, 1, filesize,pFile2);
		if (writelen != filesize)
		{
			haveerror = TRUE;
			errcode = USB_ERROR_WRITE_FULL;
			if(errno == 5)//Input/output error
			{
				errcode = USB_ERROR_WRITE_ACCESS;
			}
			printf( "%s Write error,errcode=%d,errstr=%s\n", __func__, errno, strerror(errno) );
			break;
		}
		/**********************/
END:
		if(pFile1 != NULL)
		{
			fclose(pFile1);
			pFile1 = NULL;
		}
		if(pFile2 != NULL)
		{
			fflush(pFile2);
			fsync(fileno(pFile2));//csp modify
			fclose(pFile2);
			pFile2 = NULL;
		}
		if(haveerror)
		{
			printf("copy failed!\n");
			break;
		}

		tCopy.curpos += readlen;
		sEventParaIns.sProgress.lCurSize= tCopy.curpos;
		sEventParaIns.sProgress.nProgress = 100*tCopy.curpos/tCopy.totalpos;
			
		EvNotify(sModSysIns.pFNSysNotifyCB, EM_SYSEVENT_BACKUP_RUN, &sEventParaIns);
		
		if(modsysCmplx_QueryBreakBackup())
		{
			printf("backup break!\n");
			modsysCmplx_ClrBreakBackup();
			umount_user("myusb");//cw_backup
			return 0;
		}
	}
	if(haveerror)
	{
		sEventParaIns.emResult = EM_BACKUP_FAILED_UNKNOW;
	}
	else
	{
		sEventParaIns.emResult = EM_BACKUP_SUCCESS;
	}

	EvNotify(sModSysIns.pFNSysNotifyCB, EM_SYSEVENT_BACKUP_DONE, &sEventParaIns);
	
	umount_user("myusb");
	return 0;
}


s32 modSysCmplx_Backup_USB( u32 nFileNum, SRecfileInfo* pRecInfoList, int justdo )
{
	u32 open_offset;
	u8  chn;
	struct tm tm_time;
	time_t t_time;
	int filesize;
	u8  errcode = 0;
	u64 totalsize = 0;
	char usb_device[32];
	FILE *pFile1 = NULL;
	FILE *pFile2 = NULL;
	int index, i;
	unsigned char buf[BAK_BUFSIZE];//zlbfix20110719 backup
	char filename[64];
	char ufilename[128];
	char tmpfile[64];
	BOOL haveerror = FALSE;
	int ret = -1;
	DiskInfo dinfo;
	SEventPara sEventParaIns;
	
	if(pRecInfoList==NULL)
	{
		printf("NULL file list!\n");

		return -1;//
	}
	
	EvNotify(sModSysIns.pFNSysNotifyCB, EM_SYSEVENT_BACKUP_INIT, &sEventParaIns);
	
	mkdir("myusb",1);
	MODSYS_DEBUG_STR("before while.\n");
	
	ret = get_usb_device(usb_device);
	if(ret != 0)
	{
		sEventParaIns.emResult = EM_BACKUP_FAILED_UNKNOW;
		EvNotify(sModSysIns.pFNSysNotifyCB, EM_SYSEVENT_BACKUP_DONE, &sEventParaIns);
		return 0;
	}

	ret = ifly_diskinfo(usb_device,&dinfo);
	if(ret == 0)
	{
		printf("检测到u盘:capability:%lld B,%dM\n",dinfo.capability,(int)(dinfo.capability/(1024*1024)));
	}

	for(index=0; index<nFileNum; index++)
	{
		get_rec_file_name((recfileinfo_t *)&pRecInfoList[index],filename,&open_offset);
		totalsize += (u64)pRecInfoList[index].nSize;
		
		printf("filename %s size %d\n", filename, pRecInfoList[index].nSize);
	}

	tCopy.cancel = 0;
	tCopy.curpos = 0;
	tCopy.totalpos = totalsize;

#if 0
	struct statfs s;
	statfs("myusb", &s);

	dinfo.freesize=s.f_bsize*s.f_bavail;

	printf("u free size %lld B,%lld M,file %lld M\n",dinfo.freesize,dinfo.freesize/1024/1024,totalsize/1024/1024);

	if(totalsize>dinfo.freesize)
	{
		umount_user("myusb");
		
		sEventParaIns.emResult = EM_BACKUP_FAILED_UNKNOW;
		EvNotify(sModSysIns.pFNSysNotifyCB, EM_SYSEVENT_BACKUP_DONE, &sEventParaIns);

		return -1; // too big files
	}
#else
	SDevInfo* pInfo = NULL;
	for(i=0; i<nMaxSupportDiskNum; i++)
	{
		printf("%s i: %d\n", __func__, i);
		pInfo = modsys_GetDiskInfo(i);
		if(pInfo && pInfo->nStatus==HDD_IDLE && pInfo->nType==1)
		{
			break;
		}
	}
	if (NULL == pInfo || pInfo->nType !=1)
	{
		printf("%s not found USB disk\n", __func__);
		return -1;
	}
	printf("%s: pInfo->nFree: %lld, totalsize: %lld\n", __func__, pInfo->nFree*(u64)KB, totalsize);
	if((u64)pInfo->nFree*(u64)KB < totalsize)
	{
		//printf("%s yg 2\n", __func__);
		umount_user("myusb");
		
		sEventParaIns.emResult = EM_BACKUP_FAILED_NOSPACE;
		EvNotify(sModSysIns.pFNSysNotifyCB, EM_SYSEVENT_BACKUP_DONE, &sEventParaIns);
		
		return -1; // too big files
	}
#endif

	memset(&sEventParaIns, 0, sizeof(sEventParaIns));

	sEventParaIns.sProgress.lTotalSize = tCopy.totalpos;

	BOOL bBreakBackup = FALSE;

	for(index=0; index<nFileNum; index++)
	{
		get_rec_file_name((recfileinfo_t *)&pRecInfoList[index],filename,&open_offset);
		pFile1 = fopen(filename,"rb");
		if(pFile1 == NULL)
		{
			printf("open recfile error\n");
			haveerror = TRUE;
			errcode = USB_NOERROR;
			goto END;
		}
		
		fseek(pFile1,open_offset,SEEK_SET);
		
		chn = pRecInfoList[index].nChn;
		
		t_time = pRecInfoList[index].nStartTime;
		
		//csp modify 20131213
		t_time += GetTZOffset(sModSysIns.nTimeZone);
		
		//memcpy(&tm_time,localtime(&t_time),sizeof(tm_time));
		localtime_r(&t_time, &tm_time);
		sprintf(tmpfile,"chn%d%04d%02d%02d%02d%02d%02d",
				chn,
				tm_time.tm_year+1900,
				tm_time.tm_mon+1,
				tm_time.tm_mday,
				tm_time.tm_hour,
				tm_time.tm_min,
				tm_time.tm_sec);

		sprintf(ufilename,"myusb/%s.ifv",tmpfile);	//08-01-18

		//printf("ufilename = %s\n", ufilename);

		pFile2 = fopen(ufilename,"wb");
		if(pFile2 == NULL)
		{
			printf("open usb file error\n");
			haveerror = TRUE;
			errcode = USB_ERROR_WRITE_ACCESS;
			goto END;
		}

		filesize = pRecInfoList[index].nSize;
		//printf("bakup filesize=%d,size=%d\n",filesize ,pRecInfoList[index].nSize);
		int tmpsize = 0; //zlbfix20110719 backup

		while((!feof(pFile1)) && (filesize > 0))
		{
			// check if need break backup
			if(modsysCmplx_QueryBreakBackup())
			{
				bBreakBackup = TRUE;
				break;
			}
			
			// if recv quit cmd, quit!!!
			#if 1
			if(tCopy.cancel)
			{
				haveerror = TRUE;
				errcode = USB_ERROR_WRITE_CANCEL;
				tCopy.cancel = 0;
				break;
			}
			#endif
 
			int readlen = 0;
			int writelen = 0;
			int rlen = min(sizeof(buf),filesize);

			readlen = fread(buf,1,rlen,pFile1);

			if( ferror(pFile1) )
			{
				haveerror = TRUE;
				errcode = USB_NOERROR;
				printf( "Read error" );
				break;
			} 
			
			writelen = fwrite(buf,readlen,1,pFile2);
			if( ferror(pFile2) )
			{
				haveerror = TRUE;
				errcode = USB_ERROR_WRITE_FULL;
				if(errno == 5)//Input/output error
				{
					errcode = USB_ERROR_WRITE_ACCESS;
				}
				printf( "Write error,errcode=%d,errstr=%s\n", errno, strerror(errno) );
				break;
			}
			
			filesize = filesize-readlen;
			tCopy.curpos += readlen;//zlbfix20110719 backup
			
			//csp modify
			//if(tCopy.curpos > tmpsize + 4 * BAK_BUFSIZE)//zlbfix20110719 backup
			if(tCopy.curpos >= tmpsize + 16 * BAK_BUFSIZE)//zlbfix20110719 backup
			{
				//printf("fflush %u\n",tmpsize);
				
				//csp modify
				//tmpsize += 4 * BAK_BUFSIZE;//zlbfix20110719 backup
				tmpsize = tCopy.curpos;
				
				fflush(pFile2);
				fsync(fileno(pFile2));
			}
			
			sEventParaIns.sProgress.lCurSize= tCopy.curpos;
			sEventParaIns.sProgress.nProgress = 100*tCopy.curpos/tCopy.totalpos;
			
			EvNotify(sModSysIns.pFNSysNotifyCB, EM_SYSEVENT_BACKUP_RUN, &sEventParaIns);
		}
END:
		if(pFile1 != NULL)
		{
			fclose(pFile1);
		}
		if(pFile2 != NULL)
		{
			fflush(pFile2);
			fsync(fileno(pFile2));//csp modify
			fclose(pFile2);
		}
		if(haveerror)
		{
			printf("copy failed!\n");
			break;
		}
		
		if(bBreakBackup)
		{
			printf("backup break!\n");
			modsysCmplx_ClrBreakBackup();
			umount_user("myusb");//cw_backup
			return 0;
//			break;
		}
	}
	if(haveerror)
	{
		sEventParaIns.emResult = EM_BACKUP_FAILED_UNKNOW;
	}
	else
	{
		sEventParaIns.emResult = EM_BACKUP_SUCCESS;
	}

	EvNotify(sModSysIns.pFNSysNotifyCB, EM_SYSEVENT_BACKUP_DONE, &sEventParaIns);
	
	umount_user("myusb");
	return 0;
}

s32 modSysCmplx_Backup_USB_Avi( u32 nFileNum, SRecfileInfo* pRecInfoList )
{
	u32 open_offset;
	u8  chn;
	struct tm tm_time;
	time_t t_time;
	u8  errcode = 0;

	u64 totalsize = 0, total_frames = 0;

	char usb_device[32];
	custommp4_t* pFile1 = NULL;
	avi_t* pAviHandle = NULL;
	int index;
	
	//csp modify 20121116//这里需要验证//CHIP_HISI3531是否起作用?
	#ifdef CHIP_HISI3531
	unsigned char buf[512 << 10];
	#else
	//csp modify 20121120
	//unsigned char buf[100 << 10];
	unsigned char buf[512 << 10];//unsigned char buf[256 << 10];//csp modify 20140302
	#endif
	
	char filename[64];
	char ufilename[128];
	char tmpfile[64];
	BOOL haveerror = FALSE;
	int ret = -1, i;
	DiskInfo dinfo;

	u32 start_time, size;
	u8 key;
	SEventPara sEventParaIns;

	if(pRecInfoList==NULL)
	{
		printf("NULL file list!\n");

		return -1;//
	}

	EvNotify(sModSysIns.pFNSysNotifyCB, EM_SYSEVENT_BACKUP_INIT, &sEventParaIns);
	
	mkdir("myusb",1);
	MODSYS_DEBUG_STR("before while.\n");

	ret = get_usb_device(usb_device);
	if(ret != 0)
	{
		return 0;
	}

	ret = ifly_diskinfo(usb_device,&dinfo);
	if(ret == 0)
	{
		printf("found disk : capability:%lld B,%dM\n",dinfo.capability,(int)(dinfo.capability/(1024*1024)));
	}

	for(index=0; index<nFileNum; index++)
	{
		get_rec_file_name((recfileinfo_t *)&pRecInfoList[index],filename,&open_offset);

		printf("filename %s size %d\n", filename, pRecInfoList[index].nSize);
		totalsize += (u64)pRecInfoList[index].nSize;

		pFile1 = custommp4_open(filename, O_R, open_offset);
		if (pFile1)
		{
			total_frames += custommp4_video_length(pFile1);
			custommp4_close(pFile1);
			pFile1 = NULL;
		}
	}
	
	tCopy.cancel 	= 0;
	tCopy.curpos 	= 0;
	tCopy.totalpos 	= total_frames;
	
	SDevInfo* pInfo;
	for(i=0; i<nMaxSupportDiskNum; i++)
	{
		pInfo = modsys_GetDiskInfo(i);
		if(pInfo && pInfo->nStatus==HDD_IDLE && pInfo->nType==1)
		{
			break;
		}
	}
	
	if (NULL == pInfo || pInfo->nType !=1)
	{
		printf("%s not found USB disk\n", __func__);
		return -1;
	}
	
	if((u64)pInfo->nFree*(u64)KB < totalsize)
	{
		umount_user("myusb");
		
		sEventParaIns.emResult = EM_BACKUP_FAILED_NOSPACE;
		EvNotify(sModSysIns.pFNSysNotifyCB, EM_SYSEVENT_BACKUP_DONE, &sEventParaIns);
		
		return -1; // too big files
	}
	
	memset(&sEventParaIns, 0, sizeof(sEventParaIns));

	sEventParaIns.sProgress.lTotalSize = tCopy.totalpos;

	BOOL bBreakBackup = FALSE;

	for(index=0; index<nFileNum; index++)
	{
		get_rec_file_name((recfileinfo_t*)&pRecInfoList[index],filename,&open_offset);
		
		pFile1 = custommp4_open(filename, O_R, open_offset);
		if(pFile1 == NULL)
		{
			printf("avi custommp4_open recfile error\n");
			haveerror = TRUE;
			errcode = USB_NOERROR;
			goto END;
		}
		
		chn = pRecInfoList[index].nChn;
		
		t_time = pRecInfoList[index].nStartTime;
		
		//csp modify 20131213
		t_time += GetTZOffset(sModSysIns.nTimeZone);
		
		//memcpy(&tm_time,localtime(&t_time),sizeof(tm_time));
		localtime_r(&t_time, &tm_time);
		sprintf(tmpfile,"chn%d%04d%02d%02d%02d%02d%02d",
				chn,
				tm_time.tm_year+1900,
				tm_time.tm_mon+1,
				tm_time.tm_mday,
				tm_time.tm_hour,
				tm_time.tm_min,
				tm_time.tm_sec);
		
		sprintf(ufilename,"myusb/%s.avi",tmpfile);	//08-01-18
		printf("ufilename = %s\n", ufilename);
		
		pAviHandle = AVI_open_output_file(ufilename);
		if (!pAviHandle)
		{
			printf("avi file open failed \n");
			haveerror = TRUE;
			errcode = USB_ERROR_WRITE_ACCESS;
			goto END;
		}
		
		//USE_AUDIO_PCMU begin
 		u32 file_totaltime = abs(pRecInfoList[index].nEndTime - pRecInfoList[index].nStartTime);
		if (0 == file_totaltime) file_totaltime = 1;
		
		AVI_set_video(pAviHandle,
				 custommp4_video_width(pFile1),
				 custommp4_video_height(pFile1),
				 (float)custommp4_video_length(pFile1)/(float)file_totaltime/*custommp4_video_frame_rate(pFile1)*/,
				 "H264" );
		
		AVI_set_audio(pAviHandle,
				custommp4_audio_channels(pFile1),
				custommp4_audio_sample_rate(pFile1),
				custommp4_audio_bits(pFile1),
				WAVE_FORMAT_PCM, 0 );
		//USE_AUDIO_PCMU end
		
		size = custommp4_video_length(pFile1);
		
		#ifdef USE_AUDIO_PCMU
		u8 media_type;
		u64 pts = 0;
		#endif
		
		while(size > 0)
		{
			// check if need break backup
			if(modsysCmplx_QueryBreakBackup())
			{
				bBreakBackup = TRUE;
				break;
			}
			
			if(tCopy.cancel)
			{
				haveerror = TRUE;
				errcode = USB_ERROR_WRITE_CANCEL;
				tCopy.cancel = 0;
				break;
			}
			
			#ifdef USE_AUDIO_PCMU
			int readlen = custommp4_read_one_media_frame(pFile1, buf, sizeof(buf), &start_time, &key, &pts,&media_type);
			#else
			int readlen = custommp4_read_one_video_frame(pFile1, buf, sizeof(buf), &start_time, &duration, &key);
			#endif
			if (readlen <= 0)
			{
				tCopy.curpos += size;
				haveerror = TRUE;
				errcode = USB_NOERROR;
				printf( "avi Read error 10,errcode=%d,errstr=%s\n", errno, strerror(errno));
				break;
			}
			#ifdef USE_AUDIO_PCMU
			if(0 == media_type){
			#endif
				int ret_avi = AVI_write_frame(pAviHandle, buf, readlen, key);
				if (ret_avi)
				{
					haveerror = TRUE;
					errcode = USB_ERROR_WRITE_FULL;
					if(errno == 5)//Input/output error
					{
						errcode = USB_ERROR_WRITE_ACCESS;
					}
					printf( "avi Write error 1,errcode=%d,errstr=%s\n", errno, strerror(errno) );
					break;
				}

				tCopy.curpos++;
				size--;
			#ifdef USE_AUDIO_PCMU
			}
			else
			{
				int ret_avi = AVI_write_audio(pAviHandle, buf, readlen);
				if (ret_avi)
				{
					haveerror = TRUE;
					errcode = USB_ERROR_WRITE_FULL;
					if(errno == 5)//Input/output error
					{
						errcode = USB_ERROR_WRITE_ACCESS;
					}
					printf( "avi Write error 1,errcode=%d,errstr=%s\n", errno, strerror(errno) );
					break;
				}
			}
			#endif
			
			sEventParaIns.sProgress.lCurSize = tCopy.curpos;
			u8 nProg = 100*tCopy.curpos/tCopy.totalpos;
			if(nProg != sEventParaIns.sProgress.nProgress)
			{
				sEventParaIns.sProgress.nProgress = nProg;
				
				EvNotify(sModSysIns.pFNSysNotifyCB, EM_SYSEVENT_BACKUP_RUN, &sEventParaIns);
			}
		}
END:
		if (pFile1)
		{
			custommp4_close(pFile1);
			pFile1 = NULL;
		}
		
		if (pAviHandle)
		{
			AVI_close(pAviHandle);
			pAviHandle = NULL;
		}
		if(haveerror)
		{
			printf("avi copy failed\n");
			break;
		}
		
		if(bBreakBackup)
		{
			printf("backup break!\n");
			modsysCmplx_ClrBreakBackup();
			umount_user("myusb");//cw_backup
			return 0;
//			break;
		}
	}
	
	if(haveerror)
	{
		sEventParaIns.emResult = EM_BACKUP_FAILED_UNKNOW;
	}
	else
	{
		sEventParaIns.emResult = EM_BACKUP_SUCCESS;
	}
	
	EvNotify(sModSysIns.pFNSysNotifyCB, EM_SYSEVENT_BACKUP_DONE, &sEventParaIns);
	
	umount_user("myusb");
	return 0;
}

s32 modSysCmplx_Backup_DVDR(
		u32 nFileNum,
		SRecfileInfo* pRecInfoList,
		EMBACKUPDVDRWMODE mode
	)
{
	FILE *pFile1 = NULL;
	my_daemon_param daemon;
	int  i;
	int  index;
	unsigned char buf[1024*32];
	char filename[64];
	//char ufilename[128];
	char tmpfilestat[128];
	char tmpfile[64];
	BOOL haveerror = FALSE;
	u8   errcode = 0;
	int  ret = -1;

	SEventPara 	sEventParaIns;
	SProgress	sProg;

#ifdef DVD_RW_DMA
#ifdef HISI_DVDR

	MODSYS_ASSERT(NULL!=pRecInfoList);

	printf("file number is %d\n",nFileNum);

	for(i=0;i<MAX_HDD_NUM;i++)
	{
		char medianame[64];

		//sprintf(medianame,"/dev/scsi/host%d/bus0/target0/lun0/cd",i);
		sprintf(medianame,"/dev/sr0");
		ret = open(medianame, O_RDONLY);
		//FILE *fpidetype = fopen(medianame,"rb");
		if(ret < 0)
		{
			if(errno==123) //无光盘
			{
				printf("~~~~~~	无光盘!\n");
			}
			else //其他情况
			{
				printf("其他情况!\n");
			}
			continue;
		}
		else
		{
			close(ret);
			break;
		}
	}
	//printf("dvd index=%d\n",i);

	memset(&sProg, 0, sizeof(sProg));

	if(i>=MAX_HDD_NUM)
	{
		modSysCmplx_UpdateNotify(NULL, -1, -1); // dvdr no hdd

		;// TEMP_DEBUG_DROP_IT //return -1;
	}

	if(EM_DVDRW_MODE_NEW==mode)
	{
		sprintf(daemon.arg2,"-Z /dev/sr0");
	}
	else if(EM_DVDRW_MODE_ADD==mode)
	{
		sprintf(daemon.arg2,"-M /dev/sr0");
	}
	else
	{
		modSysCmplx_UpdateNotify(NULL, -1, -1); // dvdr no hdd

		return -1;
	}
#else
	printf("else HISI_DVDR!\n");

	for(i=0;i<2;i++)
	{
		char medianame[64];
		char dvdrwname[64];
		char idetype[32];

		sprintf(medianame,"/proc/ide/hd%c/media",'a'+i);
		FILE *fpidetype = fopen(medianame,"rb");
		if(fpidetype == NULL)
		{
			continue;
		}

		fgets(idetype,32,fpidetype);
		fclose(fpidetype);

		if(strcmp(idetype,"cdrom\n") != 0)
		{
			continue;
		}
		else
		{
			sprintf(dvdrwname,"/dev/hd%c",'a'+i);
			ret = open(dvdrwname, O_RDONLY);
			if(ret < 0)
			{
				if(errno==123) //无光盘
				{
					printf("无光盘!\n");
				}
				else //其他情况
				{
					printf("其他情况!\n");
				}
				continue;
			}
			else
			{
				close(ret);
				break;
			}
		}
	}
	//printf("dvd index=%d\n",i);
	if(i>=2)
	{
		modSysCmplx_UpdateNotify(NULL, -1, -1); // dvdr no hdd

		#ifndef TEMP_DEBUG_DROP_IT
		return -1;
		#endif
	}

	if(EM_DVDRW_MODE_NEW==mode)
	{
		sprintf(daemon.arg2,"-Z /dev/hd%c",'a'+i);
	}
	else if(EM_DVDRW_MODE_ADD==mode)
	{
		sprintf(daemon.arg2,"-M /dev/hd%c",'a'+i);
	}
	else
	{
		modSysCmplx_UpdateNotify(NULL, -1, -1); // dvdr no hdd

		return -1;
	}
#endif


//pw:match along with DVD_RW_DMA
#else

	if(zSel)
	{
		ret = open("/dev/sr0", O_RDONLY);
		if(ret<0)
		{
			perror("open");
			printf("dvd recorder in sr0 ret = %d, errno=%d . \n",ret, errno);
			if(errno==123) //无光盘
			{
				printf("无光盘!\n");
			}
			ret = open("/dev/sr1", O_RDONLY);
			if(ret<0)
			{
				if(errno==123) //无光盘
				{
					printf("无光盘!\n");
				}
				else if(errno==2)//无刻录机
				{
					printf("无刻录机!\n");
				}
				else //其他情况
				{
					printf("其他情况!\n");
				}

				perror("open");
				printf("dvd recorder in sr1 ret = %d, errno=%d . \n",ret, errno);
				//byFileCopy = 0;
				return;
			}
			else
			{
				sprintf(daemon.arg2,"%s","-Z /dev/sr1");
				printf("dvd recorder in -Z sr1 . \n");
				close(ret);
			}
		}
		else
		{
			sprintf(daemon.arg2,"%s","-Z /dev/sr0");
			printf("dvd recorder in -Z sr0 . \n");
			close(ret);
		}
	}
	else if(mSel)
	{
		ret = open("/dev/sr0", O_RDONLY);
		if(ret<0)
		{
			perror("open");
			printf("dvd recorder in sr0 ret = %d, errno=%d . \n",ret, errno);
			ret = open("/dev/sr1", O_RDONLY);
			if(ret<0)
			{
				perror("open");
				printf("dvd recorder in sr1 ret = %d, errno=%d . \n",ret, errno);
				return;
			}
			else
			{
				sprintf(daemon.arg2,"%s","-M /dev/sr1");
				printf("dvd recorder in -M sr1 . \n");
				close(ret);
			}
		}
		else
		{
			sprintf(daemon.arg2,"%s","-M /dev/sr0");
			printf("dvd recorder in -M sr0 . \n");
			close(ret);
		}
	}
	else
	{
		ret = open("/dev/sr0", O_RDONLY);
		if(ret<0)
		{
			printf("dvd recorder in sr0 ret = %d, errno=%d . \n",ret, errno);
			perror("open");
			ret = open("/dev/sr1", O_RDONLY);
			if(ret<0)
			{
				perror("open");
				printf("dvd recorder in sr1 ret = %d, errno=%d . \n",ret, errno);
				return;
			}
			else
			{
				sprintf(daemon.arg2,"%s","-Z /dev/sr1");
				printf("dvd recorder in -Z sr1 . \n");
				close(ret);
			}
		}
		else
		{
			sprintf(daemon.arg2,"%s","-Z /dev/sr0");
			printf("dvd recorder in -Z sr0 . \n");
			close(ret);
		}
	}
#endif

	u64 total_size=0;

	FILE *pfilelist=fopen("/tmp/recordfilelist","wb");
	if (pfilelist==NULL)
	{
		printf("creat /tmp/recordfilelist failed!!\n");
		//byFileCopy = 0;
		return -1; // cannot create tmp recfilelist file
	}
	else
	{
		printf("creat /tmp/recordfilelist success!!\n");
	}

	for( index=0; index<nFileNum; index++ )
	{
		u32 open_offset;
		u8  chn;

		get_rec_file_name((recfileinfo_t*)&pRecInfoList[index],filename,&open_offset);
		total_size += pRecInfoList[index].nSize;

		chn = pRecInfoList[index].nChn;

		struct tm tm_time;
		time_t t_time;
		t_time = pRecInfoList[index].nStartTime;

		//zhao:cwr modify
		//memcpy(&tm_time,localtime(&t_time),sizeof(tm_time));
		localtime_r(&t_time,&tm_time);

		sprintf(tmpfile,"chn%d%04d%02d%02d%02d%02d%02d",
				chn,
				tm_time.tm_year+1900,
				tm_time.tm_mon+1,
				tm_time.tm_mday,
				tm_time.tm_hour,
				tm_time.tm_min,
				tm_time.tm_sec);

		sprintf(tmpfilestat,"/tmp/%s.ifv %d\n",tmpfile,pRecInfoList[index].nSize);
		fwrite(tmpfilestat, sizeof(char), strlen(tmpfilestat), pfilelist);
	}

	printf("write file to tmfilestat log file!\n");

	fflush(pfilelist);
	fsync(fileno(pfilelist));

	fclose(pfilelist);

	daemon.proc_type = MY_DAEMON_PROCTYPE_DVDRECORDER;
	sprintf(daemon.arg1,"/tmp/recordfilelist");
	SendMsgToDaemon(&daemon);

	int s2 = socket(AF_INET,SOCK_STREAM,0);
	//fprintf(stderr, "app:s2 is %d\n",s2);
	if(s2 < 0)
	{
	    fprintf(stderr,"app:socket error\n");
	    //byFileCopy = 0;
	    haveerror = TRUE;//备份出错，作标记
	    goto END1;//跳到结束，返回播放查询页面
		//return;
	}

	fcntl(s2,F_SETFL,O_NONBLOCK);

	struct linger m_sLinger;
	m_sLinger.l_onoff = 1;  //(在closesocket()调用,但是还有数据没发送完毕的时候容许逗留)
	m_sLinger.l_linger = 0; //(容许逗留的时间为0秒)
	ret = setsockopt(s2,SOL_SOCKET,SO_LINGER,(char*)&m_sLinger,sizeof(struct linger));
	if( SOCKET_ERROR == ret )
	{
		fprintf(stderr,"app:SetSockLinkOpt:set socket linger error 1!\n");
	}

	struct sockaddr_in local;
	local.sin_family = AF_INET;
	local.sin_addr.s_addr = 0;
	local.sin_port = htons(7000);
	int databind = 0;
	int databindret = 0;
	while(databind<10)
	{
		if((databindret=bind(s2,(struct sockaddr *)&local,sizeof(local))) == -1)
		{
		    system("netstat -an");
		    fprintf(stderr,"app:send data bind failed,databind=%d, errno=%d,errstr=%s\n",databind,errno,strerror(errno));
		}
		else
		{
			printf("app:send data 1 bind success\n");
			break;
		}
		databind++;
	}
	printf("app:send data 1.1 bind success\n");
	if(databindret == -1)
	{
	    system("netstat -an");
	    fprintf(stderr,"app:send data bind failed,errno=%d,errstr=%s\n",errno,strerror(errno));
	    close(s2);
		//byFileCopy = 0;
	    haveerror = TRUE;//备份出错，作标记
	    goto END1;//跳到结束，返回播放查询页面
		//return;
	}
	
	fprintf(stderr,"app:bind11111 success\n");
	
	if(listen(s2,15) == -1)
	{
	    fprintf(stderr,"app:listen error\n");
	    close(s2);
		//byFileCopy = 0;
	    haveerror = TRUE;//备份出错，作标记
	    goto END1;//跳到结束，返回播放查询页面
		//return;
	}
	fprintf(stderr,"app:listen success\n");
	
	struct sockaddr_in from;
	socklen_t fromlen = sizeof(from);
	int acpt = 0;
	int s = -1;
	while(acpt<40)//超时返回
	{
		s = accept(s2,(struct sockaddr *)&from,&fromlen);
		fprintf(stderr,"app:accept %d......\n",acpt);
		if(s < 0)
		{
		    fprintf(stderr,"app:accept failed...\n");
			char datatoobig[32];
			FILE *fpdatatoobig = fopen("/tmp/datatoobigflag","a+");
			if(NULL == fpdatatoobig)
			{
				printf("fopen /tmp/datatoobigflag error!");
				continue;
			}
			memset(datatoobig,0,sizeof(datatoobig));//csp add
			fgets(datatoobig,32,fpdatatoobig);
			//fwrite("a",1,1,fpdatatoobig);
			fclose(fpdatatoobig);
			printf("dataflag is %s\n",datatoobig);
			if(strcmp(datatoobig,"cdrdatatoobig")==0)
			{
				break;
			}
			sleep(1);
			acpt++;
		    //return;
		}
		else
		{
			break;
		}
	}
	if(s < 0)
	{
	    fprintf(stderr,"app:accept failed\n");
	    close(s2);
		//byFileCopy = 0;
		
		haveerror = TRUE;//备份出错，作标记
	    goto END1;//跳到结束，返回播放查询页面
		//return;
	}
	
	fprintf(stderr,"app:accept success\n");
	
	{
		struct linger m_sLinger;
		m_sLinger.l_onoff = 1;  //(在closesocket()调用,但是还有数据没发送完毕的时候容许逗留)
		m_sLinger.l_linger = 0; //(容许逗留的时间为0秒)
		int ret = setsockopt(s,SOL_SOCKET,SO_LINGER,(char*)&m_sLinger,sizeof(struct linger));
		if( SOCKET_ERROR == ret )
		{
			fprintf(stderr,"app:SetSockLinkOpt:set socket linger error 2!\n");
		}
	}
	
	close(s2);
	
#if 1//progess
	tCopy.cancel = 0;
	tCopy.curpos = 0;
	tCopy.totalpos = total_size;
#endif
	
	for( i=0; i<nFileNum; i++ )
	{
		u32 open_offset;
		//index = i*8+j%8;
		/*******************/
		//05-04 mp4与ifv文件兼容
		get_rec_file_name((recfileinfo_t *)&pRecInfoList[i],filename,&open_offset);
		pFile1 = fopen(filename,"rb");
		if(pFile1 == NULL)
		{
			printf("open ifv recfile error\n");
			get_rec_file_mp4_name((recfileinfo_t *)&pRecInfoList[i],filename,&open_offset);
			pFile1 = fopen(filename,"rb");
			if(pFile1 == NULL)
			{
				printf("open mp4 recfile error\n");
				haveerror = TRUE;
				errcode = USB_NOERROR;

				goto END1;
			}
		}
		//printf("filename is %s \n",filename);

		fseek(pFile1,open_offset,SEEK_SET);

		unsigned int filesize = pRecInfoList[i].nSize;

		BOOL bBreakBackup = FALSE;

		while((!feof(pFile1)) && (filesize > 0))
		{
			// check if need break backup
			if(modsysCmplx_QueryBreakBackup())
			{
				bBreakBackup = TRUE;
				break;
			}

			int rlen = min(sizeof(buf),filesize);

			int readlen = fread(buf, 1, rlen, pFile1);
			//printf("%d,%d,%d\n",readlen,rlen,filesize);
			if( ferror(pFile1) )
			{
				haveerror = TRUE;
				errcode = USB_NOERROR;
				printf( "Read error\n" );
				break;
			}

			int ret = send(s,buf,readlen,MSG_NOSIGNAL);
			if(ret != readlen)//ok
			{
				fprintf(stderr,"send error,ret=%d,readlen=%d\n",ret,readlen);
				haveerror = TRUE;
				break;
			}

			filesize = filesize-readlen;
			tCopy.curpos += readlen;

			sEventParaIns.sProgress.lCurSize= tCopy.curpos;
			sEventParaIns.sProgress.nProgress = 100*tCopy.curpos/tCopy.totalpos;

			EvNotify(sModSysIns.pFNSysNotifyCB, 0xff, &sEventParaIns);

			//usleep(800*1000);
		}
END1:
		if(pFile1 != NULL)
		{
			fclose(pFile1);
		}

		if(bBreakBackup)
		{
			printf("break dvdr backup!\n");
			break;
		}

		if(haveerror) break;
	}

	printf("\n\n");
	//sleep(1);

	printf("send data socket was closed! \n\n");
	sleep(2);

	close(s);

	char datatoobig[32];
	FILE *fpdatatoobig = fopen("/tmp/datatoobigflag","a+");
	fgets(datatoobig,32,fpdatatoobig);
	fwrite("a",1,1,fpdatatoobig);
	fclose(fpdatatoobig);
	printf("dataflag is %s\n",datatoobig);

	if(strcmp(datatoobig,"cdrdatatoobig")==0||haveerror)//cwr modify
	{
		printf("befor cdrecode Showmsgbox...\n");
		//ShowMsgBox(IDC_TEXT_EQUIPERROR,IDC_CAP_BAKREC,NULL,TRUE);
	}
	else
	{
		printf("success!!!...\n");
		;//ShowMsgBox(IDC_TEXT_BAKUPSUC,IDC_CAP_BAKREC,NULL,TRUE);
	}

	return 0;
}

int SendMsgToDaemon(my_daemon_param *pDaemon)
{
	if(pDaemon == NULL)
	{
		return -1;
	}
	pDaemon->magic_no = MY_DAEMON_MAGIC;
	int s = socket(AF_INET,SOCK_DGRAM,0);
	if(s < 0)
	{
		return -1;
	}
	struct sockaddr_in to;
	to.sin_family = AF_INET;
	to.sin_addr.s_addr = inet_addr("127.0.0.1");
	to.sin_port = htons(MY_DAEMON_PORT);
	int ret = sendto(s,pDaemon,sizeof(my_daemon_param),0,(struct sockaddr *)&to,sizeof(to));
	printf("SendMsgToDaemon ret=%d\n",ret);
	close(s);
	return ret;
}

