#ifndef _PAGE_ONLINE_USER_H_
#define _PAGE_ONLINE_USER_H_

#include "PageFrame.h"
//#include "PageChild.h"
#include <vector>


class CPageOnlineUser :public CPageFrame
{
public:
	CPageOnlineUser(VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon = NULL,CPage * pParent = NULL);
	~CPageOnlineUser();
	
	VD_BOOL UpdateData( UDM mode );

	VD_PCSTR GetDefualtTitleCenter();
	void OnClickCtl();
	void OnClickBtn();

	void SetUsername(int row, char* szText);
	void SetIP(int row, char* szText);
	void SetState(int row, char* szText);

protected:


private:

	CStatic* pTitle[3];
	CStatic* pUsername[9];
	CStatic* pIP[9];
	CStatic* pState[9];
	CStatic* pResult;
	CButton* pCtl[4];
	CButton* pBtn[2];

	CTableBox* pTable0;
	CTableBox* pTable1;

	CStatic* pInfoBar;
	void SetInfo(char* szInfo);
	void ClearInfo();

};

#endif //_PAGE_ONLINE_USER_H_




