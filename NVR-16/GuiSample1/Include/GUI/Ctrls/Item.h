

#ifndef __ITEM_H__
#define __ITEM_H__

#include "Devices/DevFrontboard.h"
//#include "Devices/DevVideo.h"
//#include "Devices/DevSplit.h"
#include "Devices/DevMouse.h"
#include "System/Object.h"
#include "MultiTask/Thread.h"
#include "GUI/GDI/DC.h"
#include "Defs.h"
#include <string>

typedef void (CObject::*CTRLPROC)();

typedef enum
{
	IT_BITMAPBUTTON,
		IT_BUTTON,
		IT_CHECKBOX,
		IT_COMBOBOX,
		IT_DATETIMECTRL,
		IT_EDIT,
		IT_GRIDCTRL,
		IT_GROUPBOX,
		IT_IPADDRESSCTRL,
		IT_LISTBOX,
		IT_MENU,
		IT_NUMBERBOX,
		IT_NUMBERPAD,
		IT_PAGE,
		IT_PROGRESSCTRL,
		IT_SCROLLBAR,
		IT_SLIDERCTRL,
		IT_STATIC,
		IT_TIMINGCTRL,
		IT_RADIO,
		IT_CALENDAR,
		//IT_TIMETABLE,
		IT_TABLEBOX,
		IT_RECTIMEBOX,
		IT_NR,
}ITEM_TYPE;

typedef enum
{
	IF_EDITABLED = 0,	//控件是否能被编辑
	IF_ENABLED ,		//是否允许键盘或鼠标输入
	IF_POPUP,		//控件是否弹出式
	IF_AUTOFOCUS,	//自动聚焦，即控件选中后立即可以编辑
	IF_CAPTURED ,	//输入捕获
	IF_FOCUSED,		//是否为输入焦点
	IF_SHOWN ,		//是否显示
	IF_SELECTED ,	//是否选中
	IF_OPENED ,		//是否已打开
	IF_ORDERED,		//是否加入控件排序
}ITEM_FLAG;

typedef enum {
	KIF_NUMERIC,			//有数字键
		KIF_SHIFT,		//有Shift键
		KIF_CORRECTMAP,	//数字字母映射表错误
		KIF_NEWATM,	//新的二代ATM前面板
		KIF_NR,
}KEY_INPUT_FLAG;

typedef enum {
	KI_NUMERIC,			//数字
		KI_UPPERCASE,	//大写字母
		KI_LOWERCASE,	//小写字母
		KI_MARK,		//常用符号
		KI_PINYIN,		//拼音输入
		KI_CONTROL,		//ATM原始控制键
		KI_ALL,
}KEY_INPUT_TYPE;

typedef enum {
	UDM_EMPTY = 0,
	UDM_OPEN,
	UDM_OK,
	UDM_DFLT,
	UDM_CANCEL,
	UDM_COPY,
	UDM_PASTE,
	UDM_OPENED,
	UDM_CLOSED,
	UDM_GUIDE,
}UDM;

enum
{
	NORMAL,
	SELECT,
	PUSH,
	ELAPSED = PUSH,
	DISABLE,
	BACKGROUND,
};

typedef struct {
	char *name;
	int size;
}CTRL_CLASS;

class CPage;
class CItem;
class CItemThread : public CThread
{
	friend class CPage;
	friend class CItem;
	void ThreadProc();
	CItem * m_pItem;

public:
	CItemThread(VD_PCSTR name, CItem * pItem);
	virtual ~CItemThread();
};

//控件状态 20070412 added by wangqin 
typedef enum {
	ctrlStateNormal		= 0x0001,//普通状态
	ctrlStateSelect		= 0x0002,//选中
	ctrlStatePushed		= 0x0003,//按下
	ctrlstateDisabled	= 0x0004,
}CTRL_STATE;

typedef struct  
{
	CItem * m_pLeftRelated;			//左关联控件	zx add 2006-03-01
	CItem * m_pRightRelated;		//右关联控件
	CItem * m_pUpRelated;		//上关联控件
	CItem * m_pDownRelated;	//下关联控件
}RELATEDITEM;

/*!
	控件属性，包括自动布局和固有属性 
	8-16位用于表示控件共有属性及其他基类风格属性
	高16位用于表示Item控件自动布局属性。
	通过位运算进行属性判断。
*/
typedef enum
{
	//控件共有属性 8-16位表示
	styleNoBorder		= 0x00000100,//无边框，程序员自己设置。
	styleUseBmp			= 0x00000200,//控件贴图属性，由程序自己控制，程序员不用设置。
	//Item自有风格属性，所有从Item继承来的控件都有此属性
	stylePopup			= 0x0000400, //popup类型控件？
	styleEditable		= 0x00000800, //是否可编辑
	styleAutoFocus		= 0x00001000, //自动聚焦    //自动布局属性 高16位表示
	//剩余3个标志位now
	changeLine		= 0x00010000, // 换行
	alignAlt		= 0x00020000, // 控件交替显示或隐藏
	smallWidth		= 0x00040000, // 小间距
	alignCenter		= 0x00080000, //行居中对齐，只对行的第一个控件有效
	alignLeft			= 0x00100000, // 向左对齐
	alignRight			= 0x00200000, // 向右对齐
	alignTop			= 0x00400000, // 向上对齐
	alignBottom			= 0x00800000, // 向下对齐
	inflateX			= 0x00300000, // 水平膨胀
	inflateY			= 0x00C00000, // 垂直膨胀
	inflateAll			= 0x00F00000, // 全方向膨胀
	alignTable1		= 0x01000000, // 对齐到第1列，会根据前一列控件自动调整
	alignTable2		= 0x02000000, // 对齐到第2列，会根据前一列控件自动调整
	alignTable3		= 0x03000000, // 对齐到第3列，会根据前一列控件自动调整
	alignTable4		= 0x04000000, // 对齐到第4列，会根据前一列控件自动调整
	alignTable5		= 0x05000000, // 对齐到第5列，为需要特殊指定X坐标对齐的控件保留
	alignTable6		= 0x06000000, // 对齐到第6列，为需要特殊指定X坐标对齐的控件保留
	alignTable7		= 0x07000000, // 对齐到第7列，为需要特殊指定X坐标对齐的控件保留
	alignTableMask	= 0x0F000000, //列对齐掩码

}ITEM_STYLE;
/*end wangqin 20070309 added*/


class CDC;
class CItem : public CObject
{
	friend class CPage;
	friend class CDC;
	friend  class CItemThread;

public:
	const int		m_screen;			//screen ID for each item

protected:
	CPage *	m_pParent;	//父页
	CItem *	m_pOwner;	//直属控件

	//brother items in popup item
	CItem * m_pBrother;		//the brother item
	CItem * m_pBrotherR;		//the reverse brother item
	RELATEDITEM *m_pRelatedItems;	//关联控件

	//opened popup items
	CItem * m_pUp;				//the up popup item
	CItem * m_pDown;			//the down popup item
	static	CItem * m_pTop[N_GUI_CORE];		//the top popup item
	static	CItem * m_pNoTopMost[N_GUI_CORE];//the top popup item, excluding top-most item
	static	CItem * m_pActive[N_GUI_CORE];	//the current popup item
	static	CItem * m_pInputTray[N_GUI_CORE]; //the input tray item
	static	CItem * m_pSpellPad[N_GUI_CORE];  //the spell pad item

	//key input flags
	static VD_BOOL m_bKeyInputFlags[KIF_NR];

protected:
	static int m_nInstance[IT_NR];

	uint m_dwStyle; //ITEM控件属性
	//for states
	uint		m_dwFlags;//控件标志

	CDevGraphics* m_pDevGraphics; //the graphics device
	CCursor* m_pCursor;	//the cursor


protected:
	ITEM_TYPE	m_iType;//控件类型
	std::string	tip;	//提示字符串
	CDC		m_DC;//设备上下文
	CRect	m_Rect;
	CRect	m_Margin;//4个方向上的边距
	CItemThread * m_pThread;
	static CMutex m_MutexMgr;

protected:
	std::string	string;	//控件字符串 --by wangqin 20070406从各个控件移到此处。

private:
	//region clipping operations
	virtual VD_BOOL InvalidateRgn(CRgn *  pRgn);
	virtual VD_BOOL ValidateRgn(CRgn *  pRgn);
	virtual void UpdateRgn();
	virtual void ClearRgn();

	void GlobalInvalidateRect(VD_PCRECT pRect);
	void GlobalValidateRect(VD_PCRECT pRect);
	void GlobalInvalidateRgn(CRgn *  pRgn);
	void GlobalValidateRgn(CRgn *  pRgn);

private:
	// transform link
	void Attach();
	void Detach();
	void Activate();

protected:
	void ItemProc(CTRLPROC proc);
	VD_BOOL DrawCheck();
	
public:

	uint GetStyle();
	CItem(VD_PCRECT pRect, CPage * pParent, ITEM_TYPE type,uint style = 0);					//构造函数
	virtual ~CItem();										//析构函数			
	virtual void SetTip(VD_PCSTR psz);									//设置提示字符串
	virtual VD_SIZE& GetDefaultSize(void);//各个控件的默认大小，各个支持自动布局的控件自己实现。--by wangqin 20070401
	virtual void Draw() = 0;									//绘制
	virtual VD_BOOL PreMsgProc(uint msg, uint wpa, uint lpa){return FALSE;};	//事件预处理
	virtual VD_BOOL MsgProc(uint msg, uint wpa, uint lpa){return FALSE;};	//事件处理
	VD_BOOL VD_SendMessage(uint msg, uint wpa = 0, uint lpa = 0);
	virtual void SetText(VD_PCSTR psz){};						//设置控件字符串
	virtual VD_PCSTR GetText(){return NULL;}					//得到控件字符串
	virtual void Editable(VD_BOOL flag);						//设置是否能被编辑
	virtual void Show(VD_BOOL flag, VD_BOOL redraw = TRUE);							//显示/隐藏控件
	virtual void Enable(VD_BOOL flag);							//激活/禁止控件
	virtual void Select(VD_BOOL flag);							//选中/不选中控件
	virtual void SetFocus(VD_BOOL flag);						//设置为输入焦点
	virtual void ShowTip(VD_BOOL flag);
	virtual VD_BOOL Open();//打开
	virtual VD_BOOL Close();//关闭
	virtual VD_BOOL AdjustRect(){return TRUE;};
	virtual void AddItem(CItem *pItem){};
	virtual void DeleteItem(CItem *pItem){};
	VD_BOOL IsOpened();										//得到打开/关闭状态
	VD_BOOL GetFlag(ITEM_FLAG index);
	void SetFlag(ITEM_FLAG index, VD_BOOL flag);
	uint ReadFlag();//cw_mainpage
	void SetAutoFocus(VD_BOOL flag);			//设置自动聚焦
	VD_BOOL IsShown();							//得到显示/隐藏状态
	VD_BOOL IsEnabled();						//得到激活/禁止状态
	VD_BOOL IsSelected();						//得到选中/不选中状态
	void SetCapture(VD_BOOL flag);				//设置鼠标捕获
	VD_BOOL GetCapture();						//得到鼠标捕获状态
	VD_BOOL GetFocus();						//得到输入焦点状态
	VD_BOOL SetOwner(CItem * pOwner);			//设置拥有者
	CItem * GetOwner();						//得到拥有者
	CItem* GetParent();						//得到所在控件
	CItem* GetBrother();					//得到兄弟控件
	static	CItem* GetActive(int screen);	//得到当前的激活控件
	CItem*	SetActive();					//将当前控件激活
	VD_BOOL	IsActive();						//返回是否为激活控件
	void	GetRect(VD_PRECT pRect) const;
	virtual void	SetRect(VD_PCRECT pRect, VD_BOOL bReDraw = TRUE);
	CDC * GetDC();
	ITEM_TYPE GetType();

	int		GetScreen(CPage* page);		//从页面指针（或者伪指针）得到ID号

	static void DumpInstances();
	void	ScreenToClient(VD_PPOINT pPoint);
	void	ScreenToClient(VD_PRECT pRect);
	void	ClientToScreen(VD_PRECT pRect);
	void	GetClientRect(VD_PRECT pRect);
	void	DrawBackground();

	void    SetRelatedItem(CItem *up = NULL, CItem *down = NULL, CItem *left = NULL, CItem *right = NULL);

	std::string GetName();

	static VD_BOOL VD_DispatchMessage(int screen, uint msg, uint wpa, uint lpa);
	static void CloseAll(int iPageCloseFlag = 0);    //modified by nike.xie 20090903//增加页面判断标志的缺省参数
	static void AdjustRectAll();
	static void SetKeyInputFlag(KEY_INPUT_FLAG index, int flag);
	static int GetKeyInputFlag(KEY_INPUT_FLAG index);

	CItemThread* GetThread();
};
#endif //__ITEM_H__
