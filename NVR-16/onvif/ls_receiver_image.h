
/* -*- C++ -*- */
#ifndef __lS_RECEIVER_IMAGE_H
#define __LS_RECEIVER_IMAGE_H 1

#include "ls_receiver_media.h"

#include "ls_ptz.h"

class current_image_config
{
public:
	current_image_config(){backlight_compensation=False;};
	~current_image_config(){};
public:
	int32_t brightness;
	int32_t color_saturation;
	int32_t contrast;
	int32_t sharpness;
	Boolean backlight_compensation;		//on = true ; false = off
};
class max_image_config
{
public:
	max_image_config(){already_init=False;};
	~max_image_config(){};
public:
	int32_t brightness_max;
	int32_t brightness_min;
	int32_t color_saturation_max;
	int32_t color_saturation_min;
	int32_t contrast_max;
	int32_t contrast_min;
	int32_t sharpness_max;
	int32_t sharpness_min;
	Boolean already_init;
};

//只做5个基本都支持的接口有的没有做 如:CutFilterModes  Exposure WhiteBalance
class device_image : public device_media
{
public:
	device_image();
	virtual ~device_image();
	Boolean handle_image_setting_support();
	max_image_config *get_max_image_setting();
	int32_t handle_image_max_config();					//we use it init max_image_setting
	int32_t handle_image_current_config(uint32_t stream_index,current_image_config *current_image_setting );	//we use it init curent_image_setting
	int32_t handle_image_set_brightness(uint32_t stream_index,int32_t brightness);
	int32_t handle_image_set_contrast(uint32_t stream_index,int32_t contrast);
	int32_t handle_image_set_color_saturation(uint32_t stream_index,int32_t color_saturation);
	int32_t handle_image_set_sharpness(uint32_t stream_index,int32_t sharpness);
	int32_t handle_image_set_backlight_compensation(uint32_t stream_index,Boolean mode);

	int32_t handle_image_set_color(uint32_t stream_index,int32_t brightness,int32_t contrast,int32_t color_saturation,int32_t sharpness);
	int32_t handle_image_focus(uint32_t stream_index,int32_t plus);

	static void handle_image_max_config_helper(ls_http_response* http_response,void *client_data);
	static void handle_image_current_config_helper(ls_http_response* http_response,void *client_data,void *current_image_setting);
	static void handle_image_set_brightness_helper(ls_http_response* http_response,void *client_data);
	static void handle_image_set_color_saturation_helper(ls_http_response* http_response,void *client_data);
	static void handle_image_set_contrast_helper(ls_http_response* http_response,void *client_data);
	static void handle_image_set_sharpness_helper(ls_http_response* http_response,void *client_data);
	static void handle_image_set_backlight_compensation_helper(ls_http_response* http_response,void *client_data);
private:
	void handle_message_current_config(ls_http_response* http_response,current_image_config *current_image_setting);
	void handle_message_max_config(ls_http_response* http_response);
	void handle_message_set_brightness(ls_http_response* http_response);
	void handle_message_set_color_saturation(ls_http_response* http_response);
	void handle_message_set_contrast(ls_http_response* http_response);
	void handle_message_set_sharpness(ls_http_response* http_response);
	void handle_message_set_backlight_compensation(ls_http_response* http_response);
private:

protected:
	max_image_config									max_image_setting;

};


#endif
