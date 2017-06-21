#ifndef _PAGE_MENUE_OUTPUT_DEV_H_
#define _PAGE_MENUE_OUTPUT_DEV_H_

#include "PageFrame.h"
//#include "PageChild.h"
#include <vector>

class CPageMenueOutputDev:public CPageFrame
{
public:
	CPageMenueOutputDev(VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon = NULL,CPage * pParent = NULL);
	~CPageMenueOutputDev();
	
	VD_BOOL UpdateData( UDM mode );
	
	CStatic* pStatic[4];
	unsigned char IsClosed;
	
	VD_BOOL MsgProc(uint msg, uint wpa, uint lpa);
private:
	
	void OnClkBtn();
	
	CButton* pButton[1];
	CComboBox* pCombox;
	//CStatic* pInfo;
};

#endif //_PAGE_MENUE_OUTPUT_DEV_H_

