//
//  "$Id: CheckBox.h 117 2008-11-27 09:44:43Z liwj $"
//
//  Copyright (c)2008-2010, RealVision Technology CO., LTD.
//  All Rights Reserved.
//
//	Description:	
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//


#ifndef __CTRL_CHECKBOX_H__
#define __CTRL_CHECKBOX_H__

typedef enum{
	CKB_NORMAL_TRUE,
	CKB_PUSHED_TRUE,
	CKB_SELECTED_TRUE,
	CKB_DISABLED_TRUE,
	CKB_NORMAL_FALSE,
	CKB_PUSHED_FALSE,
	CKB_SELECTED_FALSE,
	CKB_DISABLED_FALSE,
}CheckBoxBitmap;

class CCheckBox : public CItem
{
	int value;
	int index;
	//std::string	string;	//¿Ø¼þ×Ö·û´®
	CTRLPROC m_vProc;
	static VD_BITMAP* m_bmpNormalTrue;
	static VD_BITMAP* m_bmpPushedTrue;
	static VD_BITMAP* m_bmpSelectedTrue;
	static VD_BITMAP* m_bmpDisabledTrue;
	static VD_BITMAP* m_bmpNormalFalse;
	static VD_BITMAP* m_bmpSelectedFalse;
	static VD_BITMAP* m_bmpPushedFalse;
	static VD_BITMAP* m_bmpDisabledFalse;

public:
	
	CCheckBox(VD_PCRECT pRect, CPage * pParent,uint style = styleEditable, CTRLPROC vproc = NULL, int vindex = -1);
	virtual ~CCheckBox();

	void Draw();
	VD_BOOL MsgProc(uint msg, uint wpa, uint lpa);
	void SetValue(int vvalue);
	int GetValue();
	void SetText(VD_PCSTR vstring);
	VD_PCSTR GetText();
	VD_SIZE& GetDefaultSize(void);

	static void SetDefaultBitmap(CheckBoxBitmap region, VD_PCSTR name);

};
CCheckBox* CreateCheckBox(VD_PCRECT pRect, CPage * pParent,uint style = styleEditable, CTRLPROC vproc = NULL, int vindex = -1);

#endif //__CTRL_CHECKBOX_H__
