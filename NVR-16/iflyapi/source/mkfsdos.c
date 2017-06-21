/*
   Filename:     mkdosfs.c
   Version:      0.3b (Yggdrasil)
   Author:       Dave Hudson
   Started:      24th August 1994
   Last Updated: 7th May 1998
   Updated by:   Roman Hodek <Roman.Hodek@informatik.uni-erlangen.de>
   Target O/S:   Linux (2.x)

   Description: Utility to allow an MS-DOS filesystem to be created
   under Linux.  A lot of the basic structure of this program has been
   borrowed from Remy Card's "mke2fs" code.

   As far as possible the aim here is to make the "mkdosfs" command
   look almost identical to the other Linux filesystem make utilties,
   eg bad blocks are still specified as blocks, not sectors, but when
   it comes down to it, DOS is tied to the idea of a sector (512 bytes
   as a rule), and not the block.  For example the boot block does not
   occupy a full cluster.

   Fixes/additions May 1998 by Roman Hodek
   <Roman.Hodek@informatik.uni-erlangen.de>:
   - Atari format support
   - New options -A, -S, -C
   - Support for filesystems > 2GB
   - FAT32 support
   
   Copying:     Copyright 1993, 1994 David Hudson (dave@humbug.demon.co.uk)

   Portions copyright 1992, 1993 Remy Card (card@masi.ibp.fr)
   and 1991 Linus Torvalds (torvalds@klaava.helsinki.fi)

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA. */


/* Include the header files */


#define	VERSION			"2.11"
#define VERSION_DATE	"12 Mar 2005"

#include <fcntl.h>
#include <linux/hdreg.h>
#include <linux/fs.h>
#include <linux/fd.h>
#include <endian.h>
#include <mntent.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <common.h>

#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0)
# define __KERNEL__
# include <asm/types.h>
# undef __KERNEL__
#endif

#include "fs_yg.h"//yaogang hdd
#include "partitionindex.h"

ifly_format_progress_t tFormat;

#if __BYTE_ORDER == __BIG_ENDIAN

#include <asm/byteorder.h>
#ifdef __le16_to_cpu
/* ++roman: 2.1 kernel headers define these function, they're probably more
 * efficient then coding the swaps machine-independently. */
#define CF_LE_W	__le16_to_cpu
#define CF_LE_L	__le32_to_cpu
#define CT_LE_W	__cpu_to_le16
#define CT_LE_L	__cpu_to_le32
#else
#define CF_LE_W(v) ((((v) & 0xff) << 8) | (((v) >> 8) & 0xff))
#define CF_LE_L(v) (((unsigned)(v)>>24) | (((unsigned)(v)>>8)&0xff00) | \
               (((unsigned)(v)<<8)&0xff0000) | ((unsigned)(v)<<24))
#define CT_LE_W(v) CF_LE_W(v)
#define CT_LE_L(v) CF_LE_L(v)
#endif /* defined(__le16_to_cpu) */

#else

#define CF_LE_W(v) (v)
#define CF_LE_L(v) (v)
#define CT_LE_W(v) (v)
#define CT_LE_L(v) (v)

#endif /* __BIG_ENDIAN */

/* In earlier versions, an own llseek() was used, but glibc lseek() is
 * sufficient (or even better :) for 64 bit offsets in the meantime */
#define llseek lseek

/* Constant definitions */

#define TRUE 1			/* Boolean constants */
#define FALSE 0

#define TEST_BUFFER_BLOCKS 16
#define HARD_SECTOR_SIZE   512
#define SECTORS_PER_BLOCK ( BLOCK_SIZE / HARD_SECTOR_SIZE )

/* Macro definitions */

/* Report a failure message and return a failure error code */

//#define die( str ) fatal_error( "%s: " str "\n" )

/* Compute ceil(a/b) */

inline int
cdiv (int a, int b)
{
  return (a + b - 1) / b;
}

/* MS-DOS filesystem structures -- I included them here instead of
   including linux/msdos_fs.h since that doesn't include some fields we
   need */

#define ATTR_RO      1		/* read-only */
#define ATTR_HIDDEN  2		/* hidden */
#define ATTR_SYS     4		/* system */
#define ATTR_VOLUME  8		/* volume label */
#define ATTR_DIR     16		/* directory */
#define ATTR_ARCH    32		/* archived */

#define ATTR_NONE    0		/* no attribute bits */
#define ATTR_UNUSED  (ATTR_VOLUME | ATTR_ARCH | ATTR_SYS | ATTR_HIDDEN)
	/* attribute bits that are copied "as is" */

/* FAT values */
//#define FAT_EOF    (atari_format ? 0x0fffffff : 0x0ffffff8)
#define FAT_BAD      0x0ffffff7
#define FAT_EOF      0x0fffffff

//#define TIME_BASE_VALUE		1167609600

#define MSDOS_EXT_SIGN 0x29	/* extended boot sector signature */
#define MSDOS_FAT12_SIGN "FAT12   "	/* FAT12 filesystem signature */
#define MSDOS_FAT16_SIGN "FAT16   "	/* FAT16 filesystem signature */
#define MSDOS_FAT32_SIGN "FAT32   "	/* FAT32 filesystem signature */

#define BOOT_SIGN 0xAA55	/* Boot sector magic number */

#define MAX_CLUST_12	((1 << 12) - 16)
#define MAX_CLUST_16	((1 << 16) - 16)
#define MIN_CLUST_32    65529
/* M$ says the high 4 bits of a FAT32 FAT entry are reserved and don't belong
 * to the cluster number. So the max. cluster# is based on 2^28 */
#define MAX_CLUST_32	((1 << 28) - 16)

#define FAT12_THRESHOLD	4085

#define OLDGEMDOS_MAX_SECTORS	32765
#define GEMDOS_MAX_SECTORS	65531
#define GEMDOS_MAX_SECTOR_SIZE	(16*1024)

#define BOOTCODE_SIZE		448
#define BOOTCODE_FAT32_SIZE	420

/* __attribute__ ((packed)) is used on all structures to make gcc ignore any
 * alignments */

struct msdos_volume_info {
  __u8		drive_number;	/* BIOS drive number */
  __u8		RESERVED;	/* Unused */
  __u8		ext_boot_sign;	/* 0x29 if fields below exist (DOS 3.3+) */
  __u8		volume_id[4];	/* Volume ID number */
  __u8		volume_label[11];/* Volume label */
  __u8		fs_type[8];	/* Typically FAT12 or FAT16 */
} __attribute__ ((packed));

struct msdos_boot_sector
{
  __u8	        boot_jump[3];	/* Boot strap short or near jump */
  __u8          system_id[8];	/* Name - can be used to special case
				   partition manager volumes */
  __u8          sector_size[2];	/* bytes per logical sector */
  __u8          cluster_size;	/* sectors/cluster */
  __u16         reserved;	/* reserved sectors */
  __u8          fats;		/* number of FATs */
  __u8          dir_entries[2];	/* root directory entries */
  __u8          sectors[2];	/* number of sectors */
  __u8          media;		/* media code (unused) */
  __u16         fat_length;	/* sectors/FAT */
  __u16         secs_track;	/* sectors per track */
  __u16         heads;		/* number of heads */
  __u32         hidden;		/* hidden sectors (unused) */
  __u32         total_sect;	/* number of sectors (if sectors == 0) */
  union {
    struct {
      struct msdos_volume_info vi;
      __u8	boot_code[BOOTCODE_SIZE];
    } __attribute__ ((packed)) _oldfat;
    struct {
      __u32	fat32_length;	/* sectors/FAT */
      __u16	flags;		/* bit 8: fat mirroring, low 4: active fat */
      __u8	version[2];	/* major, minor filesystem version */
      __u32	root_cluster;	/* first cluster in root directory */
      __u16	info_sector;	/* filesystem info sector */
      __u16	backup_boot;	/* backup boot sector */
      __u16	reserved2[6];	/* Unused */
      struct msdos_volume_info vi;
      __u8	boot_code[BOOTCODE_FAT32_SIZE];
    } __attribute__ ((packed)) _fat32;
  } __attribute__ ((packed)) fstype;
  __u16		boot_sign;
} __attribute__ ((packed));
#define fat32	fstype._fat32
#define oldfat	fstype._oldfat

struct fat32_fsinfo {
  __u32		reserved1;	/* Nothing as far as I can tell */
  __u32		signature;	/* 0x61417272L */
  __u32		free_clusters;	/* Free cluster count.  -1 if unknown */
  __u32		next_cluster;	/* Most recently allocated cluster.
				 * Unused under Linux. */
  __u32		reserved2[4];
};

struct msdos_dir_entry
  {
    char	name[8], ext[3];	/* name and extension */
    __u8        attr;			/* attribute bits */
    __u8	lcase;			/* Case for base and extension */
    __u8	ctime_ms;		/* Creation time, milliseconds */
    __u16	ctime;			/* Creation time */
    __u16	cdate;			/* Creation date */
    __u16	adate;			/* Last access date */
    __u16	starthi;		/* high 16 bits of first cl. (FAT32) */
    __u16	time, date, start;	/* time, date and first cluster */
    __u32	size;			/* file size (in bytes) */
  } __attribute__ ((packed));

/* The "boot code" we put into the filesystem... it writes a message and
   tells the user to try again */

char dummy_boot_jump[3] = { 0xeb, 0x3c, 0x90 };

char dummy_boot_jump_m68k[2] = { 0x60, 0x1c };

#define MSG_OFFSET_OFFSET 3
char dummy_boot_code[BOOTCODE_SIZE] =
  "\x0e"			/* push cs */
  "\x1f"			/* pop ds */
  "\xbe\x5b\x7c"		/* mov si, offset message_txt */
				/* write_msg: */
  "\xac"			/* lodsb */
  "\x22\xc0"			/* and al, al */
  "\x74\x0b"			/* jz key_press */
  "\x56"			/* push si */
  "\xb4\x0e"			/* mov ah, 0eh */
  "\xbb\x07\x00"		/* mov bx, 0007h */
  "\xcd\x10"			/* int 10h */
  "\x5e"			/* pop si */
  "\xeb\xf0"			/* jmp write_msg */
				/* key_press: */
  "\x32\xe4"			/* xor ah, ah */
  "\xcd\x16"			/* int 16h */
  "\xcd\x19"			/* int 19h */
  "\xeb\xfe"			/* foo: jmp foo */
				/* message_txt: */

  "This is not a bootable disk.  Please insert a bootable floppy and\r\n"
  "press any key to try again ... \r\n";

#define MESSAGE_OFFSET 29	/* Offset of message in above code */

/* Global variables - the root of all evil :-) - see these and weep! */

static char *program_name = "mkdosfs";	/* Name of the program */
static char *device_name = NULL;	/* Name of the device on which to create the filesystem */
static int atari_format = 0;	/* Use Atari variation of MS-DOS FS format */
static int check = FALSE;	/* Default to no readablity checking */
static int verbose = 0;		/* Default to verbose mode off */
static long volume_id;		/* Volume ID number */
static time_t create_time;	/* Creation time */
static char volume_name[] = "           "; /* Volume name */
static unsigned long long blocks;	/* Number of blocks in filesystem */
//find1
static int sector_size = 512;	/* Size of a logical sector */
//static int sector_size = 4096;
//static int sector_size = 4096;//wrchen 090908

static int sector_size_set = 0; /* User selected sector size */
static int backup_boot = 0;	/* Sector# of backup boot sector */
static int reserved_sectors = 0;/* Number of reserved sectors */
//static int badblocks = 0;	/* Number of bad blocks in the filesystem */
static int nr_fats = 2;		/* Default number of FATs to produce */
static int size_fat = 0;	/* Size in bits of FAT entries */
static int size_fat_by_user = 0; /* 1 if FAT size user selected */
static int dev = -1;		/* FS block device file handle */
static int  ignore_full_disk = 0; /* Ignore warning about 'full' disk devices */
//static off_t currently_testing = 0;	/* Block currently being tested (if autodetect bad blocks) */
static struct msdos_boot_sector bs;	/* Boot sector data */
static int start_data_sector;	/* Sector number for the start of the data area */
static int start_data_block;	/* Block number for the start of the data area */
static unsigned char *fat;	/* File allocation table */
static unsigned char *info_sector;	/* FAT32 info sector */
static struct msdos_dir_entry *root_dir;	/* Root directory */
static int size_root_dir;	/* Size of the root directory in bytes */
//find1
//static int sectors_per_cluster = 0;	/* Number of sectors per disk cluster */
//static int sectors_per_cluster = 64;
static int sectors_per_cluster = 0;//wrchen 090908

static int root_dir_entries = 0;	/* Number of root directory entries */
static char *blank_sector;		/* Blank sector - all zeros */
static int hidden_sectors = 0;		/* Number of hidden sectors */

/* Function prototype definitions */

//static void fatal_error (const char *fmt_string) __attribute__((noreturn));
//static void fatal_error_user (const char *fmt_string);
static void mark_FAT_cluster (int cluster, unsigned int value);
static int valid_offset (int fd, loff_t offset);
static unsigned long long count_blocks (char *filename);
static int check_mount (char *device_name);
static int establish_params (int device_num, int size);
static int setup_tables (void);
static int write_tables (void);

//void printlog(char *msg);

/* The function implementations */

/* Handle the reporting of fatal errors.  Volatile to let gcc know that this doesn't return */

/*static void
fatal_error (const char *fmt_string)
{
  fprintf (stderr, fmt_string, program_name, device_name);
  exit (1);			// The error exit code is 1!
}*/

/*static void
fatal_error_user (const char *fmt_string)
{
	fprintf (stderr, fmt_string, program_name, device_name);
}*/

/* Mark the specified cluster as having a particular value */

static void
mark_FAT_cluster (int cluster, unsigned int value)
{
  switch( size_fat ) {
    case 12:
      value &= 0x0fff;
      if (((cluster * 3) & 0x1) == 0)
	{
	  fat[3 * cluster / 2] = (unsigned char) (value & 0x00ff);
	  fat[(3 * cluster / 2) + 1] = (unsigned char) ((fat[(3 * cluster / 2) + 1] & 0x00f0)
						 | ((value & 0x0f00) >> 8));
	}
      else
	{
	  fat[3 * cluster / 2] = (unsigned char) ((fat[3 * cluster / 2] & 0x000f) | ((value & 0x000f) << 4));
	  fat[(3 * cluster / 2) + 1] = (unsigned char) ((value & 0x0ff0) >> 4);
	}
      break;

    case 16:
      value &= 0xffff;
      fat[2 * cluster] = (unsigned char) (value & 0x00ff);
      fat[(2 * cluster) + 1] = (unsigned char) (value >> 8);
      break;

    case 32:
      value &= 0xfffffff;
      fat[4 * cluster] =       (unsigned char)  (value & 0x000000ff);
      fat[(4 * cluster) + 1] = (unsigned char) ((value & 0x0000ff00) >> 8);
      fat[(4 * cluster) + 2] = (unsigned char) ((value & 0x00ff0000) >> 16);
      fat[(4 * cluster) + 3] = (unsigned char) ((value & 0xff000000) >> 24);
      break;

    default:
      //die("Bad FAT size (not 12, 16, or 32)");
	  printf("Bad FAT size (not 12, 16, or 32)\n");
	  return;
  }
}

/* Given a file descriptor and an offset, check whether the offset is a valid offset for the file - return FALSE if it
   isn't valid or TRUE if it is */

static int
valid_offset (int fd, loff_t offset)
{
  char ch;

  if (llseek (fd, offset, SEEK_SET) < 0)
    return FALSE;
  if (read (fd, &ch, 1) < 1)
    return FALSE;
  return TRUE;
}

/* Given a filename, look to see how many blocks of BLOCK_SIZE are present, returning the answer */

#if 1
#define _LARGEFILE64_SOURCE
#include <sys/types.h>
#include <unistd.h>
#else
#include <unistd.h>
#include <linux/unistd.h>

_syscall5(int,  _llseek, uint, fd, ulong, hi, ulong, lo, loff_t *, res,
		  uint, wh);
#endif

static unsigned long long
count_blocks (char *filename)
{
  off_t high, low;
  int fd;

  if ((fd = open (filename, O_RDONLY)) < 0) {
	  perror (filename);
#if 1
	  system("fdisk -l");
	  int k = 0;
	  while((fd = open (filename, O_RDONLY)) < 0)
	  {
		perror (filename);
		if(k++>10)
		{
			sleep(1);
			break;
		}
	  }
	  if(fd < 0)
	  {
		printf("all open failed\n");
		return 0;
	  }
	  else
	  {
		printf("retry open success\n");
	  }
#else
	  return 0;
#endif
  }

#if 1
  #if 1
	unsigned long long bytes = 0;/* really u64 */
	//unsigned long long bytes2 = 0;/* really u64 */
	if(ioctl(fd, BLKGETSIZE64, &bytes) == 0)
	{
		printf("count_blocks BLKGETSIZE64 success\n");
	}
	else
	{
		printf("count_blocks BLKGETSIZE64 failed\n");
		
		unsigned long longsectors = 0;
		if(ioctl(fd, BLKGETSIZE, &longsectors))
		{
			printf("count_blocks BLKGETSIZE failed\n");
			longsectors = 0;
		}
		bytes = ((unsigned long long) longsectors) << 9;
	}
	
	if(bytes)
	{
		unsigned long long mybytes = bytes;
		printf("count_blocks:mybytes=%lld,BLOCK_SIZE=%d,blocks=%lld\n", mybytes, BLOCK_SIZE, mybytes / BLOCK_SIZE);
		close(fd);	//测试运行到此处
		return mybytes / BLOCK_SIZE;
	}
	//fstat应该也是一种办法
  #else
  s64 mybytes = lseek64(fd,0,SEEK_END);
  if(mybytes != -1)
  {
  	printf("count_blocks:mybytes=%lld,BLOCK_SIZE=%d,blocks=%lld\n", mybytes, BLOCK_SIZE, mybytes / BLOCK_SIZE);
    close(fd);
    return mybytes / BLOCK_SIZE;
  }
  printf("count_blocks:lseek64 failed,err:(%d,%s)\n",errno,strerror(errno));
  #endif
#else
  loff_t mybytes;
  int ret = _llseek(fd,0,0,&mybytes,SEEK_END);
  printf("ret=%d,sizeof(mybytes)=%d,mybytes=(0x%08x,%08x),blocks=%d\n",ret,sizeof(mybytes),(int)(mybytes>>32),(int)mybytes,(int)(mybytes / BLOCK_SIZE));
  if(ret == 0) {
	  close (fd);
	  return mybytes / BLOCK_SIZE;
  }
#endif

  /* first try SEEK_END, which should work on most devices nowadays */
  if ((low = llseek(fd, 0, SEEK_END)) <= 0) {
	  printf("$$$lseek error:(%d,%s),sizeof(off_t)=%d\n",errno,strerror(errno),sizeof(off_t));
      low = 0;
      for (high = 1; valid_offset (fd, high); high *= 2)
	  low = high;
      while (low < high - 1) {
	  const loff_t mid = (low + high) / 2;
	  if (valid_offset (fd, mid))
	      low = mid;
	  else
	      high = mid;
      }
      ++low;
  }
  else{
		printf("lseek right\n");
  }

  printf("sizeof(low)=%d,low=%ld,BLOCK_SIZE=%d,blocks=%ld\n",sizeof(low),low,BLOCK_SIZE,(low / BLOCK_SIZE));

  close (fd);
  return low / BLOCK_SIZE;
}

/* Check to see if the specified device is currently mounted - abort if it is */

static int
check_mount (char *device_name)
{
  FILE *f;
  struct mntent *mnt;

  if ((f = setmntent (MOUNTED, "r")) == NULL)
  {
	  //return;
	  return 0;
  }
  while ((mnt = getmntent (f)) != NULL)
    if (strcmp (device_name, mnt->mnt_fsname) == 0)
	{
		//die ("%s contains a mounted file system.");
		endmntent (f);
		printf("%s contains a mounted file system.\n",device_name);
		return -1;
	}
  endmntent (f);
  return 0;
}

/* Establish the geometry and media parameters for the device */

static int
establish_params (int device_num,int size)
{
  long loop_size;
  struct hd_geometry geometry;
  struct floppy_struct param;

  //printf("@@@@@@@@@@device_num=0x%x,size=%d\n",device_num,size);

  if ((0 == device_num) || ((device_num & 0xff00) == 0x0200))
    /* file image or floppy disk */
    {
	  printf("establish_params 1\n");

      if (0 == device_num)
	{
	  param.size = size/512;
	  switch(param.size)
	    {
	    case 720:
	      param.sect = 9 ;
	      param.head = 2;
	      break; 
	    case 1440:
	      param.sect = 9;
	      param.head = 2;
	      break;
	    case 2400:
	      param.sect = 15;
	      param.head = 2;
	      break;
	    case 2880:
	      param.sect = 18;
	      param.head = 2;
	      break;
	    case 5760:
	      param.sect = 36;
	      param.head = 2;
	      break;
	    default:
	      /* fake values */
	      param.sect = 32;
	      param.head = 64;
	      break;
	    }
	  
	}
      else 	/* is a floppy diskette */
	{
	  if (ioctl (dev, FDGETPRM, &param))	/*  Can we get the diskette geometry? */
	  {
		  //die ("unable to get diskette geometry for '%s'");
		  printf("unable to get diskette geometry for '%s'\n",device_name);
		  return -1;
	  }
	}
      bs.secs_track = CT_LE_W(param.sect);	/*  Set up the geometry information */
      bs.heads = CT_LE_W(param.head);
      switch (param.size)	/*  Set up the media descriptor byte */
	{
	case 720:		/* 5.25", 2, 9, 40 - 360K */
	  bs.media = (char) 0xfd;
	  bs.cluster_size = (char) 2;
	  bs.dir_entries[0] = (char) 112;
	  bs.dir_entries[1] = (char) 0;
	  break;

	case 1440:		/* 3.5", 2, 9, 80 - 720K */
	  bs.media = (char) 0xf9;
	  bs.cluster_size = (char) 2;
	  bs.dir_entries[0] = (char) 112;
	  bs.dir_entries[1] = (char) 0;
	  break;

	case 2400:		/* 5.25", 2, 15, 80 - 1200K */
	  bs.media = (char) 0xf9;
	  bs.cluster_size = (char)(atari_format ? 2 : 1);
	  bs.dir_entries[0] = (char) 224;
	  bs.dir_entries[1] = (char) 0;
	  break;
	  
	case 5760:		/* 3.5", 2, 36, 80 - 2880K */
	  bs.media = (char) 0xf0;
	  bs.cluster_size = (char) 2;
	  bs.dir_entries[0] = (char) 224;
	  bs.dir_entries[1] = (char) 0;
	  break;

	case 2880:		/* 3.5", 2, 18, 80 - 1440K */
	floppy_default:
	  bs.media = (char) 0xf0;
	  bs.cluster_size = (char)(atari_format ? 2 : 1);
	  bs.dir_entries[0] = (char) 224;
	  bs.dir_entries[1] = (char) 0;
	  break;

	default:		/* Anything else */
	  if (0 == device_num)
	      goto def_hd_params;
	  else
	      goto floppy_default;
	}
    }
  else if ((device_num & 0xff00) == 0x0700) /* This is a loop device */
    {
	  printf("establish_params 2\n");

      if (ioctl (dev, BLKGETSIZE, &loop_size)) 
	  {
		  //die ("unable to get loop device size");
		  printf("unable to get loop device size\n");
		  return -1;
	  }

      switch (loop_size)  /* Assuming the loop device -> floppy later */
	{
	case 720:		/* 5.25", 2, 9, 40 - 360K */
	  bs.secs_track = CF_LE_W(9);
	  bs.heads = CF_LE_W(2);
	  bs.media = (char) 0xfd;
	  bs.cluster_size = (char) 2;
	  bs.dir_entries[0] = (char) 112;
	  bs.dir_entries[1] = (char) 0;
	  break;

	case 1440:		/* 3.5", 2, 9, 80 - 720K */
	  bs.secs_track = CF_LE_W(9);
	  bs.heads = CF_LE_W(2);
	  bs.media = (char) 0xf9;
	  bs.cluster_size = (char) 2;
	  bs.dir_entries[0] = (char) 112;
	  bs.dir_entries[1] = (char) 0;
	  break;

	case 2400:		/* 5.25", 2, 15, 80 - 1200K */
	  bs.secs_track = CF_LE_W(15);
	  bs.heads = CF_LE_W(2);
	  bs.media = (char) 0xf9;
	  bs.cluster_size = (char)(atari_format ? 2 : 1);
	  bs.dir_entries[0] = (char) 224;
	  bs.dir_entries[1] = (char) 0;
	  break;
	  
	case 5760:		/* 3.5", 2, 36, 80 - 2880K */
	  bs.secs_track = CF_LE_W(36);
	  bs.heads = CF_LE_W(2);
	  bs.media = (char) 0xf0;
	  bs.cluster_size = (char) 2;
	  bs.dir_entries[0] = (char) 224;
	  bs.dir_entries[1] = (char) 0;
	  break;

	case 2880:		/* 3.5", 2, 18, 80 - 1440K */
	  bs.secs_track = CF_LE_W(18);
	  bs.heads = CF_LE_W(2);
	  bs.media = (char) 0xf0;
	  bs.cluster_size = (char)(atari_format ? 2 : 1);
	  bs.dir_entries[0] = (char) 224;
	  bs.dir_entries[1] = (char) 0;
	  break;

	default:		/* Anything else: default hd setup */
	  printf("Loop device does not match a floppy size, using "
		 "default hd params\n");
	  bs.secs_track = CT_LE_W(32); /* these are fake values... */
	  bs.heads = CT_LE_W(64);
	  goto def_hd_params;
	}
    }
  else
    /* Must be a hard disk then! */
    {
	  printf("establish_params 3\n");
	  
      /* Can we get the drive geometry? (Note I'm not too sure about */
      /* whether to use HDIO_GETGEO or HDIO_REQ) */
      if (ioctl (dev, HDIO_GETGEO, &geometry)) {
	printf ("unable to get drive geometry, using default 255/63");
        bs.secs_track = CT_LE_W(63);
        bs.heads = CT_LE_W(255);
      }
      else {
        bs.secs_track = CT_LE_W(geometry.sectors);	/* Set up the geometry information */
        bs.heads = CT_LE_W(geometry.heads);
      }
    def_hd_params:
      bs.media = (char) 0xf8; /* Set up the media descriptor for a hard drive */
      bs.dir_entries[0] = (char) 0;	/* Default to 512 entries */
      bs.dir_entries[1] = (char) 2;
      if (!size_fat && blocks*SECTORS_PER_BLOCK > 1064960) {
	  if (verbose) printf("Auto-selecting FAT32 for large filesystem\n");
	  size_fat = 32;
      }
      if (size_fat == 32) {
	  /* For FAT32, try to do the same as M$'s format command:
	   * fs size < 256M: 0.5k clusters
	   * fs size <   8G: 4k clusters
	   * fs size <  16G: 8k clusters
	   * fs size >= 16G: 16k clusters
	   */
	  unsigned long sz_mb =
	      (blocks+(1<<(20-BLOCK_SIZE_BITS))-1) >> (20-BLOCK_SIZE_BITS);
#if 1 //zhao1 20100715 for HITACHI HTS545025B9A300
   bs.cluster_size = sz_mb >= 32*1024 ? 64 :
       sz_mb >= 16*1024 ? 32 :
       sz_mb >=  8*1024 ? 16 :
       sz_mb >=     256 ?  8 :
             1;
#else
	  bs.cluster_size = sz_mb >= 16*1024 ? 32 :
			    sz_mb >=  8*1024 ? 16 :
			    sz_mb >=     256 ?  8 :
					        1;
#endif
			printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@blocks=(%x,%x),BLOCK_SIZE_BITS=%d,sz_mb=%ld,bs.cluster_size=%d\n",(int)(blocks>>32),(int)blocks,BLOCK_SIZE_BITS,sz_mb,bs.cluster_size);
      }
      else {
	  /* FAT12 and FAT16: start at 4 sectors per cluster */
	  bs.cluster_size = (char) 4;
      }
    }
  return 0;
}


/* Create the filesystem data tables */

static int
setup_tables (void)
{
  unsigned num_sectors;
  unsigned cluster_count = 0, fat_length;
  unsigned fatdata;			/* Sectors for FATs + data area */
  struct tm *ctime;
  struct msdos_volume_info *vi = (size_fat == 32 ? &bs.fat32.vi : &bs.oldfat.vi);
  
  if (atari_format)
      /* On Atari, the first few bytes of the boot sector are assigned
       * differently: The jump code is only 2 bytes (and m68k machine code
       * :-), then 6 bytes filler (ignored), then 3 byte serial number. */
    strncpy((char *)bs.system_id-1, "mkdosf", 6);
  else
    strncpy((char *)bs.system_id, "mkdosfs", sizeof(bs.system_id));//csp modify

  if (sectors_per_cluster)
    bs.cluster_size = (char) sectors_per_cluster;

	//yaogang modify 20170328
  if (bs.media == 0xf8)
	vi->drive_number=0x80;
  else
	vi->drive_number=0x00;
  //yaogang modify 20170328

  printf("%s sectors_per_cluster=%d, cluster_size=%d, sector_size=%d, drive_number: %d\n",
  	__func__, sectors_per_cluster, bs.cluster_size, sector_size, vi->drive_number);//wrchen 090909
  
  if (size_fat == 32)
    {
      /* Under FAT32, the root dir is in a cluster chain, and this is
       * signalled by bs.dir_entries being 0. */
      bs.dir_entries[0] = bs.dir_entries[1] = (char) 0;
      root_dir_entries = 0;
    }
  else if (root_dir_entries)
    {
      /* Override default from establish_params() */
      bs.dir_entries[0] = (char) (root_dir_entries & 0x00ff);
      bs.dir_entries[1] = (char) ((root_dir_entries & 0xff00) >> 8);
    }
  else
    root_dir_entries = bs.dir_entries[0] + (bs.dir_entries[1] << 8);

  if (atari_format) {
    bs.system_id[5] = (unsigned char) (volume_id & 0x000000ff);
    bs.system_id[6] = (unsigned char) ((volume_id & 0x0000ff00) >> 8);
    bs.system_id[7] = (unsigned char) ((volume_id & 0x00ff0000) >> 16);
  }
  else {//
    vi->volume_id[0] = (unsigned char) (volume_id & 0x000000ff);
    vi->volume_id[1] = (unsigned char) ((volume_id & 0x0000ff00) >> 8);
    vi->volume_id[2] = (unsigned char) ((volume_id & 0x00ff0000) >> 16);
    vi->volume_id[3] = (unsigned char) (volume_id >> 24);
  }

  if (!atari_format) {
    memcpy(vi->volume_label, volume_name, 11);
  
    memcpy(bs.boot_jump, dummy_boot_jump, 3);
    /* Patch in the correct offset to the boot code */
    bs.boot_jump[1] = ((size_fat == 32 ?
			(char *)&bs.fat32.boot_code :
			(char *)&bs.oldfat.boot_code) -
		       (char *)&bs) - 2;

    if (size_fat == 32) {
	int offset = (char *)&bs.fat32.boot_code -
		     (char *)&bs + MESSAGE_OFFSET + 0x7c00;
	if (dummy_boot_code[BOOTCODE_FAT32_SIZE-1])
	  printf ("Warning: message too long; truncated\n");
	dummy_boot_code[BOOTCODE_FAT32_SIZE-1] = 0;
	memcpy(bs.fat32.boot_code, dummy_boot_code, BOOTCODE_FAT32_SIZE);
	bs.fat32.boot_code[MSG_OFFSET_OFFSET] = offset & 0xff;
	bs.fat32.boot_code[MSG_OFFSET_OFFSET+1] = offset >> 8;
    }
    else {
	memcpy(bs.oldfat.boot_code, dummy_boot_code, BOOTCODE_SIZE);
    }
    bs.boot_sign = CT_LE_W(BOOT_SIGN);
  }
  else {
    memcpy(bs.boot_jump, dummy_boot_jump_m68k, 2);
  }
  if (verbose >= 2)
    printf( "Boot jump code is %02x %02x\n",
	    bs.boot_jump[0], bs.boot_jump[1] );

  if (!reserved_sectors)
      reserved_sectors = (size_fat == 32) ? 32 : 1;
  else {
      if (size_fat == 32 && reserved_sectors < 2)
	  {
		  //die("On FAT32 at least 2 reserved sectors are needed.");
		  printf("On FAT32 at least 2 reserved sectors are needed.\n");
		  return -1;
	  }
  }
  bs.reserved = CT_LE_W(reserved_sectors);
  if (verbose >= 2)
    printf( "Using %d reserved sectors\n", reserved_sectors );
  bs.fats = (char) nr_fats;
  if (!atari_format || size_fat == 32)
    bs.hidden = CT_LE_L(hidden_sectors);
  else {
    /* In Atari format, hidden is a 16 bit field */
    __u16 hidden = CT_LE_W(hidden_sectors);
    if (hidden_sectors & ~0xffff)
	{
		//die("#hidden doesn't fit in 16bit field of Atari format\n");
		printf("#hidden doesn't fit in 16bit field of Atari format\n");
		return -1;
	}
    memcpy( &bs.hidden, &hidden, 2 );
  }

  num_sectors = (long long)blocks*BLOCK_SIZE/sector_size;
  printf("@@@num_sectors=%u,blocks=%u,BLOCK_SIZE=%d,sector_size=%d\n",num_sectors,(int)blocks,(int)BLOCK_SIZE,sector_size);//3T4T
  if (!atari_format) {
    unsigned fatlength12, fatlength16, fatlength32;
    unsigned maxclust12, maxclust16, maxclust32;
    unsigned clust12, clust16, clust32;
    int maxclustsize;
    						//FAT32  0
    fatdata = num_sectors - cdiv (root_dir_entries * 32, sector_size) -
	      reserved_sectors;

    if (sectors_per_cluster)
      bs.cluster_size = maxclustsize = sectors_per_cluster;
    else
      /* An initial guess for bs.cluster_size should already be set */
      maxclustsize = 128;

    if (verbose >= 2)
      printf( "%d sectors for FAT+data, starting with %d sectors/cluster\n",
	      fatdata, bs.cluster_size );
    do {
      if (verbose >= 2)
	printf( "Trying with %d sectors/cluster:\n", bs.cluster_size );

      /* The factor 2 below avoids cut-off errors for nr_fats == 1.
       * The "nr_fats*3" is for the reserved first two FAT entries */
      clust12 = 2*((long long) fatdata *sector_size + nr_fats*3) /
	(2*(int) bs.cluster_size * sector_size + nr_fats*3);
      fatlength12 = cdiv (((clust12+2) * 3 + 1) >> 1, sector_size);
      /* Need to recalculate number of clusters, since the unused parts of the
       * FATS and data area together could make up space for an additional,
       * not really present cluster. */
      clust12 = (fatdata - nr_fats*fatlength12)/bs.cluster_size;
      maxclust12 = (fatlength12 * 2 * sector_size) / 3;
      if (maxclust12 > MAX_CLUST_12)
	maxclust12 = MAX_CLUST_12;
      if (verbose >= 2)
	printf( "FAT12: #clu=%u, fatlen=%u, maxclu=%u, limit=%u\n",
		clust12, fatlength12, maxclust12, MAX_CLUST_12 );
      if (clust12 > maxclust12-2) {
	clust12 = 0;
	if (verbose >= 2)
	  printf( "FAT12: too much clusters\n" );
      }

      clust16 = ((long long) fatdata *sector_size + nr_fats*4) /
	((int) bs.cluster_size * sector_size + nr_fats*2);
      fatlength16 = cdiv ((clust16+2) * 2, sector_size);
      /* Need to recalculate number of clusters, since the unused parts of the
       * FATS and data area together could make up space for an additional,
       * not really present cluster. */
      clust16 = (fatdata - nr_fats*fatlength16)/bs.cluster_size;
      maxclust16 = (fatlength16 * sector_size) / 2;
      if (maxclust16 > MAX_CLUST_16)
	maxclust16 = MAX_CLUST_16;
      if (verbose >= 2)
	printf( "FAT16: #clu=%u, fatlen=%u, maxclu=%u, limit=%u\n",
		clust16, fatlength16, maxclust16, MAX_CLUST_16 );
      if (clust16 > maxclust16-2) {
	if (verbose >= 2)
	  printf( "FAT16: too much clusters\n" );
	clust16 = 0;
      }
      /* The < 4078 avoids that the filesystem will be misdetected as having a
       * 12 bit FAT. */
      if (clust16 < FAT12_THRESHOLD && !(size_fat_by_user && size_fat == 16)) {
	if (verbose >= 2)
	  printf( clust16 < FAT12_THRESHOLD ?
		  "FAT16: would be misdetected as FAT12\n" :
		  "FAT16: too much clusters\n" );
	clust16 = 0;
      }

      clust32 = ((long long) fatdata *sector_size + nr_fats*8) /
	((int) bs.cluster_size * sector_size + nr_fats*4);
      fatlength32 = cdiv ((clust32+2) * 4, sector_size);
	  printf("fatdata=%u,clust32=%u,fatlength32=%u\n",fatdata,clust32,fatlength32);//3T4T
      /* Need to recalculate number of clusters, since the unused parts of the
       * FATS and data area together could make up space for an additional,
       * not really present cluster. */
      clust32 = (fatdata - nr_fats*fatlength32)/bs.cluster_size;
      maxclust32 = (fatlength32 * sector_size) / 4;
      if (maxclust32 > MAX_CLUST_32)
	maxclust32 = MAX_CLUST_32;
      if (clust32 && clust32 < MIN_CLUST_32 && !(size_fat_by_user && size_fat == 32)) {
       clust32 = 0;
       if (verbose >= 2)
         printf( "FAT32: not enough clusters (%d)\n", MIN_CLUST_32);
      }
      if (verbose >= 2)
	printf( "FAT32: #clu=%u, fatlen=%u, maxclu=%u, limit=%u\n",
		clust32, fatlength32, maxclust32, MAX_CLUST_32 );
      if (clust32 > maxclust32) {
	clust32 = 0;
	if (verbose >= 2)
	  printf( "FAT32: too much clusters\n" );
      }

      if ((clust12 && (size_fat == 0 || size_fat == 12)) ||
	  (clust16 && (size_fat == 0 || size_fat == 16)) ||
	  (clust32 && size_fat == 32))
	break;

      bs.cluster_size <<= 1;
    } while (bs.cluster_size && bs.cluster_size <= maxclustsize);

    /* Use the optimal FAT size if not specified;
     * FAT32 is (not yet) choosen automatically */
    if (!size_fat) {
	size_fat = (clust16 > clust12) ? 16 : 12;
	if (verbose >= 2)
	  printf( "Choosing %d bits for FAT\n", size_fat );
    }

    switch (size_fat) {
      case 12:
	cluster_count = clust12;
	fat_length = fatlength12;
	bs.fat_length = CT_LE_W(fatlength12);
	memcpy(vi->fs_type, MSDOS_FAT12_SIGN, 8);
	break;

      case 16:
	if (clust16 < FAT12_THRESHOLD) {
	    if (size_fat_by_user) {
		fprintf( stderr, "WARNING: Not enough clusters for a "
			 "16 bit FAT! The filesystem will be\n"
			 "misinterpreted as having a 12 bit FAT without "
			 "mount option \"fat=16\".\n" );
	    }
	    else {
		fprintf( stderr, "This filesystem has an unfortunate size. "
			 "A 12 bit FAT cannot provide\n"
			 "enough clusters, but a 16 bit FAT takes up a little "
			 "bit more space so that\n"
			 "the total number of clusters becomes less than the "
			 "threshold value for\n"
			 "distinction between 12 and 16 bit FATs.\n" );
		//die( "Make the file system a bit smaller manually." );
		printf( "Make the file system a bit smaller manually.\n" );
		return -1;
	    }
	}
	cluster_count = clust16;
	fat_length = fatlength16;
	bs.fat_length = CT_LE_W(fatlength16);
	memcpy(vi->fs_type, MSDOS_FAT16_SIGN, 8);
	break;

      case 32:
	cluster_count = clust32;
	fat_length = fatlength32;
	printf("fat_length=%d,fatlength32=%d\n",fat_length,fatlength32);
	bs.fat_length = CT_LE_W(0);
	bs.fat32.fat32_length = CT_LE_L(fatlength32);
	memcpy(vi->fs_type, MSDOS_FAT32_SIGN, 8);
	break;
	
      default:
	//die("FAT not 12, 16 or 32 bits");
	printf("FAT not 12, 16 or 32 bits\n");
	return -1;
    }
  }
  else {
    unsigned clusters, maxclust;
      
    /* GEMDOS always uses a 12 bit FAT on floppies, and always a 16 bit FAT on
     * hard disks. So use 12 bit if the size of the file system suggests that
     * this fs is for a floppy disk, if the user hasn't explicitly requested a
     * size.
     */
    if (!size_fat)
      size_fat = (num_sectors == 1440 || num_sectors == 2400 ||
		  num_sectors == 2880 || num_sectors == 5760) ? 12 : 16;
    if (verbose >= 2)
      printf( "Choosing %d bits for FAT\n", size_fat );

    /* Atari format: cluster size should be 2, except explicitly requested by
     * the user, since GEMDOS doesn't like other cluster sizes very much.
     * Instead, tune the sector size for the FS to fit.
     */
    bs.cluster_size = sectors_per_cluster ? sectors_per_cluster : 2;
    if (!sector_size_set) {
      while( num_sectors > GEMDOS_MAX_SECTORS ) {
	num_sectors >>= 1;
	sector_size <<= 1;
      }
    }
    if (verbose >= 2)
      printf( "Sector size must be %d to have less than %d log. sectors\n",
	      sector_size, GEMDOS_MAX_SECTORS );

    /* Check if there are enough FAT indices for how much clusters we have */
    do {
      fatdata = num_sectors - cdiv (root_dir_entries * 32, sector_size) -
		reserved_sectors;
      /* The factor 2 below avoids cut-off errors for nr_fats == 1 and
       * size_fat == 12
       * The "2*nr_fats*size_fat/8" is for the reserved first two FAT entries
       */
      clusters = (2*((long long)fatdata*sector_size - 2*nr_fats*size_fat/8)) /
		 (2*((int)bs.cluster_size*sector_size + nr_fats*size_fat/8));
      fat_length = cdiv( (clusters+2)*size_fat/8, sector_size );
      /* Need to recalculate number of clusters, since the unused parts of the
       * FATS and data area together could make up space for an additional,
       * not really present cluster. */
      clusters = (fatdata - nr_fats*fat_length)/bs.cluster_size;
      maxclust = (fat_length*sector_size*8)/size_fat;
      if (verbose >= 2)
	printf( "ss=%d: #clu=%d, fat_len=%d, maxclu=%d\n",
		sector_size, clusters, fat_length, maxclust );
      
      /* last 10 cluster numbers are special (except FAT32: 4 high bits rsvd);
       * first two numbers are reserved */
      if (maxclust <= (size_fat == 32 ? MAX_CLUST_32 : (1<<size_fat)-0x10) &&
	  clusters <= maxclust-2)
	break;
      if (verbose >= 2)
	printf( clusters > maxclust-2 ?
		"Too many clusters\n" : "FAT too big\n" );

      /* need to increment sector_size once more to  */
      if (sector_size_set)
	  {
		  //die( "With this sector size, the maximum number of FAT entries "
	       //"would be exceeded." );
		  printf( "With this sector size, the maximum number of FAT entries "
			  "would be exceeded.\n" );
		  return -1;
	  }
      num_sectors >>= 1;
      sector_size <<= 1;
    } while( sector_size <= GEMDOS_MAX_SECTOR_SIZE );
    
    if (sector_size > GEMDOS_MAX_SECTOR_SIZE)
	{
		//die( "Would need a sector size > 16k, which GEMDOS can't work with");
		printf( "Would need a sector size > 16k, which GEMDOS can't work with\n");
		return -1;
	}

    cluster_count = clusters;
    if (size_fat != 32)
	bs.fat_length = CT_LE_W(fat_length);
    else {
	bs.fat_length = 0;
	bs.fat32.fat32_length = CT_LE_L(fat_length);
    }
  }

  bs.sector_size[0] = (char) (sector_size & 0x00ff);
  bs.sector_size[1] = (char) ((sector_size & 0xff00) >> 8);

  if (size_fat == 32)
    {
      /* set up additional FAT32 fields */
      bs.fat32.flags = CT_LE_W(0);
      bs.fat32.version[0] = 0;
      bs.fat32.version[1] = 0;
      bs.fat32.root_cluster = CT_LE_L(2);
      bs.fat32.info_sector = CT_LE_W(1);
      if (!backup_boot)
	backup_boot = (reserved_sectors >= 7) ? 6 :
		      (reserved_sectors >= 2) ? reserved_sectors-1 : 0;
      else
	{
	  if (backup_boot == 1)
	  {
		  //die("Backup boot sector must be after sector 1");
		  printf("Backup boot sector must be after sector 1\n");
		  return -1;
	  }
	  else if (backup_boot >= reserved_sectors)
	  {
		  //die("Backup boot sector must be a reserved sector");
		  printf("Backup boot sector must be a reserved sector\n");
		  return -1;
	  }
	}
      if (verbose >= 2)
	printf( "Using sector %d as backup boot sector (0 = none)\n",
		backup_boot );
      bs.fat32.backup_boot = CT_LE_W(backup_boot);
      memset( &bs.fat32.reserved2, 0, sizeof(bs.fat32.reserved2) );
    }
  
  if (atari_format) {
      /* Just some consistency checks */
      if (num_sectors >= GEMDOS_MAX_SECTORS)
	  {
		  //die( "GEMDOS can't handle more than 65531 sectors" );
		  printf( "GEMDOS can't handle more than 65531 sectors\n" );
		  return -1;
	  }
      else if (num_sectors >= OLDGEMDOS_MAX_SECTORS)
	  printf( "Warning: More than 32765 sector need TOS 1.04 "
		  "or higher.\n" );
  }
  if (num_sectors >= 65536)
    {
      bs.sectors[0] = (char) 0;
      bs.sectors[1] = (char) 0;
      bs.total_sect = CT_LE_L(num_sectors);
    }
  else
    {
      bs.sectors[0] = (char) (num_sectors & 0x00ff);
      bs.sectors[1] = (char) ((num_sectors & 0xff00) >> 8);
      if (!atari_format)
	  bs.total_sect = CT_LE_L(0);
    }

  if (!atari_format)
    vi->ext_boot_sign = MSDOS_EXT_SIGN;

  if (!cluster_count)
    {
      if (sectors_per_cluster)	/* If yes, die if we'd spec'd sectors per cluster */
	  {
		  //die ("Too many clusters for file system - try more sectors per cluster");
		  printf("Too many clusters for file system - try more sectors per cluster\n");
		  return -1;
	  }
      else
	  {
		  //die ("Attempting to create a too large file system");
		  printf("Attempting to create a too large file system\n");
		  return -1;
	  }
    }

  
  /* The two following vars are in hard sectors, i.e. 512 byte sectors! */
  start_data_sector = (reserved_sectors + nr_fats * fat_length) *
		      (sector_size/HARD_SECTOR_SIZE);
  start_data_block = (start_data_sector + SECTORS_PER_BLOCK - 1) /
		     SECTORS_PER_BLOCK;

  if (blocks < start_data_block + 32)	/* Arbitrary undersize file system! */
  {
	  //die ("Too few blocks for viable file system");
	  printf("Too few blocks for viable file system\n");
	  return -1;
  }

  if (verbose)
    {
      printf("%s has %d head%s and %d sector%s per track,\n",
	     device_name, CF_LE_W(bs.heads), (CF_LE_W(bs.heads) != 1) ? "s" : "",
	     CF_LE_W(bs.secs_track), (CF_LE_W(bs.secs_track) != 1) ? "s" : ""); 
      printf("logical sector size is %d,\n",sector_size);
      printf("using 0x%02x media descriptor, with %d sectors;\n",
	     (int) (bs.media), num_sectors);
      printf("file system has %d %d-bit FAT%s and %d sector%s per cluster.\n",
	     (int) (bs.fats), size_fat, (bs.fats != 1) ? "s" : "",
	     (int) (bs.cluster_size), (bs.cluster_size != 1) ? "s" : "");
      printf ("FAT size is %d sector%s, and provides %d cluster%s.\n",
	      fat_length, (fat_length != 1) ? "s" : "",
	      cluster_count, (cluster_count != 1) ? "s" : "");
      if (size_fat != 32)
	printf ("Root directory contains %d slots.\n",
		(int) (bs.dir_entries[0]) + (int) (bs.dir_entries[1]) * 256);
      printf ("Volume ID is %08lx, ", volume_id &
	      (atari_format ? 0x00ffffff : 0xffffffff));
      if ( strcmp(volume_name, "           ") )
	printf("volume label %s.\n", volume_name);
      else
	printf("no volume label.\n");
    }

  /* Make the file allocation tables! */

  //if ((fat = (unsigned char *) malloc (fat_length * sector_size)) == NULL)
  printf("%s fat malloc size: %d, sector_size: %d, TEST_BUFFER_BLOCKS: %d, BLOCK_SIZE: %d\n", 
  	__func__, 2*sector_size+TEST_BUFFER_BLOCKS*BLOCK_SIZE, sector_size ,TEST_BUFFER_BLOCKS ,BLOCK_SIZE);
  
  if ((fat = (unsigned char *) malloc (2*sector_size+TEST_BUFFER_BLOCKS*BLOCK_SIZE)) == NULL)
  {
	  //die ("unable to allocate space for FAT image in memory");
	  printf("unable to allocate space for FAT image in memory\n");
	  return -1;
  }

  //memset( fat, 0, fat_length * sector_size );
  memset( fat, 0, 2*sector_size+TEST_BUFFER_BLOCKS*BLOCK_SIZE );

  mark_FAT_cluster (0, 0xffffffff);	/* Initial fat entries */
  mark_FAT_cluster (1, 0xffffffff);
  fat[0] = (unsigned char) bs.media;	/* Put media type in first byte! */
  if (size_fat == 32) {
    /* Mark cluster 2 as EOF (used for root dir) */
    mark_FAT_cluster (2, FAT_EOF);
  }

  /* Make the root directory entries */

  size_root_dir = (size_fat == 32) ?
		  bs.cluster_size*sector_size :
		  (((int)bs.dir_entries[1]*256+(int)bs.dir_entries[0]) *
		   sizeof (struct msdos_dir_entry));
  printf("size_root_dir=%d\n",size_root_dir);
  if ((root_dir = (struct msdos_dir_entry *) malloc (size_root_dir)) == NULL)
    {
      free (fat);		/* Tidy up before we die! */
      //die ("unable to allocate space for root directory in memory");
	  printf("unable to allocate space for root directory in memory\n");
	  return -1;
    }

  memset(root_dir, 0, size_root_dir);
  if ( memcmp(volume_name, "           ", 11) )
    {
      struct msdos_dir_entry *de = &root_dir[0];
      memcpy(de->name, volume_name, 11);
      de->attr = ATTR_VOLUME;
      ctime = localtime(&create_time);
      de->time = CT_LE_W((unsigned short)((ctime->tm_sec >> 1) +
			  (ctime->tm_min << 5) + (ctime->tm_hour << 11)));
      de->date = CT_LE_W((unsigned short)(ctime->tm_mday +
					  ((ctime->tm_mon+1) << 5) +
					  ((ctime->tm_year-80) << 9)));
      de->ctime_ms = 0;
      de->ctime = de->time;
      de->cdate = de->date;
      de->adate = de->date;
      de->starthi = CT_LE_W(0);
      de->start = CT_LE_W(0);
      de->size = CT_LE_L(0);
	  printf("diff:volume_name:%s,size_root_dir=%d\n",volume_name,size_root_dir);
    }
  else
	{
	  printf("same:volume_name:%s,size_root_dir=%d\n",volume_name,size_root_dir);
	}

  if (size_fat == 32) {
    /* For FAT32, create an info sector */
    struct fat32_fsinfo *info;
    
    if (!(info_sector = malloc( sector_size )))
	{
		free (fat);
		free (root_dir);
		//die("Out of memory");
		printf("Out of memory\n");
		return -1;
	}
    memset(info_sector, 0, sector_size);
    /* fsinfo structure is at offset 0x1e0 in info sector by observation */
    info = (struct fat32_fsinfo *)(info_sector + 0x1e0);

    /* Info sector magic */
    info_sector[0] = 'R';
    info_sector[1] = 'R';
    info_sector[2] = 'a';
    info_sector[3] = 'A';

    /* Magic for fsinfo structure */
    info->signature = CT_LE_L(0x61417272);
    /* We've allocated cluster 2 for the root dir. */
    info->free_clusters = CT_LE_L(cluster_count - 1);
    info->next_cluster = CT_LE_L(2);
	//printf("cluster_count=%d,free_clusters=%d\n",cluster_count,info->free_clusters);

    /* Info sector also must have boot sign */
    *(__u16 *)(info_sector + 0x1fe) = CT_LE_W(BOOT_SIGN);
  }
  
  if (!(blank_sector = malloc( sector_size )))
  {
	  free (fat);
	  free (root_dir);
	  if (size_fat == 32) free (info_sector);
	  //die( "Out of memory" );
	  printf( "Out of memory\n" );
	  return -1;
  }
  memset(blank_sector, 0, sector_size);
  
  return 0;
}


/* Write the new filesystem's data tables to wherever they're going to end up! */

#define error(str)				\
  do {						\
    free (fat);					\
    if (info_sector) free (info_sector);	\
    free (root_dir);				\
	if (blank_sector) free( blank_sector );	\
    /*die (str);					\*/	\
	printf(str);					\
	return -1;						\
  } while(0)

#define seekto(pos,errstr)						\
  do {									\
    loff_t __pos = (pos);						\
    if (llseek (dev, __pos, SEEK_SET) != __pos)				\
	error ("seek to " errstr " failed whilst writing tables");	\
  } while(0)

#define writebuf(buf,size,errstr)			\
  do {							\
    int __size = (size);				\
    if (write (dev, buf, __size) != __size)		\
	error ("failed whilst writing " errstr);	\
  } while(0)

struct short_format
{
	char name[8];
	char ext[3];
	unsigned char  property;
	unsigned char  reserved;
	unsigned char  ten_ms;
	unsigned short create_time;
	unsigned short create_date;
	unsigned short last_date;
	unsigned short hi_start;
	unsigned short modify_time;
	unsigned short modify_date;
	unsigned short low_start;
	unsigned int   len;
} __attribute__ ((packed));

struct long_format
{
	unsigned char  property;
	unsigned short name1[5];
	unsigned char  flag;
	unsigned char  reserved;
	unsigned char  check;
	unsigned short name2[6];
	unsigned short start;
	unsigned short name3[2];
} __attribute__ ((packed));

//#define FILE_VECTOR_SIZE	(128*1024*1024)//yaogang hdd

int fillshortDVRname(struct short_format *psf,time_t t,int start_cluster,int size)
{
	char name[16];
	sprintf(name,"IFLYDVR ");
	strncpy(psf->name,name,8);
	strncpy(psf->ext,"   ",3);
	psf->property = 0x0;
	psf->reserved = 0;
	psf->ten_ms = 0;
	struct tm* ctime = localtime(&t);
	psf->create_time = CT_LE_W((unsigned short)((ctime->tm_sec >> 1) +
		(ctime->tm_min << 5) + (ctime->tm_hour << 11)));
	psf->create_date = CT_LE_W((unsigned short)(ctime->tm_mday +
		((ctime->tm_mon+1) << 5) +
		((ctime->tm_year-80) << 9)));
	psf->last_date = psf->create_date;
	psf->hi_start = CT_LE_W((unsigned short)((start_cluster)>>16));
	psf->modify_time = psf->create_time;
	psf->modify_date = psf->create_date;
	psf->low_start = CT_LE_W((unsigned short)(start_cluster));
	psf->len = CT_LE_L(size);
	return 0;
}

int fillshortPicindexname(struct short_format *psf,int index,time_t t,int start_cluster,int size)
{
	char name[16];
	
	//yaogang hdd
	//sprintf(name,"INDEX%02d ",index);
	memset(name,0,sizeof(name));
	sprintf(name,"PINDX%02d ",index+10);
	
	strncpy(psf->name,name,8);
	strncpy(psf->ext,"BIN",3);
	psf->property = 0x0;
	psf->reserved = 0;
	psf->ten_ms = 0;
	struct tm* ctime = localtime(&t);
	psf->create_time = CT_LE_W((unsigned short)((ctime->tm_sec >> 1) +
		(ctime->tm_min << 5) + (ctime->tm_hour << 11)));
	psf->create_date = CT_LE_W((unsigned short)(ctime->tm_mday +
		((ctime->tm_mon+1) << 5) +
		((ctime->tm_year-80) << 9)));
	psf->last_date = psf->create_date;
	psf->hi_start = CT_LE_W((unsigned short)((start_cluster)>>16));
	psf->modify_time = psf->create_time;
	psf->modify_date = psf->create_date;
	psf->low_start = CT_LE_W((unsigned short)(start_cluster));
	psf->len = CT_LE_L(size);
	return 0;
}


int filllongPicindexname(struct long_format *plf,int index,time_t t,int start_cluster,int size)
{
	int i,j=0,chksum=0;
	char name[16];
	char shortname[16];
	
	//yaogang hdd
	//sprintf(name,"index%02d.bin",index);
	//sprintf(shortname,"INDEX%02d BIN",index);
	memset(name,0,sizeof(name));
	memset(shortname,0,sizeof(shortname));
	sprintf(name,"pindx%02d.bin",index+10);
	sprintf(shortname,"PINDX%02d BIN",index+10);
	
	plf->property = 0x41;
	for(i=0;i<5;i++)
	{
		plf->name1[i] = name[i];
	}
	plf->flag = 0x0f;
	plf->reserved = 0;
	for(i=11;i>0;i--)
	{
		chksum = ((chksum&1)?0x80:0) + ((chksum&0xfe)>>1) + shortname[j++];
	}
	plf->check = chksum;
	for(i=0;i<6;i++)
	{
		plf->name2[i] = name[i+5];
	}
	plf->start = 0;
	for(i=0;i<2;i++)
	{
		plf->name3[i] = name[i+11];
	}
	return 0;
}

int fillshortindexname(struct short_format *psf,int index,time_t t,int start_cluster,int size)
{
	char name[16];
	
	//yaogang hdd
	//sprintf(name,"INDEX%02d ",index);
	memset(name,0,sizeof(name));
	sprintf(name,"INDEX%02d ",index+10);
	
	strncpy(psf->name,name,8);
	strncpy(psf->ext,"BIN",3);
	psf->property = 0x0;
	psf->reserved = 0;
	psf->ten_ms = 0;
	struct tm* ctime = localtime(&t);
	psf->create_time = CT_LE_W((unsigned short)((ctime->tm_sec >> 1) +
		(ctime->tm_min << 5) + (ctime->tm_hour << 11)));
	psf->create_date = CT_LE_W((unsigned short)(ctime->tm_mday +
		((ctime->tm_mon+1) << 5) +
		((ctime->tm_year-80) << 9)));
	psf->last_date = psf->create_date;
	psf->hi_start = CT_LE_W((unsigned short)((start_cluster)>>16));
	psf->modify_time = psf->create_time;
	psf->modify_date = psf->create_date;
	psf->low_start = CT_LE_W((unsigned short)(start_cluster));
	psf->len = CT_LE_L(size);
	return 0;
}


int filllongindexname(struct long_format *plf,int index,time_t t,int start_cluster,int size)
{
	int i,j=0,chksum=0;
	char name[16];
	char shortname[16];
	
	//yaogang hdd
	//sprintf(name,"index%02d.bin",index);
	//sprintf(shortname,"INDEX%02d BIN",index);
	memset(name,0,sizeof(name));
	memset(shortname,0,sizeof(shortname));
	sprintf(name,"index%02d.bin",index+10);
	sprintf(shortname,"INDEX%02d BIN",index+10);
	
	plf->property = 0x41;
	for(i=0;i<5;i++)
	{
		plf->name1[i] = name[i];
	}
	plf->flag = 0x0f;
	plf->reserved = 0;
	for(i=11;i>0;i--)
	{
		chksum = ((chksum&1)?0x80:0) + ((chksum&0xfe)>>1) + shortname[j++];
	}
	plf->check = chksum;
	for(i=0;i<6;i++)
	{
		plf->name2[i] = name[i+5];
	}
	plf->start = 0;
	for(i=0;i<2;i++)
	{
		plf->name3[i] = name[i+11];
	}
	return 0;
}

int fillshortformat(struct short_format *psf,int index,time_t t,int start_cluster,int file_clusters, int type)//type 0: pic 1: rec
{
	char name[16];

	if (type)
	{
		sprintf(name,"FLY%05d",index);
	}
	else
	{
		sprintf(name,"PIC%05d",index);
	}
	
	strncpy(psf->name,name,8);
	strncpy(psf->ext,"IFV",3);//strncpy(psf->ext,"MP4",3);//csp modify
	psf->property = 0x0;
	psf->reserved = 0;
	psf->ten_ms = 0;
	struct tm* ctime = localtime(&t);
	psf->create_time = CT_LE_W((unsigned short)((ctime->tm_sec >> 1) +
		(ctime->tm_min << 5) + (ctime->tm_hour << 11)));
	psf->create_date = CT_LE_W((unsigned short)(ctime->tm_mday +
		((ctime->tm_mon+1) << 5) +
		((ctime->tm_year-80) << 9)));
	psf->last_date = psf->create_date;
	psf->hi_start = CT_LE_W((unsigned short)((start_cluster+file_clusters*index)>>16));
	psf->modify_time = psf->create_time;
	psf->modify_date = psf->create_date;
	psf->low_start = CT_LE_W((unsigned short)(start_cluster+file_clusters*index));
	psf->len = CT_LE_L(FILE_VECTOR_SIZE);
	return 0;
}

int filllongformat(struct long_format *plf,int index,time_t t,int start_cluster,int file_clusters, int type)//type 0: pic 1: rec
{
	int i,j=0,chksum=0;
	char name[16];
	char shortname[16];
	//sprintf(name,"fly%05d.mp4",index);
	if (type)
	{
		sprintf(name,"fly%05d.ifv",index);
		sprintf(shortname,"FLY%05dIFV",index);//sprintf(shortname,"FLY%05dMP4",index);//csp modify
	}
	else
	{
		sprintf(name,"pic%05d.ifv",index);
		sprintf(shortname,"PIC%05dIFV",index);//sprintf(shortname,"FLY%05dMP4",index);//csp modify
	}
	plf->property = 0x41;
	for(i=0;i<5;i++)
	{
		plf->name1[i] = name[i];
	}
	plf->flag = 0x0f;
	plf->reserved = 0;
	for(i=11;i>0;i--)
	{
		chksum = ((chksum&1)?0x80:0) + ((chksum&0xfe)>>1) + shortname[j++];
	}
	plf->check = chksum;
	for(i=0;i<6;i++)
	{
		plf->name2[i] = name[i+5];
	}
	plf->start = 0;
	for(i=0;i<2;i++)
	{
		plf->name3[i] = name[i+11];
	}
	return 0;
}

int fillPicindexfilehead(unsigned char *pbyBuf,int num)
{
	memset(pbyBuf, 0, sizeof(partition_pic_index_header));

	u16 *p;
	u32 *q;

	p = (u16 *)(pbyBuf);//file_nums
	*p = CT_LE_W(num);

	//p = (u16 *)(pbyBuf+2);//file_cur_no
	//*p = CT_LE_W(0xffff);

	int verify = 0;
	int i;
	for(i=0;i<sizeof(partition_pic_index_header) - 4;i++)
	{
		verify += pbyBuf[i];
	}

	q = (u32 *)(pbyBuf+sizeof(partition_pic_index_header) - 4);
	*q = CT_LE_L(verify);

	return 0;
}

int fillindexfilehead(unsigned char *pbyBuf,int num)
{
	memset(pbyBuf,0,512);

	unsigned int *p;

	p = (unsigned int *)(pbyBuf);//op_nums
	*p = CT_LE_L(1);

	p = (unsigned int *)(pbyBuf+8);//version
	*p = CT_LE_L(1);

	p = (unsigned int *)(pbyBuf+16);//total_file_nums;			//分区文件容器数量
	*p = CT_LE_L(num);

	int i;
	for(i=0;i<=32;i++)
	{
		p = (unsigned int *)(pbyBuf + 32 + i*sizeof(chn_last_use_file));//file_no
		*p = CT_LE_L(0xffff);
	}

	int verify = 0;
	for(i=0;i<508;i++)
	{
		verify += pbyBuf[i];
	}

	p = (unsigned int *)(pbyBuf+508);
	*p = CT_LE_L(verify);

	return 0;
}

static int
write_tables (void)
{
  int x,y,i,j;
  int fat_length;
  
  fat_length = (size_fat == 32) ?
	       CF_LE_L(bs.fat32.fat32_length) : CF_LE_W(bs.fat_length);
  
	printf("%s fat_length: %d, size_fat: %d, bs.fat32.fat32_length: %d, bs.fat_length: %d\n", 
		__func__, fat_length, size_fat, bs.fat32.fat32_length, bs.fat_length);
  //printlog("write_tables : reserved sector\n");

  seekto( 0, "start of device" );
  /* clear all reserved sectors */
  for( x = 0; x < reserved_sectors; ++x )
    writebuf( blank_sector, sector_size, "reserved sector" );

  //printlog("write_tables : boot sector\n");

  /* seek back to sector 0 and write the boot sector */
  seekto( 0, "boot sector" );
  writebuf( (char *) &bs, sizeof (struct msdos_boot_sector), "boot sector" );

  //printlog("write_tables : fat sector\n");

  /* seek to start of FATS and write them all */
  seekto( reserved_sectors*sector_size, "first FAT" );

  unsigned int total_sectors = CF_LE_L(bs.total_sect);//312560640;//fat16?//3T4T
  printf("%s total_sectors: %u\n", __func__, total_sectors);
  unsigned int file_clusters = cdiv(FILE_VECTOR_SIZE, bs.cluster_size*sector_size);//3T4T
  unsigned int nr_files = (total_sectors-reserved_sectors-nr_fats*fat_length)/bs.cluster_size/file_clusters-1-1;//3T4T
  nr_files -= 2;//3T4T
  
  //yaogang modify 20141225
  nr_files -= 1;//空间留给图片索引文件
/*
图片索引文件大小= sizeof(partition_pic_index_header) \
							+ PIC_FILE_NUMS * (sizeof(file_pic_use_info) 
								+ sizeof(pic_use_info) * PIC_NUMS_PER_FILE)  = 67113024
	PIC_FILE_NUMS: 	用于保存图片文件容器数量128
	PIC_NUMS_PER_FILE: 	图片大小以8K计，文件容器中最多可写入的图片数量	32768
所以再把文件容器数量-1，腾出空间放这两个索引文件(加备份索引文件)
*/
	printf("sizeof(partition_pic_index_header): %d, sizeof(file_pic_use_info): %d, sizeof(pic_use_info): %d\n",
		sizeof(partition_pic_index_header), sizeof(file_pic_use_info),sizeof(pic_use_info));

	unsigned int pic_index_file_size = sizeof(partition_pic_index_header) \
		+ (PIC_FILE_NUMS-PRE_RECSNAP_FILES) * (sizeof(file_pic_use_info) + sizeof(pic_use_info) * PIC_NUMS_PER_FILE);
  
  	unsigned int pic_index_file_clusters = cdiv(pic_index_file_size, bs.cluster_size*sector_size);//3T4T

  unsigned int total_file_clusters = nr_files*file_clusters;//3T4T
	printf("%s pic_index_file_size: %u, pic_index_file_clusters: %u\n", 
		__func__, pic_index_file_size, pic_index_file_clusters);
  
  //512: sizeof(partition_index_header)/16: sizeof(file_use_info) /32: sizeof(segment_use_info)
  printf("sizeof(partition_index_header): %d, sizeof(file_use_info): %d, sizeof(segment_use_info): %d\n",
		sizeof(partition_index_header), sizeof(file_use_info),sizeof(segment_use_info));
  //unsigned int index_file_size = 512 + (nr_files - PIC_FILE_NUMS) * (16+NFILE_PER_SECT*32);//512+nr_files*8208;//yaogang hdd//3T4T
  unsigned int index_file_size = sizeof(partition_index_header) \
  	+ (nr_files - PIC_FILE_NUMS) * (sizeof(file_use_info) + NFILE_PER_SECT*sizeof(segment_use_info));//yaogang hdd//3T4T
  
  unsigned int index_file_clusters = cdiv(index_file_size, bs.cluster_size*sector_size);//3T4T

  printf("%s index_file_size: %u, index_file_clusters: %u\n", 
  	__func__, index_file_size, index_file_clusters);

  printf("file_clusters=%u,nr_files=%u,total_file_clusters=%u\n",file_clusters,nr_files,total_file_clusters);//3T4T

  unsigned int total_clusters = (total_sectors-reserved_sectors-nr_fats*fat_length)/bs.cluster_size;//3T4T
  
  //根目录区包含的表项:IFLYDVR短文件表项、
  //		两个索引文件的长短表项、两个图片索引文件的长短表项、////3T4T
  //		nr_files个文件容器的长短文件表项
  unsigned int root_clusters = cdiv(nr_files*64+32+2*64 + 2*64, bs.cluster_size*sector_size);
  //root_clusters += 1;//3T4T
  printf("total_clusters=%u,root_clusters=%u\n",total_clusters,root_clusters);//3T4T

  unsigned int nr_bufs = (fat_length*sector_size)/(TEST_BUFFER_BLOCKS*BLOCK_SIZE);//3T4T
  unsigned int remain = (fat_length*sector_size)%(TEST_BUFFER_BLOCKS*BLOCK_SIZE);//3T4T
  unsigned int test_buf_entries = TEST_BUFFER_BLOCKS*BLOCK_SIZE/4;//3T4T buffer中一次能写入多少个目录项
  printf("nr_bufs=%u,remain=%u,test_buf_entries=%u\n",nr_bufs,remain,test_buf_entries);//3T4T
	//nr_bufs=11175,remain=5120,test_buf_entries=4096
	
  //写FAT区，设置所有文件容器的簇链
  unsigned int *pFat = (unsigned int *)(fat+2*sector_size);

	//yaogang modify 20141225
	// FAT 文件次序
	// 2: 第0  & 第1号簇系统保留
	// root_clusters : 根目录簇链
	// IFLYDVR 文件占1簇链
	// 两个录像索引文件簇链
	// 两个图片索引文件簇链
	// nr_files 个录像/图片文件簇链
	unsigned int file_cluster_offset = 2 + root_clusters + 1 + 2*index_file_clusters + 2*pic_index_file_clusters;

// 先写nr_files 个录像/图片文件簇链	
  for (x = 0; x < nr_fats; x++)// 两个FAT表: FAT1 & FAT2
  {
	  for(y=0;y<nr_bufs;y++)
	  {
		  for(i=0;i<test_buf_entries;i++)
		  {
			  pFat[i] = y*test_buf_entries+i+1;

			  if (pFat[i] > file_cluster_offset)
			  {
				  if(pFat[i] - file_cluster_offset > total_file_clusters)
				  {
					  pFat[i] = 0;
				  }
				  else if((pFat[i] - file_cluster_offset) % file_clusters == 0)
				  {
					  pFat[i] = FAT_EOF;
				  }
			  }
		  }
		  writebuf( fat+2*sector_size, TEST_BUFFER_BLOCKS*BLOCK_SIZE, "FAT" );

		  tFormat.curpos += TEST_BUFFER_BLOCKS*BLOCK_SIZE;
	  }
	  if(remain)
	  {
		  for(i=0;i<remain/4;i++)
		  {
			  pFat[i] = nr_bufs*test_buf_entries+i+1;

			  if (pFat[i] > file_cluster_offset)
			  {
				  if(pFat[i] - file_cluster_offset> total_file_clusters)
				  {
					  pFat[i] = 0;
				  }
				  else if((pFat[i] - file_cluster_offset) % file_clusters == 0)
				  {
					  pFat[i] = FAT_EOF;
				  }
			  }
		  }
		  writebuf( fat+2*sector_size, remain, "FAT" );

		  tFormat.curpos += remain;
	  }
  }

//再写之前的文件簇链
	for(x = 0; x < nr_fats; x++)
	{
		seekto( reserved_sectors*sector_size+x*fat_length*sector_size, "FAT" );

		pFat[0] = 0x0ffffff8;
		pFat[1] = 0x0fffffff;
		writebuf( fat+2*sector_size, 8, "FAT" );

		//写FAT区，设置根目录区的簇链，根目录区在FAT32文件系统中其实也是一个文件
		for(i=0;i<root_clusters-1;i++)
		{
			pFat[i] = 2+i+1;
		}
		
		pFat[i] = FAT_EOF;
		writebuf( fat+2*sector_size, root_clusters*4, "FAT" );

		//写FAT区，设置IFLYDVR文件的簇链，IFLYDVR文件占一簇
		pFat[0] = FAT_EOF;
		writebuf( fat+2*sector_size, 4, "FAT" );

		//写FAT区，设置两个录像索引文件的簇链
		for(i=0;i<2;i++)
		{
			for(j=0;j<index_file_clusters-1;j++)
			{
				pFat[j] = 2+root_clusters+1+i*index_file_clusters+j+1;
			}
			
			pFat[j] = FAT_EOF;
			writebuf( fat+2*sector_size, index_file_clusters*4, "FAT" );
		}

		//yaogang modify 20141225 
		//写FAT区，设置两个图片索引文件的簇链
		for(i=0;i<2;i++)
		{
			for(j=0;j<pic_index_file_clusters-1;j++)
			{
				pFat[j] = 2+root_clusters+1+ 2*index_file_clusters + i*pic_index_file_clusters+j+1;
			}
			
			pFat[j] = FAT_EOF;
			writebuf( fat+2*sector_size, pic_index_file_clusters*4, "FAT" );
		}
	}

  //写根目录
  //printlog("write_tables : root directory\n");
  
  seekto(reserved_sectors*sector_size+nr_fats*fat_length*sector_size, "root directory");
  
  struct short_format sf;
  struct long_format  lf;
  time_t t = time(NULL);
  
  //写根目录，IFLYDVR 短文件名目录项
  fillshortDVRname(&sf, t, 2+root_clusters, 512);
  writebuf(&sf, sizeof(sf), "sf");
  
  //写根目录，两个录像索引文件长/短文件名目录项
  for(i = 0; i < 2; i++)
  {
	  filllongindexname(&lf, i, t, 2 + root_clusters + 1 + i * index_file_clusters, index_file_size);
	  fillshortindexname(&sf, i, t, 2 + root_clusters + 1 + i * index_file_clusters, index_file_size);
	  writebuf(&lf, sizeof(lf), "lf");
	  writebuf(&sf, sizeof(sf), "sf");
  }

  //写根目录，两个图片索引文件长/短文件名目录项
  for(i = 0; i < 2; i++)
  {
	  filllongPicindexname(&lf, i, t, 2 + root_clusters + 1 + 2*index_file_clusters + i * pic_index_file_clusters, pic_index_file_size);
	  fillshortPicindexname(&sf, i, t, 2 + root_clusters + 1 + 2*index_file_clusters + i * pic_index_file_clusters, pic_index_file_size);
	  writebuf(&lf, sizeof(lf), "lf");
	  writebuf(&sf, sizeof(sf), "sf");
  }

  //写根目录，所有图片文件长/短文件名目录项
  for(i=0;i<PIC_FILE_NUMS;i++)
  {
	filllongformat(&lf,i,t,2+root_clusters+1+2*index_file_clusters + 2*pic_index_file_clusters, file_clusters, 0);
	fillshortformat(&sf,i,t,2+root_clusters+1+2*index_file_clusters + 2*pic_index_file_clusters, file_clusters, 0);	
	writebuf(&lf,sizeof(lf),"lf");
	writebuf(&sf,sizeof(sf),"sf");
  }
  //写根目录，所有录像文件长/短文件名目录项
  for(i=0;i<nr_files-PIC_FILE_NUMS;i++)
  {
	filllongformat(&lf,i,t,2+root_clusters+1+2*index_file_clusters + 2*pic_index_file_clusters+PIC_FILE_NUMS*file_clusters, file_clusters, 1);
	fillshortformat(&sf,i,t,2+root_clusters+1+2*index_file_clusters + 2*pic_index_file_clusters+PIC_FILE_NUMS*file_clusters, file_clusters, 1);	
	writebuf(&lf,sizeof(lf),"lf");
	writebuf(&sf,sizeof(sf),"sf");
  }
  
  //写根目录区的根目录区占据的不足簇的地方全部用0补齐
  memset(fat+2*sector_size,0,sector_size);
  remain = sector_size - ((nr_files*64+32+2*64+ 2*64)%sector_size);
  if(remain && remain != sector_size)//if(remain)//3T4T
  {
	writebuf(fat+2*sector_size,remain,"remain");
  }
  int nr_sectors = root_clusters*bs.cluster_size-cdiv(nr_files*64+32+2*64 + 2*64,sector_size);
  for(i=0;i<nr_sectors;i++)
  {
	writebuf(fat+2*sector_size,sector_size,"root");
  }

  //printlog("write_tables : info sector\n");
  
  struct fat32_fsinfo *info = (struct fat32_fsinfo *)(info_sector + 0x1e0);
  info->free_clusters = total_clusters-root_clusters-total_file_clusters-1-2*index_file_clusters - 2*pic_index_file_clusters;//CT_LE_L(total_clusters-root_clusters-total_file_clusters-1-2*index_file_clusters);//3T4T
  info->next_cluster = 2+root_clusters+total_file_clusters+1+2*index_file_clusters+2*pic_index_file_clusters;//2+root_clusters+total_file_clusters+1+2*index_file_clusters-1;//3T4T
  printf("free_clusters=%u,next_cluster=%u\n",info->free_clusters,info->next_cluster);//3T4T
  
  /* on FAT32, write the info sector and backup boot sector */
  if (size_fat == 32)
  {
      seekto( CF_LE_W(bs.fat32.info_sector)*sector_size, "info sector" );
      writebuf( info_sector, 512, "info sector" );
      if (backup_boot != 0)
	  {
		  seekto( backup_boot*sector_size, "backup boot sector" );
		  writebuf( (char *) &bs, sizeof (struct msdos_boot_sector),
			  "backup boot sector" );
	  }
  }
  
  //写IFLYDVR文件的内容
  seekto(reserved_sectors*sector_size+nr_fats*fat_length*sector_size+root_clusters*bs.cluster_size*sector_size,"dvr");
  memset(fat+2*sector_size, 0x63, 512);
  writebuf(fat+2*sector_size, 512, "dvr");
  
  //写索引文件的内容
  for(i=0;i<2;i++)
  {
  	//定位到index0x.bin文件头
	  seekto(reserved_sectors*sector_size+nr_fats*fat_length*sector_size+(root_clusters+1+i*index_file_clusters)*bs.cluster_size*sector_size,"index");

	  fillindexfilehead(fat+2*sector_size,nr_files - PIC_FILE_NUMS);
	  writebuf(fat+2*sector_size,512,"index");

	  memset(fat+2*sector_size,0x0,16);
	  *(fat+2*sector_size+4) = 0xff;//chn_no
	  *(fat+2*sector_size+5) = 0xff;//busy
	  for(j=0;j<nr_files-PIC_FILE_NUMS;j++)
	  {
		  *((unsigned int *)(fat+2*sector_size)) = CT_LE_L(j);//file_no
		  writebuf(fat+2*sector_size,16,"index");
	  }

	  ////yaogang hdd
	  //memset(fat+2*sector_size,0x0,8192);
	  memset(fat+2*sector_size, 0x0, NFILE_PER_SECT*32);
	  for(j=0;j<nr_files-PIC_FILE_NUMS;j++)
	  {
	      //yaogang hdd
	      //writebuf(fat+2*sector_size,8192,"index");
	      writebuf(fat+2*sector_size, NFILE_PER_SECT*32,"index");
	  }
  }

  //写图片索引文件的内容
   for(i=0;i<2;i++)
   {
	   seekto(reserved_sectors*sector_size+nr_fats*fat_length*sector_size+(root_clusters+1+2*index_file_clusters + i*pic_index_file_clusters)*bs.cluster_size*sector_size,"pic_index");
  
	   fillPicindexfilehead(fat+2*sector_size, PIC_FILE_NUMS-PRE_RECSNAP_FILES);//预录使用2个
	   writebuf(fat+2*sector_size, sizeof(partition_pic_index_header),"pic_index");
  
	   memset(fat+2*sector_size,0x0, sizeof(file_pic_use_info));
	   //*(fat+2*sector_size+4) = 0xff;
	   //*(fat+2*sector_size+5) = 0xff;
	   //*((u32 *)(fat+2*sector_size+8)) = 0xffffffff;//naxt_pic_pos
	    
	   for(j=0;j<PIC_FILE_NUMS-PRE_RECSNAP_FILES;j++)
	   {
		   *((u16 *)(fat+2*sector_size)) = CT_LE_W(j);
		   writebuf(fat+2*sector_size, sizeof(file_pic_use_info),"pic_index");
	   }
  
	   //yaogang hdd 耗时太长，也没有必要清零
	   #if 0
	   //memset(fat+2*sector_size,0x0,8192);
	   printf("%s 1\n", __FUNCTION__);
	   memset(fat+2*sector_size, 0x0, sizeof(pic_use_info));
	   for(j=0;j<PIC_FILE_NUMS*PIC_NUMS_PER_FILE;j++)
	   {
		   //yaogang hdd
		   //writebuf(fat+2*sector_size,8192,"index");
		   writebuf(fat+2*sector_size, sizeof(pic_use_info),"pic_index");
	   }
	   printf("%s 2\n", __FUNCTION__);
	   #endif
   }

  

  if (blank_sector)
  	free( blank_sector );
  if (info_sector)
  	free( info_sector );
  free (root_dir);   /* Free up the root directory space from setup_tables */
  free (fat);  /* Free up the fat table space reserved during setup_tables */

  //printlog("write_tables : end\n");

  return 0;
}

/* The "main" entry point into the utility - we pick up the options and attempt to process them in some sort of sensible
   way.  In the event that some/all of the options are invalid we need to tell the user so that something can be done! */

void reset_sector_size(int size)//wrchen 090914
{
	printf("resize sector_size,default sector_size=%d,new=%d\n",sector_size,size);
  	sector_size = size;
}

//yaogang modify 20170328
/*
 * Get start offset of partition
 */
int blkdev_get_start(int fd, unsigned int *s)
{
	struct hd_geometry geometry;

	if (ioctl(fd, HDIO_GETGEO, &geometry) == 0) 
	{
		*s = geometry.start;
		return 0;
	}
	else
	{
		*s = 0;
		printf("%s ioctl HDIO_GETGEO failed, errno(%d, %s)\n",
			__func__, errno, strerror(errno));
	}
	
	return -1;
}


int
mkfsdos_main (int argc, char **argv)
{
  //int c;
  //char *listfile = NULL;
  //FILE *msgfile;
  struct stat statbuf;
  //int i = 0, pos, ch;
  //int create = 0;
  //unsigned long long cblocks;
  
  if (argc && *argv) {		/* What's the program name? */
    char *p;
    program_name = *argv;
    if ((p = strrchr( program_name, '/' )))
	program_name = p+1;
  }
  
  //printf("format:setup 0\n");
  //printlog("format:setup 0\n");

  time(&create_time);

  volume_id = (long)create_time;	/* Default volume ID = creation time */
  
  printf ("%s " VERSION " (" VERSION_DATE ")\n", program_name);

  device_name = argv[1];  /* Determine the number of blocks in the FS */
  
  /*cblocks = count_blocks (device_name);
  if(cblocks == 0)
  {
	  printf( "%s count_blocks failed\n", device_name );
	  return -1;
  }
  blocks = cblocks;*/

  blocks = count_blocks (device_name); /*  Have a look and see! */
  if(blocks == 0)
  {
	  printf( "%s count_blocks failed\n", device_name );
	  return -1;
  }
  //blocks = blocks;
  
  printf("device_name=%s,BLOCK_SIZE=%d,blocks=(%x,%x)\n",device_name,BLOCK_SIZE,(int)(blocks>>32),(int)blocks);

  if(check_mount (device_name))	/* Is the device already mounted? */
  {
	  return -1;
  }
  dev = open (device_name, O_RDWR);	/* Is it a suitable device to build the FS on? */
  if (dev < 0)
  {
	  //die ("unable to open %s");
	  printf("unable to open %s\n",device_name);
	  return -1;
  }
  
  if (fstat(dev, &statbuf) < 0)
  {
	  //die ("unable to stat %s");
	  printf ("unable to stat %s\n",device_name);
	  close(dev);
	  return -1;
  }
  if (!S_ISBLK (statbuf.st_mode)) {
	printf("not block device\n");
    statbuf.st_rdev = 0;
    check = 0;
  }
  else
    /*
     * Ignore any 'full' fixed disk devices, if -I is not given.
     * On a MO-disk one doesn't need partitions.  The filesytem can go
     * directly to the whole disk.  Under other OSes this is known as
     * the 'superfloppy' format.  As I don't know how to find out if
     * this is a MO disk I introduce a -I (ignore) switch.  -Joey
     */
  {
  	//st_rdev  块设备号: 主/子设备号
	  printf("ignore_full_disk=%d, (statbuf.st_rdev & 0xff3f)=0x%04x\n", ignore_full_disk, (int)(statbuf.st_rdev & 0xff3f));
	  
	  if (!ignore_full_disk && (
		  (statbuf.st_rdev & 0xff3f) == 0x0300 || /* hda, hdb */
		  (statbuf.st_rdev & 0xff0f) == 0x0800 || /* sd */
		  (statbuf.st_rdev & 0xff3f) == 0x0d00 || /* xd */
		  (statbuf.st_rdev & 0xff3f) == 0x1600 )  /* hdc, hdd */
		  )
	  {
		  //die ("Will not try to make filesystem on full-disk device '%s' (use -I if wanted)");
		  printf("Will not try to make filesystem on full-disk device '%s' (use -I if wanted)\n",device_name);
		  close(dev);
		  return -1;
	  }
  }
  
  printf("###sizeof(st_rdev)=%d,sizeof(st_size)=%d,st_size=%ld,sizeof(st_blksize)=%d,st_blksize=%ld,sizeof(st_blocks)=%d,st_blocks=%ld\n",
	sizeof(statbuf.st_rdev),
	sizeof(statbuf.st_size),statbuf.st_size,
	sizeof(statbuf.st_blksize),statbuf.st_blksize,
	sizeof(statbuf.st_blocks),statbuf.st_blocks);
  
  //printf("format:setup 1\n");
  //printlog("format:setup 1\n");
  
	//yaogang modify 20170328
	hidden_sectors = 0;

	unsigned int partition_offset;
	if (blkdev_get_start(dev, &partition_offset) == 0)
	{
		hidden_sectors = partition_offset;
	}
	printf("%s partition: %s hidden_sectors: %u\n",
		__func__, device_name, hidden_sectors);
	//yaogang modify 20170328

  if(establish_params (statbuf.st_rdev,statbuf.st_size))	
                                /* Establish the media parameters */
  {
	  printf("establish_params failed\n");
	  close(dev);
	  return -1;
  }

  //printf("format:setup 2\n");
  //printlog("format:setup 2\n");

  if(setup_tables ())		/* Establish the file system tables */
  {
	  printf("setup_tables failed\n");
	  close(dev);
	  return -1;
  }

  //printf("format:setup 3\n");
  //printlog("format:setup 3\n");

#if 1
  if(write_tables ())		/* Write the file system tables away! */
  {
	printf("write_tables failed\n");
	close(dev);
	return -1;
  }
#else
  int fat_length;
  FILE *fp;
  int ret;
  int i;
  fat_length = (size_fat == 32) ?
	  CF_LE_L(bs.fat32.fat32_length) : CF_LE_W(bs.fat_length);
	printf("fat_length=%d,sector_size=%d,nr_fats=%d,reserved_sectors=%d\n",fat_length,sector_size,nr_fats,reserved_sectors);
	printf("total_sect=%d,blocks=%ld,cluster_size=%d\n",CF_LE_L(bs.total_sect),(long)blocks,bs.cluster_size);
	
	fp = fopen("dbr","wb");
	seekto(0,"dbr");
	ret = read(dev,blank_sector,sector_size);
	printf("read dbr ret=%d\n",ret);
	fwrite(blank_sector,1,sector_size,fp);
	fclose(fp);
	
	fp = fopen("info","wb");
	seekto(bs.fat32.info_sector*sector_size,"info");
	ret = read(dev,blank_sector,sector_size);
	printf("read info ret=%d\n",ret);
	fwrite(blank_sector,1,sector_size,fp);
	fclose(fp);

	/*海康250G*/
	//fat_length = 14905;
	//bs.cluster_size = 64;
	/*海康250G*/

	fp = fopen("fat1","wb");
	seekto( reserved_sectors*sector_size, "first FAT" );
	for(i=0;i<fat_length;i++)
	{
		ret = read(dev,blank_sector,sector_size);
		fwrite(blank_sector,1,sector_size,fp);
	}
	fclose(fp);

	fp = fopen("fat2","wb");
	for(i=0;i<fat_length;i++)
	{
		ret = read(dev,blank_sector,sector_size);
		fwrite(blank_sector,1,sector_size,fp);
	}
	fclose(fp);

	fp = fopen("root","wb");
	seekto( (reserved_sectors+nr_fats*fat_length)*sector_size, "root" );
	for(i=0;i<bs.cluster_size;i++)
	{
		ret = read(dev,blank_sector,sector_size);
		fwrite(blank_sector,1,sector_size,fp);
	}
	fclose(fp);

	/*fp = fopen("fly00000.ifv","wb");
	for(i=0;i<2048;i++)
	{
		ret = read(dev,blank_sector,sector_size);
		fwrite(blank_sector,1,sector_size,fp);
	}
	fclose(fp);*/

	if (blank_sector) free( blank_sector );
	if (info_sector) free( info_sector );
	free (root_dir);   /* Free up the root directory space from setup_tables */
	free (fat);  /* Free up the fat table space reserved during setup_tables */
#endif

  //printf("format:setup 4\n");
  //printlog("format:setup 4\n");
  
  close(dev);

  sync();
  
  //printf("format:setup 5\n");
  //printlog("format:setup 5\n");

  //exit (0);			/* Terminate with no errors! */
  return 0;
}

s64 GetSizePart(int argc,char **argv)
{
  //int c;
  //char *listfile = NULL;
  //FILE *msgfile;
  #if 1
  struct stat statbuf;
  //int i = 0, pos, ch;
  //int create = 0;
  //unsigned long long cblocks;
  #if 0						//000000
  if (argc && *argv) {		/* What's the program name? */
    char *p;
    program_name = *argv;
    if ((p = strrchr( program_name, '/' )))
	program_name = p+1;
  }
  #endif
  //printf("format:setup 0\n");
  //printlog("format:setup 0\n");

  time(&create_time);

  volume_id = (long)create_time;	/* Default volume ID = creation time */
  
  //printf ("%s " VERSION " (" VERSION_DATE ")\n", program_name);

  device_name = argv[1];  /* Determine the number of blocks in the FS */
  
  /*cblocks = count_blocks (device_name);
  if(cblocks == 0)
  {
	  printf( "%s count_blocks failed\n", device_name );
	  return -1;
  }
  blocks = cblocks;*/

  blocks = count_blocks (device_name); /*  Have a look and see! */
  if(blocks == 0)
  {
	  printf( "%s count_blocks failed\n", device_name );
	  return -1;
  }
  //blocks = blocks;
  
  //printf("device_name=%s,BLOCK_SIZE=%d,blocks=(%x,%x)\n",device_name,BLOCK_SIZE,(int)(blocks>>32),(int)blocks);
#if 0
  if(check_mount (device_name))	/* Is the device already mounted? */
  {
	  return -1;
  }
#endif
  device_name = argv[1];  /* Determine the number of blocks in the FS */
  
  dev = open (device_name, O_RDWR);	/* Is it a suitable device to build the FS on? */
  if (dev < 0)
  {
	  //die ("unable to open %s");
	  printf("unable to open %s\n",device_name);
	  return -1;
  }
  
  if (fstat (dev, &statbuf) < 0)
  {
	  //die ("unable to stat %s");
	  printf ("unable to stat %s\n",device_name);
	  close(dev);
	  return -1;
  }
  #if 0//000000
  if (!S_ISBLK (statbuf.st_mode)) {
	printf("not block device\n");
    statbuf.st_rdev = 0;
    check = 0;
  }
  else
    /*
     * Ignore any 'full' fixed disk devices, if -I is not given.
     * On a MO-disk one doesn't need partitions.  The filesytem can go
     * directly to the whole disk.  Under other OSes this is known as
     * the 'superfloppy' format.  As I don't know how to find out if
     * this is a MO disk I introduce a -I (ignore) switch.  -Joey
     */
  {
	  printf("ignore_full_disk=%d,(statbuf.st_rdev & 0xff3f)=0x%04x\n",ignore_full_disk,(int)(statbuf.st_rdev & 0xff3f));
	  
	  if (!ignore_full_disk && (
		  (statbuf.st_rdev & 0xff3f) == 0x0300 || /* hda, hdb */
		  (statbuf.st_rdev & 0xff0f) == 0x0800 || /* sd */
		  (statbuf.st_rdev & 0xff3f) == 0x0d00 || /* xd */
		  (statbuf.st_rdev & 0xff3f) == 0x1600 )  /* hdc, hdd */
		  )
	  {
		  //die ("Will not try to make filesystem on full-disk device '%s' (use -I if wanted)");
		  printf("Will not try to make filesystem on full-disk device '%s' (use -I if wanted)\n",device_name);
		  close(dev);
		  return -1;
	  }
  }
  
  printf("###sizeof(st_rdev)=%d,sizeof(st_size)=%d,st_size=%ld,sizeof(st_blksize)=%d,st_blksize=%ld,sizeof(st_blocks)=%d,st_blocks=%ld\n",
	sizeof(statbuf.st_rdev),
	sizeof(statbuf.st_size),statbuf.st_size,
	sizeof(statbuf.st_blksize),statbuf.st_blksize,
	sizeof(statbuf.st_blocks),statbuf.st_blocks);
  
  //printf("format:setup 1\n");
  //printlog("format:setup 1\n");
#endif
  if(establish_params (statbuf.st_rdev,statbuf.st_size))	
                                /* Establish the media parameters */
  {
	  printf("establish_params failed\n");
	  close(dev);
	  return -1;
  }

  //printf("format:setup 2\n");
  //printlog("format:setup 2\n");

  if(setup_tables ())		/* Establish the file system tables */
  {
	  printf("setup_tables failed\n");
	  close(dev);
	  return -1;
  }
  close(dev);
  #endif

  #if 1
  int fat_length;
  
  fat_length = (size_fat == 32) ?
	       CF_LE_L(bs.fat32.fat32_length) : CF_LE_W(bs.fat_length);

  return (s64)(nr_fats* fat_length*sector_size);
  #endif
}

/* That's All Folks */
/* Local Variables: */
/* tab-width: 8     */
/* End:             */
