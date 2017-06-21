#ifndef _BIZ_PTZ_H_
#define _BIZ_PTZ_H_

#include "biz.h"
#include "mod_ptz.h"
#include "biz_manager.h"

typedef struct
{
	u8 nChnNum;
	u8* pDevPath;
} SBizPtz;

s32 PtzInit(u8 nEnable, SBizPtz* psInitPara);
s32 PtzSetPara(u8 nChn, SBizPtzPara* psBizPtzPara);
s32 PtzCtrl(SBizPtzCtrl* psBizPtzCtrl);
s32 PtzCheckIsTouring(u8 nChn);

//csp modify 20130323
s32 PtzInitPara(u8 nChn, SBizPtzPara* psBizPtzPara);

#endif

