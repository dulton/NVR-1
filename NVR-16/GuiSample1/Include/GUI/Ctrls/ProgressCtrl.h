//
//  "$Id: ProgressCtrl.h 117 2008-11-27 09:44:43Z liwj $"
//
//  Copyright (c)2008-2010, RealVision Technology CO., LTD.
//  All Rights Reserved.
//
//	Description:	
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#ifndef __CTRL_PROGRESS_H__
#define __CTRL_PROGRESS_H__

class CProgressCtrl : public CItem
{
	int min;
	int max;
	int curpos;
	char text[16];
public:
	CProgressCtrl(VD_PCRECT pRect, CPage * pParent,int vmin = 0,int vmax = 100,uint style = 0);
	virtual ~CProgressCtrl();
	void Draw();
	void SetRange(int vmin, int vmax);
	void SetPos(int newpos);
	int GetPos();
	void StepIt(int step);
	VD_SIZE& GetDefaultSize(void);
};
CProgressCtrl* CreateProgressCtrl(VD_PCRECT pRect, CPage * pParent,int vmin = 0,int vmax = 100,uint style = 0);


#endif //__CTRL_PROGRESS_H__
