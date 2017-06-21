

#ifndef __PAGE_FRAME_H__
#define __PAGE_FRAME_H__

//#include "PageDefs.h"
#include "GUI/Ctrls/Page.h"



typedef struct tagMAINFRAME_ITEM
{
	std::vector<CItem*> SubItems[2];
	int			cursubindex;
}MAINFRAME_ITEM;

#define pageMainFrame_w 642//cw_page
#define pageMainFrame_h 418

#define pageLogin_w 320
#define pageLogin_h 250

//csp modify
//#define Messagebox_h 480
//#define Messagebox_w 300
#define Messagebox_w 480
#define Messagebox_h 300

class CPageFrame : public CPage
{
public:
	CPageFrame(VD_PCRECT pRect,VD_PCSTR psz = NULL,VD_BITMAP* icon = NULL,CPage * pParent = NULL, uint vstyle = 0);
	~CPageFrame();
	
	virtual void Draw();
	void SetRect(VD_PCRECT pRect, VD_BOOL bReDraw = TRUE);
	CButton* GetCloseButton();
	virtual void DrawTitle_left();
	
	virtual void DrawTitle_center(const char* string);
	
	virtual void DrawTip(const char *pcTip); //add by xym GUI //显示主菜单每项提示信息
	
	virtual VD_BOOL MsgProc(uint msg, uint wpa, uint lpa);
	
	void OnClose();
	
protected:
	BOOL m_bPlayBackHide;
	BOOL m_bPlayBackPage;
	//BOOL m_bStartPage;
	
private:
//	CButton	*m_pButtonClose;
	CButton* pCloseButton;
};

#endif //__PAGE_FRAME_H__

