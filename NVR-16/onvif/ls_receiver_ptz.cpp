#include "ls_receiver_ptz.h"
#include <string.h>
#include "ls_string.h"

#define PTZ_MOVE_TIMEOUT_S 30
#define PTZ_TIMEOUT_S      4

#define PTZ_CMDS_START_1   "<?xml version=\"1.0\" encoding=\"utf-8\"?>"\
	"<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\""\
	" xmlns:tptz=\"http://www.onvif.org/ver20/ptz/wsdl\""\
	" xmlns:tt=\"http://www.onvif.org/ver10/schema\">"
#define PTZ_CMDS_START_2   "<s:Body>"
#define PTZ_CMDS_END       "</s:Body></s:Envelope>"


device_ptz::device_ptz()
{

	/* Using default value instead of setting. */
	this->ptz_config_options_already_done   = false;
	this->ptz_pt_xrange_min  = -1.0;
    this->ptz_pt_xrange_max  = 1.0;
    this->ptz_pt_yrange_min  = -1.0;
    this->ptz_pt_yrange_max  = 1.0;
    this->ptz_z_xrange_min   = -1.0;
    this->ptz_z_xrange_max   = 1.0;
    this->ptz_move_timeout_s = PTZ_MOVE_TIMEOUT_S;
}
device_ptz::~device_ptz()
{
}
Boolean device_ptz::handle_ptz_support()
{
	if(	profiles.empty() )
		handle_media_profiles();
	if( profiles.empty() || ptz_supported == NOT_SUPPORT || ptz_supported == SUPPORT_NOT_INIT)
		return False;
	return True;
}
uint32_t device_ptz::parse_time(char *time_str, uint32_t *time_s)
{
    uint32_t h = 0, m = 0, s = 0;
    char *p1 = NULL, *p2 = NULL, *p_h = NULL, *p_m = NULL, *p_s = NULL;

    if(NULL == time_str || NULL == time_s) return 1;
    if('P' != time_str[0] || 'T' != time_str[1]) return 1;
    p1 = time_str + 2;

    if(NULL != (p2 = strchr(p1, 'H')))
    {
        *p2 = '\0';
        p_h = p1;
        p1 = p2 + 1;
    }
    if(NULL != (p2 = strchr(p1, 'M')))
    {
        *p2 = '\0';
        p_m = p1;
        p1 = p2 + 1;
    }
    if(NULL != (p2 = strchr(p1, 'S')))
    {
        *p2 = '\0';
        p_s = p1;
    }

    if(NULL != p_h) ls_str_to_uint32(p_h, &h);
    if(NULL != p_m) ls_str_to_uint32(p_m, &m);
    if(NULL != p_s) ls_str_to_uint32(p_s, &s);

    *time_s = (3600 * h) + (60 * m) + s;

    return 0;
}
int32_t device_ptz::handle_ptz_config_options()
{
	char                wss_header[2048] = {'\0'};
	char				buf[4096]        = {'\0'};
	
	this->status = STATUS_ERROR;
	
	if(	this->ptz_supported == SUPPORT_NOT_INIT  )
		handle_media_profiles();
	//not support
	if(	this->ptz_supported != SUPPORT)
		return STATUS_ERROR;
	build_wss_header(wss_header, sizeof(wss_header));
	size_t len = 0;
	
	//csp modify
	len = sprintf(buf,		   "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
                                   "<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\""
                                   " xmlns:tptz=\"http://www.onvif.org/ver20/ptz/wsdl\">%s"
                                   "<s:Body>"
                                   "<tptz:GetConfigurationOptions>"
                                   "<tptz:ConfigurationToken>%s</tptz:ConfigurationToken>"
                                   "</tptz:GetConfigurationOptions>"
                                   "</s:Body>"
								   "</s:Envelope>", wss_header,profiles.at(0)->ptz_token);
	string content_type = string("application/soap+xml; charset=utf-8; action=\"http://www.onvif.org/ver20/ptz/wsdl/GetConfigurationOptions\"");
	
	if(this->xaddrs_uri.empty())
		set_xaddrs();
	if( this->xaddrs_uri_ptz.empty() )
		handle_mngmt_capabilities();
	ls_http_request request(HTTP_POST,1,1,this->xaddrs_port);
	request.setUri(this->xaddrs_uri_ptz.c_str(),this->xaddrs_uri_ptz.length());
	request.setHost(this->IP.c_str(),this->IP.length());
	request.setContentType(content_type.c_str(),content_type.length());
	request.setContent(buf,len);
	request.setHandler((HandlerProc *)&handle_ptz_config_options_helper,(void *)this);
	if( -1 == request.sendTo() )
			return STATUS_CONNECT_DEVICE_TIMEOUT;
	request.recvFrom();
	return this->status;
}
void device_ptz::handle_ptz_config_options_helper(ls_http_response* http_response,void *client_data)
{
	device_ptz *me = static_cast<device_ptz *>(client_data);
    me->handle_message_ptz_config_options(http_response);
}
void device_ptz::handle_message_ptz_config_options(ls_http_response* http_response)
{
	xmlDocPtr           doc = NULL;
    xmlNodePtr          node = NULL, node2 = NULL, node3 = NULL, node4 = NULL, node5 = NULL;
    xmlXPathContextPtr  xpath_context = NULL;
    xmlXPathObjectPtr   xpath_object = NULL;
    xmlChar            *p = NULL;
    char                p_buf[1024];
    int                 i;
    uint32_t            ptz_move_timeout_s_min = 0, ptz_move_timeout_s_max = 0;

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
		if(NULL == (xpath_object = xmlXPathEvalExpression(BAD_CAST"/SOAP-ENV:Envelope/SOAP-ENV:Body/tptz:GetConfigurationOptionsResponse/tptz:PTZConfigurationOptions", xpath_context))) break;
		if(xmlXPathNodeSetIsEmpty(xpath_object->nodesetval)) break;

		/* get the first "PTZConfigurationOptions" */
		for(i = 0; i < xpath_object->nodesetval->nodeNr; i++)
		{
			if(XML_ELEMENT_NODE == xpath_object->nodesetval->nodeTab[i]->type)
			{
				node = xpath_object->nodesetval->nodeTab[i];            
				break;
			}
		}
		if(NULL == node) break;

		/* get PanTilt XRange Min */
		if(NULL == (node2 = seek_child_node_by_name(node, BAD_CAST"Spaces"))) break;
		if(NULL == (node3 = seek_child_node_by_name(node2, BAD_CAST"ContinuousPanTiltVelocitySpace"))) break;
		if(NULL == (node4 = seek_child_node_by_name(node3, BAD_CAST"XRange"))) break;
		if(NULL == (node5 = seek_child_node_by_name(node4, BAD_CAST"Min"))) break;
		if(NULL == (p = xmlNodeGetContent(node5))) break;
		strncpy(p_buf, (const char *)p, sizeof(p_buf));
		xmlFree(p);
		if(0 != ls_str_to_double(p_buf, &ptz_pt_xrange_min)) break;

		/* get PanTilt XRange Max */
		if(NULL == (node5 = seek_child_node_by_name(node4, BAD_CAST"Max"))) break;
		if(NULL == (p = xmlNodeGetContent(node5))) break;
		strncpy(p_buf, (const char *)p, sizeof(p_buf));
		xmlFree(p);
		if(0 != ls_str_to_double(p_buf, &ptz_pt_xrange_max)) break;

		/* get PanTilt YRange Min */
		if(NULL == (node4 = seek_child_node_by_name(node3, BAD_CAST"YRange"))) break;
		if(NULL == (node5 = seek_child_node_by_name(node4, BAD_CAST"Min"))) break;
		if(NULL == (p = xmlNodeGetContent(node5))) break;
		strncpy(p_buf, (const char *)p, sizeof(p_buf));
		xmlFree(p);
		if(0 != ls_str_to_double(p_buf, &ptz_pt_yrange_min)) break;

		/* get PanTilt YRange Max */
		if(NULL == (node5 = seek_child_node_by_name(node4, BAD_CAST"Max"))) break;
		if(NULL == (p = xmlNodeGetContent(node5))) break;
		strncpy(p_buf, (const char *)p, sizeof(p_buf));
		xmlFree(p);
		if(0 != ls_str_to_double(p_buf, &ptz_pt_yrange_max)) break;

		/* get Zoom XRange Min */
		if(NULL == (node3 = seek_child_node_by_name(node2, BAD_CAST"ContinuousZoomVelocitySpace"))) break;
		if(NULL == (node4 = seek_child_node_by_name(node3, BAD_CAST"XRange"))) break;
		if(NULL == (node5 = seek_child_node_by_name(node4, BAD_CAST"Min"))) break;
		if(NULL == (p = xmlNodeGetContent(node5))) break;
		strncpy(p_buf, (const char *)p, sizeof(p_buf));
		xmlFree(p);
		if(0 != ls_str_to_double(p_buf, &ptz_z_xrange_min)) break;

		/* get Zoom XRange Max */
		if(NULL == (node5 = seek_child_node_by_name(node4, BAD_CAST"Max"))) break;
		if(NULL == (p = xmlNodeGetContent(node5))) break;
		strncpy(p_buf, (const char *)p, sizeof(p_buf));
		xmlFree(p);
		if(0 != ls_str_to_double(p_buf, &ptz_z_xrange_max)) break;

		/* get ptz move timeout Min */
		if(NULL == (node2 = seek_child_node_by_name(node, BAD_CAST"PTZTimeout"))) break;
		if(NULL == (node3 = seek_child_node_by_name(node2, BAD_CAST"Min"))) break;
		if(NULL == (p = xmlNodeGetContent(node3))) break;
		strncpy(p_buf, (const char *)p, sizeof(p_buf));
		xmlFree(p);
		if(0 != parse_time(p_buf, &ptz_move_timeout_s_min)) break;

		/* get ptz move timeout Max */
		if(NULL == (node3 = seek_child_node_by_name(node2, BAD_CAST"Max"))) break;
		if(NULL == (p = xmlNodeGetContent(node3))) break;
		strncpy(p_buf, (const char *)p, sizeof(p_buf));
		xmlFree(p);
		if(0 != parse_time(p_buf, &ptz_move_timeout_s_max)) break;

		/* save the rational ptz-move-timeout-s */
		if(ptz_move_timeout_s_min <= PTZ_MOVE_TIMEOUT_S && PTZ_MOVE_TIMEOUT_S <= ptz_move_timeout_s_max)
			this->ptz_move_timeout_s = PTZ_MOVE_TIMEOUT_S;
		else if(PTZ_MOVE_TIMEOUT_S < ptz_move_timeout_s_min)
			this->ptz_move_timeout_s = ptz_move_timeout_s_min;
		else if(PTZ_MOVE_TIMEOUT_S > ptz_move_timeout_s_max)
			this->ptz_move_timeout_s = ptz_move_timeout_s_max;
		else
			this->ptz_move_timeout_s = PTZ_MOVE_TIMEOUT_S;
		this->status = STATUS_ERRNO;
	}while(0);
 
    //lsprint( "Receiver_onvif, ptz config: (speed: %.1f, %.1f, %.1f, %.1f, %.1f, %.1f. timeout: %u). \n",
    //        ptz_pt_xrange_min, ptz_pt_xrange_max, ptz_pt_yrange_min, ptz_pt_yrange_max,
    //        ptz_z_xrange_min, ptz_z_xrange_max, ptz_move_timeout_s);

    this->ptz_config_options_already_done   = true;
    if(NULL != xpath_object) xmlXPathFreeObject(xpath_object);
    if(NULL != xpath_context) xmlXPathFreeContext(xpath_context);
    if(NULL != doc) xmlFreeDoc(doc);
    xmlCleanupParser();
	
    return;


  
}

int32_t device_ptz::do_ptz(uint32_t stream_index,uint32_t command, uint32_t pan,uint32_t tilt,uint32_t zoom,uint32_t preset_token)
{
	uint32_t param;
	this->status = STATUS_ERROR;
	
	//init the ptz range
	if( !this->ptz_config_options_already_done )
		handle_ptz_config_options();
	//not support
	if( profiles.empty() || this->ptz_supported != SUPPORT || stream_index >= profiles.size() )
		return STATUS_ERROR;
	
	switch( command )
	{
	case LS_PTZ_CMD_UP:
	case LS_PTZ_CMD_DOWN:
		handle_ptz(stream_index,command,tilt%(LS_PTZ_SPEED_MAX+1));
		break;
	case LS_PTZ_CMD_LEFT:
	case LS_PTZ_CMD_RIGHT:
		handle_ptz(stream_index,command,pan%(LS_PTZ_SPEED_MAX+1) );
	case LS_PTZ_CMD_UR:
	case LS_PTZ_CMD_UL:
	case LS_PTZ_CMD_DR:  
	case LS_PTZ_CMD_DL:
		param = (tilt & 0x00ff ) % (LS_PTZ_SPEED_MAX+1);
        param &= (pan & 0x00ff) %(LS_PTZ_SPEED_MAX+1) << 8;
		handle_ptz(stream_index,command,param);
		break;
	case LS_PTZ_CMD_ZOOMIN: 
	case LS_PTZ_CMD_ZOOMOUT:
		handle_ptz(stream_index,command,zoom);
		break;
	case LS_PTZ_CMD_UP_STOP:        
    case LS_PTZ_CMD_DOWN_STOP:      
    case LS_PTZ_CMD_LEFT_STOP:      
    case LS_PTZ_CMD_RIGHT_STOP:     
    case LS_PTZ_CMD_UR_STOP:        
    case LS_PTZ_CMD_UL_STOP:        
    case LS_PTZ_CMD_DR_STOP:        
    case LS_PTZ_CMD_DL_STOP:        
    case LS_PTZ_CMD_ZOOMIN_STOP:    
    case LS_PTZ_CMD_ZOOMOUT_STOP: 
		handle_ptz(stream_index,command,0);
		break;
	case LS_PTZ_CMD_POS_CALL:
	case LS_PTZ_CMD_POS_CLEAR:  
	case LS_PTZ_CMD_POS_SET:
		handle_ptz(stream_index,command,preset_token);
		break;
	default:
		break;
	}
	return this->status;

}
void device_ptz::handle_ptz_helper(ls_http_response* http_response,void *client_data)
{
    device_ptz *me = static_cast<device_ptz *>(client_data);
    me->handle_message_ptz(http_response);

  //  if(ptz_info->time - time(NULL) <= PTZ_TIMEOUT_S)
  //      me->do_ptz(ptz_info->command, ptz_info->param);
    
   // free(arg);
}
void device_ptz::handle_message_ptz(ls_http_response* http_response)
{
	//lsprint("%s",http_response->response);
	/* check error */
	this->status = check_reponse_error(http_response);
    if( STATUS_PARSE_RESPONSE_ERROR != this->status )
		return;
	this->status = STATUS_ERRNO;
}

void device_ptz::handle_ptz(uint32_t stream_index,uint32_t command, uint32_t param)
{
	char                wss_header[2048] = {'\0'};
    double              tilt_speed = 0.0; /* speed for up/down */
    double              pan_speed = 0.0;  /* speed for left/right */
    double              zoom_speed = 0.0; /* speed for zoom-in/zoom-out */
	char			    content[4096] = {'\0'};
	string				content_type = string("application/soap+xml; charset=utf-8;");
	size_t				content_len = 0;

	//if port do not init
	if(this->xaddrs_uri.empty())
		set_xaddrs();
	ls_http_request request(HTTP_POST,1,1,this->xaddrs_port);

    build_wss_header(wss_header, sizeof(wss_header));

	
	request.setUri(this->xaddrs_uri_ptz.c_str(),this->xaddrs_uri_ptz.length());
	request.setHost(this->IP.c_str(),this->IP.length());
	

	

    /* speed for tilt or pan */
    if(LS_PTZ_CMD_UP == command || LS_PTZ_CMD_DOWN == command ||
       LS_PTZ_CMD_LEFT == command || LS_PTZ_CMD_RIGHT == command)
    {
        if(param < LS_PTZ_SPEED_MIN || param > LS_PTZ_SPEED_MAX) return;
        switch(command)
        {
        case LS_PTZ_CMD_UP:
            tilt_speed = ptz_pt_yrange_max * param / LS_PTZ_SPEED_MAX;
            if((int)tilt_speed == 0 && (int)(tilt_speed * 10) == 0) tilt_speed = 0.1;
            break;
        case LS_PTZ_CMD_DOWN:
            tilt_speed = ptz_pt_yrange_min * param / LS_PTZ_SPEED_MAX;
            if((int)tilt_speed == 0 && (int)(tilt_speed * 10) == 0) tilt_speed = -0.1;
            break;
        case LS_PTZ_CMD_LEFT:
            pan_speed = ptz_pt_xrange_min * param / LS_PTZ_SPEED_MAX;
            if((int)pan_speed == 0 && (int)(pan_speed * 10) == 0) pan_speed = -0.1;
            break;
        case LS_PTZ_CMD_RIGHT:
            pan_speed = this->ptz_pt_xrange_max * param / LS_PTZ_SPEED_MAX;
            if((int)pan_speed == 0 && (int)(pan_speed * 10) == 0) pan_speed = 0.1;
            break;
        default:
            return;
        }
    }

    /* speed for tilt and pan */
    if(LS_PTZ_CMD_UR == command || LS_PTZ_CMD_UL == command ||
       LS_PTZ_CMD_DR == command || LS_PTZ_CMD_DL == command)
    {
        tilt_speed = (param & 0xff00) >> 8;
        pan_speed = param & 0xff;
        if(tilt_speed < LS_PTZ_SPEED_MIN || tilt_speed > LS_PTZ_SPEED_MAX) return;
        if(pan_speed < LS_PTZ_SPEED_MIN || pan_speed > LS_PTZ_SPEED_MAX) return;
        switch(command)
        {
        case LS_PTZ_CMD_UR:
            tilt_speed = ptz_pt_yrange_max * tilt_speed / LS_PTZ_SPEED_MAX;
            pan_speed = ptz_pt_xrange_max * pan_speed / LS_PTZ_SPEED_MAX;
            if((int)tilt_speed == 0 && (int)(tilt_speed * 10) == 0) tilt_speed = 0.1;
            if((int)pan_speed == 0 && (int)(pan_speed * 10) == 0) pan_speed = 0.1;
            break;
        case LS_PTZ_CMD_UL: 
            tilt_speed = ptz_pt_yrange_max * tilt_speed / LS_PTZ_SPEED_MAX;
            pan_speed = ptz_pt_xrange_min * pan_speed / LS_PTZ_SPEED_MAX;
            if((int)tilt_speed == 0 && (int)(tilt_speed * 10) == 0) tilt_speed = 0.1;
            if((int)pan_speed == 0 && (int)(pan_speed * 10) == 0) pan_speed = -0.1;
            break;
        case LS_PTZ_CMD_DR:
            tilt_speed = ptz_pt_yrange_min * tilt_speed / LS_PTZ_SPEED_MAX;
            pan_speed = ptz_pt_xrange_max * pan_speed / LS_PTZ_SPEED_MAX;
            if((int)tilt_speed == 0 && (int)(tilt_speed * 10) == 0) tilt_speed = -0.1;
            if((int)pan_speed == 0 && (int)(pan_speed * 10) == 0) pan_speed = 0.1;
            break;
        case LS_PTZ_CMD_DL:
            tilt_speed = ptz_pt_yrange_min * tilt_speed / LS_PTZ_SPEED_MAX;
            pan_speed = ptz_pt_xrange_min * pan_speed / LS_PTZ_SPEED_MAX;
            if((int)tilt_speed == 0 && (int)(tilt_speed * 10) == 0) tilt_speed = -0.1;
            if((int)pan_speed == 0 && (int)(pan_speed * 10) == 0) pan_speed = -0.1;
            break;
        default:
            return;
        }
    }

    /* speed for zoom */
    if(LS_PTZ_CMD_ZOOMIN == command || LS_PTZ_CMD_ZOOMOUT == command)
    {
        if(param < LS_PTZ_SPEED_MIN || param > LS_PTZ_SPEED_MAX) return;
        switch(command)
        {
        case LS_PTZ_CMD_ZOOMIN:
            zoom_speed = ptz_z_xrange_max * param / LS_PTZ_SPEED_MAX;
            if((int)zoom_speed == 0 && (int)(zoom_speed * 10) == 0) zoom_speed = 0.1;
            break;
        case LS_PTZ_CMD_ZOOMOUT:
            zoom_speed = ptz_z_xrange_min * param / LS_PTZ_SPEED_MAX;
            if((int)zoom_speed == 0 && (int)(zoom_speed * 10) == 0) zoom_speed = -0.1;
            break;
        default:
            return;
        }        
    }

    switch(command)
    {
    case LS_PTZ_CMD_RESET:          
    case LS_PTZ_CMD_LOCK:           
    case LS_PTZ_CMD_UNLOCK:         
        return;
    case LS_PTZ_CMD_UP:
    case LS_PTZ_CMD_DOWN:
		
		content_type.append("action=\"http://www.onvif.org/ver20/ptz/wsdl/ContinuousMove\"");
        content_len =
            snprintf(content, sizeof(content),
                     "%s%s%s<tptz:ContinuousMove><tptz:ProfileToken>%s</tptz:ProfileToken>"
                     "<tptz:Velocity><tt:PanTilt y=\"%.1f\"/></tptz:Velocity>"
                     "<tptz:Timeout>PT%uS</tptz:Timeout></tptz:ContinuousMove>%s",
                     PTZ_CMDS_START_1, wss_header, PTZ_CMDS_START_2,
					 profiles.at(stream_index)->stream_token, tilt_speed, ptz_move_timeout_s, PTZ_CMDS_END);
        break;
    case LS_PTZ_CMD_LEFT:
    case LS_PTZ_CMD_RIGHT:          
        content_type.append("action=\"http://www.onvif.org/ver20/ptz/wsdl/ContinuousMove\"");
        content_len =
            snprintf(content, sizeof(content),
                     "%s%s%s<tptz:ContinuousMove><tptz:ProfileToken>%s</tptz:ProfileToken>"
                     "<tptz:Velocity><tt:PanTilt x=\"%.1f\"/></tptz:Velocity>"
                     "<tptz:Timeout>PT%uS</tptz:Timeout></tptz:ContinuousMove>%s",
                     PTZ_CMDS_START_1, wss_header, PTZ_CMDS_START_2,
                     profiles.at(stream_index)->stream_token, pan_speed, ptz_move_timeout_s, PTZ_CMDS_END);
        break;
    case LS_PTZ_CMD_UR:             
    case LS_PTZ_CMD_UL:             
    case LS_PTZ_CMD_DR:             
    case LS_PTZ_CMD_DL:             
        content_type.append("action=\"http://www.onvif.org/ver20/ptz/wsdl/ContinuousMove\"");
        content_len =
            snprintf(content, sizeof(content),
                     "%s%s%s<tptz:ContinuousMove><tptz:ProfileToken>%s</tptz:ProfileToken>"
                     "<tptz:Velocity><tt:PanTilt x=\"%.1f\" y=\"%.1f\"/></tptz:Velocity>"
                     "<tptz:Timeout>PT%uS</tptz:Timeout></tptz:ContinuousMove>%s",
                     PTZ_CMDS_START_1, wss_header, PTZ_CMDS_START_2,
                     profiles.at(stream_index)->stream_token, pan_speed, tilt_speed, ptz_move_timeout_s, PTZ_CMDS_END);
        lsprint("x=%.1f, y=%.1f\n", pan_speed, tilt_speed);
        break;
    case LS_PTZ_CMD_ZOOMIN:         
    case LS_PTZ_CMD_ZOOMOUT:        
        content_type.append("action=\"http://www.onvif.org/ver20/ptz/wsdl/ContinuousMove\"");
        content_len =
            snprintf(content, sizeof(content),
                     "%s%s%s<tptz:ContinuousMove><tptz:ProfileToken>%s</tptz:ProfileToken>"
                     "<tptz:Velocity><tt:Zoom x=\"%.1f\"/></tptz:Velocity>"
                     "<tptz:Timeout>PT%uS</tptz:Timeout></tptz:ContinuousMove>%s",
                     PTZ_CMDS_START_1, wss_header, PTZ_CMDS_START_2,
                     profiles.at(stream_index)->stream_token, zoom_speed, ptz_move_timeout_s, PTZ_CMDS_END);
        break;
    case LS_PTZ_CMD_UP_STOP:        
    case LS_PTZ_CMD_DOWN_STOP:      
    case LS_PTZ_CMD_LEFT_STOP:      
    case LS_PTZ_CMD_RIGHT_STOP:     
    case LS_PTZ_CMD_UR_STOP:        
    case LS_PTZ_CMD_UL_STOP:        
    case LS_PTZ_CMD_DR_STOP:        
    case LS_PTZ_CMD_DL_STOP:        
    case LS_PTZ_CMD_ZOOMIN_STOP:    
    case LS_PTZ_CMD_ZOOMOUT_STOP:   
        content_type.append("action=\"http://www.onvif.org/ver20/ptz/wsdl/Stop\"");
        content_len =
            snprintf(content, sizeof(content),
                     "%s%s%s<tptz:Stop><tptz:ProfileToken>%s</tptz:ProfileToken>"
                     "<tptz:PanTilt>true</tptz:PanTilt><tptz:Zoom>true</tptz:Zoom></tptz:Stop>%s",
                     PTZ_CMDS_START_1, wss_header, PTZ_CMDS_START_2, profiles.at(stream_index)->stream_token, PTZ_CMDS_END);
        break;
    case LS_PTZ_CMD_IRISOPEN:       
    case LS_PTZ_CMD_IRISOPEN_STOP:  
    case LS_PTZ_CMD_IRISCLOSE:      
    case LS_PTZ_CMD_IRISCLOSE_STOP: 
    case LS_PTZ_CMD_FOCUSNEAR:      
    case LS_PTZ_CMD_FOCUSNEAR_STOP: 
    case LS_PTZ_CMD_FOCUSFAR:       
    case LS_PTZ_CMD_FOCUSFAR_STOP:  
    case LS_PTZ_CMD_ASS:            
    case LS_PTZ_CMD_ASS_STOP:       
        return;
    case LS_PTZ_CMD_POS_SET:        
        content_type.append("action=\"http://www.onvif.org/ver20/ptz/wsdl/SetPreset\"");
        content_len =
            snprintf(content, sizeof(content),
                     "%s%s%s<tptz:SetPreset><tptz:ProfileToken>%s</tptz:ProfileToken>"
                     "<tptz:PresetToken>%u</tptz:PresetToken></tptz:SetPreset>%s",
                     PTZ_CMDS_START_1, wss_header, PTZ_CMDS_START_2, profiles.at(stream_index)->stream_token, param, PTZ_CMDS_END);
        break;
    case LS_PTZ_CMD_POS_CLEAR:      
        content_type.append("action=\"http://www.onvif.org/ver20/ptz/wsdl/RemovePreset\"");
        content_len =
            snprintf(content, sizeof(content),
                     "%s%s%s<tptz:RemovePreset><tptz:ProfileToken>%s</tptz:ProfileToken>"
                     "<tptz:PresetToken>%u</tptz:PresetToken></tptz:RemovePreset>%s",
                     PTZ_CMDS_START_1, wss_header, PTZ_CMDS_START_2, profiles.at(stream_index)->stream_token, param, PTZ_CMDS_END);
        break;
    case LS_PTZ_CMD_POS_CALL:       
        content_type.append("action=\"http://www.onvif.org/ver20/ptz/wsdl/GotoPreset\"");
        content_len =
        snprintf(content, sizeof(content),
                     "%s%s%s<tptz:GotoPreset><tptz:ProfileToken>%s</tptz:ProfileToken>"
                     "<tptz:PresetToken>%u</tptz:PresetToken></tptz:GotoPreset>%s",
                     PTZ_CMDS_START_1, wss_header, PTZ_CMDS_START_2, profiles.at(stream_index)->stream_token, param, PTZ_CMDS_END);
        break;
    case LS_PTZ_CMD_COMBINE:        
    case LS_PTZ_CMD_COMBINE_STOP:   
    default:
        return;
    }
    
	request.setContentType(content_type.c_str(),content_type.length());
	request.setContent(content,content_len);
	request.setHandler((HandlerProc *)&handle_ptz_helper,(void *)this);
	
	/*send request*/
	if( -1 == request.sendTo() )
	{
		this->status = STATUS_CONNECT_DEVICE_TIMEOUT;
		return;
	}
	/*receive the response and handle it*/
	request.recvFrom();
  
    lsprint("Receiver_onvif, send PTZ. command:%u. param:%u. peer:[%s].\n",
		command, param, this->IP.c_str());
}

