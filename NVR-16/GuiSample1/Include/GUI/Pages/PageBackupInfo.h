#ifndef _PAGE_BACKINFO_H_
#define _PAGE_BACKINFO_H_

#include "PageFrame.h"
#include "BizData.h"

#include <vector>

typedef enum
{
	EM_BACKUP_REC,
	EM_BACKUP_SNAP,
}EM_BACKUP_MODE;


class CPageBackupInfo:public CPageFrame
{
public:
	CPageBackupInfo(VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon = NULL,CPage * pParent = NULL);
	~CPageBackupInfo();
	
	VD_BOOL UpdateData( UDM mode );
	VD_BOOL MsgProc( uint msg, uint wpa, uint lpa );

	int SetBakProg(int nProg);
	void SetStartTime( char* szStartTime);
	void SetEndTime( char* szEndTime);
	void SetFileNum( int num);
	void SetTotalSize( u32 totalSize);
	void SetFreeSize( u32 freeSize);
	void SetFileSize ( u32 filesizeM, u32 filesizeK );//cw_backup
	void SetBackupTitle(EM_BACKUPTYPE type) ;
	void SetBackupInfo(EMBIZFILETYPE fileType, SBizBackTgtAtt* attribute, SBizSearchCondition* condition);
	void SetUdskInfo( u32 totalSize, u32 freeSize );
	void WaitCheckFree(int times);
	void CancelBackup(void);
	void SetSnapMode();
	void SetRecMode();

private:
	uchar bChecking; // 为退出备份和进入备份之间检测USB设备剩余空间状态
	uchar nBackProg;
	uchar bVisible;
	uchar nUdskNum;
	void GetUdevStoreInfo();
	BOOL bStartBackup;

	CTableBox* pTab;
	CRect m_screenRT;

	CButton* pCtlBtn[3];
	CStatic* pBakItem[8];
	CStatic* pBakVal[6];//cw_backup 5
	CComboBox* pCombo[2];
	//yaogang modify for snap backup
	CStatic* pTextFileType;
	CCheckBox* pChk;

	CStatic* bakProg;
	CStatic* percentInfo;

	EMBIZFILETYPE m_fileType;
	SBizBackTgtAtt* m_backupAttr;
	SBizSearchCondition* m_backupCondition;
	EM_BACKUP_MODE 	m_WorkMode;

	void OnClkButton();

};

#endif

