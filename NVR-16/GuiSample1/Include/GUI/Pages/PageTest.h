#ifndef _PAGE_TEST_H_
#define _PAGE_TEST_H_

#include "PageFrame.h"
//#include "PageChild.h"
#include <vector>



class CPageTest :public CPageFrame
{
public:
	CPageTest(VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon = NULL,CPage * pParent = NULL);
	~CPageTest();

	VD_PCSTR GetDefualtTitleCenter();

protected:


private:


};

#endif //_PAGE_TEST_H_



