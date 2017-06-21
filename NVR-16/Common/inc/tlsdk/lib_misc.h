#ifndef __LIB_MISC_H
#define __LIB_MISC_H

#include "lib_common.h"

#ifdef __cplusplus
extern "C" {
#endif

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

int tl_sata_status(void);

/*
	return value:
	0----->set ok
	1----->use VGA or HDMI default resolution
	usage:在tl_hslib_init之前必须调用改函数，而且只测试一次
*/
int tl_vga_resolution(int vga_resol);

/*args: 
		ctl: 1->enable display, 0->disable display
	return value:
		0->set ok, other fail
	usage:Set all of the output display device. HD,AD, SD
*/
int tl_display_ctl(int ctl);

/*
	return value:
	1-->plug ok
	0--->not plug 
*/
int tl_hdmi_detecte(void);

int config_hdmi_resolution(int resol);

int tl_VI_Enable_UserPic(VI_CHN ViChn);
int tl_VI_Disable_UserPic(VI_CHN ViChn);
HI_S32 tl_VI_SetUserPic(HI_CHAR *pszYuvFile, HI_U32 u32Width, HI_U32 u32Height, HI_U32 u32Stride);

#ifdef __cplusplus
}
#endif

#endif

