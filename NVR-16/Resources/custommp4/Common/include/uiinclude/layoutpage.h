#ifndef _LAYOUTPAGE_H_
#define _LAYOUTPAGE_H_

#include "ui.h"

#ifdef INPUT_OUTPUT_MODULE
#define IDC_STATIC_LAYOUT_OPTYPE		IDD_DIALOG_LAYOUT+1
#define IDC_COMBO_LAYOUT_OPTYPE			IDD_DIALOG_LAYOUT+2

#define IDC_BUTTON_LAYOUT_OK			IDD_DIALOG_LAYOUT+3
#define IDC_BUTTON_LAYOUT_CANCEL		IDD_DIALOG_LAYOUT+4

#define IDC_STATIC_LAYOUT_NOTICE		IDD_DIALOG_LAYOUT+5

BOOL CreateLayoutPage();
BOOL ShowLayoutPage();

void OnLayoutOK(Wnd* pWnd,Message_event event,s32 param);
void OnLayoutCancel(Wnd* pWnd,Message_event event,s32 param);
#endif

#endif
