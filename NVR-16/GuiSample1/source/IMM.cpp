
#include "GUI/GDI/IMM.h"

CIMM::CIMM()
{
	m_pHead = NULL;
	m_pSectors = NULL;
	m_pChars = NULL;
	m_nStartPos = -1;
	m_nEndPos = -1;
}

CIMM::~CIMM()
{
}

VD_BOOL CIMM::Open(VD_PCSTR table)
{
	uchar *pBuffer;

	if(!(pBuffer = m_FileTable.Load(table)))
	{
		return FALSE;
	}

	if(m_FileTable.GetLength() < sizeof(HZMB_HEAD))
	{
		m_FileTable.UnLoad();
		return FALSE;
	}

	m_pHead = (HZMB_HEAD *)pBuffer;
	if(m_FileTable.GetLength() != (sizeof(HZMB_HEAD) + m_pHead->sec_num * sizeof(HZMB_SECTOR)
		+ m_pHead->hz_num * 2))
	{
		m_FileTable.UnLoad();
		return FALSE;
	}

	m_pSectors = (HZMB_SECTOR *)(pBuffer + sizeof(HZMB_HEAD));
	m_pChars = (char *)(pBuffer + sizeof(HZMB_HEAD) + m_pHead->sec_num * sizeof(HZMB_SECTOR));

	return TRUE;
}

void CIMM::Close()
{
	m_FileTable.UnLoad();
	m_pHead = NULL;
	m_pSectors = NULL;
	m_pChars = NULL;
}

int CIMM::Filter(VD_PCSTR key)
{
	if(!m_pHead || !m_pChars || !m_pSectors)
	{
		return 0;
	}

	int level = 0;
	const char *p = key;
	HZMB_SECTOR *pCurSec = m_pSectors;
	HZMB_SECTOR *pChildEndSec = m_pSectors + m_pHead->sec_num;

	m_nStartPos = -1;
	m_nEndPos = m_pHead->hz_num;

	while(*p)
	{
		while(pCurSec && (pCurSec->str[level] != *p))
		{
			if(pCurSec->next)
			{
				pCurSec = m_pSectors + pCurSec->next;
			}
			else
			{
				pCurSec = NULL;
			}
		}
		if(pCurSec == NULL)
		{
			m_nStartPos = -1;
			m_nEndPos = -1;
			break;
		}
		m_nStartPos = pCurSec->offset;
		if(pCurSec->next)
		{
			pChildEndSec = m_pSectors + pCurSec->next;
			m_nEndPos =  pChildEndSec->offset;
		}
		level++;
		p++;
		if(pCurSec < pChildEndSec)
		{
			pCurSec++;
		}
		else
		{
			m_nStartPos = -1;
			m_nEndPos = -1;
			break;
		}
	}
	
	if(m_nStartPos >= 0 && m_nEndPos >= m_nStartPos)
	{
		return m_nEndPos - m_nStartPos;
	}
	return 0;
}

VD_PCSTR CIMM::GetChar(int offset)
{
	if(!m_pHead || !m_pChars || !m_pSectors)
	{
		return NULL;
	}

	if((m_nStartPos >= 0) && (m_nEndPos >= m_nStartPos) && (offset >= 0) && (offset < m_nEndPos - m_nStartPos))
	{
		return m_pChars + (m_nStartPos + offset) * 2;
	}

	return NULL;
}
