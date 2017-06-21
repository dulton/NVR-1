
#ifndef __LIB_MOTION_DETECT_H
#define __LIB_MOTION_DETECT_H

#include "lib_common.h"

#define MD_AREA_MAX_ROW	36 //the VDA size is VDA_MB_16PIXEL
#define MD_AREA_MAX_COLUMN 	60 //the VDA size is VDA_MB_16PIXEL

typedef struct{
	unsigned short sad;
	unsigned short sad_num;
	unsigned long long area[MD_AREA_MAX_ROW];
}md_atr_t;

typedef struct{
	int venc_idx;
}md_result_t;

int tl_md_open(void);
/*
	ret:
		< 0: error
		== 0: nothing(maybe timeout)
		== 1: [result.venc_idx] md condition ok
		== 2: [result.venc_idx] md condition fail
*/
int tl_md_read_result(md_result_t *result, unsigned int timeout_us);
int tl_md_close(void);

int tl_md_enable(int venc_idx);
int tl_md_disable(int venc_idx);
int tl_md_set_atr(int venc_idx, md_atr_t *atr);


#endif
