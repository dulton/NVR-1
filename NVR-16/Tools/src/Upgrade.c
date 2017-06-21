#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#ifdef WIN32
#include <io.h>
#include <stdlib.h>
#else
#include <unistd.h>
#endif

#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
//#include <sys/ioctl.h>
//#include <sys/mman.h>

//create by kong, 2011-11-9

#include "ToolUpgrade.h"
#include "ToolCustom.h"
#include "crc32.h"

#define V20_TOOLS_VERSION	0x10000	/// 100.00

#define V20_FILE_MAGIC	0xAB34F100
#define MAX_FILE_NAME_LEN	32

#define FS_TYPE_RAMDISK	1
#define FS_TYPE_CRAMFS	2
#define FS_TYPE_JFS	3

typedef u8 U8;
typedef u32 U32;

typedef EMUPGRADEFILETYPE file_type_e;

typedef struct{
	U32 magic;
	U32	version;
	U32 crc;
	U32 file_cnt;
	U32 flash_size;
	U32 ddr_size;
	U32 rsv;
	char user_info[64];
} file_head_t;

typedef struct{
	U32 file_oft;
	U32 flash_oft;
	U32 mtd_size;
	U32 len;
	file_type_e ft;
	U32 ft_arg;
	char file_name[MAX_FILE_NAME_LEN+4];
} file_info_t;


#define OUT_FILE_NAME	"mainboard2nd.bin"
#define BUF_SIZE	1024

pFnUpgradeProgressCb gFnUpgradeCb = NULL;

extern u8 gStrContainerBinDir[];

unsigned int asc_to_u32(char *s, int *err)
{
	unsigned int val;
	char *p;
		
	*err = -1;	
	if(s == NULL)
		return 0;
	
	if((*s && *s == '0') && ( *(s+1) && *(s+1) == 'x')){
		p = s + 2;
		if('\0' == *p)
			return 0;
		for(; *p; p++){
			if(!((*p >= '0' && *p <= '9') || (*p >= 'a' && *p <= 'f')))
				return 0;
		}
		val = 0;
		for(p = s + 2; *p; p++){
			val = val * 16 + ((*p <= '9')? (*p - '0'):(*p - 'a' + 10));
		}
	}else{
		p = s ;
		if('\0' == *p)
			return 0;
		for(; *p; p++){
			if(!(*p >= '0' && *p <= '9'))
				return 0;
		}
		val = 0;
		for(p = s; *p; p++){
			val = val * 10 + (*p - '0');
		}
	}
		
	*err = 0;
	return val;
}

int check_args(int argc, char **argv, unsigned int *out, int len)
{
	int i;
	int err;
	
	if(argc < len){
		printf("too few args!\n");
		return -1;
	}
	for(i = 0; i < len; i++){
		out[i] = asc_to_u32(argv[i], &err);
		if(err){
			printf("wrong arg %d:(%s)\n", i, argv[i]);
			return -1;
		}
	}
	return 0;
}

int check_file(int left_args, char **argv)
{
	unsigned int arg_val[5];
	struct stat stbuf;
	
	if(left_args < 4){
		printf("error %s\n", argv[0]);
		return -1;
	}
	if(check_args(2, &argv[1], arg_val, 2)){
		printf("error %s args\n", argv[0]);
		return -1;
	}
	if(strlen(argv[3]) > MAX_FILE_NAME_LEN){
		printf("file %s name too long, max length=%d\n", argv[3], MAX_FILE_NAME_LEN);
		return -1;
	}
	if(stat(argv[3], &stbuf)){
		printf("stat file %s error\n", argv[3]);
		return -1;
	}
	if(stbuf.st_size > arg_val[1]){
		printf("too large:%s, max_size=%u, file_size=%u\n", argv[0], arg_val[1], (unsigned int)stbuf.st_size);
		return -1;
	}
	
	if(memcmp("-f", argv[0], 2) == 0){
		if(strlen(argv[0]) < 5){
			printf("error2 %s\n", argv[0]);
			return -1;
		}
		if(argv[0][2] != 'n' && argv[0][2] != 'r'){
			printf("error2 %s\n", argv[0]);
			return -1;
		}
		if(strcmp(&argv[0][3], "ramdisk") != 0
			&& strcmp(&argv[0][3], "cramfs") != 0
			&& strcmp(&argv[0][3], "jfs") != 0){
			printf("error3 %s\n", argv[0]);
			return -1;
		}
	}
	
	return 0;
}

void usage(void)
{
	printf("usage:\n"
	"mkv20_uf -u user_info -[bkf[[r|n]type]et] flash_oft max_size file ... \n"
	"-u user_info (max 63B) \n"
	"-h flash size "
	"-m ddr size "
	"-b bootload	\n"
	"-k kernel	\n"
	"-f[r|n][type] filesys; r=rootfs,n=normalfs; type=ramdisk,cramfs,jfs	\n"
	"-e other \n"
	"-t container \n"
	"-c by name id \n");
}

int pack(int argc, char **argv)
{
	FILE *fp_in, *fp_out;
	file_head_t file_head;
	file_info_t	file_info;
	unsigned int offset;
	struct stat stbuf;
	__u32 checksum;
	unsigned char buf[BUF_SIZE];
	unsigned int arg_val[5];
	int len, ret;
	int i, file_idx;
	unsigned int flash_size = 0, ddr_size = 0;
	
	if(argc < 4){
		usage();
		return -1;
	}
	
	memset(&file_head, 0, sizeof(file_head_t));
	file_head.magic = V20_FILE_MAGIC;

	//step1 arg check
	for(i = 1; i < argc; ){
		if(strcmp("-u", argv[i]) == 0){
			if((argc - i < 1) || strlen(argv[i+1]) > 63){
				printf("error -u\n");
				return -1;
			}
			i += 2;
		}else if(strcmp("-h", argv[i]) == 0){
			if((argc - i < 1) || check_args(1, &argv[i + 1], arg_val, 1)){
				printf("error %s args\n", argv[i]);
				return -1;
			}
			flash_size = arg_val[0];
			i += 2;
		}else if(strcmp("-m", argv[i]) == 0){
			if((argc - i < 1) || check_args(1, &argv[i + 1], arg_val, 1)){
				printf("error %s args\n", argv[i]);
				return -1;
			}
			ddr_size = arg_val[0];
			i += 2;
		}else if(strcmp("-b", argv[i]) == 0
						|| strcmp("-k", argv[i]) == 0
						|| strcmp("-e", argv[i]) == 0
						|| strcmp("-c", argv[i]) == 0
						|| strcmp("-t", argv[i]) == 0
						|| memcmp("-f", argv[i], 2) == 0){
			if(check_file(argc - i, &argv[i])){
				return -1;
			}
			file_head.file_cnt++;
			i += 4;
		}else{
			printf("error unknow %s\n", argv[i]);
			return -1;
		}
	}
	
	if((flash_size == 0) || (ddr_size == 0)){
		printf("error flash size = 0x%x(-h), ddr_size=0x%x(-m)\n", flash_size, ddr_size);
		return -1;
	}
	
	//step2, new file
	fp_out = fopen(OUT_FILE_NAME, "wb+");
	if(!fp_out){
		printf("open file %s error\n", OUT_FILE_NAME);
		return -1;
	}
	
	checksum = 0;
	file_idx = 0;
	offset = sizeof(file_head_t) + (file_head.file_cnt * sizeof(file_info_t));
	for(i = 1; i < argc; ){
		if(strcmp("-u", argv[i]) == 0){
			memcpy(file_head.user_info, argv[i+1], strlen(argv[i+1]));
			i+=2;
			continue;
		}
		if(strcmp("-h", argv[i]) == 0){
			i+=2;
			continue;
		}
		if(strcmp("-m", argv[i]) == 0){
			i+=2;
			continue;
		}
		
		memset(&file_info, 0, sizeof(file_info_t));
		
		if(strcmp("-c", argv[i]) == 0){//由文件名识别文件类型，文件名有命名标准
			file_info.ft = EM_UPGRADE_FT_BYFNAME;
		}else if(strcmp("-b", argv[i]) == 0){
			file_info.ft = EM_UPGRADE_FT_BOOTLOAD;
		}else if(strcmp("-t", argv[i]) == 0){
			file_info.ft = EM_UPGRADE_FT_CONTAINER;
		}else if(strcmp("-k", argv[i]) == 0){
			file_info.ft = EM_UPGRADE_FT_KERNEL;
		}else if(strcmp("-e", argv[i]) == 0){
			file_info.ft = EM_UPGRADE_FT_OTHER;
		}else if(memcmp("-f", argv[i], 2) == 0){
			file_info.ft = EM_UPGRADE_FT_FS_NORMAL;
			if(argv[i][2] == 'r'){
				file_info.ft = EM_UPGRADE_FT_FS_ROOT;
			}
			if(strcmp(&argv[i][3], "ramdisk") == 0){
				file_info.ft_arg = FS_TYPE_RAMDISK;
			}else if(strcmp(&argv[0][3], "cramfs") == 0){
				file_info.ft_arg = FS_TYPE_CRAMFS;
			}else if(strcmp(&argv[0][3], "jfs") == 0){
				file_info.ft_arg = FS_TYPE_JFS;
			}
		}else{
			printf("should not here -1: %s\n", argv[i]);
			return -1;
		}
		
		check_args(2, &argv[i+1], arg_val, 2);//arg_val[0]=flash_oft, arg_val[1]=max_size
		file_info.file_oft = offset;
		file_info.flash_oft = arg_val[0];
		file_info.mtd_size = arg_val[1];
		stat(argv[i+3], &stbuf);
		file_info.len = stbuf.st_size;
		strcpy(file_info.file_name, argv[i+3]);
		
		fseek(fp_out, sizeof(file_head_t) + (file_idx * sizeof(file_info_t)), SEEK_SET);
		//printf("file :%s, flash_oft=0x%X, len=%u\n", file_info.file_name, (unsigned int)file_info.flash_oft, (unsigned int)file_info.len);
		file_idx++;
		ret = fwrite(&file_info, sizeof(char), sizeof(file_info_t), fp_out);
		if(ret != sizeof(file_info_t)){
			printf("write file error\n");
			return -1;
		}
		fseek(fp_out, file_info.file_oft, SEEK_SET);
		fp_in = fopen(argv[i+3], "rb");
		if(!fp_in){
			printf("open file %s error\n", argv[i+3]);
			return -1;
		}
		while(!feof(fp_in)){
			len = fread(buf, sizeof(char), BUF_SIZE, fp_in);
			if(len == 0){
				break;
			}else if(len < 0){
				printf("read file error, ret=%d\n", len);
				return -1;
			}
			ret = fwrite(buf, sizeof(char), len, fp_out);
			if(ret != len){
				printf("write file error\n");
				return -1;
			}
		}
		fclose(fp_in);
		i += 4;
		offset += file_info.len;
		continue;
	}
	
	//crc
	checksum = 0;
	fseek(fp_out, sizeof(file_head_t), SEEK_SET);
	while(!feof(fp_out)){
		len = fread(buf, sizeof(char), BUF_SIZE, fp_out);
		if(len == 0){
			break;
		}else if(len < 0){
			printf("read file error, ret=%d\n", len);
			return -1;
		}
		checksum = crc32(checksum, buf, len);
	}
	
	file_head.crc = checksum;
	file_head.flash_size = flash_size;
	file_head.ddr_size = ddr_size;
	file_head.version = V20_TOOLS_VERSION;
	fseek(fp_out, 0, SEEK_SET);
	ret = fwrite(&file_head, sizeof(char), sizeof(file_head_t), fp_out);
	if(ret != sizeof(file_head_t)){
		printf("write file error\n");
		return -1;
	}
	fclose(fp_out);
	{
		printf("out image ok!\n");
		//printf("file name		:%s\n", OUT_FILE_NAME);
		//printf("file size		:%u\n", offset);
		//printf("crc val			:0x%X\n", (unsigned int)file_head.crc);
	}
	
	return 0;
}

#define BUFFER_SIZE		0x20000

int gen_gz_file(char *in, char *out)
{
	char *buf;
	int fd;
	FILE *fp_out;
	int ret, len;
	
	fd = 0;
	fp_out = NULL;
	buf = NULL;
	
	fd = open(in, O_RDONLY);
	if (fd < 0) {
		perror("open");
		goto gen_gz_out;
	}
	fp_out = fopen(out, "wb");
	if(fp_out == NULL){
		perror("open file");
		goto gen_gz_out;
	}
	buf = malloc(BUFFER_SIZE);
	if(buf == NULL){
		printf("no memory\n");
		goto gen_gz_out;
	}
	
	lseek(fd, 64, SEEK_SET);
	
	do{
		len = read(fd, buf, BUFFER_SIZE);
		if(len < 0){
			printf("read error, ret=%d\n", len);
			goto gen_gz_out;
		}else if(len == 0){
			break;
		}
		ret = fwrite(buf, 1, len, fp_out);
		if(ret != len){
			printf("write error, ret=%d, len=%d\n", ret, len);
			goto gen_gz_out;
		}
	}while(1);
	
gen_gz_out:
	if(buf)free(buf);
	if(fd)	close(fd);
	if(fp_out)fclose(fp_out);
	
	//printf("gen gz end!\n");	
	return 0;
}

s32 unpack(const char* strUpgradeFile)
{
	FILE *fp_in, *fp_out;
	file_head_t file_head;
	file_info_t	file_info;
	unsigned int size;
	__u32 checksum;
	unsigned char buf[BUF_SIZE];
	int len, ret;
	int file_idx;
	
	if (!strUpgradeFile){
		printf("please input file name\n");
		return -1;
	}
	
	memset(&file_head, 0, sizeof(file_head_t));
	file_head.magic = V20_FILE_MAGIC;

	fp_in = fopen(strUpgradeFile, "rb");
	if(!fp_in){
		printf("open file %s error\n", strUpgradeFile);
		return -1;
	}
	
	len = fread(&file_head, sizeof(char), sizeof(file_head_t), fp_in);
	if(len != sizeof(file_head_t)){
		printf("read file error, ret=%d\n", len);
		return -1;
	}
	if(V20_TOOLS_VERSION != file_head.version){
		printf("warning: tool ver=%x, file ver=%x\n", V20_TOOLS_VERSION, (unsigned int)file_head.version);
	}
	if(V20_FILE_MAGIC != file_head.magic){
		printf("error: tool mag=%x, file mag=%x\n", V20_FILE_MAGIC, (unsigned int)file_head.magic);
		return -1;
	}
	
	//printf("file_head:magic=0x%08X, crc=0x%08X, file_cnt=%u, user_info=%s\n", (unsigned int)file_head.magic, (unsigned int)file_head.crc, (unsigned int)file_head.file_cnt, file_head.user_info);

	//crc check
	checksum = 0;
	while(!feof(fp_in)){
		len = fread(buf, sizeof(char), BUF_SIZE, fp_in);
		if(len == 0){
			break;
		}else if(len < 0){
			printf("read file error, ret=%d\n", len);
			return -1;
		}
		checksum = crc32(checksum, buf, len);
	}
	
	if(checksum == file_head.crc){
		printf("crc check ok\n");
	}else{
		printf("crc check fail, check_crc=0x%08X\n", (unsigned int)checksum);
		fclose(fp_in);
		return -1;
	}
	
	for(file_idx = 0; file_idx < file_head.file_cnt; file_idx++){
		fseek(fp_in, sizeof(file_head_t) + (file_idx * sizeof(file_info_t)), SEEK_SET);
		len = fread(&file_info, sizeof(char), sizeof(file_info_t), fp_in);
		if(len != sizeof(file_info_t)){
			printf("read file error, ret=%d\n", len);
			return -1;
		}
		//printf("get file:%s, len=%d, flash_oft=0x%x, ft=%d, ft_arg=%d\n", file_info.file_name, (unsigned int)file_info.len, (unsigned int)file_info.flash_oft, file_info.ft, file_info.ft_arg);
		size = file_info.len;
		fseek(fp_in, file_info.file_oft, SEEK_SET);
		fp_out = fopen(file_info.file_name, "wb");
		if(!fp_out){
			printf("open file %s error\n", file_info.file_name);
			return -1;
		}
		while(size > 0){
			len = (size > BUF_SIZE) ? BUF_SIZE : size;
			len = fread(buf, sizeof(char), len, fp_in);
			if(len < 0){
				printf("0 fail\n");
				return -1;
			}
			if(len == 0){
				break;
			}
			size -= len;			
			ret = fwrite(buf, sizeof(char), len, fp_out);
			if(ret != len){
				printf("write file error\n");
				return -1;
			}
		}
		fclose(fp_out);

		if (FS_TYPE_RAMDISK == file_info.ft_arg)
		{
			if (EM_UPGRADE_FT_FS_ROOT == file_info.ft)
			{
				system("mkdir -p temp/fs");
				system("mkdir rootfs");
				char strCmd[256];
				sprintf(strCmd, "mv %s rootfs_ramfs.uboot", file_info.file_name);
				system(strCmd);
				gen_gz_file("rootfs_ramfs.uboot", "temp/coms_rootfs");
				system("gunzip -c temp/coms_rootfs > temp/fs.img");
				system("mount -o loop temp/fs.img temp/fs");
				system("rm -rf rootfs/*");
				system("cp -rf temp/fs/* rootfs/");
				system("umount temp/fs");
				system("rm -rf temp");
				system("rm -rf rootfs_ramfs.uboot");
			}
			else
			{
				system("mkdir -p temp/fs");
				system("mkdir normalfs");
				char strCmd[256];
				sprintf(strCmd, "mv %s normalfs_ramfs.uboot", file_info.file_name);
				system(strCmd);
				gen_gz_file("normalfs_ramfs.uboot", "temp/coms_normalfs");
				system("gunzip -c temp/coms_normalfs > temp/fs.img");
				system("mount -o loop temp/fs.img temp/fs");
				system("rm -rf normalfs/*");
				system("cp -rf temp/fs/* normalfs/");
				system("umount temp/fs");
				system("rm -rf temp");
				system("rm -rf normalfs_ramfs.uboot");
			}
		}
	}
	
	//printf("unpack over\n");
	fclose(fp_in);
	return 0;
}

int Convert2Flash(const char* strUpgradeFile)
{
	FILE *fp_in, *fp_out;
	file_head_t file_head;
	file_info_t	file_info;
	unsigned int size;
	__u32 checksum;
	unsigned char buf[BUF_SIZE];
	char filename[256];
	int len, ret;
	int file_idx;
	int find_bootload = 0;
	
	if (!strUpgradeFile){
		printf("please input file name\n");
		return -1;
	}
	
	memset(&file_head, 0, sizeof(file_head_t));
	file_head.magic = V20_FILE_MAGIC;

	fp_in = fopen(strUpgradeFile, "rb");
	if(!fp_in){
		printf("open file %s error\n", strUpgradeFile);
		return -1;
	}
	
	len = fread(&file_head, sizeof(char), sizeof(file_head_t), fp_in);
	if(len != sizeof(file_head_t)){
		printf("read file error, ret=%d\n", len);
		return -1;
	}
	
	//printf("file_head:magic=0x%08X, crc=0x%08X, file_cnt=%u, user_info=%s\n", file_head.magic, file_head.crc, file_head.file_cnt, file_head.user_info);

	//crc check
	checksum = 0;
	while(!feof(fp_in)){
		len = fread(buf, sizeof(char), BUF_SIZE, fp_in);
		if(len == 0){
			break;
		}else if(len < 0){
			printf("read file error, ret=%d\n", len);
			return -1;
		}
		checksum = crc32(checksum, buf, len);
	}
	if(checksum == file_head.crc){
		printf("crc check ok\n");
	}else{
		printf("crc check fail, check_crc=0x%08X\n", checksum);
		fclose(fp_in);
		return -1;
	}
	
	sprintf(filename, "flash_image_%dM", file_head.flash_size/1024/1024);
	fp_out = fopen(filename, "wb");
	if(!fp_out){
		printf("open file %s error\n", filename);
		return -1;
	}
	memset(buf, 0xff, BUF_SIZE);
	size = file_head.flash_size;
	while(size > 0){
		len = (size > BUF_SIZE) ? BUF_SIZE : size;
		ret = fwrite(buf, sizeof(char), len, fp_out);
		if(ret != len){
			printf("write file error\n");
			return -1;
		}
		size -= len;
	}
	
	for(file_idx = 0; file_idx < file_head.file_cnt; file_idx++){
		fseek(fp_in, sizeof(file_head_t) + (file_idx * sizeof(file_info_t)), SEEK_SET);
		len = fread(&file_info, sizeof(char), sizeof(file_info_t), fp_in);
		if(len != sizeof(file_info_t)){
			printf("read file error, ret=%d\n", len);
			return -1;
		}
		//printf("get file:%s, len=%d, flash_oft=0x%x\n", file_info.file_name, file_info.len, file_info.flash_oft);
		if (file_info.ft == EM_UPGRADE_FT_BOOTLOAD){
			find_bootload = 1;
		}
		size = file_info.len;
		fseek(fp_in, file_info.file_oft, SEEK_SET);
		fseek(fp_out, file_info.flash_oft, SEEK_SET);
		while(size > 0){
			len = (size > BUF_SIZE) ? BUF_SIZE : size;
			len = fread(buf, sizeof(char), len, fp_in);
			if(len < 0){
				printf("0 fail\n");
				return -1;
			}
			if(len == 0){
				break;
			}
			size -= len;			
			ret = fwrite(buf, sizeof(char), len, fp_out);
			if(ret != len){
				printf("write file error\n");
				return -1;
			}
		}
	}
	
	fclose(fp_out);
	if(!find_bootload){
		fp_out = fopen(filename, "wb");
		if(!fp_out){
			fclose(fp_out);
			printf("not find bootload, fail\n");
		}
	}
	
	//printf("updatefile2flash over\n");
	fclose(fp_in);
	return 0;
}


s32 UpgradeFileGenerate(int argc, char** argv)
{
	return pack(argc, argv);
}

s32 UpgradeConvertToFlashFile(const char* strUpgradeFile)
{
	return Convert2Flash(strUpgradeFile);
}

s32 UpgradeFileDestruct(const char* strUpgradeFile)
{
	return unpack(strUpgradeFile);
}

s32 UpgradeFileOut(const char* strUpgradeFile, EMUPGRADEFILETYPE emType, char* strFileOutName, u32 nMaxNameLen)
{
	if (strUpgradeFile && strFileOutName)
	{
		FILE *fp_in, *fp_out;
		file_head_t file_head;
		file_info_t	file_info;
		unsigned int size;
		__u32 checksum;
		unsigned char buf[BUF_SIZE];
		int len, ret;
		int file_idx;
		
		memset(&file_head, 0, sizeof(file_head_t));
		file_head.magic = V20_FILE_MAGIC;

		fp_in = fopen(strUpgradeFile, "rb");
		if(!fp_in){
			printf("open file %s error\n", strUpgradeFile);
			return -1;
		}
		
		len = fread(&file_head, sizeof(char), sizeof(file_head_t), fp_in);
		if(len != sizeof(file_head_t)){
			printf("read file error, ret=%d\n", len);
			return -1;
		}
		if(V20_TOOLS_VERSION != file_head.version){
			printf("warning: tool ver=%x, file ver=%x\n", V20_TOOLS_VERSION, (unsigned int)file_head.version);
		}
		if(V20_FILE_MAGIC != file_head.magic){
			printf("error: tool mag=%x, file mag=%x\n", V20_FILE_MAGIC, (unsigned int)file_head.magic);
			return -1;
		}
		
		//printf("UpgradeFileOut file_head:magic=0x%08X, crc=0x%08X, file_cnt=%u, user_info=%s\n", (unsigned int)file_head.magic, (unsigned int)file_head.crc, (unsigned int)file_head.file_cnt, file_head.user_info);

		//crc check
		checksum = 0;
		while(!feof(fp_in)){
			len = fread(buf, sizeof(char), BUF_SIZE, fp_in);
			if(len == 0){
				break;
			}else if(len < 0){
				printf("read file error, ret=%d\n", len);
				return -1;
			}
			checksum = crc32(checksum, buf, len);
		}
		
		if(checksum == file_head.crc){
			printf("crc check ok\n");
		}else{
			printf("crc check fail, check_crc=0x%08X\n", (unsigned int)checksum);
			fclose(fp_in);
			return -1;
		}
		
		for(file_idx = 0; file_idx < file_head.file_cnt; file_idx++){
			fseek(fp_in, sizeof(file_head_t) + (file_idx * sizeof(file_info_t)), SEEK_SET);
			len = fread(&file_info, sizeof(char), sizeof(file_info_t), fp_in);
			if(len != sizeof(file_info_t)){
				printf("read file error, ret=%d\n", len);
				return -1;
			}

			if (emType == file_info.ft)
			{
				if (strlen(file_info.file_name) <= nMaxNameLen)
				{
					memset(strFileOutName, 0, sizeof(strFileOutName));
					sprintf(strFileOutName, "%s%s", gStrContainerBinDir, file_info.file_name);
					printf("container dir %s \n", strFileOutName);
				}
				else
				{
					return -1;
				}
				//printf("get file:%s, len=%d, flash_oft=0x%x\n", file_info.file_name, (unsigned int)file_info.len, (unsigned int)file_info.flash_oft);
				size = file_info.len;
				fseek(fp_in, file_info.file_oft, SEEK_SET);
				//fp_out = fopen(file_info.file_name, "wb");
				fp_out = fopen(strFileOutName, "wb");
				if(!fp_out){
					printf("open file %s error\n", file_info.file_name);
					return -1;
				}
				while(size > 0){
					len = (size > BUF_SIZE) ? BUF_SIZE : size;
					len = fread(buf, sizeof(char), len, fp_in);
					if(len < 0){
						printf("0 fail\n");
						return -1;
					}
					if(len == 0){
						printf("1 fail\n");
						break;
					}
					size -= len;			
					ret = fwrite(buf, sizeof(char), len, fp_out);
					if(ret != len){
						printf("write file error\n");
						return -1;
					}
				}
				fclose(fp_out);
				break;
			}
		}
		
		//printf("unpack over\n");
		fclose(fp_in);

		if (file_idx == file_head.file_cnt)
		{
		#if 1
			return -1;
		#else
			switch (emType)
			{
				case EM_UPGRADE_FT_BOOTLOAD:
					strcpy(strFileOutName, "uboot2nd.bin");
					break;
				case EM_UPGRADE_FT_KERNEL:
					strcpy(strFileOutName, "uImage2nd.bin");
					break;
				case EM_UPGRADE_FT_FS_ROOT:
					strcpy(strFileOutName, "rootfs2nd.bin");
					break;
				case EM_UPGRADE_FT_FS_NORMAL:
					strcpy(strFileOutName, "normalfs2nd.bin");
					break;
				case EM_UPGRADE_FT_OTHER:
					strcpy(strFileOutName, "other2nd.bin");
					break;
				case EM_UPGRADE_FT_CONTAINER: //放各种类型文件
					strcpy(strFileOutName, "container2nd.bin");
					break;
				default:
					return -1;
			}
		#endif
		}
		
		return 0;
	}

	return -1;
}

extern s32 CopyFile(u8* strFileFrom, u8* strFileTo);


s32 UpgradeFileRenew(const char* strUpgradeFile, EMUPGRADEFILETYPE emType, char* strFileInName)
{
#ifndef FLASHDEV
	FILE *fp_in = NULL, *fp_out = NULL;
	s32 nRet = 0;

	if (strUpgradeFile && strFileInName)
	{
		file_head_t file_head, out_file_head;
		file_info_t file_info, out_file_info;
		struct stat stbuf;
		__u32 checksum;
		unsigned char buf[BUF_SIZE];
		int ret;
		int file_idx;
		u8 strTmpFile[256], strNewFile[256];
		s32 nCopyItem = 0, nDataOffset = 0, nFileLen = 0, nIndex = 0;
		s32 len, nReadLen = 0;
				
		sprintf(strTmpFile, "%s.tmp", strUpgradeFile);
		sprintf(strNewFile, "%s", strUpgradeFile);
		
		memset(&file_head, 0, sizeof(file_head_t));
		file_head.magic = V20_FILE_MAGIC;

		fp_in = fopen(strUpgradeFile, "rb");
		if(!fp_in){
			printf("open file %s error\n", strUpgradeFile);
			nRet = -1;
			goto UpgradeRenewErr;
		}
		
		len = fread(&file_head, sizeof(char), sizeof(file_head_t), fp_in);
		if(len != sizeof(file_head_t)){
			printf("read file error, ret=%d\n", len);
			nRet = -2;
			goto UpgradeRenewErr;
		}
		if(V20_TOOLS_VERSION != file_head.version){
			printf("warning: tool ver=%x, file ver=%x\n", V20_TOOLS_VERSION, (unsigned int)file_head.version);
		}
		if(V20_FILE_MAGIC != file_head.magic){
			printf("error: tool mag=%x, file mag=%x\n", V20_FILE_MAGIC, (unsigned int)file_head.magic);
			nRet = -3;
			goto UpgradeRenewErr;
		}
		
		//printf("UpgradeFileRenew file_head:magic=0x%08X, crc=0x%08X, file_cnt=%u, user_info=%s\n", (unsigned int)file_head.magic, (unsigned int)file_head.crc, (unsigned int)file_head.file_cnt, file_head.user_info);

		//crc check
		checksum = 0;
		while(!feof(fp_in)){
			len = fread(buf, sizeof(char), BUF_SIZE, fp_in);
			if(len == 0){
				printf("read file error, ret=%d\n", len);
				break;
			}else if(len < 0){
				printf("read file error, ret=%d\n", len);
				nRet = -4;
				goto UpgradeRenewErr;

			}
			checksum = crc32(checksum, buf, len);
		}
		
		if(checksum == file_head.crc){
			printf("crc check ok\n");
		}else{
			printf("crc check fail, check_crc=0x%08X\n", (unsigned int)checksum);
			nRet = -5;
			goto UpgradeRenewErr;
		}

		for(file_idx = 0; file_idx < file_head.file_cnt; file_idx++)
		{
			fseek(fp_in, sizeof(file_head_t) + (file_idx * sizeof(file_info_t)), SEEK_SET);
			len = fread(&file_info, sizeof(char), sizeof(file_info_t), fp_in);
			//printf("file_idx:%d, name:%s, file_oft:%d, flash_oft:%d, ft:%d arg:%d len:%d mtd:%d\n", file_idx, file_info.file_name
			//	, file_info.file_oft, file_info.flash_oft, file_info.ft, file_info.ft_arg, file_info.len, file_info.mtd_size);
			if(len != sizeof(file_info_t)){
				printf("read file error, ret=%d\n", len);
				nRet = -6;
				goto UpgradeRenewErr;
			}

			if (emType == file_info.ft)
			{
				break;
			}
		}

		if (file_idx == file_head.file_cnt)
		{
			printf("file_idx == file_head.file_cnt error\n");
			nRet = -7;
			goto UpgradeRenewErr;
		}

		fp_out = fopen(strTmpFile, "wb+");
		if(!fp_out){
			printf("open file %s error\n", strTmpFile);
			nRet = -8;
			goto UpgradeRenewErr;
		}

		memcpy(&out_file_head, &file_head, sizeof(file_head_t));
		len = fwrite(&out_file_head, sizeof(char), sizeof(file_head_t), fp_out);
		if(len != sizeof(file_head_t)){
			printf("write file_head_t error, ret=%d\n", len);
			nRet = -9;
			goto UpgradeRenewErr;
		}

		for(file_idx = 0; file_idx < file_head.file_cnt; file_idx++)
		{
			len = fwrite(&file_info, sizeof(char), sizeof(file_info_t), fp_out);
			if(len != sizeof(file_info_t))
			{
				printf("write file_info_t error, ret=%d\n", len);
				nRet = -10;
				goto UpgradeRenewErr;
			}
		}
		
		nCopyItem = 0, nDataOffset = sizeof(file_head_t) + file_head.file_cnt * sizeof(file_info_t), nFileLen = 0, nIndex = 0;
		for(file_idx = 0; file_idx < file_head.file_cnt; file_idx++){
			fseek(fp_in, sizeof(file_head_t) + (file_idx * sizeof(file_info_t)), SEEK_SET);
			len = fread(&file_info, sizeof(char), sizeof(file_info_t), fp_in);
			//printf("file_idx:%d, name:%s, file_oft:%d, flash_oft:%d, ft:%d arg:%d len:%d mtd:%d\n", file_idx, file_info.file_name
			//	, file_info.file_oft, file_info.flash_oft, file_info.ft, file_info.ft_arg, file_info.len, file_info.mtd_size);
			if(len != sizeof(file_info_t)){
				printf("read file error, ret=%d\n", len);
				nRet = -11;
				goto UpgradeRenewErr;
			}

			if (emType != file_info.ft)
			{
				memcpy(&out_file_info, &file_info, sizeof(file_info_t));
				out_file_info.file_oft = nDataOffset;
				//printf("out file_idx:%d, name:%s, file_oft:%d, flash_oft:%d, ft:%d arg:%d len:%d mtd:%d\n", nCopyItem, out_file_info.file_name
				//, out_file_info.file_oft, out_file_info.flash_oft, out_file_info.ft, out_file_info.ft_arg, out_file_info.len, out_file_info.mtd_size);
				fseek(fp_out, sizeof(file_head_t) + (nCopyItem * sizeof(file_info_t)), SEEK_SET);
				len = fwrite(&out_file_info, sizeof(char), sizeof(file_info_t), fp_out);
				if(len != sizeof(file_info_t))
				{
					printf("write file_info_t error, ret=%d\n", len);
					nRet = -12;
					goto UpgradeRenewErr;
				}

				fseek(fp_in, file_info.file_oft, SEEK_SET);
				fseek(fp_out, out_file_info.file_oft, SEEK_SET);

				nFileLen = file_info.len;

				while(nFileLen > 0){
					nReadLen = nFileLen > BUF_SIZE ? BUF_SIZE : nReadLen;
					len = fread(buf, sizeof(char), nReadLen, fp_in);
					if (len == 0)
					{
						nRet = -13;
						printf("read file error, ret=%d\n", len);
						goto UpgradeRenewErr;
					}else if(len < 0){
						printf("read file error, ret=%d\n", len);
						nRet = -14;
						goto UpgradeRenewErr;
					}
					
					nFileLen -= len;
					ret = fwrite(buf, sizeof(char), len, fp_out);
					if(ret != len){
						printf("write file error\n");
						nRet = -15;
						goto UpgradeRenewErr;
					}
				}				
				nCopyItem++;
				nDataOffset += file_info.len;
			}
			else
			{
				nIndex = file_idx;
			}
		}

		fseek(fp_in, sizeof(file_head_t) + (nIndex * sizeof(file_info_t)), SEEK_SET);
		len = fread(&file_info, sizeof(char), sizeof(file_info_t), fp_in);
		if(len != sizeof(file_info_t)){
			printf("read file error, ret=%d\n", len);
			nRet = -16;
			goto UpgradeRenewErr;
		}

		fclose(fp_in);
		fp_in = NULL;

		memcpy(&out_file_info, &file_info, sizeof(file_info_t));
		out_file_info.file_oft = nDataOffset;
		stat(strFileInName, &stbuf);
		out_file_info.len = stbuf.st_size;
		//strcpy(out_file_info.file_name, strFileInName);

		if (out_file_info.len > out_file_info.mtd_size)
		{
			printf("space not enough, len=%d, mtd_size=%d\n", out_file_info.len, out_file_info.mtd_size);
			nRet = -100;
			goto UpgradeRenewErr;
		}

		//printf("out file_idx:%d, name:%s, file_oft:%d, flash_oft:%d, ft:%d arg:%d len:%d mtd:%d\n", nCopyItem, out_file_info.file_name
		//, out_file_info.file_oft, out_file_info.flash_oft, out_file_info.ft, out_file_info.ft_arg, out_file_info.len, out_file_info.mtd_size);

		fseek(fp_out, sizeof(file_head_t) + (nCopyItem * sizeof(file_info_t)), SEEK_SET);
		len = fwrite(&out_file_info, sizeof(char), sizeof(file_info_t), fp_out);
		if(len != sizeof(file_info_t))
		{
			printf("write file_info_t error, ret=%d\n", len);
			nRet = -17;
			goto UpgradeRenewErr;
		}

		fp_in = fopen(strFileInName, "rb");
		if (!fp_in)
		{
			printf("open file %s error\n", strFileInName);
			nRet = -18;
			goto UpgradeRenewErr;
		}

		fseek(fp_out, out_file_info.file_oft, SEEK_SET);
		while(!feof(fp_in)){
			len = fread(buf, sizeof(char), BUF_SIZE, fp_in);
			if(len == 0){
				printf("read file error, ret=%d\n", len);
				break;
			}else if(len < 0){
				printf("read file error, ret=%d\n", len);
				nRet = -19;
				goto UpgradeRenewErr;
			}
			ret = fwrite(buf, sizeof(char), len, fp_out);
			if (ret != len)
			{
				printf("write file error\n");
				nRet = -20;
				goto UpgradeRenewErr;
			}
		}

		//crc
		checksum = 0;
		fseek(fp_out, sizeof(file_head_t), SEEK_SET);
		while(!feof(fp_out)){
			len = fread(buf, sizeof(char), BUF_SIZE, fp_out);
			if(len == 0){
				break;
			}else if(len < 0){
				printf("read file error, ret=%d\n", len);
				return -1;
			}
			checksum = crc32(checksum, buf, len);
		}

		fseek(fp_out, 0, SEEK_SET);
		out_file_head.crc = checksum;
		len = fwrite(&out_file_head, sizeof(char), sizeof(file_head_t), fp_out);
		if(len != sizeof(file_head_t)){
			printf("write file_head_t error, ret=%d\n", len);
			nRet = -21;
			goto UpgradeRenewErr;
		}

		//printf("After UpgradeFileRenew file_head:magic=0x%08X, crc=0x%08X, file_cnt=%u, user_info=%s\n", (unsigned int)out_file_head.magic, (unsigned int)out_file_head.crc, (unsigned int)out_file_head.file_cnt, out_file_head.user_info);
		
		fclose(fp_in);
		fp_in = NULL;
		fclose(fp_out);
		fp_out = NULL;	

		CopyFile(strTmpFile, (u8*)strNewFile);

		remove(strTmpFile);

		return 0;
	}

UpgradeRenewErr:
	printf("nRet=%d\n", (int)nRet);

	if (fp_out)
	{
		fclose(fp_out);
		fp_out = NULL;
	}

	if (fp_in)
	{
		fclose(fp_in);
		fp_in = NULL;
	}
#endif

	return -1;
}

#ifdef WIN32

#elif defined(FLASHDEV)
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

//create by kong, 2011-11-9

//#define V20_FLASH_TEST_WR

#ifdef V20_FLASH_TEST_WR
struct mtd_info_user{
	unsigned int size;
	unsigned int erasesize;
};
struct erase_info_user{
	unsigned int start;
	unsigned int length;
};
#else
//#include <linux/types.h>
#include <mtd/mtd-user.h>
#endif

static u32 process_total_size = 0;
static u32 process_cur_pos = 0;
static char Model[30];

u32 UpgradeGetProgress(void)
{
	u32 nCurPos = process_cur_pos, nTotalPos = process_total_size;
	if (nTotalPos)
	{
		if (nCurPos >= nTotalPos)
		{
			return 100;
		}
		return (nCurPos * 100) / nTotalPos;
	}
	
	return 0;
}

unsigned int get_flash_cap(void)
{
	char tmp_buf[256];
	struct mtd_info_user mtd;
	unsigned int size = 0;
	int mtd_idx = 0;
	int fd, rtn;
	
	#ifdef V20_FLASH_TEST_WR
	
	return 1024;
	
	#else
	
	while(1){
		sprintf(tmp_buf, "/dev/mtd/%d", mtd_idx++);
		fd = open(tmp_buf, O_SYNC | O_RDWR);
		if (fd < 0){
			break;
		}
		rtn = ioctl(fd, MEMGETINFO, &mtd);
		if (rtn < 0){
			perror("MEMGETINFO");
			close(fd);
			return 0;
		}
		size += mtd.size;
		close(fd);
	}
	return size;
	#endif
}

int write_flash(FILE *fp_in, unsigned int file_oft, unsigned int flash_oft, unsigned int len)
{
	struct mtd_info_user mtd;
	struct erase_info_user erase;
	int fd = -1;
	int rtn = 0, oft, size, wr_size;
	unsigned char *buf_file = NULL, *buf_flash = NULL;
	int mtd_idx = 0, buf_size;
	unsigned int left_size = len;
	char tmp_buf[256];
	
	fseek(fp_in, file_oft, SEEK_SET);
	
	#ifdef V20_FLASH_TEST_WR
	fd = open("mtd/0", O_SYNC | O_RDWR);
	#else
	fd = open("/dev/mtd/0", O_SYNC | O_RDWR);
	#endif
	if (fd < 0){
		perror("/dev/mtd/0");
		rtn = -1;
		goto end;
	}
	
	#ifdef V20_FLASH_TEST_WR
	{
		struct stat stbuf;
		stat("mtd/0", &stbuf);
		mtd.erasesize = 0x8;
		mtd.size = stbuf.st_size;
		printf("mtd/0 size=%d\n", mtd.size);
	}
	#else
	rtn = ioctl(fd, MEMGETINFO, &mtd);
	if (rtn < 0){
		perror("MEMGETINFO");
		rtn = -1;
		goto end;
	}
	#endif
	
	buf_size = mtd.erasesize;
	buf_file = malloc(buf_size);
	if(buf_file == NULL){
		printf("no buffer\n");
		rtn = -1;
		goto end;
	}
	buf_flash = malloc(buf_size);
	if(buf_flash == NULL){
		printf("no buffer\n");
		rtn = -1;
		goto end;
	}
	
	//step1, look for first mtdblock
	oft = flash_oft;
	do{
		if(mtd.size > oft)
			break;
		
		close(fd);
		
		oft -= mtd.size;
		
		#ifdef V20_FLASH_TEST_WR
		sprintf(tmp_buf, "mtd/%d", ++mtd_idx);
		#else
		sprintf(tmp_buf, "/dev/mtd/%d", ++mtd_idx);
		#endif
		fd = open(tmp_buf, O_SYNC | O_RDWR);
		if (fd < 0){
			perror(tmp_buf);
			rtn = -1;
			goto end;
		}
		#ifdef V20_FLASH_TEST_WR
		{
			struct stat stbuf;
			stat(tmp_buf, &stbuf);
			mtd.erasesize = 0x8;
			mtd.size = stbuf.st_size;
			printf("%s size=%d\n", tmp_buf, mtd.size);
		}
		#else
		rtn = ioctl(fd, MEMGETINFO, &mtd);
		if (rtn < 0){
			perror("MEMGETINFO");
			rtn = -1;
			goto end;
		}
		#endif
	}while(1);
	
	//
	lseek(fd, oft, SEEK_SET);
	do
	{
		erase.start = oft;
		wr_size = mtd.size - oft;	//一个分区内需要写的大小
		wr_size = (wr_size >= left_size) ? left_size : wr_size;
		left_size -= wr_size;
		oft = 0;
		//printf("mtd_idx=%d, wr_size=0x%x, buf_size=0x%x\n", mtd_idx, wr_size, buf_size);
		while(wr_size > 0)
		{
			size = (wr_size > buf_size) ? buf_size : wr_size;
			wr_size -= size;
			//compare
			//printf("size=%d\n", size);//单次写的大小
			rtn = read(fd, buf_flash, size);
			if(rtn != size)
			{
				printf("size=%d, rtn=%d\n", size, rtn);
				perror("read flsh");
				rtn = -1;
				goto end;
			}
			rtn = fread(buf_file, sizeof(char), size, fp_in);
			if(rtn != size)
			{
				printf("read file error, ret=%d\n", rtn);
				rtn = -1;
				goto end;
			}
			
			process_cur_pos += size;
			
			if(gFnUpgradeCb)
			{
				//printf("^^^process_total_size=%d, process_cur_pos=%d\n", process_total_size, process_cur_pos);
				gFnUpgradeCb(process_total_size, process_cur_pos);
				//usleep(10*1000);//csp modify
			}

			#if 1//yaogang debug
			if(memcmp(buf_file, buf_flash, size) == 0)
			{
				//static int cccc = 0;
				//printf("same size=0x%x, idx=%d\n", size, ++cccc);
				erase.start += buf_size;
				continue;
			}
			#endif
			
			#ifdef V20_FLASH_TEST_WR
			//////////
			#else
			erase.length = buf_size;
			//printf("1 erase start=0x%x, length=0x%x, size=0x%x\n", erase.start, erase.length, size);
			rtn = ioctl (fd, MEMERASE, &erase);
			if(rtn < 0)
			{
				perror("erase failed");
				rtn = -1;
				goto end;
			}
			erase.start += buf_size;
			#endif
			lseek(fd, size * -1, SEEK_CUR);
			//printf("not wr size=0x%x\n", size);
			write(fd, buf_file, size);
			
			//csp modify
			//usleep(5*1000);
			fsync(fd);
		}
		
		if(left_size <= 0)
		{
			break;
		}
		
		close(fd);
		
		#ifdef V20_FLASH_TEST_WR
		sprintf(tmp_buf, "mtd/%d", ++mtd_idx);
		#else
		sprintf(tmp_buf, "/dev/mtd/%d", ++mtd_idx);
		#endif
		fd = open(tmp_buf, O_SYNC | O_RDWR);
		if(fd < 0)
		{
			perror(tmp_buf);
			rtn = -1;
			goto end;
		}
		#ifdef V20_FLASH_TEST_WR
		{
			struct stat stbuf;
			stat(tmp_buf, &stbuf);
			mtd.erasesize = 0x8;
			mtd.size = stbuf.st_size;
			printf("%s size=%d\n", tmp_buf, mtd.size);
		}
		#else
		rtn = ioctl(fd, MEMGETINFO, &mtd);
		if(rtn < 0)
		{
			perror("MEMGETINFO");
			rtn = -1;
			goto end;
		}
		#endif
	}while(1);
	
	rtn = 0;
end:
	if(fd > 0) close(fd);
	if(buf_file) free(buf_file);
	if(buf_flash) free(buf_flash);
	
	return rtn;
}

s32 UpgradeByFlashFile(char* strFileName)
{
	int all_mtd_flash_cap, nLen;
	struct stat stbuf;
	FILE *fp_in = NULL;

	stat(strFileName, &stbuf);
	nLen = stbuf.st_size;
	
	//获得flash容量
	all_mtd_flash_cap = get_flash_cap();
	//printf("UpgradeByFlashFile all_mtd_flash_cap=0x%x\n", all_mtd_flash_cap);
	if(all_mtd_flash_cap == 0){
		fclose(fp_in);
		return -1;
	}

	if (nLen == all_mtd_flash_cap)
	{
		fp_in = fopen(strFileName, "rb");
		if(!fp_in){
			printf("open file %s error\n", strFileName);
			return -3;
		}
		process_total_size = nLen;
		write_flash(fp_in, 0, 0, nLen);
	}
	else
	{
		printf("UpgradeByFlashFile nLen != all_mtd_flash_cap\n");
		return -2;
	}

	return 0;
}

//check updatefile
static void get_board_cfg(unsigned long *flash_size, unsigned long *ddr_size)
{
	int fd, rtn;
	struct mtd_info_user mtd;
	int idx;
	char buf[32];
	
	*flash_size = 0;
	*ddr_size = 0;
	
	idx = 0;
	
	while(1)
	{
		sprintf(buf, "/dev/mtd%d", idx++);
		fprintf(stderr, "11111111111%s\n", buf);
		fd = open(buf, O_SYNC | O_RDWR);
		if (fd < 0){
			break;
		}
		rtn = ioctl(fd, MEMGETINFO, &mtd);
		if (rtn < 0){
			perror("MEMGETINFO");
			close(fd);
			break;
		}
		*flash_size += mtd.size;
		close(fd);
	}
	fprintf(stderr, "11111111111flash_size=%lu\n", *flash_size);
	if (*flash_size < 256 * 1024) //zfk 20120301
	{
		while(1)
		{
			sprintf(buf, "/dev/mtd/%d", idx++);
			fd = open(buf, O_SYNC | O_RDWR);
			if (fd < 0)
			{
				break;
			}
			rtn = ioctl(fd, MEMGETINFO, &mtd);
			if (rtn < 0)
			{
				perror("MEMGETINFO");
				close(fd);
				break;
			}
			*flash_size += mtd.size;
			close(fd);
		}
	}
	fprintf(stderr, "22222222222flash_size=%lu\n", *flash_size);
	{
		unsigned int buf[5] = {0};
		int fd_ipc = -1;
		
		#define TL_DEV_REG_RW 			_IOWR('V', 192 + 9, unsigned int)
		#define TL_IPC_REG_RW 			_IOWR('S', 192 + 9, unsigned int)
		
		fd = open("/dev/tl_R9508", O_RDWR);
		if (fd < 0)
		{
			fd_ipc = open("/dev/tl_ipcramera", O_RDWR);
		}
		
		if (fd < 0 && fd_ipc < 0) //zfk 20120301
		{
			fprintf(stderr,"%s\n", "The model is IP9211");
			fd_ipc = open("/dev/misc/tl_ipcramera", O_RDWR);
			if (fd_ipc < 0)
			{
				perror("tl_hs3511");
				return; 
			}
			else
			{
				buf[0] = 0x01;
				buf[1] = 0x10150010;
				if (fd > 0)
				{
					if (ioctl(fd, TL_DEV_REG_RW,buf) < 0)
					{
						fprintf(stderr, "%s","hs3515_rd error!\n");
						close(fd);
						return;
					}
				}
				else
				{
					fd = fd_ipc;
					if (ioctl(fd, TL_IPC_REG_RW,buf) < 0)
					{
						fprintf(stderr, "%s","hs3515_rd error!\n");
						close(fd);
						return;
					}
				}
				close(fd);
			}
		}
		else
		{
			buf[0] = 0x01;
			buf[1] = 0x20110010;
			if (fd > 0)
			{
				if(ioctl(fd, TL_DEV_REG_RW,buf) < 0){
					printf("hs3515_rd error!\n");
					close(fd);
					return;
				}
			}
			else
			{
				fd = fd_ipc;
				if(ioctl(fd, TL_IPC_REG_RW,buf) < 0){
					printf("hs3515_rd error!\n");
					close(fd);
					return;
				}
			}

			close(fd);
		}
		
		if (buf[0] & (1 << 7))
		{
			*ddr_size = 256*1024*1024;
		}
		else
		{
			*ddr_size = 128*1024*1024;
		}
	}
}

//check updatefile
/*
	返回值：0,->成功
				1,->没有检测到配置文件
				其他错误时配置文件错误
*/
static int check_update_file(char *model, char *cfg_parm)
{
	char c_model[32];
	unsigned long c_fsize = 0, c_dsize = 0;
	int len, i;
	char *p = cfg_parm;
	int ret = -1;
	unsigned long flash_size = 0, ddr_size = 0;
	fprintf(stderr, "model=%s,cfg_parm=%s\n", model, cfg_parm);
	get_board_cfg(&flash_size, &ddr_size);
	
	if(model == NULL || p == NULL)
	{
		ret = -1;
		goto out;
	}
	
	len = strlen(p);
	if(len < 4){ret = -2; goto out;}
	
	if(memcmp("CFG_", p, 4)){ ret = -3; goto out;}
	
	p += 4;
	
	if(*p == '\0'){ret = -4; goto out;}
	
	if(*p++ == 'F')
	{
		ret = 0;
		goto out;
	}
	
	if(*p++ != '_'){ret = -5; goto out;}

	for(i = 0; *p != '\0' && *p != '_';)
	{
		c_model[i++] = *p++;
	} 
	
	if(memcmp(c_model, model, strlen(model))){ ret = -6; goto out;}
	
	if(*p++ == '\0'){ ret = -7; goto out;}
	
	while(*p != '\0' && *p >= '0' && *p <= '9')
	{
		c_fsize *= 10;
		c_fsize += *p++ - '0';
	}
	
	if(*p == '\0'){ ret = -8; goto out;}
	
	if(*p == 'm' || *p == 'M'){c_fsize *= 1*1024*1024;}
	else if(*p == 'k' || *p == 'K'){c_fsize *= 1*1024;}

	if(c_fsize > flash_size){ ret = -9; goto out;}
	
	if(*++p != '_'){ ret = -10; goto out;}
	
	if(*++p == '\0'){ ret = -11; goto out;}
	
	while(*p != '\0' && *p >= '0' && *p <= '9')
	{
		c_dsize *= 10;
		c_dsize += *p++ - '0';
	}
	
 if(*p == '\0'){ ret = -12; goto out;}
 
 if(*p == 'm' || *p == 'M'){c_dsize *= 1*1024*1024;}
 else if(*p == 'k' || *p == 'K'){c_dsize *= 1*1024;}
 
 if(c_dsize > ddr_size){ ret = -13; goto out;}
 
 ret = 0;
 
out:
 fprintf(stderr, "\ncheck_update_file:ret=%d, modle=%s,fs1=0x%lx,ds1=0x%lx, cfg_param=%s\n", ret, model, flash_size, ddr_size, cfg_parm);
 return ret;
}

void UpgradeSetModel(char* updatemodel)
{
	strncpy(Model, updatemodel,sizeof(Model));
}

s32 CheckUpdateFileModel(char* CfgParm)
{
	//yaogang modify 20151209
	printf("%s ****** Warning, not check direct return success\n", __func__);
	return 0;
	
	if(0 != strncmp(Model,CfgParm,sizeof(Model)))
	{
		//csp modify 20130106
		if(strstr(Model,"R31") != NULL && strstr(CfgParm,"R31") != NULL)
		{
			printf("CheckUpdateFileModel[R31xx] succ, main board model: %s, update file model: %s\n",Model,CfgParm);
			return 0;
		}
		
		printf("CheckUpdateFileModel failed, main board model: %s, update file model: %s\n",Model,CfgParm);
		return -1;
	}
	else
	{
		printf("CheckUpdateFileModel succ, main board model: %s, update file model: %s\n",Model,CfgParm);
		return 0;
	}
}

s32 UpgradeCheckFile(char* strModel, char* strFileName)
{
	FILE *fp_in;
	file_head_t file_head;
	//unsigned char buf[1024];
	int len;
	
	char *filename = strFileName;
	fprintf(stderr, "strModel=%s,strFileName=%s\n", strModel, strFileName);
	if (0 == strncmp("flash_image_", filename, 12))
	{
		return 0;
	}

	memset(&file_head, 0, sizeof(file_head_t));

	fp_in = fopen(filename, "rb");
	if(!fp_in){
		printf("open file %s error\n", filename);
		return -1;
	}
	
	len = fread(&file_head, sizeof(char), sizeof(file_head_t), fp_in);
	if(len != sizeof(file_head_t)){
		printf("read file error, ret=%d\n", len);
		fclose(fp_in);
		return -1;
	}
	
	if(V20_TOOLS_VERSION != file_head.version){
		printf("warning: tool ver=%x, file ver=%x\n", V20_TOOLS_VERSION, file_head.version);
	}
	
	if(V20_FILE_MAGIC != file_head.magic){
		printf("error: tool mag=%x, file mag=%x\n", V20_FILE_MAGIC, file_head.magic);
		fclose(fp_in);
		return -1;
	}
	
	fclose(fp_in);
	
	//zfk 20120301
	if (0 != strncmp(strModel, SIMPLE_MODEL_CHK, strlen(SIMPLE_MODEL_CHK)))
	{
		return check_update_file(strModel, file_head.user_info);
	}
	else
	{
		return CheckUpdateFileModel(file_head.user_info);
	}
}

s32 UpgradeToFlash(char* strFileName, pFnUpgradeProgressCb pFnCb)
{
	FILE *fp_in;
	file_head_t file_head;
	file_info_t	file_info;
	unsigned int size;
	__u32 checksum;
	unsigned char buf[1024];
	int len, fd, rtn;
	int file_idx;
	int all_mtd_flash_cap;
	struct mtd_info_user mtd;
	char *filename = strFileName;
	//struct erase_info_user erase;
	
	process_total_size = 0;
	process_cur_pos = 0;
	
	if (0 == strncmp("flash_image_", filename, 12))
	{
		gFnUpgradeCb = pFnCb;
		return UpgradeByFlashFile(strFileName);
	}
	
	memset(&file_head, 0, sizeof(file_head_t));
	
	fp_in = fopen(filename, "rb");
	if(!fp_in){
		printf("open file %s error\n", filename);
		return -1;
	}
	
	len = fread(&file_head, sizeof(char), sizeof(file_head_t), fp_in);
	if(len != sizeof(file_head_t)){
		printf("read file error, ret=%d\n", len);
		fclose(fp_in);
		return -1;
	}
	
	if(V20_TOOLS_VERSION != file_head.version){
		printf("warning: tool ver=%x, file ver=%x\n", V20_TOOLS_VERSION, file_head.version);
	}
	if(V20_FILE_MAGIC != file_head.magic){
		printf("error: tool mag=%x, file mag=%x\n", V20_FILE_MAGIC, file_head.magic);
		fclose(fp_in);
		return -1;
	}
	
	//printf("file_head:magic=0x%08X, crc=0x%08X, file_cnt=%u, user_info=%s\n", file_head.magic, file_head.crc, file_head.file_cnt, file_head.user_info);

	//crc check
	checksum = 0;
	while(!feof(fp_in)){
		len = fread(buf, sizeof(char), sizeof(buf), fp_in);
		if(len == 0){
			break;
		}else if(len < 0){
			printf("read file error, ret=%d\n", len);
			fclose(fp_in);
			return -1;
		}
		checksum = crc32(checksum, buf, len);
	}
	if(checksum == file_head.crc){
		printf("crc check ok\n");
	}else{
		printf("crc check fail, check_crc=0x%08X\n", checksum);
		fclose(fp_in);
		return -1;
	}
	
	//zfk 20120301
	#if 0
	int ret = CheckUpdateFileModel(file_head.user_info);

	if(ret < 0)
	{
		fclose(fp_in);
		return -1;
	}
	#endif
	
	//获得flash容量
	all_mtd_flash_cap = get_flash_cap();
	printf("all_mtd_flash_cap=0x%x\n", all_mtd_flash_cap);
	if(all_mtd_flash_cap == 0){
		fclose(fp_in);
		return -1;
	}
	
	fd = open("/dev/mtd/0", O_SYNC | O_RDWR);
	if (fd < 0){
		perror("/dev/mtd/0");
		fclose(fp_in);
		return -1;
	}
	rtn = ioctl(fd, MEMGETINFO, &mtd);
	if (rtn < 0){
		perror("MEMGETINFO");
		fclose(fp_in);
		return -1;
	}
	close(fd);
	
	gFnUpgradeCb = pFnCb;
	
	//检查容量及对齐
	size = 0;
	for(file_idx = 0; file_idx < file_head.file_cnt; file_idx++){
		fseek(fp_in, sizeof(file_head_t) + (file_idx * sizeof(file_info_t)), SEEK_SET);
		len = fread(&file_info, sizeof(char), sizeof(file_info_t), fp_in);
		if(len != sizeof(file_info_t)){
			printf("read file error, ret=%d\n", len);
			fclose(fp_in);
			return -1;
		}
		//printf("get file:%s, len=%d, flash_oft=0x%x\n", file_info.file_name, file_info.len, file_info.flash_oft);
		if(file_info.flash_oft + file_info.len > all_mtd_flash_cap){
			printf("out of flash size, oft=0x%x, len=%d\n", file_info.flash_oft, file_info.len);
			return -1;
		}
		if((file_info.flash_oft % mtd.erasesize) != 0){
			printf("not in flash bank start, oft=0x%x, erasesize=%d\n", file_info.flash_oft, mtd.erasesize);
			fclose(fp_in);
			return -1;
		}
		process_total_size += file_info.len;
	}

	printf("%s check flash & align ok\n", __func__);
	fflush(stdout);
	
	//write flash
	for(file_idx = 0; file_idx < file_head.file_cnt; file_idx++){
		fseek(fp_in, sizeof(file_head_t) + (file_idx * sizeof(file_info_t)), SEEK_SET);
		len = fread(&file_info, sizeof(char), sizeof(file_info_t), fp_in);
		if(len != sizeof(file_info_t)){
			printf("read file error, ret=%d\n", len);
			fclose(fp_in);
			return -1;
		}
		//printf("get file:%s, len=%d, flash_oft=0x%x\n", file_info.file_name, file_info.len, file_info.flash_oft);
		size = file_info.len;
		write_flash(fp_in, file_info.file_oft, file_info.flash_oft, file_info.len);
	}
	
	fclose(fp_in);

	printf("upgrade over\n");
	fflush(stdout);
	
	return 0;
}

int GetFlashOffset(char* strFlashDev)
{
	char tmp_buf[256];
	struct mtd_info_user mtd;
	unsigned int nOft = 0;
	int mtd_idx = 0;
	int fd, rtn;
	
	while (1)
	{
		sprintf(tmp_buf, "/dev/mtd/%d", mtd_idx++);
		if ( 0 == strcmp(strFlashDev, tmp_buf))
		{
			return nOft;
		}
		
		fd = open(tmp_buf, O_SYNC | O_RDWR);
		if (fd < 0){
			break;
		}
		rtn = ioctl(fd, MEMGETINFO, &mtd);
		if (rtn < 0){
			perror("MEMGETINFO");
			close(fd);
			return 0;
		}
		nOft += mtd.size;
		close(fd);
	}
	
	return -1;
}

s32 UpgradeStartLogo(char* strLogoFile, pFnUpgradeProgressCb pFnCb, char* strFlashDev, int nOft, int nLimitSize)
{
	if (strFlashDev && strLogoFile && strcmp(strFlashDev, "/dev/mtd/0"))
	{
		struct stat stbuf;
		int nOftTotal;
		FILE* fp_in;

		if (stat(strLogoFile, &stbuf))
		{
			printf("UpgradeStartLogo stat file error\n");
			return -1;
		}
		
		if (stbuf.st_size > nLimitSize)
		{
			printf("UpgradeStartLogo stat file too large\n");
			return -1;
		}
		
		process_total_size = stbuf.st_size;
		process_cur_pos = 0;	

		gFnUpgradeCb = pFnCb;

		nOftTotal = GetFlashOffset(strFlashDev);
		if (nOftTotal >= 0x40000)
		{
			nOftTotal += nOft;
			fp_in = fopen(strLogoFile, "rb");
			if (!fp_in){
				printf("open file %s error\n", strLogoFile);
				return -1;
			}
			
			write_flash(fp_in, 0, nOftTotal, process_total_size);

			fclose(fp_in);

			return 0;
		}
	}
	
	return -1;
}

extern s32 GetContainerDev(char* strDev, u32 nMaxLen);
extern s32 CustomChange(EMCUSTOMTYPE emType, SCustomData* psCustomData, char* strContainerFile);
extern s32 CustomExportContainer(char* strDev, char* strFileOut);

s32 UpgradeAppLogo(char* strLogoFile, pFnUpgradeProgressCb pFnCb, char* strFlashDev, int nOft, int nLimitSize)
{
	if (strLogoFile)
	{
		struct stat stbuf;
		if (stat(strLogoFile, &stbuf))
		{
			printf("UpgradeAppLogo stat file error\n");
			return -1;
		}
		
		if (stbuf.st_size > 128*1024)
		{
			printf("UpgradeAppLogo stat file too large\n");
			return -1;
		}
		
		char strContainerDev[32], strContainer[32];
		if (strFlashDev)
		{
			if (0 == strcmp(strFlashDev, "/dev/mtd/0"))
			{
				return -1;
			}

			if (strlen(strFlashDev) >= sizeof(strContainerDev))
			{
				return -1;
			}
			
			strcpy(strContainerDev, strFlashDev);
		}
		else
		{
			if (GetContainerDev(strContainerDev, sizeof(strContainerDev)))
			{
				return -1;
			}
		}

		strcpy(strContainer, "container.bin");

		if (0 == CustomExportContainer(strContainerDev, strContainer))
		{
			EMCUSTOMTYPE emType = EM_CUSTOM_LOGO_APP;
			SCustomData sCustData;

			strcpy(sCustData.strFilePath, strLogoFile);
			
			if (0 == CustomChange(emType, &sCustData, strContainer))
			{
				struct stat stbuf;
				int nOftTotal;
				FILE* fp_in;

				if (stat(strContainer, &stbuf))
				{
					printf("UpgradeAppLogo stat file error\n");
					return -1;
				}
				
				if (stbuf.st_size > nLimitSize)
				{
					printf("UpgradeAppLogo stat file too large\n");
					return -1;
				}
				
				process_total_size = stbuf.st_size;
				process_cur_pos = 0;	

				gFnUpgradeCb = pFnCb;

				nOftTotal = GetFlashOffset(strContainerDev);
				if (nOftTotal >= 0x40000)
				{
					nOftTotal += nOft;
					fp_in = fopen(strContainer, "rb");
					if (fp_in)
					{
						write_flash(fp_in, 0, nOftTotal, stbuf.st_size);

						fclose(fp_in);

						remove(strContainer);

						return 0;
					}
					else
					{
						printf("open file %s error\n", strContainer);
					}
				}				
				remove(strContainer);
			}
		}

		remove(strContainer);

	}
	
	return -1;
}

#endif

