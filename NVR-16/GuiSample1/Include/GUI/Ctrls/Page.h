//
//  "$Id: Page.h 117 2008-11-27 09:44:43Z liwj $"
//
//  Copyright (c)2008-2010, RealVision Technology CO., LTD.
//  All Rights Reserved.
//
//	Description:	
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//


#ifndef __PAGE_H__
#define __PAGE_H__

#include "Item.h"
#include "BitmapButton.h"
#include "Button.h"
#include "CheckBox.h"
#include "NumberPad.h"
#include "NumberBox.h"
#include "ScrollBar.h"
#include "ListBox.h"
#include "ComboBox.h"
#include "DateTimeCtrl.h"
#include "GroupBox.h"
#include "IPAddressctrl.h"
#include "GridCtrl.h"
#include "Menu.h"
#include "ProgressCtrl.h"
#include "SliderCtrl.h"
#include "Static.h"
#include "Edit.h"
#include "TimingCtrl.h"
#include "Radio.h"
#include "Calendar.h"
#include "TableBox.h"
//#include "TimeTable.h"
#include "RecFileTimeBox.h"

#include "MultiTask/MsgQue.h"
#include <list>

#include "GUI/Pages/BizData.h"


//#define BIZ

//页面属性
typedef enum
{
	pageDialog	= 0x0001,
	pageFullScreen = 0x0002,
	pageAlignCenter = 0x0004,
	pageNoExpire = 0x0008,		//不自动注销
	pageTopMost = 0x0010,		//总在最上
	pageActiveMost = 0x0020,	//总是激活
	pageChild = 0x0040, //cj@20110419
}PAGESTYLE;


#define ROW_MAX_NUM 20		//每页的最大行数
#define MAX_TABLE_NUM 7     //最多可以设置的TABLE数
/*begin 自动布局*/
typedef struct _AUTO_LAYOUT
{
	CItem* pCur;			//当前控件指针
	CItem* head;			//第一个控件
	CItem* pPre;			//当前遍历的控件的先前控件指针
	CItem* pLineHead;		//行的第一个控件指针.用处:假设一行排序超出客户区,那么需要重新排序该行,所以必须记录下来
	int row[ROW_MAX_NUM];	//控件几行几列数组
	int rowNum;			//控件行数
	int curRow;			//当前所在行数
	int curColumn;			//当前所在的列数
	int column;				//记录对齐的列号
	int width;				//列间距
	int maxBottom;			//上一行的最大底端，用于行对齐
	VD_BOOL reRowLayout;		//该行是否需要重新布局,一行超过客户区的时候,用到
	VD_BOOL secondLayout;		//是否需要二次布局，在有列对齐控件时，如果该列的left发生变化需要二次布局
	int startX;		//控件布局起始x位置
	int startY;		//控件布局起始y位置
	int columnLeft[MAX_TABLE_NUM];		//列的left，列队齐控件使用
}AUTO_LAYOUT;
/*end 自动布局*/
typedef enum
{
	pageIconSmall,
	pageIconNor,
	pageIconSel,

}PAGEICON;

typedef enum{
	PGB_KEY_PAGEUP,
	PGB_KEY_PAGEDOWN,
	PGB_KEY_FUNCTION,
	PGB_KEY_ENTER,
	PGB_CLOSE_BUTTON_NORMAL,
	PGB_CLOSE_BUTTON_SELECTED,
	PGB_CLOSE_BUTTON_PUSHED,
	PGB_CLOSE_BUTTON_DISABLED,
	PGB_ICON_SMALL,
}PageBitmap;

class CGUI;

class CPage : public CItem
{
public:
	enum LayoutSpace
	{
		spaceCheckBoxSmall,
		spaceCount,
	};

	static CPage* m_pDesktop;
private:

	//for page up/down
	int		m_iPage;//子控件页号
	int		m_nPage;//子控件页数

	friend	class CItem;
	friend  class CItemThread;

	int *m_playoutSpace;

	std::string	m_pTitle;	//标题字符串

	VD_BOOL	m_bDrawBkgdOnly;	//只画背景
	VD_BOOL    ishPageMain;     // 主页面是横向菜单，主要是去处居中属性影响。
	
/*begin 自动布局*/
	int m_iSpace; //控件间距
	int m_iStartX; //控件起始 X 坐标
	int m_iStartY; //控件起始 Y 坐标
	int m_iTable[MAX_TABLE_NUM]; //列对齐left坐标
	int SetCtrlLeft(AUTO_LAYOUT *pAL);
	int SetCtrlRight(AUTO_LAYOUT *pAL, int left);
	int SetCtrlTop(AUTO_LAYOUT *pAL);
	int SetCtrlBottom(AUTO_LAYOUT *pAL, int top);
	void LayoutInit(AUTO_LAYOUT *pAL);

protected:

	/*自动布局函数*/
	void AutoLayout();

	/*设置首控间的起始坐标*/
	void SetStartPoint(int x = CLIENT_X, int y = CLIENT_Y);

	/*设置控件的间距*/
	void SetCtrlSpace(int width);

	/*设置列对齐控件的左坐标*/
	void SetTable(int TableNum, int left);

	// 和SetCtrlSpace有重复
	void SetCtrlSpace(LayoutSpace type, int space);

	
/*end 自动布局*/

protected:
	UDM		m_ExitState;	//页面最后一次退出时的状态， 如确认退出/取消退出等
	VD_BOOL m_ifPageMain; //是否是主界面

	//for mouse buttons double click
	VD_BOOL	m_bLClicked;	//鼠标左键按下
	VD_BOOL	m_bRClicked;	//鼠标右键按下
	VD_BOOL	m_bMClicked;	//鼠标中键按下
	
	//for resize page
	CRect	m_RectResizing;	//移动页面时显示的边框区域
	VD_BOOL	m_bResizing;	//页面是否处于移动状态
	VD_BOOL	m_bResizable;	//是否可以移动页面
	int		m_iResizeXOffs;	//使用鼠标移动页面时鼠标点击的x坐标
	int		m_iResizeYOffs; //使用鼠标移动页面时鼠标点击的y坐标

	//child items of page
	CItem *	m_pChildHead;		//the first child item added
	CItem *	m_pChildTail;		//the last child item added
	CItem * m_pChildDflt;		//default item
	CItem * m_pChildFocus;		//the input focus item
	CItem * m_pChildFocusOld;	//store the old focus when it missed
	CItem * m_pChildSelect;		//the select item
	CItem * m_pChildSelectOld;	//store the old select when it missed

	int		m_TitleWidth;		//标题栏宽度
	int		m_TitleHeight;		//标题栏高度

	VD_BITMAP*	m_pIconBmp;     //标题栏图标
	VD_BITMAP* m_pNorBmp;
	VD_BITMAP* m_pSelBmp;
	
	VD_BITMAP *bmp_frame_title;
	VD_BITMAP *bmp_frame_mid;
	VD_BITMAP *bmp_frame_bottom;

	static VD_BITMAP* bmp_key_pageup;
	static VD_BITMAP* bmp_key_pagedown;
	static VD_BITMAP* bmp_key_function;
	static VD_BITMAP* bmp_key_enter;
	static VD_BITMAP* bmp_button_close[4];
	static VD_BITMAP* icon_dvr;

	int m_mouseX;
	int m_mouseY;

private:
	int	m_nAge;//年龄,和graphics不一致时需要更新
	CPage* m_pPopupCaller;		//对话框的调用者

	typedef std::map<std::string, CPage*> PAGE_MAP;
	static  PAGE_MAP m_existentPages[N_GUI_CORE];

	static int sm_layoutSpace[spaceCount];

private:
	void UpdateInputTray();

protected:
	void	Draw();//Draw
	UDM		PopupRun();
	void	AddItem(CItem *pItem);//将控件加入到页面
	void	DeleteItem(CItem *pItem);//将控件从页面中删除

	CItem*	ShiftItem(int key, CItem *pCur, CItem *pOld);
	CItem* 	FindStartItem();
	CItem*	GetItemAt(int px, int py);

	virtual VD_BOOL InvalidateRgn(CRgn *  pRgn);
	virtual VD_BOOL ValidateRgn(CRgn *  pRgn);
	virtual void UpdateRgn();
	virtual void ClearRgn();

	void	ResizeStart();
	void	ResizeEnd();
	void	KillFocus();
	void	Resize(int ox, int oy);
	void	Scroll(int oPage);
	void	SetTitleSize(int w, int h);
	void	SetMargin(int l, int t, int r, int b);

	virtual void OnChildClosed(CPage *pPage){};

public:	
	virtual void DrawTip(const char *pcTip){} //xym GUI 
	
	void SetPageIcon(PAGEICON pageIcon,VD_BITMAP* bmp);
	VD_BITMAP* GetPageIcon(PAGEICON pageIcon);
	VD_PCSTR  GetTip(void);

	VD_BOOL Open();//打开
	VD_BOOL Close();//关闭
	virtual VD_BOOL Close(UDM mode);//关闭

	
	virtual void SetMute(BOOL bMute);
	virtual void GetCurPlayMute(uchar* mute);
	virtual void SetCurPlayMute(uchar mute);
	
	virtual VD_BOOL UpdateData(UDM mode){return TRUE;}
	CPage(VD_PCRECT pRect, VD_PCSTR psz, CPage *pParent = NULL, uint style = 0);
	virtual ~CPage();
	void ThreadProc();
	VD_BOOL PreMsgProc(uint msg, uint wpa, uint lpa);//预处理消息
	VD_BOOL MsgProc(uint msg, uint wpa, uint lpa);//处理消息
	virtual VD_BOOL AdjustRect();
	virtual void ShowTip(VD_PCSTR tip, VD_BITMAP * icon = NULL){};
	virtual void SetRect(VD_PCRECT pRect, VD_BOOL bReDraw = TRUE);

	VD_PCSTR	GetTitle();
	void    SetTitle(VD_PCSTR psz);

	CItem*	GetFocusItem();
	CItem*  GetSelectItem();//cw_page
	void	SetDefaultItem(CItem *pItem);
	void	SetSelectItem(CItem *pItem);
	void	SetFocusItem(CItem *pItem);

	CPage* GetParentPage();
	void SetParentPage(CPage *pParent);
	int GetThreadID();
	UDM GetExitState();

	void GetMargin(VD_PRECT rect);
	
	static CPage* FindPage(int screen, VD_PCSTR name, VD_BOOL mustOpened = FALSE);
	//SIZE& GetDefaultSize(void);

	static void SetDefaultBitmap(PageBitmap region, VD_PCSTR name);

	void SetStyle(VD_BOOL iPageMain = FALSE){ishPageMain = iPageMain;};
	
};

#endif //__PAGE_H__
