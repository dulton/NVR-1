
#ifndef __LIB_MISC_H
#define __LIB_MISC_H

#include "lib_common.h"

//args: 
//	ctl: 1->enable, 0->disable
//return value:
//	0->set ok, other fail
int tl_rs485_ctl(int ctl);

/*args: 
		ctl: 1->enable, 0->disable
	return value:
		0->set ok, other fail
*/
int tl_buzzer_ctl(int ctl);

/*
args: 
		dataQ : input radom data
		size : radom data size(fixed to 8)
return value:
		0->ok, other fail
*/
int tl_license_check(char *dataQ, int size);

/*args: 
		ctl: 1->enable, 0->disable
	return value:
		0->set ok, other fail
*/
int tl_keypad_ctl(int ctl);

int tl_hdmi_detecte(void);
/*
	return value:
	1-->plug ok
	0--->not plug 
*/

int tl_sata_status(void);


int tl_vga_resolution(int vga_resol);
/*
	return value:
	0----->set ok
	1----->use VGA or HDMI default resolution
	usage:在tl_hslib_init之前必须调用改函数，而且只测试一次
*/

/*args: 
		ctl: 1->enable display, 0->disable display
	return value:
		0->set ok, other fail
	usage:Set all of the output display device. HD,AD, SD
*/
int tl_display_ctl(int ctl);

int  config_hdmi_resolution(int resol);

int get_mdin240_state(void);
/*
	return value:
	1----->HDMI OUTPUT
	0----->VGA   OUTPUT
	usage:
	调用顺序:
	1、get_mdin240_state 得到HDMI的状态；
	2、config_hdmi_resolution设置HDMI输出分辨率;
	3、tl_hslib_init_c初始化。
*/

int tl_VI_Enable_UserPic(VI_CHN ViChn);
int tl_VI_Disable_UserPic(VI_CHN ViChn);
HI_S32 tl_VI_SetUserPic(HI_CHAR *pszYuvFile, HI_U32 u32Width, HI_U32 u32Height, HI_U32 u32Stride);

#endif
