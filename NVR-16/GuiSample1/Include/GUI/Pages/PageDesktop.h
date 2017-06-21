#ifndef __PAGE_DESKTOP_H__
#define __PAGE_DESKTOP_H__


#include "GUI/Ctrls/Page.h"


//#include "System/AppEvent.h"
#include "PageFrame.h"


#include <vector>

//add by liu
#include "MultiTask/Timer.h"

typedef enum 
{
	EM_SPLIT_H,  //横
	EM_SPLIT_V,  //竖
}EMSPLITLINETYPE;


typedef enum 
{
	EM_CHNSTATE_HIDE, 
	EM_CHNSTATE_1,	
	EM_CHNSTATE_2,			 
}EMCHNSTATESHOW;

typedef enum
{
	EM_STATE_RECORD, //录像状态
	EM_STATE_MD, //移动侦测
	EM_STATE_ALARM, //报警录像
	EM_STATE_TIMER, //定时录像
	EM_STATE_MANUAL_REC, //手动录像
}EM_STATE_TYPE;

//add by liu
typedef enum
{
	EM_STATE_SNAP,	//抓图图标
	EM_STATE_LINK, //联接状态
	EM_STATE_MKPHONE, //麦克风状态
	EM_STATE_SOUND, //声音状态	
}EM_ELSESTATE_TYPE;


typedef struct
{
	int x;
	int y;
	int x2;
	int y2;

} ChnRegion;

class CPageMainFrameWork;
class CPageSplitFrameWork;
class CPagePlayBackFrameWork;
class CPageStartFrameWork;
class CPageInputTray;
class CPageLogin;
class CPageAlarmList;

class CPageDesktop: public CPage
{
public:
	CPageDesktop(VD_PCRECT pRect,VD_PCSTR psz = NULL,VD_BITMAP* icon = NULL,CPage * pParent = NULL);
	~CPageDesktop();
	
	/*事件处理*/
	//void onAppEvent(appEventCode code, int index, appEventAction action, EVENT_HANDLER *param, const CConfigTable* data = NULL);
	
	VD_BOOL UpdateData(UDM mode);                     //页面刷新    
	VD_BOOL MsgProc(uint msg, uint wpa, uint lpa);  //消息处理

	static void SetLogin(int login);
	void SetCurPreviewMode(EMBIZPREVIEWMODE emMod);//获得当前的预览模式	
	void SetCurPreModePage(u32 nPage);
	u32 ClikCurChn(EMBIZPREVIEWMODE emMod, u32 nPage, int x, int y);//返回当前点击的通道
	void SetPageFlag(BOOL bFlag);
	void SetIsDwellFlag(BOOL bFlag);

	int StartZoomVideoArea();
	int ChangZoomArea();
	int DrawZoomArea();
	void ExitFromZoomStatue();
	int ExitFromDrawStatue();
	
	void OnRec();
	
	//主菜单回调函数
#if 0//csp modify
	void OnMenu();
	void On1x1();
	void OnTriangle();
	void On2x2();
	void OnDwellNo();
	void OnZoom();
	void OnAudio();
	void OnPtz();
	void OnColor();
	void OnSnap();
	void OnPlay();
	void OnEdit();
#endif
	
#if 0//csp modify
	void OnLogoShow(int _spt,int _sub);
	void OnTvAjust();//TV调节设置
	void OnMatrix();/*!< 视频矩阵 */
	void OnPtzCtrl();//云台控制
	void OnImgConf();//图像设置
	void OnRecord();//手动录像控制
	void OnQuickSch();//回放查询
	void OnPlayback();
	void OnBackupOnline();//实时刻录
	void OnCtrlAlmIn();//报警输入
	void OnCtrlAlmOut();//报警输出
	
	void OnMain();//主菜单
	void OnLogout();//注销
	void OnBackUp(); 
	void OnSystemSilence();//系统静音added by wyf on 090922
	void OnShutDown();
	//void OnShutDown(uint param);
	void OnLocksys();
	
	void OnMenuSplit();
	void OnSetSpt();//通道切换
	
	//子菜单回调函数
	void OnSetSpt1Chan();//切换单画面
	void OnSetSpt2Chan();//切换2画面
	void OnSetSpt4Chan();
	void OnSetSpt6Chan(void);//切换6画面
	void OnSetSpt8Chan();
	void OnSetSpt9Chan();
	void OnSetPIPChan();
	
#ifdef FUNC_QUICK_PLAY
	void OnQuickPlay(int ch, int RecType, int BackTime);
	void OnQuickPlayDefault();
	void GetQuickPlayMenuCurSel();
#endif
	
	//定时刷新桌面回调函数
	void OnDelay(uint arg);//打开主页面延时处理
	void runLButtonDown(uint lpa);//add langzi 2009-11-9
	void OnTimer(uint arg);//定时刷新标题和时间回调函数
	//add by yanjun for config data updata
	void UpdataDisplayConfig();
	void OnSwitchVideoOutput(uint param);
	void OnConfigChanged();
	void OnSplitChange(VD_BOOL post);
	void OnTourStateChanged(VD_BOOL started, VD_BOOL locked);
#endif
	
	//配置结构更新时的回调函数
	//void OnConfigMonitorTour(CConfigMonitorTour& config, int& ret);
	//void OnConfigGUISet(CConfigGUISet& config, int& ret);
	//void OnConfigTVAdjust(CConfigTVAdjust& config, int& ret);
	//void OnConfigChannelTitle(CConfigChannelTitle& config, int& ret);
	
private:
	void Draw();
	VD_BOOL AdjustRect();
	
#if 0//csp modify
	void UpdateConfig();
	void SetChannelTitle(VD_BOOL bFlag);
	void SetChannelLogo(VD_BOOL bFlag, VD_BOOL bForced = FALSE);
	void SetChannelState(VD_BOOL recordStatus, VD_BOOL alarmStatus, VD_BOOL bForced = FALSE);
	void SetTimeTitle(VD_BOOL bFlag, VD_BOOL bForced = FALSE);
	//bForced: TRUE 时屏幕可能发生了缩放，需要重新调整坐标
	void SetBitRateInfo(VD_BOOL bFlag, VD_BOOL bForced = FALSE);
	void SetCardInfo(VD_BOOL value); 
	void SetNextSplit();
	VD_BOOL SetSplitSub(int *spt, int *sub);
	void SetSilenceMenu();//界面右键菜单设置为"静音"字样added by wyf on 090925
#endif
	
private:
	//yaogang modify 20150324
	u8 nNVROrDecoder;//跃天: 1 nvr，2 轮巡解码器，3 切换解码器

	CMenu *pMainMenu;
	EMBIZPREVIEWMODE emCurPreviewMode;//当前的窗口预览模式
	EMBIZPREVIEWMODE eMaxViewMode;//当前设置的最大通道数
	EMBIZPREVIEWMODE emLastPreMode;//上一次的预览模式
	u32 nPreviewPage;
	u32 RowNum;//当前模式下有多少行
	u32 ColNum;//当前模式下有多少列
	int nScreanWidth;
	int nScreanHeight;
	int maxChn;
	int px;
	int py;
	CStatic *pAlarmIcon;
	
	CMenu*  pSubMenu[N_SPLIT];
	CMenu*	pPOPMenu; 
	BOOL	bThisPageFlag;
	BOOL	IsDwell;

	BOOL 	m_IsPbElecZoomDrawStatus;
	BOOL 	m_IsElecZoomDrawStatus;
	BOOL 	IsDrawElecZoomArea;
	BOOL 	IsElecZoomBegin;
	BOOL	IsElecZoomStatue;
	BOOL	IsLbuttonDown;
	int 	mouse_eleczoom_x;
	int 	mouse_eleczoom_y;
	int 	start_x;
	int 	start_y;
	int 	end_x;
	int 	end_y;
	int 	zoom_w;
	int 	zoom_h;
	int 	iszoom_x1;
	int 	iszoom_y1;
	int 	iszoom_x2;
	int 	iszoom_y2;
	int 	Is3520;
	int 	Is9616S;
	int 	m_CurChn;
	
	int		CurScreenHeight;//电子放大标准尺寸 高
	int		CurScreenWidth;//电子放大标准尺寸  宽
	
	SBizPreviewElecZoom stCapRect;
	SBizPreviewElecZoom stPbCapRect;
	SBizSystemPara sysPara;
	
	int		mouse_spt_x;
	int		mouse_spt_y;

	//add by liu
	CTimer *m_DeskStreamTimer;

//	CTimer	m_PageMainTimer;//打开主页面定时器
	
// 	CTimer	m_UpdateTimer;//时间标题和通道标题刷新定时器
// 	CTimer	m_ShutTimer;	//关机确认定时器
// 	CTimer	m_SwitchTimer;//切换视频输出类型定时器 
// 	int		m_ShutCountDown;
	
//	CDevSplit *m_pDevSplit;
// 	CDisplay *m_pDisplay;
// 	CConfigMonitorTour m_ConfigMonitorTour;
// 	CConfigGUISet m_ConfigGUISet;
// 	CConfigTVAdjust m_ConfigTVAdjust;
// 	CConfigChannelTitle m_ConfigChannelTitle;

//	CPageCountDown *pPageCountDown;

//	VD_BITMAP * m_pBMPChannelStates[ALLCHSTATES];

//	CDevVideo *m_pDevVideo;

// 	CStatic* m_pBpsChannel[2];
// 	CStatic* m_pBpsUnit[2];
// 	CStatic* m_ChannelNo[8];
// 	CStatic* m_pStaticShowBps[8];   
// 	int  m_count;  //显示码流时控制码流的刷新次数

// 	//在桌面上显示ATM卡号叠加等情况
// 	CStatic* m_pStaticCardName[N_DESKTOP_INFO_LINE];  //要叠加的项目名称,如交易金额
// 	CStatic* m_pStaticCardContent[N_DESKTOP_INFO_LINE]; //每一项对应的具体数字,如具体交易金额的大小.
// 
// 	char m_CardName[N_DESKTOP_INFO_LINE * N_SYS_CH][DESKTOP_INFO_MAX_LEN]; //存放叠加的项目名称,每通道对应8组, 如1通道对应m_CardName[0][32]-m_CardName[7][32]
// 	char m_CardContent[N_DESKTOP_INFO_LINE * N_SYS_CH][DESKTOP_INFO_MAX_LEN];
// 
// 	uint m_ShowCardInfo[N_SYS_CH];		/*!< 表示当前是不是正在叠加编码信息在屏幕上 */

// 	CButton* m_pButtonTour;
// 	VD_RECT	m_rectTour;// 轮巡按钮矩形区域
// 	VD_BITMAP * bmp_tour_enable;
// 	VD_BITMAP * bmp_tour_disable;
// 	CMutex	m_MutexState;//保护页面状态

// 	VD_BITMAP * bmp_logo_middle;
// 	VD_BITMAP * bmp_logo_blank;//空闲区域的LOGO图片add by nike.xie 09/5/21
 	
// 	VD_COLORREF m_bkColor;
	
	VD_COLORREF m_keyColor;
	VD_RECT m_RectScreen;
	
	CPageInputTray* m_pPageInputTray;
	CPageStartFrameWork* m_pPageStart;

	CPageAlarmList *m_alarmList;
	
	static int isLogin;
	CPageLogin* m_pPageLogin;
 	CPageMainFrameWork* m_pPageMain;
	CPageSplitFrameWork* m_pPageSplit;
//	CPagePlayBackFrameWork* m_pPagePlayBack;//csp modify 20130509
	
// 	CPageFunction* m_pPageFunction;
// 	CPageInputTray* m_pPageInputTray;
	
// 	SYSTEM_TIME m_OldSt;
// 	int m_iTimeFmt;
// 	int m_bTitleFlag;
// 	VD_BITMAP * bmp_systemsilence[2];   //静音指针  add langzi 2009-11-9
// 	VD_BITMAP * bmp_systemnosilence[2];  //非静音指针  add langzi 2009-11-9	
	
public:
	std::vector<CStatic*> m_vSplitLine;
	std::vector<CStatic*> m_vElecZoomSplitLine;
	std::vector<CStatic*> m_vElecZoomAreaSplitLine;
	std::vector<CStatic*> m_vCoverRect;
	int m_maxCH;
	int m_maxRect;
	int* m_curCover;

	BOOL m_bAlarmListStatus;
	void ToggleAlarmIcon(bool isVisible);
	
	//csp modify
	//void InitSplitLine(int maxlines = 16, VD_COLORREF color = VD_RGB(255,255,255));
	void InitSplitLine(int maxlines = 16, VD_COLORREF color = VD_RGB(232,232,232));
	
	void SetSplitLineRect(int index, int x, int y, int length, EMSPLITLINETYPE type);
	void ShowSplitLine(int index, BOOL bShow);
	void SetCurVideoSize();//改变电子放大标准尺寸
	
	std::vector<CStatic*>m_vChannelName;
	void InitChnName(int maxName = 4);
	void SetChnName(int index, VD_PCRECT pRect, VD_PCSTR psz);
	void EditChnName( int index,  VD_PCSTR psz );
	void EditChnWidth( int index ,int width);
	void ShowChnName(int index, BOOL bShow);

	/*add by liu*/
	std::vector<CStatic*>m_vChannelKbps;
	void InitChnKbps(int maxName = 16);
	void SetChnKbps(int index, VD_PCRECT pRect, VD_PCSTR psz);
	void ShowChnKbps(int index, BOOL bShow);
	std::vector<CStatic*>m_vChnElseState;
	int m_nMaxElseStatetype;


	std::vector<CStatic*>m_vRegion;
	void InitRegion(int maxName);
	void ShowRegion( int index, BOOL bShow );
	ChnRegion nChnRegion[16];
	void SetRegion( int index, VD_PCRECT pRect, VD_PCSTR psz );
	void ShowRegionALl(int hua);
	void DeleteRegion(int index);
	
	std::vector<CStatic*>m_vTimeTile;
	void InitTimeTitle(int max = 1);
	//void SetTimeTitle(int index, VD_PCRECT pRect, VD_PCSTR psz);
	void SetTimeTitleRect(int index, VD_PCRECT pRect);
	void SetTimeTitleString(int index, VD_PCSTR psz);
	void ShowTimeTitle(int index, BOOL bShow);
	
	std::vector<CStatic*>m_vChnState;
	int m_nMaxStatetype;
	
	void SetPbElecZoomstatus(BOOL flag);
	void SetElecZoomChn(int nChn);
	void SetAllElecZoomStatueEmpty();
	int GetElecZoomChn();
	
	std::vector<CStatic*>m_vElecZoomTile;
	void InitElecZoomTile(int max);
	void SetElecZoomstatus(BOOL flag);
	void SetElecZoomRect( int index, VD_PCRECT pRect);
	void SetElecZoomTileString( int index, VD_PCSTR psz );
	void ShowElecZoomTile(int index, BOOL bShow);
	
	std::vector<CStatic*>m_vElecZoomInvalidTile;
	void InitElecZoomInvalidTile(int max);
	void SetElecZoomInvalidRect( int index, VD_PCRECT pRect);
	void SetElecZoomInvalidTileString( int index, VD_PCSTR psz );
	void ShowElecZoomInvalidTile(int index, BOOL bShow);
	
	void InitElecZoomSplitLine(int maxlines = 16, VD_COLORREF color = VD_RGB(255,255,255));
	void SetElecZoomSplitLineRect(int index, int x, int y, int length, EMSPLITLINETYPE type);
	void ShowElecZoomSplitLine(int index, BOOL bShow);
	
	void InitElecZoomAreaSplitLine(int maxlines = 16, VD_COLORREF color = VD_RGB(255,255,255));
	void SetElecZoomAreaSplitLineRect(int index, int x, int y, int length, EMSPLITLINETYPE type);
	void ShowElecZoomAreaSplitLine(int index, BOOL bShow);
	
	void InitChnState(int maxChn = 4, int maxstatetype = 5);
	void SetChnStateBMP(int nChn, int nStatetype, VD_PCSTR pszBmpName1, VD_PCSTR pszBmpName2 );
	void ShowChnState(int nChn, int nStatetype, int x, int y, EMCHNSTATESHOW showtype);
	void ClearAllChnState();

	//add by liu
	VD_BOOL ChnElseStateIsShow(int nChn, int nStatetype);
	void InitChnElseState(int maxChn = 16, int maxstatetype = 3);
	void ShowChnElseState( int nChn, int nStatetype, int x, int y, EMCHNSTATESHOW showtype );
	void SetChnElseStateBMP(int nChn, int nStatetype, VD_PCSTR pszBmpName1, VD_PCSTR pszBmpName2 );

	CStatic *pStaticVerifySNResult;//csp modify
	
	void HideAllOsdWithoutTime(int nChnNum);
	void HideDeskOsdWithoutTime(int nMaxChn);
	
	void InitCoverRect(int maxCh, int maxRect, VD_COLORREF color = VD_RGB(120,120,120));
	void SetCoverRect(int ch, int x, int y, int w, int h);
	void ClearCoverRect(int ch);
	
	void ShowCoverRect();
	void HideCoverRect();
	
	void SetModePlaying();
	void SetModePreviewing();
	
	//add by liu
	int ShowDeskStreamInfo(uint param);
	
	u8 m_FlagPlaying;
	u8 m_nPlayBackFileFormat;
	u8* m_AllChnStatus;
};

#if 0
class CPageCountDown : public CPageFrame
{
public:
	CPageCountDown(VD_PCRECT pRect,VD_PCSTR psz = NULL,VD_BITMAP* icon = NULL,CPage * pParent = NULL);
	~CPageCountDown();
	VD_BOOL UpdateData(UDM mode);
	void SetProgress(uint pos);
	VD_BOOL MsgProc(uint msg, uint wpa, uint lpa);

private:
	CProgressCtrl*	pProgress;
};
#endif

#endif //__PAGE_DESKTOP_H__

