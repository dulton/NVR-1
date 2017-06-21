
#ifndef __IMM_H__
#define __IMM_H__


#include "APIs/DVRDEF.H"
#include "System/Object.h"
#include "System/File.h"

typedef struct _HZMB_HEAD{
	int sec_num;		//区段数
	int hz_num;			//汉字数
}HZMB_HEAD;

typedef struct _HZMB_SECTOR{
	char str[8];		//关键字字符串,可以是拼音,注音,笔画等等
	int offset;			//区段内容开始偏移
	int next;			//下一个区段的节点号
}HZMB_SECTOR;

class CIMM : public CObject
{
public:
	CIMM();
	~CIMM();

	VD_BOOL Open(VD_PCSTR table);
	void Close();
	int Filter(VD_PCSTR key);
	VD_PCSTR GetChar(int offset);

private:
	CFile m_FileTable;
	HZMB_HEAD* m_pHead;
	HZMB_SECTOR* m_pSectors;
	char* m_pChars;

	int m_nStartPos;
	int m_nEndPos;
};

#endif //__IMM_H__
