#ifndef _PAGE_SYS_INFO_H_
#define _PAGE_SYS_INFO_H_

#include "PageFrame.h"

#define MAX_SYS_INFO 12

typedef enum{SIT_SYS, SIT_DSK}EMSYSINFOTYPE;

class CPageSysInfo:public CPageFrame
{
public:
	CPageSysInfo(VD_PCRECT pRect,VD_PCSTR psz,EMSYSINFOTYPE type, VD_BITMAP* icon = NULL,CPage * pParent = NULL);
	~CPageSysInfo();
	
	void SetSysInfo(int row, char* szText);
	
	VD_BOOL UpdateData( UDM mode );
	void SetDiskIdx(uchar nIdx);
private:
	uchar m_nDiskIdx;
	CTableBox* pFrame;
	CTableBox* pTab;
	CStatic* pSysInfo[2*MAX_SYS_INFO];
	
	EMSYSINFOTYPE m_emSysType;
};
#endif

