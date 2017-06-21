#ifndef _RESUMEPAGE_H_
#define _RESUMEPAGE_H_

#include "ui.h"

#define IDC_STATIC_RESUME_TEXT1		IDD_DIALOG_RESUME+1
#define IDC_STATIC_RESUME_TEXT2		IDD_DIALOG_RESUME+2

#define IDC_BUTTON_RESUME_OK		IDD_DIALOG_RESUME+3
#define IDC_BUTTON_RESUME_CANCEL	IDD_DIALOG_RESUME+4

#define IDC_STATIC_RESUME_TEXT3		IDD_DIALOG_RESUME+5
#define IDC_STATIC_RESUME_INFO		IDD_DIALOG_RESUME+6

BOOL CreateResumePage();
BOOL ShowResumePage();

void OnResumeOK(Wnd *pWnd,Message_event msg_ev,s32 param);
void OnResumeCancel(Wnd *pWnd,Message_event msg_ev,s32 param);

#endif

