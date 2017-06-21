#ifndef _PAGE_IPCAMERACFGFRAMEWORK_H_
#define _PAGE_IPCAMERACFGFRAMEWORK_H_

#include "PageFrame.h"

#include <vector>

#define IPCAMERACFG_SUBPAGES	2
#define IPCAMERACFG_BTNNUM	    5

class CPageInfoBox;
class CPageIPCameraModifyFrameWork;
class CPageIPCameraSetChnFrameWork;

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
	
	void GetChannelList();
	
protected:
	
private:
	void InitPage0();
	
	void SwitchPage( int subID);
	void ShowSubPage(int subID, BOOL bShow);
	
	void OnClickSubPage();
	
	void OnCheckSel();
	
	void OnTrackMove0();
	void OnTrackMove1();
	
	VD_BOOL AddIPC(ipc_unit &camera);
	VD_BOOL AddIPC_nvr(int firstChn, ipc_unit &camera);
	
	CTableBox *pTabFrame0[4];
	CTableBox *pTable0[4];
	
	CStatic *pStatic0[128];
	CButton *pButton0[32];
	CCheckBox *pCheckBox0[32];
	CComboBox *pCombox0[16];
	CIPAddressCtrl *pIpCtrl0[16];
	CEdit *pEdit0[16];
	CScrollBar *pScrollbar0[4];
	
	CRect m_rtTable1;
	CRect m_rtTable2;
	
	int m_cols;
	int m_rows;
	int m_offset;
	
	int m_cols2;
	int m_rows2;
	int m_offset2;
	
	ipc_node *m_ipc_head;
	int m_nPageIndex;
	int m_nCurMouseLine;
	
	ipc_node *m_channel_list;
	int m_nPageIndex2;
	int m_nCurMouseLine2;
	//yaogang modify 20160304
	u8 nNVROrDecoder;
	
	CPageIPCameraModifyFrameWork *m_pPageIPCModify;
	CPageIPCameraSetChnFrameWork *m_pPageIPCSetChn;
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
	CIPAddressCtrl *pIpCtrl0[8];
	CButton *pButton0[8];
	CEdit *pEdit0[8];//csp modify 20140411
	
	CStatic *pStatus;
	
	ipc_unit ipcam;
	
};

class CPageIPCameraSetChnFrameWork :public CPageFrame
{
public:
	CPageIPCameraSetChnFrameWork(VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon = NULL,CPage * pParent = NULL);
	~CPageIPCameraSetChnFrameWork();
	
	VD_PCSTR GetDefualtTitleCenter();
	VD_BOOL UpdateData( UDM mode );
	
	VD_BOOL MsgProc( uint msg, uint wpa, uint lpa );
	
	VD_BOOL SetChn(int chn, unsigned char add);
	
private:
	void InitPage0();
	
	void OnClickSubPage();
	
	void OnComboBox0();
	void OnComboBox1();
	
	CTableBox* pTabFrame0[3];
	CTableBox* pTable0[3];
	
	CStatic *pStatic0[20];
	CComboBox* pCombox0[16];
	CCheckBox* pCheckBox0[16];
	CIPAddressCtrl* pIpCtrl0[16];
	CEdit* pEdit0[16];
	CButton *pButton0[16];
	
	int m_chn;
	unsigned char m_add_flag;
	//yaogang modify 20160304
	u8 nNVROrDecoder;
};

#endif //_PAGE_IPCAMERACFGFRAMEWORK_H_

