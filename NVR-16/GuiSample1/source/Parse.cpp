#ifdef WIN32
  #pragma warning( disable : 4786)
#endif

#include <stdlib.h> /* for atol() only */
#include <string.h>
#include "System/Parse.h"


#define W_NEW(var,classname) {var = new classname;}
#define W_DELETE(p) if(NULL != (p)) {delete((p)); (p)=NULL; }
#define W_FREE(p) if(NULL != (p)) {free((p)); (p)=NULL; }
#define EMPTY_STRING ""

/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*/

void CParse::Initialize()
{
	m_iPosition = 0;
	m_vItem.clear();
}

/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*/

CParse::CParse()
{
	m_bTrim = true;
	m_strSpliter = ":";
	Initialize();
}

/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*/

CParse::CParse(
    const std::string &strSrc,
    const std::string &strSpliter)
{
	m_bTrim = true;
	m_strSpliter = strSpliter;

	Parse(strSrc);
}

/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*/

bool CParse::Parse(const std::string &strSrc)
{
	Initialize();

	m_strSrc = strSrc;

	return doParse();
}

/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*/

CParse::~CParse()
{
	// No need, but write here
	m_vItem.clear();
}

/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*/

bool CParse::doParse()
{
	char *buf;
	//char *buf_left; //zlb20111117  去掉部分malloc
	//char *buf_right; //zlb20111117  去掉部分malloc

	if (m_strSrc.empty() || m_strSpliter.empty())
	{
		return false;
	}

	char buf_left[m_strSrc.size() + 1];
	char buf_right[m_strSrc.size() + 1];

	strcpy(buf_right, m_strSrc.c_str());
	buf = buf_right;

	m_vItem.clear();

	while (0
	       == split_str(
	           buf,
	           buf_left,
	           buf_right,
	           m_strSpliter.c_str(),
	           0))
	{
		if (m_bTrim)
		{
			trim_blank(buf_left, 0);
		}

		m_vItem.push_back(buf_left);
		buf = buf_right;
	} // while

	//
	// last word is seperator, we add a item.
	//
	if ( m_strSrc.size() >= m_strSpliter.size()
		&& 0 == strcmp(m_strSrc.c_str()+m_strSrc.size()-m_strSpliter.size(), 
						m_strSpliter.c_str()) )
	{
		m_vItem.push_back("");
	}

	//zlb20111117  去掉部分malloc
	//W_FREE(buf_left);
	//W_FREE(buf_right);

	return true;
}

/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*/

void CParse::setSpliter(const std::string &strSpliter)
{
	m_strSpliter = strSpliter;
}

/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*/

std::string CParse::getSpliter()
{
	return m_strSpliter;
}

/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*/

void CParse::setTrim(bool isTrim )
{
	m_bTrim = isTrim;
}

/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*/

std::string CParse::getWord(int iPos)
{
	if (iPos >= 0)
	{
		m_iPosition = iPos+1;
	}
	else
	{
		m_iPosition++;
	}

	if (m_iPosition > m_vItem.size())
	{
		return EMPTY_STRING;
	}

	return m_vItem[m_iPosition-1];
}

/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*/

long CParse::getValue(int iPos)
{
	return atol(getWord(iPos).c_str());
}

/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*/

int CParse::Size()
{
	return m_vItem.size();
}

/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*/
