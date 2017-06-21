#ifndef HDD_CMD_H_INCLUDED
#define HDD_CMD_H_INCLUDED

#include "iflytype.h"

typedef	struct {
	s64 capability;
	s64 freesize;
	int status;
	int sleep;
}DiskInfo;

int ifly_format(char *path,int fs);
int ifly_partition(char *path,int count,int fs);
int ifly_diskinfo(char *diskname,DiskInfo *inf);

int mkfsdos_main(int argc, char **argv);
s64 GetSizePart(int argc,char **argv);

#endif /* HDD_CMD_H_INCLUDED */
