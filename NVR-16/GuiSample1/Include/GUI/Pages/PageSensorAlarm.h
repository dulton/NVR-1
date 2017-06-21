#ifndef _PAGE_SENSORALARM_H_
#define _PAGE_SENSORALARM_H_

#include "PageFrame.h"
#include "PageAlarmDeal.h"
#include "PageAlarmCfgFrameWork.h"

struct SENSEOR_BASEITEM
{
	CStatic* pChnIndex;
	CCheckBox* pChkEnable;
	CComboBox* pComboType;
	CEdit* pEditName;
};

struct SENSEOR_DEALITEM
{
	CStatic* pChnIndex;
	CComboBox* pComboDelay;
	CButton* pBtDeal;
};

typedef struct 
{
	char bEnable;
	int  nAlarmType;
	char szName[32];
	int  nDelayTime;
} SSensorAlarmPara;


#define SENSOR_SUBPAGES		3
#define SENSOR_COMMBUTTON (SENSOR_SUBPAGES+3)  //tabs + default,apply.exit

class CPageAlarmDeal;
class CPageSensorAlarm:public CPageFrame
{
public:
	CPageSensorAlarm(VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon = NULL,CPage * pParent = NULL);
	~CPageSensorAlarm();
	
	VD_BOOL UpdateData( UDM mode );
	void GetPara(uchar nCh, SGuiAlarmDispatch *pPara);
	void SavePara(uchar nCh, SGuiAlarmDispatch *pPara);
	void SaveDealPara2Cfg(uchar nRealChNum);
	
	void SetSch( uchar nCh, uchar idx, uint* sch);
	void GetSch( uchar nCh, uint* sch);
	void ShowSch( uchar nCh );

	void WriteLogs(int PageIndex);
		
private:
#define TAB_NUM	2
	CScrollBar* 	pScrollbar[TAB_NUM];
	SSensorAlarmPara*	pTabPara;
	int             nRealRow[TAB_NUM];
	int             nPageNum[TAB_NUM];
	int             nHideRow[TAB_NUM];
	int             nCurPage[TAB_NUM];
	void 			OnTrackMove0();
	void			AdjHide();
	STabSize* 		pTabSz;
	
	void OnComboBoxChn();
	void OnClkCopy();
	VD_BOOL MsgProc( uint msg, uint wpa, uint lpa );
	CGridCtrl** GetCurGrid();
	uchar GetCurChn();
	uchar* bSchChange;
	uint** sch;

	CStatic* pIconInfo;
	CStatic* pInfoBar;
	void SetInfo(char* szInfo);
	void ClearInfo();
	
private:
	void LoadPara();
	void OnEditName();
	void OnComboBoxPg1();
	void OnComboBoxPg2();
	void OnClickEnable();
	
	uchar nChMax;
	SGuiAlarmDispatch *psAlarmDispatchIns;
	uchar bModified;
	void OnClickTypeAll();

	void OnClickEnableAll();
	int m_nCurID;

	void SwitchPage( int subID);
	void ShowSubPage(int subID, BOOL bShow);

	void OnClickSubPage();

	CButton* pButton[SENSOR_COMMBUTTON];
	CTableBox* pTabFrame;

	void InitPage0();
	CTableBox* pTabPage0[2];
	CStatic* pTabTitlePage0[4];
	CStatic* pStaticAllPage0;
	CCheckBox* pChkChnAllPage0;
	CCheckBox* pChkEnableAllPage0;
	CComboBox* pComboTypeAllPage0;

	SENSEOR_BASEITEM tBasePage0[4+2];

	void InitPage1();
	CTableBox* pTabPage1[2];
	CStatic* pTabTitlePage1[3];
	CStatic* pStaticAllPage1;
	CCheckBox* pChkChnAllPage1;
	CComboBox* pComboDelayAllPage1;
	CButton* pBtDealAllPage1;
	void OnClkDealAllPage1();
	void OnClkAlarmDeal();

	SENSEOR_DEALITEM tDealPage1[4+2];

	CPageAlarmDeal* m_pPageAlarmDeal;

	void InitPage2();
	CComboBox* pComboChnPage2;
	CComboBox* pComboCopy2ChnPage2;
	CButton* pBtCopyPage2;
	CTableBox* pTabSCHPage2;
	CGridCtrl* pSCHGridPage2[7];



};


#endif
