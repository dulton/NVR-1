
#include "common.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <sys/stat.h>
#include <sys/types.h>

#include <stdio.h>
#include <stddef.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

#include <sys/syscall.h>

int mount_user(char *mounted_path,char *user_path);
int umount_user(char *user_path);

int iflytek_system(char *command)
{
	//return ifly_partition("/dev/sda", 4, 0xc);
	
#if 1
	int wait_val, pid;
	__sighandler_t save_quit, save_int, save_chld;
	
	if (command == 0)
		return 1;
	
	save_quit = signal(SIGQUIT, SIG_IGN);
	save_int = signal(SIGINT, SIG_IGN);
	save_chld = signal(SIGCHLD, SIG_DFL);
	
	if ((pid = fork()) < 0) {
	//if ((pid = vfork()) < 0) {
		signal(SIGQUIT, save_quit);
		signal(SIGINT, save_int);
		signal(SIGCHLD, save_chld);
		return -1;
	}
	if (pid == 0) {
		signal(SIGQUIT, SIG_DFL);
		signal(SIGINT, SIG_DFL);
		signal(SIGCHLD, SIG_DFL);
		
		#ifdef __NR_vfork
		printf("__NR_vfork\n");
		#else
		printf("no __NR_vfork\n");
		#endif
		
		#ifdef _GNU_SOURCE
		printf("_GNU_SOURCE\n");
		#else
		printf("no _GNU_SOURCE\n");
		#endif
		
		#ifdef __USE_GNU
		printf("__USE_GNU\n");
		#else
		printf("no __USE_GNU\n");
		#endif
		
		//fcloseall();
		//fclose(fp);
		
		execl("/bin/sh", "sh", "-c", command, (char *) 0);
		_exit(127);
	}
	/* Signals are not absolutly guarenteed with vfork */
	signal(SIGQUIT, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	
#if 0
	printf("Waiting for child %d\n", pid);
#endif
	
	if (wait4(pid, &wait_val, 0, 0) == -1)
		wait_val = -1;
	
	signal(SIGQUIT, save_quit);
	signal(SIGINT, save_int);
	signal(SIGCHLD, save_chld);
	return wait_val;
#else
	return system(command);
#endif
}

//extern int mount_main(int argc, char **argv);
//extern int fdisk_main(int argc, char **argv);
extern int mkfsdos_main(int argc, char **argv);
extern int ifly_partition(char *path,int count,int fs);

extern int wget_user(char *user_name,char *user_passwd,char *hostname,int typeflag);
extern u32 GetBroadcast();
extern int SetBroadcast(u32 dwIp);

static FILE *fp = NULL;

int main(int argc, char *argv[])
{
#if 0
	return ifly_partition(argv[1], 4, 0xc);
	
//	char *devpathname = "/dev/sda";
//	return ifly_partition(devpathname,4,0xc);
	//return 0;
	
	u32 brdip = GetBroadcast();
	printf("brdip1=0x%08x\n",brdip);
	
	brdip = inet_addr("192.168.1.255");
	SetBroadcast(brdip);
	
	brdip = GetBroadcast();
	printf("brdip2=0x%08x\n",brdip);
	
	return 0;
	
	return wget_user("zerging","123","zerging.3322.org");
#endif
	
	mkdir("/root/myusb",1);
	
	int ret = mount_user("/dev/sdb1","/root/myusb");
	if(ret)
	{
		printf("此硬盘分区不存在\n");
		return -1;
	}
	
	//flock//fcntl//flockfile//ftrylockfile
	//FILE *fp = fopen("rec/a1/test.bin","w");
	fp = fopen("/root/myusb/test.bin","w");
	if(fp == NULL)
	{
		printf("fopen failed\n");
		return -1;
	}
	
	//fflush(fp);
	//fsync(fileno(fp));
	//fclose(fp);
	
	//iflytek_system("webs -p 3180 &");
	//sleep(3);
	
	fflush(fp);
	fsync(fileno(fp));
	fclose(fp);
	
	system("mount");
	system("ls");
	
	ret = umount_user("/root/myusb");
	if(ret)
	{
		printf("硬盘正在忙,此时不能格式化硬盘!\n");
		return -1;
	}
	
	system("mount");
	
//	ret = ifly_partition("/dev/hda",4,0xc);
//	printf("format result:%d\n",ret);
	
	return 0;
	
	//printf("hehe,before\n");
	//ftpget("192.168.1.72","rec/uImage.bin","uImage.bin");
	//printf("hehe,after\n");
	
#if 0
	int i = 0;
	int j = 0;
	while(1)
	{
		if(i%1 == 0)
		{
			j = i/1;
			printf("\n%dst:\n",j);
			system("free");
			printf("\n");
		}
		i++;
		
		int ret = ifly_partition("/dev/hda",4,0xc);
		if(ret == 0)
		{
			printf("ifly_partition:success\n");
		}
		else
		{
			printf("ifly_partition:failed\n");
		}
	}
#else
	int i = 0;
	int j = 0;
	while(1)
	{
		if(i%20 == 0)
		{
			j = i/20;
			printf("\n%dst:\n",j);
			system("free");
			printf("\n");
		}
		i++;
		
		int ret = ftpget("192.168.1.30","rec/uImage.bin","uImage.bin");
		if(ret == FTP_SUCCESS)
		{
			//printf("ftpget:success\n");
		}
		else if(ret == FTP_ERROR_SERVER)
		{
			printf("ftpget:connect failed\n");	
		}
		else if(ret == FTP_ERROR_FILE)
		{
			printf("ftpget:file is not exist\n");
		}
		else
		{
			printf("ftpget:other error,ret=%d\n",ret);
		}
	}
#endif

#if 0
	int comFd = OpenDev("/dev/tts/3");
	if(comFd > 0)
	{
		set_speed(comFd, 9600);
	}
	else
	{
		printf("Can't Open Serial Port!\n");
		exit(-1);
	}
	
	if(comFd > 0)
	{
		if(set_Parity(comFd, 8, 1, 'N') == -1)
		{
			printf("Set Parity Error\n");
			exit(-1);
		}
	}
	printf("open com succ,fd=%d\n",comFd);
	
	while(1)
	{
		int key;
		int nread = read(comFd, &key, sizeof(key));
		if(nread <= 0)
		{
			printf("recv key msg error,errno=%d,nread=%d\n",errno,nread);
			continue;
		}
		printf("recv key msg:0x%x\n",key);
	}
#endif

#if 0
	int  usb_index = 'a';
	char usb_device[32];
	int  ret = -1;
	mkdir("myusb",1);
	while(usb_index <= 'd')
	{
		//有些U盘比较特殊
		sprintf(usb_device,"/dev/sd%c",usb_index);
		ret = mount_user(usb_device,"myusb");
		if(ret == 0) break;
		
		int i;
		for(i=0;i<8;i++)
		{
			sprintf(usb_device,"/dev/sd%c%d",usb_index,i+1);
			ret = mount_user(usb_device,"myusb");
			if(ret == 0) break;
		}
		if(ret == 0) break;
		++usb_index;
	}
	if(ret == 0)
	{
		printf("mount usb device success\n");
	}
	else
	{
		printf("mount usb device failed\n");
	}
#endif
	
	/*int i =0;
	char *argv[10];
	argv[i++] = "mount";
	//argv[i++] = "-t";
	//argv[i++] = "vfat";
	argv[i++] = "/dev/sda1";
	argv[i++] = "myusb";
	mount_main(i, argv);*/
	
	/*char *argv[10];
	argv[0] = "fdisk";
	argv[1] = "/dev/hda";
	fdisk_main(2, argv);
	while(1)
	{
		sleep(10);
	}*/
	//ifly_partition("/dev/hda",4,0xc);
	
#if 0
	int i;
	for(i=0;i<10;i++)
	{
		char filename[32];
		sprintf(filename,"rec/a1/hiv%05d.ifv",i);
		FILE *fp = fopen(filename,"wb");
		fseek(fp,1024*1024-1,SEEK_SET);
		fputc(0,fp);
		fclose(fp);
	}
	/*printf("errno=%d,result=%s\n",errno,strerror(errno));
	FILE *fp = fopen("rec/a1/xfm00016.ifv","rb+");
	if(fp==NULL)
	{
		printf("fopen failed,errno=%d,result=%s\n",errno,strerror(errno));
	}
	else
	{
		printf("fopen success\n");

		int len = 0,ch = 23,ret = 0;
		fseek(fp,0,SEEK_END);
		len = ftell(fp);
		
		printf("len=%d\n",len);
		fseek(fp,len-1,SEEK_SET);
		ret = fread(&ch,1,1,fp);
		printf("ret=%d,ch=%d\n",ret,ch);
		
		fseek(fp,0,SEEK_SET);
		ret = fread(&ch,1,1,fp);
		printf("len=%d,ch=%d,ret=%d,errno=%d,result=%s\n",len,ch,ret,errno,strerror(errno));
		
		ch = 37;
		fseek(fp,0,SEEK_SET);
		ret = fwrite(&ch,1,1,fp);
		printf("len=%d,ch=%d,ret=%d,errno=%d,result=%s\n",len,ch,ret,errno,strerror(errno));
		
		ch = 34;
		fseek(fp,0,SEEK_SET);
		ret = fread(&ch,1,1,fp);
		printf("len=%d,ch=%d,ret=%d,errno=%d,result=%s\n",len,ch,ret,errno,strerror(errno));
		
		fclose(fp);
	}*/
	/*char *shortname = "HIV00000IFV";
	int i,j=0,chksum=0;
	for(i=11;i>0;i--)
	{
		chksum = ((chksum&1)?0x80:0) + ((chksum&0xfe) >> 1) + shortname[j++];
	}
	printf("chksum=%x\n",chksum);*/
#else
	/*int i =0;
	char *argv[10];
	argv[i++] = "mkfsdos";
	argv[i++] = "/dev/hda1";
	mkfsdos_main(i, argv);*/
#endif
	
	//mount_user("/dev/sda1","myusb");
	//umount_user("myusb");
	
	/*u32 gateway = GetDefaultGateway();
	printf("gateway=0x%08x\n",gateway);
	gateway = inet_addr("192.168.1.101");
	SetDefaultGateway(gateway);*/
	
	return 0;
}

