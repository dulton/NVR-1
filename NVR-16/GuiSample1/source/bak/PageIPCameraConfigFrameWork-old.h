#ifndef _PAGE_IPCAMERACFGFRAMEWORK_H_
#define _PAGE_IPCAMERACFGFRAMEWORK_H_

#include "PageFrame.h"

#include <vector>

#define IPCAMERACFG_SUBPAGES	2
#define IPCAMERACFG_BTNNUM	    5

class CPageInfoBox;
class CPageIPCameraModifyFrameWork;
class CPageIPCameraConfigFrameWork :public CPageFrame
{
public:
	CPageIPCameraConfigFrameWork(VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon = NULL,CPage * pParent = NULL);
	~CPageIPCameraConfigFrameWork();
	
	VD_PCSTR GetDefualtTitleCenter();
	VD_BOOL UpdateData( UDM mode );
	VD_BOOL MsgProc( uint msg, uint wpa, uint lpa );
	VD_BOOL Open();
	
	void MouseMoveToLine(int index);
	
	void UpdateIPC(ipc_unit *pCam);
	
protected:
	
private:
	void SwitchPage( int subID);
	void ShowSubPage(int subID, BOOL bShow);
	
	void OnClickSubPage();
	
	void OnComboBox0();
	
	void OnTrackMove0();
	
	void InitPage0();
	
	CTableBox* pTabFrame0[3];
	CTableBox* pTable0[3];
	
	CStatic *pStatic0[64];
	CComboBox* pCombox0[16];
	CCheckBox* pCheckBox0[16];
	CIPAddressCtrl* pIpCtrl0[16];
	CEdit* pEdit0[16];
	CButton *pButton0[16];
	CScrollBar *pScrollbar0[3];
	
	CRect m_rtTable1;
	int m_nCurMouseLine;
	
	int m_cols;
	int m_rows;
	int m_offset;
	
	ipc_node *m_ipc_head;
	
	int m_nPageIndex;
	
	CPageIPCameraModifyFrameWork* m_pPageIPCModify;
};

class CPageIPCameraModifyFrameWork :public CPageFrame
{
public:
	CPageIPCameraModifyFrameWork(VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon = NULL,CPage * pParent = NULL);
	~CPageIPCameraModifyFrameWork();
	
	VD_PCSTR GetDefualtTitleCenter();
	VD_BOOL UpdateData( UDM mode );
	
	VD_BOOL MsgProc( uint msg, uint wpa, uint lpa );
	
	void SetIPC(ipc_unit *pCam);
	void GetIPC(ipc_unit *pCam);
	
private:
	void InitPage0();
	
	void OnClickSubPage();
	
	CStatic *pStatic0[8];
	CIPAddressCtrl* pIpCtrl0[8];
	CButton *pButton0[8];
	
	CStatic *pStatus;
	
	ipc_unit ipcam;
};

#endif //_PAGE_IPCAMERACFGFRAMEWORK_H_

