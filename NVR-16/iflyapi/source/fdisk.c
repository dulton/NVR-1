/* fdisk.c -- Partition table manipulator for Linux.
 *
 * Copyright (C) 1992  A. V. Le Blanc (LeBlanc@mcc.ac.uk)
 * Copyright (C) 2001,2002 Vladimir Oleynik <dzo@simtreas.ru> (initial bb port)
 *
 * Licensed under the GPL v2 or later, see the file LICENSE in this tarball.
 */

/*csp*/

#include "bbfunc.h"
#include <common.h>

#include <platform.h>

//#include <linux/fs.h> /*BLKGETSIZE*/

#define CONFIG_FEATURE_FDISK_WRITABLE

/*csp*/

#define _(x) x

#define PROC_PARTITIONS "/proc/partitions"

#include <features.h>
#include <sys/types.h>
#include <sys/stat.h>           /* stat */
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
//#include <setjmp.h>
//#include <assert.h>             /* assert */
//#include <getopt.h>
#include <endian.h>
#include <sys/ioctl.h>
#include <sys/param.h>
#include <sys/sysmacros.h>     /* major */

#include <stdint.h>        /* for uint32_t, uint16_t, uint8_t, int16_t, etc */

#include <sys/utsname.h>



#define DEFAULT_SECTOR_SIZE     512
#define MAX_SECTOR_SIZE 2048
#define SECTOR_SIZE     512     /* still used in BSD code */
#define MAXIMUM_PARTS   60

#define ACTIVE_FLAG     0x80

#define EXTENDED        0x05
#define WIN98_EXTENDED  0x0f
#define LINUX_PARTITION 0x81
#define LINUX_SWAP      0x82
#define LINUX_NATIVE    0x83
#define LINUX_EXTENDED  0x85
#define LINUX_LVM       0x8e
#define LINUX_RAID      0xfd

#define SUNOS_SWAP 3
#define WHOLE_DISK 5

#define SIZE(a) (sizeof(a)/sizeof((a)[0]))

//void printlog(char *msg);

/* including <linux/hdreg.h> also fails */
struct hd_geometry {
	unsigned char heads;
	unsigned char sectors;
	unsigned short cylinders;
	unsigned long start;
};

#define HDIO_GETGEO             0x0301  /* get device geometry */

/*
 * Raw disk label. For DOS-type partition tables the MBR,
 * with descriptions of the primary partitions.
 */
#if (MAX_SECTOR_SIZE) > (BUFSIZ+1)
static char MBRbuffer[MAX_SECTOR_SIZE];
#else
# define MBRbuffer bb_common_bufsiz1
#endif

struct partition {
	unsigned char boot_ind;         /* 0x80 - active */
	unsigned char head;             /* starting head */
	unsigned char sector;           /* starting sector */
	unsigned char cyl;              /* starting cylinder */
	unsigned char sys_ind;          /* What partition type */
	unsigned char end_head;         /* end head */
	unsigned char end_sector;       /* end sector */
	unsigned char end_cyl;          /* end cylinder */
	unsigned char start4[4];        /* starting sector counting from 0 */
	unsigned char size4[4];         /* nr of sectors in partition */
} ATTRIBUTE_PACKED;

struct my_partition {
	unsigned char boot_ind;         /* 0x80 - active */
	unsigned char head;             /* starting head */
	unsigned char sector;           /* starting sector */
	unsigned int  cyl;              /* starting cylinder */
	unsigned char sys_ind;          /* What partition type */
	unsigned char end_head;         /* end head */
	unsigned char end_sector;       /* end sector */
	unsigned int  end_cyl;          /* end cylinder */
	unsigned int  start;			/* starting sector counting from 0 */
	unsigned int  size;				/* nr of sectors in partition */
} ;

static int fd;                  /* the disk */
//static int partitions = 4;      /* maximum partition + 1 */	//3T4T
static uint sector_size = DEFAULT_SECTOR_SIZE;
static uint sector_offset = 1;
static uint heads, sectors, cylinders;
static uint pt_heads, pt_sectors;
static uint kern_heads, kern_sectors;
static unsigned long long total_number_of_sectors;

#define pt_offset(b, n) ((struct partition *)((b) + 0x1be + \
				(n) * sizeof(struct partition)))
#define sector(s)       ((s) & 0x3f)
#define cylinder(s, c)  ((c) | (((s) & 0xc0) << 2))

/*static unsigned int
read4_little_endian(const unsigned char *cp)
{
	return (uint)(cp[0]) + ((uint)(cp[1]) << 8)
		+ ((uint)(cp[2]) << 16) + ((uint)(cp[3]) << 24);
}

static int32_t
get_start_sect(const struct partition *p)
{
	return read4_little_endian(p->start4);
}

static int32_t
get_nr_sects(const struct partition *p)
{
	return read4_little_endian(p->size4);
}*/

static void
store4_little_endian(unsigned char *cp, unsigned int val)
{
	cp[0] = (val & 0xff);
	cp[1] = ((val >> 8) & 0xff);
	cp[2] = ((val >> 16) & 0xff);
	cp[3] = ((val >> 24) & 0xff);
}

static void
set_start_sect(struct partition *p, unsigned int start_sect)
{
	store4_little_endian(p->start4, start_sect);
}

static void
set_nr_sects(struct partition *p, int32_t nr_sects)
{
	store4_little_endian(p->size4, nr_sects);
}

/*
 * per partition table entry data
 *
 * The four primary partitions have the same sectorbuffer (MBRbuffer)
 * and have NULL ext_pointer.
 * Each logical partition table entry has two pointers, one for the
 * partition and one link to the next one.
 */
static struct pte {
	struct partition *part_table;   /* points into sectorbuffer */
	struct partition *ext_pointer;  /* points into sectorbuffer */
#ifdef CONFIG_FEATURE_FDISK_WRITABLE
	char changed;           /* boolean */
#endif
	off_t offset;            /* disk sector number */
	char *sectorbuffer;     /* disk sector contents */
} ptes[MAXIMUM_PARTS];

static void
write_part_table_flag(char *b)
{
	b[510] = 0x55;
	b[511] = 0xaa;
}

static int
valid_part_table_flag(const char *mbuffer) {
	const unsigned char *b = (const unsigned char *)mbuffer;
	return (b[510] == 0x55 && b[511] == 0xaa);
}

static void
get_sectorsize(void)
{
	int arg;
	if(ioctl(fd, BLKSSZGET, &arg) == 0)
	{
		sector_size = arg;
		printf("get_sectorsize:sector_size=%d\n",sector_size);
	}
	else
	{
		printf("get_sectorsize failed\n");
	}
	if(sector_size != DEFAULT_SECTOR_SIZE)
	{
		printf(_("Note: sector size is %d (not %d)\n"), sector_size, DEFAULT_SECTOR_SIZE);
	}
}

static inline void
get_kernel_geometry(void)
{
	struct hd_geometry geometry;
	
	if(!ioctl(fd, HDIO_GETGEO, &geometry))
	{
		kern_heads = geometry.heads;
		kern_sectors = geometry.sectors;
		/* never use geometry.cylinders - it is truncated */
		printf("get_kernel_geometry:kern_heads=%d,kern_sectors=%d\n", kern_heads, kern_sectors);
	}
	else
	{
		printf("get_kernel_geometry failed\n");
	}
}

static void
get_partition_table_geometry(void)
{
	const unsigned char *bufp = (const unsigned char *)MBRbuffer;
	struct partition *p;
	int i, h, s, hh, ss;
	int first = 1;
	int bad = 0;
	
	if (!(valid_part_table_flag((char*)bufp)))
		return;
	
	hh = ss = 0;
	for (i = 0; i < 4; i++) {
		p = pt_offset(bufp, i);
		if (p->sys_ind != 0) {
			h = p->end_head + 1;
			s = (p->end_sector & 077);
			if (first) {
				hh = h;
				ss = s;
				first = 0;
			} else if (hh != h || ss != s)
				bad = 1;
		}
	}
	
	if (!first && !bad) {
		pt_heads = hh;
		pt_sectors = ss;
	}
	
	printf("get_partition_table_geometry:pt_heads=%d,pt_sectors=%d\n",pt_heads,pt_sectors);
}

extern void reset_sector_size();//wrchen 090914

static void
get_geometry(void)
{
	int sec_fac;
	unsigned long long bytes;       /* really u64 */
	
	get_sectorsize();
	sec_fac = sector_size / 512;
	
	heads = cylinders = sectors = 0;
	kern_heads = kern_sectors = 0;
	pt_heads = pt_sectors = 0;
	
	get_kernel_geometry();
	get_partition_table_geometry();

	//磁头数
	heads = pt_heads ? pt_heads : kern_heads ? kern_heads : 255;
	//每磁道扇区数
	sectors = pt_sectors ? pt_sectors : kern_sectors ? kern_sectors : 63;
	
	if (ioctl(fd, BLKGETSIZE64, &bytes) == 0) {
		/* got bytes */
	} else {
		unsigned long longsectors;
		
		if (ioctl(fd, BLKGETSIZE, &longsectors))
			longsectors = 0;
		bytes = ((unsigned long long) longsectors) << 9;
	}
	
	if(bytes<9000000000LL)//wrchen 090914
	{
		printf("get_geometry bytes=%llu???\n",bytes);
		//reset_sector_size(512);//3T4T
	}else if(bytes<17000000000LL)
	{
		printf("get_geometry bytes=%llu!!!\n",bytes);
		//reset_sector_size(1024);//3T4T
	}
	total_number_of_sectors = (bytes >> 9);
	
	/*char msg[256];
	sprintf(msg,"\ntotal_number_of_sectors=%ld\n",(long)total_number_of_sectors);
	printlog(msg);*/
	
	sector_offset = sectors;
	
	cylinders = total_number_of_sectors / (heads * sectors * sec_fac);
	
	printf("%s: cylinders=%d, heads=%d, sectors=%d, sector_size=%d, sec_fac=%d, sector_offset=%d, total_number_of_sectors=%lld\n",
		__func__, cylinders, heads, sectors, sector_size, sec_fac, sector_offset, total_number_of_sectors);
	
	//3T4T
	if(cylinders > 2*(267349-1))
	{
		cylinders = 2*(267349-1);
		printf("adjust cylinders=%d\n",cylinders);
	}
}

static void
reread_partition_table()
{
	int error = 0;
	int i;
	
	printf(_("Calling ioctl() to re-read partition table.\n"));
	sync();
	sleep(2);
	if ((i = ioctl(fd, BLKRRPART)) != 0) {
		error = errno;
	} else {
	/* some kernel versions (1.2.x) seem to have trouble
	rereading the partition table, but if asked to do it
		twice, the second time works. - biro@yggdrasil.com */
		sync();
		sleep(2);
		if ((i = ioctl(fd, BLKRRPART)) != 0)
		{
			error = errno;
		}
	}
	
	if (i) {
		printf(_("\nWARNING: Re-reading the partition table "
			"failed with error %d: %s.\n"
			"The kernel still uses the old table.\n"
			"The new table will be used "
			"at the next reboot.\n"),
			error, strerror(error));
		//printlog("Re-reading the partition table failed\n");
	}
	else {
		//printlog("Re-reading the partition table success\n");
	}
}

#include "hddcmd.h"

int ifly_format(char *path,int fs)
{
	return 0;
}

typedef struct 
{
	long long curpos;
	long long totalpos;		
} parition_prog;

static parition_prog part_prog;

int get_parition_prog(void* prog)
{
	if(prog)
	{
		memcpy(prog, &part_prog, sizeof(part_prog));
	}
	else
	{
		return -1;
	}
	
	return 0;
}

void set_parition_prog(parition_prog* prog)
{
	memcpy(&part_prog, prog, sizeof(part_prog));	
}

int ifly_partition(char *path,int count,int fs)
{
	//3T4T
	//if(path == NULL || count == 0 || count > 4 || fs == 0)
	if(path == NULL || count == 0 || count > 2 || fs == 0)
	{
		printf("ifly_partition:param error,count=%d\n",count);//3T4T
		return -1;
	}
	
	part_prog.curpos = 0;
	part_prog.totalpos = count;
	
	//partitions = 4;//3T4T
	
	fd = open(path,O_RDWR);
	if(fd == -1)
	{
		printf("%s is not exist\n",path);
		return -1;
	}

	printf("%s disk: %s, count: %d, fs: %d, MAX_SECTOR_SIZE: %d, BUFSIZ: %d\n",
		__func__, path, count, fs, MAX_SECTOR_SIZE, BUFSIZ);
	
	if(512 != read(fd, MBRbuffer, 512))
	{
		printf("read MBR failed\n");
		close(fd);
		return -1;
	}
	
	//memset(MBRbuffer, 0, 512);//3T4T
	
	/*FILE *fp = fopen("mbr","wb");
	if(fp != NULL)
	{
		fwrite(MBRbuffer,1,512,fp);
		fclose(fp);
	}*/
	
	int i;
	for(i = 0; i < 4; i++)
	{
		struct pte *pe = &ptes[i];
		pe->part_table = pt_offset(MBRbuffer, i);
		pe->ext_pointer = NULL;
		pe->offset = 0;
		pe->sectorbuffer = MBRbuffer;
#ifdef CONFIG_FEATURE_FDISK_WRITABLE
		pe->changed = 0;
#endif
	}
	
	/*for(i=0;i<4;i++)
	{
		struct pte *pe = &ptes[i];
		struct partition *pt = pe->part_table;
		printf("\n");
		printf("partition%d info:\n",i+1);
		printf("\tboot_ind:%d\n",pt->boot_ind);
		printf("\tstart_head:%d\n",pt->head);
		printf("\tstart_sector:%d\n",sector(pt->sector));
		printf("\tstart_cyl:%d\n",cylinder(pt->sector,pt->cyl));
		printf("\tsys_ind:%d\n",pt->sys_ind);
		printf("\tend_head:%d\n",pt->end_head);
		printf("\tend_sector:%d\n",sector(pt->end_sector));
		printf("\tend_cyl:%d\n",cylinder(pt->end_sector,pt->end_cyl));
		printf("\tstart_sect:%d\n",get_start_sect(pt));
		printf("\tnr_sects:%d\n",get_nr_sects(pt));
		printf("\n");
		
		char msg[256];
		sprintf(msg,"\npartition%d info:\n",i+1);
		printlog(msg);
		
		sprintf(msg,"\tstart_sect:%d\n",get_start_sect(pt));
		printlog(msg);
		
		sprintf(msg,"\tnr_sects:%d\n",get_nr_sects(pt));
		printlog(msg);
	}*/
	
	get_geometry();
	
	if(!cylinders)
	{
		printf("get_geometry failed\n");
		close(fd);
		return -1;
	}
	
	struct my_partition my_pts[4];
	memset(my_pts,0,sizeof(my_pts));
	uint cyls_per_pt = cylinders / count;//每分区柱面数
	for(i=0;i<count;i++)
	{
		struct my_partition *my_pt = &my_pts[i];
		my_pt->boot_ind = 0;//ACTIVE_FLAG;
		my_pt->head = 0;
		my_pt->sector = 1;
		my_pt->cyl = cyls_per_pt*i;
		my_pt->sys_ind = fs;
		my_pt->end_head = heads - 1;
		my_pt->end_sector = sectors;
		my_pt->end_cyl = cyls_per_pt*(i+1)-1;
		my_pt->start = cyls_per_pt*i*heads*sectors;
		my_pt->size = cyls_per_pt*heads*sectors;
	}
	
	//3T4T
	//my_pts[count-1].size = (cylinders-cyls_per_pt*(count-1))*heads*sectors;
	//my_pts[count-1].end_cyl = cylinders - 1;//3T4T
	
	my_pts[0].head = sector_offset / sectors;
	my_pts[0].sector = (sector_offset % sectors) + 1;
	my_pts[0].start = sector_offset;//FAT32 文件系统在包括MBR在内的保留区之后，sector_offset 即是该保留区的大小
	my_pts[0].size -= sector_offset;
	
	for(i=0;i<count;i++)
	{
		struct my_partition *my_pt = &my_pts[i];
		printf("ptn%d cyl:(%d,%d)\n",i+1,my_pt->cyl,my_pt->end_cyl);
		if(my_pt->cyl > 1023)
		{
			//my_pt->head = heads - 1;//3T4T
			//my_pt->sector = sectors;//3T4T
			my_pt->cyl = 1023;
		}
		if(my_pt->end_cyl > 1023)
		{
			my_pt->end_cyl = 1023;
			//my_pt->cyl = 1023;//3T4T
		}
	}
	
	for(i=0;i<4;i++)
	{
		struct pte *pe = &ptes[i];
		memset(pe->part_table,0,sizeof(struct partition));
	}
	
	for(i=0;i<count;i++)
	{
		struct pte *pe = &ptes[i];
		struct partition *pt = pe->part_table;
		struct my_partition *my_pt = &my_pts[i];
		pt->boot_ind = my_pt->boot_ind;
		pt->head = my_pt->head;
		pt->sector = (my_pt->sector&0x3f) | ((my_pt->cyl&0x300)>>2);
		pt->cyl = my_pt->cyl&0xff;
		pt->sys_ind = my_pt->sys_ind;
		pt->end_head = my_pt->end_head;
		pt->end_sector = (my_pt->end_sector&0x3f) | ((my_pt->end_cyl&0x300)>>2);
		pt->end_cyl = my_pt->end_cyl&0xff;
		set_start_sect(pt,my_pt->start);
		set_nr_sects(pt,my_pt->size);
	}
	write_part_table_flag(MBRbuffer);

	// write MBR
	lseek(fd,0,SEEK_SET);
	if(512 != write(fd, MBRbuffer, 512))
	{
		printf("write MBR failed\n");
		close(fd);
		return -1;
	}

	//yaogang modify for bad disk
	//清除坏盘标记
	char sector_clr[512];
	memset(sector_clr, 0, sizeof(sector_clr));
	write(fd, sector_clr, 512);
	
	printf(_("The partition table has been altered!\n\n"));
	//printlog("\n\nThe partition table has been altered!\n\n");
	
	//yg modify 20140813
	//??ê?3 ′?
	int j;
	char pt_name[64];
	
	//3T4T
	//for(i=0; i<3; i++)
	//for(i=0; i<10; i++)
	for(i=0; i<15; i++)
	{
		fsync(fd);//csp modify 20140824
		
		reread_partition_table();
		
		for(j=0; j<count; j++)
		{
			sprintf(pt_name, "%s%d", path, j+1);
			if(access(pt_name, F_OK) != 0)//?·??2?′??ú￡?·?????1ò??é?
			{
				break;
			}
		}
		if(j < count)
		{
			printf("reread_partition_table retry %d\n", i+1);
			usleep(500*1000);
		}
		else
		{
			break;
		}
	}
	
	fsync(fd);
	close(fd);
	
	sync();
	
	//sleep(6);
	sleep(8);
	
	printf("ifly_partition success\n");
	//printlog("ifly_partition success\n");
	
	char *argv[2];
	
	argv[0] = "mkfsdos";
	argv[1] = pt_name;
	#ifndef HISI_3515
	char path2[64];//wrchen 081216
	strncpy(path2,path,strlen(path)-4);//wrchen 081216
	#endif
	s64 totalsize = 0;
	for(i=0;i<count;i++)
	{
		#ifndef HISI_3515
		sprintf(argv[1], "%spart%d", path2, i + 1);//wrchen 081216
		#else
		sprintf(argv[1], "%s%d", path, i + 1);//zlb20100327
		#endif
		printf("prt is %s\n",argv[1]);
		totalsize += GetSizePart(2,argv);
	}
	
	//3T4T
	//part_prog.curpos = 0;
	//part_prog.totalpos = count;
	
	for(i=0;i<count;i++)
	{
		#ifndef HISI_3515
		sprintf(argv[1], "%spart%d", path2, i + 1);//wrchen 081216
		#else
		sprintf(argv[1], "%s%d", path, i + 1);//zlb20100327
		#endif
		//sprintf(argv[1],"%s%d",path,i+1);
		printf("*************************%s partition start*************************\n",pt_name);
		
		/*char msg[256];
		sprintf(msg,"*************************%s partition start*************************\n",pt_name);
		printlog(msg);*/
		
		if(mkfsdos_main(2,argv))
		{
			printf("%s partition failed\n",pt_name);
			return -1;
		}
		
		part_prog.curpos = i+1;
	}
	
	return 0;
}

int ifly_diskinfo(char *diskname,DiskInfo *inf)
{
	if(diskname == NULL || inf == NULL)
	{
		return -1;
	}
	
	int disk_fd = open(diskname,O_RDONLY);
	if(disk_fd == -1)
	{
		//PRINT("open diskname error\n");
		return -1;
	}
	
	memset(inf,0,sizeof(DiskInfo));
	
	unsigned long long bytes;       /* really u64 */
	if(ioctl(disk_fd, BLKGETSIZE64, &bytes) == 0)
	{
		/* got bytes */
	}
	else
	{
		unsigned long longsectors;
		if(ioctl(disk_fd, BLKGETSIZE, &longsectors))
		{
			longsectors = 0;
		}
		bytes = ((unsigned long long) longsectors) << 9;
	}
	inf->capability = bytes;
	
	//unsigned long long bytes2;       /* really u64 */
	//if(ioctl(disk_fd,FIONFREE64,&bytes2)==0)
	//{
	//	inf->freesize= bytes2;
	//	printf("disk free %lld\n",bytes2);
	//}
	
	close(disk_fd);
	
	return 0;
}

