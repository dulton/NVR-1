
/* -*- C++ -*- */
#ifndef __lS_RECEIVER_MNGMT_H
#define __LS_RECEIVER_MNGMT_H 1
//#include <string.h>
#include <string>
#include <vector>
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <time.h>
#include "ls_http.h"
#include "ls_ptz.h"





class discovery_device
{
public:
	discovery_device(){};
	virtual ~discovery_device(){};
	const string& get_xaddrs(){return XAddrs;};
	const string& get_uuid(){return uuid;};
	const string& get_ip() {return IP ;};
	const char *  get_xaddrs(int & len){len = XAddrs.length();return XAddrs.c_str();};
	const char *  get_uuid(int & len) {len = uuid.length();return uuid.c_str() ;};
	const char *  get_ip(int & len) {len = IP.length();return IP.c_str() ;};
	void set_ip(const char *str,size_t  len){IP.assign(str,len);};
	void set_xaddrs(const char *str,size_t  len){XAddrs.assign(str,len);};
	void set_uuid(const char *str, size_t  len){uuid.assign(str,len);};

protected:
	std::string					 XAddrs;
	std::string					 uuid;
	std::string					 IP;
};


#define STATUS_NOT_INIT											-1
#define STATUS_ERRNO											0
#define STATUS_ERROR											1
#define STATUS_NOTAUTHORIZED									2    
#define STATUS_PARSE_RESPONSE_ERROR								3	
#define STATUS_DEVICE_NOT_RESPONSE								4
#define STATUS_DEVICE_RETURN_ERROR								5
#define STATUS_CONNECT_DEVICE_TIMEOUT							6


//STATUS_MAX_IMAGE_CONFIG_ALREADY_INIT and STATUS_IMAGE_CONFIG_NOT_SUPPORT used for handle_image_max_config
#define STATUS_MAX_IMAGE_CONFIG_ALREADY_INIT					7
#define STATUS_IMAGE_CONFIG_NOT_SUPPORT							8




#define HARD_FACTORY_DEFAULT		0
#define SOFT_FACTORY_DEFAULT		1


/*handle_get_device_infomation parametor*/
typedef struct device_infomation
{
	char manufacturer[256];
	char model[256];
	char firmware_version[256];
	char serial_number[256];
	char hardwareId[256];
}device_infomation;

typedef struct device_dns
{
	Boolean from_DHCP;
	char dns[20];
	char type[20];    //ipv4 or ipv6
}device_dns;

typedef struct device_ip
{
	char ip[20];
	int32_t  prefix;
	char hardware_address[30];
	Boolean from_DHCP;
}device_ip;

typedef struct device_gateway
{
	char gateway[20];
}device_gateway;

typedef struct device_ntp
{
	char ntp[20];
}device_ntp;

class device_mngmt : public discovery_device
{
public:
	device_mngmt();
	virtual ~device_mngmt();
	/*DeviceMngmt*/
	int32_t handle_mngmt_capabilities();//need XAddrs
	int32_t handle_mngmt_get_device_infomation(device_infomation *info);//need XAddrs
	//if ip = NULL or ip_len = 0   ip address from DHCP
	int32_t handle_mngmt_set_ip_prefix(const char *ip,uint32_t ip_len,uint32_t prefix_length);
	int32_t handle_mngmt_set_default_gateway(const char *gateway ,uint32_t gateway_len);
	int32_t handle_mngmt_get_default_gateway(device_gateway *gateway);
	int32_t handle_mngmt_get_ip(device_ip *ip);
	//if dns = NULL or dns_len = 0   dns address from DHCP
	int32_t handle_mngmt_set_dns(const char *dns,uint32_t dns_len);
	int32_t handle_mngmt_get_dns(device_dns *dns);
	//if ntp = NULL or ntp_len = 0   ntp from DHCP
	int32_t handle_mngmt_set_ntp(const char *ntp,uint32_t ntp_len);
	int32_t handle_mngmt_get_ntp();
	int32_t handle_mngmt_set_date_time(uint32_t year,uint32_t month,uint32_t day, uint32_t hours, uint32_t minutes, uint32_t seconds, const char* time_zone = NULL);
	int32_t handle_mngmt_set_time_zone(const char* time_zone);
	int32_t handle_mngmt_set_date_time(const time_t time, const char* time_zone = NULL);
	int32_t handle_mngmt_system_reboot();
	int32_t handle_mngmt_system_factory_default(uint32_t hard_or_soft);
	
	static void handle_mngmt_capabilities_help(ls_http_response* http_response,void *client_data);
	static void handle_mngmt_get_device_infomation_help(ls_http_response* http_response,void *client_data,void *info);
	static void handle_mngmt_set_ip_prefix_help(ls_http_response* http_response,void *client_data);
	static void handle_mngmt_set_default_gateway_help(ls_http_response* http_response,void *client_data);
	static void handle_mngmt_get_default_gateway_help(ls_http_response* http_response,void *client_data,void *gateway);
	static void handle_mngmt_get_ip_help(ls_http_response* http_response,void *client_data,void *ip);
	static void handle_mngmt_set_dns_help(ls_http_response* http_response,void *client_data);
	static void handle_mngmt_get_dns_help(ls_http_response* http_response,void *client_data,void *dns);
	static void handle_mngmt_set_ntp_help(ls_http_response* http_response,void *client_data);
	static void handle_mngmt_get_ntp_help(ls_http_response* http_response,void *client_data);
	static void handle_mngmt_set_time_zone_help(ls_http_response* http_response,void *client_data);
	static void handle_mngmt_set_date_time_help(ls_http_response* http_response,void *client_data);
	static void handle_mngmt_system_reboot_help(ls_http_response* http_response,void *client_data);
	static void handle_mngmt_system_factory_default_help(ls_http_response* http_response,void *client_data);
	
	void set_username_password(const char *username,const char* password);
private:
	void handle_message_capabilities(ls_http_response* http_response);
	void handle_message_get_device_infomation(ls_http_response* http_response,device_infomation *info);


	void handle_message_set_ip_prefix(ls_http_response* http_response);
	void handle_message_set_default_gateway(ls_http_response* http_response);
	void handle_message_get_default_gateway(ls_http_response* http_response,device_gateway *gateway);
	void handle_message_get_ip(ls_http_response* http_response,device_ip *ip);
	void handle_message_set_dns(ls_http_response* http_response);
	void handle_message_get_dns(ls_http_response* http_response,device_dns *dns);
	void handle_message_set_ntp(ls_http_response* http_response);
	void handle_message_get_ntp(ls_http_response* http_response);
	void handle_message_set_date_time(ls_http_response* http_response);
	void handle_message_system_reboot(ls_http_response* http_response);
	void handle_message_set_time_zone(ls_http_response* http_response);
	void handle_message_system_factory_default(ls_http_response* http_response);
protected:
	xmlNodePtr seek_child_node_by_name(xmlNodePtr node, const xmlChar *name);
	uint32_t register_onvif_namespaces(xmlXPathContextPtr xpath_context);

	int build_wss_header(char *buf, size_t buf_len);
	int32_t check_reponse_error(ls_http_response* http_response);
	int set_xaddrs();
protected:
	std::string						username;		//use to authorization
	std::string						password;		//use to authorization
	
	
	int								xaddrs_port;//parser from XAddrs
	std::string						xaddrs_uri;//parser from XAddrs
	
	std::string						xaddrs_uri_media;  //init by handle_mngmt_capabilities
	std::string						xaddrs_uri_ptz;    //init by handle_mngmt_capabilities
	std::string						xaddrs_uri_image;  //init by handle_mngmt_capabilities
	std::string						xaddrs_uri_event;  //init by handle_mngmt_capabilities
	Boolean							image_config_support; //init by handle_mngmt_capabilities
	
	string							internet_token ;	//need it when set ip (handle_mngmt_set_ip_prefix)
	int32_t							status;				//the return status;
	
};


#endif
