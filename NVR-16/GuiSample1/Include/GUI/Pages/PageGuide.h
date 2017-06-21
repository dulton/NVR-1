#ifndef _PAGE_GUIDE_H_
#define _PAGE_GUIDE_H_

#include "PageFrame.h"

#define MAX_GUIDE_INFO 12

//class CPageEditMacAddress;
class CPageGuide:public CPageFrame
{
public:
	CPageGuide(VD_PCRECT pRect,VD_PCSTR psz, VD_BITMAP* icon = NULL,CPage * pParent = NULL);
	~CPageGuide();

	void SetValue(int row, const char* szText);
	VD_BOOL UpdateData( UDM mode );
	//VD_BOOL UpdateMacAddress(char *pBuf);
	VD_BOOL MsgProc( uint msg, uint wpa, uint lpa );
	void OnClkBtn();

private:
	u8 m_count0;
	u8 m_count1;
	CTableBox* pFrame;
	CTableBox* pTab;
	CStatic* pTitle[MAX_GUIDE_INFO];
	CStatic* pValue[MAX_GUIDE_INFO];
	CButton* pButton[3];
	//CPageEditMacAddress* m_pPageEditMacAddress;
};
#endif

