#ifndef _LOGPAGE_H_
#define _LOGPAGE_H_

#include "ui.h"

#define IDC_STATIC_LOG_QUERYWAY				IDD_DIALOG_LOG+1
#define IDC_STATIC_LOG_MAINTYPE				IDD_DIALOG_LOG+2
#define IDC_STATIC_LOG_SECONDTYPE			IDD_DIALOG_LOG+3
#define IDC_STATIC_LOG_STARTTIME			IDD_DIALOG_LOG+4
#define IDC_STATIC_LOG_ENDTIME				IDD_DIALOG_LOG+5

#define IDC_COMBO_LOG_QUERYWAY				IDD_DIALOG_LOG+6
#define IDC_COMBO_LOG_MAINTYPE				IDD_DIALOG_LOG+7
#define IDC_COMBO_LOG_SECONDTYPE			IDD_DIALOG_LOG+8
#define IDC_EDIT_LOG_STARTDATE				IDD_DIALOG_LOG+9
#define IDC_EDIT_LOG_STARTTIME				IDD_DIALOG_LOG+10
#define IDC_EDIT_LOG_ENDDATE				IDD_DIALOG_LOG+11
#define IDC_EDIT_LOG_ENDTIME				IDD_DIALOG_LOG+12

#define IDC_BUTTON_LOG_QUERY				IDD_DIALOG_LOG+13

#define IDC_STATIC_LOG_INDEX				IDD_DIALOG_LOG+14
#define IDC_STATIC_LOG_RECORDTIME			IDD_DIALOG_LOG+15
#define IDC_STATIC_LOG_RECORDMAINTYPE		IDD_DIALOG_LOG+16
#define IDC_STATIC_LOG_RECORDSECONDTYPE		IDD_DIALOG_LOG+17

#define IDC_LIST_LOG_INFO					IDD_DIALOG_LOG+18

#define IDC_BUTTON_LOG_PREPAGE				IDD_DIALOG_LOG+19
#define IDC_BUTTON_LOG_NEXTPAGE				IDD_DIALOG_LOG+20

#define IDC_STATIC_LOG_SELECTPAGE			IDD_DIALOG_LOG+21
#define IDC_EDIT_LOG_SELECTPAGE				IDD_DIALOG_LOG+22

#define IDC_STATIC_LOG_COMPART				IDD_DIALOG_LOG+23
#define IDC_STATIC_LOG_TOTALPAGE			IDD_DIALOG_LOG+24

//#define IDC_BUTTON_LOG_MORE				IDD_DIALOG_LOG+25
#define IDC_BUTTON_LOG_BACK					IDD_DIALOG_LOG+26

#define IDC_DATETIME_LOG_STARTTIME			IDD_DIALOG_LOG+27
#define IDC_DATETIME_LOG_ENDTIME			IDD_DIALOG_LOG+28

#define IDC_BUTTON_LOG_GOTOPAGE				IDD_DIALOG_LOG+29

/* 07-10-08 */
#define IDC_STATIC_LOG_MOREINFO				IDD_DIALOG_LOG+30	
#define IDC_STATIC_LOG_TO					IDD_DIALOG_LOG+31	
#define IDC_INPUTBOX_LOG_DENO					IDD_DIALOG_LOG+32

#define IDC_COMBO_LOG_SECONDTYPE_ALMBEG		IDC_COMBO_LOG_SECONDTYPE+100
#define IDC_COMBO_LOG_SECONDTYPE_ALMEND		IDC_COMBO_LOG_SECONDTYPE+101
#define IDC_COMBO_LOG_SECONDTYPE_MDBEG		IDC_COMBO_LOG_SECONDTYPE+102
#define IDC_COMBO_LOG_SECONDTYPE_MDEND		IDC_COMBO_LOG_SECONDTYPE+103
#define IDC_COMBO_LOG_SECONDTYPE_ALL		IDC_COMBO_LOG_SECONDTYPE+104
#define IDC_COMBO_LOG_SECONDTYPE_STARTUP    IDC_COMBO_LOG_SECONDTYPE+105
#define IDC_COMBO_LOG_SECONDTYPE_SHUTDOWN   IDC_COMBO_LOG_SECONDTYPE+106
#define IDC_COMBO_LOG_SECONDTYPE_LLOGIN		IDC_COMBO_LOG_SECONDTYPE+107
#define IDC_COMBO_LOG_SECONDTYPE_LLOGOUT    IDC_COMBO_LOG_SECONDTYPE+108
#define IDC_COMBO_LOG_SECONDTYPE_LCONF		IDC_COMBO_LOG_SECONDTYPE+109
#define IDC_COMBO_LOG_SECONDTYPE_STARTV		IDC_COMBO_LOG_SECONDTYPE+110
#define IDC_COMBO_LOG_SECONDTYPE_STOPV		IDC_COMBO_LOG_SECONDTYPE+111
#define IDC_COMBO_LOG_SECONDTYPE_LUPDATE    IDC_COMBO_LOG_SECONDTYPE+112
#define IDC_COMBO_LOG_SECONDTYPE_FORMAT		IDC_COMBO_LOG_SECONDTYPE+113
#define IDC_COMBO_LOG_SECONDTYPE_RLOGIN		IDC_COMBO_LOG_SECONDTYPE+114
#define IDC_COMBO_LOG_SECONDTYPE_RLOGOUT    IDC_COMBO_LOG_SECONDTYPE+115
#define IDC_COMBO_LOG_SECONDTYPE_RCONF		IDC_COMBO_LOG_SECONDTYPE+116
#define IDC_COMBO_LOG_SECONDTYPE_RREBOOT    IDC_COMBO_LOG_SECONDTYPE+117
#define IDC_COMBO_LOG_SECONDTYPE_VOIPST		IDC_COMBO_LOG_SECONDTYPE+118
#define IDC_COMBO_LOG_SECONDTYPE_VOIPEND    IDC_COMBO_LOG_SECONDTYPE+119
#define IDC_COMBO_LOG_SECONDTYPE_RUPDATE    IDC_COMBO_LOG_SECONDTYPE+120
#define IDC_COMBO_LOG_SECONDTYPE_INPUTLOSS  IDC_COMBO_LOG_SECONDTYPE+121
#define IDC_COMBO_LOG_SECONDTYPE_BLIND		IDC_COMBO_LOG_SECONDTYPE+122
#define IDC_COMBO_LOG_SECONDTYPE_HDDERR		IDC_COMBO_LOG_SECONDTYPE+123
#define IDC_COMBO_LOG_SECONDTYPE_HDDFULL    IDC_COMBO_LOG_SECONDTYPE+124
#define IDC_COMBO_LOG_SECONDTYPE_IPCONF		IDC_COMBO_LOG_SECONDTYPE+125
#define IDC_COMBO_LOG_SECONDTYPE_ILLEGALOP  IDC_COMBO_LOG_SECONDTYPE+126
#define IDC_COMBO_LOG_SECONDTYPE_UNKNOWM    IDC_COMBO_LOG_SECONDTYPE+127
#define IDC_COMBO_LOG_SECONDTYPE_UNKNOWS    IDC_COMBO_LOG_SECONDTYPE+128

#define IDC_COMBO_LOG_SECONDTYPE_REBOOT   IDC_COMBO_LOG_SECONDTYPE+129
#define IDC_COMBO_LOG_SECONDTYPE_RSHUTDOWN   IDC_COMBO_LOG_SECONDTYPE+130

BOOL CreateLogPage();
BOOL ShowLogPage();

void OnLogQueryWay(Wnd* pWnd,Message_event event,s32 param);
void OnLogMainType(Wnd* pWnd,Message_event event,s32 param);
void OnLogSearch(Wnd* pWnd,Message_event event,s32 param);
void OnLogPre(Wnd* pWnd,Message_event event,s32 param);
void OnLogNext(Wnd* pWnd,Message_event event,s32 param);
void OnLogCurPage(Wnd* pWnd,Message_event event,s32 param);
void OnLogMore(Wnd* pWnd,Message_event event,s32 param);
void OnLogBack(Wnd* pWnd,Message_event event,s32 param);

#endif
