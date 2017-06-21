#ifndef _RESTARTPAGE_H_
#define _RESTARTPAGE_H_

#include "ui.h"

#define IDC_STATIC_RESTART_TEXT1		IDD_DIALOG_RESTART+1
#define IDC_STATIC_RESTART_TEXT2		IDD_DIALOG_RESTART+2

//pw 2010/6/11
#if defined(HANKE) ||defined(MEIDIANBEIER_UI)

#define IDC_PICTURE_RESTART_LOGOUT		IDD_DIALOG_RESTART+6
#define IDC_PICTURE_RESTART_RESTART		IDD_DIALOG_RESTART+7
#define IDC_PICTURE_RESTART_POWEROFF		IDD_DIALOG_RESTART+8

#define PIC_LOGOUT		"data/pics/tools/logout.png"
#define PIC_RESTART		"data/pics/tools/restart.png"
#define PIC_POWEROFF	"data/pics/tools/poweroff.png"

#else

#define IDC_BUTTON_RESTART_OK				IDD_DIALOG_RESTART+3
#define IDC_BUTTON_RESTART_CANCEL		IDD_DIALOG_RESTART+4
#define IDC_COMBO_RESTART_TYPE			IDD_DIALOG_RESTART+5
#endif







BOOL CreateRestartPage();
BOOL ShowRestartPage();

#endif
