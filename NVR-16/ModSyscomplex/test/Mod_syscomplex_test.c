#include "mod_syscomplex.h"
#include "search.h"
#include <sys/time.h>
#include "syscomplxtime.h"

#if 1
// callback example
void modSys_notify(EMSYSEVENT emSysEvent, SEventPara *psEventPara)
{
	return 0;
	
	printf("\n event id : %d ", emSysEvent);
	printf("\n event para :\n");
	if(1)//emSysEvent>-199 && emSysEvent<-100)
	{
		printf("\n total      %llu ", psEventPara->sProgress.lTotalSize );
		printf("\n cur        %llu ", psEventPara->sProgress.lCurSize );
		printf("\n progress   %d ", psEventPara->sProgress.nProgress );
	}
	else if(emSysEvent>-499 && emSysEvent<-400)
	{
		printf("\n result   %d ", psEventPara->emResult );		
	}
}
#endif

disk_manager hddmanager;

void PUpdateTimeCBIns(char* pTime)
{
	if(pTime)
	{
		printf(pTime);
	}

	//getchar();
}

#if 1//TEST_ALONE
int main( int argc, char* argv[] )
#else
int test_modsyscomplx( int ac, char* av[] )
#endif
{
#if 1
	s32 rtn;
	s32 cmd;
	u32 i, j;
	SModSysCmplxInit sMSCInit;
	
#if 0 // debug lanston
	HddInfo *phinfo;
	disk_manager *hdd_manager;

	hdd_manager = &hddmanager;

	for(j=0; j<MAX_HDD_NUM; j++)
	{
		phinfo = &hdd_manager->hinfo[j];
		if(phinfo->is_disk_exist)
		{
		/*
			u8 is_disk_exist;
			u8 is_partition_exist[MAX_PARTITION_NUM];
			partition_index ptn_index[MAX_PARTITION_NUM];
			long total;
			long free;
			u8 storage_type;	
		*/
			printf("phinfo->is_disk_exist %s %d %d\n", __FILE__, __LINE__, phinfo->is_disk_exist); // debug lanston
			printf("phinfo->is_partition_exist %s %d %d\n",
				__FILE__, __LINE__, phinfo->is_partition_exist[0]); // debug lanston
			printf("phinfo->total %s %d %lu\n", 
				__FILE__, __LINE__, phinfo->total); // debug lanston
		}
	}
#endif

	
	cmd = 7;

	char* exp[] = {
		"0 update\n",
		"1 search\n",
		"2 backup usb-avi\n",
		"3 backup dvdr\n",
		"4 format\n",
		"5 datetime set/get\n",
		"6 storage device info query\n",
		"7 update file info query\n",
		"8 get usb mounted dir\n"
	};

	int nTestItem = sizeof(exp)/sizeof(char*);
	
	init_disk_manager(&hddmanager);
	
	sMSCInit.pDMHdr = &hddmanager;
	sMSCInit.pSysNotifyCB = modSys_notify;
	sMSCInit.pUpTimeCB = PUpdateTimeCBIns;
	
	// module init
	rtn = ModSysComplexInit(&sMSCInit);
	if(rtn!=0)
	{
		printf(" modsyscomplex init failed!\n");

		getchar();

		exit(1);
	}

	getchar();
	
	printf("enter number to start matched test:\n");
	for(i=0;i<nTestItem;i++)
	{
		printf("Test %d - %s", i, exp[i]);
	}

	while(1)
	{
		printf("please enter number to start test:");
		scanf("%d", &cmd);
		printf("\n\n");
		
		if(cmd>=0&&cmd<nTestItem)
		{
			printf("got number %d, let's do test - %s\n", cmd, exp[cmd]);
			break;
		}
		else
		{
			printf("got number %d, invalid, enter again!\n");
		}
	}
	
	printf("Init finish!!\n");
	
	// give the test para
	// cmd
	printf("~~~~~~~start test modsyscomplex module~~~~~~\n\n\n");
	
	switch(cmd)
	{
		case 8:
		{
			SUsbDirList sPIns;

			memset(&sPIns, 0,sizeof(SUsbDirList));
			
			if( 0==ModSysComplexGetUsbDirInfo(&sPIns) )
			{
				printf("ModSysComplexGetUsbDirInfo\n");

				printf("usb dev mounted dir: %s\n", sPIns.sInfo[0].szDir);
			}
			
		} break;
		case 7: // ModSysComplexGetUpdateFileInfo
		{
			SUpdateFileInfoList sUFInfoLst;
			memset(&sUFInfoLst, 0, sizeof(sUFInfoLst));

			sUFInfoLst.nMax = 10;
			sUFInfoLst.pInfo = malloc(sizeof(SUpdateFileInfo)*sUFInfoLst.nMax);
			if(sUFInfoLst.pInfo)
			{
				ModSysComplexGetUpdateFileInfo(EM_UPGRADE_MAINBOARD, &sUFInfoLst);

				printf("ModSysComplexGetUpdateFileInfo.. \n");
				for(i=0; i<sUFInfoLst.nFileNum; i++)
				{
					printf("name: %s\n", sUFInfoLst.pInfo[i].szName);
					printf("ext : %s\n", sUFInfoLst.pInfo[i].szExt);
					printf("size: %d\n", sUFInfoLst.pInfo[i].nSize);
					printf("stamp: %d\n", sUFInfoLst.pInfo[i].nTimeStamp);					
				}
			}
			if(sUFInfoLst.pInfo) free(sUFInfoLst.pInfo);
			
		}
		break;
		case 6: // storage device info request
		{
			SStoreDevManage sStDevMgr;

			printf("check hddmgr content: \n");
			for( i=0; i<4; i++ ) 
			{
				printf("dev %d nStatus: 	%d\n",i, hddmanager.hinfo[i].is_disk_exist);
				printf("dev %d nType: 		%d\n",i, hddmanager.hinfo[i].storage_type);
				printf("dev %d nTotal: 		%d\n",i, hddmanager.hinfo[i].total);
				printf("dev %d nFree: 		%d\n",i, hddmanager.hinfo[i].free);
				printf("dev %d parition index length: 	%d\n",i, hddmanager.hinfo[i].ptn_index[0].length1);
				printf("dev %d nPartitionNo: %d\n",i, hddmanager.hinfo[i].is_partition_exist[0]);
			}
			
			memset( &sStDevMgr, 0, sizeof(sStDevMgr) );

			sStDevMgr.psDevList = (SDevInfo*)malloc(sizeof(SDevInfo)*64);
			
			rtn = ModSysComplexDMGetInfo(&sStDevMgr, 8);
			if(0==rtn)
			{
				printf("dev num: %d\n",sStDevMgr.nDiskNum);
				if(sStDevMgr.nDiskNum>0)
				{
					for( i=0; i<sStDevMgr.nDiskNum; i++ )
					{
						printf("dev %d nStatus: 	%d\n",i, sStDevMgr.psDevList[i].nStatus);
						printf("dev %d nType: 		%d\n",i, sStDevMgr.psDevList[i].nType);
						printf("dev %d nTotal: 		%d\n",i, sStDevMgr.psDevList[i].nTotal);
						printf("dev %d nFree: 		%d\n",i, sStDevMgr.psDevList[i].nFree);
						printf("dev %d strDevPath: 	%s\n",i, sStDevMgr.psDevList[i].strDevPath);
						printf("dev %d nPartitionNo: %d\n",i, sStDevMgr.psDevList[i].nPartitionNo);
					}
				}
			}
			else
				printf("failed to get dev info!\n");

			if(sStDevMgr.psDevList) free(sStDevMgr.psDevList);
			getchar();
			
			break;
		}
		case 5:  // datetime set
		{			
			//char path = strtol(argv[1], NULL, 10);
			//u32 way = strtol(argv[2], NULL, 10);
			SDateTime sDtIns;
			
			memset( &sDtIns, 0, sizeof(sDtIns) );
			
			sDtIns.nYear 	= strtol(argv[1], NULL, 10);
			sDtIns.nMonth 	= strtol(argv[2], NULL, 10);
			sDtIns.nDay 	= strtol(argv[3], NULL, 10);
			sDtIns.nHour	= strtol(argv[4], NULL, 10);
			sDtIns.nMinute 	= strtol(argv[5], NULL, 10);
			
			sDtIns.nSecode	= 0;
			
			printf("cmd to set datetime \n");
			
			rtn = ModSysComplexDTSet(&sDtIns);			
			if(rtn)
			{
				printf("ModSysComplexDTSet failed, err %d!\n", rtn);

				exit(1);
			}


			printf("enter to get time and show:\n");
			
			getchar();
			
			memset( &sDtIns, 0, sizeof(sDtIns) );
			
			rtn = ModSysComplexDTGet(&sDtIns);	
			if(rtn)
			{
				printf("ModSysComplexDTGet failed, err %d!\n", rtn);

				exit(1);
			}

			printf("get time %04d-%02d-%02d-%02d-%02d-%02d\n",
				sDtIns.nYear,sDtIns.nMonth,sDtIns.nDay,
				sDtIns.nHour,sDtIns.nMinute,sDtIns.nSecode
			);
			
			getchar();

			break;
		}
	
		case 4: // format test case 
		{			
			//char path = strtol(argv[1], NULL, 10);
			//u32 way = strtol(argv[2], NULL, 10);

			if(argv[1]==NULL)
			{
				printf("invalid dev ptah!!\n");

				getchar();

				exit(1);
			}
			
			printf("cmd to format such dev %s\n", argv[1]);
			
			rtn = ModSysComplexDMFormat(argv[1], 0);			
			if(rtn)
			{
				printf("Upgrade failed, err %d!\n", rtn);

				getchar();

				exit(1);
			}
			break;
		}
		case 0: // update 
		{			
			u32 evt = strtol(argv[1], NULL, 10);
			u32 way = strtol(argv[2], NULL, 10);
			if( argv[3] )
			{
				if(strlen(argv[3])<0)
				{
					printf("invalid path!\n");
					getchar();
					exit(1);
				}
			}
			printf("get para as such : evt %d, way %d\n", evt, way);
			
			rtn = ModSysComplexUpgrade( evt, way, argv[3] );
			if(rtn)
			{
				printf("Upgrade failed, err %d!\n", rtn);

				getchar();

				exit(1);
			}
			break;
		}
		case 3:
		{			
			//u32 evt = strtol(argv[1], NULL, 10);
			//u32 way = strtol(argv[2], NULL, 10);
			// cmd type ch y-m-d-h-m y-m-d-h-m
			// ..  1/2/4/8/10 .. 
			struct tm tm_time;
			u8 y,m,d, h,min, type, ch;
			char* pFileBuf = NULL;
			
			SSearchPara sSearchParaIns;
			SSearchResult sSearchResultIns;
			int nMaxFileNum;
			
			memset(&tm_time,0,sizeof(tm_time));
			
			type = strtol(argv[1], NULL, 10);
			ch = strtol(argv[2], NULL, 10);
			y = strtol(argv[3], NULL, 10);
			m = strtol(argv[4], NULL, 10);
			d = strtol(argv[5], NULL, 10);
			h = strtol(argv[6], NULL, 10);
			min = strtol(argv[7], NULL, 10);
			
			tm_time.tm_year = 2000+y - 1900;
			tm_time.tm_mon = m - 1;
			tm_time.tm_mday = d;
			tm_time.tm_hour = h;
			tm_time.tm_min = min;
			tm_time.tm_sec = 0;

			sSearchParaIns.nMaskType = type;
			sSearchParaIns.nMaskChn	 = ch;
			sSearchParaIns.nStartTime = mktime(&tm_time);
			
			printf("ymdhm %d-%d-%d-%d-%d %d\n", 
					tm_time.tm_year,
					tm_time.tm_mon,
					tm_time.tm_mday,
					tm_time.tm_hour,
					tm_time.tm_min, sSearchParaIns.nStartTime );
			
			y = strtol(argv[8], NULL, 10);
			m = strtol(argv[9], NULL, 10);
			d = strtol(argv[10], NULL, 10);
			h = strtol(argv[11], NULL, 10);
			min = strtol(argv[12], NULL, 10);
			
			tm_time.tm_year = 2000+y - 1900;
			tm_time.tm_mon = m - 1;
			tm_time.tm_mday = d;
			tm_time.tm_hour = h;
			tm_time.tm_min = min;
			tm_time.tm_sec = 0;
			
			sSearchParaIns.nEndTime = mktime(&tm_time);			

			printf("ymdhm %d-%d-%d-%d-%d %d\n", 
					tm_time.tm_year,
					tm_time.tm_mon,
					tm_time.tm_mday,
					tm_time.tm_hour,
					tm_time.tm_min, sSearchParaIns.nEndTime );
			
			nMaxFileNum = 10;

			SSearchCondition sSCondIns;// = (SSearchCondition*)pFileBuf;
			
			//pFileBuf= malloc( sizeof(SRecfileInfo)*nMaxFileNum );
			
			sSCondIns.emSearchWay = 0; 
			memcpy( &sSCondIns.sSearchPara, &sSearchParaIns, sizeof(sSearchParaIns) );
				
				//sSearchResultIns.psRecfileInfo = pFileBuf;
			
			//if(!sSearchResultIns.psRecfileInfo) printf("not enough memory for found files!\n");

			SBackTgtAtt sBackTgtAtt;

			memset( &sBackTgtAtt, 0, sizeof(sBackTgtAtt) );
			sBackTgtAtt.devt = EM_BACKUP_DVDR;
			sBackTgtAtt.fmt = EM_FORMAT_ORIGINAL;
			sBackTgtAtt.mode = EM_DVDRW_MODE_ADD;
			
			rtn = ModSysComplexBackup(
					0,
					&sBackTgtAtt, 
					&sSCondIns );
			if(rtn)
			{
				printf("search failed, err %d!\n", rtn);

				getchar();
			}
			else
			{
				printf("found the %d files as following: \n", sSearchResultIns.nFileNum);

				for( i=0; i<sSearchResultIns.nFileNum; i++ )
					printf("file %d %d %d %d %d\n", 
							sSearchResultIns.psRecfileInfo[i].nChn,
							sSearchResultIns.psRecfileInfo[i].nDiskNo, 
							sSearchResultIns.psRecfileInfo[i].nStartTime,
							sSearchResultIns.psRecfileInfo[i].nEndTime,
							sSearchResultIns.psRecfileInfo[i].nFileNo );
			}

			
			//if(pFileBuf) free(pFileBuf);
			
			break;
		}
		case 2:
		{			
			//u32 evt = strtol(argv[1], NULL, 10);
			//u32 way = strtol(argv[2], NULL, 10);
			// cmd type ch y-m-d-h-m y-m-d-h-m
			// ..  1/2/4/8/10 .. 
			struct tm tm_time;
			u8 y,m,d, h,min, type, ch;
			char* pFileBuf = NULL;
			
			SSearchPara sSearchParaIns;
			SSearchResult sSearchResultIns;
			int nMaxFileNum;
			
			memset(&tm_time,0,sizeof(tm_time));
			
			type = strtol(argv[1], NULL, 10);
			ch = strtol(argv[2], NULL, 10);
			y = strtol(argv[3], NULL, 10);
			m = strtol(argv[4], NULL, 10);
			d = strtol(argv[5], NULL, 10);
			h = strtol(argv[6], NULL, 10);
			min = strtol(argv[7], NULL, 10);
			
			tm_time.tm_year = 2000+y - 1900;
			tm_time.tm_mon = m - 1;
			tm_time.tm_mday = d;
			tm_time.tm_hour = h;
			tm_time.tm_min = min;
			tm_time.tm_sec = 0;

			sSearchParaIns.nMaskType = type;
			sSearchParaIns.nMaskChn	 = ch;
			sSearchParaIns.nStartTime = mktime(&tm_time);
			
			printf("ymdhm %d-%d-%d-%d-%d %d\n", 
					tm_time.tm_year,
					tm_time.tm_mon,
					tm_time.tm_mday,
					tm_time.tm_hour,
					tm_time.tm_min, sSearchParaIns.nStartTime );
			
			y = strtol(argv[8], NULL, 10);
			m = strtol(argv[9], NULL, 10);
			d = strtol(argv[10], NULL, 10);
			h = strtol(argv[11], NULL, 10);
			min = strtol(argv[12], NULL, 10);
			
			tm_time.tm_year = 2000+y - 1900;
			tm_time.tm_mon = m - 1;
			tm_time.tm_mday = d;
			tm_time.tm_hour = h;
			tm_time.tm_min = min;
			tm_time.tm_sec = 0;
			
			sSearchParaIns.nEndTime = mktime(&tm_time);			

			printf("ymdhm %d-%d-%d-%d-%d %d\n", 
					tm_time.tm_year,
					tm_time.tm_mon,
					tm_time.tm_mday,
					tm_time.tm_hour,
					tm_time.tm_min, sSearchParaIns.nEndTime );
			
			nMaxFileNum = 10;

			SSearchCondition sSCondIns;// = (SSearchCondition*)pFileBuf;
			
			//pFileBuf= malloc( sizeof(SRecfileInfo)*nMaxFileNum );
			
			sSCondIns.emSearchWay = 0; 
			memcpy( &sSCondIns.sSearchPara, &sSearchParaIns, sizeof(sSearchParaIns) );
				
				//sSearchResultIns.psRecfileInfo = pFileBuf;
			
			//if(!sSearchResultIns.psRecfileInfo) printf("not enough memory for found files!\n");
			SBackTgtAtt sBackTgtAtt;

			memset( &sBackTgtAtt, 0, sizeof(sBackTgtAtt) );
			sBackTgtAtt.devt = EM_BACKUP_USB;
			sBackTgtAtt.fmt = EM_FORMAT_AVI;
			//sBackTgtAtt.mode = EM_DVDRW_MODE_NEW;
			
			rtn = ModSysComplexBackup(
					0,
					&sBackTgtAtt, 
					&sSCondIns );
			if(rtn)
			{
				printf("search failed, err %d!\n", rtn);

				getchar();
			}
			else
			{
				printf("found the %d files as following: \n", sSearchResultIns.nFileNum);

				for( i=0; i<sSearchResultIns.nFileNum; i++ )
					printf("file %d %d %d %d %d\n", 
							sSearchResultIns.psRecfileInfo[i].nChn,
							sSearchResultIns.psRecfileInfo[i].nDiskNo, 
							sSearchResultIns.psRecfileInfo[i].nStartTime,
							sSearchResultIns.psRecfileInfo[i].nEndTime,
							sSearchResultIns.psRecfileInfo[i].nFileNo );
			}

			
			//if(pFileBuf) free(pFileBuf);
			
			break;
		}
		case 1:
		{			
			//u32 evt = strtol(argv[1], NULL, 10);
			//u32 way = strtol(argv[2], NULL, 10);
			// cmd type ch y-m-d-h-m y-m-d-h-m
			// ..  1/2/4/8/10 .. 
			struct tm tm_time;
			u8 y,m,d, h,min, type, ch;
			char* pFileBuf = NULL;
			
			SSearchPara sSearchParaIns;
			SSearchResult sSearchResultIns;
			int nMaxFileNum;

			if(argc<13)
			{
				printf("no enough parameters, quit!!\n");
				
				break;
			}
			
			memset(&tm_time,0,sizeof(tm_time));
			
			type = strtol(argv[1], NULL, 10);
			ch = strtol(argv[2], NULL, 10);
			y = strtol(argv[3], NULL, 10);
			m = strtol(argv[4], NULL, 10);
			d = strtol(argv[5], NULL, 10);
			h = strtol(argv[6], NULL, 10);
			min = strtol(argv[7], NULL, 10);
			
			tm_time.tm_year = 2000+y - 1900;
			tm_time.tm_mon = m - 1;
			tm_time.tm_mday = d;
			tm_time.tm_hour = h;
			tm_time.tm_min = min;
			tm_time.tm_sec = 0;

			sSearchParaIns.nMaskType = type;
			sSearchParaIns.nMaskChn	 = ch;
			sSearchParaIns.nStartTime = mktime(&tm_time);
			
			printf("ymdhm %d-%d-%d-%d-%d %d\n", 
					tm_time.tm_year,
					tm_time.tm_mon,
					tm_time.tm_mday,
					tm_time.tm_hour,
					tm_time.tm_min, sSearchParaIns.nStartTime );
			
			y = strtol(argv[8], NULL, 10);
			m = strtol(argv[9], NULL, 10);
			d = strtol(argv[10], NULL, 10);
			h = strtol(argv[11], NULL, 10);
			min = strtol(argv[12], NULL, 10);
			
			tm_time.tm_year = 2000+y - 1900;
			tm_time.tm_mon = m - 1;
			tm_time.tm_mday = d;
			tm_time.tm_hour = h;
			tm_time.tm_min = min;
			tm_time.tm_sec = 0;
			
			sSearchParaIns.nEndTime = mktime(&tm_time);			

			printf("ymdhm %d-%d-%d-%d-%d %d\n", 
					tm_time.tm_year,
					tm_time.tm_mon,
					tm_time.tm_mday,
					tm_time.tm_hour,
					tm_time.tm_min, sSearchParaIns.nEndTime );
			
			nMaxFileNum = 10;

			pFileBuf= malloc(sizeof(SRecfileInfo)*nMaxFileNum);
			sSearchResultIns.psRecfileInfo = pFileBuf;
			
			if(!sSearchResultIns.psRecfileInfo) printf("not enough memory for found files!\n");
			
			rtn = ModSysComplexDMSearch(EM_FILE_REC, &sSearchParaIns, &sSearchResultIns, nMaxFileNum);

			if(rtn)
			{
				printf("search failed, err %d!\n", rtn);

				getchar();
			}
			else
			{
				printf("found the %d files as following: \n", sSearchResultIns.nFileNum);

				for( i=0; i<sSearchResultIns.nFileNum; i++ )
					printf("file %d %d %d %d %d\n", 
							sSearchResultIns.psRecfileInfo[i].nChn,
							sSearchResultIns.psRecfileInfo[i].nDiskNo, 
							sSearchResultIns.psRecfileInfo[i].nStartTime,
							sSearchResultIns.psRecfileInfo[i].nEndTime,
							sSearchResultIns.psRecfileInfo[i].nFileNo );
			}

			
			if(pFileBuf) free(pFileBuf);
			
			break;
		}
		default:
			printf("no such cmd!\n");
			break;
	}
	
	printf("This is the test program for mod syscomplex!\n");
#endif
	return 0;
}

