#ifndef _TOOLPAGE_H_
#define _TOOLPAGE_H_

#include "ui.h"

#define PIC_TOOL_PARAM					"data/pics/tools/param.png"
#define PIC_TOOL_HDD					"data/pics/tools/hdd.png"//
#define PIC_TOOL_ONLINE					"data/pics/tools/online.png"
#define PIC_TOOL_LOCK					"data/pics/tools/lock.png"
#define PIC_TOOL_CLEARALARM				"data/pics/tools/clearalarm.png"//
#define PIC_TOOL_KEY					"data/pics/tools/key.png"
#define PIC_TOOL_DEFAULT				"data/pics/tools/default.png"//ok
#define PIC_TOOL_UPDATE					"data/pics/tools/update.png"//
#define PIC_TOOL_LAYOUT					"data/pics/tools/layout.png"
#define PIC_TOOL_RESTART				"data/pics/tools/restart.png"
#define PIC_TOOL_INFO					"data/pics/tools/info.png"//
#define PIC_TOOL_DATETIME				"data/pics/tools/time.png"//comment by lshu 20070521
#define PIC_TOOL_USER					"data/pics/tools/usermanage.png"//

#define IDC_STATIC_TOOL_PARAM			IDD_DIALOG_TOOL+1
#define IDC_STATIC_TOOL_HDD				IDD_DIALOG_TOOL+2
#define IDC_STATIC_TOOL_ONLINE			IDD_DIALOG_TOOL+3
#define IDC_STATIC_TOOL_LOCK			IDD_DIALOG_TOOL+4
#define IDC_STATIC_TOOL_CLEARALARM		IDD_DIALOG_TOOL+5
#define IDC_STATIC_TOOL_KEY				IDD_DIALOG_TOOL+6
#define IDC_STATIC_TOOL_RESUME			IDD_DIALOG_TOOL+7
#define IDC_PICTURE_TOOL_RESUME			0
#define IDC_STATIC_TOOL_UPDATE			IDD_DIALOG_TOOL+8
#define IDC_STATIC_TOOL_LAYOUT			IDD_DIALOG_TOOL+9
#define IDC_STATIC_TOOL_RESTART			IDD_DIALOG_TOOL+10
#define IDC_STATIC_TOOL_INFO			IDD_DIALOG_TOOL+11
#define IDC_PICTURE_TOOL_PARAM			IDD_DIALOG_TOOL+12
#define IDC_PICTURE_TOOL_HDD			IDD_DIALOG_TOOL+13
#define IDC_PICTURE_TOOL_ONLINE			IDD_DIALOG_TOOL+14
#define IDC_PICTURE_TOOL_LOCK			IDD_DIALOG_TOOL+15
#define IDC_PICTURE_TOOL_CLEARALARM		IDD_DIALOG_TOOL+16
#define IDC_PICTURE_TOOL_KEY			IDD_DIALOG_TOOL+17
#define IDC_PICTURE_TOOL_DEFAULT		IDD_DIALOG_TOOL+18
#define IDC_PICTURE_TOOL_UPDATE			IDD_DIALOG_TOOL+19
#define IDC_PICTURE_TOOL_LAYOUT			IDD_DIALOG_TOOL+20
#define IDC_PICTURE_TOOL_RESTART		IDD_DIALOG_TOOL+21
#define IDC_PICTURE_TOOL_INFO			IDD_DIALOG_TOOL+22

#define IDC_STATIC_TOOL_DATETIME		IDD_DIALOG_TOOL+23//comment by lshu 2007052
#define IDC_PICTURE_TOOL_DATETIME		IDD_DIALOG_TOOL+24//comment by lshu 20070521
#define IDC_PICTURE_TOOL_USER			IDD_DIALOG_TOOL+25
#define IDC_PICTURE_TOOL_LINE			IDD_DIALOG_TOOL+26
#define IDC_PICTURE_TOOL_TIPS			IDD_DIALOG_TOOL+27
#define IDC_PICTURE_TOOL_CONTACT		IDD_DIALOG_TOOL+28

#ifdef _NVR_
#define IDC_PICTURE_TOOL_IPNC			IDD_DIALOG_TOOL+29
#define IDC_PICTURE_TOOL_SUPPORT		IDD_DIALOG_TOOL+30

#define IDC_PICTURE_SUPPORT_INFO		IDD_DIALOG_SUPPORT+1
#define IDC_PICTURE_SUPPORT_LOGO		IDD_DIALOG_SUPPORT+2
#endif

BOOL CreateToolPage();
BOOL ShowToolPage();

#ifdef JUNMINGSHI_KAICONG
BOOL CreateSupportPage();
BOOL ShowSupportPage();
#endif

void OnToolParam(Wnd *pWnd,Message_event msg_ev,s32 param);
void OnToolOnline(Wnd *pWnd,Message_event msg_ev,s32 param);
void OnToolLock(Wnd *pWnd,Message_event msg_ev,s32 param);
void OnToolClearAlarm(Wnd *pWnd,Message_event msg_ev,s32 param);
void OnToolKey(Wnd *pWnd,Message_event msg_ev,s32 param);
void OnToolResume(Wnd *pWnd,Message_event msg_ev,s32 param);
void OnToolLayout(Wnd *pWnd,Message_event msg_ev,s32 param);
void OnToolRestart(Wnd *pWnd,Message_event msg_ev,s32 param);
void OnToolInfo(Wnd *pWnd,Message_event msg_ev,s32 param);
void OnToolSysDateTime(Wnd *pWnd,Message_event msg_ev,s32 param);
#endif

