//
//  "$Id: Menu.h 117 2008-11-27 09:44:43Z liwj $"
//
//  Copyright (c)2008-2010, RealVision Technology CO., LTD.
//  All Rights Reserved.
//
//	Description:	
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#ifndef __CTRL_MENU_H__
#define __CTRL_MENU_H__

#define MENU_MAX 32   //弹出菜单不超过32项

class CMenu;

typedef enum{
	styleVertical		 = 0x0001, //普通竖向排部的menu，如桌面右键菜单
	styleNavigation		 = 0x0002, //类似于主菜单式的排布方式,文字显示在图标下面，如AeBell的二级菜单
	styleHorizontal		 = 0x0004, //横向排布的菜单，如honywell的一级菜单
	styleDeskTop		 = 0x0008, //桌面
	styleDeskTopNew		 = 0x0010, //桌面_cj
	styleNavTextInside	 = 0x0040, //自动具有styleNavigation属性，,区别是文字显示在图标内部，如CDJF
}MENU_STYLE;

//配色枚举 
typedef enum{
	colBKNormal			= 0x00,  //背景颜色
	colBKTitle			= 0x01,  //标题背景色
	colBKFrame			= 0x02,  //背景边框色
	colSingleNormal		= 0x03,  //单项未被选中的颜色
	colSingleSelect		= 0x04,  //单项被选中的颜色
	colFontNormal		= 0x05,  //正常字体颜色
	colFontSelect		= 0x06,  //选中字体颜色
	colFontTitle		= 0x07,  //标题背景颜色
	colFrameNormal		= 0x08,  //正常边框颜色、单项
	colFrameSelect		= 0x09,  //选中边框颜色、单项
}MENU_COLOR_TYPE;
enum MenuType
{
	MT_NULL,	// 空类型
	MT_NOBODER,	// 没有边框的菜单
	//.. 其它可以依次添加
};
typedef enum{
	MNB_PANEL,
	MNB_TITLED_PANEL,
	MNB_BAR,
	MNB_ITEM_NORMAL,
	MNB_ITEM_SELECTED,
	MNB_ITEM_DISABLED,
}MenuBitmap;

typedef struct tagMENU_ITEM{
	VD_BITMAP*		bmp1;
	VD_BITMAP*		bmp2;
	VD_BITMAP*		bmp3;
	std::string	string;
	std::string	tip;
	CMenu*		sub;
    CTRLPROC	proc;
	CPage*		child;
	VD_BOOL		sprtr;	//分割符号
	int			y_offset;
	int			x_offset;
}MENU_ITEM;


class CMenu : public CItem   //用于弹出多级菜单的控件
{
	int			num;
	int			y_offset;
	int			x_offset;
	MENU_ITEM	items[MENU_MAX];
	CMenu*		pParentMenu;
	int			cursel;     //当前项

	int			lastssel;

	//BITMAP*		templet;
	int			clicked;	//非弹出式菜单已展开
	std::string	title;
	VD_BOOL		m_IFDTop;
	int			m_iItemHeight; //menu每项的高度
	int			m_iItemWidth; //menu每项的宽度
	int			m_iySpace; //styleVertical类型menu垂直边距
	int			m_ixSpace; //styleVertical类型menu水平边距
	int			m_iStartX; //styleVertical类型的menu图标或文字的起始x坐标

	int			m_iConstColNum;
	int			m_iColNum;
	int			m_iRowNum;
	int			m_iFrameWidth; //边框宽度
	VD_BOOL         isRevPopup;   //是否反向弹出方式显示
	int item_height;
	VD_BOOL		m_autoSpread;
	VD_BITMAP*		m_bmpBKNormal; //背景图片
	VD_BITMAP*     m_bmpBKNormal1;
	VD_BITMAP*		m_bmpSingleNormal;//单项未被选中的图片
	VD_BITMAP*		m_bmpSingleSelect; //单项被选中的图片
	VD_SIZE		m_customSize;
	VD_SIZE		m_customInterval;
	VD_COLORREF	m_colBKNormal; //背景颜色
	VD_COLORREF	m_colBKTitle; //标题背景色
	VD_COLORREF	m_colBKFrame; //背景边框颜色
	VD_COLORREF	m_colSingleNormal; //单项未被选中的颜色
	VD_COLORREF	m_colSingleSelect; //单项被选中的颜色
	VD_COLORREF	m_colFontNormal; //正常字体颜色
	VD_COLORREF	m_colFontSelect; //选中字体颜色
	VD_COLORREF	m_colFontTitle; //标题背景颜色
	VD_COLORREF	m_colFrameNormal; //正常边框颜色、单项
	VD_COLORREF	m_colFrameSelect; //选中边框颜色、单项
	
	static VD_BITMAP* m_bmpPanel;
	static VD_BITMAP* m_bmpTitledPanel;
	static VD_BITMAP* m_bmpBar;
	static VD_BITMAP* m_bmpItemNormal;
	static VD_BITMAP* m_bmpItemSelected;
	static VD_BITMAP* m_bmpItemDisabled;

	void Init(void);
	void SetIconArray(int number, MENU_ITEM *pitem);
	void SetItemRight(VD_RECT* pRect);
	void SetItemArray(void);
	void DrawBk(void);
	enum MenuType m_menuType;
public:
	int                  move_sel;  //主菜单上一次鼠标移动选中项
	
public: 

	CMenu(VD_PCRECT pRect, CPage * pParent, VD_PCSTR psz = NULL, CItem *pOwner = NULL,uint style = stylePopup | styleVertical, VD_BOOL IfDeskTop = FALSE);
	virtual ~CMenu();   

	void DrawTip(int sel); //xym GUI //显示提示信息
	void SetMenuType(enum MenuType type) {m_menuType = type;}
	void Draw(void);
	void DrawSingle(int index, VD_BOOL bDrawBk = TRUE);
	void Select(VD_BOOL flag);
	int  GetAt(int px, int py);
	VD_BOOL MsgProc(uint msg, uint wpa, uint lpa);
	int  GetCurSel(void);   
	void SetCurSel(int sel);   
	void ApendItem(VD_PCSTR psz, CTRLPROC vproc, VD_BITMAP* icon1 = NULL,VD_BITMAP* icon2 = NULL,VD_BITMAP* icon3 = NULL,VD_PCSTR tip = NULL);
	void ApendItem(VD_PCSTR psz, CMenu *pMenu, VD_BITMAP* icon1 = NULL,VD_BITMAP* icon2 = NULL,VD_BITMAP* icon3 = NULL,VD_PCSTR tip = NULL);
	void ApendItem(VD_PCSTR psz, CPage *pPage, VD_BITMAP* icon1 = NULL,VD_BITMAP* icon2 = NULL,VD_BITMAP* icon3 = NULL,VD_PCSTR tip = NULL);
	void ApendItem(VD_BITMAP* icon1 = NULL);
	VD_BOOL SetCustomPosition(int sel, const VD_POINT &top);
	VD_BOOL SetCustomColumnSize(const VD_SIZE &size);
	VD_BOOL SetCustomInterval(const VD_SIZE &size);
	void RemoveAllItems();
	void TrackPopup(int px, int py);
	VD_BOOL Close();
	void TrackSubMenu();
	void SetItemCallback(int index, CTRLPROC vproc);
	void SetItemText(int index, VD_PCSTR psz, VD_BOOL redraw = FALSE);
	void SetItemBmp(int index, VD_BITMAP* icon1 = NULL, VD_BITMAP* icon2 = NULL, VD_BITMAP* icon3 = NULL, VD_BOOL redraw = FALSE); //设置菜单每项的图片add langzi 2009-11-9 
	int GetCount();
	VD_BOOL GetItem(int index, MENU_ITEM* pItem);

	/*设置menu每项的高度*/
	void SetItemHeight(int itemHeight); 

	/*设置menu每项的宽度*/
	void SetItemWidth(int itemWidth);

	/*设置styleVertical类型menu垂直边距*/
	void SetYSpace(int ySpace);

	/*设置styleVertical类型menu水平边距*/
	void SetXSpace(int xSpace);

	/*设置menu配色接口*/
	void SetMenuColor(MENU_COLOR_TYPE colorType, VD_COLORREF color, VD_BOOL redraw = TRUE);

	/*设置menut贴图接口*/
	void SetMenuBmp(VD_BITMAP* bkNor, VD_BITMAP* singleNor = NULL, VD_BITMAP* singleSel = NULL, VD_BOOL redraw = TRUE);

	/*设置Navigation型的menu的列数*/
	void SetNavColumnNum(int columnNum);

	/*设置menu边框的宽度*/
	void SetFrameWidth(int width, VD_BOOL redraw = FALSE);

	void SetRevPopup(VD_BOOL iRevPopup = FALSE) { isRevPopup = iRevPopup ;};

	void SetAutoSpread(VD_BOOL autoSpread);
	
	static void SetDefaultBitmap(MenuBitmap region, VD_PCSTR name);
};

CMenu* CreateCMenu(VD_PCRECT pRect, CPage * pParent, VD_PCSTR psz = NULL, CItem *pOwner = NULL,uint style = stylePopup | styleVertical, VD_BOOL IfDeskTop = FALSE);

#endif //__CTRL_MENU_H__

