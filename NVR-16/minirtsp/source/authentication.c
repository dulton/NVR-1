/******************************************************************************

  Copyright (C), 2013-2020, Juan Optical & Electronical Tech Co., Ltd. All Rights Reserved.

 ******************************************************************************
  File Name    : authentication.c
  Version       : Initial Draft
  Author        : kejiazhw@gmail.com(kaga)
  Created       : 2013/04/25
  Last Modified : 2013/04/25
  Description   : http authentication util
 
  History       : 
  1.Date        : 2013/04/25
    	Author      : kaga
 	Modification: Created file	
******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "authentication.h"
#include "_base64.h"
#include "md5.h"

#ifndef TRUE
#define TRUE	(1)
#endif
#ifndef FALSE
#define FALSE	(0)
#endif

#ifndef NULL
#define NULL ((void *)0)
#endif


#define AUTH_DEFAULT_USER	"admin"
#define AUTH_DEFAULT_PWD	"12345"


static void digest_challenge(char *out)
{
	srand(time(NULL));
	sprintf(out,"%lu%lu%u",time(NULL)%13579,time(NULL),rand());
}

static int auth_user_validate(char *usr,char *pwd)
{
	if((strcmp(usr,AUTH_DEFAULT_USER)==0) &&
		(strcmp(pwd,AUTH_DEFAULT_PWD)==0)){
		return TRUE;
	}else{
		return FALSE;
	}
}


static int basic_setup(Authentication_t *auth)
{
	char tmp[128];
	int ret;
	sprintf(tmp,"%s:%s",auth->user,auth->pwd);
	if((ret=BASE64_encode(tmp,strlen(tmp),auth->responce,sizeof(auth->responce)))< 0)
		return AUTH_RET_FAIL;

	printf("AUTH_DBG: Basic setup: user:%s pwd:%s %s\n",auth->user,auth->pwd,auth->responce);
	return AUTH_RET_OK;
}


static int basic_validate(Authentication_t *auth)
{
	char dst[128];
	int ret;
	
	ret = BASE64_decode(auth->responce,strlen(auth->responce),dst,sizeof(dst));
	if(ret< 0)
		return FALSE;
	dst[ret]=0;
	ret=sscanf(dst,"%[^:]:%s",auth->user,auth->pwd);
	if(ret != 2)
		return FALSE;
	//printf("AUTH_DBG: Basic Validate Pass: user:%s pwd:%s (%s)\n",auth->user,auth->pwd,auth->responce);

	return auth_user_validate(auth->user,auth->pwd);
}

static int digest_setup(Authentication_t *auth)
{
	/*response= md5(md5(username:realm:password):nonce:md5(public_method:url));*/
	char temp_1[33];
	char temp_2[33];
	//char temp_3[33];
	char buf_1[32+1+32+1+128+1];
	char buf_2[32+1+128+1];
	char buf_3[32+1+32+1+32+1];
	sprintf(buf_1,"%s:%s:%s",auth->user,auth->realm,auth->pwd);
	sprintf(buf_2,"%s:%s",auth->method,auth->url);
	MD5_encode(buf_1,temp_1);
	/////////////
	MD5_encode(buf_2,temp_2);
	sprintf(buf_3,"%s:%s:%s",temp_1,auth->nonce,temp_2);
	MD5_encode(buf_3,auth->responce);
	return AUTH_RET_OK;
}

//static int digest_validate(Authentication_t *auth)
int digest_validate(Authentication_t *auth)
{
	return TRUE;
}

int HTTP_AUTH_client_init(
	struct _authentication **auth,
	char *www_authenticate/*the value in the domain of <WWW-Authenticate>,not contain CRLF*/
)
{
	char *ptr = NULL;
	//
	if(*auth != NULL) return AUTH_RET_OK;
	//
	*auth = (Authentication_t *)malloc(sizeof(Authentication_t));
	if(*auth == NULL)
	{
		printf("HTTP-AUTH ERROR: malloc for auth failed");
		return AUTH_RET_FAIL;
	}
	memset(*auth,0,sizeof(Authentication_t));
	
	if(strncmp(www_authenticate,"Basic",strlen("Basic")) == 0)
	{
		//Basic 认证
		(*auth)->type = HTTP_AUTH_BASIC;
		ptr = strstr(www_authenticate,"realm");
		if(ptr) sscanf(ptr,"realm=\"%[^\"]",(*auth)->realm);
	}
	/*添加digest认证方法*/
	else if(strncmp(www_authenticate,"Digest",strlen("Digest")) == 0)
	{
		(*auth)->type = HTTP_AUTH_DIGEST;
		ptr = strstr(www_authenticate,"realm");
		if(ptr) sscanf(ptr,"realm=\"%[^\"]",(*auth)->realm);
		ptr = strstr(www_authenticate,"nonce");
		if(ptr) sscanf(ptr,"nonce=\"%[^\"]",(*auth)->nonce);
	}
	else
	{
		printf("HTTP-AUTH ERROR: invalid WWW-Authenticate format\n");
		free(*auth);
		return AUTH_RET_FAIL;
	}
	
	return AUTH_RET_OK;
}

int HTTP_AUTH_setup(
	struct _authentication *auth,
	char *username,char *password,
	char *url,char *method, /* if use digest ,must given these two parameters,else ignore these*/
	char *out,int out_size)
{
	char DigestFormat[] = "Digest username=\"%s\", realm=\"%s\", nonce=\"%s\", uri=\"%s\", response=\"%s\"\r\n";
	int ret;
	
	//
	if(auth == NULL || out == NULL || username == NULL || password == NULL)
	{
		printf("HTTP-AUTH ERROR: invalid parameter*********!\n");
		return AUTH_RET_FAIL;
	}
	if(auth->type == HTTP_AUTH_DIGEST)
	{
		if(url== NULL || method == NULL)
		{
			printf("HTTP-AUTH ERROR: invalid parameter!**\n");
			return AUTH_RET_FAIL;
		}
	}
	//
	
	strcpy(auth->user,username);
	strcpy(auth->pwd,password);
	if(url) strcpy(auth->url,url);
	if(method) strcpy(auth->method,method);
	
	if(auth->type == HTTP_AUTH_BASIC)
	{
		//printf("HTTP_AUTH_setup: basic\n");
		//fflush(stdout);
		if(basic_setup(auth) == AUTH_RET_FAIL) return AUTH_RET_FAIL;
		ret = sprintf(out,"Basic %s",auth->responce);
		if(ret > out_size)
		{
			printf("HTTP-AUTH ERROR: maybe given buffer is too small!\n");
			return AUTH_RET_FAIL;
		}
	}
	/*digest auth*/
	else if(auth->type == HTTP_AUTH_DIGEST)
	{
		//printf("HTTP_AUTH_setup: digest\n");
		//fflush(stdout);
		if(digest_setup(auth) == AUTH_RET_FAIL) return AUTH_RET_FAIL;
		ret = sprintf(out,DigestFormat,auth->user,auth->realm,auth->nonce,auth->url,auth->responce);
		if(ret > out_size)
		{
			printf("HTTP-AUTH ERROR: maybe given buffer is too small!\n");
			return AUTH_RET_FAIL;
		}
	}
	
	return AUTH_RET_OK;
}

// use for server
int HTTP_AUTH_server_init(struct _authentication **auth,int type)
{
	if(*auth != NULL) return AUTH_RET_OK;
	//
	*auth=(Authentication_t *)malloc(sizeof(Authentication_t));
	if(*auth == NULL)
	{
		printf("AUTH_ERR:malloc for auth failed");
		return AUTH_RET_FAIL;
	}
	memset(*auth,0,sizeof(Authentication_t));
	strcpy((*auth)->realm,HTTP_AUTH_REALM);
	(*auth)->type = type;
	return AUTH_RET_OK;
}

int HTTP_AUTH_chanllenge(struct _authentication *auth,char *out,int out_size)
{
	int ret;
	if(auth == NULL || out == NULL){
		printf("HTTP-AUTH ERROR: invalid parameter!\n");
		return AUTH_RET_FAIL;
	}
	if(auth->type == HTTP_AUTH_BASIC){
		ret = sprintf(out,"Basic realm=\"%s\"",auth->realm);
		if(ret > out_size){
			printf("HTTP-AUTH ERROR: maybe given buffer is too small!\n");
			return AUTH_RET_FAIL;
		}
	}else{
		digest_challenge(auth->nonce);
		ret = sprintf(out,"Digest realm=\"%s\", nonce=\"%s\"",auth->realm,auth->nonce);
		if(ret > out_size){
			printf("HTTP-AUTH ERROR: maybe given buffer is too small!\n");
			return AUTH_RET_FAIL;
		}
	}
	return AUTH_RET_OK;
}

int HTTP_AUTH_validate(struct _authentication *auth,
	char *authorization,/* the value in the domain of <Authorization>,not contain CRLF */
	char *method)
{
	char *ptr = NULL;
	//char nonce[128];
	//char realm[128];
	//char algorithm[32];
	int ret = AUTH_RET_FAIL;
	if(auth == NULL || authorization == NULL){
		printf("HTTP-AUTH ERROR: invalid parameter!\n");
		return FALSE;
	}
	
	if(strncmp(authorization,"Basic",strlen("Basic")) == 0){
		if(auth->type != HTTP_AUTH_BASIC){
			printf("HTTP-AUTH ERROR: invalid authorization type,expect Basic!\n");
			return FALSE;
		}
		ptr = authorization + strlen("Basic");
		ptr++; // empty space
		strcpy(auth->responce,ptr);
		// validate
		ret = basic_validate(auth);
	}
	
	return ret;
}

int HTTP_AUTH_destroy(struct _authentication *auth)
{
	if(auth){
		free(auth);
	}
	return AUTH_RET_OK;
}

