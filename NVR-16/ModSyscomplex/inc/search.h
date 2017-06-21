#ifndef __SEARCH_H_
#define __SEARCH_H_

#include "partitionindex.h"
#include "Mod_syscomplex.h"
#include "diskmanage.h"

#ifdef __cplusplus
extern "C" {
#endif

// #define

#define SEARCH_FILE_MAX 4000

// record type mask
//#define REC_MANUAL   (1<<0)
//#define REC_ALARM	 (1<<1)
//#define REC_MOVE	 (1<<2)
//#define REC_TIMER	 (1<<3)

// typdef
typedef struct _sSearch
{
	u16 			nMaxFileNum;
	EMFILETYPE 		emFileType;
	SSearchPara* 	psSearchPara;
	SSearchResult* 	psSearchResult;
} SSearch, *PSSearch;

// extern ..
extern s32 modSysComplex_Search_do( PSSearch para );

#ifdef __cplusplus
}
#endif


#endif // __SEARCH_H_
