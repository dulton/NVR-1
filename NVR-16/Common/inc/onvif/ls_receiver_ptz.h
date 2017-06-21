
/* -*- C++ -*- */
#ifndef __lS_RECEIVER_PTZ_H
#define __LS_RECEIVER_PTZ_H 1

#include "ls_receiver_image.h"

#include "ls_ptz.h"


class device_ptz: public device_image
{
public:
	device_ptz();
	virtual ~device_ptz();
	Boolean handle_ptz_support();
	int32_t handle_ptz_config_options();
	/*the parameters do not associate with the command does not effect */
	/*return */
	int32_t do_ptz(uint32_t stream_index,uint32_t command, uint32_t pan,uint32_t tilt,uint32_t zoom,uint32_t preset_token);
	static void handle_ptz_config_options_helper(ls_http_response* http_response,void *client_data);
	static void handle_ptz_helper(ls_http_response* http_response,void *client_data);
private:
	void handle_message_ptz_config_options(ls_http_response* http_response);
	void handle_message_ptz(ls_http_response* http_response);
	uint32_t parse_time(char *time_str, uint32_t *time_s);
	void handle_ptz(uint32_t stream_index,uint32_t command, uint32_t param);
protected:
	bool						 ptz_config_options_already_done;
	uint32_t                     ptz_move_timeout_s;
    double                       ptz_pt_xrange_min;
    double                       ptz_pt_xrange_max;
    double                       ptz_pt_yrange_min;
    double                       ptz_pt_yrange_max;
    double                       ptz_z_xrange_min;
    double                       ptz_z_xrange_max;

};


#endif
