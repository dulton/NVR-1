#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include <ctype.h>
#include <unistd.h>
#include <netdb.h>

#include "bbfunc.h"

#include "common.h"

#ifndef STDIN_FILENO
# define STDIN_FILENO 0
#endif

#ifndef STDOUT_FILENO
# define STDOUT_FILENO 1
#endif

#define RESERVE_CONFIG_BUFFER(buffer,len)           char buffer[len]
#define RELEASE_CONFIG_BUFFER(buffer)				((void)0)

typedef struct ftp_host_info_s {
	char *user;
	char *password;
	struct sockaddr_in *s_in;
} ftp_host_info_t;

static char verbose_flag = 0;
static char do_continue = 0;

static int ftpcmd(const char *s1, const char *s2, FILE *stream, char *buf)
{
	if (verbose_flag) {
		bb_error_msg("cmd %s%s", s1, s2);
	}
	
	if (s1) {
		if (s2) {
			fprintf(stream, "%s%s\r\n", s1, s2);
		} else {
			fprintf(stream, "%s\r\n", s1);
		}
	}
	do {
		char *buf_ptr;
		
		if (fgets(buf, 510, stream) == NULL) {
			bb_perror_msg_and_die("fgets()");
		}
		buf_ptr = strstr(buf, "\r\n");
		if (buf_ptr) {
			*buf_ptr = '\0';
		}
	} while (! isdigit(buf[0]) || buf[3] != ' ');
	
	return atoi(buf);
}

#include <sys/ioctl.h>

static int connectWithTimeout(int sockfd,const struct sockaddr *serv_addr,socklen_t addrlen)
{
	int ret;
	int arg;
	int dwTimeOut = 10;//连接超时时间(秒)
	fd_set ConnectSet;
	struct timeval timeVal;
	int nErrCode;
	socklen_t errorlen;
	
	//判断参数有效性
	if(sockfd < 0)
	{
		return -1;
	}
	
	//设置为非阻塞模式, 防止connect阻塞
	arg = 1;
	ret = ioctl(sockfd,FIONBIO,(int)&arg);
	if(ret < 0)
	{
		//close(sockfd);
		printf("set socket no block failed\n");
		return ret;
	}
	
	//连接
	ret = connect(sockfd,serv_addr,addrlen);
	if(ret == -1)
	{
		FD_ZERO(&ConnectSet);
		timeVal.tv_sec  = dwTimeOut;
		timeVal.tv_usec = 0;
		FD_SET(sockfd, &ConnectSet);
		ret = select(sockfd+1, NULL, &ConnectSet, NULL, &timeVal);
		if(ret > 0 && FD_ISSET(sockfd, &ConnectSet))
		{
			nErrCode = 1;
			errorlen = sizeof(nErrCode);
			ret = getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &nErrCode, &errorlen);
			if(ret == -1 || nErrCode != 0)
			{
				//close(sockfd);
				printf("connect error 1\n");
				return -1;
			}
		}
		else
		{
			//close(sockfd);
			if(ret == 0)
			{
				printf("connect time out\n");
				return -1;
			}
			else
			{
				printf("connect error 2\n");
				return -1;
			}
		}
	}
	
	//连接成功,设置为阻塞模式
	arg = 0;
	ret = ioctl(sockfd,FIONBIO,(int)&arg);
	if(ret == -1)
	{
		//close(sockfd);
		printf("set socket block failed\n");
		return ret;
	}
	
	return 0;
}

int xconnect(struct sockaddr_in *s_addr)
{
	int s = socket(AF_INET, SOCK_STREAM, 0);
	if(s < 0)
	{
		return -1;
	}
	//if (connect(s, (struct sockaddr *)s_addr, sizeof(struct sockaddr_in)) < 0)
	if (connectWithTimeout(s, (struct sockaddr *)s_addr, sizeof(struct sockaddr_in)) < 0)
	{
		close(s);
		bb_perror_msg_and_die("Unable to connect to remote host (%s)",
			inet_ntoa(s_addr->sin_addr));
		return -1;
	}
	return s;
}

static int xconnect_ftpdata(ftp_host_info_t *server, const char *buf)
{
	char *buf_ptr;
	unsigned short port_num;
	
	buf_ptr = strrchr(buf, ',');
	*buf_ptr = '\0';
	port_num = atoi(buf_ptr + 1);
	
	buf_ptr = strrchr(buf, ',');
	*buf_ptr = '\0';
	port_num += atoi(buf_ptr + 1) * 256;
	
	server->s_in->sin_port=htons(port_num);
	return(xconnect(server->s_in));
}

static FILE *ftp_login(ftp_host_info_t *server)
{
	FILE *control_stream;
	char buf[512];
	
	/* Connect to the command socket */
	control_stream = fdopen(xconnect(server->s_in), "r+");
	if (control_stream == NULL) {
		bb_perror_msg_and_die("Couldnt open control stream");
		return NULL;
	}
	
	if (ftpcmd(NULL, NULL, control_stream, buf) != 220) {
		bb_error_msg_and_die("%s", buf + 4);
		close(fileno(control_stream));
		fclose(control_stream);
		return NULL;
	}
	
	/*  Login to the server */
	switch (ftpcmd("USER ", server->user, control_stream, buf)) {
	case 230:
		break;
	case 331:
		if (ftpcmd("PASS ", server->password, control_stream, buf) != 230) {
			bb_error_msg_and_die("PASS error: %s", buf + 4);
			close(fileno(control_stream));
			fclose(control_stream);
			return NULL;
		}
		break;
	default:
		bb_error_msg_and_die("USER error: %s", buf + 4);
		close(fileno(control_stream));
		fclose(control_stream);
		return NULL;
	}
	
	ftpcmd("TYPE I", NULL, control_stream, buf);
	
	return(control_stream);
}

int bb_xopen(const char *pathname, int flags)
{
	int ret;
	
	ret = open(pathname, flags, 0777);
	if (ret < 0) {
		bb_perror_msg_and_die("%s", pathname);
	}
	return ret;
}

int safe_strtoul(char *arg, unsigned long* value)
{
	char *endptr;
	int errno_save = errno;
	
	assert(arg!=NULL);
	errno = 0;
	*value = strtoul(arg, &endptr, 0);
	if (errno != 0 || *endptr!='\0' || endptr==arg) {
		return 1;
	}
	errno = errno_save;
	return 0;
}

ssize_t safe_read(int fd, void *buf, size_t count)
{
	ssize_t n;
	
	do {
		n = read(fd, buf, count);
	} while (n < 0 && errno == EINTR);
	
	return n;
}

ssize_t safe_write(int fd, const void *buf, size_t count)
{
	ssize_t n;
	
	do {
		n = write(fd, buf, count);
	} while (n < 0 && errno == EINTR);
	
	return n;
}

ssize_t bb_full_write(int fd, const void *buf, size_t len)
{
	ssize_t cc;
	ssize_t total;
	
	total = 0;
	
	while (len > 0) {
		cc = safe_write(fd, buf, len);
		
		if (cc < 0)
			return cc;		/* write() returns -1 on failure. */
		
		total += cc;
		buf = ((const char *)buf) + cc;
		len -= cc;
	}
	
	return total;
}

static ssize_t bb_full_fd_action(int src_fd, int dst_fd, size_t size)
{
	int status = -1;
	size_t total = 0;
	RESERVE_CONFIG_BUFFER(buffer,BUFSIZ);
	
	if (src_fd < 0) goto out;
	while (!size || total < size)
	{
		ssize_t wrote, xread;
		
		xread = safe_read(src_fd, buffer,
			(!size || size - total > BUFSIZ) ? BUFSIZ : size - total);
		
		if (xread > 0) {
			/* A -1 dst_fd means we need to fake it... */
			wrote = (dst_fd < 0) ? xread : bb_full_write(dst_fd, buffer, xread);
			if (wrote < xread) {
				bb_perror_msg("Write Error");
				break;
			}
			total += wrote;
			if (total == size) status = 0;
		} else if (xread < 0) {
			bb_perror_msg("Read Error");
			break;
		} else if (xread == 0) {
			/* All done. */
			status = 0;
			break;
		}
	}
	
out:
	RELEASE_CONFIG_BUFFER(buffer);
	
	return status ? status : (ssize_t)total;
}

int bb_copyfd_size(int fd1, int fd2, const off_t size)
{
	if (size) {
		return(bb_full_fd_action(fd1, fd2, size));
	}
	return(0);
}

int bb_copyfd_eof(int fd1, int fd2)
{
	return(bb_full_fd_action(fd1, fd2, 0));
}

static int ftp_receive(ftp_host_info_t *server, FILE *control_stream,
					   const char *local_path, char *server_path)
{
	char buf[512];
	off_t filesize = 0;
	int fd_data;
	int fd_local = -1;
	off_t beg_range = 0;
	
	/* Connect to the data socket */
	if (ftpcmd("PASV", NULL, control_stream, buf) != 227) {
		bb_error_msg_and_die("PASV error: %s", buf + 4);
		close(fileno(control_stream));
		fclose(control_stream);
		return FTP_ERROR_SERVER;
	}
	
	fd_data = xconnect_ftpdata(server, buf);
	if(fd_data < 0)
	{
		close(fileno(control_stream));
		fclose(control_stream);
		return FTP_ERROR_SERVER;
	}

	if (ftpcmd("SIZE ", server_path, control_stream, buf) == 213) {
		unsigned long value=filesize;
		if (safe_strtoul(buf + 4, &value)) {
			bb_error_msg_and_die("SIZE error: %s", buf + 4);
			close(fd_data);
			close(fileno(control_stream));
			fclose(control_stream);
			return FTP_ERROR_SERVER;
		}
		filesize = value;
	} else {
		filesize = -1;
		do_continue = 0;	
	}

	if ((local_path[0] == '-') && (local_path[1] == '\0')) {
		fd_local = STDOUT_FILENO;
		do_continue = 0;
	}
	
	if (do_continue) {
		struct stat sbuf;
		memset(&sbuf,0,sizeof(sbuf));
		if (lstat(local_path, &sbuf) < 0) {
			do_continue = 0;
		}
		if (sbuf.st_size > 0) {
			beg_range = sbuf.st_size;
		} else {
			do_continue = 0;
		}
	}
	
	if (do_continue) {
		sprintf(buf, "REST %ld", (long)beg_range);
		if (ftpcmd(buf, NULL, control_stream, buf) != 350) {
			do_continue = 0;
		} else {
			filesize -= beg_range;
		}
	}
	
	if (ftpcmd("RETR ", server_path, control_stream, buf) > 150) {
		bb_error_msg_and_die("RETR error: %s", buf + 4);
		close(fd_data);
		close(fileno(control_stream));
		fclose(control_stream);
		return FTP_ERROR_FILE;
	}
	
	/* only make a local file if we know that one exists on the remote server */
	if (fd_local == -1) {
		if (do_continue) {
			fd_local = bb_xopen(local_path, O_APPEND | O_WRONLY);
		} else {
			fd_local = bb_xopen(local_path, O_CREAT | O_TRUNC | O_WRONLY);
		}
	}
	
	/* Copy the file */
	if (filesize != -1) {
		if (-1 == bb_copyfd_size(fd_data, fd_local, filesize)) {
			if(fd_local != STDOUT_FILENO) close(fd_local);
			close(fd_data);
			close(fileno(control_stream));
			fclose(control_stream);
			return FTP_ERROR_FILE;
		}
	} else {
		if (-1 == bb_copyfd_eof(fd_data, fd_local)) {
			if(fd_local != STDOUT_FILENO) close(fd_local);
			close(fd_data);
			close(fileno(control_stream));
			fclose(control_stream);
			return FTP_ERROR_FILE;
		}
	}
	
	/* close it all down */
	if(fd_local != STDOUT_FILENO) close(fd_local);
	close(fd_data);

	if (ftpcmd(NULL, NULL, control_stream, buf) != 226) {
		bb_error_msg_and_die("ftp error: %s", buf + 4);
		close(fileno(control_stream));
		fclose(control_stream);
		return FTP_ERROR_SERVER;
	}
	ftpcmd("QUIT", NULL, control_stream, buf);
	
	close(fileno(control_stream));
	fclose(control_stream);

	return FTP_SUCCESS;
}

static int ftp_send(ftp_host_info_t *server, FILE *control_stream,
					const char *server_path, char *local_path)
{
	struct stat sbuf;
	char buf[512];
	int fd_data;
	int fd_local;
	int response;
	
	/*  Connect to the data socket */
	if (ftpcmd("PASV", NULL, control_stream, buf) != 227) {
		bb_error_msg_and_die("PASV error: %s", buf + 4);
		close(fileno(control_stream));
		fclose(control_stream);
		return FTP_ERROR_SERVER;
	}
	fd_data = xconnect_ftpdata(server, buf);
	if(fd_data < 0)
	{
		close(fileno(control_stream));
		fclose(control_stream);
		return FTP_ERROR_SERVER;
	}
	
	/* get the local file */
	if ((local_path[0] == '-') && (local_path[1] == '\0')) {
		fd_local = STDIN_FILENO;
	} else {
		fd_local = bb_xopen(local_path, O_RDONLY);
		if(fd_local < 0)
		{
			printf("ftp send error:file is not exist\n");
			close(fd_local);
			close(fd_data);
			close(fileno(control_stream));
			fclose(control_stream);
			return FTP_ERROR_FILE;
		}
		fstat(fd_local, &sbuf);
		
		sprintf(buf, "ALLO %lu", (unsigned long)sbuf.st_size);
		response = ftpcmd(buf, NULL, control_stream, buf);
		switch (response) {
		case 200:
		case 202:
			break;
		default:
			close(fd_local);
			bb_error_msg_and_die("ALLO error: %s", buf + 4);
			close(fd_data);
			close(fileno(control_stream));
			fclose(control_stream);
			return FTP_ERROR_SERVER;
			break;
		}
	}
	response = ftpcmd("STOR ", server_path, control_stream, buf);
	switch (response) {
	case 125:
	case 150:
		break;
	default:
		close(fd_local);
		bb_error_msg_and_die("STOR error: %s", buf + 4);
		close(fd_data);
		close(fileno(control_stream));
		fclose(control_stream);
		return FTP_ERROR_SERVER;
	}
	
	/* transfer the file  */
	if (bb_copyfd_eof(fd_local, fd_data) == -1) {
		close(fd_local);
		close(fd_data);
		close(fileno(control_stream));
		fclose(control_stream);
		return FTP_ERROR_FILE;
	}
	
	/* close it all down */
	close(fd_local);
	close(fd_data);

	if (ftpcmd(NULL, NULL, control_stream, buf) != 226) {
		bb_error_msg_and_die("error: %s", buf + 4);
		close(fileno(control_stream));
		fclose(control_stream);
		return FTP_ERROR_SERVER;
	}
	ftpcmd("QUIT", NULL, control_stream, buf);
	
	close(fileno(control_stream));
	fclose(control_stream);
	
	return FTP_SUCCESS;
}

void bb_vherror_msg(const char *s, va_list p)
{
	if(s == 0)
		s = "";
	bb_verror_msg(s, p);
	if (*s)
		fputs(": ", stderr);
	herror("");
}

void bb_herror_msg_and_die(const char *s, ...)
{
	va_list p;
	
	va_start(p, s);
	bb_vherror_msg(s, p);
	va_end(p);
}

struct hostent *xgethostbyname(const char *name)
{
	struct hostent *retval;
	
	#if 0//
	return NULL;
	#else
	if ((retval = gethostbyname(name)) == NULL)
		bb_herror_msg_and_die("%s", name);
	#endif
	
	return retval;
}

void bb_lookup_host(struct sockaddr_in *s_in, const char *host)
{
	struct hostent *he;
	
	memset(s_in, 0, sizeof(struct sockaddr_in));
	s_in->sin_family = AF_INET;
	he = xgethostbyname(host);
	memcpy(&(s_in->sin_addr), he->h_addr_list[0], he->h_length);
}

unsigned short bb_lookup_port(const char *port, const char *protocol, unsigned short default_port)
{
	unsigned short port_nr = htons(default_port);
	if (port) {
		char *endptr;
		int old_errno;
		long port_long;

		/* Since this is a lib function, we're not allowed to reset errno to 0.
		 * Doing so could break an app that is deferring checking of errno. */
		old_errno = errno;
		errno = 0;
		port_long = strtol(port, &endptr, 10);
		if (errno != 0 || *endptr!='\0' || endptr==port || port_long < 0 || port_long > 65535) {
			struct servent *tserv = getservbyname(port, protocol);
			if (tserv) {
				port_nr = tserv->s_port;
			}
		} else {
			port_nr = htons(port_long);
		}
		errno = old_errno;
	}
	return port_nr;
}

int ftpgetput_main(int argc, char **argv)
{
	/* content-length of the file */
	char *port = "ftp";

	/* socket to ftp server */
	FILE *control_stream;
	struct sockaddr_in s_in;

	/* continue a prev transfer (-c) */
	ftp_host_info_t *server;
	ftp_host_info_t host_info;

	int (*ftp_action)(ftp_host_info_t *, FILE *, const char *, char *) = NULL;

	/* Check to see if the command is ftpget or ftput */
	if (argv[0][3] == 'p') {
		ftp_action = ftp_send;
	}
	else if (argv[0][3] == 'g') {
		ftp_action = ftp_receive;
	}
	else {
		return FTP_ERROR_PARAM;
	}

	/* Set default values */
	server = &host_info;
	server->user = "anonymous";
	server->password = "busybox@";
	
	verbose_flag = 1;//cyl debug it
	do_continue = 0;
	
	/* Process the non-option command line arguments */
	if (argc < 4) {
		bb_show_usage();
		return FTP_ERROR_PARAM;
	}
	
	/* We want to do exactly _one_ DNS lookup, since some
	 * sites (i.e. ftp.us.debian.org) use round-robin DNS
	 * and we want to connect to only one IP... */
	server->s_in = &s_in;
	bb_lookup_host(&s_in, argv[1]);
	s_in.sin_port = bb_lookup_port(port, "tcp", 21);
	if (verbose_flag) {
		printf("Connecting to %s[%s]:%d\n",
				argv[1], inet_ntoa(s_in.sin_addr), ntohs(s_in.sin_port));
	}

	/*  Connect/Setup/Configure the FTP session */
	control_stream = ftp_login(server);
	if(control_stream == NULL)
	{
		return FTP_ERROR_SERVER;
	}

	return(ftp_action(server, control_stream, argv[2], argv[3]));
}

int ftpget(char *serverip,char *localfile,char *remotefile)
{
	char* my_argv[4] = {"ftpg",serverip,localfile,remotefile};
	return ftpgetput_main(4, my_argv);
}

int ftpput(char *serverip,char *remotefile,char *localfile)
{
	char* my_argv[4] = {"ftpp",serverip,remotefile,localfile};
	return ftpgetput_main(4, my_argv);
}
