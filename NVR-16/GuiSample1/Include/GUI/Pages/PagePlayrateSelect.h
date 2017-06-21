#ifndef _PAGE_PLAYRATE_SELECT_H_
#define _PAGE_PLAYRATE_SELECT_H_

#include "PageFloat.h"


class CPagePlayrateSelect:public CPageFloat
{
public:
	CPagePlayrateSelect(VD_PCRECT pRect,VD_PCSTR psz = NULL,VD_BITMAP* icon = NULL,CPage * pParent = NULL, uint vstyle = 0, int type = 0);//type: 1  forward rate  ; type: 0 backward rate
	~CPagePlayrateSelect();

private:
	int m_type;
	
	CListBox*  pListBox;

	void OnClick();
	void OnEnter();
	void OnChange();

};

#endif

