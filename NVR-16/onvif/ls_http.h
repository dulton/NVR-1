#ifndef __LS_HTTP_H
#define __LS_HTTP_H 1



#include "ls_type.h"
#include "ls_charbuf.h"
#include <string>
using namespace std;


typedef enum
{
	HTTP_GET,
	HTTP_PUT,
	HTTP_POST,
	HTTP_OPTIONS,
	HTTP_HEAD,
	HTTP_DELETE,
	HTTP_TRACE
}ls_http_method;

typedef struct ls_http_response
{
	char * response;
	int32_t response_len;
	int status ;
	char * respone_content;
	int32_t respone_content_len;
}ls_http_response;


typedef void HandlerProc(ls_http_response* http_response,void *clientData,void *out = NULL);
class ls_http_request
{
public:
	ls_http_request(ls_http_method method , int major_version,int minor_version, int peer_port);
	~ls_http_request();
	void setUri(const char *uri,size_t len);
	void setContentType(const char *content_type,size_t len);
	void setContent(const char *content,size_t content_len);
	void setHost(const char *host,size_t len);
	//we use out return the data 
	void setHandler(HandlerProc *handler,void * clientData,void *out = NULL );
	ssize_t recvFrom();
	ssize_t sendTo();

private:
	HandlerProc *handler;
	void *client_data;
	void *out_data;
	int32_t read_fom_socket (ls_http_response* http_response);
	Boolean buildRequest(); 
	ls_charbuf_t       cb;
private:
	int socket;
	int peer_port;
	string method;
	string uri;
	string host;
	int major_version;
	int minor_version;
	string content_type;
	string content;
	size_t content_len;
};


#endif
