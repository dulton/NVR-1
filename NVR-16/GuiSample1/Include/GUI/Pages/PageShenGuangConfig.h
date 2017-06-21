#ifndef _PAGE_SHENGUANG_CONFIG_H_
#define _PAGE_SHENGUANG_CONFIG_H_//shixin


#include "PageFrame.h"

#include <vector>
#include "common_basetypes.h"
//#include "GUI/Pages/PageAlarmPictureConfig.h"


#define SHENGUANGCFG_SUBPAGES	5
#define SHENGUANGCFG_BTNNUM	8

//#define PICTURECONFIG_BTNNUM	2

//class CPageAlarmPictureConfig;//shixin

class CPageAlarmPictureConfig :public CPageFrame
{

public:
	CPageAlarmPictureConfig(VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon = NULL,CPage * pParent = NULL);
	~CPageAlarmPictureConfig();
	VD_PCSTR GetDefualtTitleCenter();
	
	void ShowSubPage(int mainID,int subID, BOOL bShow);
	void OnCombox3();
	void OnCheckBox();
	void OnEditChange3();
	VD_BOOL UpdateData( UDM mode );
private:
	u8 m_SensorNum;	//系统中本地报警数量
	u8 m_IpcNum;	//系统中IPC数量
	CButton* pButton[4];
	CStatic* 		pStatus;
	void OnClickBtn();
	/////
	void InitPageAlarmPictureConfig();
	CStatic* pStatic3[6];
	//CCheckBox* pCheckBox3[2];
	CStatic* pStaticChn[16];
	CCheckBox* pCheckBoxAll;
	CCheckBox* pCheckBoxChn[16];
	CComboBox* pComboBox3[2];
	CEdit* pEdit3[3];
	//CButton* pButton3;
	//CButton* pButton4;
	CTableBox* pTable3;
	
	
};

class CPageDailyPictureConfig :public CPageFrame
{

public:
	CPageDailyPictureConfig(VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon = NULL,CPage * pParent = NULL);
	~CPageDailyPictureConfig();
	VD_PCSTR GetDefualtTitleCenter();
	void OnCombox3();
	void OnCheckBox();
	void OnEditChange3();
	VD_BOOL UpdateData( UDM mode );
private:
	CButton* pButton[4];
	
	CTableBox* pTable3;
	
	CStatic* pStatus;
	CStatic* pStatic3[6];
	CStatic* pStatic1[16];
	CStatic* pStatic2[16];
	
	CCheckBox* pCheckBoxTime[2];
	CCheckBox* pCheckBoxChnAll[2];
	CCheckBox* pCheckBoxChn[2][16];
	
	CDateTimeCtrl* pDateTime[2];
	
	CEdit* pEdit3[3];
	
	void OnClickBtn();
	void InitPageDailyPictureConfig();
	void OnDateTimeChange1();

};



class CPageShenGuangConfig:public CPageFrame
{
public:
	CPageShenGuangConfig(VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon = NULL,CPage * pParent = NULL);
	~CPageShenGuangConfig();
	
	VD_PCSTR GetDefualtTitleCenter();

	void SwitchPage( int mainID,int subID);
	void ShowSubPage(int mainID,int subID, BOOL bShow);

	VD_BOOL UpdateData( UDM mode );
	void			AdjHide();


	void OnCombox3();
	void OnCheckBox();
	void RecvNotify(u8 chn, u8 ErrVal);
	

private:
	int             curID;
	int             nRealRow;
	int             nPageNum;
	int             nHideRow;
	int             nCurPage;


	CButton* pButton[SHENGUANGCFG_BTNNUM+2];
	CStatic* 		pStatus;
	CStatic*		pSSGUpload_result;
	void OnClickSubPage();
	void ShowRegistButton( void );

	//yaogang modify 20150316
	//图片上传结果显示
	VD_BITMAP* pBmpSuccess;
	VD_BITMAP* pBmpFailure;
	
	/////用户报修页面

	void InitPageUserRepair();
	CComboBox* pComboBox0[2];
	
	void OnEditChange3();
	CStatic* pStatic3[6];
	CStatic* pStaticChn[16];
	CCheckBox* pCheckBox3;
	CComboBox* pComboBox3[2];
	CEdit* pEdit3[3];
	
	CTableBox* pTable3;
	//void LoadButtomCtl();
	
	unsigned char m_IsLoader;

	////维护维修

	void InitPageMaintain();
	CComboBox* pComboBox1[2];

	////测试保养
	void InitPageTest();
	CComboBox* pComboBox2[1];
	CCheckBox* pCheckBox2All;
	CCheckBox* pCheckBox2Chn[16];
	
	//yaogang modify 20150316
	CStatic* pUploadRet1[16];//图片上传结果显示
	
	////验收管理
	void InitPageAcceptManage();
	CCheckBox* pCheckBox3All;
	CCheckBox* pCheckBox3Chn[16];
	
	//yaogang modify 20150316
	CStatic* pUploadRet2[16];//图片上传结果显示
	
	////基本设置
	void  InitPageBasicSetup();
	CEdit* pEdit4[2];
	CCheckBox* pCheckBox4[3];
	CButton* pButton4[2];

	//CStatic* pStatic[PICTURECONFIG_BTNNUM];
	//CButton* pButtonPictureCfg[2];
	//CStatic* pStatic[PICTURECONFIG_BTNNUM];
	CPageAlarmPictureConfig* m_pPageAlarmPictureCfg;
	
	CPageDailyPictureConfig* m_pPageDailyPictureCfg;
	

};





#endif

