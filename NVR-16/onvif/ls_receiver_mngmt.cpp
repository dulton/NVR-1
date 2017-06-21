
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include "ls_string.h"
#include <string.h>
#include "ls_socket.h"
#include "ls_sha.h"
#include "ls_base64.h"
#include "ls_receiver_mngmt.h"
#include "ls_string.h"
using namespace std;

typedef struct
{
    const char *prefix;
    const char *ns_uri;
} xmlns_info_t;

xmlns_info_t onvif_xmlns_info[] = {
    {"SOAP-ENV", "http://www.w3.org/2003/05/soap-envelope"},
    {"SOAP-ENC", "http://www.w3.org/2003/05/soap-encoding"},
    {"xsi",      "http://www.w3.org/2001/XMLSchema-instance"},
    {"xsd",      "http://www.w3.org/2001/XMLSchema"},
    {"wsa",      "http://schemas.xmlsoap.org/ws/2004/08/addressing"},
    {"xmime",    "http://tempuri.org/xmime.xsd"},
    {"xop",      "http://www.w3.org/2004/08/xop/include"},
    {"tt",       "http://www.onvif.org/ver10/schema"},
    {"wsrf_bf",  "http://tempuri.org/wsrf_bf.xsd"},
    {"wstop",    "http://docs.oasis-open.org/wsn/t-1"},
    {"c14n",     "http://www.w3.org/2001/10/xml-exc-c14n#"},
    {"wsu",      "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd"},
    {"xenc",     "http://www.w3.org/2001/04/xmlenc#"},
    {"ds",       "http://www.w3.org/2000/09/xmldsig#"},
    {"wsrf_r2",  "http://tempuri.org/wsrf_r2.xsd"},
    {"ns10",     "http://tempuri.org/NetworkSoap"},
    {"ns11",     "http://tempuri.org/NetworkSoap12"},
    {"ns12",     "http://tempuri.org/PTZSoap"},
    {"ns13",     "http://tempuri.org/PTZSoap12"},
    {"ns14",     "http://tempuri.org/UtilitySoap"},
    {"ns15",     "http://tempuri.org/UtilitySoap12"},
    {"ns16",     "http://tempuri.org/VideoAnalysisSoap"},
    {"ns17",     "http://tempuri.org/VideoAnalysisSoap12"},
    {"ns2",      "http://tempuri.org/AccountSoap"},
    {"ns3",      "http://tempuri.org/AccountSoap12"},
    {"ns4",      "http://tempuri.org/AudioVideoSoap"},
    {"ns5",      "http://tempuri.org/AudioVideoSoap12"},
    {"ns6",      "http://tempuri.org/BasicSystemSoap"},
    {"ns7",      "http://tempuri.org/BasicSystemSoap12"},
    {"ns8",      "http://tempuri.org/EventScheduleSoap"},
    {"ns1",      "http://tempuri.org/"},
    {"ns9",      "http://tempuri.org/EventScheduleSoap12"},
    {"tds",      "http://www.onvif.org/ver10/device/wsdl"},
    {"tev_1",    "http://tempuri.org/tev_1.xsd"},
    {"tev_2",    "http://tempuri.org/tev_2.xsd"},
    {"tev_3",    "http://tempuri.org/tev_3.xsd"},
    {"tev",      "http://www.onvif.org/ver10/events/wsdl"},
    {"tev_4",    "http://tempuri.org/tev_4.xsd"},
    {"tev_5",    "http://tempuri.org/tev_5.xsd"},
    {"tev_6",    "http://tempuri.org/tev_6.xsd"},
    {"tev_7",    "http://tempuri.org/tev_7.xsd"},
    {"tev_8",    "http://tempuri.org/tev_8.xsd"},
    {"wsnt",     "http://docs.oasis-open.org/wsn/b-2"},
    {"timg",     "http://www.onvif.org/ver20/imaging/wsdl"},
    {"timg10",   "http://www.onvif.org/ver10/imaging/wsdl"},
    {"tptz",     "http://www.onvif.org/ver20/ptz/wsdl"},
    {"tptz10",   "http://www.onvif.org/ver10/ptz/wsdl"},
    {"trt",      "http://www.onvif.org/ver10/media/wsdl"},
    {"wsse",     "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd"},
    {"ter",      "http://www.onvif.org/ver10/error"}
};
size_t onvif_xmlns_info_len = sizeof(onvif_xmlns_info) / sizeof(xmlns_info_t);

device_mngmt::device_mngmt()
{
	this->xaddrs_port = 80;
	this->status = STATUS_NOT_INIT;
	this->image_config_support = True;
}
device_mngmt::~device_mngmt()
{

	
}

int device_mngmt::set_xaddrs()
{
	if(this->XAddrs.empty())
		return 1;
	const char *p1 = NULL;
	const char *p2 = NULL;
	if ( string::npos == XAddrs.find(string("http://") )  )
		lsprint("NO SUCH DEVICE \n ");
	p1 = XAddrs.c_str() + 7;

	if(NULL == (p2 = strchr(p1, ':')))
    {
        /* not found http port, use 80 default */
        xaddrs_port = 80;

		if(NULL == (p2 = strchr(p1, '/')) )
			this->xaddrs_uri.assign("/");
		else
			this->xaddrs_uri.assign(p2,strlen(p2));
	}
	else
	{
		ls_str_to_int(p2+1,&xaddrs_port);

		if(NULL == (p1 = strchr(p2, '/')) )
			this->xaddrs_uri.assign("/");
		else
			this->xaddrs_uri.assign(p1,strlen(p1));
	}
	
	//cout<<this->xaddrs_port<<endl<<this->xaddrs_uri<<endl;
	return 0;
}
int32_t device_mngmt::check_reponse_error(ls_http_response* http_response)
{
	if(NULL == http_response || NULL == http_response->respone_content )
		 return STATUS_DEVICE_NOT_RESPONSE;
	if(NULL != strstr(http_response->respone_content, "NotAuthorized")) 
		return STATUS_NOTAUTHORIZED;
	if( http_response->status != 200 )  
		return STATUS_DEVICE_RETURN_ERROR;
    return STATUS_PARSE_RESPONSE_ERROR;
}
void device_mngmt::set_username_password(const char *username,const char* password)
{
	this->username = std::string(username);
	this->password = std::string(password);
}
uint32_t device_mngmt::register_onvif_namespaces(xmlXPathContextPtr xpath_context)
{
    size_t i = 0;
    
    if(NULL == xpath_context) return 1;

    for(i = 0; i < onvif_xmlns_info_len; i++)
    {
        if(0 != xmlXPathRegisterNs(xpath_context, BAD_CAST(onvif_xmlns_info[i].prefix),
                                   BAD_CAST(onvif_xmlns_info[i].ns_uri)))
            return 1;
    }

    return 0;
}

//extern std::string base64_decode(std::string const& s);

/*
static const std::string base64_chars = 
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789+/";

static inline bool is_base64(unsigned char c) 
{
  return (isalnum(c) || (c == '+') || (c == '/'));
}

static std::string base64_decode(std::string const& encoded_string) 
{
  int in_len = encoded_string.size();
  int i = 0, j = 0, in_ = 0;
  unsigned char char_array_4[4], char_array_3[3];
  std::string ret;

  while (in_len-- && ( encoded_string[in_] != '=') && is_base64(encoded_string[in_])) 
	{
    char_array_4[i++] = encoded_string[in_]; in_++;
    if (i ==4) {
      for (i = 0; i <4; i++)
        char_array_4[i] = base64_chars.find(char_array_4[i]);

      char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
      char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
      char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

      for (i = 0; (i < 3); i++)
        ret += char_array_3[i];
      i = 0;
    }
  }

  if (i) 
	{
    for (j = i; j <4; j++)
      char_array_4[j] = 0;

    for (j = 0; j <4; j++)
      char_array_4[j] = base64_chars.find(char_array_4[j]);

    char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
    char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
    char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

    for (j = 0; (j < i - 1); j++) 
			ret += char_array_3[j];
  }

  return ret;
}
*/

int device_mngmt::build_wss_header(char *buf, size_t buf_len)
{
	struct USHAContext sha;
	struct timeval     tv;
	struct tm          tm;
	//long               nonce;
	char               *nonce_b64;
	//size_t           nonce_b64_len;
	char               created[64];
	size_t             created_len;
	uint8_t            password_sha[20];
	char               *password_sha_b64;
	size_t             password_sha_b64_len;
	//char  password[] = "system";
	//char username[] = "system";
	//lsprint("%s:%s",this->username.c_str(),this->password.c_str());
	if(NULL == buf || buf_len < 1024)
		return 1;
	if( this->username.empty() ) 
		return 1;
	
	/* build "nonce" */
	//nonce = random();
	
	//nonce_b64 = base64Encode((char const*)&nonce, sizeof(long));
	char noncestring[32] = {0};
	sprintf(noncestring, "%022d", random());
	nonce_b64 = base64Encode((char const*)noncestring, strlen(noncestring));
	//printf("noncestring=%s,nonce_b64=%s\n",noncestring,nonce_b64);
	
	//"Hezrg4r67ki7bcykB6PpDAABAAAAAA==";
	//"MDAwMDAwMDAwMDAwMDQ4MDE4OTI5Nw=="
	//noncestring=000000000000000706675407,nonce_b64=MDAwMDAwMDAwMDAwMDAwNzA2Njc1NDA3
	//dh1=MDAwMDAwMDAwMDAwMDAwNzA2Njc1NDA3,dh2=000000000000000706675407
	//std::string dh1 = "MDAwMDAwMDAwMDAwMDAwNzA2Njc1NDA3";
	//std::string dh2 = base64_decode(dh1);
	//printf("dh1=%s,dh2=%s\n",dh1.c_str(),dh2.c_str());
	
	//cout<< base64Decode(nonce_b64,strlen(nonce_b64),unsigned resultSize);
	//cout<<nonce_b64[0]<<endl;
	//nonce_b64_len = strlen((char const*)nonce_b64);
	//cout<<nonce_b64_len<<endl;
	//nonce_b64[nonce_b64_len] = 0;
	
	/*char in[] = "Ly7xFaSa+Sfg4W+djvCjuA==";
	cout<<in<<endl;
	unsigned char *out;
	unsigned resultSize;
	out = base64Decode(in, sizeof(in),	resultSize);
	cout<<"out:"<<out<<endl;
	cout<<"size:"<<resultSize<<endl;
	nonce_b64 = base64Encode((char const*)out, resultSize);
	cout<<nonce_b64<<endl;
	delete []out;
	*/
	
	/* build "created" */
	gettimeofday(&tv, NULL);
	localtime_r((time_t*)(&(tv.tv_sec)), &tm);
	//created_len = snprintf(created, sizeof(created), "%04d-%02d-%02dT%02d:%02d:%02d.%3ldZ",
	//	tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, tv.tv_usec);
	created_len = snprintf(created, sizeof(created), "%04d-%02d-%02dT%02d:%02d:%02d.%03ldZ",
		tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, tv.tv_usec/1000);
	
	/* build "password" */
	USHAReset(&sha, SHA1);
	//USHAInput(&sha, (uint8_t *)&nonce, sizeof(long));
	USHAInput(&sha, (uint8_t *)noncestring, strlen(noncestring));
	USHAInput(&sha, (uint8_t *)created, created_len);
	USHAInput(&sha, (uint8_t *)this->password.c_str(), this->password.length());
	USHAResult(&sha, password_sha);
	//ls_base64_encode(password_sha, sizeof(password_sha), password_sha_b64, sizeof(password_sha_b64), &password_sha_b64_len);
	password_sha_b64 =  base64Encode((char const*)&password_sha, sizeof(password_sha));
	password_sha_b64_len = strlen((char const*)password_sha_b64);
	password_sha_b64[password_sha_b64_len] = 0;
	
	/* build wss header */
	//csp modify
	snprintf(buf, buf_len,
		"<s:Header>"
		"<Security s:mustUnderstand=\"1\" xmlns=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd\">"
		"<UsernameToken>"
		"<Username>%s</Username>"
		"<Password Type=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-username-token-profile-1.0#PasswordDigest\">%s</Password>"
		"<Nonce EncodingType=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-soap-message-security-1.0#Base64Binary\">%s</Nonce>"
		"<Created xmlns=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd\">%s</Created>"
		"</UsernameToken>"
		"</Security>"
		"</s:Header>", this->username.c_str(), (char *)password_sha_b64, (char *)nonce_b64, created);
	delete []nonce_b64;
	delete []password_sha_b64;
	return 0;
}

int32_t device_mngmt::handle_mngmt_capabilities()
{
	char                wss_header[2048] = {'\0'};
	char				buf[4096]        = {'\0'};
	
	this->status = STATUS_ERROR;
	build_wss_header(wss_header, sizeof(wss_header));
	size_t len = 0;
	
	//csp modify
	len = sprintf(buf,		   "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
                               "<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\""
                               " xmlns:tds=\"http://www.onvif.org/ver10/device/wsdl\""
                               " xmlns:tt=\"http://www.onvif.org/ver10/schema\">"
                               "%s"
                               "<s:Body>"
                               "<tds:GetCapabilities>"
							   "<tds:Category>All</tds:Category>"
							   "</tds:GetCapabilities>"
                               "</s:Body>"
                               "</s:Envelope>", wss_header);
	string content_type = string("application/soap+xml; charset=utf-8; action=\"http://www.onvif.org/ver10/device/wsdl/GetCapabilities\"");
	
	//printf("handle_mngmt_capabilities:wss_header=%s\n",wss_header);
	
	if(this->xaddrs_uri.empty())
		set_xaddrs();
	
	ls_http_request request(HTTP_POST,1,1,this->xaddrs_port);
	request.setUri(this->xaddrs_uri.c_str(),this->xaddrs_uri.length());
	request.setHost(this->IP.c_str(),this->IP.length());
	request.setContentType(content_type.c_str(),content_type.length());
	request.setContent(buf,len);
	request.setHandler((HandlerProc *)&handle_mngmt_capabilities_help,(void *)this);
	//printf("handle_mngmt_capabilities-1\n");
	//struct timeval tv1;
//gettimeofday(&tv1,NULL);
//printf("%s 1 tv1:%u.%u\n", __func__, tv1.tv_sec, tv1.tv_usec);
	if( -1 == request.sendTo() )
	{
		//printf("handle_mngmt_capabilities-2\n");
		return STATUS_CONNECT_DEVICE_TIMEOUT;
	}
	//printf("handle_mngmt_capabilities-3\n");
	//gettimeofday(&tv1,NULL);
//printf("%s 2 tv1:%u.%u\n", __func__, tv1.tv_sec, tv1.tv_usec);
	request.recvFrom();
	//gettimeofday(&tv1,NULL);
//printf("%s 3 tv1:%u.%u\n", __func__, tv1.tv_sec, tv1.tv_usec);
	//printf("handle_mngmt_capabilities-4\n");
	return this->status;
}

xmlNodePtr device_mngmt::seek_child_node_by_name(xmlNodePtr node, const xmlChar *name)
{
    if(NULL == node || NULL == name) return NULL;

    for(node = node->children; NULL != node; node = node->next)
    {
        if(XML_ELEMENT_NODE == node->type && node->name && 0 == xmlStrcmp(node->name, name))
        {
            return node;
        }
    }

    return NULL;
}
void device_mngmt::handle_mngmt_capabilities_help(ls_http_response* http_response,void *client_data)
{
    device_mngmt *me = static_cast<device_mngmt *>(client_data);
    me->handle_message_capabilities(http_response);
	
}
void device_mngmt::handle_message_capabilities(ls_http_response* http_response)
{
	xmlDocPtr           doc = NULL;
    xmlNodePtr          node = NULL, node2 = NULL, node3 = NULL;
    xmlXPathContextPtr  xpath_context = NULL;
    xmlXPathObjectPtr   xpath_object = NULL;
    xmlChar            *p = NULL;
	char                p_buf[1024]={'\0'};
    char               *p_tmp = NULL;
    int                 i;
	
	//lsprint("handle_message_capabilities:%s\n",http_response->respone_content);
	
    /* check error */
    this->status = check_reponse_error(http_response);
    
    if( STATUS_PARSE_RESPONSE_ERROR != this->status )
		return;
    /* parse XML */
	do
	{
		if(NULL == (doc = xmlParseMemory(http_response->respone_content, http_response->respone_content_len))) break;
		if(NULL == (xpath_context = xmlXPathNewContext(doc))) break;
		if(0 != register_onvif_namespaces(xpath_context)) break;
		if(NULL == (xpath_object = xmlXPathEvalExpression(BAD_CAST"/SOAP-ENV:Envelope/SOAP-ENV:Body/tds:GetCapabilitiesResponse/tds:Capabilities", xpath_context))) break;
		if(xmlXPathNodeSetIsEmpty(xpath_object->nodesetval)) break;

		/* get the first "Capabilities" */
		for(i = 0; i < xpath_object->nodesetval->nodeNr; i++)
		{
			if(XML_ELEMENT_NODE == xpath_object->nodesetval->nodeTab[i]->type)
			{
				node = xpath_object->nodesetval->nodeTab[i];            
				break;
			}
		}
		if(NULL == node) break;

		/* get Media Xaddr */
		if(NULL == (node2 = seek_child_node_by_name(node, BAD_CAST"Media"))) break;
		if(NULL == (node3 = seek_child_node_by_name(node2, BAD_CAST"XAddr"))) break;
		if(NULL == (p = xmlNodeGetContent(node3))) break;
		strncpy(p_buf, (const char *)p, sizeof(p_buf));
		xmlFree(p);
		//lsprint("Receiver_onvif, receive media_xaddrs: %s.\n", p_buf);
		p_tmp = p_buf;
		if(0 != strncmp(p_tmp, "http://", 7)) break;
		p_tmp += 7;
		p_tmp = strchr(p_tmp, '/');
		string str;
		if(NULL == p_tmp)
			xaddrs_uri_media.assign("/");
		else
		    xaddrs_uri_media.assign(p_tmp,strlen(p_tmp));
		
		
		/* get PTZ Xaddr */
		if(NULL == (node2 = seek_child_node_by_name(node, BAD_CAST"PTZ")))
		{
			xaddrs_uri_ptz = xaddrs_uri ;
		}
		else
		{
			if(NULL == (node3 = seek_child_node_by_name(node2, BAD_CAST"XAddr"))) break;
			if(NULL == (p = xmlNodeGetContent(node3))) break;
			strncpy(p_buf, (const char *)p, sizeof(p_buf));
			xmlFree(p);
			p = NULL;
			//lsprint("Receiver_onvif, receive ptz_xaddrs: %s.\n", p_buf);
			p_tmp = p_buf;
			if(0 != strncmp(p_tmp, "http://", 7)) break;
			p_tmp += 7;
			p_tmp = strchr(p_tmp, '/');
			if(NULL == p_tmp)
				xaddrs_uri_ptz.assign("/");
			else
				xaddrs_uri_ptz.assign(p_tmp,strlen(p_tmp));
			
		}
		
		/* get PTZ Xaddr */
		if(NULL == (node2 = seek_child_node_by_name(node, BAD_CAST"PTZ")))
		{
			xaddrs_uri_ptz = xaddrs_uri ;
		}
		else
		{
			if(NULL == (node3 = seek_child_node_by_name(node2, BAD_CAST"XAddr"))) break;
			if(NULL == (p = xmlNodeGetContent(node3))) break;
			strncpy(p_buf, (const char *)p, sizeof(p_buf));
			xmlFree(p);
			//lsprint("Receiver_onvif, receive ptz_xaddrs: %s.\n", p_buf);
			p_tmp = p_buf;
			if(0 != strncmp(p_tmp, "http://", 7)) break;
			p_tmp += 7;
			p_tmp = strchr(p_tmp, '/');
			if(NULL == p_tmp)
				xaddrs_uri_ptz.assign("/");
			else
				xaddrs_uri_ptz.assign(p_tmp,strlen(p_tmp));
			
		}
		/* get Imaging Xaddr */
		if(NULL == (node2 = seek_child_node_by_name(node, BAD_CAST"Imaging")))
		{
			//this->xaddrs_uri_image = xaddrs_uri ;
		}
		else
		{
			if(NULL == (node3 = seek_child_node_by_name(node2, BAD_CAST"XAddr"))) break;
			if(NULL == (p = xmlNodeGetContent(node3))) break;
			strncpy(p_buf, (const char *)p, sizeof(p_buf));
			xmlFree(p);
			//lsprint("Receiver_onvif, receive ptz_xaddrs: %s.\n", p_buf);
			p_tmp = p_buf;
			if(0 != strncmp(p_tmp, "http://", 7)) break;
			p_tmp += 7;
			p_tmp = strchr(p_tmp, '/');
			if(NULL == p_tmp)
				this->xaddrs_uri_image.assign("/");
			else
				this->xaddrs_uri_image.assign(p_tmp,strlen(p_tmp));
			
		}
		/* get Events Xaddr */
		if(NULL == (node2 = seek_child_node_by_name(node, BAD_CAST"Events")))
		{
			//this->xaddrs_uri_event = xaddrs_uri ;
		}
		else
		{
			if(NULL == (node3 = seek_child_node_by_name(node2, BAD_CAST"XAddr"))) break;
			if(NULL == (p = xmlNodeGetContent(node3))) break;
			strncpy(p_buf, (const char *)p, sizeof(p_buf));
			xmlFree(p);
			//lsprint("Receiver_onvif, receive ptz_xaddrs: %s.\n", p_buf);
			p_tmp = p_buf;
			if(0 != strncmp(p_tmp, "http://", 7)) break;
			p_tmp += 7;
			p_tmp = strchr(p_tmp, '/');
			if(NULL == p_tmp)
				this->xaddrs_uri_event.assign("/");
			else
				this->xaddrs_uri_event.assign(p_tmp,strlen(p_tmp));
			
		}
		this->status = STATUS_ERRNO;
	}while(0);

   // lsprint("Receiver_onvif, media_xaddrs: %s, ptz_xaddrs: %s.\n",
	//	this->xaddrs_uri_media.c_str(), this->xaddrs_uri_ptz.c_str());
	if( this->xaddrs_uri_media.empty())
		this->xaddrs_uri_media = xaddrs_uri;
	if( this->xaddrs_uri_media.empty())
		this->xaddrs_uri_ptz = xaddrs_uri;
	if( this->xaddrs_uri_image.empty())
		this->image_config_support = False;
	else
		this->image_config_support = True;
	//if( this->xaddrs_uri_event.empty())
		//this->xaddrs_uri_event = xaddrs_uri;
	//cout<<"media:"<<this->xaddrs_uri_media<<endl;
	//cout<<"ptz:"<<this->xaddrs_uri_ptz<<endl;
	//cout<<"image:"<<this->xaddrs_uri_image<<endl;
	//cout<<"event:"<<this->xaddrs_uri_event<<endl;
    if(NULL != xpath_object) xmlXPathFreeObject(xpath_object);
    if(NULL != xpath_context) xmlXPathFreeContext(xpath_context);
    if(NULL != doc) xmlFreeDoc(doc);
    xmlCleanupParser();
    return;

    /* Use xaddrs for all */
    //strncpy(this->xaddrs_uri_media, this->xaddrs_uri, sizeof(this->xaddrs_uri_media));
    //strncpy(this->xaddrs_uri_ptz, this->xaddrs_uri, sizeof(this->xaddrs_uri_ptz));
   // lsprint("Receiver_onvif, use xaddrs for all.\n");

}

int32_t device_mngmt::handle_mngmt_get_device_infomation(device_infomation *info)
{
	char                wss_header[2048] = {'\0'};
	char				buf[4096]        = {'\0'};
	
	this->status = STATUS_ERROR;
	build_wss_header(wss_header, sizeof(wss_header));
	size_t					len;
	len = sprintf(buf,		   "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
                               "<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\""
                               " xmlns:tds=\"http://www.onvif.org/ver10/device/wsdl\""
                               " xmlns:tt=\"http://www.onvif.org/ver10/schema\">%s"
                               "<s:Body>"
                               "<tds:GetDeviceInformation />"
                               "</s:Body>"
                               "</s:Envelope>", wss_header);
	string content_type = string("application/soap+xml; charset=utf-8 ");

	if(this->xaddrs_uri.empty())
		set_xaddrs();

	ls_http_request request(HTTP_POST,1,1,this->xaddrs_port);
	request.setUri(this->xaddrs_uri.c_str(),this->xaddrs_uri.length());
	request.setHost(this->IP.c_str(),this->IP.length());
	request.setContentType(content_type.c_str(),content_type.length());
	request.setContent(buf,len);
	request.setHandler((HandlerProc *)&handle_mngmt_get_device_infomation_help,(void *)this,(void *)info);
	if( -1 == request.sendTo() )
		return STATUS_CONNECT_DEVICE_TIMEOUT;
	request.recvFrom();
	return this->status;
}
void device_mngmt::handle_mngmt_get_device_infomation_help(ls_http_response* http_response,void *client_data,void *info)
{
	device_mngmt *me = static_cast<device_mngmt *>(client_data);
    me->handle_message_get_device_infomation(http_response,(device_infomation *)info);
}
void device_mngmt::handle_message_get_device_infomation(ls_http_response* http_response,device_infomation *info)
{
	xmlDocPtr           doc = NULL;
    xmlNodePtr          node = NULL, node2 = NULL;
    xmlXPathContextPtr  xpath_context = NULL;
    xmlXPathObjectPtr   xpath_object = NULL;
    xmlChar            *p = NULL;
    int                 i;

  //  lsprint("%s",http_response->response);
	/* check error */
	this->status = check_reponse_error(http_response);
    if( STATUS_PARSE_RESPONSE_ERROR != this->status )
		return;

	/* parse XML */
	do
	{
		if(NULL == (doc = xmlParseMemory(http_response->respone_content, http_response->respone_content_len))) break;
		if(NULL == (xpath_context = xmlXPathNewContext(doc))) break;
		if(0 != register_onvif_namespaces(xpath_context)) break;
		if(NULL == (xpath_object = xmlXPathEvalExpression(BAD_CAST"/SOAP-ENV:Envelope/SOAP-ENV:Body/tds:GetDeviceInformationResponse", xpath_context))) break;
		if(xmlXPathNodeSetIsEmpty(xpath_object->nodesetval)) break;

		/* get the first "GetDeviceInformationResponse" */
		for(i = 0; i < xpath_object->nodesetval->nodeNr; i++)
		{
			if(XML_ELEMENT_NODE == xpath_object->nodesetval->nodeTab[i]->type)
			{
				node = xpath_object->nodesetval->nodeTab[i];            
				break;
			}
		}
		if(NULL == node) break;

		if(NULL == (node2 = seek_child_node_by_name(node, BAD_CAST"Manufacturer"))) continue;
		if(NULL == (p = xmlNodeGetContent(node2))) break;
		strncpy(info->manufacturer, (const char *)p, sizeof(info->manufacturer));
		xmlFree(p);

		if(NULL == (node2 = seek_child_node_by_name(node, BAD_CAST"Model"))) continue;
		if(NULL == (p = xmlNodeGetContent(node2))) break;
		strncpy(info->model, (const char *)p, sizeof(info->model));
		xmlFree(p);
		
		if(NULL == (node2 = seek_child_node_by_name(node, BAD_CAST"FirmwareVersion"))) continue;
		if(NULL == (p = xmlNodeGetContent(node2))) break;
		strncpy(info->firmware_version, (const char *)p, sizeof(info->firmware_version));
		xmlFree(p);

		if(NULL == (node2 = seek_child_node_by_name(node, BAD_CAST"SerialNumber"))) continue;
		if(NULL == (p = xmlNodeGetContent(node2))) break;
		strncpy(info->serial_number, (const char *)p, sizeof(info->serial_number));
		xmlFree(p);

		if(NULL == (node2 = seek_child_node_by_name(node, BAD_CAST"HardwareId"))) continue;
		if(NULL == (p = xmlNodeGetContent(node2))) break;
		strncpy(info->hardwareId, (const char *)p, sizeof(info->hardwareId));
		xmlFree(p);

		this->status = STATUS_ERRNO;
		
		
	}while(0);
 
   
    if(NULL != xpath_object) xmlXPathFreeObject(xpath_object);
    if(NULL != xpath_context) xmlXPathFreeContext(xpath_context);
    if(NULL != doc) xmlFreeDoc(doc);
    xmlCleanupParser();
    return;
}



int32_t device_mngmt::handle_mngmt_set_dns(const char *dns,uint32_t dns_len)
{
	char                wss_header[2048] = {'\0'};
	char				buf[4096]        = {'\0'};
	build_wss_header(wss_header, sizeof(wss_header));
	size_t					len;
	this->status = STATUS_ERROR;
	if( dns != NULL && dns_len != 0 )
	{
		char				*this_dns = new char[dns_len +1 ];
		strncpy(this_dns,dns,dns_len);
		this_dns[dns_len] = '\0';
		len = sprintf(buf,		   "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
			"<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\""
			" xmlns:tds=\"http://www.onvif.org/ver10/device/wsdl\""
			" xmlns:tt=\"http://www.onvif.org/ver10/schema\">%s"
			"<s:Body>"
			"<tds:SetDNS>"
			"<tds:DNSManual>"
			"<tt:Type>IPv4</tt:Type>"
			"<tt:IPv4Address>%s</tt:IPv4Address>"
			"</tds:DNSManual>"
			"</tds:SetDNS>"
			"</s:Body>"
			"</s:Envelope>", wss_header,this_dns);
		delete []this_dns;
	}
	else
	{
		len = sprintf(buf,		   "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
			"<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\""
			" xmlns:tds=\"http://www.onvif.org/ver10/device/wsdl\""
			" xmlns:tt=\"http://www.onvif.org/ver10/schema\">%s"
			"<s:Body>"
			"<tds:SetDNS>"
            "<tds:FromDHCP>true</tds:FromDHCP>"
			"</tds:SetDNS>"
			"</s:Body>"
			"</s:Envelope>", wss_header);
	}
	string content_type = string("application/soap+xml; charset=utf-8");
	if(this->xaddrs_uri.empty())
		set_xaddrs();

	ls_http_request request(HTTP_POST,1,1,this->xaddrs_port);
	request.setUri(this->xaddrs_uri.c_str(),this->xaddrs_uri.length());
	request.setHost(this->IP.c_str(),this->IP.length());
	request.setContentType(content_type.c_str(),content_type.length());
	request.setContent(buf,len);
	request.setHandler((HandlerProc *)&handle_mngmt_set_dns_help,(void *)this);
	if( -1 == request.sendTo() )
			return STATUS_CONNECT_DEVICE_TIMEOUT;

	
	request.recvFrom();
	return this->status;
}
void device_mngmt::handle_mngmt_set_dns_help(ls_http_response* http_response,void *client_data)
{
	device_mngmt *me = static_cast<device_mngmt *>(client_data);
    me->handle_message_set_dns(http_response);
}
void device_mngmt::handle_message_set_dns(ls_http_response* http_response)
{
	//lsprint("%s",http_response->response);//csp modify
	/* check error */
	this->status = check_reponse_error(http_response);
    if( STATUS_PARSE_RESPONSE_ERROR != this->status )
		return;
	this->status = STATUS_ERRNO;
}

int32_t device_mngmt::handle_mngmt_get_dns(device_dns *dns)
{
	char                wss_header[2048] = {'\0'};
	char				buf[4096]        = {'\0'};
	this->status = STATUS_ERROR;
	build_wss_header(wss_header, sizeof(wss_header));
	size_t					len;
	len = sprintf(buf,		   "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
                               "<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\""
                               " xmlns:tds=\"http://www.onvif.org/ver10/device/wsdl\""
                               " xmlns:tt=\"http://www.onvif.org/ver10/schema\">%s"
                               "<s:Body>"
                                "<tds:GetDNS />"
                                "</s:Body>"
                                "</s:Envelope>", wss_header);
	string content_type = string("application/soap+xml; charset=utf-8");
	
	if(this->xaddrs_uri.empty())
		set_xaddrs();

	ls_http_request request(HTTP_POST,1,1,this->xaddrs_port);
	request.setUri(this->xaddrs_uri.c_str(),this->xaddrs_uri.length());
	request.setHost(this->IP.c_str(),this->IP.length());
	request.setContentType(content_type.c_str(),content_type.length());
	request.setContent(buf,len);
	request.setHandler((HandlerProc *)&handle_mngmt_get_dns_help,(void *)this,(void *)dns);
	if( -1 == request.sendTo() )
			return STATUS_CONNECT_DEVICE_TIMEOUT;
	request.recvFrom();
	return this->status;
}
void device_mngmt::handle_mngmt_get_dns_help(ls_http_response* http_response,void *client_data,void *dns)
{
	device_mngmt *me = static_cast<device_mngmt *>(client_data);
    me->handle_message_get_dns(http_response,(device_dns *)dns);
}
void device_mngmt::handle_message_get_dns(ls_http_response* http_response,device_dns *dns)
{
	xmlDocPtr           doc = NULL;
    xmlNodePtr          node = NULL, node2 = NULL , node3 = NULL;
    xmlXPathContextPtr  xpath_context = NULL;
    xmlXPathObjectPtr   xpath_object = NULL;
    xmlChar            *p = NULL;
    int                 i;
	
	//csp modify
	//lsprint("%s",http_response->response);
	
	/* check error */
	this->status = check_reponse_error(http_response);
    if( STATUS_PARSE_RESPONSE_ERROR != this->status )
		return;

	/* parse XML */
	do
	{
		if(NULL == (doc = xmlParseMemory(http_response->respone_content, http_response->respone_content_len))) break;
		if(NULL == (xpath_context = xmlXPathNewContext(doc))) break;
		if(0 != register_onvif_namespaces(xpath_context)) break;
		if(NULL == (xpath_object = xmlXPathEvalExpression(BAD_CAST"/SOAP-ENV:Envelope/SOAP-ENV:Body/tds:GetDNSResponse/tds:DNSInformation", xpath_context))) break;
		if(xmlXPathNodeSetIsEmpty(xpath_object->nodesetval)) break;

		/* get the first "DNSInformation" */
		for(i = 0; i < xpath_object->nodesetval->nodeNr; i++)
		{
			if(XML_ELEMENT_NODE == xpath_object->nodesetval->nodeTab[i]->type)
			{
				node = xpath_object->nodesetval->nodeTab[i];   
				break;
			}
		}
		if(NULL == node) break;
		//memset(dns,0,sizeof(dns));
		node2 = seek_child_node_by_name(node,BAD_CAST"FromDHCP");
		if(NULL == (p = xmlNodeGetContent(node2))) break;
		if( *(const char *)p == 'T' ||  *(const char *)p == 't'  )
			dns->from_DHCP = True;
		else
			dns->from_DHCP = False;
		xmlFree(p);
		p = NULL;
		if( dns->from_DHCP )
		{
			node2 = seek_child_node_by_name(node,BAD_CAST"DNSFromDHCP");
			if(NULL == node2 )break;
		}
		else
		{
			node2 = seek_child_node_by_name(node,BAD_CAST"DNSManual");
			if(NULL == node2 ) break;
		}

		node3 = seek_child_node_by_name(node2,BAD_CAST"Type");
		if(NULL == (p = xmlNodeGetContent(node3))) break;
		//printf("p:%s\n",p);
		strcpy(dns->type,(const char *)p );
		xmlFree(p);
		p = NULL;
		node3 = seek_child_node_by_name(node2,BAD_CAST"IPv4Address");
		if(NULL == (p = xmlNodeGetContent(node3))) break;
		//printf("p:%s\n",p);
		strcpy(dns->dns,(const char *)p );
		xmlFree(p);
		p = NULL;
	
		this->status = STATUS_ERRNO;

	}while(0);
 
   
    if(NULL != xpath_object) xmlXPathFreeObject(xpath_object);
    if(NULL != xpath_context) xmlXPathFreeContext(xpath_context);
    if(NULL != doc) xmlFreeDoc(doc);
	
    xmlCleanupParser();
    return;
}
int32_t device_mngmt::handle_mngmt_set_ntp(const char *ntp,uint32_t ntp_len)
{
	char                wss_header[2048] = {'\0'};
	char				buf[4096]        = {'\0'};
	size_t					len;
	this->status = STATUS_ERROR;
	build_wss_header(wss_header, sizeof(wss_header));
	if( ntp != NULL && ntp_len != 0 )
	{
		char				*this_ntp = new char[ntp_len +1 ];
		strncpy(this_ntp,ntp,ntp_len);
		this_ntp[ntp_len] = '\0';
		len = sprintf(buf,		   "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
			"<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\""
			" xmlns:tds=\"http://www.onvif.org/ver10/device/wsdl\""
			" xmlns:tt=\"http://www.onvif.org/ver10/schema\">%s"
			"<s:Body>"
			"<tds:SetNTP>"
			"<tds:FromDHCP>false</tds:FromDHCP>"
			"<tds:NTPManual>"
			"<tt:Type>IPv4</tt:Type>"
			"<tt:IPv4Address>%s</tt:IPv4Address>"
			"</tds:NTPManual>"
			"</tds:SetNTP>"
			"</s:Body>"
			"</s:Envelope>", wss_header,this_ntp);
		delete []this_ntp;
	}
	else
	{
		len = sprintf(buf,		   "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
			"<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\""
			" xmlns:tds=\"http://www.onvif.org/ver10/device/wsdl\""
			" xmlns:tt=\"http://www.onvif.org/ver10/schema\">%s"
			"<s:Body>"
			"<tds:SetNTP>"
			"<tds:FromDHCP>true</tds:FromDHCP>"
			"</tds:SetNTP>"
			"</s:Body>"
			"</s:Envelope>", wss_header);
	}
	string content_type = string("application/soap+xml; charset=utf-8");
	
	if(this->xaddrs_uri.empty())
		set_xaddrs();

	ls_http_request request(HTTP_POST,1,1,this->xaddrs_port);
	request.setUri(this->xaddrs_uri.c_str(),this->xaddrs_uri.length());
	request.setHost(this->IP.c_str(),this->IP.length());
	request.setContentType(content_type.c_str(),content_type.length());
	request.setContent(buf,len);
	request.setHandler((HandlerProc *)&handle_mngmt_set_ntp_help,(void *)this);
	if( -1 == request.sendTo() )
			return STATUS_CONNECT_DEVICE_TIMEOUT;

	
	request.recvFrom();
	return this->status;
}
void device_mngmt::handle_mngmt_set_ntp_help(ls_http_response* http_response,void *client_data)
{
	device_mngmt *me = static_cast<device_mngmt *>(client_data);
    me->handle_message_set_ntp(http_response);
}
void device_mngmt::handle_message_set_ntp(ls_http_response* http_response)
{
	lsprint("%s",http_response->response);
	/* check error */
	this->status = check_reponse_error(http_response);
    if( STATUS_PARSE_RESPONSE_ERROR != this->status )
		return;
	this->status = STATUS_ERRNO;
}
int32_t device_mngmt::handle_mngmt_get_ntp()
{
	char                wss_header[2048] = {'\0'};
	char				buf[4096]        = {'\0'};

	this->status = STATUS_ERROR;
	build_wss_header(wss_header, sizeof(wss_header));
	size_t					len;
	len = sprintf(buf,		   "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
                               "<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\""
                               " xmlns:tds=\"http://www.onvif.org/ver10/device/wsdl\""
                               " xmlns:tt=\"http://www.onvif.org/ver10/schema\">%s"
                               "<s:Body>"
                                "<tds:GetNTP />"
                                "</s:Body>"
                                "</s:Envelope>", wss_header);
	string content_type = string("application/soap+xml; charset=utf-8");
	
	if(this->xaddrs_uri.empty())
		set_xaddrs();

	ls_http_request request(HTTP_POST,1,1,this->xaddrs_port);
	request.setUri(this->xaddrs_uri.c_str(),this->xaddrs_uri.length());
	request.setHost(this->IP.c_str(),this->IP.length());
	request.setContentType(content_type.c_str(),content_type.length());
	request.setContent(buf,len);
	request.setHandler((HandlerProc *)&handle_mngmt_get_ntp_help,(void *)this);
	if( -1 == request.sendTo() )
			return STATUS_CONNECT_DEVICE_TIMEOUT;
	request.recvFrom();
	return this->status;
}
void device_mngmt::handle_mngmt_get_ntp_help(ls_http_response* http_response,void *client_data)
{
	device_mngmt *me = static_cast<device_mngmt *>(client_data);
    me->handle_message_get_ntp(http_response);
}
void device_mngmt::handle_message_get_ntp(ls_http_response* http_response)
{
	//lsprint("handle_message_get_ntp:%s",http_response->response);
	/* check error */
	this->status = check_reponse_error(http_response);
    if( STATUS_PARSE_RESPONSE_ERROR != this->status )
		return;
	this->status = STATUS_ERRNO;
}

int32_t device_mngmt::handle_mngmt_system_reboot()
{
	char                wss_header[2048] = {'\0'};
	char				buf[4096]        = {'\0'};

	this->status = STATUS_ERROR;
	build_wss_header(wss_header, sizeof(wss_header));
	size_t					len;
	len = sprintf(buf,		   "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
                               "<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\""
                               " xmlns:tds=\"http://www.onvif.org/ver10/device/wsdl\""
                               " xmlns:tt=\"http://www.onvif.org/ver10/schema\">%s"
                               "<s:Body>"
                                "<tds:SystemReboot/>"
                                "</s:Body>"
                                "</s:Envelope>", wss_header);
	string content_type = string("application/soap+xml; charset=utf-8");
	
	if(this->xaddrs_uri.empty())
		set_xaddrs();

	ls_http_request request(HTTP_POST,1,1,this->xaddrs_port);
	request.setUri(this->xaddrs_uri.c_str(),this->xaddrs_uri.length());
	request.setHost(this->IP.c_str(),this->IP.length());
	request.setContentType(content_type.c_str(),content_type.length());
	request.setContent(buf,len);
	//csp modify
	//request.setHandler((HandlerProc *)&handle_mngmt_get_ntp_help,(void *)this);
	request.setHandler((HandlerProc *)&handle_mngmt_system_reboot_help,(void *)this);
	if( -1 == request.sendTo() )
			return STATUS_CONNECT_DEVICE_TIMEOUT;
	request.recvFrom();
	return this->status;
}
void device_mngmt::handle_mngmt_system_reboot_help(ls_http_response* http_response,void *client_data)
{
	device_mngmt *me = static_cast<device_mngmt *>(client_data);
    me->handle_message_system_reboot(http_response);
}
void device_mngmt::handle_message_system_reboot(ls_http_response* http_response)
{
	//lsprint("%s",http_response->response);
	/* check error */
	this->status = check_reponse_error(http_response);
    if( STATUS_PARSE_RESPONSE_ERROR != this->status )
		return;
	this->status = STATUS_ERRNO;
}
int32_t device_mngmt::handle_mngmt_system_factory_default(uint32_t hard_or_soft)
{
	char                wss_header[2048] = {'\0'};
	char				buf[4096]        = {'\0'};

	this->status = STATUS_ERROR;
	build_wss_header(wss_header, sizeof(wss_header));
	size_t					len;
	len = sprintf(buf,		   "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
                               "<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\""
                               " xmlns:tds=\"http://www.onvif.org/ver10/device/wsdl\""
                               " xmlns:tt=\"http://www.onvif.org/ver10/schema\">%s"
                               "<s:Body>"
                               "<tds:SetSystemFactoryDefault>"
								"<tds:FactoryDefault>%s</tds:FactoryDefault>"
								"</tds:SetSystemFactoryDefault>"
                                "</s:Body>"
                                "</s:Envelope>", wss_header,(hard_or_soft == HARD_FACTORY_DEFAULT ? "Hard":"Soft") );
	string content_type = string("application/soap+xml; charset=utf-8");
	
	if(this->xaddrs_uri.empty())
		set_xaddrs();

	ls_http_request request(HTTP_POST,1,1,this->xaddrs_port);
	request.setUri(this->xaddrs_uri.c_str(),this->xaddrs_uri.length());
	request.setHost(this->IP.c_str(),this->IP.length());
	request.setContentType(content_type.c_str(),content_type.length());
	request.setContent(buf,len);
	request.setHandler((HandlerProc *)&handle_mngmt_get_ntp_help,(void *)this);
	if( -1 == request.sendTo() )
			return STATUS_CONNECT_DEVICE_TIMEOUT;
	request.recvFrom();
	return this->status;
}
void device_mngmt::handle_mngmt_system_factory_default_help(ls_http_response* http_response,void *client_data)
{
	device_mngmt *me = static_cast<device_mngmt *>(client_data);
    me->handle_message_system_factory_default(http_response);
}
void device_mngmt::handle_message_system_factory_default(ls_http_response* http_response)
{
	lsprint("%s",http_response->response);
	/* check error */
	this->status = check_reponse_error(http_response);
    if( STATUS_PARSE_RESPONSE_ERROR != this->status )
		return;
	this->status = STATUS_ERRNO;
}


int32_t device_mngmt::handle_mngmt_set_ip_prefix(const char *ip,uint32_t ip_len,uint32_t prefix_length)
{
	char                wss_header[2048] = {'\0'};
	char				buf[4096]        = {'\0'};
	this->status = STATUS_ERROR;
	device_ip dummy;  //dummy
	if( this->internet_token.empty() )
	{
		if( STATUS_ERRNO != handle_mngmt_get_ip(&dummy) )
			this->internet_token.assign("eth0");
	}
	build_wss_header(wss_header, sizeof(wss_header));
	size_t					len;
	if( ip != NULL && ip_len != 0 )
	{
		char				*this_ip = new char[ip_len + 1];
		strncpy(this_ip,ip,ip_len);
		this_ip[ip_len] = '\0';
		#if 1//csp modify
		len = sprintf(buf,		   "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
			"<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\""
			" xmlns:tds=\"http://www.onvif.org/ver10/device/wsdl\""
			" xmlns:tt=\"http://www.onvif.org/ver10/schema\">%s"
			"<s:Body>"
			"<SetNetworkInterfaces xmlns=\"http://www.onvif.org/ver10/device/wsdl\">"
			"<InterfaceToken>%s</InterfaceToken>"
			"<NetworkInterface>"
			"<Enabled xmlns=\"http://www.onvif.org/ver10/schema\">true</Enabled>"
			"<MTU xmlns=\"http://www.onvif.org/ver10/schema\">1500</MTU>"
			"<IPv4 xmlns=\"http://www.onvif.org/ver10/schema\"><Enabled>true</Enabled>"
			"<Manual><Address>%s</Address><PrefixLength>%u</PrefixLength></Manual><DHCP>false</DHCP>"
			"</IPv4>"
			"</NetworkInterface>"
			"</SetNetworkInterfaces>"
			"</s:Body>"
			"</s:Envelope>", wss_header, internet_token.c_str(), this_ip, prefix_length % 33);//prefix_length must prefix_length >= 0 &&prefix_length <=32
		#else
		len = sprintf(buf,		   "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
			"<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\""
			" xmlns:tds=\"http://www.onvif.org/ver10/device/wsdl\""
			" xmlns:tt=\"http://www.onvif.org/ver10/schema\">%s"
			"<s:Body>"
			"<SetNetworkInterfaces xmlns=\"http://www.onvif.org/ver10/device/wsdl\">"
			"<InterfaceToken>%s</InterfaceToken>"
			"<NetworkInterface>"
			"<Enabled xmlns=\"http://www.onvif.org/ver10/schema\">true</Enabled>"
			"<Link xmlns=\"http://www.onvif.org/ver10/schema\"><AutoNegotiation>true</AutoNegotiation>"
			"<Speed>100</Speed>"
			"<Duplex>Full</Duplex>"
			"</Link><MTU xmlns=\"http://www.onvif.org/ver10/schema\">1500</MTU>"
			"<IPv4 xmlns=\"http://www.onvif.org/ver10/schema\"><Enabled>true</Enabled>"
			"<Manual><Address>%s</Address><PrefixLength>%u</PrefixLength></Manual>"
			"</IPv4>"
			"</NetworkInterface>"
			"</SetNetworkInterfaces>"
			"</s:Body>"
			"</s:Envelope>", wss_header, internet_token.c_str(), this_ip, prefix_length % 33);//prefix_length must prefix_length >= 0 &&prefix_length <=32
		#endif
		delete []this_ip;
	}
	else
	{
		len = sprintf(buf,		   "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
			"<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\""
			" xmlns:tds=\"http://www.onvif.org/ver10/device/wsdl\""
			" xmlns:tt=\"http://www.onvif.org/ver10/schema\">%s"
			"<s:Body>"
			"<tds:SetNetworkInterfaces>"
            "<tds:InterfaceToken>%s</tds:InterfaceToken>"
            "<tds:NetworkInterface>"
            "<tt:Enabled>true</tt:Enabled>"
            "</tds:NetworkInterface>"
            "</tds:SetNetworkInterfaces>"
			"</s:Body>"
			"</s:Envelope>", wss_header, internet_token.c_str());
	}
	string content_type = string("application/soap+xml; charset=utf-8");
	if(this->xaddrs_uri.empty())
		set_xaddrs();
	
	//printf("handle_mngmt_set_ip_prefix xml=%s\n",buf);
	
	ls_http_request request(HTTP_POST,1,1,this->xaddrs_port);
	request.setUri(this->xaddrs_uri.c_str(),this->xaddrs_uri.length());
	request.setHost(this->IP.c_str(),this->IP.length());
	request.setContentType(content_type.c_str(),content_type.length());
	request.setContent(buf,len);
	request.setHandler((HandlerProc *)&handle_mngmt_set_ip_prefix_help,(void *)this);
	if( -1 == request.sendTo() )
			return STATUS_CONNECT_DEVICE_TIMEOUT;
	request.recvFrom();
	return this->status;
}
void device_mngmt::handle_mngmt_set_ip_prefix_help(ls_http_response* http_response,void *client_data)
{
	device_mngmt *me = static_cast<device_mngmt *>(client_data);
    me->handle_message_set_ip_prefix(http_response);
}
void device_mngmt::handle_message_set_ip_prefix(ls_http_response* http_response)
{
	//lsprint("%s",http_response->response);//csp modify
	/* check error */
	this->status = check_reponse_error(http_response);
    if( STATUS_PARSE_RESPONSE_ERROR != this->status )
		return;
	this->status = STATUS_ERRNO;
}
int32_t device_mngmt::handle_mngmt_get_ip(device_ip *ip)
{
	char                wss_header[2048] = {'\0'};
	char				buf[4096]        = {'\0'};
	
	this->status = STATUS_ERROR;
	build_wss_header(wss_header, sizeof(wss_header));
	size_t					len;
	len = sprintf(buf,		   "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
                               "<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\""
                               " xmlns:tds=\"http://www.onvif.org/ver10/device/wsdl\""
                               " xmlns:tt=\"http://www.onvif.org/ver10/schema\">%s"
                               "<s:Body>"
                               "<tds:GetNetworkInterfaces />"
                               "</s:Body>"
                               "</s:Envelope>", wss_header);
	string content_type = string("application/soap+xml; charset=utf-8");
	
	if(this->xaddrs_uri.empty())
		set_xaddrs();
	
	ls_http_request request(HTTP_POST,1,1,this->xaddrs_port);
	request.setUri(this->xaddrs_uri.c_str(),this->xaddrs_uri.length());
	request.setHost(this->IP.c_str(),this->IP.length());
	request.setContentType(content_type.c_str(),content_type.length());
	request.setContent(buf,len);
	request.setHandler((HandlerProc *)&handle_mngmt_get_ip_help,(void *)this,(void *)ip);
	if( -1 == request.sendTo() )
			return STATUS_CONNECT_DEVICE_TIMEOUT;
	request.recvFrom();
	return this->status;
}
void device_mngmt::handle_mngmt_get_ip_help(ls_http_response* http_response,void *client_data,void *ip)
{
	device_mngmt *me = static_cast<device_mngmt *>(client_data);
    me->handle_message_get_ip(http_response,(device_ip *)ip);
}
void device_mngmt::handle_message_get_ip(ls_http_response* http_response,device_ip * ip)
{
	xmlDocPtr           doc = NULL;
    xmlNodePtr          node = NULL, node2 = NULL,node3 = NULL,node4 = NULL,node5 = NULL;
    xmlXPathContextPtr  xpath_context = NULL;
    xmlXPathObjectPtr   xpath_object = NULL;
    xmlChar            *p = NULL;
    int                 i;
	
    //lsprint("handle_message_get_ip:%s",http_response->response);//csp modify
	
	/* check error */
	this->status = check_reponse_error(http_response);
    if(STATUS_PARSE_RESPONSE_ERROR != this->status)
		return;
	
	/* parse XML */
	do
	{
		if(NULL == (doc = xmlParseMemory(http_response->respone_content, http_response->respone_content_len))) break;
		if(NULL == (xpath_context = xmlXPathNewContext(doc))) break;
		if(0 != register_onvif_namespaces(xpath_context)) break;
		if(NULL == (xpath_object = xmlXPathEvalExpression(BAD_CAST"/SOAP-ENV:Envelope/SOAP-ENV:Body/tds:GetNetworkInterfacesResponse/tds:NetworkInterfaces", xpath_context))) break;
		if(xmlXPathNodeSetIsEmpty(xpath_object->nodesetval)) break;
		
		/* get the first "NetworkInterfaces" */
		for(i = 0; i < xpath_object->nodesetval->nodeNr; i++)
		{
			if(XML_ELEMENT_NODE == xpath_object->nodesetval->nodeTab[i]->type)
			{
				node = xpath_object->nodesetval->nodeTab[i];            
				break;
			}
		}
		if(NULL == node) break;
		
		if (xmlHasProp(node,BAD_CAST "token"))
		{
			if(NULL != (p = xmlGetNoNsProp(node, BAD_CAST"token")))
			{
				internet_token.assign( (const char*)p );
				xmlFree(p);
				p = NULL;
			}
            //lsprint("handle_message_get_ip Token:%s\n",internet_token.c_str());
		}
		
		//get hardware address
		node2 = seek_child_node_by_name(node,BAD_CAST"Info");
		if(NULL == node2 ) break;
		node3 = seek_child_node_by_name(node2,BAD_CAST"HwAddress");
		if(NULL == (p = xmlNodeGetContent(node3))) break;
		strcpy(ip->hardware_address,(const char *)p );
		//printf("p:%s\n",p);
		xmlFree(p);
		p = NULL;
		node2 = seek_child_node_by_name(node,BAD_CAST"IPv4");
		if(NULL == node2) break;
		node3 = seek_child_node_by_name(node2,BAD_CAST"Config");
		if(NULL == node3) break;
		node4 = seek_child_node_by_name(node3,BAD_CAST"DHCP");
		if(NULL == (p = xmlNodeGetContent(node4))) break;
		//printf("p:%s\n",p);
		if( *(const char *)p == 'T' ||  *(const char *)p == 't' )
			ip->from_DHCP = True;
		else
			ip->from_DHCP = False;
		xmlFree(p);
		p = NULL;
		
		if( ip->from_DHCP )
		{
			node4 = seek_child_node_by_name(node3,BAD_CAST"FromDHCP");
			if(NULL == node4) break;
		}
		else
		{
			node4 = seek_child_node_by_name(node3,BAD_CAST"Manual");
			if(NULL == node4) break;
		}
		
		node5 = seek_child_node_by_name(node4,BAD_CAST"Address");
		if(NULL == (p = xmlNodeGetContent(node5))) break;
		strcpy(ip->ip,(const char *)p);
		//printf("ip:%s\n",p);
		xmlFree(p);
		p = NULL;
		node5 = seek_child_node_by_name(node4,BAD_CAST"PrefixLength");
		if(NULL == (p = xmlNodeGetContent(node5))) break;
		//printf("netmask:%s\n",p);
		ls_str_to_int32((const char *)p,&ip->prefix);
		xmlFree(p);
		p = NULL;
		this->status = STATUS_ERRNO;
	}while(0);
	
    if(NULL != xpath_object) xmlXPathFreeObject(xpath_object);
    if(NULL != xpath_context) xmlXPathFreeContext(xpath_context);
    if(NULL != doc) xmlFreeDoc(doc);
    xmlCleanupParser();
	//if(!this->internet_token.empty() )
	//	this->status = STATUS_ERRNO;
    return;
}
int32_t device_mngmt::handle_mngmt_set_default_gateway(const char *gateway ,uint32_t gateway_len)
{
	char                wss_header[2048] = {'\0'};
	char				buf[4096]        = {'\0'};

	this->status = STATUS_ERROR;
	char				*this_gateway = new char[gateway_len +1 ];
	strncpy(this_gateway,gateway,gateway_len);
	this_gateway[gateway_len] = '\0';
	build_wss_header(wss_header, sizeof(wss_header));
	size_t					len;
	len = sprintf(buf,		   "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
                               "<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\""
                               " xmlns:tds=\"http://www.onvif.org/ver10/device/wsdl\""
                               " xmlns:tt=\"http://www.onvif.org/ver10/schema\">%s"
                               "<s:Body>"
                               "<tds:SetNetworkDefaultGateway>"
							   "<tds:IPv4Address>%s</tds:IPv4Address>"
							   "</tds:SetNetworkDefaultGateway>"
                               "</s:Body>"
                               "</s:Envelope>", wss_header,this_gateway);
	string content_type = string("application/soap+xml; charset=utf-8");
	
	delete []this_gateway;
	if(this->xaddrs_uri.empty())
		set_xaddrs();

	ls_http_request request(HTTP_POST,1,1,this->xaddrs_port);
	request.setUri(this->xaddrs_uri.c_str(),this->xaddrs_uri.length());
	request.setHost(this->IP.c_str(),this->IP.length());
	request.setContentType(content_type.c_str(),content_type.length());
	request.setContent(buf,len);
	request.setHandler((HandlerProc *)&handle_mngmt_set_default_gateway_help,(void *)this);
	if( -1 == request.sendTo() )
			return STATUS_CONNECT_DEVICE_TIMEOUT;
	request.recvFrom();
	return this->status;
}
void device_mngmt::handle_mngmt_set_default_gateway_help(ls_http_response* http_response,void *client_data)
{
	device_mngmt *me = static_cast<device_mngmt *>(client_data);
    me->handle_message_set_default_gateway(http_response);
}
void device_mngmt::handle_message_set_default_gateway(ls_http_response* http_response)
{
	//lsprint("%s",http_response->response);//csp modify
	/* check error */
	this->status = check_reponse_error(http_response);
    if( STATUS_PARSE_RESPONSE_ERROR != this->status )
		return;
	this->status = STATUS_ERRNO;
}
int32_t device_mngmt::handle_mngmt_get_default_gateway(device_gateway *gateway)
{
	char                wss_header[2048] = {'\0'};
	char				buf[4096]        = {'\0'};

	this->status = STATUS_ERROR;

	build_wss_header(wss_header, sizeof(wss_header));
	size_t					len;
	len = sprintf(buf,		   "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
                               "<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\""
                               " xmlns:tds=\"http://www.onvif.org/ver10/device/wsdl\""
                               " xmlns:tt=\"http://www.onvif.org/ver10/schema\">%s"
                               "<s:Body>"
                               "<tds:GetNetworkDefaultGateway />"
                               "</s:Body>"
                               "</s:Envelope>", wss_header);
	string content_type = string("application/soap+xml; charset=utf-8");
	
	if(this->xaddrs_uri.empty())
		set_xaddrs();

	ls_http_request request(HTTP_POST,1,1,this->xaddrs_port);
	request.setUri(this->xaddrs_uri.c_str(),this->xaddrs_uri.length());
	request.setHost(this->IP.c_str(),this->IP.length());
	request.setContentType(content_type.c_str(),content_type.length());
	request.setContent(buf,len);
	request.setHandler((HandlerProc *)&handle_mngmt_get_default_gateway_help,(void *)this,(void *)gateway);
	if( -1 == request.sendTo() )
			return STATUS_CONNECT_DEVICE_TIMEOUT;
	request.recvFrom();
	return this->status;
}
void device_mngmt::handle_mngmt_get_default_gateway_help(ls_http_response* http_response,void *client_data,void *gateway)
{
	device_mngmt *me = static_cast<device_mngmt *>(client_data);
    me->handle_message_get_default_gateway(http_response,(device_gateway *)gateway);
}
void device_mngmt::handle_message_get_default_gateway(ls_http_response* http_response,device_gateway *gateway)
{
	xmlDocPtr           doc = NULL;
    xmlNodePtr          node = NULL, node2 = NULL;
    xmlXPathContextPtr  xpath_context = NULL;
    xmlXPathObjectPtr   xpath_object = NULL;
    xmlChar            *p = NULL;
    int                 i;
	
	//lsprint("handle_message_get_default_gateway:%s",http_response->response);//csp modify
	//lsprint("gateway:%s",gateway->gateway);//csp modify
	
	/* check error */
	this->status = check_reponse_error(http_response);
    if( STATUS_PARSE_RESPONSE_ERROR != this->status )
		return;
	
	/* parse XML */
	do
	{
		if(NULL == (doc = xmlParseMemory(http_response->respone_content, http_response->respone_content_len))) break;
		if(NULL == (xpath_context = xmlXPathNewContext(doc))) break;
		if(0 != register_onvif_namespaces(xpath_context)) break;
		if(NULL == (xpath_object = xmlXPathEvalExpression(BAD_CAST"/SOAP-ENV:Envelope/SOAP-ENV:Body/tds:GetNetworkDefaultGatewayResponse/tds:NetworkGateway", xpath_context))) break;
		if(xmlXPathNodeSetIsEmpty(xpath_object->nodesetval)) break;
		
		/* get the first "NetworkGateway" */
		for(i = 0; i < xpath_object->nodesetval->nodeNr; i++)
		{
			if(XML_ELEMENT_NODE == xpath_object->nodesetval->nodeTab[i]->type)
			{
				node = xpath_object->nodesetval->nodeTab[i];
				break;
			}
		}
		if(NULL == node) break;
		//memset(dns,0,sizeof(dns));
		node2 = seek_child_node_by_name(node,BAD_CAST"IPv4Address");
		if(NULL == (p = xmlNodeGetContent(node2))) break;
		strcpy(gateway->gateway,(const char *)p );
		xmlFree(p);
		p = NULL;
		this->status = STATUS_ERRNO;
	}while(0);
	
    if(NULL != xpath_object) xmlXPathFreeObject(xpath_object);
    if(NULL != xpath_context) xmlXPathFreeContext(xpath_context);
    if(NULL != doc) xmlFreeDoc(doc);
	
    xmlCleanupParser();
    return;
}
int32_t device_mngmt::handle_mngmt_set_date_time(uint32_t year,uint32_t month,uint32_t day, uint32_t hours, uint32_t minutes, uint32_t seconds, const char* time_zone)
{
	char                wss_header[2048] = {'\0'};
	char				buf[4096]        = {'\0'};
	
	this->status = STATUS_ERROR;
	build_wss_header(wss_header, sizeof(wss_header));
	
	size_t len = 0;
	
	//printf("Time:(%04d-%02d-%02d %02d:%02d:%02d),Zone:%s\n",year,month,day,hours,minutes,seconds,time_zone);
	
	//csp modify
	if(time_zone == NULL)
	{
		//"Manual"//"NTP"
		len = sprintf(buf,     "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
                               "<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\""
                               " xmlns:tds=\"http://www.onvif.org/ver10/device/wsdl\""
                               " xmlns:tt=\"http://www.onvif.org/ver10/schema\">%s"
                               "<s:Body>"
                               "<tds:SetSystemDateAndTime>"
							   "<tds:DateTimeType>Manual</tds:DateTimeType>"
							   "<tds:DaylightSavings>false</tds:DaylightSavings>"
							   //"<tds:TimeZone>"
							   //"<tt:TZ>GMT+0</tt:TZ>"
							   //"</tds:TimeZone>"
					           "<tds:UTCDateTime>"
							   "<tt:Time>"
							   "<tt:Hour>%u</tt:Hour>"
							   "<tt:Minute>%u</tt:Minute>"
							   "<tt:Second>%u</tt:Second>"
							   "</tt:Time>"
							   "<tt:Date>"
							   "<tt:Year>%u</tt:Year>"
							   "<tt:Month>%u</tt:Month>"
							   "<tt:Day>%u</tt:Day>"
							   "</tt:Date>"
							   "</tds:UTCDateTime>"
							   "</tds:SetSystemDateAndTime>"
                               "</s:Body>"
                               "</s:Envelope>",wss_header,hours,minutes,seconds,year,month,day);
	}
	else
	{
		//"Manual"//"NTP"
		len = sprintf(buf,     "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
                               "<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\""
                               " xmlns:tds=\"http://www.onvif.org/ver10/device/wsdl\""
                               " xmlns:tt=\"http://www.onvif.org/ver10/schema\">%s"
                               "<s:Body>"
                               "<tds:SetSystemDateAndTime>"
							   "<tds:DateTimeType>Manual</tds:DateTimeType>"
							   "<tds:DaylightSavings>false</tds:DaylightSavings>"
							   "<tds:TimeZone>"
							   "<tt:TZ>%s</tt:TZ>"
							   "</tds:TimeZone>"
					           "<tds:UTCDateTime>"
							   "<tt:Time>"
							   "<tt:Hour>%u</tt:Hour>"
							   "<tt:Minute>%u</tt:Minute>"
							   "<tt:Second>%u</tt:Second>"
							   "</tt:Time>"
							   "<tt:Date>"
							   "<tt:Year>%u</tt:Year>"
							   "<tt:Month>%u</tt:Month>"
							   "<tt:Day>%u</tt:Day>"
							   "</tt:Date>"
							   "</tds:UTCDateTime>"
							   "</tds:SetSystemDateAndTime>"
                               "</s:Body>"
                               "</s:Envelope>",wss_header,time_zone,hours,minutes,seconds,year,month,day);
	}
	
	string content_type = string("application/soap+xml; charset=utf-8");
	
	if(this->xaddrs_uri.empty())
		set_xaddrs();
	
	ls_http_request request(HTTP_POST,1,1,this->xaddrs_port);
	request.setUri(this->xaddrs_uri.c_str(),this->xaddrs_uri.length());
	request.setHost(this->IP.c_str(),this->IP.length());
	request.setContentType(content_type.c_str(),content_type.length());
	request.setContent(buf,len);
	request.setHandler((HandlerProc *)&handle_mngmt_set_date_time_help,(void *)this);
	if( -1 == request.sendTo() )
			return STATUS_CONNECT_DEVICE_TIMEOUT;
	request.recvFrom();
	return this->status;
}
void device_mngmt::handle_mngmt_set_date_time_help(ls_http_response* http_response,void *client_data)
{
	device_mngmt *me = static_cast<device_mngmt *>(client_data);
    me->handle_message_set_date_time(http_response);
}
void device_mngmt::handle_message_set_date_time(ls_http_response* http_response)
{
	//csp modify
	//lsprint("%s",http_response->response);
	
	/* check error */
	this->status = check_reponse_error(http_response);
    if( STATUS_PARSE_RESPONSE_ERROR != this->status )
		return;
	this->status = STATUS_ERRNO;
}
int32_t device_mngmt::handle_mngmt_set_date_time(const time_t time, const char* time_zone)
{
	 struct tm stm;
	 //csp modify
	 //localtime_r(&time, &stm);
	 gmtime_r(&time, &stm);
	 return handle_mngmt_set_date_time(stm.tm_year+1900,stm.tm_mon+1,stm.tm_mday,stm.tm_hour,stm.tm_min,stm.tm_sec,time_zone);
}

int32_t device_mngmt::handle_mngmt_set_time_zone(const char* time_zone)
{
	char                wss_header[2048] = {'\0'};
	char				buf[4096]        = {'\0'};
	
	this->status = STATUS_ERROR;
	build_wss_header(wss_header, sizeof(wss_header));
	size_t					len;
	len = sprintf(buf,		   "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
                               "<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\""
                               " xmlns:tds=\"http://www.onvif.org/ver10/device/wsdl\""
                               " xmlns:tt=\"http://www.onvif.org/ver10/schema\">%s"
                               "<s:Body>"
                                "<tds:SetSystemDateAndTime>"
								"<tds:TimeZone>"
								"<tt:TZ>%s</tt:TZ>"
								"</tds:TimeZone>"
								"</tds:SetSystemDateAndTime>"
                               "</s:Body>"
                               "</s:Envelope>",wss_header,time_zone);
	string content_type = string("application/soap+xml; charset=utf-8");
	
	if(this->xaddrs_uri.empty())
		set_xaddrs();
	
	ls_http_request request(HTTP_POST,1,1,this->xaddrs_port);
	request.setUri(this->xaddrs_uri.c_str(),this->xaddrs_uri.length());
	request.setHost(this->IP.c_str(),this->IP.length());
	request.setContentType(content_type.c_str(),content_type.length());
	request.setContent(buf,len);
	request.setHandler((HandlerProc *)&handle_mngmt_set_time_zone_help,(void *)this);
	if( -1 == request.sendTo() )
			return STATUS_CONNECT_DEVICE_TIMEOUT;
	request.recvFrom();
	return this->status;
}
void device_mngmt::handle_mngmt_set_time_zone_help(ls_http_response* http_response,void *client_data)
{
	device_mngmt *me = static_cast<device_mngmt *>(client_data);
    me->handle_message_set_time_zone(http_response);
}
void device_mngmt::handle_message_set_time_zone(ls_http_response* http_response)
{
	lsprint("%s",http_response->response);//csp modify
	/* check error */
	this->status = check_reponse_error(http_response);
    if( STATUS_PARSE_RESPONSE_ERROR != this->status )
		return;
	this->status = STATUS_ERRNO;
}

/*#define BUFSIZE 2048
#define DestIp "192.168.4.125"
#define URI "/onvif/device_service"
#define DestPort 80
#define CONTENTTYPE "application/soap+xml; charset=utf-8; action=\"http://www.onvif.org/ver10/device/wsdl/GetCapabilities\" "
#define Req "POST /onvif/device_service HTTP/1.1\r\nHost: 192.168.4.146\r\nContent-Type: application/soap+xml; charset=utf-8; action=\"http://www.onvif.org/ver10/device/wsdl/GetCapabilities\"\r\nContent-Length: \r\n\r\n"
#define ReqLen sizeof(Req)
#define      SVX_HTTP_CONTENT_TYPE_APPLICATION_SOAP "application/soap+xml"
void device_info::handle_mngmt_capabilities(char *buf,int &len)
{
    
    uint8_t            *request_buf = NULL;
    size_t              request_buf_len;
	int					len1;
    char				buf1[2048];
    char                wss_header[2048] = {'\0'};

    build_wss_header(wss_header, sizeof(wss_header));
    
 
    len1 = sprintf(buf1,			   "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
                                   "<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\""
                                   "               xmlns:tds=\"http://www.onvif.org/ver10/device/wsdl\""
                                   "               xmlns:tt=\"http://www.onvif.org/ver10/schema\">%s"
                                   "  <s:Body>"
                                   "    <tds:GetCapabilities/>"
                                   "  </s:Body>"
                                   "</s:Envelope>", wss_header);
	len =sprintf(buf,			   "POST /onvif/device_service HTTP/1.1\r\nHost: 192.168.4.146\r\nContent-Type: application/soap+xml; charset=utf-8; action=\"http://www.onvif.org/ver10/device/wsdl/GetCapabilities\"\r\nContent-Length:%d \r\n\r\n"
                                   "%s",len1,buf1);
    
    //ls_tcp_client_connection_write(tcp_client_xaddrs, request_buf, request_buf_len);
 
}
*/

/*int main(int argc ,char *argv[] )
{
	//test();
	device_info
	return 0;
}*/
//int main(int argc ,char *argv[] )
//{
//	
//	ssize_t i;
//	int nRequestLen;
//	//cout<<Req<<endl;
//	char strResponse[BUFSIZE]={0};
//	char strRequest[BUFSIZE]={0};
//	int sockfd, numbytes;
//
//
//	struct sockaddr_in dest_addr; /* connector's address information */
//
//
//	handle_mngmt_capabilities(strRequest,nRequestLen);
//	cout<<strRequest<<nRequestLen<<endl;
//	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
//		perror("socket");
//		exit(1);
//	}
//
//	dest_addr.sin_family = AF_INET; /* host byte order */
//	dest_addr.sin_port = htons(DestPort); /* short, network byte order */
//	dest_addr.sin_addr.s_addr = inet_addr(DestIp);
//
//	/* Create and setup the connection */
//	if (connect(sockfd, (struct sockaddr *)&dest_addr,sizeof(struct sockaddr)) == -1) {
//		perror("connect");
//		exit(1);
//	}
//
//	/* Send the request */
//	//strncpy(strRequest, Req,ReqLen);
//	//nRequestLen = ReqLen;
//
//	if ( write(sockfd,strRequest,nRequestLen) == -1) {
//		perror("write");
//		exit(1);
//	}
//	/* Read in the response */
//	while(1) {
//		i = read(sockfd,strResponse,BUFSIZE-1);
//		if(0 >= i){
//			break;
//		}
//		strResponse[i]='\0';
//		cout<<strResponse<<endl;
//
//	}
//
//	/* Close the connection */
//	close(sockfd);
//
//
//
//
//	return 0;
//}
