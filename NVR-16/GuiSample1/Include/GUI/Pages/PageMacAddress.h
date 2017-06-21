#ifndef _PAGE_EDIT_MAC_H_
#define _PAGE_EDIT_MAC_H_

#include "PageFrame.h"
#include <vector>

class CPageNetwork;
class CPageEditMacAddress:public CPageFrame
{
public:
	CPageEditMacAddress(VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon = NULL,CPage * pParent = NULL);
	~CPageEditMacAddress();

	VD_PCSTR GetDefualtTitleCenter();
	void SetParentPage(CPage* page);
	void SetEditValue(const char* szText);
	VD_BOOL MsgProc( uint msg, uint wpa, uint lpa );
	
protected:


private:
	char m_username[15];
	char m_oldPasswd[15];
	
	void OnClkBtn();

	
	
	CStatic* pStatic[6];
	CEdit* pEdit[6];
	CButton* pButton[2];

	CPageNetwork* m_nPageNetwork;
};

#endif //_PAGE_EDIT_PWD_H_


