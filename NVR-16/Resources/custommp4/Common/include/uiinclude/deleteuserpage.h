#ifndef _DELETEPAGE_H
#define _DELETEPAGE_H

#include "ui.h"

#define IDC_STATIC_DELETEYUSER_TEXT			IDD_DIALOG_DELETEUSER+1
#define IDC_COMBO_DELETE_USER						IDD_DIALOG_DELETEUSER+2
#define IDC_BUTTON_DELETEUSER_OK				IDD_DIALOG_DELETEUSER+3
#define IDC_BUTTON_DELETEUSER_CANCEL		IDD_DIALOG_DELETEUSER+4

BOOL CreateDeleteUserPage();
BOOL ShowDeleteUserPage();

void OnDeleteBack(Wnd * pWnd,Message_event msg_ev,s32 param);

#endif
