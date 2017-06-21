//
//  "$Id: IPAddressctrl.h 117 2008-11-27 09:44:43Z liwj $"
//
//  Copyright (c)2008-2010, RealVision Technology CO., LTD.
//  All Rights Reserved.
//
//	Description:	
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//


#ifndef __CTRL_IPADDRESS_H__
#define __CTRL_IPADDRESS_H__

typedef enum{
	IPB_NORMAL,
	IPB_SELECTED,
	IPB_PUSHED,
	IPB_DISABLED,
}IPAddressCtrlBitmap;

class CIPAddressCtrl : public CItem
{
	CNumberBox *pnb[4];
	IPADDR ipa;
protected:
	static VD_BITMAP* bmp_ipb_normal;
	static VD_BITMAP* bmp_ipb_selected ;
	static VD_BITMAP* bmp_ipb_pushed;
	static VD_BITMAP* bmp_ipb_disabled;
	VD_COLORREF 	m_BkColor;
	VD_COLORREF 	m_ColorFrameSelect;
	VD_COLORREF 	m_ColorFrameNormal;
public:
	CIPAddressCtrl(VD_PCRECT pRect, CPage * pParent,uint style = 0);
	virtual ~CIPAddressCtrl();

	void Draw();
	void SetIPAddress(IPADDR *p);
	void GetIPAddress(IPADDR *p);
	void SetBkColor(VD_COLORREF color, VD_BOOL redraw = FALSE );
	void SetSerialNumber(SERIALNUMBER *p);
	void GetSerialNumber(SERIALNUMBER *p);
	void Select(VD_BOOL flag);
	void Enable(VD_BOOL flag);
	VD_SIZE& GetDefaultSize(void);
	void Show(VD_BOOL flag);
	virtual void Show(VD_BOOL flag, VD_BOOL redraw = TRUE);	

	static void SetDefaultBitmap(IPAddressCtrlBitmap region, VD_PCSTR name);
	
	void setSplit(const std::string &spliter);
	void setMaxNumber(int iMaxNumber);

	void setSerialFormat(VD_PCSTR pcFmt);

private:
	std::string m_spliter;
};
CIPAddressCtrl* CreateIPAddressCtrl(VD_PCRECT pRect, CPage * pParent,uint style = 0);


#endif //__CTRL_IPADDRESS_H__
