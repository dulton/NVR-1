#include "ls_receiver_media.h"
#include <algorithm>
#include <string.h>
#include "ls_string.h"

#include <iostream>
using namespace std;

media_profile::media_profile()
{
	memset(stream_token,0,64);
	memset(image_token,0,64);
	memset(ptz_token,0,64);
}
device_media::device_media()
{
	this->ptz_supported = SUPPORT_NOT_INIT;
	this->image_supported = SUPPORT_NOT_INIT;
}
device_media::~device_media()
{
	//lsprint("profiles:%d\n.stream_uris:%d\n",profiles.size(),stream_uris.size());
	while (!profiles.empty())
	{
		delete profiles.back();
		profiles.pop_back();
	}
	while( !stream_uris.empty() )
	{
		delete stream_uris.back();
		stream_uris.pop_back();
	}
}
vector<std::string *> *device_media::get_stream_uri()
{
	if( this->stream_uris.empty() )
		handle_media_stream_uri();
	return &(this->stream_uris);
}
vector<media_profile *> *device_media::get_profiles()
{
	if( this->profiles.empty() )
		handle_media_profiles();
	return &(this->profiles);
}
bool onvif_media_profile_cmp(media_profile* i, media_profile* j)
{
    if(i->resolution_width != j->resolution_width)
        return (i->resolution_width > j->resolution_width);
    else if(i->resolution_height != j->resolution_height)
        return (i->resolution_height > j->resolution_height);
    else if(i->bitrate_limit != j->bitrate_limit)
        return (i->bitrate_limit > j->bitrate_limit);
    else if(i->with_audio != j->with_audio)
        return (i->with_audio > j->with_audio);
    else if(i->sequence != j->sequence)
        return (i->sequence < j->sequence);
    else
        return true;
}

int32_t device_media::handle_media_stream_uri()
{
	char                wss_header[2048] = {'\0'};
	char				buf[4096]        = {'\0'};
	
	this->status = STATUS_ERROR;
	
	if(profiles.empty() )
		handle_media_profiles();
	//if we do not get profiles we must return
	if(profiles.empty() )
		return STATUS_ERROR;
	
	for(vector<media_profile *>::iterator it = profiles.begin(); it != profiles.end(); it++)
	{
		build_wss_header(wss_header, sizeof(wss_header));
		size_t len = 0;
		
		//printf("handle_media_stream_uri-0\n");
		
		#if 1
		//csp modify
		len = sprintf(buf,				"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
										"<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\""
										" xmlns:trt=\"http://www.onvif.org/ver10/media/wsdl\""
										" xmlns:tt=\"http://www.onvif.org/ver10/schema\">%s"
										"<s:Body xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\">"
										"<GetStreamUri xmlns=\"http://www.onvif.org/ver10/media/wsdl\">"
										"<StreamSetup>"
										"<Stream xmlns=\"http://www.onvif.org/ver10/schema\">RTP-Unicast</Stream>"
										"<Transport xmlns=\"http://www.onvif.org/ver10/schema\">"
										"<Protocol>RTSP</Protocol>"
										"</Transport>"
										"</StreamSetup>"
										"<ProfileToken>%s</ProfileToken>"
										"</GetStreamUri>"
										"</s:Body>"
										"</s:Envelope>", wss_header,(*it)->stream_token);
		#else
		//csp modify
		len = sprintf(buf,		        "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
										"<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\""
										" xmlns:trt=\"http://www.onvif.org/ver10/media/wsdl\""
										" xmlns:tt=\"http://www.onvif.org/ver10/schema\">%s"
										"<s:Body>"
										"<GetStreamUri xmlns=\"http://www.onvif.org/ver10/media/wsdl\">"
										"<StreamSetup>"
										"<Stream xmlns=\"http://www.onvif.org/ver10/schema\">RTP-Unicast</Stream>"
										"<Transport xmlns=\"http://www.onvif.org/ver10/schema\">"
										"<Protocol>UDP</Protocol>"
										"</Transport>"
										"</StreamSetup>"
										"<ProfileToken>%s</ProfileToken>"
										"</GetStreamUri>"
										"</s:Body>"
										"</s:Envelope>", wss_header,(*it)->stream_token);
		#endif
		string content_type = string("application/soap+xml; charset=utf-8; action=\"http://www.onvif.org/ver10/media/wsdl/GetStreamUri\"");
		
		//get peer port
		set_xaddrs();
		//if xaddrs_uri_media do not exists!!! we get it!
		if(xaddrs_uri_media.empty())
		{
			//xaddrs_uri_media.assign("/onvif/media_service");
			handle_mngmt_capabilities();
			
			//csp modify
			if(xaddrs_uri_media.empty())
			{
				return this->status;
			}
		}
		
		//printf("handle_media_stream_uri-1\n");
		
		ls_http_request request(HTTP_POST,1,1,this->xaddrs_port);
		request.setUri(this->xaddrs_uri_media.c_str(),this->xaddrs_uri_media.length());
		request.setHost(this->IP.c_str(),this->IP.length());
		request.setContentType(content_type.c_str(),content_type.length());
		request.setContent(buf,len);
		request.setHandler((HandlerProc *)&handle_media_stream_uri_help,(void *)this);
		
		//printf("handle_media_stream_uri-2\n");
		
		if( -1 == request.sendTo() )
			return STATUS_CONNECT_DEVICE_TIMEOUT;
		
		//printf("handle_media_stream_uri-3\n");
		
		request.recvFrom();
		
		//printf("handle_media_stream_uri-4\n");
	}
	return this->status;
}
void device_media::handle_media_stream_uri_help(ls_http_response* http_response,void *client_data)
{
	device_media *me = static_cast<device_media *>(client_data);
    me->handle_message_stream_uri(http_response);
}
void device_media::handle_message_stream_uri(ls_http_response* http_response)
{
	xmlDocPtr           doc = NULL;
	xmlNodePtr          node = NULL;
	xmlXPathContextPtr  xpath_context = NULL;
	xmlXPathObjectPtr   xpath_object = NULL;
	xmlChar				*p = NULL;
	int                 i;
	//char				uri[512];
	//char				*p1 = NULL, *p2 = NULL;
	//string				*stream_uri;
	
	//lsprint("handle_message_stream_uri:%s",http_response->response);
	//printf("%s 1\n", __func__);
	this->status = check_reponse_error(http_response);
	//printf("%s 2\n", __func__);
	//printf("\n handle_message_stream_uri:%s\n",http_response->response);
	//printf("respone_content_len: %d\n", http_response->respone_content_len);
    if( STATUS_PARSE_RESPONSE_ERROR != this->status )
		return;
	
	/* parse XML */
	do
	{
		if(NULL == (doc = xmlParseMemory(http_response->respone_content, http_response->respone_content_len))) 
		{
			//printf("%s 3\n", __func__);
			//printf("\n\n handle_message_stream_uri:%s\n\n\n",http_response->response);
			break;			
		}
		if(NULL == (xpath_context = xmlXPathNewContext(doc))) 
		{			
			//printf("%s 4\n", __func__);
			break;
		}
		if(0 != register_onvif_namespaces(xpath_context)) 
		{
			//printf("%s 5\n", __func__);
			break;			
		}
		if(NULL == (xpath_object = xmlXPathEvalExpression(BAD_CAST"/SOAP-ENV:Envelope/SOAP-ENV:Body/trt:GetStreamUriResponse/trt:MediaUri/tt:Uri", xpath_context))) 
		{
			//printf("%s 6\n", __func__);
			break;			
		}
			
		if(xmlXPathNodeSetIsEmpty(xpath_object->nodesetval)) 
		{
			//printf("%s 7\n", __func__);
			break;			
		}

		/* get uri */
		for(i = 0; i < xpath_object->nodesetval->nodeNr; i++)
		{
			if(XML_ELEMENT_NODE == xpath_object->nodesetval->nodeTab[i]->type)
			{
				node = xpath_object->nodesetval->nodeTab[i];            
				break;
			}
		}
		if(NULL == node) break;
		if(NULL == (p = xmlNodeGetContent(node))) 
		{
			//printf("%s 8\n", __func__);
			break;			
		}
		//strncpy(uri, (const char *)p, sizeof(uri));
		
		//printf("%s 9\n", __func__);
		stream_uris.push_back(new string( (const char *)p, strlen( (const char *)p ) ));
		//printf("%s 10\n", __func__);
		//lsprint("Receiver_onvif, receive stream_url: %s\n",(const char *)p);
		xmlFree(p);
		
		
		/* parse rtsp url suffix. format: rtsp://username:password@192.168.1.100:554/suffix/more_suffix 
		if(NULL == (p1 = strstr(uri, "rtsp://"))) break;
		p1 += 7;
		if(NULL == (p2 = strchr(p1, '/'))) break;
		*p2 = '\0';
		if(NULL != (p1 = strrchr(p1, ':')))
		{
			if(NULL == strchr(p1, '@'))
			{
				p1 += 1;
				//strncpy(this->peer_control_serv, p1, sizeof(this->peer_control_serv));
			}
		}
		p1 = p2 + 1;
		if(NULL != (p2 = strchr(p1, '?'))) *p2 = '\0';
		*/
		/* save rtsp url suffix */
		//strncpy(this->rtsp_url_suffix, p1, sizeof(rtsp_url_suffix));
		//lsprint( "Receiver_onvif, rtsp_port:%s, rtsp_url_suffix: %s.\n",
		//	this->peer_control_serv, rtsp_url_suffix);
		
		this->status = STATUS_ERRNO;
	}while(0);

	//printf("%s 11\n", __func__);
	if(NULL != xpath_object) xmlXPathFreeObject(xpath_object);
	if(NULL != xpath_context) xmlXPathFreeContext(xpath_context);
	if(NULL != doc) xmlFreeDoc(doc);
	xmlCleanupParser();
	return;
}
int32_t device_media::handle_media_profiles()
{
	char                wss_header[2048] = {'\0'};
	char				buf[4096]        = {'\0'};
	
	//printf("handle_media_profiles-0\n");
	
	this->status = STATUS_ERROR;
	build_wss_header(wss_header, sizeof(wss_header));
	size_t len = 0;
	
	#if 1
	//csp modify
	len = sprintf(buf,             "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
                                   "<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\">%s"
                                   //" xmlns:trt=\"http://www.onvif.org/ver10/media/wsdl\""
                                   //" xmlns:tt=\"http://www.onvif.org/ver10/schema\">%s"
                                   "<s:Body xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\">"
                                   "<GetProfiles xmlns=\"http://www.onvif.org/ver10/media/wsdl\"/>"
                                   "</s:Body>"
                                   "</s:Envelope>", wss_header);
	#else
	//csp modify
	len = sprintf(buf,		       "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
                                   "<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\""
                                   " xmlns:trt=\"http://www.onvif.org/ver10/media/wsdl\""
                                   " xmlns:tt=\"http://www.onvif.org/ver10/schema\">%s"
                                   "<s:Body>"
                                   "<trt:GetProfiles/>"
                                   "</s:Body>"
                                   "</s:Envelope>", wss_header);
	#endif
	string content_type = string("application/soap+xml; charset=utf-8; action=\"http://www.onvif.org/ver10/media/wsdl/GetProfiles\"");
	
	//get peer port
	set_xaddrs();
	//printf("handle_media_profiles-1\n");
	//if xaddrs_uri_media do not exists!!! we get it!
	if(xaddrs_uri_media.empty())
	{
		//xaddrs_uri_media.assign("/onvif/media_service");
		handle_mngmt_capabilities();
		
		//csp modify
		if(xaddrs_uri_media.empty())
		{
			return this->status;
		}
	}
	ls_http_request request(HTTP_POST,1,1,this->xaddrs_port);
	request.setUri(this->xaddrs_uri_media.c_str(),this->xaddrs_uri_media.length());
	request.setHost(this->IP.c_str(),this->IP.length());
	request.setContentType(content_type.c_str(),content_type.length());
	request.setContent(buf,len);
	request.setHandler((HandlerProc *)&handle_media_profiles_help,(void *)this);
	//printf("handle_media_profiles-1\n");
	if( -1 == request.sendTo() )
	{
		//printf("handle_media_profiles-2\n");
		return STATUS_CONNECT_DEVICE_TIMEOUT;
	}
	//printf("handle_media_profiles-3\n");
	request.recvFrom();
	//printf("handle_media_profiles-4\n");
	return this->status;
}
void device_media::handle_media_profiles_help(ls_http_response* http_response,void *client_data)
{
	device_media *me = static_cast<device_media *>(client_data);
    me->handle_message_profiles(http_response);
}
void device_media::handle_message_profiles(ls_http_response* http_response)
{
	media_profile                  *profile = NULL;
	uint32_t                       sequence = 0;
	xmlDocPtr                      doc = NULL;
	xmlNodePtr                     node = NULL, node2 = NULL, node3 = NULL, node4 = NULL;
	xmlXPathContextPtr             xpath_context = NULL;
	xmlXPathObjectPtr              xpath_object = NULL;
	xmlChar                        *p = NULL;
	char                           p_buf[1024];
	int                            i;
	
	this->ptz_supported = NOT_SUPPORT;
	this->image_supported = NOT_SUPPORT;
	
	//lsprint("handle_message_profiles:%s",http_response->response);
	
	/* check error */
	this->status = check_reponse_error(http_response);
    if( STATUS_PARSE_RESPONSE_ERROR != this->status )
		return;
	
	//cout<<"\n\n\n$$$handle_message_profiles content:"<<http_response->respone_content<<"\n\n\n"<<endl;
	//fflush(stdout);
	//cout<<"\n\n\n^^^handle_message_profiles respone:"<<http_response->response<<"\n\n\n"<<endl;
	//fflush(stdout);
	
	/* parse XML, save all profiles to a vector */
	do
	{
		//lsprint("%s",http_response->respone_content);
		
		if(NULL == (doc = xmlParseMemory(http_response->respone_content,http_response->respone_content_len))) break;
		if(NULL == (xpath_context = xmlXPathNewContext(doc))) break;
		if(0 != register_onvif_namespaces(xpath_context)) break;
		if(NULL == (xpath_object = xmlXPathEvalExpression(BAD_CAST"/SOAP-ENV:Envelope/SOAP-ENV:Body/trt:GetProfilesResponse/trt:Profiles", xpath_context))) break;
		if(xmlXPathNodeSetIsEmpty(xpath_object->nodesetval)) break;
		
		//printf("nodeNr=%d\n",xpath_object->nodesetval->nodeNr);
		
		for(i = 0; i < xpath_object->nodesetval->nodeNr; i++)
		{
			profile = new media_profile();
			
			//csp modify
			if(profile == NULL)
			{
				break;
			}
			
			node = xpath_object->nodesetval->nodeTab[i];
			if(XML_ELEMENT_NODE != node->type)
			{
				//csp modify
				delete profile;
				profile = NULL;
				
				continue;
			}
			
			/* get token */
			if(NULL == (p = xmlGetNoNsProp(node, BAD_CAST"token")))
			{
				//csp modify
				delete profile;
				profile = NULL;
				
				continue;
			}
			strncpy(profile->stream_token, (const char *)p, sizeof(profile->stream_token));
			xmlFree(p);
			p = NULL;
			
			if(0 == strlen(profile->stream_token))
			{
				//csp modify
				delete profile;
				profile = NULL;
				
				continue;
			}
			
			//printf("index=%d,stream_token=%s\n",i,profile->stream_token);
			
			/* get resolution_width */
			if(NULL == (node2 = seek_child_node_by_name(node, BAD_CAST"VideoEncoderConfiguration")))
			{
				//csp modify
				delete profile;
				profile = NULL;
				
				continue;
			}
			if(NULL == (node3 = seek_child_node_by_name(node2, BAD_CAST"Resolution")))
			{
				//csp modify
				delete profile;
				profile = NULL;
				
				continue;
			}
			if(NULL == (node4 = seek_child_node_by_name(node3, BAD_CAST"Width")))
			{
				//csp modify
				delete profile;
				profile = NULL;
				
				continue;
			}
			if(NULL == (p = xmlNodeGetContent(node4)))
			{
				//csp modify
				delete profile;
				profile = NULL;
				
				continue;
			}
			strncpy(p_buf, (const char *)p, sizeof(p_buf));
			xmlFree(p);
			p = NULL;
			
			if(0 != ls_str_to_uint32(p_buf, &(profile->resolution_width)))
			{
				//csp modify
				delete profile;
				profile = NULL;
				
				continue;
			}

			/* get resolution_height */
			if(NULL == (node4 = seek_child_node_by_name(node3, BAD_CAST"Height")))
			{
				//csp modify
				delete profile;
				profile = NULL;
				
				continue;
			}
			if(NULL == (p = xmlNodeGetContent(node4)))
			{
				//csp modify
				delete profile;
				profile = NULL;
				
				continue;
			}
			strncpy(p_buf, (const char *)p, sizeof(p_buf));
			xmlFree(p);
			p = NULL;
			
			if(0 != ls_str_to_uint32(p_buf, &(profile->resolution_height)))
			{
				//csp modify
				delete profile;
				profile = NULL;
				
				continue;
			}
			
			/* get framerate_limit */
			if(NULL == (node3 = seek_child_node_by_name(node2, BAD_CAST"RateControl")))
			{
				//csp modify
				delete profile;
				profile = NULL;
				
				continue;
			}
			if(NULL == (node4 = seek_child_node_by_name(node3, BAD_CAST"FrameRateLimit")))
			{
				//csp modify
				delete profile;
				profile = NULL;
				
				continue;
			}
			if(NULL == (p = xmlNodeGetContent(node4)))
			{
				//csp modify
				delete profile;
				profile = NULL;
				
				continue;
			}
			strncpy(p_buf, (const char *)p, sizeof(p_buf));
			xmlFree(p);
			p = NULL;
			
			if(0 != ls_str_to_uint32(p_buf, &(profile->framerate_limit)))
			{
				//csp modify
				delete profile;
				profile = NULL;
				
				continue;
			}

			/* get bitrate_limit */
			if(NULL == (node4 = seek_child_node_by_name(node3, BAD_CAST"BitrateLimit")))
			{
				//csp modify
				delete profile;
				profile = NULL;
				
				continue;
			}
			if(NULL == (p = xmlNodeGetContent(node4)))
			{
				//csp modify
				delete profile;
				profile = NULL;
				
				continue;
			}
			strncpy(p_buf, (const char *)p, sizeof(p_buf));
			xmlFree(p);
			p = NULL;
			
			if(0 != ls_str_to_uint32(p_buf, &(profile->bitrate_limit)))
			{
				//csp modify
				delete profile;
				profile = NULL;
				
				continue;
			}
			
			/* check if with audio */
			profile->with_audio = (NULL == seek_child_node_by_name(node, BAD_CAST"AudioEncoderConfiguration") ? False : True);
			
			/* get ptz_config_token */
			//profile->ptz_token[0] = '\0';
			if(NULL != (node2 = seek_child_node_by_name(node, BAD_CAST"PTZConfiguration")))
			{
				if(NULL != (p = xmlGetNoNsProp(node2, BAD_CAST"token")))
				{
					strncpy(profile->ptz_token, (const char *)p, sizeof(profile->ptz_token));
					this->ptz_supported = SUPPORT;
					xmlFree(p);
					p = NULL;
				}
				//printf("ptz_config_token:%s\n",profile.ptz_config_token);
			}
			if(NULL != (node2 = seek_child_node_by_name(node, BAD_CAST"VideoSourceConfiguration")))
			{
				if(NULL == (node3 = seek_child_node_by_name(node2, BAD_CAST"SourceToken")))
				{
					//csp modify
					delete profile;
					profile = NULL;
					
					continue;
				}
				if(NULL == (p = xmlNodeGetContent(node3)))
				{
					//csp modify
					delete profile;
					profile = NULL;
					
					continue;
				}
				strncpy(profile->image_token, (const char *)p, sizeof(profile->image_token));
				//printf("image_token:%s\n",profile->image_token);
				this->image_supported = SUPPORT;
				xmlFree(p);
				p = NULL;
			}
			
			/* set sequence */
			profile->sequence = sequence++;
			
			/* save this profile */
			profiles.push_back(profile);
			profile = NULL;
		}
		
		if( profiles.empty() ) break;
		
		/* do sort for the vector */
		//sort(profiles.begin(), profiles.end(), onvif_media_profile_cmp);//csp modify

		/* ignore useless item */
		/*profile->token[0] = '\0';
		for(vector<onvif_media_profile_t *>::iterator it = profiles.begin(); it != profiles.end();)
		{
			if('\0' != profile->token[0] &&
				(*it)->resolution_width == profile->resolution_width &&
				(*it)->resolution_height == profile->resolution_height &&
				(*it)->bitrate_limit == profile->bitrate_limit)
			{
				it = profiles.erase(it);
			}
			else
			{
				strncpy(profile->token, (*it)->token, sizeof(profile->token));
				profile->resolution_width = (*it)->resolution_width;
				profile->resolution_height = (*it)->resolution_height;
				profile->bitrate_limit = (*it)->bitrate_limit;
				it++;
			}
		}*/
		/*for(vector<media_profile *>::iterator it = profiles.begin(); it != profiles.end(); it++)
		{
			lsprint("Receiver_onvif, stream token: %s (%u * %u, %ukbit/s, %ufps, audio:%s).\n",
				(*it)->stream_token, (*it)->resolution_width, (*it)->resolution_height, (*it)->bitrate_limit, (*it)->framerate_limit,
				(1 == (*it)->with_audio ? "ON" : "OFF"));
		}*/


		/* select profile according to receiver-config-item: "stream" */
	//	if(stream >= profiles.size()) break;
	//	strncpy(this->profile_token, profiles[stream].token, sizeof(this->profile_token));

		/* save the frame rate from onvif */
	//	if(0 == this->frame_rate) this->frame_rate = profiles[stream].framerate_limit;

		/* check ptz_supported, save the ptz_config_token */
	//	if(!profiles.empty() &&  ( '\0' != profiles[0]->ptz_config_token[0] ))
	//	{
			//strncpy(this->ptz_config_token, profiles[stream].ptz_config_token, sizeof(this->ptz_config_token));
			//this->ptz_config_token.assign(profiles.at(0)->ptz_config_token );		
		//}
		//else
		//{
			//this->ptz_config_token[0] = '\0';
		//}
		this->status = STATUS_ERRNO;
	//	lsprint("ptz_supported:%s\n", ( this->ptz_supported == SUPPORT ? "yes":"no" ));
		/* update status */
		/*if(this->ptz_supported)
		this->status = VAS_RECEIVER_ONVIF_WAIT_PTZ_CONFIG_OPTIONS;
		else
		this->status = VAS_RECEIVER_ONVIF_WAIT_STREAM_URL;
		*/
		/* do TCP connect */
		//ls_tcp_client_connect(this->tcp_client_xaddrs);

		//end:
		//profiles.clear();
	}while(0);
	
	//if error reclaim the profile
	if(profile != NULL) delete profile;
	if(NULL != xpath_object) xmlXPathFreeObject(xpath_object);
	if(NULL != xpath_context) xmlXPathFreeContext(xpath_context);
	if(NULL != doc) xmlFreeDoc(doc);
	xmlCleanupParser();
	
	return;
}

