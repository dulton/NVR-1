#ifndef _PAGE_SENSORALARM_COMMON_H_
#define _PAGE_SENSORALARM_COMMON_H_

struct SENSEOR_BASEITEM
{
	CStatic* pChnIndex;
	CCheckBox* pChkEnable;
	CComboBox* pComboType;
	CEdit* pEditName;
};

struct SENSEOR_DEALITEM
{
	CStatic* pChnIndex;
	CComboBox* pComboDelay;
	CButton* pBtDeal;
};

typedef struct 
{
	char bEnable;
	int  nAlarmType;
	char szName[32];
	int  nDelayTime;
} SSensorAlarmPara;


#endif
