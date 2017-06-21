//
//  "$Id: Button.h 117 2008-11-27 09:44:43Z liwj $"
//
//  Copyright (c)2008-2010, RealVision Technology CO., LTD.
//  All Rights Reserved.
//
//	Description:	
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#ifndef __CTRL_BUTTON_H__
#define __CTRL_BUTTON_H__

typedef enum {
	ID_EMPTY,
		ID_OK,
		ID_CANCEL,
		ID_DEFAULT,
		ID_COPY,
		ID_PASTE,
}VD_IID;

typedef enum
{
	buttonNormalBmp 	= 0x0001,//简单的bmp，通过调用setbmp接口设置的属性
	buttonFullBmpRect   = 0x0002,//贴一张整体的button图，程序员不需要设置，代码自己设置。added by wangqin 200704-25
}BUTTONSTYLE;

typedef enum{
	BTB_NORMAL,
	BTB_SELECTED,
	BTB_PUSHED,
	BTB_DISABLED,
}ButtonBitmap;

class CButton;
typedef std::deque<CButton *> PGROUPITEMQUEUE;
class CButton : public CItem
{
	//std::string	string;	//控件字符串
	VD_BOOL	m_bDown;	//按钮按下
	VD_RECT	rect_normal;
	VD_RECT	rect_selected;
	VD_RECT	rect_disabled;
	VD_RECT	rect_pushed;
	VD_BITMAP * m_pBmpNormal;
	VD_BITMAP * m_pBmpSelect;
	VD_BITMAP * m_pBmpDisable;
	VD_BITMAP * m_pBmpPush;
	VD_BOOL m_bWithBmp;
	int m_iBkMode;
	//BOOL m_bBmpIntergrate;
	VD_IID		id;		//控件id
	CPage * child;
	//int	m_iGroupID;
	//static PGROUPITEMQUEUE m_Queue;
	CTRLPROC	m_onClick;
	CTRLPROC	m_onChanged;
	VD_COLORREF m_TextColor;
	BOOL		m_bPage;
	
	static VD_BITMAP * m_bmpDefNormal;
	static VD_BITMAP * m_bmpDefSelected;
	static VD_BITMAP * m_bmpDefDisabled;
	static VD_BITMAP * m_bmpDefPushed;

public:
	CButton(VD_PCRECT pRect, CPage * pParent, VD_PCSTR psz,CTRLPROC onClicked,CTRLPROC onChanged = NULL, uint style =0, BOOL bPage = FALSE);
	//CButton(PCRECT pRect, CPage * pParent, PCSTR psz,CTRLPROC onClicked, int iGroupID,uint style=0 );
	CButton(VD_PCRECT pRect, CPage * pParent, VD_PCSTR psz,VD_IID vid,uint style = 0, BOOL bPage = FALSE);
	CButton(VD_PCRECT pRect, CPage * pParent, VD_PCSTR psz,CPage *pPage,uint style = 0, BOOL bPage = FALSE);
	virtual ~CButton();

	void SetTextColor(VD_COLORREF color, VD_BOOL redraw = TRUE);

	void Init();

	VD_SIZE& GetDefaultSize(void);
	void Draw();
	VD_BOOL MsgProc(uint msg, uint wpa, uint lpa);
	void SetText(VD_PCSTR psz);
	VD_PCSTR GetText();
	//void SetGroupButtonState(int iState);
	void SetBitmap(VD_BITMAP* pBmpNormal, VD_PCRECT pRectNormal, VD_PCRECT pRectSelected, VD_PCRECT pRectDisabled, VD_PCRECT pRectPushed, VD_BOOL bTransparent = FALSE/*TRUE*/);
	void SetBitmap(VD_BITMAP* pBmpNormal, VD_BITMAP* pBmpSelect = NULL, VD_BITMAP* pBmpPush = NULL, VD_BITMAP* pBmpDisable = NULL, VD_BOOL bTransparent = FALSE/*TRUE*/);
	VD_BOOL GetState();
	void DrawCtrlButton(CTRL_STATE state = ctrlStateNormal);

	static void SetDefaultBitmap(ButtonBitmap region, VD_PCSTR name);
};
CButton* CreateButton(VD_PCRECT pRect, CPage * pParent, VD_PCSTR psz,CTRLPROC onClicked,
							 CTRLPROC onChanged = NULL, uint style = 0, BOOL bPage = FALSE);
//CButton* CreateButton(PCRECT pRect, CPage * pParent, PCSTR psz,CTRLPROC onClicked,int iGroupID,uint style = 0);
CButton* CreateButton(VD_PCRECT pRect, CPage * pParent, VD_PCSTR psz,VD_IID vid,uint style = 0, BOOL bPage = FALSE);
CButton* CreateButton(VD_PCRECT pRect, CPage * pParent, VD_PCSTR psz,CPage *pPage,uint style = 0, BOOL bPage = FALSE);


#endif //__CTRL_BUTTON_H__
