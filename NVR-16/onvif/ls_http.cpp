#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>

#include <string.h>
#include "ls_http.h"
#include "ls_error.h"
#include "ls_socket.h"
#include "ls_string.h"

//csp modify
//#define HTTP_CONNECT_TIME 1000000  //microseconds
#define HTTP_CONNECT_TIME 5000  	//ºÁÃë

#define HTTP_READ_TIMEOUT 5			//seconds

ls_http_request::ls_http_request(ls_http_method method , int major_version,int minor_version, int peer_port)
{
	this->socket = -1;
	this->handler = NULL;
	this->out_data = NULL;
	this->major_version = major_version;
	this->minor_version = minor_version;
	this->peer_port = peer_port;
	ls_charbuf_init(&cb);
	switch(method)
	{
	case HTTP_GET:
		this->method = "GET";
		break;
	case HTTP_PUT:
		this->method = "PUT";
		break;
	case HTTP_POST:
		this->method = "POST";
		break;
	case HTTP_OPTIONS:
		this->method = "OPTIONS";
		break;
	case HTTP_HEAD:
		this->method = "HEAD";
		break;
	case HTTP_DELETE:
		this->method = "DELETE";
		break;
	case HTTP_TRACE:
		this->method = "TRACE";
		break;
	default:
		this->method = "POST";
		break;
	}
}

ls_http_request::~ls_http_request()
{
	ls_charbuf_clean(&cb);
	
	//csp modify
	if(socket != -1)
	{
		close(socket);
		socket = -1;
	}
}
 void ls_http_request::setUri(const char *uri,size_t len)
{
	this->uri.assign(uri,len);
}
 void ls_http_request::setContentType(const char *content_type,size_t len)
{
	this->content_type.assign(content_type,len);
}
 void ls_http_request::setContent(const char *content,size_t content_len)
{
	this->content.assign(content,content_len);
	this->content_len = content_len;
}
 void ls_http_request::setHost(const char *host,size_t len)
{
	this->host.assign(host,len);
}
 void ls_http_request::setHandler(HandlerProc *handler , void * client_data,void *out)
 {
	 this->handler = handler;
	 this->client_data = client_data;
	 this->out_data = out;
 }
Boolean ls_http_request::buildRequest()
{
#if 1
	if( 
		LS_ERRNO_OK  == ls_charbuf_append_back(&cb,"%s %s HTTP/%d.%d\r\n"
								"Content-Type: %s\r\n"
								"Host: %s\r\n"
								"Content-Length: %d\r\n"
								//"Accept-Encoding: gzip, deflate\r\n"
								"Connection: Close\r\n\r\n"
								"%s",
								method.c_str(),uri.c_str(),major_version,minor_version,
								content_type.c_str(),host.c_str(),content_len,content.c_str())
		)
	{
		return True;
	}
	else
		return False;
#else
	if( 
		LS_ERRNO_OK  == ls_charbuf_append_back(&cb,"%s %s HTTP/%d.%d\r\n"
								"Host: %s\r\n"
								"Content-Type: %s\r\n"
								"Content-Length: %d\r\n\r\n"
								"%s",
								method.c_str(),uri.c_str(),major_version,minor_version,
								host.c_str(),content_type.c_str(),content_len,content.c_str())
		)
	{
		return True;
	}
	else
		return False;
#endif
}

ssize_t ls_http_request::sendTo()
{
	char port[10]={'\0'};
	char            *request_buf = NULL;
    size_t          request_buf_len = 0;
	int error = 0;
	char local_port[10] = {'\0'};
    sprintf(port, "%d", peer_port);
	//uint8_t connected = 0;
	if ( False == buildRequest() )
	{
		lsprint("buildRequest error!\n");
		return -1;
	}
	
	//ls_charbuf_get_str(&cb, (char **)&request_buf);
	//lsprint("%s\n",request_buf);
	
	//lsprint("%s\t\t%s\n",port,host.c_str());
	if( LS_ERRNO_OK != (error = ls_sock_tcp_connect_wait(&socket,host.c_str(),port,"0.0.0.0",local_port,HTTP_CONNECT_TIME)) )
	{
		//csp modify
		socket = -1;
		
		//lsprint("ls_sock_tcp_connect_wait error:%d!\n",error);//csp modify
		return -1;
	}
	
	ls_charbuf_get_str(&cb, (char **)&request_buf);
	//lsprint("sendto=%s\nsocket=%d\n",request_buf,socket);
	//lsprint("sendto=%s\n",request_buf);
    ls_charbuf_get_strlen(&cb, &request_buf_len);
	return ls_write_all(socket,request_buf,request_buf_len);
}

//len indicate the length of buf
ssize_t ls_http_request::recvFrom()
{
	//printf("%s 1\n", __func__);
	if(this->socket < 0)
	{
		return -1;
	}
	
	ls_http_response http_response;
	if(LS_ERRNO_OK != read_fom_socket(&http_response))
	{
		//csp modify
		if(socket != -1)
		{
			close(socket);
			socket = -1;
		}
		
		return -2;
	}
	//printf("%s 2\n", __func__);
	//csp modify
	if(socket != -1)
	{
		close(socket);
		socket = -1;
	}
	
	ls_str_to_int(http_response.response+strlen("HTTP/1.1 "),&http_response.status);
	//printf("%s 3\n", __func__);
	//lsprint("responselen=%d\nrecvfrom=%s\n",http_response.response_len,http_response.response);
	
	if(handler != NULL && this->client_data!= NULL)
	{
		//printf("%s 4\n", __func__);
		if(this->out_data == NULL)
		{
			//printf("responselen=%d\nrecvfrom=%s\n",http_response.response_len,http_response.response);
			(*handler)(&http_response,(void *)this->client_data,NULL);
			//printf("%s 5\n", __func__);
		}
		else
		{
			(*handler)(&http_response,(void *)this->client_data,this->out_data);
			//printf("%s 6\n", __func__);
		}
	}
	//lsprint("%d\n%s\n%d\n%s\n",http_response.response_len,http_response.response,http_response.respone_content_len,http_response.respone_content);
	
	free(http_response.response);
	return 0;
}

#define MALLOC_SIZE 6000
int32_t ls_http_request::read_fom_socket (ls_http_response* http_response)
{
	int32_t m;
	
	uint32_t length;
	long flags;
	char *fbuf = NULL;
	char *p = NULL;
	
	//csp modify
	//char buf[2048];
	char buf[2048+16];
	buf[2048] = 0;
	buf[sizeof(buf)-1] = 0;
	
	struct timeval timeout={HTTP_READ_TIMEOUT,0}; 
	
	//momery  alloc
	size_t used = 0;
	size_t multiple = 1;
	http_response->response = (char *)malloc(MALLOC_SIZE * multiple *sizeof(char ));
	if(http_response->response == NULL)
	{
		//csp modify
		close(socket);
		socket = -1;
		
		return LS_ERRNO_NOMEM;
	}
	
	//set block
	flags = fcntl(socket, F_GETFL);
	fcntl(socket, F_SETFL, flags  & ~O_NONBLOCK);
	
	m = 0;
	length = 0;
	
	//csp modify
	http_response->respone_content_len = 0;
	http_response->respone_content = NULL;
	
	//set timeout 
	setsockopt(socket,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(struct timeval));

	//struct timeval tv1;
	//printf("%s 1\n", __func__);
	while(1)
	{
		//gettimeofday(&tv1,NULL);
		//printf("%s read s tv1:%u.%u\n", __func__, tv1.tv_sec, tv1.tv_usec);
		m = read(socket,buf,2048);
		
		//buf[m] = 0;//csp modify
		
		//lsprint("%s",buf);
		//printf("%s m: %d\n", __func__, m);
		//gettimeofday(&tv1,NULL);
		//printf("%s read e tv1:%u.%u\n", __func__, tv1.tv_sec, tv1.tv_usec);
		
		if(m <= 0)
			break;
		if(used + m  >= MALLOC_SIZE * multiple )
		{
			++multiple; 
            		http_response->response = (char *)realloc(http_response->response, MALLOC_SIZE * multiple *sizeof(char ));
			// recalculate the content
			fbuf = NULL;
			
			//csp modify
			length = 0;
			http_response->respone_content_len = 0;
			http_response->respone_content = NULL;
			
			if(http_response->response == NULL)
			{
				//csp modify
				close(socket);
				socket = -1;
				
				return LS_ERRNO_NOMEM;
			}
		}
		memcpy(http_response->response+used,buf,m);
		used += m;
		//printf("%s 1 used: %d, length: %d\n", __func__, used, length);
		//printf("%s\n", http_response->response);
		if(fbuf == NULL)
		{
			fbuf = strstr(http_response->response,"Content-Length");
			
			//csp modify
			if(fbuf != NULL)
			{
				if(strstr(fbuf,"\r\n\r\n") == NULL)
				{
					fbuf = NULL;
				}
			}
			
			if(fbuf != NULL)
			{
				while( (fbuf < http_response->response + used ) && !isdigit(*fbuf) ) ++fbuf;
				ls_str_to_uint(fbuf,&length);
				//printf("%s 2 length: %d\n", __func__, length);
				
				http_response->respone_content_len = length;
				
				
				//csp modify
				#if 1
				p = strstr(fbuf,"\r\n\r\n");
				http_response->respone_content = p + 4;
				length += http_response->respone_content - http_response->response;
				//printf("%s 3 used: %d, length: %d\n", __func__, used, length);
				//printf("%s respone_content-response: %u\n", __func__, http_response->respone_content - http_response->response);
				if(http_response->respone_content >= http_response->response + used)
				{
					//printf("%s 6 used: %d, length: %d\n", __func__, used, length);
					fbuf = NULL;
					length = 0;
					http_response->respone_content_len = 0;
					http_response->respone_content = NULL;
					
				}
				#else
				length += fbuf - http_response->response;
				p = fbuf;
				while ( (p < http_response->response + used ) )
				{
					if( p + 4 < http_response->response + used  && p[0] == '\r' &&p[1] == '\n' && p[2] == '\r' && p[3] == '\n')
					{
							length += 4;
							http_response->respone_content = p + 4;
							//while(*http_response->respone_content == ' ') ++http_response->respone_content;
							break;
					}
					++p;
					++length;
				}
				if(p >= http_response->response + used)
				{
					fbuf = NULL;
					
					//csp modify
					length = 0;
					http_response->respone_content_len = 0;
					http_response->respone_content = NULL;
				}
				#endif
			}

			//yaogang modify 20150811
			if(length != 0 && used >= length)
			{
				//printf("%s 4 used: %d, length: %d\n", __func__, used, length);
				break;
			}
		}
		else
		{
			//lsprint("%d:%d\n",used,length);
			if(length != 0 && used >= length)
			{
				//printf("%s 5 used: %d, length: %d\n", __func__, used, length);
				break;				
			}
		}
	}

	//printf("%s 2\n", __func__);
	
	
	close(socket);
	socket = -1;
	
	//csp modify
	if(http_response->respone_content == NULL || http_response->respone_content_len == 0)
	{
		if(http_response->response != NULL)
		{
			free(http_response->response);
			http_response->response = NULL;
		}
		return LS_ERRNO_NOMEM;
	}
	
	http_response->response_len = used;
	http_response->response[used] = 0;
	
	//fcntl(socket, F_SETFL, flags);//csp modify
	
	return LS_ERRNO_OK;
}

