#include <semaphore.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "utils.h"

#include "custommp4.h"
#include "diskmanage.h"
#include "partitionindex.h"

#include "mod_log.h"
#include "circlebuf.h"

#include <sys/ioctl.h>
#include <pthread.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>

//#include "biz_types.h"

#define FILEHEAD	0x1234ABCD

#define MODLOGNUM	1024//512//4096//64		//每个日志文件的条数
#define MAXNUM		4//8//24//4//256		//总共多少个日志文件
#define MSGLEN		64//128//256//512		//一条日志信息的长度

void print_info(ModLog *pLogManager)
{
	printf("\nprint_info:(0x%08x %d %d %d %d)\n",pLogManager->index_head.magic,pLogManager->index_head.opNums,pLogManager->index_head.index,pLogManager->index_head.isAgain,pLogManager->current_num);
}

BOOL is_log_index_valid(FileHeadInfo *pInfo, unsigned int nMaxNum)
{
	if(pInfo == NULL)
	{
		return FALSE;
	}
	if(pInfo->magic != FILEHEAD)
	{
		return FALSE;
	}
	if(pInfo->index >= nMaxNum)
	{
		return FALSE;
	}
	if(pInfo->isAgain > 1)
	{
		return FALSE;
	}
	if(pInfo->verify != pInfo->opNums + pInfo->index + pInfo->isAgain + pInfo->reserved1 + pInfo->reserved2 + pInfo->reserved3)
	{
		return FALSE;
	}
	return TRUE;
}

static int reset_log_index_head(FileHeadInfo *pInfo)
{
	if(pInfo == NULL)
	{
		return -1;
	}
	memset(pInfo, 0, sizeof(*pInfo));
	pInfo->magic = 0x1234ABCD;
	pInfo->index = 0;
	pInfo->isAgain = 0;
	pInfo->verify = pInfo->opNums + pInfo->index + pInfo->isAgain + pInfo->reserved1 + pInfo->reserved2 + pInfo->reserved3;
	return 0;
}

int update_log_index_head(int fd, FileHeadInfo *pInfo)
{
	if(fd < 0)
	{
		return -1;
	}
	
	if(pInfo == NULL)
	{
		return -1;
	}
	
	int ret = lseek(fd, 0, 0);
	if(ret < 0)
	{
		return -1;
	}
	
	ret = write(fd, (char*)pInfo, sizeof(FileHeadInfo));
	if(ret != sizeof(FileHeadInfo))
	{
		return -1;
	}
	
	return 0;
}

int init_log_index_head(ModLog *pLogManager, char *filename1, char *filename2)
{
	if(pLogManager == NULL || filename1 == NULL)
	{
		printf("init_log_index_head: failed-1\n");
		return -1;
	}
	
	int ret = -1;
	FileHeadInfo info1, info2;
	BOOL bValid = TRUE, bBackupValid = TRUE;
	
	strcpy(pLogManager->index1_filename, filename1);
	pLogManager->index1_fd = open(filename1, O_RDWR|O_CREAT, S_IRUSR|S_IWUSR);
	if(pLogManager->index1_fd < 0)
	{
		perror("init_log_index_head: failed-2");
		reset_log_index_head(&pLogManager->index_head);
		return -1;
	}
	
	ret = read(pLogManager->index1_fd, &info1, sizeof(info1));
	if(ret != sizeof(info1) || !is_log_index_valid(&info1, pLogManager->max_file_num))
	{
		bValid = FALSE;
	}
	
	if(filename2 != NULL)
	{
		strcpy(pLogManager->index2_filename, filename2);
		pLogManager->index2_fd = open(filename2, O_RDWR|O_CREAT, S_IRUSR|S_IWUSR);
	}
	
	if(pLogManager->index2_fd < 0)
	{
		if(bValid)
		{
			memcpy(&pLogManager->index_head, &info1, sizeof(info1));
			return 0;
		}
		else
		{
			reset_log_index_head(&pLogManager->index_head);
			update_log_index_head(pLogManager->index1_fd, &pLogManager->index_head);
			return 0;
		}
	}
	else
	{
		ret = read(pLogManager->index2_fd, &info2, sizeof(info2));
		if(ret != sizeof(info2) || !is_log_index_valid(&info2, pLogManager->max_file_num))
		{
			bBackupValid = FALSE;
		}
		
		if(bValid)
		{
			memcpy(&pLogManager->index_head, &info1, sizeof(info1));
			if(memcmp(&info1, &info2, sizeof(info2)) != 0)
			{
				update_log_index_head(pLogManager->index2_fd, &pLogManager->index_head);
			}
			return 0;
		}
		else
		{
			if(bBackupValid)
			{
				memcpy(&pLogManager->index_head, &info2, sizeof(info2));
				update_log_index_head(pLogManager->index1_fd, &pLogManager->index_head);
				return 0;
			}
			else
			{
				reset_log_index_head(&pLogManager->index_head);
				update_log_index_head(pLogManager->index1_fd, &pLogManager->index_head);
				update_log_index_head(pLogManager->index2_fd, &pLogManager->index_head);
				return 0;
			}
		}
	}
	
	if(pLogManager->index1_fd != -1)
	{
		close(pLogManager->index1_fd);
		pLogManager->index1_fd = -1;
	}
	
	if(pLogManager->index2_fd != -1)
	{
		close(pLogManager->index2_fd);
		pLogManager->index2_fd = -1;
	}
	
	printf("init_log_index_head: failed-3\n");
	return -1;
}

int LogManager_Init(ModLog *pLogManager)
{
	//printf("LogManager_Init: start\n");
	
	if(pLogManager == NULL)
	{
		printf("LogManager_Init: failed-1\n");
		return -1;
	}
	
	pLogManager->init_flag = 0;
	
	pLogManager->current_fd = -1;
	pLogManager->current_num = 0;
	
	memset(pLogManager->index1_filename, 0, sizeof(pLogManager->index1_filename));
	memset(pLogManager->index2_filename, 0, sizeof(pLogManager->index2_filename));
	
	pLogManager->index1_fd = -1;
	pLogManager->index2_fd = -1;
	
	reset_log_index_head(&pLogManager->index_head);
	
	pLogManager->max_file_num = MAXNUM;
	
	if(pthread_mutex_init(&pLogManager->lock, NULL) != 0)
	{
		printf("LogManager_Init: failed-2\n");
		return -1;
	}
	
	if(init_log_index_head(pLogManager, "/root/log/index0", "/root/log/index1") != 0)
	{
		pthread_mutex_destroy(&pLogManager->lock);
		printf("LogManager_Init: failed-3\n");
		return -1;
	}
	
	char filename[32] = {0};
	sprintf(filename, "/root/log/log%d", pLogManager->index_head.index);
	pLogManager->current_fd = open(filename, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR);
	if(pLogManager->current_fd < 0)
	{
		pthread_mutex_destroy(&pLogManager->lock);
		printf("LogManager_Init: failed-4\n");
		return -1;
	}
	
	struct stat statbuf;
	fstat(pLogManager->current_fd, &statbuf);
	pLogManager->current_num = statbuf.st_size/MSGLEN;
	lseek(pLogManager->current_fd, pLogManager->current_num*MSGLEN, 0);
	
	pLogManager->init_flag = 1;
	
	//printf("LogManager_Init: finish\n");
	
	return 0;
}

int LogManager_Write(ModLog *pLogManager, SBizAlarmInfo* info)
{
	if(pLogManager == NULL || pLogManager->init_flag == 0 || info == NULL)
	{
		return -1;
	}
	
	print_info(pLogManager);
	
	pthread_mutex_lock(&pLogManager->lock);
	
	if(pLogManager->current_num == MODLOGNUM)
	{
		if(pLogManager->current_fd != -1)
		{
			close(pLogManager->current_fd);
			pLogManager->current_fd = -1;
		}
		
		pLogManager->index_head.index++;
		if(pLogManager->index_head.index == pLogManager->max_file_num)
		{
			pLogManager->index_head.index = 0;
			pLogManager->index_head.isAgain = 1;
		}
		
		pLogManager->index_head.opNums++;
		pLogManager->index_head.verify = pLogManager->index_head.opNums + pLogManager->index_head.index + pLogManager->index_head.isAgain;
		pLogManager->index_head.verify += (pLogManager->index_head.reserved1 + pLogManager->index_head.reserved2 + pLogManager->index_head.reserved3);
		
		update_log_index_head(pLogManager->index1_fd, &pLogManager->index_head);
		update_log_index_head(pLogManager->index2_fd, &pLogManager->index_head);
		
		char filename[32] = {0};
		sprintf(filename, "/root/log/log%d", pLogManager->index_head.index);
		pLogManager->current_fd = open(filename, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR);
		pLogManager->current_num = 0;
	}
	
	if(pLogManager->current_fd != -1)
	{
		int ret = write(pLogManager->current_fd, (char*)info, sizeof(SBizAlarmInfo));
		if(ret != sizeof(SBizAlarmInfo))
		{
			close(pLogManager->current_fd);
			pLogManager->current_fd = -1;
			pLogManager->current_num = MODLOGNUM;
			pthread_mutex_unlock(&pLogManager->lock);
			return -1;
		}
		pLogManager->current_num++;
	}
	else
	{
		pLogManager->current_num = MODLOGNUM;
		pthread_mutex_unlock(&pLogManager->lock);
		return -1;
	}
	
	pthread_mutex_unlock(&pLogManager->lock);
	
	return 0;
}

int LogManager_Read(ModLog *pLogManager, SAlarmLogSearchPara* psLogSearch, SAlarmLogResult* psLogResult)
{
	int ret = -1;
	int fd = -1;
	
	if(psLogSearch == NULL || psLogResult == NULL || psLogResult->nMaxLogs == 0 || psLogResult->psLogList == NULL)
	{
		return -1;
	}
	
	psLogResult->nRealNum = 0;
	
	if(pLogManager == NULL || pLogManager->init_flag == 0)
	{
		return -1;
	}
	
	pthread_mutex_lock(&pLogManager->lock);//先读出共享资源，就可以解锁了
	
	int index = pLogManager->index_head.index;
	int again = pLogManager->index_head.isAgain;
	int currt = pLogManager->current_num;
	
	int id = index;//记住它，回转时要用到
	
	pthread_mutex_unlock(&pLogManager->lock);
	
	int i = 0;
	SBizAlarmInfo nSAlarmBuf[MODLOGNUM];
	
	while(psLogResult->nRealNum < psLogResult->nMaxLogs)
	{
		if(fd < 0)
		{
			char filename[32] = {0};
			sprintf(filename, "/root/log/log%d", index);
			fd = open(filename, O_RDONLY);
			if(fd < 0)
			{
				goto NextFile;
			}
		}
		
		ret = read(fd, (char*)nSAlarmBuf, sizeof(SBizAlarmInfo)*currt);
		
		close(fd);
		fd = -1;
		
		if(ret > 0)
		{
			currt = ret / sizeof(SBizAlarmInfo);
		}
		else
		{
			currt = 0;
		}
		
		//一条一条查询
		for(i=currt-1; i>=0; i--)
		{
			if(difftime(nSAlarmBuf[i].nTime, psLogSearch->nStart) > 0 && difftime(nSAlarmBuf[i].nTime, psLogSearch->nEnd) < 0)
			{
				//强制转化????
				memcpy(&psLogResult->psLogList[psLogResult->nRealNum], &nSAlarmBuf[i], sizeof(nSAlarmBuf[i]));
				psLogResult->nRealNum++;
				
				if(psLogResult->nRealNum >= psLogResult->nMaxLogs)
				{
					return 0;
				}
			}
		}
		
	NextFile:
		//计算下次要打开的文件
		currt = MODLOGNUM;
		if(index == 0)
		{
			if(again == 1)
			{
				index = pLogManager->max_file_num - 1;
				if(index == id)
				{
					return 0;
				}
			}
			else
			{
				return 0;
			}
		}
		else
		{
			index--;
			//if(again == 1 && index == id)
			if(index == id)
			{
				return 0;
			}
		}
	}
	
	return 0;
}

static disk_manager *g_pDisker = NULL;

int update_disk_log_index_head(int fd, FileHeadInfo *pInfo, int offset)
{
	if(fd < 0)
	{
		return -1;
	}
	
	if(pInfo == NULL)
	{
		return -1;
	}
	
	int ret = lseek(fd, offset, SEEK_SET);
	if(ret < 0)
	{
		return -1;
	}
	
	ret = write(fd, (char*)pInfo, sizeof(FileHeadInfo));
	if(ret != sizeof(FileHeadInfo))
	{
		return -1;
	}
	
	return 0;
}

int DiskLoger_Init(DiskLog *pLogManager, void* pHddMgr)
{
	//printf("DiskLoger_Init: start\n");
	
	if(pLogManager == NULL || pHddMgr == NULL)
	{
		printf("DiskLoger_Init: failed-1\n");
		return -1;
	}
	
	g_pDisker = (disk_manager *)pHddMgr;
	
	pLogManager->init_flag = 0;
	
	reset_log_index_head(&pLogManager->index_head);
	pLogManager->max_log_num = MAXNUM * MODLOGNUM;
	
	pLogManager->using_disk_idx = 0xff;
	memset(pLogManager->logFile, 0, sizeof(pLogManager->logFile));
	memset(pLogManager->disk_sn, 0, sizeof(pLogManager->disk_sn));
	
	if(pthread_mutex_init(&pLogManager->lock, NULL) != 0)
	{
		printf("DiskLoger_Init: failed-2\n");
		return -1;
	}
	
	u32 i = 0;
	for(i = 0; i < MAX_HDD_NUM; i++)
	{
		if(g_pDisker->hinfo[i].is_disk_exist && g_pDisker->hinfo[i].is_partition_exist[0])
		{
			pLogManager->using_disk_idx = i;
			sprintf(pLogManager->logFile,"rec/%c%d/log.dat",'a'+i,1);
			strcpy(pLogManager->disk_sn, g_pDisker->hinfo[i].disk_sn);
			break;
		}
	}
	if(i >= MAX_HDD_NUM)
	{
		pthread_mutex_destroy(&pLogManager->lock);
		printf("DiskLoger_Init: No HDD\n");
		return -1;
	}
	
	int fd = open(pLogManager->logFile, O_RDWR);
	if(fd != -1)
	{
		struct stat statbuf;
		if(fstat(fd, &statbuf) < 0 || statbuf.st_size != 2*sizeof(FileHeadInfo)+pLogManager->max_log_num*sizeof(SBizAlarmInfo))
		{
			printf("DiskLoger_Init: alarm log file len is error,recreate...\n");
			close(fd);
			fd = -1;
			//remove(pLogManager->logFile);
		}
	}
	if(fd < 0)
	{
		fd = open(pLogManager->logFile, O_RDWR|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR);
		if(fd < 0)
		{
			pLogManager->using_disk_idx = 0xff;
			memset(pLogManager->logFile, 0, sizeof(pLogManager->logFile));
			memset(pLogManager->disk_sn, 0, sizeof(pLogManager->disk_sn));
			pthread_mutex_destroy(&pLogManager->lock);
			printf("DiskLoger_Init: failed-3\n");
			return -1;
		}
		
		update_disk_log_index_head(fd, &pLogManager->index_head, 0);
		update_disk_log_index_head(fd, &pLogManager->index_head, sizeof(FileHeadInfo));
		
		SBizAlarmInfo log;
		memset(&log, 0, sizeof(log));
		int i = 0;
		for(i = 0; i < pLogManager->max_log_num; i++)
		{
			write(fd, &log, sizeof(log));
		}
		fsync(fd);
		
		goto INIT_END;
	}
	
	int ret = -1;
	FileHeadInfo info1, info2;
	BOOL bValid = TRUE, bBackupValid = TRUE;
	
	ret = read(fd, &info1, sizeof(info1));
	if(ret != sizeof(info1))
	{
		bValid = FALSE;
		bBackupValid = FALSE;
	}
	else
	{
		if(!is_log_index_valid(&info1, pLogManager->max_log_num))
		{
			bValid = FALSE;
		}
		
		ret = read(fd, &info2, sizeof(info2));
		if(ret != sizeof(info2) || !is_log_index_valid(&info2, pLogManager->max_log_num))
		{
			bBackupValid = FALSE;
		}
	}
	
	if(bValid)
	{
		memcpy(&pLogManager->index_head, &info1, sizeof(info1));
		if(!bBackupValid || (memcmp(&info1, &info2, sizeof(FileHeadInfo)) != 0))
		{
			update_disk_log_index_head(fd, &pLogManager->index_head, sizeof(FileHeadInfo));
		}
	}
	else
	{
		if(bBackupValid)
		{
			memcpy(&pLogManager->index_head, &info2, sizeof(info2));
			update_disk_log_index_head(fd, &pLogManager->index_head, 0);
		}
		else
		{
			update_disk_log_index_head(fd, &pLogManager->index_head, 0);
			update_disk_log_index_head(fd, &pLogManager->index_head, sizeof(FileHeadInfo));
		}
	}
	
INIT_END:
	close(fd);
	fd = -1;
	
	pLogManager->init_flag = 1;
	
	//printf("DiskLoger_Init: finish\n");
	
	return 0;
}

int DiskLoger_Write(DiskLog *pLogManager, SBizAlarmInfo* info)
{
	if(pLogManager == NULL || pLogManager->init_flag == 0 || info == NULL)
	{
		return -1;
	}
	
	if(pLogManager->using_disk_idx < MAX_HDD_NUM)
	{
		if(!g_pDisker->hinfo[pLogManager->using_disk_idx].is_disk_exist)
		{
			return -1;
		}
		
		if(!g_pDisker->hinfo[pLogManager->using_disk_idx].is_partition_exist[0])
		{
			return -1;
		}
		
		if(strcmp(g_pDisker->hinfo[pLogManager->using_disk_idx].disk_sn, pLogManager->disk_sn) != 0)
		{
			return -1;
		}
	}
	else
	{
		return -1;
	}
	
	pthread_mutex_lock(&pLogManager->lock);
	
	int fd = open(pLogManager->logFile, O_RDWR);
	if(fd != -1)
	{
		struct stat statbuf;
		if(fstat(fd, &statbuf) < 0 || statbuf.st_size != 2*sizeof(FileHeadInfo)+pLogManager->max_log_num*sizeof(SBizAlarmInfo))
		{
			printf("DiskLoger_Write: alarm log file len is error,recreate...\n");
			close(fd);
			fd = -1;
			//remove(pLogManager->logFile);
		}
	}
	if(fd < 0)
	{
		fd = open(pLogManager->logFile, O_RDWR|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR);
		if(fd < 0)
		{
			printf("DiskLoger_Write: failed-1\n");
			pthread_mutex_unlock(&pLogManager->lock);
			return -1;
		}
		
		reset_log_index_head(&pLogManager->index_head);
		
		update_disk_log_index_head(fd, &pLogManager->index_head, 0);
		update_disk_log_index_head(fd, &pLogManager->index_head, sizeof(FileHeadInfo));
		
		SBizAlarmInfo log;
		memset(&log, 0, sizeof(log));
		int i = 0;
		for(i = 0; i < pLogManager->max_log_num; i++)
		{
			write(fd, &log, sizeof(log));
		}
		fsync(fd);
	}
	
	int ret = lseek(fd, 2*sizeof(FileHeadInfo) + pLogManager->index_head.index*sizeof(SBizAlarmInfo), SEEK_SET);
	if(ret < 0)
	{
		printf("DiskLoger_Write: failed-2\n");
		close(fd);
		pthread_mutex_unlock(&pLogManager->lock);
		return -1;
	}
	
	ret = write(fd, (char*)info, sizeof(SBizAlarmInfo));
	if(ret != sizeof(SBizAlarmInfo))
	{
		printf("DiskLoger_Write: failed-3\n");
		close(fd);
		pthread_mutex_unlock(&pLogManager->lock);
		return -1;
	}
	
	pLogManager->index_head.index++;
	if(pLogManager->index_head.index == pLogManager->max_log_num)
	{
		pLogManager->index_head.index = 0;
		pLogManager->index_head.isAgain = 1;
	}
	
	pLogManager->index_head.opNums++;
	pLogManager->index_head.verify = pLogManager->index_head.opNums + pLogManager->index_head.index + pLogManager->index_head.isAgain;
	pLogManager->index_head.verify += (pLogManager->index_head.reserved1 + pLogManager->index_head.reserved2 + pLogManager->index_head.reserved3);
	
	update_disk_log_index_head(fd, &pLogManager->index_head, 0);
	update_disk_log_index_head(fd, &pLogManager->index_head, sizeof(FileHeadInfo));
	
	fsync(fd);
	close(fd);
	pthread_mutex_unlock(&pLogManager->lock);
	
	return 0;
}

int DiskLoger_Read(DiskLog *pLogManager, SAlarmLogSearchPara* psLogSearch, SAlarmLogResult* psLogResult)
{
	if(psLogSearch == NULL || psLogResult == NULL || psLogResult->nMaxLogs == 0 || psLogResult->psLogList == NULL)
	{
		return -1;
	}
	
	psLogResult->nRealNum = 0;
	
	if(pLogManager == NULL || pLogManager->init_flag == 0)
	{
		return -1;
	}
	
	if(pLogManager->using_disk_idx < MAX_HDD_NUM)
	{
		if(!g_pDisker->hinfo[pLogManager->using_disk_idx].is_disk_exist)
		{
			return -1;
		}
		
		if(!g_pDisker->hinfo[pLogManager->using_disk_idx].is_partition_exist[0])
		{
			return -1;
		}
		
		if(strcmp(g_pDisker->hinfo[pLogManager->using_disk_idx].disk_sn, pLogManager->disk_sn) != 0)
		{
			return -1;
		}
	}
	else
	{
		return -1;
	}
	
	pthread_mutex_lock(&pLogManager->lock);//先读出共享资源，就可以解锁了
	
	SBizAlarmInfo nSAlarmBuf[MAXNUM * MODLOGNUM];
	
	unsigned int index = pLogManager->index_head.index;
	unsigned int again = pLogManager->index_head.isAgain;
	unsigned int total = again ? pLogManager->max_log_num : index;
	unsigned int start = 0;
	unsigned int end = index;
	if(again)
	{
		start = index;
		end = index + pLogManager->max_log_num;
	}
	
	printf("DiskLoger_Read: start=%d,end=%d,total=%d,index=%d,again=%d\n",start,end,total,index,again);
	
	if(total > 0)
	{
		int fd = open(pLogManager->logFile, O_RDONLY);
		if(fd < 0)
		{
			printf("DiskLoger_Read: failed-1\n");
			pthread_mutex_unlock(&pLogManager->lock);
			return -1;
		}
		
		int ret = lseek(fd, 2*sizeof(FileHeadInfo), SEEK_SET);
		if(ret != 2*sizeof(FileHeadInfo))
		{
			printf("DiskLoger_Read: failed-2\n");
			close(fd);
			pthread_mutex_unlock(&pLogManager->lock);
			return -1;
		}
		
		ret = read(fd, (char*)nSAlarmBuf, sizeof(SBizAlarmInfo)*total);
		if(ret > 0)
		{
			total = ret / sizeof(SBizAlarmInfo);
		}
		else
		{
			total = 0;
		}
		
		printf("DiskLoger_Read: ret=%d,total=%d\n",ret,total);
		
		close(fd);
		fd = -1;
	}
	
	pthread_mutex_unlock(&pLogManager->lock);
	
	if(total > 0)
	{
		int idx = 0;
		for(idx = (int)end-1; idx >= (int)start; idx--)
		{
			int i = (idx % pLogManager->max_log_num);
			
			//printf("DiskLoger_Read: i=%d,idx=%d,start=%d,end=%d\n",i,idx,start,end);
			
			//一条一条查询
			if(nSAlarmBuf[i].magic == 0x5A && difftime(nSAlarmBuf[i].nTime, psLogSearch->nStart) > 0 && difftime(nSAlarmBuf[i].nTime, psLogSearch->nEnd) < 0)
			{
				//强制转化????
				memcpy(&psLogResult->psLogList[psLogResult->nRealNum], &nSAlarmBuf[i], sizeof(nSAlarmBuf[i]));
				psLogResult->nRealNum++;
				
				if(psLogResult->nRealNum >= psLogResult->nMaxLogs)
				{
					return 0;
				}
			}
		}
	}
	
	return 0;
}

