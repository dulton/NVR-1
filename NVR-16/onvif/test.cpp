#include <iostream>
#include "ls_discovery.h"
using namespace std;

#include <string.h>

int main()
{
	discovery_device *receiver_onvif = new device_ptz();
	if(NULL == receiver_onvif) return -1;
	
	receiver_onvif->set_ip("192.168.1.196",strlen("192.168.1.196"));
	receiver_onvif->set_xaddrs((const char *)"http://192.168.1.196:8080/onvif/device_service",(size_t)strlen("http://192.168.1.196:8080/onvif/device_service"));
	
	printf("before get_stream_uri......\n");
	
	vector<std::string *> *pVector = ((device_ptz*)receiver_onvif)->get_stream_uri();
	vector<std::string *>::iterator uri_it = pVector->begin();
	
	printf("after get_stream_uri......\n");
	
	int idx = 0;
	
	while(uri_it != pVector->end())
	{
		cout<<"stream"<<idx<<":"<<(*(*uri_it))<<endl;
		idx++;
		uri_it++;
	}
	
	delete receiver_onvif;
	
	return 0;
	
    onvif_wsdiscovery onvif;
    onvif.init(0);
    onvif.send_once_wsdiscovery_multi();
	
    int num = 0;
    for(list<discovery_device *>::iterator it = onvif.receivers.begin(); it != onvif.receivers.end(); it++)
    {
    	printf("in loop......\n");
		
        cout<<"num:"<<++num<<":\t";
        cout<<(*it)->get_ip()<<endl;
        cout<<(*it)->get_xaddrs()<<endl;
        cout<<(*it)->get_uuid()<<endl;
		
		printf("set user......\n");
		
        if(  (*it)->get_ip() == string("192.168.4.120")  ||  (*it)->get_ip() == string("192.168.4.115")  )
        {
            ((device_ptz*)(*it))->set_username_password("admin","9999");

            //cout<< ((device_info*)(*it))->handle_mngmt_capabilities()<<endl;
        }
        else if((*it)->get_ip() == string("192.168.4.4")  )
        {
            ((device_ptz*)(*it))->set_username_password("system","system");
            //cout<< ((device_info*)(*it))->handle_mngmt_capabilities()<<endl;
        }
        else if( (*it)->get_ip() == string("192.168.4.40")  )
            ((device_ptz*)(*it))->set_username_password("admin","");
        else if( (*it)->get_ip() == string("192.168.4.65")  )
        {
            ((device_ptz*)(*it))->set_username_password("admin","12345");
            //cout<< ((device_info*)(*it))->handle_mngmt_capabilities()<<endl;
            //cout<< "\nstatus:"<<((device_ptz*)(*it))->handle_mngmt_set_ip_prefix("192.168.4.65",strlen("192.168.4.65"),24)<<endl;
        }
        else if( (*it)->get_ip() == string("192.168.4.127")  )
            ((device_ptz*)(*it))->set_username_password("admin","123456");
        else
            ((device_ptz*)(*it))->set_username_password("admin","admin");
		
		printf("before handle_media_stream_uri......\n");
		
        //cout<< ((device_ptz*)(*it))->handle_mngmt_capabilities()<<endl;
        //cout<< ((device_ptz*)(*it))->handle_media_profiles()<<endl;
        cout<< ((device_ptz*)(*it))->handle_media_stream_uri()<<endl;
        //((device_ptz*)(*it))->do_ptz(LS_PTZ_CMD_LEFT,30,10,10,10);
        //((device_ptz*)(*it))->handle_ptz_config_options();
        //((device_ptz*)(*it))->handle_mngmt_set_dns("192.168.0.17",strlen("192.168.0.17"));
        //((device_ptz*)(*it))->handle_mngmt_get_dns();
        //cout<< "\nstatus:"<<((device_ptz*)(*it))->handle_mngmt_set_ntp("192.168.0.17",strlen("192.168.0.17"))<<endl;
        //cout<< "\nstatus:"<<((device_ptz*)(*it))->handle_mngmt_get_ntp()<<endl;
        //cout<< "\nstatus:"<<((device_ptz*)(*it))->handle_mngmt_system_reboot()<<endl;
        //cout<< "\nstatus:"<<((device_ptz*)(*it))->handle_mngmt_system_factory_default(SOFT_FACTORY_DEFAULT)<<endl;
        //cout<< "\nstatus:"<<((device_ptz*)(*it))->handle_mngmt_set_ip_prefix("192.168.4.65",strlen("192.168.4.65"),24)<<endl;
        //cout<< "\nstatus:"<<((device_ptz*)(*it))->handle_mngmt_get_ip()<<endl;
        //cout<< "\nstatus:"<<((device_ptz*)(*it))->handle_mngmt_set_default_gateway("192.168.4.1",strlen("192.168.4.1"))<<endl;
        //cout<< "\nstatus:"<<((device_ptz*)(*it))->handle_mngmt_get_default_gateway()<<endl;
        //cout<< "\nstatus:"<<((device_ptz*)(*it))->handle_mngmt_set_date_time(2013,9,12,11,13,0)<<endl;
        //cout<< "\nstatus:"<<((device_ptz*)(*it))->handle_mngmt_set_date_time(time(NULL))<<endl;
        /*struct device_infomation dv;
        cout<< "\nstatus:"<<((device_ptz*)(*it))->handle_mngmt_get_device_infomation(&dv)<<endl;
        lsprint("Manufacturer:%s\nModel:%s\nFirmwareVersion:%s\nSerialNumber:%s\nHardwareId:%s\n",
            dv.manufacturer,dv.model,dv.firmware_version,dv.serial_number,dv.hardwareId);
        */
        printf("after handle_media_stream_uri......\n");
		
		printf("before get_stream_uri......\n");
		
		vector<std::string *> *pVector = ((device_ptz*)(*it))->get_stream_uri();
		vector<std::string *>::iterator uri_it = pVector->begin();
		
		printf("after get_stream_uri......\n");
		
		int idx = 0;
		
		while(uri_it != pVector->end())
		{
			cout<<"stream"<<idx<<":"<<(*(*uri_it))<<endl;
			idx++;
			uri_it++;
		}
		
        if(((device_ptz*)(*it))->handle_image_setting_support() )
        {
            cout<< "\nstatus:"<<((device_ptz*)(*it))->handle_image_set_backlight_compensation(0,true)<<endl;
            cout<< "\nstatus:"<<((device_ptz*)(*it))->handle_image_set_brightness(0,50)<<endl;
            cout<< "\nstatus:"<<((device_ptz*)(*it))->handle_image_set_color_saturation(0,50)<<endl;
            cout<< "\nstatus:"<<((device_ptz*)(*it))->handle_image_set_contrast(0,50)<<endl;
            cout<< "\nstatus:"<<((device_ptz*)(*it))->handle_image_set_sharpness(0,50)<<endl;

            struct current_image_config cf;
            int32_t status;
            if(  ( status = ((device_ptz*)(*it))->handle_image_current_config(0,&cf) )  == STATUS_ERRNO)
                lsprint("BacklightCompensation:%s\nSharpness:%d\nContrast:%d\nBrightness:%d\nColorSaturation:%d\n",
                cf.backlight_compensation ?"ON":"OFF" , cf.sharpness,cf.contrast,cf.brightness,cf.sharpness);
            lsprint("Status:%d\n",status);
        }
        /*const struct  max_image_config *mig;
        mig = ((device_ptz*)(*it))->get_max_image_setting();//不支持最大值返回是0
        lsprint("Sharpness:%d-%d\nContrast:%d-%d\nBrightness:%d-%d\nColorSaturation:%d-%d\n",
        mig->sharpness_min,mig->sharpness_max,mig->contrast_min,mig->contrast_max,mig->brightness_min,mig->brightness_max,mig->sharpness_min,mig->sharpness_max);*/
    /*	vector<std::string *> *stream_uri;
        stream_uri = ((device_ptz*)(*it))->get_stream_uri();
        for( size_t n = 0 ; n < stream_uri->size(); ++n)
            lsprint("stream_url: %s\n",(*stream_uri)[n]->c_str());
        */
        /*vector<media_profile *> *profiles;
        profiles = ((device_ptz*)(*it))->get_profiles();
        for( size_t n = 0 ; n < profiles->size(); ++n)
            lsprint("Receiver_onvif, stream token: %s (%u * %u, %ukbit/s, %ufps, audio:%s).\n",
                (*profiles)[n]->stream_token, (*profiles)[n]->resolution_width, (*profiles)[n]->resolution_height,(*profiles)[n]->bitrate_limit, (*profiles)[n]->framerate_limit,
                (1 == (*profiles)[n]->with_audio ? "ON" : "OFF"));
                */


    }

/*	char buf[2000];
    memset(buf,0,2000);
    build_wss_header(buf, 2000);
    cout<<buf<<endl;*/
    return 0;
}
