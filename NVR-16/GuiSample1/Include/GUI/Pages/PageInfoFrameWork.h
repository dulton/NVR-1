#ifndef _PAGE_INFO_FRAMEWORK_H_
#define _PAGE_INFO_FRAMEWORK_H_

#include "PageFrame.h"

/*modify by liu*/
#define MAX_INFO_PAGES 5
//#define MAX_INFO_PAGES 4//5 //6	

class CPageSysInfo;
class CPageEventList;
class CPageNetwork;
class CPageOnlineUser;
class CPageSysLog;
//add by liu
class CPageStream;
class CPageInfoFrameWork :public CPageFrame
{
public:
	CPageInfoFrameWork(VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon = NULL,CPage * pParent = NULL);
	~CPageInfoFrameWork();
	
	VD_BOOL UpdateData( UDM mode );
	void ReflushItemName();

private:
	void OnClkItem();

	CStatic* pStatic[MAX_INFO_PAGES];
	CButton* pButton[MAX_INFO_PAGES];
	CStatic* pStaticPic;

	CPageSysInfo* m_pPageSysInfo;
	CPageEventList* m_pPageEventList;
	CPageNetwork* m_pPageNetwork;
	CPageOnlineUser* m_pPageOnline;
	CPageSysLog* m_pPageSysLog;
	//add by liu
	CPageStream* m_pPageStream;
};

#endif

