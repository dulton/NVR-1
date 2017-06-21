
/* -*- C++ -*- */
#ifndef __lS_RECEIVER_MEDIA_H
#define __LS_RECEIVER_MEDIA_H 1

#include "ls_receiver_mngmt.h"

#include "ls_ptz.h"


class media_profile
{
public:
	media_profile();
    uint32_t resolution_width;
    uint32_t resolution_height;
    uint32_t framerate_limit;
    uint32_t bitrate_limit;
    Boolean  with_audio;
    uint32_t sequence;
    char     stream_token[64];
	char	 image_token[64];
    char     ptz_token[64];
	
} ;
class device_media : public device_mngmt
{
public:
	device_media();
	virtual ~device_media();

	/*MediaConf*/
	int32_t handle_media_profiles();    // need handle_mngmt_capabilities  get profiles
	int32_t handle_media_stream_uri();  // need profile.token  onvif_media_profile_t
	vector<std::string *> *get_stream_uri();
	vector<media_profile *> *get_profiles();

	static void handle_media_profiles_help(ls_http_response* http_response,void *client_data);
	static void handle_media_stream_uri_help(ls_http_response* http_response,void *client_data);

private:

	void handle_message_stream_uri(ls_http_response* http_response);
	void handle_message_profiles(ls_http_response* http_response);

protected:

protected:
	vector<media_profile *>			profiles;		   //init by handle_media_profiles
	int8_t                          ptz_supported;	   // construstor it -1 indicate not init&& init by handle_media_profiles
	int8_t                          image_supported;	   // construstor it -1 indicate not init&& init by handle_media_profiles
	vector<std::string *>			stream_uris;

};


#endif
