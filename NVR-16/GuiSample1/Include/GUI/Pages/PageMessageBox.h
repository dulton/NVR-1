
#ifndef __PAGE_MESSAGE_BOX_H__
#define __PAGE_MESSAGE_BOX_H__

#include "PageFrame.h"

#ifndef WIN32		
#define MB_NULL              		0x00000001L
#define MB_OK                 		0x00000002L
#define MB_OKCANCEL              	0x00000004L

#define MB_ICONERROR                0x00000010L
#define MB_ICONQUESTION             0x00000020L
#define MB_ICONWARNING				0x00000040L
#define MB_ICONINFORMATION          0x00000080L

#define MB_DEFBUTTON1               0x00000000L
#define MB_DEFBUTTON2               0x00000100L
#endif


class CPageMessageBox: public CPageFrame
{
public:
	CPageMessageBox(VD_PCRECT pRect,VD_PCSTR psz = NULL,VD_BITMAP* icon = NULL,CPage * pParent = NULL, int bModal = FALSE);
	~CPageMessageBox();
	VD_BOOL UpdateData(UDM mode);
	UDM Run(VD_PCSTR psz, int vtype);
	VD_BOOL MsgProc( uint msg, uint wpa, uint lpa );

	void SetType(int vtype);
	
private:
	CStatic    *pStaticText;
	CStatic    *pStaticIcon;
	CButton    *pOK;
	CButton    *pCancel;
	CButton    *pOK1;
	int		type;
	int m_bModal;
	int m_delay;
};

UDM	GetRunStatue();
UDM MessageBox(VD_PCSTR pText, VD_PCSTR pCaption, int nType, int bModal = FALSE);

#endif //__PAGE_MESSAGE_BOX_H__
