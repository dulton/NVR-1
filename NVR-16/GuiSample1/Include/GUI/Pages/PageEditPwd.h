#ifndef _PAGE_EDIT_PWD_H_
#define _PAGE_EDIT_PWD_H_

#include "PageFrame.h"
//#include "PageChild.h"
#include <vector>

class CPageEditPwd:public CPageFrame
{
public:
	CPageEditPwd(VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon = NULL,CPage * pParent = NULL);
	~CPageEditPwd();

	VD_PCSTR GetDefualtTitleCenter();
	void SetUser(char* username);
	void SetOldPasswd(char* passwd);
	VD_BOOL MsgProc( uint msg, uint wpa, uint lpa );
	VD_BOOL Close(UDM mode);
	
protected:


private:
	char m_username[15];
	char m_oldPasswd[15];
	
	void OnClkBtn();

	
	
	CStatic* pStatic[3];
	CEdit* pEdit[3];
	CButton* pButton[2];

	CPageUserManager* m_ppageusermanager;

};

#endif //_PAGE_EDIT_PWD_H_





