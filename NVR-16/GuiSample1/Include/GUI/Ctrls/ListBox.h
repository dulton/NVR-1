//
//	"$Id: ListBox.h 117 2008-11-27 09:44:43Z liwj $"
//
//	Copyright (c)2008-2010, RealVision Technology CO., LTD.
//	All Rights Reserved.
//
//	Description:	
//	Revisions:		Year-Month-Day	SVN-Author	Modification
//


#ifndef __CTRL_LIST_H__
#define __CTRL_LIST_H__

#if defined(WIN32)
	#pragma warning (disable : 4786)
#endif
#include <vector>


typedef enum
{
	LIS_CHECKED =	0x0001,	//是否选中, 只对序号栏有效
	LIS_VALID =		0x0002,	//是否有效, 只对序号栏有效
	LIS_SELECTED =	0x0004,	//是否选中, 只对序号栏有效
	LIS_ASCENDING = 0x0008,	//排序类型, 只对标题栏有效
}
LIST_ITEM_STATE;

typedef enum
{
	LIF_TEXTALIGN =			0x0000FFFF,	//文本对齐方式掩码
	LIF_FORMAT =			0xFFFF0000,	//图片显示格式掩码
	LIF_IMAGE =				0x00010000,	//是否有图片, 只对标题栏有效
	LIF_IMAGE_ON_RIGHT =	0x00020000,	//图片显示在右边, 只对标题栏有效
}
LIST_ITEM_FORMAT;

struct LIST_ITEM
{
	ushort width;				//项宽度, 只对标题栏有效
	ushort height;			//项高度, 只对序号栏有效
	uint state;			//项状态, 由各种状态组合而成
	VD_BITMAP* image;			//图片
	int	 format;			//字符串对齐方式与图片显示格式
	uint data;				//用户数据
	std::string string;		//字符串
	std::string customData;
};

typedef std::vector<LIST_ITEM> LIST_ITEM_VECTOR;

typedef enum
{
	//LTS_POPUP = 0x0001,		//弹出式  请日后务必使用stylePopup来代替，不要再次使用这个标志。20070326--by wangqin 
	listboxIndex = 0x0001,		//带序号
	listboxTitle = 0x0002,		//带标题栏
	listboxCheck = 0x0004,  //带复选框
	listboxIcon = 0x0008,		//序号带图标
	listboxAutoIndex = 0x0010, //自动获得序号
	listboxTopCheck = 0x0020, //在标题栏显示全选项
}LISTSTYLE;

enum _list_color_t
{
	LTC_TEXT	= 0,
	LTC_BK,
	LTC_BORDER,
	LTC_SELBORDER,
	LTC_SEL,
	LTC_NOSEL,
	LTC_TITLELINE,
};

typedef enum{
	LTB_NORMAL,
	LTB_SELECTED,
	LTB_DISABLED,
}ListBoxBitmap;

class CListBox : public CItem
{
//	BOOL	hheader;	//显示标题栏
//	BOOL	vheader;	//显示序号栏
	VD_COLORREF	m_TextColor;
	VD_COLORREF	m_BkColor;
	VD_COLORREF	m_BorderColor;
	VD_COLORREF	m_SelBorderColor;
	VD_COLORREF	m_SelectColor;
	VD_COLORREF	m_NoSelectColor;
	VD_COLORREF 	m_TitleLine;//标题栏与内容的分隔线
	VD_COLORREF	m_PopupColor;
	VD_COLORREF	m_CtrlSelect;//被选中的条目颜色
	
//	uint	style;		//列表样式
	int		num;		//总项数, 包括标题栏
	int		column;		//总栏数, 包括序号栏
	int		cursel;		//当前选中项
	int		curhl;		//当前高亮项
	int		index_start;//当前页序号开始项
	int		col_start;	//当前页栏开始项
	int		rows;		//页面行数
	LIST_ITEM_VECTOR items;  //列表项
	CScrollBar* vscrollbar;	//垂直滚动条
	CScrollBar* hscrollbar;	//水平滚动条
	int		m_iTextAlign;	//默认字符串对齐方式
	VD_BOOL   m_FlagSelectAll;  // 全选标志位   zgzhit 
	//BOOL   m_bSetCheckAll;  //是否有全选框标志   zgzhit //使用listboxTopCheck标志替代原有的bool变量m_bSetCheckAll WQ deleted  2007-05-05
	CTRLPROC m_onEntered;
	CTRLPROC m_onChecked;
	CTRLPROC m_onChanged;
protected:
	static VD_BITMAP* bmp_ltb_normal;
	static VD_BITMAP* bmp_ltb_selected;
	static VD_BITMAP* bmp_ltb_disabled;
private:
	void DrawSingle(int index, int col = -1, VD_BOOL drawback = TRUE);
	void DrawFrame();

public:
	CListBox(VD_PCRECT pRect, CPage * pParent, VD_PCSTR psz, CItem *p = NULL, uint vstyle = listboxIndex | listboxTitle | listboxAutoIndex,
					CTRLPROC onEntered = NULL, CTRLPROC onChecked = NULL, CTRLPROC onChanged = NULL);
	virtual ~CListBox();
	
	void Draw();
	VD_BOOL Open();
	VD_BOOL MsgProc(uint msg, uint wpa, uint lpa);
	VD_BOOL GetIfSelectAll();
	void SetRows();
	
	//栏操作
	int	InsertColumn(int col, LIST_ITEM *pItem, VD_BOOL redraw = FALSE);
	VD_BOOL DeleteColumn(int col, VD_BOOL redraw = FALSE);
	VD_BOOL SetColumn(int nCol, LIST_ITEM *pItem, VD_BOOL redraw = FALSE);
	VD_BOOL GetColumn(int nCol, LIST_ITEM *pItem);
	void RemoveAllColumn(VD_BOOL redraw = FALSE);

	//项操作
	int InsertItem(int index, LIST_ITEM *pItem, VD_BOOL redraw = FALSE);
	VD_BOOL DeleteItem(int index, VD_BOOL redraw = FALSE);
	VD_BOOL SetItem(int index, LIST_ITEM *pItem, VD_BOOL redraw = FALSE);
	VD_BOOL GetItem(int index, LIST_ITEM *pItem);
	VD_BOOL SetItemData(int index, uint data);
	VD_BOOL SetCustomData(int index, const char *data);
	VD_BOOL SetSubData(int index, int col, uint data);
	uint GetSubData(int index, int col);
	uint GetItemData(int index);
	std::string GetCustomItemData(int index);
	void RemoveAll(VD_BOOL redraw = FALSE);

	//子项操作
	int SetSubItem(int index, int col, LIST_ITEM *pItem, VD_BOOL redraw = FALSE);
	int GetSubItem(int index, int col, LIST_ITEM *pItem);

	//字符串操作
	void SetTitle(VD_PCSTR psz);
	int AddString(VD_PCSTR string, VD_PCSTR header = "", VD_BOOL redraw = FALSE);
	void DeleteString(VD_BOOL redraw = FALSE);
	void SetString(VD_PCSTR string, VD_PCSTR header = "", VD_BOOL redraw = FALSE);
	void SetCorner(VD_PCSTR psz, VD_BOOL redraw = FALSE);
	VD_BOOL SetSubString(int index, int col, VD_PCSTR psz, VD_BOOL redraw = FALSE);
	void SetTextAlign(int align = VD_TA_LEFTTOP);

	int GetCurSel();
	int GetAt(int px, int py);
	void SetCurSel(int sel, VD_BOOL redraw = FALSE);
	int GetCount();
	VD_BOOL GetCheck(int index);
	VD_BOOL SetCheck(int index, VD_BOOL checked = TRUE, VD_BOOL redraw = FALSE);
	VD_BOOL GetValid(int index);
	VD_BOOL SetValid(int index, VD_BOOL valid = TRUE, VD_BOOL redraw = FALSE);
	void ShowScrollBar(VD_BOOL show);
	void Select(VD_BOOL flag);
	void Show(VD_BOOL flag);
	VD_BOOL Close();
	void SetRect(VD_PCRECT pRect, VD_BOOL bReDraw = TRUE);
	void SetColor(int nRegion, VD_COLORREF color, VD_BOOL redraw = FALSE);
	 //设置拥有全部复选框的接口函数
	void SetSelectAll(VD_BOOL bSet);
	VD_SIZE& GetDefaultSize(void);

	static void SetDefaultBitmap(ListBoxBitmap region, VD_PCSTR name);
};
CListBox* CreateListBox(VD_PCRECT pRect, CPage * pParent, VD_PCSTR psz, CItem *p = NULL, uint vstyle = listboxIndex | listboxTitle | listboxAutoIndex,
							  CTRLPROC onEntered = NULL, CTRLPROC onChecked = NULL, CTRLPROC onChanged = NULL);
#endif //__CTRL_LIST_H__
