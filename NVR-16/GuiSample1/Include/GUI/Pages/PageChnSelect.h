#ifndef _PAGE_CHN_SELECT_H_
#define _PAGE_CHN_SELECT_H_

#include "PageFloat.h"
#include "GUI/Pages/PageMessageBox.h"

class CPageSearch;
class CPageChnSelect:public CPageFloat
{
public:
	CPageChnSelect(VD_PCRECT pRect,VD_PCSTR psz = NULL,VD_BITMAP* icon = NULL,CPage * pParent = NULL, uint vstyle = 0, uchar nChnNum = 0);
	~CPageChnSelect();
	
	void SetDesktop(CPage* pDesktop);
	VD_BOOL UpdateData( UDM mode );
	void GetSel(int* pStyle, int* pChMask);
	VD_BOOL MsgProc( uint msg, uint wpa, uint lpa );  //cw_panel
	void SetCurStatusPlaying(u8 flag);
	void SetCurStatusElecZoom(u8 flag);//电子防大
	void SetPbCurStatusElecZoom(u8 flag);
	void SetCurStatusSearching(u8 flag);
	void SetCurStatusRecorDing(u8 flag);
	void SetSearchPage(CPageSearch* page);
	void CheckVLineCheckBox(int nLine); //检查同一水平checkbox是否都全选中
	
	void StartManuelRec();
	void StartPbElecZoom();
	void StartPreviewSelect();
	void StartPreviewElecZoom();
	
	int  SetCurDwellStatue(int flag);
	int  GetCurDwellStatue();
	
private:
	int nChMax;
	int curSel;
	int m_nLines;
	int m_nCheckBox;
	u8 m_nIsPlaying;
	u8 m_nIsSearching;
	u8 m_nIsElecZoom;
	u8 m_nIsPbElecZoom;
	u8 m_nIsDwell;
	u8 m_nIsRecorDing;
	uint m_vstyle; /*= 0 single, =1 4 channels in row*/
	
	CPage* m_pDesktop;
	CPageSearch* m_pSearchPage;
	//CPageElectronicZoom* m_pElectronicZoom;
	
	CStatic** pStatic;
	CCheckBox** pCheckBox;
	CStatic* pStaticAll;
	CCheckBox* pCheckBoxAll;
	CButton* pButton[2];
	
	void OnClkButton();
	void OnCheckSel();
	void OnHeadCheckSel();
	void OnCheckAllSel();
};

#endif

