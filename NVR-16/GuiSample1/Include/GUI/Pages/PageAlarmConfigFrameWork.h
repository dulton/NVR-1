#ifndef _PAGE_ALARMCONFIGFRAMEWORK_H_
#define _PAGE_ALARMCONFIGFRAMEWORK_H_

#include "PageFrame.h"
//#include "PageChild.h"
#include <vector>

#define ALARMCFGMENU_ITEMS	6

/*
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
*/

class CPageSensorAlarmFrameWork;
class CPageAlarmConfigFrameWork :public CPageFrame
{
public:
	CPageAlarmConfigFrameWork(VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon = NULL,CPage * pParent = NULL);
	~CPageAlarmConfigFrameWork();

	VD_PCSTR GetDefualtTitleCenter();

protected:


private:

	int curMainItemSel;
	int totalItemNum;

	void OnClkAlarmCfgItem();

	//void OnTest();


	CStatic* pStatic[ALARMCFGMENU_ITEMS];
	CButton* pButton[ALARMCFGMENU_ITEMS];
	CStatic* pStaticPic;

	//CPageRecordFrameWork* m_pPageRecord;
	//CPageRecordScheduleFrameWork* m_pPageRecordSchedule;
	CPageSensorAlarmFrameWork* m_pPageSensorAlarm;

};

#endif //_PAGE_ALARMCONFIGFRAMEWORK_H_


