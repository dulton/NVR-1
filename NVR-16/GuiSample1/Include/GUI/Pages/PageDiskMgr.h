#ifndef _PAGE_DISKMGR_H_
#define _PAGE_DISKMGR_H_

#include "PageFrame.h"
#include "PageSysInfo.h"

struct DSKINFO  
{
	std::string	 index;
	std::string  type; 
	std::string  capacity;
	std::string  freespace;
	std::string  state;
	int			 attrib;
	std::string  source;
};

#define MAX_DSKMGR_TITLE 7

struct DSKITEM
{
	CStatic *pStatic[MAX_DSKMGR_TITLE];
	CComboBox *pCombo;
};

#include <vector>

class CPageDiskMgr:public CPageFrame
{
public:
	CPageDiskMgr(VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon = NULL,CPage * pParent = NULL);
	~CPageDiskMgr();
	
	VD_BOOL UpdateData( UDM mode );

	int SetFormatProg(int nProg);
	int SetStateInfo(char* info);

	VD_BOOL MsgProc( uint msg, uint wpa, uint lpa );

	void Format();
	void SetInfoProg(u8 nType, char* pSrc, u32 nTotal, u32 nFree);
	void RemoteFormatStart(uchar nIdx, int nRslt);
	void NotifyDiskChanged(void *param);
private:
	void SelectDskItemEx( int index );
	int nTabCol;
	void SetLineInfIns(DSKINFO* pInfo, uchar type, uint total, uint free, u8 status, char* pDevPath);
	void SetDskInfoLine(int idx, uchar type, uint total, uint free, char* pDevPath);
	char bQuit;
	uchar nDiskNum;
	//char* pDskInfoBuf; //zlb20111117  È¥µô²¿·Ömalloc
	CTableBox* pTab;
	CStatic* pDskMgrTitle[MAX_DSKMGR_TITLE];

	int m_count;
	CRect m_screenRT;

	CButton* pButton[5+2];
	void OnClkButton();

	CStatic* percentInfo;
	CStatic* formatProg;
	CStatic* stateInfo;

	DSKITEM m_DskItems[9];

	std::vector<DSKINFO> m_dskdatalist;
	CPageSysInfo* m_pDskInfo;

	void UpdateDiskData();

	void SelectDskItem(int index);
	int m_nCursel;
	void LoadButtomCtl();

	unsigned char m_IsLoader;
};

#endif
