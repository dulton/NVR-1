#ifndef _PAGE_SPLITFRAMEWORK_H_
#define _PAGE_SPLITFRAMEWORK_H_

#include "PageFrame.h"
//#include "PageChild.h"
#include <vector>

#define MAX_MAIN_ITEM	6
#define MAX_SUB_PER_MAIN	5

typedef struct tagMAINFRAME_ITEM{
	VD_BITMAP*		bmp1;
	VD_BITMAP*		bmp2;
	std::string	string;
	std::string	tip;
	CButton*	pBtMainItem;
	CButton*	pBtSubItem[MAX_SUB_PER_MAIN];
	std::vector<CItem*> SubItems[MAX_SUB_PER_MAIN];
	int			cursubindex;
}MAINFRAME_ITEM;

class CPageSplitFrameWork :public CPageFrame
{
public:
	CPageSplitFrameWork(VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon = NULL,CPage * pParent = NULL);
	~CPageSplitFrameWork();

	VD_PCSTR GetDefualtTitleCenter();

protected:


private:

	int curMainItemSel;

	int totalItemNum;


	void OnClickMainItem();
	void OnClickSubItem();

	void SwitchPage(int main, int sub);

	void ShowSubPage(int main, int sub, BOOL bShow);


};

#endif //_PAGE_SPLITFRAMEWORK_H_

