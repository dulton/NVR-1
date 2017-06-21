#include <stdio.h>
#include "lib_common.h"
#include "lib_md.h"

int main()
{
	int ret;
	
	int init_param = 0;
  init_param |= TL_128MDDR;
  init_param |= TL_VGA;
  init_param |= TL_PAL;
  tl_hslib_init_c(TL_BOARD_TYPE_R9504_4D1, init_param);

	ret = tl_md_open();
	if(ret)
	{
		printf("err0\n");
		return 0;
	}
	
	ret = tl_md_enable(0);
	if(ret)
	{
		printf("err2\n");
		return 0;
	}
	
	md_atr_t para;
	memset(&para, 0xff, sizeof(para));
	para.sad = 4;
	para.sad_num = 1;	
	ret = tl_md_set_atr(0, &para);
	if(ret)
	{
		printf("err1\n");
		return 0;
	}
	
	md_result_t result;
	unsigned char cou = 1;
	while(cou++<15)
	{
		ret = tl_md_read_result(&result, 100*1000);
		printf("ret = %d, index = %d\n", ret, result.venc_idx);
	}
	
	ret = tl_md_disable(0);
	if(ret)
	{
		printf("err3\n");
		return 0;
	}
	
	ret = tl_md_close();
	if(ret)
	{
		printf("err4\n");
		return 0;
	}	
}
