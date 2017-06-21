//
//  "$Id: GroupBox.h 117 2008-11-27 09:44:43Z liwj $"
//
//  Copyright (c)2008-2010, RealVision Technology CO., LTD.
//  All Rights Reserved.
//
//	Description:	
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//


#ifndef __CTRL_GROUPBOX_H__
#define __CTRL_GROUPBOX_H__

typedef enum{
	GBB_TAB,
}GroupBoxBitmap;

class CGroupBox : public CItem
{
protected:
	static VD_BITMAP*	bmp_tab; 
	//std::string	string;	//¿Ø¼þ×Ö·û´®
	
public:
	CGroupBox(VD_PCRECT pRect, CPage * pParent, VD_PCSTR psz,uint style = 0);
	virtual ~CGroupBox();


	void Draw();
	void SetText(VD_PCSTR psz);
	VD_PCSTR GetText(void);
	VD_SIZE& GetDefaultSize(void);

	static void SetDefaultBitmap(GroupBoxBitmap region, VD_PCSTR name);
};
CGroupBox* CreateGroupBox(VD_PCRECT pRect, CPage * pParent, VD_PCSTR psz,uint style = 0);
#endif //__CTRL_GROUPBOX_H__
