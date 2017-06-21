
#ifndef __LIB_SNAPSHOT_H
#define __LIB_SNAPSHOT_H

#include "lib_common.h"

int tl_1CIFJpeg_Init(void);
int tl_1CIFJpeg_Capture(int Chn, unsigned char *in_buf, unsigned int in_len);
void tl_1CIFJpeg_Exit(void);

#endif
