#ifndef _MOBILE_H_
#define _MOBILE_H_

#include "netcommon.h"

int mobile_init(int listenPort, int videoChns, int audioChns, PStreamRequestCB pCB, int nVEncType, int yFactor);
void mobile_send_media_frame(PFRAMEHDR pFrmHdr, u32 chn);
u8 mobile_get_status(u32 chn);

#endif

