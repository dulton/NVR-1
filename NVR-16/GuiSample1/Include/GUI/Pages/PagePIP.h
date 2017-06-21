#ifndef _PAGE_PIP_H_
#define _PAGE_PIP_H_

#include "PageFloat.h"
#include "GUI/Pages/PageMessageBox.h"

class CPagePIP:public CPageFloat
{
public:
	CPagePIP(VD_PCRECT pRect,VD_PCSTR psz = NULL,VD_BITMAP* icon = NULL,CPage * pParent = NULL);
	~CPagePIP();
	
	VD_BOOL UpdateData( UDM mode );
	VD_BOOL MsgProc( uint msg, uint wpa, uint lpa );
	
private:
	void OnClkButton();
	void StartPIP();
	
	int m_nChMax;
	
	CStatic* pStatic[2];
	CComboBox* pComboBox[2];
	CButton* pButton[2];
};

#endif

