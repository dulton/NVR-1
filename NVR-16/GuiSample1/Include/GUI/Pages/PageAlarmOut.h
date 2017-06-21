#ifndef _PAGE_ALARMOUT_H_
#define _PAGE_ALARMOUT_H_

#include "PageFrame.h"
#include "PageAlarmCfgFrameWork.h"

#define ALARMOUT_SUBPAGES		3

#define ALARMOUT_COMMBUTTON (ALARMOUT_SUBPAGES+3)  //tabs + default,apply.exit

struct ALARMOUT_ITEM
{
	CStatic* pChnIndex;
	CEdit* pEditName;
	CComboBox* pComboDelay;
	CComboBox* pComboType;//csp modify 20140525
};

class CPageAlarmOut:public CPageFrame
{
public:
	CPageAlarmOut(VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon = NULL,CPage * pParent = NULL);
	~CPageAlarmOut();
	
	VD_BOOL UpdateData( UDM mode );	
	
	void SetSch( uchar nCh, uchar idx, uint* sch);
	void GetSch( uchar nCh, uint* sch);
	void ShowSch( uchar nCh );
	void WriteLogs();
	
private:
	void OnComboBoxChn();
	void OnClkCopy();
	VD_BOOL MsgProc( uint msg, uint wpa, uint lpa );
	CGridCtrl** GetCurGrid();
	uchar GetCurChn();

	uchar* bSchChange;
	uint** sch;

	CStatic* pInfoBar;
	CStatic* pIconInfo;

	void SetInfo(char* szInfo);
	void ClearInfo();

private:
	int nChMax;

	int m_nCurID;

	void SwitchPage( int subID);
	void ShowSubPage(int subID, BOOL bShow);

	void OnClickSubPage();

	CButton* pButton[ALARMOUT_COMMBUTTON];
	CTableBox* pTabFrame;

	void InitPage0();
	CTableBox* pTabPage0[2];
	CCheckBox* pChkChnAllPage0;
	CComboBox* pComboDelayAllPage0;
	CComboBox* pComboTypeAllPage0;//csp modify 20140525

	//csp modify 20140525
	//ALARMOUT_ITEM tAlarmOutPage0[1];
	ALARMOUT_ITEM tAlarmOutPage0[4];

	void InitPage1();
	CComboBox* pComboChnPage1;
	CComboBox* pComboCopy2ChnPage1;
	CButton* pBtCopyPage1;
	CTableBox* pTabSCHPage1;
	CGridCtrl* pSCHGridPage1[7];

	void InitPage2();
	CTableBox* pTabPage2;
	CCheckBox* pChkChnSoundPage2;
	CComboBox* pComboDelayPage2;
};

#endif

