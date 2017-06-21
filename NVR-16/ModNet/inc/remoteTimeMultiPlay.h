#ifndef __REMOTEMULTIPLAY_H_
#define __REMOTEMULTIPLAY_H_

#include "common_basetypes.h"
#include "netcommon.h"
#include "RemotePlay.h"
#include "custommp4.h"

#ifdef __cplusplus
extern "C" {
#endif

// #define 

#define MAX_SEARCH_NUM				4000

// typdef 

typedef struct
{
	 int real_file_nums;
	 SRecfileInfo rec_file_info[MAX_SEARCH_NUM];
	 custommp4_t *file;
	 int curIndex;
	 u32 curFrameTime;
}recplayfile_t;


// extern ..

#ifdef __cplusplus
}
#endif

#endif // __REMOTEMULTIPLAY_H_

