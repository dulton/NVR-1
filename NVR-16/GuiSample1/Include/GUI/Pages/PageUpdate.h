#ifndef _PAGE_UPDATE_H_
#define _PAGE_UPDATE_H_

#include "PageFrame.h"

class CPageUpdate:public CPageFrame
{
public:
	CPageUpdate(VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon = NULL,CPage * pParent = NULL);
	~CPageUpdate();
	void RemoteUpdateStart(u32 type, u32 nRslt);//cw_remote
	int GetCurFileNum();//cw_tab
	void SetRemoteText(u8 nRslt);
	void SetName(int row, char* szText);
	void SetSize(int row, char* szText);
	void SetType(int row, char* szText);
	void SetModifyDate(int row, char* szText);
	void SetResult(char* szText);
	VD_BOOL UpdateData( UDM mode );
	VD_BOOL MsgProc( uint msg, uint wpa, uint lpa );
	int SetUpdateProg(int nProg);

private:

	void Clear();
	void ClearBkColor();
	void OnComboBox();

	CRect m_screenRT;
	CStatic* percentInfo;
	CStatic* updateProg;
	int progWidth;
	int progRight;
	int curPage;

	CStatic* stateInfo;

	int m_RemoteUpFlag;
	int count;
	uint m_nFile;
	void Refresh();
	CComboBox *pCombo;
	CTableBox* pTab[2];
	CStatic* pTitlePage[4];
	CStatic* pszResult;
	
	CStatic* pName[8];
	CStatic* pSize[8];
	CStatic* pType[8];
	CStatic* pModifyDate[8];

	CButton* pBtPageCtr[4];
	void OnClickPageCtr();

	CButton* pBtRefresh;
	CButton* pBtUpdate;
	CButton* pBtExit;
	void OnBtRefresh();
	void OnBtUpdate();
	void OnBtExit();
	void SelectDskItem(int index);
	int m_nCursel;
};


#endif
