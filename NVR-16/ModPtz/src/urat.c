#include <stdio.h>		/*标准输入输出定义 */
#include <stdlib.h>		/*标准函数库定义 */
#include <unistd.h>		/*Unix标准函数定义 */
#include <sys/types.h> 	/**/
#include <sys/stat.h>	/**/
#include <fcntl.h>		/*文件控制定义 */
#include <termios.h>	/*PPSIX终端控制定义 */
#include <errno.h>		/*错误号定义 */

static int speed_arr[] = { B115200, B57600, B38400, B19200, B9600, B4800, B2400, B1200, B300 };
static int name_arr[]  = { 115200, 57600, 38400, 19200, 9600, 4800, 2400, 1200, 300 };

/**
*@brief  设置串口通信速率
*@param  fd     类型 int  打开串口的文件句柄
*@param  speed  类型 int  串口速度
*@return  void
***/
int set_speed(int fd, int speed)
{
	int i;
	int status;
	struct termios Opt;
	tcgetattr (fd, &Opt);
	
	Opt.c_cflag |= (CLOCAL | CREAD);
	Opt.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);	//raw input
	//Opt.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHONL | ISIG);	//raw input
	Opt.c_iflag &= ~(IXON | IXOFF | IXANY | ICRNL);		//disable software flow control
	Opt.c_cflag &= ~CRTSCTS;							//disable hardware flow control
	Opt.c_oflag &= ~OPOST;
	
	for(i = 0; i < sizeof (speed_arr) / sizeof (int); i++)
	{
		if(speed == name_arr[i])
		{
			tcflush(fd, TCIOFLUSH);
			cfsetispeed(&Opt, speed_arr[i]);
			cfsetospeed(&Opt, speed_arr[i]);
			
			status = tcsetattr(fd, TCSANOW, &Opt);
			if(status != 0)
			{
				perror("tcsetattr fd1");
				return -1;
			}
			return 0;
		}
		//tcflush(fd, TCIOFLUSH);
	}

	return -1;
}

/**
 *@brief   设置串口数据位，停止位和效验位
 *@param  fd     类型  int  打开的串口文件句柄*
 *@param  databits 类型  int 数据位   取值 为 7 或者8*
 *@param  stopbits 类型  int 停止位   取值为 1 或者2*
 *@param  parity  类型  int  效验类型 取值为N,E,O,,S
 */
int set_Parity(int fd, int databits, int stopbits, int parity)
{
	struct termios options;
	if(tcgetattr(fd, &options) != 0)
	{
		perror("SetupSerial 1");
		return -1;
	}
	
	/* 设置数据位位 */
	options.c_cflag &= ~CSIZE;
	switch(databits)
	{
		case 7:
			options.c_cflag |= CS7;
			break;
		case 8:
			options.c_cflag |= CS8;
			break;
		default:
			fprintf(stderr, "Unsupported data size\n");
			return -1;
	}
	
	/* 设置奇偶校验位 */
	switch(parity)
	{
		case 'n':
		case 'N':
			options.c_cflag &= ~PARENB;	/* Clear parity enable */
			options.c_iflag &= ~INPCK;	/* Enable parity checking */
			break;
		case 'o':
		case 'O':
			options.c_cflag |= (PARODD | PARENB);	/* 设置为奇效验 */
			options.c_iflag |= INPCK;	/* Disnable parity checking */
			break;
		case 'e':
		case 'E':
			options.c_cflag |= PARENB;	/* Enable parity */
			options.c_cflag &= ~PARODD;	/* 转换为偶效验 */
			options.c_iflag |= INPCK;	/* Disnable parity checking */
			break;
		case 'S':
		case 's':			/*as no parity */
			options.c_cflag &= ~PARENB;
			options.c_cflag &= ~CSTOPB;
			break;
		default:
			fprintf(stderr, "Unsupported parity\n");
			return -1;
	}
	
	/* 设置停止位 */
	switch(stopbits)
	{
		case 1:
			options.c_cflag &= ~CSTOPB;
			break;
		case 2:
			options.c_cflag |= CSTOPB;
			break;
		default:
			fprintf(stderr, "Unsupported stop bits\n");
			return -1;
	}

	/* Set input parity option */
	if(parity != 'n' && parity != 'N')
		options.c_iflag |= INPCK;

	options.c_cc[VTIME] = 0;	// 15 seconds
	options.c_cc[VMIN] = 4;//0;

	tcflush(fd, TCIFLUSH);	/* Update the options and do it NOW */
	if(tcsetattr (fd, TCSANOW, &options) != 0)
	{
		perror("SetupSerial 3");
		return -1;
	}
	
	return 0;
}

/**
 *@breif 打开串口
 */
int OpenDev(char *Dev)
{
	//int fd = open (Dev, O_RDWR);	//| O_NOCTTY | O_NDELAY
	int fd = open(Dev, O_RDWR | O_NOCTTY);
	if(-1 == fd)
	{				/*设置数据位数 */
		perror("Can't Open Serial Port");
		return -1;
	}
	else
	{
		return fd;
	}
}

