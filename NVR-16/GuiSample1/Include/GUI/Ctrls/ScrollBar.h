//
//  "$Id: ScrollBar.h 117 2008-11-27 09:44:43Z liwj $"
//
//  Copyright (c)2008-2010, RealVision Technology CO., LTD.
//  All Rights Reserved.
//
//	Description:	
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#ifndef __CTRL_SCROLL_H__
#define __CTRL_SCROLL_H__

typedef enum
{
	scrollbarY = 0x0001,	//垂直的
}SCROLLBARSTYLE;

enum _scrollbar_color_t
{
	SLB_TEXT_TITLE	= 0,
	SLB_BK,//背景颜色
	SLB_BORDER,//焦点不在scroolbar的边框颜色
	SLB_SELBORDER,//焦点在scroolbar的边框颜色
	SLB_SEL_ITEM_TEXT,
	SLB_NOSEL_ITEM_TEXT
};

typedef enum{
	SBB_NORMAL,
	SBB_SELECTED,
	SBB_PUSHED,
	SBB_DISABLED,
}ScrollBarBitmap;


typedef void (CObject::*TRACKPROC)(int pos);

class CScrollBar : public CItem
{
	VD_COLORREF	m_BkColor;
	VD_COLORREF	m_BorderColor;
	VD_COLORREF	m_SelBorderColor;
	VD_COLORREF	m_MidLineColor;
	int		min;
	int		max;
	int		curpos;
	int		page;	//页面项数

	VD_BOOL	track;			//正在滚动
	int		tracker_height;	//滚动块高度
	int		tracker_offset;	//滚动块偏移
	int		tracker_pick;	//点击滚动块时鼠标的相对位置

	CTRLPROC m_onTrackMove;

	enum _scroll_act{	//动作
	SA_PAGEUP = 0,
		SA_PAGEDOWN,
		SA_TRACK,
	};
	static VD_BITMAP* m_bmpNormal;
	static VD_BITMAP* m_bmpSelected;
	static VD_BITMAP* m_bmpPushed;
	static VD_BITMAP* m_bmpDisabled;
private:
	void UpdateTracker();
	
public:
	/*
		modified history: --by wangqin 20070316
		修改是否属于弹出式的控件通过控件风格进行.取消掉vpopup参数~
	*/
//	CScrollBar(PCRECT pRect, CPage * pParent,uint vstyle = scrollbarY,int vmin = 0,int vmax = 100, int vpage = 1, BOOL vpopup = FALSE);
	CScrollBar(VD_PCRECT pRect, CPage * pParent,uint vstyle = scrollbarY,int vmin = 0,int vmax = 100, int vpage = 1,CTRLPROC onTrackMove = NULL);
	virtual ~CScrollBar();


	void DrawFrame();
	void Draw();
	void DrawBar(VD_BOOL paint);
	VD_BOOL MsgProc(uint msg, uint wpa, uint lpa);
	int GetCurSel();
	int GetAt(int px, int py);
	void SetRange(int vmin, int vmax, VD_BOOL redraw = TRUE);
	void SetPos(int newpos, VD_BOOL redraw = TRUE);
	int GetPos();
	void SetPage(int vpage, VD_BOOL redraw = TRUE);
	void SetColor(int nRegion, VD_COLORREF color, VD_BOOL redraw = FALSE);

	static void SetDefaultBitmap(ScrollBarBitmap region, VD_PCSTR name);
};
CScrollBar* CreateScrollBar(VD_PCRECT pRect, CPage * pParent, uint vstyle = scrollbarY,
								   int vmin = 0,int vmax = 100, int vpage = 1,CTRLPROC onTrackMove = NULL);
#endif //__CTRL_SCROLL_H__
