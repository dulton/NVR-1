#ifndef __LIB_SNAPSHOT_H
#define __LIB_SNAPSHOT_H

#include "lib_common.h"

#ifdef __cplusplus
extern "C" {
#endif

//这些接口不合理
int tl_1CIFJpeg_Init(void);
int tl_1CIFJpeg_Capture(int Chn, unsigned char *in_buf, unsigned int in_len);
void tl_1CIFJpeg_Exit(void);

#ifdef HI3535
/*
EncChn	分辨率
0		704*576
1		352*288
2		176*144
*/
int tl_enc_jpeg_create_chn(u8 EncChn, int Width, int Height);
int tl_enc_jpeg_destroy_chn(u8 EncChn);
//int tl_enc_snap(u8 StreamChn, u8 EncChn, u8 *pdata, u32 *psize);
int tl_enc_snap(u8 EncChn, u16 *pWidth, u16 *pHeight, u8 *pdata, u32 *psize);



#endif

#ifdef __cplusplus
}
#endif

#endif

