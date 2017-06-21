#ifndef _PAGE_PLAYBACKVOCOLORSETUP_H_
#define _PAGE_PLAYBACKVOCOLORSETUP_H_

#include "PageFloat.h"

class CPagePlayBackVoColorSetup:public CPageFloat
{
public:
	CPagePlayBackVoColorSetup(VD_PCRECT pRect,VD_PCSTR psz = NULL,VD_BITMAP* icon = NULL,CPage * pParent = NULL, uint vstyle = 0);
	~CPagePlayBackVoColorSetup();
	VD_BOOL UpdateData( UDM mode );
	void SetColorChn(int chn);
    VD_BOOL MsgProc( uint msg, uint wpa, uint lpa );
	void SetPlayBackColor();
	void SetDefaultVoImageParam();

private:
	CStatic* pStatic[9];
	//CStatic* pDrawLine;//csp modify
	CSliderCtrl* pSlider[4];
	CButton* pButton[11];
	CComboBox* pComboBox;
	EMBIZPREVIEWMODE eMaxViewMode;

	vo_image_para_t vo_image_color;

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

