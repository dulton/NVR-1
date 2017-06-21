//Ð´cookies
function setCookie(name,value)
{
	var Days = 365;
	var exp = new Date(); 
	exp.setTime(exp.getTime() + Days*24*60*60*1000);
	document.cookie = name + "="+ escape (value) + ";expires=" + exp.toGMTString();
}
//¶ÁÈ¡cookies
function getCookie(name)
{
	var arr,reg=new RegExp("(^| )"+name+"=([^;]*)(;|$)");
	if(arr=document.cookie.match(reg)) return unescape(arr[2]);
	else return null;
}
//É¾³ýcookies
function delCookie(name)
{
	var exp = new Date();
	exp.setTime(exp.getTime() - 1);
	var cval=getCookie(name);
	if(cval!=null) document.cookie= name + "="+cval+";expires="+exp.toGMTString();
}

function switchHomePage(lang_str)
{
	var switch_url = "/" + lang_str + "/home.asp";
	setCookie("home", switch_url);
	document.location.href(switch_url);
}

var home_page = document.location.pathname;
if ("/home.asp" == home_page)
{
	home_page = (document.location).toString();
	var index = home_page.lastIndexOf("?lang=");
	if (index >= 0)
	{
		home_page = home_page.substr(index + 6, 2);
		switchHomePage(home_page);
	}
	else
	{
		home_page = getCookie("home");
		if (null != home_page)
		{
			setCookie("home", home_page);
			document.location.href(home_page);
		}
	}
}