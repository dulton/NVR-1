#ifndef _PAGE_OTHERALARM_H_
#define _PAGE_OTHERALARM_H_

#include "PageFrame.h"

class CPageOtherAlarm:public CPageFrame
{
public:
	CPageOtherAlarm(VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon = NULL,CPage * pParent = NULL);
	~CPageOtherAlarm();
	VD_BOOL UpdateData( UDM mode );
	VD_BOOL MsgProc( uint msg, uint wpa, uint lpa );

private:
	CTableBox* pTab;
	CComboBox* pComboAlarmType;
	CCheckBox* pChkSoundAlarm;
	CCheckBox* pChkMail;
	CCheckBox* pChkTriggerAlarm[1];
	CComboBox* pComboDiskAlarm;
	CStatic* pInfoBar;
	CStatic* pIconInfo;

	void SetInfo(char* szInfo);
	void ClearInfo();

	CButton* pButton[3]; //default apply exit

	void OnClkApp();


};


#endif
