
#include "GUI/Pages/PageSysInfo.h"
#include "GUI/Pages/BizData.h"
#include "GUI/Pages/PageInfoFrameWork.h"

char* szSysInfoTitle[MAX_SYS_INFO] =
{
	"&CfgPtn.DeviceName",
	"&CfgPtn.DeviceID",
	"&CfgPtn.DeviceModel",
	"&CfgPtn.HardwareVersion",
	"&CfgPtn.MCUVersion",
	"&CfgPtn.KernelVersion",
	"&CfgPtn.FirmwareVersion",
	"&CfgPtn.LaunchDate",
};

char* szDskInfoTitle[MAX_SYS_INFO] =
{
	"&CfgPtn.ID",
	"&CfgPtn.Type",
	"&CfgPtn.DiskSize",
	"&CfgPtn.Free",
	"&CfgPtn.Status",
	"&CfgPtn.Properties",
	"&CfgPtn.Source",
	"",
	"",
	"&CfgPtn.ID",
	"&CfgPtn.DiskSize",
	"&CfgPtn.Free"
};

CPageSysInfo::CPageSysInfo( VD_PCRECT pRect,VD_PCSTR psz,EMSYSINFOTYPE type,VD_BITMAP* icon /*= NULL*/,CPage * pParent /*= NULL*/ )
:CPageFrame(pRect, psz, icon, pParent)
{
	SetMargin(0,0,0,0);

	m_emSysType = type;

	pFrame = CreateTableBox(CRect(17, 60, m_Rect.Width()-17, m_Rect.Height()-17), this, 1, 1);
	pFrame->SetFrameColor(VD_RGB(56,108,148));

	CRect tableRT(22, 68, m_Rect.Width()-22, m_Rect.Height()-25);
	pTab = CreateTableBox(tableRT, this, 2, MAX_SYS_INFO);
	//pTab->SetColWidth(8,(m_Rect.Width()-44)/2-5);

	int i=0;
	for (i=0; i<MAX_SYS_INFO; i++)
	{
		CRect rt;
		pTab->GetTableRect(0, i, &rt);
		pSysInfo[i] = CreateStatic(CRect(rt.left+tableRT.left+3, rt.top+tableRT.top, rt.right+tableRT.left, rt.bottom+tableRT.top), this, "");
		pSysInfo[i]->SetTextAlign(VD_TA_LEFT|VD_TA_YCENTER);
	}

	for (i=0; i<MAX_SYS_INFO; i++)
	{
		CRect rt;
		pTab->GetTableRect(1, i, &rt);
		pSysInfo[i+MAX_SYS_INFO] = CreateStatic(CRect(rt.left+tableRT.left+3, rt.top+tableRT.top, rt.right+tableRT.left, rt.bottom+tableRT.top), this, "");
		pSysInfo[i+MAX_SYS_INFO]->SetTextAlign(VD_TA_LEFT|VD_TA_YCENTER);
	}
	
	//debug_trace_err("CPageSysInfo");
	
#if 0
	switch(m_emSysType)
	{
		case SIT_SYS:
		{
			GetScrStrSetCurSelDo(
				&pSysInfo[MAX_SYS_INFO],
				GSR_INFO_SYSTEM_BASE, 
				EM_GSR_CTRL_SZLIST, 
				MAX_SYS_INFO
			);
		}
		break;
		case SIT_DSK:
		{
			GetScrStrSetCurSelDo(
				&pSysInfo[MAX_SYS_INFO],
				GSR_DISK_BASE, 
				EM_GSR_CTRL_SZLIST, 
				MAX_SYS_INFO
			);
		}
		break;
	}
#else
	
	//debug_trace_err("CPageSysInfo");
	
	switch(m_emSysType)
	{
	case SIT_SYS:
		{
			for(i=0; i<8; i++)
			{
				pSysInfo[i]->SetText(szSysInfoTitle[i]);
			}
		}
		break;
	case SIT_DSK:
		{
			for (i=0; i<12; i++)
			{
				pSysInfo[i]->SetText(szDskInfoTitle[i]);
			}
		}
		break;
	}
#endif
}

CPageSysInfo::~CPageSysInfo()
{

}

void CPageSysInfo::SetSysInfo(int row, char* szText)
{
	if( row<0 || row>11)
	{
		return;
	}

	pSysInfo[row]->SetText(szText);
}

void CPageSysInfo::SetDiskIdx(uchar nIdx)
{
	m_nDiskIdx = nIdx;
}

VD_BOOL CPageSysInfo::UpdateData( UDM mode )
{
	if(UDM_OPEN == mode)
	{
		switch(m_emSysType)
		{
			case SIT_SYS:
			{
				GetScrStrSetCurSelDo(
					&pSysInfo[MAX_SYS_INFO],
					GSR_INFO_SYSTEM_BASE, 
					EM_GSR_CTRL_SZLIST, 
					MAX_SYS_INFO
				);
			}
			break;
			case SIT_DSK:
			{
				GetScrStrSetCurSelDo(
					&pSysInfo[MAX_SYS_INFO],
					GSR_DISK_BASE, 
					EM_GSR_CTRL_SZLIST, 
					m_nDiskIdx
				);
			}
			break;
		}
	} 
	else if (UDM_CLOSED == mode)
	{
        ((CPageInfoFrameWork *)(this->GetParent()))->ReflushItemName();
    }

	return TRUE;
}

