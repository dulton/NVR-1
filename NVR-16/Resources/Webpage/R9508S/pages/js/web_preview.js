var init_start = 0;//~INITSTART~
var bConnect = [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0];
var recstatus = [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0];
var lastvideostatus = [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0];
var lastvideochn = [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0];
var connstatus = 0;
var max_chn = 8;
var g_lang_id = 0;
var g_b_voip = 0;
var display_num;

NETDVR_PTZ_COM_STOP = 0;
NETDVR_PTZ_COM_MOVEUP = 1;
NETDVR_PTZ_COM_MOVEDOWN = 2;
NETDVR_PTZ_COM_MOVELEFT = 3;
NETDVR_PTZ_COM_MOVERIGHT = 4;
NETDVR_PTZ_COM_ROTATION = 5;
NETDVR_PTZ_COM_ZOOMADD = 6;
NETDVR_PTZ_COM_ZOOMSUBTRACT = 7;
NETDVR_PTZ_COM_FOCUSADD = 8;
NETDVR_PTZ_COM_FOCUSSUBTRACT = 9;
NETDVR_PTZ_COM_APERTUREADD = 10;
NETDVR_PTZ_COM_APERTURESUBTRACT = 11;
NETDVR_PTZ_COM_LIGHTINGOPEN = 12;
NETDVR_PTZ_COM_LIGHTINGCLOSE = 13;
NETDVR_PTZ_COM_WIPERSOPEN = 14;
NETDVR_PTZ_COM_WIPERSCLOSE = 15;
NETDVR_PTZ_COM_FAST = 16;
NETDVR_PTZ_COM_NORMAL = 17;
NETDVR_PTZ_COM_SLOW = 18;
NETDVR_PTZ_COM_AUXILIARYOPEN = 19;
NETDVR_PTZ_COM_AUXILIARYCLOSE = 20;

function init_activex()
{

}
function is_lanip(str)
{
	var reg = /^(127\.0\.0\.1)|(localhost)|(10(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]\d|\d)){2})(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]\d|[1-9]))|(172\.((1[6-9])|2\d|3[01])\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]\d|\d)\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]\d|[1-9]))|(192\.168\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]\d|\d)\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]\d|[1-9]))$/;
	return reg.test(str); 
}
function initasp()
{
	var webtype = 0; //0 for lan; 1 for wan
	
	if (true == is_lanip(document.location.hostname))
		webtype = 0;
	else
		webtype = 1;
	
	document.getElementById("username").focus();
	document.getElementById("nettype").options[webtype].selected = true;
	document.getElementById("streamtype").options[webtype].selected = true;
	
	//if (1 == webtype) document.getElementById("all_control").style.display = "none";
	
	page_loaded = true;
	//OnCheckAll();
}
var the_alltime;
var first = 0;
function OnCheckAll()
{
	//alert("OnCheckAll");
	var retConnect;
	retConnect = TLNetDvrCtrl.GetConnectState();
	//retConnect= 1;
	if(retConnect==0)
	{
		//clearTimeout(the_alltime);
		connstatus = 1;
		stop_all();
		if (0 == g_lang_id)
		{
			window.status="请检查主机是否断开或重启！";
		}
		else
		{
			window.status="Pls check whether machine is disconnected or not?";
		}

		//window.status="请检查主机是否断开或重启！";
		//return;
	}
	else
	{
		window.status=first;
		first ++;
		var chn;
		for(chn=0;chn<max_chn;chn++)
		{
			retConnect = TLNetDvrCtrl.GetRecordState(chn);
			if(recstatus[chn]!=retConnect)
			{
				recstatus[chn]=retConnect;
				if(recstatus[chn])
				{
					document.getElementById("recpic"+chn.toString()).src = "/images/video-1.png";
				}
				else
				{
					document.getElementById("recpic"+chn.toString()).src = "/images/video.png";
				}
			}
		}
		
		if(connstatus)
		{
			var i, ret;
			connstatus = 0;
			for (i = 0; i < max_chn; i++)
			{
				if (1 == lastvideostatus[i])
				{
					//alert(lastvideochn[i]);
					TLNetDvrCtrl.ActiveWnd = lastvideochn[i];
					ret = play_video(i, get_stream_type(i));
					if (true == ret)
					{
						document.getElementById("pic"+i.toString()).src = "/images/ch-1.png";
						document.getElementById("ch"+i.toString()).style.color="#3399FF";
						bConnect[i] = 1;
					}
				}
			}
		}
	}
	the_alltime = setTimeout("OnCheckAll();", 1000);
}

function detect_plugin(obj_name, item_name)
{
    try
    {
        if(eval(obj_name+"."+item_name) == undefined)
        {
            return false;
        }
        else
        {
            return true;
        }
    }
    catch(e)
    {
        return false;
    }
}

function ChangLanguage(languageid)
{
  //window.location.href='/home.asp?lang=EN';
//alert(languageid);
         if (0 == languageid)
         {
                   window.location.href='/CH/home.asp';
         }        
         else if        (2 == languageid)
         {
                   window.location.href='/TCH/home.asp';
         }
         else 
         {
                   window.location.href='/EN/home.asp';
         }
}


function get_version_id(str_ver)
{
	var ver_id = 0, i = 0, last_i = 0;
	
	if (str_ver)
	{
		str_ver = str_ver.replace(/\./g,",");
		str_ver = str_ver.replace(/ /g,"");
		while (i >= 0)
		{
			i = str_ver.indexOf(",", last_i);
			if (i >= 0)
			{
				ver_id <<= 8;
				ver_id += parseInt(str_ver.substring(last_i , i));
				last_i = i + 1;
			}
		}
		
		ver_id <<= 8;
		ver_id += parseInt(str_ver.substring(last_i , str_ver.length));
	}
	return ver_id;
}

function detect_version()
{
	var run_ver = TLNetDvrCtrl.GetActiveXVersion();
	var dvr_ver =TLNetDvrCtrl.codeBase;
	var run_ver_id = 0, dvr_ver_id = 0;
	var index = dvr_ver.indexOf("version=", 0);
	
	if (index < 0)
	{
		return -1;
	}
	dvr_ver = dvr_ver.substring(index + 8, dvr_ver.length);

	run_ver_id = get_version_id(run_ver);
	dvr_ver_id = get_version_id(dvr_ver);
	
	return dvr_ver_id - run_ver_id;
}

function do_login()
{
	g_lang_id = parseInt(document.getElementById("enflag").value);
	if (false == detect_plugin("TLNetDvrCtrl", "LangID"))
	{
		if (0 == g_lang_id)
		{
			alert("控件未安装!");
		}
		else
		{
			alert("Plugin is not installed!");
		}
		return false;
	}
	//else
	//{
	//	if(0!=detect_version())
	//	{
	//		 if (0 == g_lang_id)
	//		 {
	//			alert("控件版本不一致!建议手动下载安装。");
	//		 }
	//		 else
	//		 {
	//			alert("For the version of plugin is not Consistent, to download manual installation package is preferred."); 
	//		 }
	//	}
	//}
	
	TLNetDvrCtrl.ServerIP = document.location.hostname;
	TLNetDvrCtrl.ServerType = 2;// 2 3511
	TLNetDvrCtrl.MainChnNum  = max_chn;
	TLNetDvrCtrl.AlarmInNum  = 4;// 9108
	TLNetDvrCtrl.AlarmOutNum  = 1;// 9108
	TLNetDvrCtrl.MediaPort   = 9008;//~MediaPort~
	TLNetDvrCtrl.ServerPort   = 8630;//~ServerPort~
	TLNetDvrCtrl.OEMVersion   = 0;//~OEMVersion~
	TLNetDvrCtrl.UserName = document.getElementById("username").value;
	//alert("do login");
	//alert(TLNetDvrCtrl.UserName);
	TLNetDvrCtrl.Password = document.getElementById("loginpass").value;
	TLNetDvrCtrl.LangID = g_lang_id;//0 sc 1 eng
	var ret = TLNetDvrCtrl.Login();
	if (true == ret)
	{
		var nettype = parseInt(document.getElementById("nettype").value);
		if (1 == nettype)//WAN
		{
			document.getElementById("streamtype").options[1].selected = true;
			document.getElementById("streamtype").style.display = "none";
		}
		document.getElementById("login_layer").innerHTML = "";
		document.getElementById("login_layer").style.display = "none";
		document.getElementById("preview_layer").style.display = "block";
		if (0 == g_lang_id)
		{
			document.getElementById("user").innerHTML = "<font><span style='float:left'>用户:</span><span style='float:left'>" +TLNetDvrCtrl.UserName + "</span></font>";
		}
		else
		{
			document.getElementById("user").innerHTML = "<font><span style='float:left'>User:</span><span style='float:left'>" +TLNetDvrCtrl.UserName + "</span></font>";
		}
		switch (max_chn) {
		case 16:
			TLNetDvrCtrl.DisplayWndNum =16;
			break;
		case 8:
			TLNetDvrCtrl.DisplayWndNum =9;
			break;
		case 4:
			TLNetDvrCtrl.DisplayWndNum =4;
			break;
		default:
			TLNetDvrCtrl.DisplayWndNum =1;
			break;
		}
		if (max_chn > 8)
			display_num = 16;
		else if (max_chn > 4)
			display_num = 9;
		else 
			display_num = 4;
			
		TLNetDvrCtrl.DisplayWndNum = display_num;
		if (init_start > 0)
		{
			OnStartAll();
		}
		OnCheckAll();
	}
	else
	{
		if (0 == g_lang_id)
		{
			alert("登录失败!");
		}
		else
		{
			alert("Login failed!");
		}
	}

	//OnCheckAll();
	return false;
}

//属性：short DisplayWndNum 当前显示的窗口的数目(1.4.9.16)
//方法：void SetDisplayWnd(); 应用改变的窗口数目
//	  short GetDisplayNum();
function do_set_display_wndnum(count)
{
	TLNetDvrCtrl.DisplayWndNum = count;
	display_num = count;
	TLNetDvrCtrl.SetDisplayWnd();
}

function play_video(chn, streamtype)
{
//	alert(streamtype);
	var ret;
	TLNetDvrCtrl.Channel = chn;
	TLNetDvrCtrl.StreamType = streamtype;
	ret = TLNetDvrCtrl.Preview();
	return ret;
}

function stop_video(chn)
{
	var ret;
	TLNetDvrCtrl.Channel = chn;
	ret = TLNetDvrCtrl.Stop();
	return ret;
}

function get_stream_type(chn)
{
	return parseInt(document.getElementById("streamtype").value);
	//return parseInt(document.getElementById("streamtye"+chn.toString()).value);
}

function control_channel(chn)
{
	var ret;
	
	if (chn > max_chn)
	{
		return;
	}
	
	var playing_channel = TLNetDvrCtrl.Channel;	
	if (0 == bConnect[chn])
	{
		if (-1 != playing_channel)
		{
			document.getElementById("pic"+playing_channel.toString()).src = "/images/ch.png";
			
			document.getElementById("ch"+playing_channel.toString()).style.color="#000000";
			
			
			bConnect[playing_channel] = 0;
		}
		
		ret = play_video(chn, get_stream_type(chn));
		if (true == ret)
		{
			lastvideochn[chn] = TLNetDvrCtrl.ActiveWnd;
			document.getElementById("pic"+chn.toString()).src = "/images/ch-1.png";
			
			document.getElementById("ch"+chn.toString()).style.color = "#3399FF";
			bConnect[chn] = 1;
		}
		else
		{
			if (0 == g_lang_id)
			{
				alert("预览开启失败!");
			}
			else
			{
				alert("Start preview failed!");
			}
		}
	}
	else
	{
		ret = stop_video(chn);
		if (true == ret)
		{
			document.getElementById("pic"+chn.toString()).src = "/images/ch.png";
			
			document.getElementById("ch"+chn.toString()).style.color = "#000000";
			
			bConnect[chn] = 0;
		}
	}
}

function ptz_command(command)
{
	TLNetDvrCtrl.PTZCommand = command;
	TLNetDvrCtrl.PTZCtrlStart();
}

function ptz_move_up()
{
	ptz_command(NETDVR_PTZ_COM_MOVEUP);
}

function ptz_move_down()
{
	ptz_command(NETDVR_PTZ_COM_MOVEDOWN);
}

function ptz_move_left()
{
	ptz_command(NETDVR_PTZ_COM_MOVELEFT);
}

function ptz_move_right()
{
	ptz_command(NETDVR_PTZ_COM_MOVERIGHT);
}

function ptz_aper_decrease()
{
	ptz_command(NETDVR_PTZ_COM_APERTURESUBTRACT);
}

function ptz_aper_increase()
{
	ptz_command(NETDVR_PTZ_COM_APERTUREADD);
}

function ptz_focus_farther()
{
	ptz_command(NETDVR_PTZ_COM_FOCUSADD);
}

function ptz_focus_nearer()
{
	ptz_command(NETDVR_PTZ_COM_FOCUSSUBTRACT);
}

function ptz_zoom_in()
{
	ptz_command(NETDVR_PTZ_COM_ZOOMADD);
}

function ptz_zoom_out()
{
	ptz_command(NETDVR_PTZ_COM_ZOOMSUBTRACT);
}

function ptz_stop()
{
	ptz_command(NETDVR_PTZ_COM_STOP);
	//TLNetDvrCtrl.PTZCtrlStop();
}

function do_voip(theButton)
{
	var ret;
	if (0 == g_b_voip)
	{
		ret = TLNetDvrCtrl.StartVOIP();
		
		if (false == ret)
		{
			if (0 == g_lang_id)
			{
				alert("开启语音对讲失败!");
			}
			else
			{
				alert("Start VOIP failed!");
			}	
		}
		else
		{
			if (0 == g_lang_id)
			{
				theButton.innerHTML = "停止对讲";
			}
			else
			{
				theButton.innerHTML = "Stop VOIP";
			}	
			g_b_voip = 1;
		}
	}
	else
	{
		ret = TLNetDvrCtrl.StopVOIP();
		if (false == ret)
		{
			if (0 == g_lang_id)
			{
				alert("停止语音对讲失败!");
			}
			else
			{
				alert("Stop VOIP failed!");
			}	
		}
		else
		{
			if (0 == g_lang_id)
			{
				theButton.innerHTML = "开启对讲";
			}
			else
			{
				theButton.innerHTML = "Start VOIP";
			}	
			g_b_voip = 0;
		}
	}
}

function do_configure()
{
	TLNetDvrCtrl.RemoteParaCfg();
}

function do_playback()
{
	TLNetDvrCtrl.PlayBack();
}

function mouseover(obj,pin)
{
	
	if(pin == 1)
	{
		obj.src="/images/left-1.png";
	}
	else if(pin == 2)
	{
		obj.src="/images/up-1.png";
	}
	else if(pin == 3)
	{
		obj.src="/images/right-1.png";
	}
	else if(pin == 4)
	{
		obj.src="/images/down-1.png";
	}
	else if(pin == 5)
	{
		obj.src="/images/small-1.png";
	}
	else if(pin == 6)
	{
		obj.src="/images/big-1.png";
	}
	else if(pin == 7)
	{
		obj.src="/images/one-1.png";
	}
	else if(pin == 8)
	{
		obj.src="/images/four-1.png";
	}
	else if(pin == 9)
	{
		obj.src="/images/nine-1.png";
	}
	else if(pin == 10)
	{
		obj.src="/images/sixteen-1.png";
	}
	else if(pin == 11)
	{
		obj.src="/images/ptz-1.png";
	}
	else if(pin == 12)
	{
		//obj.src="/images/ok-1.png";
		document.getElementById("sure").style.background="url(../images/ok-1.png)";
		
	}
	else if(pin == 13)
	{
		document.getElementById("cancel").style.background="url(../images/cancel-1.png)";
	}
} 

function tableShow()
{
	
	if(document.getElementById("bbb").style.visibility=="visible")
	{
		document.getElementById("bbb").style.visibility="hidden";
	
	}
	else 
	{
		document.getElementById("bbb").style.visibility="visible";
			
	}
}



function mouseout(obj,pin)
{
	if(pin == 1)
	{
		obj.src="/images/left.png";
	}
	else if(pin == 2)
	{
		obj.src="/images/up.png";
	}
	else if(pin == 3)
	{
		obj.src="/images/right.png";
	}
	else if(pin == 4)
	{
		obj.src="/images/down.png";
	}
	else if(pin == 5)
	{
		obj.src="/images/small.png";
	}
	else if(pin == 6)
	{
		obj.src="/images/big.png";
	}
	else if(pin == 7)
	{
		obj.src="/images/one.png";
	}
	else if(pin == 8)
	{
		obj.src="/images/four.png";
	}
	else if(pin == 9)
	{
		obj.src="/images/nine.png";
	}
	else if(pin == 10)
	{
		obj.src="/images/sixteen.png";
	}
	else if(pin == 11)
	{
		obj.src="/images/ptz.png";
	}
	else if(pin == 12)
	{
		document.getElementById("sure").style.background="url(../images/ok.png)";
	}
	else if(pin == 13)
	{
		document.getElementById("cancel").style.background="url(../images/cancel.png)";
	}
} 



function start_all()
{
	var i, j, winstatus = 0, tmp_chns = max_chn;
	for (i = 0; i < max_chn; i++)
	{
		 winstatus |= (bConnect[i] << lastvideochn[i]);
	}
	
	TLNetDvrCtrl.DisplayWndNum = display_num;
	TLNetDvrCtrl.SetDisplayWnd();
	
	if (tmp_chns > display_num) tmp_chns = display_num;
	
	for (i = 0; i < tmp_chns; i++)
	{
		if (0 == bConnect[i])
		{
			for (j = 0; j < max_chn; j++)
			{
				if (0 == (winstatus & (1 << j)))
				{
					//alert(TLNetDvrCtrl.ActiveWnd);
					//alert(i.toString());
					TLNetDvrCtrl.ActiveWnd = j;
					ret = play_video(i, get_stream_type(i));
					if (true == ret)
					{
						lastvideochn[i] = j;
						document.getElementById("pic"+i.toString()).src = "/images/ch-1.png";
						document.getElementById("ch"+i.toString()).style.color="#3399FF";
						bConnect[i] = 1;
					}
					winstatus |= (1 << j);
					break;
				}
			}
		}
	}
}

function stop_all()
{
	var i, ret;
	for (i = 0; i < max_chn; i++)
	{
		if (1 == bConnect[i])
		{
			lastvideostatus[i] = 1;
			ret = stop_video(i);
			if (true == ret)
			{
				document.getElementById("pic"+i.toString()).src = "/images/ch.png";
				document.getElementById("ch"+i.toString()).style.color="#000000";
				bConnect[i] = 0;
			}
		}
	}
}
