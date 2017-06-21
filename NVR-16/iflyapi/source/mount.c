/* vi: set sw=4 ts=4: */
/*
 * Mini mount implementation for busybox
 *
 * Copyright (C) 1995, 1996 by Bruce Perens <bruce@pixar.com>.
 * Copyright (C) 1999-2004 by Erik Andersen <andersen@codepoet.org>
 * Copyright (C) 2005-2006 by Rob Landley <rob@landley.net>
 *
 * Licensed under GPLv2 or later, see file LICENSE in this tarball for details.
 */

/* Design notes: There is no spec for mount.  Remind me to write one.

   mount_main() calls singlemount() which calls mount_it_now().

   mount_main() can loop through /etc/fstab for mount -a
   singlemount() can loop through /etc/filesystems for fstype detection.
   mount_it_now() does the actual mount.
*/

#include <errno.h>
#include <stdio.h>
#include <mntent.h>
#include <sys/mount.h>
#include <limits.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "common.h"

//#undef DBG
//#define DBG printf

int mount_user(char *mounted_path, char *user_path)
{
	/*FILE *fp;
	struct mntent *pme;
	printf("bb_path_mtab_file=%s\n",bb_path_mtab_file);
	fp = setmntent(bb_path_mtab_file, "r");
	if(fp != NULL)
	{
		while((pme=getmntent(fp))!=NULL)
		{
			printf("mnt_fsname:%s,mnt_dir:%s,mnt_type:%s,mnt_opts:%s,mnt_freq:%d,mnt_passno:%d\n",
				pme->mnt_fsname,
				pme->mnt_dir,
				pme->mnt_type,
				pme->mnt_opts,
				pme->mnt_freq,
				pme->mnt_passno);
		}
		endmntent(fp);
	}*/
	
	if(mounted_path == NULL || user_path == NULL)
	{
		return -1;
	}
	DBG("*****************mount_user: before lstat. path:%s\n", mounted_path);
	struct stat st;
	if(lstat(mounted_path, &st))
	{
		printf("mount_user:%s is not exist\n",mounted_path);
		return -1;
	}
	DBG("*****************mount_user: before realpath\n");
	
	char path[256];
	if(realpath(user_path,path) == NULL)
	{
		printf("mount_user:realpath failed\n");
		return -1;
	}
	DBG("*****************mount_user: path: %s\n", path);
	//printf("mount_user:path:%s\n",path);
	int curstat = mount(mounted_path, path, "vfat", 32768, NULL);
	//int curstat = mount(mounted_path, path, "msdos", 32768, NULL);
	if(curstat)
	{
		//if(curstat == 16)//Device or resource busy
		if(errno == 16)//Device or resource busy
		{
			printf("mount_user:mount %s to %s failed1,errcode:%d,errstr:%s\n",mounted_path,path,errno,strerror(errno));
			//return 0;
			return -2;//wrchen 090728
		}
		else
		{
			printf("mount_user:mount %s to %s failed2,errcode:%d,errstr:%s\n",mounted_path,path,errno,strerror(errno));
			return -1;
		}
	}
	printf("mount_user:mount %s to %s success\n",mounted_path,path);
	
	return 0;
}

