#ifndef _PAGE_COLORSETUP_H_
#define _PAGE_COLORSETUP_H_

#include "PageFloat.h"
#include "GUI/Pages/PageDesktop.h"

#define	R9504SSTDVALUE	40
#define DVRSTDVALUE		128

class CPageColorSetup:public CPageFloat
{
public:
	CPageColorSetup(VD_PCRECT pRect,VD_PCSTR psz = NULL,VD_BITMAP* icon = NULL,CPage * pParent = NULL, uint vstyle = 0);
	~CPageColorSetup();
	VD_BOOL UpdateData( UDM mode );
	
	void SetColorChn(int chn);
	void SetStartPageFlag(u8 flag);
	
    VD_BOOL MsgProc( uint msg, uint wpa, uint lpa );

private:

	CStatic* pStatic[9];
	CSliderCtrl* pSlider[4];
	CButton* pButton[11];
	CComboBox* pComboBox;
	EMBIZPREVIEWMODE eMaxViewMode;
	BOOL	IsDwell;
	BOOL	IsR9504S;
	BOOL	IsMaxPre;
	u8 		isStartPage;

	int m_chn;
	int flag_all;
	int nChMax;

	int m_bright;
	int m_chroma;
	int m_saturation;
	int m_contrast;

	void OnClkButton();
	void OnSliderChange();
	void OnComboBox();

};

#endif

