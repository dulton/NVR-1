
#ifndef _PARSE_H
#define _PARSE_H

#include <string>
#include <vector>

#include "ez_libs/ezutil/str_opr.h"

class CParse
{
public:
	CParse();
	CParse(
	    const std::string &strSrc,
	    const std::string &strSpliter = ":");

	virtual ~CParse();

	void Initialize();

	// set the split string what you want
	void setSpliter(const std::string &strSpliter=":");
	std::string getSpliter();

	// trim item or not
	void setTrim(bool isTrim = true);

	// parse a new string
	bool Parse(const std::string &strSrc);

	// when iPos<0 get word one by one
	std::string getWord(int iPos=-1);

	// when iPos<0 get Value one by one
	long getValue(int iPos=-1);

	// How many items have been parsed
	int Size();
	/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*/
private:

	bool doParse();

	/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*/
	std::string m_strSrc;
	std::string m_strSpliter;
	bool m_bTrim;
	size_t m_iPosition;
	std::vector<std::string> m_vItem;
};

/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-*/

#endif // _PARSE_H
