#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#define MAX_PROTOCOL_LEN 20

typedef struct
{
	char nChn;
	char nProtocolName[20];
}SPtzParam;

typedef struct
{
	char startup[MAX_PROTOCOL_LEN];
	char startdown[MAX_PROTOCOL_LEN];
}SPtzRealCmd;

SPtzRealCmd g_PtzCmd[4];
SPtzParam g_PtzPara[4];
lua_State* L;

void CheckByte(char* src)
{
	int top1 = lua_gettop(L);
	lua_newtable(L);
	
	int i;
	for(i = 0; i < *src; i++)
	{
		lua_pushnumber(L, i + 1);
		lua_pushnumber(L, *(src + i +1));
		lua_settable(L, -3);		
	}
	lua_getglobal(L, "ptz");
	lua_pushstring(L, "Checksum");
	lua_gettable(L, -2);
	lua_pushvalue(L, -3);
	lua_pcall(L, 1, 1, 0);
	lua_pushnumber(L, *src);
	lua_gettable(L, -2);
	*(src + (*src)) = lua_tonumber(L, -1);
	
	int top2 = lua_gettop(L);
	lua_pop(L, (top2 > top1) ? (top2 - top1) : 0);
}

void ReadDataFromLua(char* data, char* elem)
{
	lua_getglobal(L, "ptz");
	lua_pushstring(L, "cmd");
	lua_gettable(L, -2);
	lua_pushstring(L, "start");
	lua_gettable(L, -2);
	lua_pushstring(L, elem);
	lua_gettable(L, -2);
	int i = 1;
	while(1)
	{
		lua_pushnumber(L, i);
		lua_gettable(L, -2);
		if(lua_isnil(L, -1))
		{
			printf("data num = %d\n", --i);
			*data = i;
			break;
		}
		*(data + i) = lua_tonumber(L, -1);
		i++;
		lua_pop(L, 1);
	}
	lua_pop(L, 4);
}

void Print(char* data)
{
	int i;

	for(i = 0; i < *data; i++)
	{
		printf("0x%02x ", data[i+1]);
	}
	printf("\n");
}

int CheckProtocol(char* ProtocalName)
{
	L = lua_open();
	lua_baselibopen(L);
	lua_dofile(L, "PTZ_LIST");

	const char* name;//[20] = {0};

	lua_getglobal(L, "PtzProtocol");
	lua_pushstring(L, "List");
	lua_gettable(L, -2);

	int i = 1;
	int ret = 0;
	while(1)
	{
		//memset(name, 0, sizeof(name));
		lua_pushnumber(L, i);
		lua_gettable(L, -2);
		if(lua_isnil(L, -1))
		{
			printf("The system is not support protocol [%s] !\n", ProtocalName);
			ret = -1;
			break;
		}
		name = lua_tostring(L, -1);
		if(0 == strcasecmp(ProtocalName, name))
		{
			strcpy(ProtocalName, name);
			printf("Find protocol OK!\n");
			break;
		}
		lua_pop(L, 1);
		i++;
	}
	lua_close(L);
	return ret;
}

int LoadProtocol(char nChn, char* FileName)
{
	int ret = 0;
	L = lua_open();
	lua_baselibopen(L);
	lua_mathlibopen(L);
	lua_dofile(L, FileName);

	char* elem = "up";
	ReadDataFromLua(g_PtzCmd[nChn].startup, elem);
	Print(g_PtzCmd[nChn].startup);
	CheckByte(g_PtzCmd[nChn].startup);
	Print(g_PtzCmd[nChn].startup);

	elem = "down";
	ReadDataFromLua(g_PtzCmd[nChn].startdown, elem);
	Print(g_PtzCmd[nChn].startdown);

	lua_close(L);
	return ret;
}

void PtzSetParam(SPtzParam* para)
{
	memcpy(&g_PtzPara[para->nChn], para, sizeof(SPtzParam));
}

int main()
{
	memset(g_PtzCmd, 0, sizeof(g_PtzCmd));
	memset(g_PtzPara, 0, sizeof(g_PtzPara));

	SPtzParam para;
	memset(&para, 0, sizeof(para));
	para.nChn = 0;

	char cmd[20] = {0};
	while(1)
	{
		scanf("%s", cmd);

		if(0 == strcasecmp(cmd, "quit"))
		{
			break;
		}

		strcpy(para.nProtocolName, cmd);
		PtzSetParam(&para);

		if(0 == CheckProtocol(g_PtzPara[0].nProtocolName))
		{
			if(0 != access(g_PtzPara[0].nProtocolName, F_OK))
			{
				printf("Warning: Protocol file is lost!\n");
			}
			else
			{
				LoadProtocol(0, g_PtzPara[0].nProtocolName);
			}
		}
	}
	return 0;
/*
	int i;
	char name[3][20] = {"Yaan", "B01", "samsung", }; 
	for(i = 0; i < 3; i++)
	{
		if(0 == CheckProtocol(name[i]))
		{
			printf("file: %s\n", name[i]);
		}
	}
	return 0;

	L = lua_open();
	lua_baselibopen(L);
	//lua_mathlibopen(L);
	lua_dofile(L, "TESTLUA");

	char data[10] = {0};

	//CheckByte(data);
	Print(data);

	char* elem = "up";
	ReadDataFromLua(data, elem);
	Print(data);

	elem = "down";
	ReadDataFromLua(data, elem);
	Print(data);

	return 0;
*/
}
