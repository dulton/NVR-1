//
//  "$Id: ComboBox.h 117 2008-11-27 09:44:43Z liwj $"
//
//  Copyright (c)2008-2010, RealVision Technology CO., LTD.
//  All Rights Reserved.
//
//	Description:	
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#ifndef __CTRL_COMBOX_H__
#define __CTRL_COMBOX_H__

#if defined(WIN32)
	#pragma warning (disable : 4786)
#endif
#include <vector>

#define COMBO_MAX 256 // 128

enum
{
	comboNoButton	= 0x0001,
};

struct COMBO_ITEM
{
	VD_BOOL valid;				//是否有效
	std::string string;		//字符串
	uint data;				//用户数据
};

typedef enum{
	CBB_NORMAL,
	CBB_SELECTED,
	CBB_PUSHED,
	CBB_DISABLED,
}ComboBoxBitmap;

typedef std::vector<COMBO_ITEM> COMBO_ITEM_VECTOR;

enum COLOR_TYPE
{
	COLOR_BK,
	COLOR_TEXT,
};
class CListBox;
class CComboBox : public CItem
{
	int			num;
	int			cursel;		//当前项
	int			newsel;	//临时当前值
	COMBO_ITEM_VECTOR	items;  //组合项
	CListBox*		listbox;	//下拉列表框
	VD_BOOL		reserve;//保留弹出的列表
	VD_SIZE		m_DropDownSize;
	int 			width;
	CTRLPROC	m_vProc;
	CTRLPROC	m_vProc1;
	//int 		isListExist;

public:
	static VD_BITMAP* m_bmpNormal;
	static VD_BITMAP* m_bmpSelected;
	static VD_BITMAP* m_bmpPushed;
	static VD_BITMAP* m_bmpDisabled;
	VD_COLORREF    m_BkColor;  
	VD_BOOL isListExist;
public:
	CComboBox(VD_PCRECT pRect, CPage * pParent, VD_PCSTR psz, CTRLPROC vproc = NULL,CTRLPROC vproc1 = NULL, uint dwStyle = 0);
	virtual ~CComboBox();
	
	void Draw();
	VD_BOOL MsgProc(uint msg, uint wpa, uint lpa);
	void Select(VD_BOOL flag);
	void SetFocus(VD_BOOL flag);
	void SetBkColor(VD_COLORREF color, VD_BOOL redraw = FALSE);
	int GetCurSel();
	void SetCurSel(int sel);
	void InitStrings(VD_PCSTR psz);
	/*! bindingNum= -1表明不用设置数据项和一个数据的绑定关系*/
	int AddString(VD_PCSTR string, VD_BOOL redraw = FALSE,int bindingNum = -1);
	char * GetString(int sel = -1);
	void DeleteString(VD_BOOL redraw = FALSE);
	void SetString(VD_PCSTR string, VD_BOOL redraw = FALSE);
	void RemoveAll();
	void SetMask(uint mask);
	void ShowDropDown(VD_BOOL show);
	void SetDropDownSize(VD_PSIZE pSize);
	VD_BOOL SetItemData(int index, uint data);
	uint GetItemData(int index);
	int IndexConvert(int index, VD_BOOL bValid);
	int GetCount();
	int GetListBoxStatus();
	VD_SIZE& GetDefaultSize(void);
	
	static void SetDefaultBitmap(ComboBoxBitmap region, VD_PCSTR name);
};
CComboBox* CreateComboBox(VD_PCRECT pRect, CPage * pParent, VD_PCSTR psz, CTRLPROC vproc = NULL,CTRLPROC vproc1 = NULL, uint dwStyle = 0);


#endif //__CTRL_COMBOX_H__
