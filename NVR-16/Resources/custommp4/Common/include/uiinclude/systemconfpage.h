#ifndef _SYSTEMCONFPAGE_H_
#define _SYSTEMCONFPAGE_H_

#include "ui.h"

#define PIC_SYSTEMCONF_PARAM			"data/pics/tools/param.png"
#define PIC_SYSTEMCONF_RECORDCONF		"data/pics/tools/recordset.png"//
#define PIC_SYSTEMCONF_IMAGE			"data/pics/tools/image.png"
#define PIC_SYSTEMCONF_ALARMIN			"data/pics/tools/alarmin.png"
#define PIC_SYSTEMCONF_ALARMOUT			"data/pics/tools/alarmout.png"//
#define PIC_SYSTEMCONF_ALARMSCH			"data/pics/tools/alarmsch.png"//wangcong
#define PIC_SYSTEMCONF_SCH				"data/pics/tools/sch.png"//²¼·Å
#define PIC_SYSTEMCONF_VIDEO			"data/pics/tools/video.png"//ok
#define PIC_SYSTEMCONF_EXCEPT			"data/pics/tools/except.png"//
#define PIC_SYSTEMCONF_PTZCONF			"data/pics/tools/ptzconf.png"
#define PIC_SYSTEMCONF_NETWORK			"data/pics/tools/network.png"//

#define IDC_PIC_SYSTEMCONF_PARAM		IDD_DIALOG_SYSTEMCONF+1

//pw 2010/6/11
#ifndef HANKE
#define IDC_PIC_SYSTEMCONF_RECORDCONF	IDD_DIALOG_SYSTEMCONF+2
#endif
#define IDC_PIC_SYSTEMCONF_IMAGE		IDD_DIALOG_SYSTEMCONF+3
#define IDC_PIC_SYSTEMCONF_ALARMIN		IDD_DIALOG_SYSTEMCONF+4
#define IDC_PIC_SYSTEMCONF_ALARMOUT		IDD_DIALOG_SYSTEMCONF+5
#define IDC_PIC_SYSTEMCONF_SCH			IDD_DIALOG_SYSTEMCONF+6
#define IDC_PIC_SYSTEMCONF_VIDEO		IDD_DIALOG_SYSTEMCONF+7
#define IDC_PIC_SYSTEMCONF_EXCEPT		IDD_DIALOG_SYSTEMCONF+8
#define IDC_PIC_SYSTEMCONF_PTZCONF		IDD_DIALOG_SYSTEMCONF+9

//pw 2010/6/11
#ifndef HANKE
#define IDC_PIC_SYSTEMCONF_NETWORK		IDD_DIALOG_SYSTEMCONF+10
#endif
#define IDC_PIC_SYSTEMCONF_ALARMSCH		IDD_DIALOG_SYSTEMCONF+11

#ifdef _NVR_
#define IDC_PIC_SYSTEMCONF_IPNC			IDD_DIALOG_SYSTEMCONF+12
#define IDC_PIC_SYSTEMCONF_LOGO			IDD_DIALOG_SYSTEMCONF+13
#endif

#define IDC_PROGRESS_SYSTEMCONF_LINE	IDD_DIALOG_SYSTEMCONF+26
#define IDC_PICTURE_SYSTEMCONF_TIPS		IDD_DIALOG_SYSTEMCONF+27

#ifdef _NVR_
#define IDC_STATIC_IPNCSET_CHN			IDD_DIALOG_IPNCSET+1
#define IDC_COMBOX_IPNCSET_CHN			IDD_DIALOG_IPNCSET+2
#define IDC_STATIC_IPNCSET_IP			IDD_DIALOG_IPNCSET+3
#define IDC_IPMAC_IPNCSET_IP			IDD_DIALOG_IPNCSET+4
#define IDC_STATIC_IPNCSET_NAME			IDD_DIALOG_IPNCSET+5
#define IDC_EDIT_IPNCSET_NAME			IDD_DIALOG_IPNCSET+6

//#define IDC_STATIC_IPNCSET_LANG		IDD_DIALOG_IPNCSET+7
//#define IDC_COMBOX_IPNCSET_LANG		IDD_DIALOG_IPNCSET+8
#define IDC_STATIC_IPNCSET_PWD			IDD_DIALOG_IPNCSET+7
#define IDC_EDIT_IPNCSET_PWD			IDD_DIALOG_IPNCSET+8

#define IDC_BUTTON_IPNCSET_INFO			IDD_DIALOG_IPNCSET+9
#define IDC_BUTTON_IPNCSET_OK			IDD_DIALOG_IPNCSET+10
#define IDC_BUTTON_IPNCSET_CANCEL		IDD_DIALOG_IPNCSET+11

#define IDC_STATIC_IPNCINFO_NAME		IDD_DIALOG_IPNCINFO+1
#define IDC_EDIT_IPNCINFO_NAME			IDD_DIALOG_IPNCINFO+2
#define IDC_STATIC_IPNCINFO_TYPE		IDD_DIALOG_IPNCINFO+3
#define IDC_EDIT_IPNCINFO_TYPE			IDD_DIALOG_IPNCINFO+4
#define IDC_STATIC_IPNCINFO_VER			IDD_DIALOG_IPNCINFO+5
#define IDC_EDIT_IPNCINFO_VER			IDD_DIALOG_IPNCINFO+6
#define IDC_STATIC_IPNCINFO_STIME		IDD_DIALOG_IPNCINFO+7
#define IDC_EDIT_IPNCINFO_STIME			IDD_DIALOG_IPNCINFO+8
#define IDC_STATIC_IPNCINFO_SD			IDD_DIALOG_IPNCINFO+9
#define IDC_EDIT_IPNCINFO_SD			IDD_DIALOG_IPNCINFO+10
#define IDC_STATIC_IPNCINFO_NET			IDD_DIALOG_IPNCINFO+11
#define IDC_EDIT_IPNCINFO_NET			IDD_DIALOG_IPNCINFO+12
#define IDC_BUTTON_IPNCINFO_RESTART		IDD_DIALOG_IPNCINFO+13
#define IDC_BUTTON_IPNCINFO_RESUME		IDD_DIALOG_IPNCINFO+14
#define IDC_BUTTON_IPNCINFO_OK			IDD_DIALOG_IPNCINFO+15
#define IDC_BUTTON_IPNCINFO_CANCEL		IDD_DIALOG_IPNCINFO+16
#endif

BOOL CreateSystemconfPage();
BOOL ShowSystemconfPage();

void OnSystemconfParam(Wnd *pWnd,Message_event msg_ev,s32 param);
void OnSystemconfRecordconf(Wnd *pWnd,Message_event msg_ev,s32 param);
void OnSystemconfImage(Wnd *pWnd,Message_event msg_ev,s32 param);
void OnSystemconfAlarmin(Wnd *pWnd,Message_event msg_ev,s32 param);
void OnSystemconfAlarmout(Wnd *pWnd,Message_event msg_ev,s32 param);
void OnSystemconfSCH(Wnd *pWnd,Message_event msg_ev,s32 param);
#ifdef ALARMSCH
void OnSystemconfALASCH(Wnd *pWnd,Message_event msg_ev,s32 param);
#endif
void OnSystemconfVideo(Wnd *pWnd,Message_event msg_ev,s32 param);
void OnSystemconfExcept(Wnd *pWnd,Message_event msg_ev,s32 param);
void OnSystemconfPTZConf(Wnd *pWnd,Message_event msg_ev,s32 param);
void OnSystemconfNetwork(Wnd *pWnd,Message_event msg_ev,s32 param);
#endif

#ifdef _NVR_
BOOL CreateIPNCSetPage();
#endif

