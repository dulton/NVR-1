#ifndef _PAGE_RECORDFRAMEWORK_H_
#define _PAGE_RECORDFRAMEWORK_H_

#include "PageFrame.h"
//#include "PageChild.h"
#include <vector>

#define RECCONFIG_SUBPAGES	6
#define RECCONFIG_BTNNUM	9

#define REC_ENABLE_ROWS 6
#define REC_ENABLE_COLS 3

#define REC_STREAM_ROWS 6
#define REC_STREAM_COLS 6

#define REC_TIME_ROWS 6
#define REC_TIME_COLS 4

#define REC_OSD_ROWS 6
#define REC_OSD_COLS 4

typedef enum
{
	EM_RESOLUTION_QCIF,
	EM_RESOLUTION_CIF,
	EM_RESOLUTION_D1,
}EmResolution;

typedef enum
{
	EM_ENCODE_VBR,
	EM_ENCODE_CBR,
}EmEncode;

typedef enum
{
	EM_QUALITY_WORST,
	EM_QUALITY_WORSE,
	EM_QUALITY_BAD,
	EM_QUALITY_NORMAL,
	EM_QUALITY_GOOD,
	EM_QUALITY_BETTER,
	EM_QUALITY_BEST,
	
}EmQuality;



typedef struct 
{
	int chn;
	EmResolution resolution;
	int frameRate;
	EmEncode encode;
	EmQuality quality;
	int bitrate;
}RecConfig;

class CPageOSDPosSet;
class CPageRecordFrameWork :public CPageFrame
{
public:
	CPageRecordFrameWork(VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon = NULL,CPage * pParent = NULL);
	~CPageRecordFrameWork();

	VD_PCSTR GetDefualtTitleCenter();
	VD_BOOL UpdateData( UDM mode );
	VD_BOOL MsgProc( uint msg, uint wpa, uint lpa );
	void CheckFrameLeft();
	int ReflushStaticString(int nIdex);

protected:
	void WriteLogs(int PageIndex);
	void AdjustEnableRows();
	void AdjustStreamRows();
	void AdjustTimeRows();
	void AdjustOsdRows();
	void AdjustAudioRows();

private:
	int nRemainCif;
	int curID;
	int chnNum;
	int totalItemNum;
	
	CComboBox* pTmpCombo;
	CCheckBox* pTmpCheck;
	
	//void OnClkConfigItem();
	void SwitchPage( int subID);
	void ShowSubPage(int subID, BOOL bShow);
	void LoadTabData(int tabID);
	void OnClickSubPage();

	CButton* pButton[RECCONFIG_BTNNUM+2];
	CTableBox* pTable;
	CStatic* pInfoBar;
	CStatic* pIconInfo;
	
	void SetInfo(char* szInfo);
	void ClearInfo();
	//CButton* pItemButton[3];

	//"启用"子页面
	void InitPage0();
	void OnTrackMove0();
	void OnVideoCheck0();
	void OnAudioCheck0();
	int m_page0;
	int m_maxPage0;
	int audio_page;

	int* bVideoEnable;
	int* bAudioEnable;
	CStatic* pStatic0[REC_ENABLE_COLS+REC_ENABLE_ROWS+1];
	CCheckBox* pCheckBox0[REC_ENABLE_ROWS*2+3];
	CTableBox* pTable00;
	CTableBox* pTable01;
	CScrollBar* pScroll0;
	
	//CTableBox* pTable01;
	
	//"录像码流"子页面
	void InitPage1();
	void OnTrackMove1();
	void OnComboSel10();
	void OnComboSel11();
	void OnComboSel12();
	void OnComboSel13();
	void OnComboSel14();
	void OnComboSel15();
	void OnComboSel16();
	void OnComboAllSel1();
	int m_page1;
	int m_maxPage1;
	int* resolution;
	int* framerate;
	int* encode;
	int* quality;
	int* qualityEnable;
	int* bitrate;
	CStatic* pStatic1[REC_STREAM_ROWS+REC_STREAM_COLS+2];
	CComboBox* pComboBox1[REC_STREAM_ROWS*5+5];
	CCheckBox* pCheckBox1;
	CTableBox* pTable10;
	CTableBox* pTable11;
	CScrollBar* pScroll1;
	
	
	//"录像时间"子页面
	void InitPage2();
	void OnComboSel20();
	void OnComboSel21();
	void OnComboSel22();
	void OnTrackMove2();
	int m_page2;
	int m_maxPage2;
	int* preRecTime;
	int* delayRecTime;
	int* recExpireTime;
	CStatic* pStatic2[REC_TIME_COLS+REC_TIME_ROWS+1];
	CComboBox* pComboBox2[REC_TIME_ROWS*3+REC_TIME_COLS];
	CCheckBox* pCheckBox2;
	CTableBox* pTable20;
	CTableBox* pTable21;
	CScrollBar* pScroll2;

	//"字符叠加"子页面
	void InitPage3();
	void OnCheckAll3();
	void OnTrackMove3();
	void OnChNameCheck3();
	void OnTimeCheck3();
	int m_page3;
	int m_maxPage3;
	int* bChNameEnable;
	int* bTimeEnable;
	//int* bSetButtomEnable;
	CStatic* pStatic3[REC_OSD_COLS+REC_OSD_ROWS+1];
	CCheckBox* pCheckBox3[REC_OSD_ROWS*2+3];
	CButton* pButton3[REC_OSD_ROWS+1];
	CTableBox* pTable30;
	CTableBox* pTable31;
	CScrollBar* pScroll3;

	void OnSetup();

	//"循环录像"子页面
	void InitPage4();
	CStatic* pStatic4[1];
	CCheckBox* pCheckBox4[1];
	CTableBox* pTable40;

	//"抓图"子页面
	void InitPage5();
	CStatic* pStatic5[7];

	CComboBox* pComboBox50;//通道选择
	//;定时抓图 使能启用、分辨率 、图片质量 、时间间隔
	//;事件抓图 使能启用、分辨率 、图片质量 、时间间隔
	CCheckBox* pCheckBox5[2];
	CComboBox* pComboBox5[2][3];
	CTableBox* pTable50;
	SGuiSnapChnPara *P5SnapParam;

	void OnCheckBox5();
	void OnComboBox_snap_chn_change();

	
	//
	CPageOSDPosSet* m_pOSDSet;

	void OnCheckBoxSelectAll();
	void OnCheckBoxSelectAll2();
	void LoadButtomCtl();

	unsigned char m_IsLoader;
};

#endif //_PAGE_RECORDFRAMEWORK_H_


