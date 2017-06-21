//
//  "$Id: BitmapButton.h 117 2008-11-27 09:44:43Z liwj $"
//
//  Copyright (c)2008-2010, RealVision Technology CO., LTD.
//  All Rights Reserved.
//
//	Description:	
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#ifndef __CTRL_BMPBUTTON_H__
#define __CTRL_BMPBUTTON_H__


class CBitmapButton : public CItem
{
	//std::string	string;	//控件字符串
	VD_BOOL	bdown;	//按钮按下
	VD_BITMAP*	bmp1;
	VD_BITMAP*	bmp2;
	VD_BITMAP*	bmp3;
	CPage * child;

public:
	CBitmapButton(VD_PCRECT pRect, CPage * pParent, VD_PCSTR psz,VD_BITMAP* res1,VD_BITMAP* res2,VD_BITMAP* res3,CPage *pPage,uint style = 0);
	virtual ~CBitmapButton();
	void Draw();
	VD_BOOL MsgProc(uint msg, uint wpa, uint lpa);
	VD_PCSTR GetText();
	VD_SIZE& GetDefaultSize(void);
};
CBitmapButton* CreateBitmapButton(VD_PCRECT pRect, 
								 CPage * pParent, 
								 VD_PCSTR psz,
								 VD_BITMAP* res1,
								 VD_BITMAP* res2,
								 VD_BITMAP* res3,
								 CPage *pPage,
								 uint style = 0);

#endif //__CTRL_BMPBUTTON_H__

