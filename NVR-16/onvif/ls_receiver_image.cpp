#include "ls_receiver_image.h"
#include <string.h>
#include "ls_string.h"

device_image::device_image()
{
	max_image_setting.already_init = 0;
	max_image_setting.brightness_min = 0;
	max_image_setting.brightness_max = 100;
	max_image_setting.contrast_min = 0;
	max_image_setting.contrast_max = 100;
	max_image_setting.color_saturation_min = 0;
	max_image_setting.color_saturation_max = 100;
	max_image_setting.sharpness_min = 0;
	max_image_setting.sharpness_max = 100;
}

device_image::~device_image()
{
	
}

max_image_config *device_image::get_max_image_setting()
{
	if(!this->max_image_setting.already_init)
		handle_image_max_config();
	return &this->max_image_setting;
}	

Boolean device_image::handle_image_setting_support()
{
	if(	profiles.empty() )
		handle_media_profiles();
	if( profiles.empty() || !image_config_support || image_supported == NOT_SUPPORT || ptz_supported == SUPPORT_NOT_INIT)
		return False;
	return True;
}
int32_t device_image::handle_image_max_config()
{
	char                wss_header[2048] = {'\0'};
	char				buf[4096]        = {'\0'};
	this->status = STATUS_ERROR;

	if( this->xaddrs_uri_image.empty() )
		handle_mngmt_capabilities();
	if(this->max_image_setting.already_init )
		return STATUS_MAX_IMAGE_CONFIG_ALREADY_INIT;
	if( !this->image_config_support )
		return STATUS_IMAGE_CONFIG_NOT_SUPPORT;
	
	if(	profiles.empty() )
		handle_media_profiles();
	if( profiles.empty() )
		return STATUS_ERROR;
	build_wss_header(wss_header, sizeof(wss_header));
	
	size_t len = 0;
	
	//csp modify
	len = sprintf(buf,		   "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
                                   "<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\""
                                   " xmlns:timg=\"http://www.onvif.org/ver20/imaging/wsdl\""
								   " xmlns:tt=\"http://www.onvif.org/ver10/schema\">%s"
                                   "<s:Body>"
                                   "<timg:GetOptions>"
								   "<timg:VideoSourceToken>%s"
								   "</timg:VideoSourceToken>"
								   "</timg:GetOptions>"
                                   "</s:Body>"
								   "</s:Envelope>", wss_header,profiles.at(0)->image_token);
	string content_type = string("application/soap+xml; charset=utf-8 ");
	
	
	ls_http_request request(HTTP_POST,1,1,this->xaddrs_port);
	request.setUri(this->xaddrs_uri_image.c_str(),this->xaddrs_uri_image.length());
	request.setHost(this->IP.c_str(),this->IP.length());
	request.setContentType(content_type.c_str(),content_type.length());
	request.setContent(buf,len);
	request.setHandler((HandlerProc *)&handle_image_max_config_helper,(void *)this);
	if( -1 == request.sendTo() )
			return STATUS_CONNECT_DEVICE_TIMEOUT;
	request.recvFrom();
	return this->status;
}
void device_image::handle_image_max_config_helper(ls_http_response* http_response,void *client_data)
{
	device_image *me = static_cast<device_image *>(client_data);
    me->handle_message_max_config(http_response);
}
void device_image::handle_message_max_config(ls_http_response* http_response)
{
	xmlDocPtr           doc = NULL;
    xmlNodePtr          node = NULL, node2 = NULL,node3 = NULL;
    xmlXPathContextPtr  xpath_context = NULL;
    xmlXPathObjectPtr   xpath_object = NULL;
    xmlChar            *p = NULL;
	char                p_buf[32];
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
		if(NULL == (xpath_object = xmlXPathEvalExpression(BAD_CAST"/SOAP-ENV:Envelope/SOAP-ENV:Body/timg:GetOptionsResponse/timg:ImagingOptions", xpath_context))) break;

		if(xmlXPathNodeSetIsEmpty(xpath_object->nodesetval)) break;

		/* get the first "ImagingOptions" */
		for(i = 0; i < xpath_object->nodesetval->nodeNr; i++)
		{
			if(XML_ELEMENT_NODE == xpath_object->nodesetval->nodeTab[i]->type)
			{
				node = xpath_object->nodesetval->nodeTab[i];            
				break;
			}
		}
		if(NULL == node) break;

		if(NULL == (node2 = seek_child_node_by_name(node, BAD_CAST"Brightness"))) break;
		if(NULL == (node3 = seek_child_node_by_name(node2, BAD_CAST"Min"))) break;
		if(NULL == (p = xmlNodeGetContent(node3))) break;
		strncpy(p_buf, (const char *)p, sizeof(p_buf));
		xmlFree(p);
		p = NULL;
		if(0 != ls_str_to_int(p_buf, &max_image_setting.brightness_min)) break;
		if(NULL == (node3 = seek_child_node_by_name(node2, BAD_CAST"Max"))) break;
		if(NULL == (p = xmlNodeGetContent(node3))) break;
		strncpy(p_buf, (const char *)p, sizeof(p_buf));
		xmlFree(p);
		p = NULL;
		if(0 != ls_str_to_int(p_buf, &max_image_setting.brightness_max)) break;

	
		if(NULL == (node2 = seek_child_node_by_name(node, BAD_CAST"ColorSaturation"))) break;
		if(NULL == (node3 = seek_child_node_by_name(node2, BAD_CAST"Min"))) break;
		if(NULL == (p = xmlNodeGetContent(node3))) break;
		strncpy(p_buf, (const char *)p, sizeof(p_buf));
		xmlFree(p);
		p = NULL;
		if(0 != ls_str_to_int(p_buf, &max_image_setting.color_saturation_min)) break;
		if(NULL == (node3 = seek_child_node_by_name(node2, BAD_CAST"Max"))) break;
		if(NULL == (p = xmlNodeGetContent(node3))) break;
		strncpy(p_buf, (const char *)p, sizeof(p_buf));
		xmlFree(p);
		p = NULL;
		if(0 != ls_str_to_int(p_buf, &max_image_setting.color_saturation_max)) break;



		if(NULL == (node2 = seek_child_node_by_name(node, BAD_CAST"Contrast"))) break;
		if(NULL == (node3 = seek_child_node_by_name(node2, BAD_CAST"Min"))) break;
		if(NULL == (p = xmlNodeGetContent(node3))) break;
		strncpy(p_buf, (const char *)p, sizeof(p_buf));
		xmlFree(p);
		p = NULL;
		if(0 != ls_str_to_int(p_buf, &max_image_setting.contrast_min)) break;
		if(NULL == (node3 = seek_child_node_by_name(node2, BAD_CAST"Max"))) break;
		if(NULL == (p = xmlNodeGetContent(node3))) break;
		strncpy(p_buf, (const char *)p, sizeof(p_buf));
		xmlFree(p);
		p = NULL;
		if(0 != ls_str_to_int(p_buf, &max_image_setting.contrast_max)) break;



		if(NULL == (node2 = seek_child_node_by_name(node, BAD_CAST"Sharpness"))) break;
		if(NULL == (node3 = seek_child_node_by_name(node2, BAD_CAST"Min"))) break;
		if(NULL == (p = xmlNodeGetContent(node3))) break;
		strncpy(p_buf, (const char *)p, sizeof(p_buf));
		xmlFree(p);
		p = NULL;
		if(0 != ls_str_to_int(p_buf, &max_image_setting.sharpness_min)) break;
		if(NULL == (node3 = seek_child_node_by_name(node2, BAD_CAST"Max"))) break;
		if(NULL == (p = xmlNodeGetContent(node3))) break;
		strncpy(p_buf, (const char *)p, sizeof(p_buf));
		xmlFree(p);
		p = NULL;
		if(0 != ls_str_to_int(p_buf, &max_image_setting.sharpness_max)) break;
		this->status = STATUS_ERRNO;
		this->max_image_setting.already_init = True;
	}while(0);
 
	/*lsprint("Sharpness:%d-%d\nContrast:%d-%d\nBrightness:%d-%d\nColorSaturation:%d-%d\n",
		this->max_image_setting.sharpness_min,this->max_image_setting.sharpness_max,this->max_image_setting.contrast_min,this->max_image_setting.contrast_max,this->max_image_setting.brightness_min,this->max_image_setting.brightness_max,this->max_image_setting.sharpness_min,this->max_image_setting.sharpness_max);
   */
    if(NULL != xpath_object) xmlXPathFreeObject(xpath_object);
    if(NULL != xpath_context) xmlXPathFreeContext(xpath_context);
    if(NULL != doc) xmlFreeDoc(doc);
    xmlCleanupParser();
    return;
}
int32_t device_image::handle_image_current_config(uint32_t stream_index,current_image_config *current_image_setting)
{
	char                wss_header[2048] = {'\0'};
	char				buf[4096]        = {'\0'};
	
	this->status = STATUS_ERROR;

	if( this->xaddrs_uri_image.empty() )
		handle_mngmt_capabilities();
	if( !this->image_config_support )
		return STATUS_IMAGE_CONFIG_NOT_SUPPORT;
	
	if(	profiles.empty() )
		handle_media_profiles();
	if( profiles.empty() )
		return STATUS_ERROR;
	build_wss_header(wss_header, sizeof(wss_header));
	size_t					len;
	len = sprintf(buf,		   "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
                                   "<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\""
                                   " xmlns:timg=\"http://www.onvif.org/ver20/imaging/wsdl\""
								   " xmlns:tt=\"http://www.onvif.org/ver10/schema\">%s"
                                   "<s:Body>"
                                   "<timg:GetImagingSettings>"
								   "<timg:VideoSourceToken>%s</timg:VideoSourceToken>"
								   "</timg:GetImagingSettings>"
                                   "</s:Body>"
								   "</s:Envelope>", wss_header,profiles.at(stream_index)->image_token);
	string content_type = string("application/soap+xml; charset=utf-8 ");

	
	ls_http_request request(HTTP_POST,1,1,this->xaddrs_port);
	request.setUri(this->xaddrs_uri_image.c_str(),this->xaddrs_uri_image.length());
	request.setHost(this->IP.c_str(),this->IP.length());
	request.setContentType(content_type.c_str(),content_type.length());
	request.setContent(buf,len);
	request.setHandler((HandlerProc *)&handle_image_current_config_helper,(void *)this,(void *)current_image_setting);
	if( -1 == request.sendTo() )
			return STATUS_CONNECT_DEVICE_TIMEOUT;
	request.recvFrom();
	return this->status;
}
void device_image::handle_image_current_config_helper(ls_http_response* http_response,void *client_data,void *current_image_setting)
{
	device_image *me = static_cast<device_image *>(client_data);
    me->handle_message_current_config(http_response,(current_image_config * )current_image_setting);
}

void device_image::handle_message_current_config(ls_http_response* http_response,current_image_config *current_image_setting)
{
	xmlDocPtr           doc = NULL;
    xmlNodePtr          node = NULL, node2 = NULL;
    xmlXPathContextPtr  xpath_context = NULL;
    xmlXPathObjectPtr   xpath_object = NULL;
    xmlChar            *p = NULL;
	char                p_buf[32];
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
		if(NULL == (xpath_object = xmlXPathEvalExpression(BAD_CAST"/SOAP-ENV:Envelope/SOAP-ENV:Body/timg:GetImagingSettingsResponse/timg:ImagingSettings", xpath_context))) break;
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

		node2 = seek_child_node_by_name(node,BAD_CAST"BacklightCompensation");
		if(NULL == (p = xmlNodeGetContent(node2))) break;
		strncpy(p_buf, (const char *)p, sizeof(p_buf));
		xmlFree(p);
		p = NULL;
		current_image_setting->backlight_compensation =  (  (strstr(p_buf,"ON") != NULL) || (strstr(p_buf,"on") != NULL)  )? True:False;

		
		if(NULL == (node2 = seek_child_node_by_name(node, BAD_CAST"Brightness"))) break;
		if(NULL == (p = xmlNodeGetContent(node2))) break;
		strncpy(p_buf, (const char *)p, sizeof(p_buf));
		xmlFree(p);
		p = NULL;
		if(0 != ls_str_to_int(p_buf, &current_image_setting->brightness)) break;
		
		if(NULL == (node2 = seek_child_node_by_name(node, BAD_CAST"Sharpness"))) break;
		if(NULL == (p = xmlNodeGetContent(node2))) break;
		strncpy(p_buf, (const char *)p, sizeof(p_buf));
		xmlFree(p);
		p = NULL;
		if(0 != ls_str_to_int(p_buf, &current_image_setting->sharpness)) break;
		

		if(NULL == (node2 = seek_child_node_by_name(node, BAD_CAST"ColorSaturation"))) break;
		if(NULL == (p = xmlNodeGetContent(node2))) break;
		strncpy(p_buf, (const char *)p, sizeof(p_buf));
		xmlFree(p);
		p = NULL;
		if(0 != ls_str_to_int(p_buf, &current_image_setting->color_saturation)) break;
		

		if(NULL == (node2 = seek_child_node_by_name(node, BAD_CAST"Contrast"))) break;
		if(NULL == (p = xmlNodeGetContent(node2))) break;
		strncpy(p_buf, (const char *)p, sizeof(p_buf));
		xmlFree(p);
		p = NULL;
		if(0 != ls_str_to_int(p_buf, &current_image_setting->contrast)) break;
		
		this->status = STATUS_ERRNO;
	}while(0);
 
    if(NULL != xpath_object) xmlXPathFreeObject(xpath_object);
    if(NULL != xpath_context) xmlXPathFreeContext(xpath_context);
    if(NULL != doc) xmlFreeDoc(doc);
    xmlCleanupParser();
}

int32_t device_image::handle_image_focus(uint32_t stream_index,int32_t plus)
{
	char                wss_header[2048] = {'\0'};
	char				buf[4096]        = {'\0'};
	size_t				len;
	this->status = STATUS_ERROR;
	
	//printf("handle_image_focus-1\n");
	
	if( this->xaddrs_uri_image.empty() )
		handle_mngmt_capabilities();
	if( !this->image_config_support )
		return STATUS_IMAGE_CONFIG_NOT_SUPPORT;
	
	//printf("handle_image_focus-2\n");
	
	if(	profiles.empty() )
		handle_media_profiles();
	if( profiles.empty() )
		return STATUS_ERROR;
	
	//printf("handle_image_focus-3\n");
	
	int speed = 1;
	if(plus == 0)
	{
		speed = -1;
	}
	
	build_wss_header(wss_header, sizeof(wss_header));
	
	if(plus == 2)//stop
	{
		len = sprintf(buf,		   "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
                                   "<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\""
                                   " xmlns:timg=\"http://www.onvif.org/ver20/imaging/wsdl\""
								   " xmlns:tt=\"http://www.onvif.org/ver10/schema\">%s"
                                   "<s:Body>"
                                   "<timg:Stop>"
								   "<timg:VideoSourceToken>%s</timg:VideoSourceToken>"
								   "</timg:Stop>"
                                   "</s:Body>"
								   "</s:Envelope>", wss_header,profiles.at(stream_index)->image_token,speed);
		//printf("handle_image_focus_stop:%s\n", buf);
	}
	else
	{
		len = sprintf(buf,		   "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
                                   "<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\""
                                   " xmlns:timg=\"http://www.onvif.org/ver20/imaging/wsdl\""
								   " xmlns:tt=\"http://www.onvif.org/ver10/schema\">%s"
                                   "<s:Body>"
                                   "<timg:Move>"
								   "<timg:VideoSourceToken>%s</timg:VideoSourceToken>"
								   "<timg:Focus>"
								   "<tt:Continuous><tt:Speed>%d</tt:Speed></tt:Continuous>"
								   "</timg:Focus>"
								   "</timg:Move>"
                                   "</s:Body>"
								   "</s:Envelope>", wss_header,profiles.at(stream_index)->image_token,speed);
		//printf("handle_image_focus_start:%s\n", buf);
	}
	
	string content_type = string("application/soap+xml; charset=utf-8 ");
	
	ls_http_request request(HTTP_POST,1,1,this->xaddrs_port);
	request.setUri(this->xaddrs_uri_image.c_str(),this->xaddrs_uri_image.length());
	request.setHost(this->IP.c_str(),this->IP.length());
	request.setContentType(content_type.c_str(),content_type.length());
	request.setContent(buf,len);
	request.setHandler((HandlerProc *)&handle_image_set_brightness_helper,(void *)this);
	if( -1 == request.sendTo() )
			return STATUS_CONNECT_DEVICE_TIMEOUT;
	request.recvFrom();
	return this->status;
}

int32_t device_image::handle_image_set_color(uint32_t stream_index,int32_t brightness,int32_t contrast,int32_t color_saturation,int32_t sharpness)
{
	char                wss_header[2048] = {'\0'};
	char				buf[4096]        = {'\0'};
	size_t				len;
	this->status = STATUS_ERROR;
	
	if( this->xaddrs_uri_image.empty() )
		handle_mngmt_capabilities();
	if( !this->image_config_support )
		return STATUS_IMAGE_CONFIG_NOT_SUPPORT;
	
	if(	profiles.empty() )
		handle_media_profiles();
	if( profiles.empty() )
		return STATUS_ERROR;
	
	build_wss_header(wss_header, sizeof(wss_header));
	
	int min = 0;
	int max = 100;
	if(this->max_image_setting.already_init)
	{
		min = this->max_image_setting.brightness_min;
		max = this->max_image_setting.brightness_max;
	}
	if(max >= min)
	{
		int value = brightness;
		value = min + (value) * (max - min) / (256);
		if(value < min)
		{
			value = min;
		}
		if(value > max)
		{
			value = max;
		}
		brightness = value;
		//printf("brightness: (%d,%d,%d)\n",min,max,value);
	}
	
	min = 0;
	max = 100;
	if(this->max_image_setting.already_init)
	{
		min = this->max_image_setting.contrast_min;
		max = this->max_image_setting.contrast_max;
	}
	if(max >= min)
	{
		int value = contrast;
		value = min + (value) * (max - min) / (256);
		if(value < min)
		{
			value = min;
		}
		if(value > max)
		{
			value = max;
		}
		contrast = value;
		//printf("contrast: (%d,%d,%d)\n",min,max,value);
	}
	
	min = 0;
	max = 100;
	if(this->max_image_setting.already_init)
	{
		min = this->max_image_setting.color_saturation_min;
		max = this->max_image_setting.color_saturation_max;
	}
	if(max >= min)
	{
		int value = color_saturation;
		value = min + (value) * (max - min) / (256);
		if(value < min)
		{
			value = min;
		}
		if(value > max)
		{
			value = max;
		}
		color_saturation = value;
		//printf("color_saturation: (%d,%d,%d)\n",min,max,value);
	}
	
	min = 0;
	max = 100;
	if(this->max_image_setting.already_init)
	{
		min = this->max_image_setting.sharpness_min;
		max = this->max_image_setting.sharpness_max;
	}
	if(max >= min)
	{
		int value = sharpness;
		value = min + (value) * (max - min) / (256);
		if(value < min)
		{
			value = min;
		}
		if(value > max)
		{
			value = max;
		}
		sharpness = value;
		//printf("sharpness: (%d,%d,%d)\n",min,max,value);
	}
	
	len = sprintf(buf,             "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
                                   "<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\""
                                   " xmlns:timg=\"http://www.onvif.org/ver20/imaging/wsdl\""
								   " xmlns:tt=\"http://www.onvif.org/ver10/schema\">%s"
                                   "<s:Body>"
                                   "<timg:SetImagingSettings>"
								   "<timg:VideoSourceToken>%s</timg:VideoSourceToken>"
								   "<timg:ImagingSettings>"
								   "<tt:Brightness>%d</tt:Brightness>"
								   "<tt:Contrast>%d</tt:Contrast>"
								   "<tt:ColorSaturation>%d</tt:ColorSaturation>"
								   //"<tt:Sharpness>%d</tt:Sharpness>"
								   "</timg:ImagingSettings>"
								   "<timg:ForcePersistence>true</timg:ForcePersistence>"
								   "</timg:SetImagingSettings>"
                                   "</s:Body>"
								   "</s:Envelope>", 
								   wss_header, profiles.at(stream_index)->image_token, 
								   brightness, contrast, color_saturation/*, sharpness*/);
	//string content_type = string("application/soap+xml; charset=utf-8; action=\"http://www.onvif.org/ver20/imaging/wsdl/SetImagingSettings\"");
	string content_type = string("application/soap+xml; charset=utf-8; ");
	
	ls_http_request request(HTTP_POST,1,1,this->xaddrs_port);
	request.setUri(this->xaddrs_uri_image.c_str(),this->xaddrs_uri_image.length());
	request.setHost(this->IP.c_str(),this->IP.length());
	request.setContentType(content_type.c_str(),content_type.length());
	request.setContent(buf,len);
	request.setHandler((HandlerProc *)&handle_image_set_brightness_helper,(void *)this);
	if( -1 == request.sendTo() )
			return STATUS_CONNECT_DEVICE_TIMEOUT;
	request.recvFrom();
	return this->status;
}

int32_t device_image::handle_image_set_brightness(uint32_t stream_index,int32_t brightness)
{
	char                wss_header[2048] = {'\0'};
	char				buf[4096]        = {'\0'};
	size_t				len;
	this->status = STATUS_ERROR;
	
	//printf("handle_image_set_brightness-1\n");
	
	if( this->xaddrs_uri_image.empty() )
		handle_mngmt_capabilities();
	if( !this->image_config_support )
		return STATUS_IMAGE_CONFIG_NOT_SUPPORT;
	
	//printf("handle_image_set_brightness-2\n");
	
	if(	profiles.empty() )
		handle_media_profiles();
	if( profiles.empty() )
		return STATUS_ERROR;
	
	//printf("handle_image_set_brightness-3\n");
	
	build_wss_header(wss_header, sizeof(wss_header));
	
	int min = 0;
	int max = 100;
	if(this->max_image_setting.already_init)
	{
		min = this->max_image_setting.brightness_min;
		max = this->max_image_setting.brightness_max;
		printf("handle_image_set_brightness-4,min=%d,max=%d\n",min,max);
	}
	else
	{
		printf("handle_image_set_brightness-5\n");
	}
	if(max >= min)
	{
		int value = brightness;
		value = min + (value) * (max - min) / (256);
		if(value < min)
		{
			value = min;
		}
		if(value > max)
		{
			value = max;
		}
		brightness = value;
		printf("brightness: (%d,%d,%d)\n",min,max,value);
	}
	
	len = sprintf(buf,		   "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
                                   "<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\""
                                   " xmlns:timg=\"http://www.onvif.org/ver20/imaging/wsdl\""
								   " xmlns:tt=\"http://www.onvif.org/ver10/schema\">%s"
                                   "<s:Body>"
                                   "<timg:SetImagingSettings>"
								   "<timg:VideoSourceToken>%s</timg:VideoSourceToken>"
								   "<timg:ImagingSettings>"
								   "<tt:Brightness>%d</tt:Brightness>"
								   "</timg:ImagingSettings>"
								   "</timg:SetImagingSettings>"
                                   "</s:Body>"
								   "</s:Envelope>", wss_header,profiles.at(stream_index)->image_token,brightness);
	string content_type = string("application/soap+xml; charset=utf-8 ");
	
	ls_http_request request(HTTP_POST,1,1,this->xaddrs_port);
	request.setUri(this->xaddrs_uri_image.c_str(),this->xaddrs_uri_image.length());
	request.setHost(this->IP.c_str(),this->IP.length());
	request.setContentType(content_type.c_str(),content_type.length());
	request.setContent(buf,len);
	request.setHandler((HandlerProc *)&handle_image_set_brightness_helper,(void *)this);
	if( -1 == request.sendTo() )
			return STATUS_CONNECT_DEVICE_TIMEOUT;
	request.recvFrom();
	return this->status;
}
void device_image::handle_image_set_brightness_helper(ls_http_response* http_response,void *client_data)
{
	device_image *me = static_cast<device_image *>(client_data);
	me->handle_message_set_brightness(http_response);
}
void device_image::handle_message_set_brightness(ls_http_response* http_response)
{
//	lsprint("%s",http_response->response);
	/* check error */
	this->status = check_reponse_error(http_response);
    if( STATUS_PARSE_RESPONSE_ERROR != this->status )
		return;
	this->status = STATUS_ERRNO;
}
int32_t device_image::handle_image_set_color_saturation(uint32_t stream_index,int32_t color_saturation)
{
	char                wss_header[2048] = {'\0'};
	char				buf[4096]        = {'\0'};
	size_t				len;
	this->status = STATUS_ERROR;

	if( this->xaddrs_uri_image.empty() )
		handle_mngmt_capabilities();
	if( !this->image_config_support )
		return STATUS_IMAGE_CONFIG_NOT_SUPPORT;
		
	if(	profiles.empty() )
		handle_media_profiles();
	if( profiles.empty() )
		return STATUS_ERROR;
	build_wss_header(wss_header, sizeof(wss_header));
	
	int min = 0;
	int max = 100;
	if(this->max_image_setting.already_init)
	{
		min = this->max_image_setting.color_saturation_min;
		max = this->max_image_setting.color_saturation_max;
	}
	if(max >= min)
	{
		int value = color_saturation;
		value = min + (value) * (max - min) / (256);
		if(value < min)
		{
			value = min;
		}
		if(value > max)
		{
			value = max;
		}
		color_saturation = value;
		printf("color_saturation: (%d,%d,%d)\n",min,max,value);
	}
	
	len = sprintf(buf,		   "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
                                   "<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\""
                                   " xmlns:timg=\"http://www.onvif.org/ver20/imaging/wsdl\""
								   " xmlns:tt=\"http://www.onvif.org/ver10/schema\">%s"
                                   "<s:Body>"
                                   "<timg:SetImagingSettings>"
								   "<timg:VideoSourceToken>%s</timg:VideoSourceToken>"
								   "<timg:ImagingSettings>"
								   "<tt:ColorSaturation>%d</tt:ColorSaturation>"
								   "</timg:ImagingSettings>"
								   "</timg:SetImagingSettings>"
                                   "</s:Body>"
								   "</s:Envelope>", wss_header,profiles.at(stream_index)->image_token,color_saturation);
	string content_type = string("application/soap+xml; charset=utf-8 ");

	
	ls_http_request request(HTTP_POST,1,1,this->xaddrs_port);
	request.setUri(this->xaddrs_uri_image.c_str(),this->xaddrs_uri_image.length());
	request.setHost(this->IP.c_str(),this->IP.length());
	request.setContentType(content_type.c_str(),content_type.length());
	request.setContent(buf,len);
	request.setHandler((HandlerProc *)&handle_image_set_color_saturation_helper,(void *)this);
	if( -1 == request.sendTo() )
			return STATUS_CONNECT_DEVICE_TIMEOUT;
	request.recvFrom();
	return this->status;
}
void device_image::handle_image_set_color_saturation_helper(ls_http_response* http_response,void *client_data)
{
	device_image *me = static_cast<device_image *>(client_data);
    me->handle_message_set_color_saturation(http_response);
}
void device_image::handle_message_set_color_saturation(ls_http_response* http_response)
{
//	lsprint("%s",http_response->response);
	/* check error */
	this->status = check_reponse_error(http_response);
    if( STATUS_PARSE_RESPONSE_ERROR != this->status )
		return;
	this->status = STATUS_ERRNO;
}
int32_t device_image::handle_image_set_contrast(uint32_t stream_index,int32_t contrast)
{
	char                wss_header[2048] = {'\0'};
	char				buf[4096]        = {'\0'};
	size_t				len;
	this->status = STATUS_ERROR;

	if( this->xaddrs_uri_image.empty() )
		handle_mngmt_capabilities();
	if( !this->image_config_support )
		return STATUS_IMAGE_CONFIG_NOT_SUPPORT;
		
	if(	profiles.empty() )
		handle_media_profiles();
	if( profiles.empty() )
		return STATUS_ERROR;
	build_wss_header(wss_header, sizeof(wss_header));
	
	int min = 0;
	int max = 100;
	if(this->max_image_setting.already_init)
	{
		min = this->max_image_setting.contrast_min;
		max = this->max_image_setting.contrast_max;
	}
	if(max >= min)
	{
		int value = contrast;
		value = min + (value) * (max - min) / (256);
		if(value < min)
		{
			value = min;
		}
		if(value > max)
		{
			value = max;
		}
		contrast = value;
		printf("contrast: (%d,%d,%d)\n",min,max,value);
	}
	
	len = sprintf(buf,		   "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
                                   "<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\""
                                   " xmlns:timg=\"http://www.onvif.org/ver20/imaging/wsdl\""
								   " xmlns:tt=\"http://www.onvif.org/ver10/schema\">%s"
                                   "<s:Body>"
                                   "<timg:SetImagingSettings>"
								   "<timg:VideoSourceToken>%s</timg:VideoSourceToken>"
								   "<timg:ImagingSettings>"
								   "<tt:Contrast>%d</tt:Contrast>"
								   "</timg:ImagingSettings>"
								   "</timg:SetImagingSettings>"
                                   "</s:Body>"
								   "</s:Envelope>", wss_header,profiles.at(stream_index)->image_token,contrast);
	string content_type = string("application/soap+xml; charset=utf-8 ");

	
	ls_http_request request(HTTP_POST,1,1,this->xaddrs_port);
	request.setUri(this->xaddrs_uri_image.c_str(),this->xaddrs_uri_image.length());
	request.setHost(this->IP.c_str(),this->IP.length());
	request.setContentType(content_type.c_str(),content_type.length());
	request.setContent(buf,len);
	request.setHandler((HandlerProc *)&handle_image_set_contrast_helper,(void *)this);
	if( -1 == request.sendTo() )
			return STATUS_CONNECT_DEVICE_TIMEOUT;
	request.recvFrom();
	return this->status;
}
void device_image::handle_image_set_contrast_helper(ls_http_response* http_response,void *client_data)
{
	device_image *me = static_cast<device_image *>(client_data);
    me->handle_message_set_contrast(http_response);
}
void device_image::handle_message_set_contrast(ls_http_response* http_response)
{
	//lsprint("%s",http_response->response);
	/* check error */
	this->status = check_reponse_error(http_response);
    if( STATUS_PARSE_RESPONSE_ERROR != this->status )
		return;
	this->status = STATUS_ERRNO;
}
int32_t device_image::handle_image_set_sharpness(uint32_t stream_index,int32_t sharpness)
{
	char                wss_header[2048] = {'\0'};
	char				buf[4096]        = {'\0'};
	size_t				len;
	this->status = STATUS_ERROR;

	if( this->xaddrs_uri_image.empty() )
		handle_mngmt_capabilities();
	if( !this->image_config_support )
		return STATUS_IMAGE_CONFIG_NOT_SUPPORT;
		
	if(	profiles.empty() )
		handle_media_profiles();
	if( profiles.empty() )
		return STATUS_ERROR;
	build_wss_header(wss_header, sizeof(wss_header));
	
	int min = 0;
	int max = 100;
	if(this->max_image_setting.already_init)
	{
		min = this->max_image_setting.sharpness_min;
		max = this->max_image_setting.sharpness_max;
	}
	if(max >= min)
	{
		int value = sharpness;
		value = min + (value) * (max - min) / (256);
		if(value < min)
		{
			value = min;
		}
		if(value > max)
		{
			value = max;
		}
		sharpness = value;
		printf("sharpness: (%d,%d,%d)\n",min,max,value);
	}
	
	len = sprintf(buf,		   "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
                                   "<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\""
                                   " xmlns:timg=\"http://www.onvif.org/ver20/imaging/wsdl\""
								   " xmlns:tt=\"http://www.onvif.org/ver10/schema\">%s"
                                   "<s:Body>"
                                   "<timg:SetImagingSettings>"
								   "<timg:VideoSourceToken>%s</timg:VideoSourceToken>"
								   "<timg:ImagingSettings>"
								   "<tt:Sharpness>%d</tt:Sharpness>"
								   "</timg:ImagingSettings>"
								   "</timg:SetImagingSettings>"
                                   "</s:Body>"
								   "</s:Envelope>", wss_header,profiles.at(stream_index)->image_token,sharpness);
	string content_type = string("application/soap+xml; charset=utf-8 ");

	
	ls_http_request request(HTTP_POST,1,1,this->xaddrs_port);
	request.setUri(this->xaddrs_uri_image.c_str(),this->xaddrs_uri_image.length());
	request.setHost(this->IP.c_str(),this->IP.length());
	request.setContentType(content_type.c_str(),content_type.length());
	request.setContent(buf,len);
	request.setHandler((HandlerProc *)&handle_image_set_sharpness_helper,(void *)this);
	if( -1 == request.sendTo() )
			return STATUS_CONNECT_DEVICE_TIMEOUT;
	request.recvFrom();
	return this->status;
}
void device_image::handle_image_set_sharpness_helper(ls_http_response* http_response,void *client_data)
{
	device_image *me = static_cast<device_image *>(client_data);
    me->handle_message_set_sharpness(http_response);
}
void device_image::handle_message_set_sharpness(ls_http_response* http_response)
{
	//lsprint("%s",http_response->response);
	/* check error */
	this->status = check_reponse_error(http_response);
    if( STATUS_PARSE_RESPONSE_ERROR != this->status )
		return;
	this->status = STATUS_ERRNO;
}
int32_t device_image::handle_image_set_backlight_compensation(uint32_t stream_index,Boolean mode)
{
	char                wss_header[2048] = {'\0'};
	char				buf[4096]        = {'\0'};
	size_t				len;
	this->status = STATUS_ERROR;

	if( this->xaddrs_uri_image.empty() )
		handle_mngmt_capabilities();
	if( !this->image_config_support )
		return STATUS_IMAGE_CONFIG_NOT_SUPPORT;
		
	if(	profiles.empty() )
		handle_media_profiles();
	if( profiles.empty() )
		return STATUS_ERROR;
	build_wss_header(wss_header, sizeof(wss_header));
	
	len = sprintf(buf,		   "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
                                   "<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\""
                                   " xmlns:timg=\"http://www.onvif.org/ver20/imaging/wsdl\""
								   " xmlns:tt=\"http://www.onvif.org/ver10/schema\">%s"
                                   "<s:Body>"
                                   "<timg:SetImagingSettings>"
								   "<timg:VideoSourceToken>%s</timg:VideoSourceToken>"
								   "<timg:ImagingSettings>"
								   "<tt:BacklightCompensation>"
								   "<tt:Mode>%s</tt:Mode>"
								   "<tt:Level>1</tt:Level>"
								   "</tt:BacklightCompensation>"
								   "</timg:ImagingSettings>"
								   "</timg:SetImagingSettings>"
                                   "</s:Body>"
								   "</s:Envelope>", wss_header,profiles.at(stream_index)->image_token,mode?"ON":"OFF");
	string content_type = string("application/soap+xml; charset=utf-8 ");

	
	ls_http_request request(HTTP_POST,1,1,this->xaddrs_port);
	request.setUri(this->xaddrs_uri_image.c_str(),this->xaddrs_uri_image.length());
	request.setHost(this->IP.c_str(),this->IP.length());
	request.setContentType(content_type.c_str(),content_type.length());
	request.setContent(buf,len);
	request.setHandler((HandlerProc *)&handle_image_set_backlight_compensation_helper,(void *)this);
	if( -1 == request.sendTo() )
			return STATUS_CONNECT_DEVICE_TIMEOUT;
	request.recvFrom();
	return this->status;
}
void device_image::handle_image_set_backlight_compensation_helper(ls_http_response* http_response,void *client_data)
{
	device_image *me = static_cast<device_image *>(client_data);
    me->handle_message_set_backlight_compensation(http_response);
}
void device_image::handle_message_set_backlight_compensation(ls_http_response* http_response)
{
		//lsprint("%s",http_response->response);
	/* check error */
	this->status = check_reponse_error(http_response);
    if( STATUS_PARSE_RESPONSE_ERROR != this->status )
		return;
	this->status = STATUS_ERRNO;
}
