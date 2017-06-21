#ifndef _MODIFYPAGE_H
#define _MODIFYPAGE_H

#include "ui.h"


#define IDC_STATIC_MODIFYUSER_TEXT			IDD_DIALOG_MODIFYUSER+1
#define IDC_COMBO_MODIFY_USER						IDD_DIALOG_MODIFYUSER+2
#define IDC_BUTTON_MODIFYUSER_OK				IDD_DIALOG_MODIFYUSER+3
#define IDC_BUTTON_MODIFYUSER_CANCEL		IDD_DIALOG_MODIFYUSER+4

BOOL CreateModifyUserPage();
BOOL ShowModifyUserPage();

void OnModifyBack(Wnd * pWnd,Message_event msg_ev,s32 param);

#endif



