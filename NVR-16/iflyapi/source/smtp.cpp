#include "CSmtp.h"
#include <iostream>

//#include "iflytype.h"
typedef int BOOL;

BOOL gmail_send_ssl(char *s_name_from, char *s_mail_from, char *s_name_to, char *s_mail_to, char *s_user, char *s_password, char *s_mailserver, char *s_subject, char *s_body, char *s_attached, char *s_encode_type)
{
	BOOL bError = false;
	
	try
	{
	#if 0//test
		CSmtp mail;

//#define test_gmail_tls
#define test_gmail_ssl

#if defined(test_gmail_tls)
		mail.SetSMTPServer("smtp.gmail.com",587);
		mail.SetSecurityType(CSmtp::USE_TLS);
#elif defined(test_gmail_ssl)
		mail.SetSMTPServer("smtp.gmail.com",465);
		mail.SetSecurityType(CSmtp::USE_SSL);
#elif defined(test_hotmail_TLS)
		mail.SetSMTPServer("smtp.live.com",25);
		mail.SetSecurityType(CSmtp::USE_TLS);
#elif defined(test_aol_tls)
		mail.SetSMTPServer("smtp.aol.com",587);
		mail.SetSecurityType(CSmtp::USE_TLS);
#elif defined(test_yahoo_ssl)
		mail.SetSMTPServer("plus.smtp.mail.yahoo.com",465);
		mail.SetSecurityType(CSmtp::USE_SSL);
#endif

		mail.SetLogin("wuhanruizhe@gmail.com");
		mail.SetPassword("07284581607");
  		mail.SetSenderName("wuhanruizhe");
  		mail.SetSenderMail("wuhanruizhe@gmail.com");
  		mail.SetReplyTo("wuhanruizhe@gmail.com");
  		mail.SetSubject("The message");
  		mail.AddRecipient("57637527@qq.com");
  		mail.SetXPriority(XPRIORITY_NORMAL);
  		mail.SetXMailer("The Bat! (v3.02) Professional");
  		mail.AddMsgLine("Hello,");
		mail.AddMsgLine("");
		mail.AddMsgLine("...");
		mail.AddMsgLine("How are you today?");
		mail.AddMsgLine("");
		mail.AddMsgLine("Regards");
		mail.ModMsgLine(5,"regards");
		mail.DelMsgLine(2);
		mail.AddMsgLine("User");
		
  		//mail.AddAttachment("../test1.jpg");
  		//mail.AddAttachment("c:\\test2.exe");
		//mail.AddAttachment("c:\\test3.txt");
		mail.Send();
	#else
		CSmtp mail;
		
		//printf("gmail_send_ssl-1,SetSMTPServer=%s\n",s_mailserver);
		
		mail.SetSMTPServer(s_mailserver,465);//"smtp.gmail.com"
		mail.SetSecurityType(CSmtp::USE_SSL);
		
		//printf("gmail_send_ssl-2,SetLogin=%s,SetPassword=%s\n",s_mail_from,s_password);
		
		mail.SetLogin(s_mail_from);//s_user//s_mail_from//"wuhanruizhe@gmail.com"
		mail.SetPassword(s_password);
		
		//printf("gmail_send_ssl-3,SetSenderName=%s,SetSenderMail=%s,SetReplyTo=%s,SetSubject=%s,AddRecipient=%s\n",
		//	s_name_from,s_mail_from,s_mail_from,s_subject,s_mail_to);
		
		mail.SetSenderName(s_name_from);//"wuhanruizhe"
  		mail.SetSenderMail(s_mail_from);//"wuhanruizhe@gmail.com"
  		mail.SetReplyTo(s_mail_from);//"wuhanruizhe@gmail.com"
  		
  		mail.SetSubject(s_subject);//"The message"
  		mail.AddRecipient(s_mail_to);//"57637527@qq.com"//(s_mail_to,s_name_to)
  		
  		mail.SetXPriority(XPRIORITY_NORMAL);
  		mail.SetXMailer("The Bat! (v3.02) Professional");
		
		#if 1
		//printf("gmail_send_ssl-4.1,AddMsgLine=%s\n",s_body);
		
		#if 0
		int len = strlen(s_body);
		if(len >= 1)
		{
			if(s_body[len-1] == '\n')
			{
				s_body[len-1] = '\0';
			}
			
			if(len >= 2)
			{
				if(s_body[len-2] == '\r')
				{
					s_body[len-2] = '\0';
				}
			}
		}
		#endif
		
		//printf("gmail_send_ssl-4.2,AddMsgLine=%s\n",s_body);
		
		mail.AddMsgLine(s_body);
		#else
  		mail.AddMsgLine("Hello,");
		mail.AddMsgLine("");
		mail.AddMsgLine("...");
		mail.AddMsgLine("How are you today?");
		mail.AddMsgLine("");
		mail.AddMsgLine("Regards");
		mail.ModMsgLine(5,"regards");
		mail.DelMsgLine(2);
		mail.AddMsgLine("User");
		#endif
		
		//printf("gmail_send_ssl-5\n");
		
  		//mail.AddAttachment("../test1.jpg");
  		//mail.AddAttachment("c:\\test2.exe");
		//mail.AddAttachment("c:\\test3.txt");
		mail.Send();
		
		//printf("gmail_send_ssl-6\n");
	#endif
	}
	catch(ECSmtp e)
	{
		//std::cout << "Error: " << e.GetErrorText().c_str() << ".\n";
		printf("Error: %s.\n",e.GetErrorText().c_str());
		bError = true;
	}
	if(!bError)
	{
		std::cout << "Mail was send successfully.\n";
	}
	else
	{
		std::cout << "Mail was send failed.\n";
	}
	return !bError;
}

extern "C" BOOL send_gmail_ssl(char *s_name_from, char *s_mail_from, char *s_name_to, char *s_mail_to, char *s_user, char *s_password, char *s_mailserver, char *s_subject, char *s_body, char *s_attached, char *s_encode_type)
{
	return gmail_send_ssl(s_name_from, s_mail_from, s_name_to, s_mail_to, s_user, s_password, s_mailserver, s_subject, s_body, s_attached, s_encode_type);
}

