#ifndef _PAGE_LOGIN_H_
#define _PAGE_LOGIN_H_

#include "PageFrame.h"
//#include "PageChild.h"
#include <vector>

class CPageLogin:public CPageFrame
{
public:
	CPageLogin(VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon = NULL,CPage * pParent = NULL);
	~CPageLogin();

	VD_PCSTR GetDefualtTitleCenter();
	VD_BOOL UpdateData( UDM mode );

protected:
	char** pUserList;
	int nRealUserNum;

private:

	void OnClkBtn();

	
	CStatic* pStatic[2];
	CEdit* pEdit[2];
	CButton* pButton[2];
	CComboBox* pCombox;
	CStatic* pInfo;;

};

#endif //_PAGE_LOGIN_H_






