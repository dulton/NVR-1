//  #include "/home/zhang_yaozhen/usr/include/semaphore.h"
//#include "/usr/include/semaphore.h"
//#include "semaphore.h.bak"
#ifndef	WIN32
#include <semaphore.h>
#endif


#include "APIs/Semaphore.h"
#include "APIs/DVRDEF.H"
// #include "dvr_inc.h"
// #include "dvr_dev.h"
// #include "dvr_debug.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

int SemaphoreCreate(HANDLE *semId, DWORD cnt)
{
	sem_t *	sem;
	
	sem = (sem_t *)malloc(sizeof(sem_t));
	if (sem == NULL)
	{
		ERR_PRINT("SemaphoreCreate Malloc Fail\n");
		return -1;
	}
	if (sem_init(sem, 0, cnt) == 0)
	{
		*semId = (HANDLE)sem;
		return 0;
	}
	
	ERR_PRINT("Sem_init Fail\n");
	return -1;
} 


int SemaphoreDestory(HANDLE semId)
{
	if (sem_destroy((sem_t *)semId) == -1)
	{
		perror("sem destroy");
		return -errno;
	}
	else
	{
		free((void*)semId);
		return 0;
	}
} 


int SemaphorePost(HANDLE semId)
{
  return sem_post((sem_t *)semId);
} 


int SemaphorePend(HANDLE semId)
{
  return sem_wait((sem_t *)semId);
} 

