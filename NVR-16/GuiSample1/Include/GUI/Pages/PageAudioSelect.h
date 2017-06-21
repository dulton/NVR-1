#ifndef _PAGE_AUDIO_SELECT_H_
#define _PAGE_AUDIO_SELECT_H_

#include "PageFloat.h"

class CPageAudioSelect:public CPageFloat
{
public:
	CPageAudioSelect(VD_PCRECT pRect,VD_PCSTR psz = NULL,VD_BITMAP* icon = NULL,CPage * pParent = NULL, uint vstyle = 0);
	~CPageAudioSelect();

	virtual VD_BOOL MsgProc(uint msg, uint wpa, uint lpa);
	VD_BOOL SetAudioInfo(u8 chn, u8 vol, u8 mute);
	void SetPreviewAudio();

private:
	int nChMax;
	CStatic* pStatic[6];
	CCheckBox* pCheckBox[2];
	CSliderCtrl* pSlider;
	CComboBox* pComboBox;
	CButton* pButton[2];

	void OnClkButton();
	void OnCheckBox();
	void OnSliderChange();

	u8 m_nIsPreview;

	unsigned char nMute;
	unsigned char nVolume;
	unsigned char nAudioChn;
};

#endif

