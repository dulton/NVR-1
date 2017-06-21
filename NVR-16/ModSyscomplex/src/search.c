// file description

#include "search.h"
#include "common_msgcmd.h"
#include "update.h"

//** macro

//** typedef

//** local var

//** global var
extern SModSyscplx sModSysIns;

//** local functions

//** global functions

s32 modSysComplex_Search_do( PSSearch para )
{
	//int i;
	s32 rtn;
	s32 nFileNum;
	search_param_t sSearchParaIns;
	SSearchPara*	psSearchParaIns;
	disk_manager*	pHddMgr;

	MODSYS_ASSERT(NULL!=para);
	MODSYS_ASSERT(NULL!=para->psSearchPara&&NULL!=para->psSearchResult);
	
	pHddMgr = sModSysIns.pDiskManager;
	
	MODSYS_ASSERT(NULL!=pHddMgr);
	
	// parse para
	//
	psSearchParaIns = para->psSearchPara;
	//printf("%s: emFileType: %d\n", __func__, para->emFileType);
	//printf("%s start %d, end %d\n", __func__, psSearchParaIns->nStartTime, psSearchParaIns->nEndTime );

	// search files on mskchn and on msktype
	memset(&sSearchParaIns, 0, sizeof(sSearchParaIns));
	sSearchParaIns.channel_no	= psSearchParaIns->nMaskChn;
	sSearchParaIns.type			= psSearchParaIns->nMaskType;
	sSearchParaIns.start_time	= psSearchParaIns->nStartTime;
	sSearchParaIns.end_time		= psSearchParaIns->nEndTime;

	if (EM_FILE_REC == para->emFileType)//Â¼Ïñ
	{
		nFileNum = search_all_rec_file( pHddMgr,
							&sSearchParaIns,
							(recfileinfo_t *)&para->psSearchResult->psRecfileInfo[0],
							para->nMaxFileNum );
		
		if(nFileNum>0)
		{
			if(nFileNum<para->nMaxFileNum)
				para->psSearchResult->nFileNum = nFileNum;
			else
				para->psSearchResult->nFileNum = para->nMaxFileNum;
			
			MODSYS_DEBUG_STR("find files number: ");
			MODSYS_DEBUG(nFileNum);
			
			/*for(i=0; i<nFileNum; i++)
			{
				printf("result : \n");
				printf("ch %d starttime %d endtime %d\n", 
					((recfileinfo_t *)&(para->psSearchResult->psRecfileInfo[i]))->channel_no,
					((recfileinfo_t *)&(para->psSearchResult->psRecfileInfo[i]))->start_time,
					((recfileinfo_t *)&(para->psSearchResult->psRecfileInfo[i]))->end_time);
			}*/
			
			rtn = 0;
		}
		else
		{
			para->psSearchResult->nFileNum = 0;
			MODSYS_DEBUG_STR("no files found or too many files found!\n");
			rtn = -1;// nofile
		}
	}
	else	//EM_FILE_SNAP Í¼Æ¬
	{
		nFileNum = search_all_rec_snap(pHddMgr,
							&sSearchParaIns,
							(recsnapinfo_t *)&para->psSearchResult->psRecSnapInfo[0],
							para->nMaxFileNum);
		
		if(nFileNum>0)
		{
			if(nFileNum<para->nMaxFileNum)
				para->psSearchResult->nFileNum = nFileNum;
			else
				para->psSearchResult->nFileNum = para->nMaxFileNum;
			
			MODSYS_DEBUG_STR("find Snaps number: ");
			MODSYS_DEBUG(nFileNum);
			
			/*for(i=0; i<nFileNum; i++)
			{
				printf("result : \n");
				printf("ch %d starttime %d endtime %d\n", 
					((recfileinfo_t *)&(para->psSearchResult->psRecfileInfo[i]))->channel_no,
					((recfileinfo_t *)&(para->psSearchResult->psRecfileInfo[i]))->start_time,
					((recfileinfo_t *)&(para->psSearchResult->psRecfileInfo[i]))->end_time);
			}*/
			
			rtn = 0;
		}
		else
		{
			para->psSearchResult->nFileNum = 0;
			MODSYS_DEBUG_STR("no Snaps found or too many files found!\n");
			rtn = -1;// nofile
		}
	}
	
	MODSYS_DEBUG(0);
	
	return rtn;
}

