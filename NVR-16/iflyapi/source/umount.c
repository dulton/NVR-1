/* vi: set sw=4 ts=4: */
/*
 * Mini umount implementation for busybox
 *
 * Copyright (C) 1999-2004 by Erik Andersen <andersen@codepoet.org>
 * Copyright (C) 2005 by Rob Landley <rob@landley.net>
 *
 * This program is licensed under the GNU General Public license (GPL)
 * version 2 or later, see http://www.fsf.org/licensing/licenses/gpl.html
 * or the file "LICENSE" in the busybox source tarball for the full text.
 *
 */

#include <errno.h>
#include <stdio.h>
#include <mntent.h>
#include <sys/mount.h>
#include <limits.h>
#include <stdlib.h>

#include "common.h"

int umount_user(char *user_path)
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

	if(user_path == NULL)
	{
		return -1;
	}

	char path[256];
	if(realpath(user_path,path) == NULL)
	{
		printf("umount_user:realpath failed 1\n");
		return -1;
	}
	printf("umount_user:user_path:%s,real_path:%s\n",user_path,path);
	
	int curstat = umount(path);
	if(curstat)
	{
		if(errno == 22)//Invalid argument
		{
			printf("umount_user:umount failed1,errno=%d,error=%s\n",errno,strerror(errno));
			return 0;
		}
		else
		{
			printf("umount_user:umount failed2,errno=%d,error=%s\n",errno,strerror(errno));
			return -1;
		}
	}
	
	printf("umount_user:umount success\n");
	
	return 0;
}

