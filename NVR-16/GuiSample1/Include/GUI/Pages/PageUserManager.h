#ifndef _PAGE_USERMGR_H_
#define _PAGE_USERMGR_H_

#include "PageFrame.h"
//#include "PageChild.h"
#include <vector>

class CPageEditPwd;
class CPageAddUser;
class CPageModifyUser;

class CPageUserManager :public CPageFrame
{
public:
	CPageUserManager(VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon = NULL,CPage * pParent = NULL);
	~CPageUserManager();

	VD_PCSTR GetDefualtTitleCenter();
	VD_BOOL UpdateData( UDM mode );
	void CleanUpBckCol();
	void SetFatherPageTitle();
	int GetCurUserNum();//cw_tab
	void RefreshPageString(void);
protected:
	
	char** pUserList;
	int nRealUserNum;
	

private:
	int m_nCursel;
	int m_itemNum;
	int m_nCurMouseLine;

	CStatic* pStatic[30];
	CButton* pButton[5];
	CTableBox* pTable;

	void OnClkBtn();
	void SetTable(int col,int row,char* text);

	VD_BOOL MsgProc( uint msg, uint wpa, uint lpa );
	void SelectItem( int index );
	void MouseMoveToLine( int index );

	CPageEditPwd* m_pPageEditPwd;
	CPageAddUser* m_pPageAddUser;
	CPageModifyUser* m_pPageModifyUser;
	
	CStatic* pszResult;
	
	//CButton* pBtPageCtr[4];//csp modify
	//void OnClickPageCtr();//csp modify
	
	CTableBox* pTabPage;
	int curPage;
	void SetOneRow(int page, int index, char** pUserList);
};

#endif //_PAGE_USERMGR_H_

