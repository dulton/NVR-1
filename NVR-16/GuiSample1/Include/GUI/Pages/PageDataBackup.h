#ifndef _PAGE_DATABACKUP_H_
#define _PAGE_DATABACKUP_H_

#include "PageFrame.h"
#include "BizData.h"

/*typedef struct _sRecFile
{
	uchar nChn;
	uint nBegin;
	uint nEnd;
} SRecFile;*/

#define MAX_SEARCH_FILE_NUM 4000 //zlbfix20110719 backup
#define TAB_LINES		9

#define BACKUP_SUBPAGES		2


class CPageBackupInfo;
class CPageSnapPlayBack;

class CPageDataBackup:public CPageFrame
{
public:
	CPageDataBackup(VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon = NULL,CPage * pParent = NULL);
	~CPageDataBackup();
	VD_BOOL UpdateData( UDM mode );
	
	void SetChn(int row, char* szText);
	void SetBegin(int row, char* szText);
	void SetEnd(int row, char* szText);
	void SetState(int row, char* szText);
	void SetResult(char* szText);
	int SetProgress(int nProg);
	void BackupwithTime(EMBIZFILETYPE fileType, SBizBackTgtAtt* attribute, SBizSearchCondition* condition);
private:
	int nChMax;
	int m_mCurID;
	CRect m_screenRT;
	
	CStatic* Progbar;
	int progWidth;
	int progRight;

	CStatic* stateInfo;

	char m_sTime[32];
	char m_eTime[32];
	
	SBizRecfileInfo m_sFileList[MAX_SEARCH_FILE_NUM];
	uchar m_nSelect[MAX_SEARCH_FILE_NUM];
	u8 m_nPageSize;
	u32 m_nTotalPages;
	u32 m_nCurPage;

	SBizBackTgtAtt sBackTgtAtt;
	SBizSearchCondition sSearchCondition;
	SBizRecfileInfo* sFileList;
	//SBizRecSnapInfo *sSnapList;
	
	void SetProg(int curpage, int totalpage);
	int m_nFiles;
	void OnClickChooseChn();
	void OnCalendarAllChn();
	void OnCalendarChn();
	
	void InitPageLeft0();
	void InitPageRight0();

	CTableBox* pTabPageLeft[2];
	CStatic* pTitlePageLeft[5];
	CStatic* pszResult;
	
	CStatic* pChn[9];
	CStatic* pBegin[9];
	CStatic* pEnd[9];
	CStatic* pState[9];

	CButton* pBtPageCtr[4];
	void OnClickPageCtr();
	
	CCheckBox* pChkChnLeft[10];
	CStatic* pszChooseAll;

	CTableBox* pTabChnSearch;
	CCheckBox* pChkChn[5*8]; //最大支持32
	CStatic*	pStaticChn[5*8]; //pStaticChn[4]
	CStatic*	pStaticBegin;
	CStatic*	pStaticEnd;

	CDateTimeCtrl* pBeginDatePR;
	CDateTimeCtrl* pBeginTimePR;
	CDateTimeCtrl* pEndDatePR;
	CDateTimeCtrl* pEndTimePR;

	CStatic* pInfoBar;
	CButton* pBtBackup;
	CButton* pBtSearch;
	CButton* pBtQuit;
	void OnBtBackup();
	void OnBtSearch();
	void OnBtQuit();

	void SetInfo(char* szInfo);
	void ClearInfo();

	CPageBackupInfo* m_pPageBackupInfo;

	int m_nCurMouseLine;
	void MouseMoveToLine( int index );
	VD_BOOL MsgProc( uint msg, uint wpa, uint lpa );

	//yaogang modify 20150121
	void InitPage0();
	void InitPage1();
	void SwitchPage( int subID);
	void ShowSubPage(int subID, BOOL bShow);
	
	void OnClickSubPage();
	void InitPageLeft1();
	void InitPageRight1();
	void OnPG3AllChn();
	void OnPG3Chn();
	void OnDaySelected();
	void UpdateCalendar(SYSTEM_TIME* pTime);
	
	//yaogang modify 20150121
	CPageSnapPlayBack *m_pPageSnapPB;
	CTableBox* pTabFrame1;
	CTableBox* pTabFrame2;
	CButton* pButton[BACKUP_SUBPAGES];//sub
	CCalendar* pCalendar;

	CStatic* pTextPG3BeginTime;
	CDateTimeCtrl* pBeginDateP3;
	CDateTimeCtrl* pBeginTimeP3;
	CStatic* pTextPG3EndTime;
	CDateTimeCtrl* pEndDateP3;
	CDateTimeCtrl* pEndTimeP3;
	CStatic* pTextPG3Chn;
	CStatic* pTextPG3PicType;
	CTableBox* pTabPG3Chn;
	CCheckBox* pPG3ChkChn[5*8]; // 最大支持32路
	CStatic* pPG3StaticChn[4*8];
	CComboBox *pComboBoxPicType;
};

#endif

