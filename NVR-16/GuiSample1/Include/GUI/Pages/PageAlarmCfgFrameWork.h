#ifndef _PAGE_ALARMCFG_FRAMEWORK_H_
#define _PAGE_ALARMCFG_FRAMEWORK_H_

#include "PageFrame.h"


//#define MAX_ALARM_PAGES 5//6
//yaogang modify 20141013
#define MAX_ALARM_PAGES 9


class CPageSensorAlarm;
class CPageIPCameraExtSensorAlarm;
class CPageMDCfg;
class CPageVideoLoss;
class CPageOtherAlarm;
class CPageAlarmOut;
class CPageAlarmDeal;

class CPageAlarmCfgFrameWork :public CPageFrame
{
public:
	CPageAlarmCfgFrameWork(VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon = NULL,CPage * pParent = NULL);
	~CPageAlarmCfgFrameWork();

	void ReflushItemName();
	VD_BOOL UpdateData(UDM mode);


private:
	void OnClkItem();
	CStatic* pStatic[MAX_ALARM_PAGES];
	CButton* pButton[MAX_ALARM_PAGES];
	CStatic* pStaticPic;

	CPageSensorAlarm* m_pPageSenser;
	CPageIPCameraExtSensorAlarm* m_pPageIPCCover;
	CPageIPCameraExtSensorAlarm* m_pPageIPCExtSensor;
	CPageIPCameraExtSensorAlarm* m_pPage485ExtSensor;
	CPageAlarmDeal* m_pPageHDD;
	CPageMDCfg* m_pPageMDCfg;
	CPageVideoLoss* m_pPageVideoLoss;
	CPageOtherAlarm* m_pPageOtherAlarm;
	CPageAlarmOut* m_pPageAlarmOut;

};

#endif
