#ifndef __APPS_MAIN_H__
#define __APPS_MAIN_H__

#include "System/Object.h"
#include "config-x.h"
#include "MultiTask/Timer.h"

class CPageStart; 

class CChallenger : public CObject
{
public:
	PATTERN_SINGLETON_DECLARE(CChallenger);
	
	CChallenger();
	~CChallenger();
	
	void Initialize(int argc, char * argv[]);
	
private:
	//CPageStart *pPageStart;
};

#define g_Challenger (*CChallenger::instance())

#endif

