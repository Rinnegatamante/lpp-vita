/*----------------------------------------------------------------------------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------#
#------  This File is Part Of : ----------------------------------------------------------------------------------------#
#------- _  -------------------  ______   _   --------------------------------------------------------------------------#
#------ | | ------------------- (_____ \ | |  --------------------------------------------------------------------------#
#------ | | ---  _   _   ____    _____) )| |  ____  _   _   ____   ____   ----------------------------------------------#
#------ | | --- | | | | / _  |  |  ____/ | | / _  || | | | / _  ) / ___)  ----------------------------------------------#
#------ | |_____| |_| |( ( | |  | |      | |( ( | || |_| |( (/ / | |  --------------------------------------------------#
#------ |_______)\____| \_||_|  |_|      |_| \_||_| \__  | \____)|_|  --------------------------------------------------#
#------------------------------------------------- (____/  -------------------------------------------------------------#
#------------------------   ______   _   -------------------------------------------------------------------------------#
#------------------------  (_____ \ | |  -------------------------------------------------------------------------------#
#------------------------   _____) )| | _   _   ___   ------------------------------------------------------------------#
#------------------------  |  ____/ | || | | | /___)  ------------------------------------------------------------------#
#------------------------  | |      | || |_| ||___ |  ------------------------------------------------------------------#
#------------------------  |_|      |_| \____|(___/   ------------------------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------#
#- Licensed under the GPL License --------------------------------------------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------#
#- Copyright (c) Nanni <lpp.nanni@gmail.com> ---------------------------------------------------------------------------#
#- Copyright (c) Rinnegatamante <rinnegatamante@gmail.com> -------------------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------#
#- Credits : -----------------------------------------------------------------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------#
#- All the devs involved in Rejuvenate and vita-toolchain --------------------------------------------------------------#
#- xerpi for drawing libs and for FTP server code ----------------------------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------*/
#define NET_INIT_SIZE 1*1024*1024
#include <vitasdk.h>
#include "include/luaplayer.h"
extern "C"{
	#include "include/ftp/ftp.h"
}
#define stringify(str) #str
#define VariableRegister(lua, value) do { lua_pushinteger(lua, value); lua_setglobal (lua, stringify(value)); } while(0)

static void* net_memory = NULL;
static char vita_ip[16];
static bool isNet = false;

typedef struct
{
	uint32_t magic;
	int sock;
	bool serverSocket;
} Socket;

#define MAX_NAME 512
struct hostent{
  char  *h_name;         /* official (cannonical) name of host               */
  char **h_aliases;      /* pointer to array of pointers of alias names      */
  int    h_addrtype;     /* host address type: AF_INET                       */
  int    h_length;       /* length of address: 4                             */
  char **h_addr_list;    /* pointer to array of pointers with IPv4 addresses */
};
#define h_addr h_addr_list[0]

// Copy-pasted from xyz code
static struct hostent *gethostbyname(const char *name)
{
    static hostent ent;
    static char sname[MAX_NAME] = "";
    static SceNetInAddr saddr = { 0 };
    static char *addrlist[2] = { (char *) &saddr, NULL };

    int rid;
    int err;
    rid = sceNetResolverCreate("resolver", NULL, 0);
    if(rid < 0) {
        return NULL;
    }

    err = sceNetResolverStartNtoa(rid, name, &saddr, 0, 0, 0);
    sceNetResolverDestroy(rid);
    if(err < 0) {
        return NULL;
    }

    ent.h_name = sname;
    ent.h_aliases = 0;
    ent.h_addrtype = SCE_NET_AF_INET;
    ent.h_length = sizeof(struct SceNetInAddr);
    ent.h_addr_list = addrlist;
    ent.h_addr = addrlist[0];

    return &ent;
}

static int lua_initFTP(lua_State *L) {
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
	if (!isNet) return luaL_error(L, "Network is not inited");
	#endif
	char vita_ip[16];
	unsigned short int vita_port = 0;
	if (ftpvita_init(vita_ip, &vita_port) < 0) return luaL_error(L, "cannot start FTP server (WiFi off?)");
	ftpvita_add_device("app0:");
	ftpvita_add_device("ux0:");
	ftpvita_add_device("ur0:");
	ftpvita_add_device("music0:");
	ftpvita_add_device("photo0:");
	lua_pushstring(L, vita_ip);
	lua_pushinteger(L, vita_port);
	return 2;
}

static int lua_termFTP(lua_State *L) {
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
	if (!isNet) return luaL_error(L, "Network is not inited");
	#endif
	ftpvita_fini();
	return 0;
}

static int lua_getip(lua_State *L) {
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
	if (!isNet) return luaL_error(L, "Network is not inited");
	#endif
	SceNetCtlInfo info;
	if (sceNetCtlInetGetInfo(SCE_NETCTL_INFO_GET_IP_ADDRESS, &info) < 0) strcpy(vita_ip, "127.0.0.1");
	else strcpy(vita_ip, info.ip_address);
	lua_pushstring(L, vita_ip);
	return 1;
}

static int lua_getmac(lua_State *L) {
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
	if (!isNet) return luaL_error(L, "Network is not inited");
	#endif
	SceNetEtherAddr mac;
	char macAddress[32];
	sceNetGetMacAddress(&mac, 0);	
	sprintf(macAddress, "%02X:%02X:%02X:%02X:%02X:%02X", mac.data[0], mac.data[1], mac.data[2], mac.data[3], mac.data[4], mac.data[5]);
	lua_pushstring(L, macAddress);
	return 1;
}

static int lua_init(lua_State *L) {
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
	if (isNet) return luaL_error(L, "Network is already inited");
	#endif
	int ret = sceNetShowNetstat();
	SceNetInitParam initparam;
	if (ret == SCE_NET_ERROR_ENOTINIT) {
		net_memory = malloc(NET_INIT_SIZE);
		initparam.memory = net_memory;
		initparam.size = NET_INIT_SIZE;
		initparam.flags = 0;
		ret = sceNetInit(&initparam);
		if (ret < 0) return luaL_error(L, "an error occurred while starting network.");
	}
	sceNetCtlInit();
	sceHttpInit(1*1024*1024);
	isNet = 1;
	return 0;
}

static int lua_wifi(lua_State *L) {
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
	if (!isNet) return luaL_error(L, "Network is not inited");
	#endif
	int state;
	sceNetCtlInetGetState(&state);
	lua_pushboolean(L, state);
	return 1;
}

static int lua_wifilv(lua_State *L) {
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
	if (!isNet) return luaL_error(L, "Network is not inited");
	#endif
	int state;
	SceNetCtlInfo info;
	sceNetCtlInetGetInfo(SCE_NETCTL_INFO_GET_RSSI_PERCENTAGE, &info);
	lua_pushinteger(L, info.rssi_percentage);
	return 1;
}

static int lua_term(lua_State *L) {
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
	if (!isNet) return luaL_error(L, "Network is not inited");
	#endif
	sceHttpTerm();
	sceNetCtlTerm();
	sceNetTerm();
	if (net_memory != NULL) free(net_memory);
	net_memory = NULL;
	isNet = 0;
	return 0;
}

static int lua_createserver(lua_State *L) {
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 1 && argc != 2) return luaL_error(L, "Socket.createServerSocket(port) takes one argument.");
	if (!isNet) return luaL_error(L, "Network is not inited");
	#endif
	int port = luaL_checkinteger(L, 1);
	int type = SCE_NET_IPPROTO_TCP;
	if (argc == 2) type = luaL_checkinteger(L, 2);
	
	Socket* my_socket = (Socket*) malloc(sizeof(Socket));
	my_socket->serverSocket = true;

	if (type == SCE_NET_IPPROTO_TCP) my_socket->sock = sceNetSocket("Server Socket", SCE_NET_AF_INET, SCE_NET_SOCK_STREAM, 0);
	else my_socket->sock = sceNetSocket("Server Socket", SCE_NET_AF_INET, SCE_NET_SOCK_DGRAM, SCE_NET_IPPROTO_UDP);
	#ifndef SKIP_ERROR_HANDLING
	if (my_socket->sock <= 0) return luaL_error(L, "invalid socket.");
	#endif

	int _true = 1;
	SceNetSockaddrIn addrTo;
	addrTo.sin_family = SCE_NET_AF_INET;
	addrTo.sin_port = sceNetHtons(port);
	if (type == SCE_NET_IPPROTO_TCP) addrTo.sin_addr.s_addr = 0;
	else{
		SceNetCtlInfo info;
		sceNetCtlInetGetInfo(SCE_NETCTL_INFO_GET_IP_ADDRESS, &info);
		sceNetInetPton(SCE_NET_AF_INET, info.ip_address, &addrTo.sin_port);
	}
	
	int err = sceNetBind(my_socket->sock, (SceNetSockaddr*)&addrTo, sizeof(addrTo));
	#ifndef SKIP_ERROR_HANDLING
	if (err != 0) return luaL_error(L, "bind error.");
	#endif

	sceNetSetsockopt(my_socket->sock, SCE_NET_SOL_SOCKET, SCE_NET_SO_NBIO, &_true, sizeof(_true));
	
	if (type == SCE_NET_IPPROTO_TCP){
		err = sceNetListen(my_socket->sock, 1);
		#ifndef SKIP_ERROR_HANDLING
		if (err != 0) return luaL_error(L, "listen error.");
		#endif
	}
	
	my_socket->magic = 0xDEADDEAD;
	lua_pushinteger(L,(uint32_t)my_socket);
	return 1;
}

static int lua_send(lua_State *L)
{
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING	
	if (argc != 2) return luaL_error(L, "wrong number of arguments");
	if (!isNet) return luaL_error(L, "Network is not inited");
	#endif
	
	Socket* my_socket = (Socket*)luaL_checkinteger(L, 1);
	size_t size;
	char* text = (char*)luaL_checklstring(L, 2, &size);

	#ifndef SKIP_ERROR_HANDLING
	if (my_socket->magic != 0xDEADDEAD) return luaL_error(L, "attempt to access wrong memory block type");
	if (my_socket->serverSocket) return luaL_error(L, "send not allowed for server sockets.");
	if (!text) return luaL_error(L, "Socket.send() expected a string.");
	#endif
	
	int result = sceNetSend(my_socket->sock, text, size, 0);
	lua_pushinteger(L, result);
	return 1;
}

static int lua_recv(lua_State *L)
{
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 2) return luaL_error(L, "wrong number of arguments");
	if (!isNet) return luaL_error(L, "Network is not inited");
	#endif
	
	Socket* my_socket = (Socket*)luaL_checkinteger(L, 1);
	uint32_t size = luaL_checkinteger(L, 2);
	
	#ifndef SKIP_ERROR_HANDLING
	if (my_socket->magic != 0xDEADDEAD) return luaL_error(L, "attempt to access wrong memory block type");			
	if (my_socket->serverSocket) return luaL_error(L, "recv not allowed for server sockets.");
	#endif

	char* data = (char*)malloc(size);
	int count = sceNetRecv(my_socket->sock, data, size, 0);
	if (count > 0) lua_pushlstring(L, data, count);
	else lua_pushstring(L, "");
	free(data);
	return 1;
}

static int lua_accept(lua_State *L)
{
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "wrong number of arguments");
	if (!isNet) return luaL_error(L, "Network is not inited");
	#endif
	
	Socket* my_socket = (Socket*)luaL_checkinteger(L, 1);
	
	#ifndef SKIP_ERROR_HANDLING
	if (my_socket->magic != 0xDEADDEAD) return luaL_error(L, "attempt to access wrong memory block type");
	if (!my_socket->serverSocket) return luaL_error(L, "accept allowed for server sockets only.");
	#endif

	SceNetSockaddrIn addrAccept;
	unsigned int cbAddrAccept = sizeof(addrAccept);
	int sockClient = sceNetAccept(my_socket->sock, (SceNetSockaddr*)&addrAccept, &cbAddrAccept);
	if (sockClient <= 0) return 0;

	Socket* incomingSocket = (Socket*) malloc(sizeof(Socket));
	incomingSocket->serverSocket = 0;
	incomingSocket->sock = sockClient;
	incomingSocket->magic = 0xDEADDEAD;
	int rcvbuf = 32768;
	int _true = 1;
	sceNetSetsockopt(my_socket->sock, SCE_NET_SOL_SOCKET, SCE_NET_SO_NBIO, &_true, sizeof(_true));
	lua_pushinteger(L, (uint32_t)incomingSocket);
	return 1;
}

static int lua_closeSock(lua_State *L)
{
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 1) return luaL_error(L, "Socket.close() takes one argument.");
	if (!isNet) return luaL_error(L, "Network is not inited");
	#endif
	
	Socket* my_socket = (Socket*)luaL_checkinteger(L, 1);
	
	#ifndef SKIP_ERROR_HANDLING
	if (my_socket->magic != 0xDEADDEAD) return luaL_error(L, "attempt to access wrong memory block type");
	#endif
	
	sceNetSocketClose(my_socket->sock);
	free(my_socket);
	return 0;
}

static int lua_connect(lua_State *L)
{
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc != 2 && argc != 3)  return luaL_error(L, "wrong number of arguments");
	if (!isNet) return luaL_error(L, "Network is not inited");
	#endif
	
	// Getting arguments
	char *host = (char*)luaL_checkstring(L, 1);
	int port = luaL_checkinteger(L, 2);
	int type = SCE_NET_IPPROTO_TCP;
	if (argc == 3) type = luaL_checkinteger(L, 3);
	char port_str[8];
	sprintf(port_str,"%i",port);
	
	// Allocating Socket memblock
	Socket* my_socket = (Socket*) malloc(sizeof(Socket));
	my_socket->serverSocket = false;
	my_socket->magic = 0xDEADDEAD;
	
	// Creating socket
	if (type == SCE_NET_IPPROTO_TCP) my_socket->sock = sceNetSocket("Socket", SCE_NET_AF_INET, SCE_NET_SOCK_STREAM, 0);
	else my_socket->sock = sceNetSocket("Socket", SCE_NET_AF_INET, SCE_NET_SOCK_DGRAM, SCE_NET_IPPROTO_UDP);
	#ifndef SKIP_ERROR_HANDLING
	if (my_socket->sock < 0){
		free(my_socket);
		return luaL_error(L, "Failed creating socket.");
	}
	#endif
	
	// Resolving host
	hostent* hostentry = gethostbyname(host);
	SceNetSockaddrIn addrTo;
	addrTo.sin_family = SCE_NET_AF_INET;
	addrTo.sin_port = sceNetHtons(port);
	addrTo.sin_addr.s_addr = *(int *)hostentry->h_addr_list[0];
	
	// Connecting to the server if TCP socket
	if (type == SCE_NET_IPPROTO_TCP){
		int res = sceNetConnect(my_socket->sock, (SceNetSockaddr*)&addrTo, sizeof(SceNetSockaddrIn));
		#ifndef SKIP_ERROR_HANDLING
		if(res < 0){
			sceNetSocketClose(my_socket->sock);
			free(my_socket);
			return luaL_error(L, "Failed connecting to the server.");
		}
		#endif
	}
	
	// Setting socket options
	int _true = 1;
	sceNetSetsockopt(my_socket->sock, SCE_NET_SOL_SOCKET, SCE_NET_SO_NBIO, &_true, sizeof(_true));	
	
	lua_pushinteger(L, (uint32_t)my_socket);
	return 1;
}

static int lua_download(lua_State *L){
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc < 2 || argc > 5) return luaL_error(L, "wrong number of arguments");
	#endif
	const char* url = luaL_checkstring(L,1);
	const char* file = luaL_checkstring(L,2);
	const char* useragent = (argc >= 3) ? luaL_checkstring(L,3) : "lpp-vita app";
	uint8_t method = (argc >= 4) ? luaL_checkinteger(L,4) : SCE_HTTP_METHOD_GET;
	const char* postdata = (argc >= 5) ? luaL_checkstring(L,5) : NULL;
	int postsize = (argc >= 5) ? strlen(postdata) : 0;
	int tpl = sceHttpCreateTemplate(useragent, 1, 1);	
	int conn = sceHttpCreateConnectionWithURL(tpl, url, 0);
	int request = sceHttpCreateRequestWithURL(conn, method, url, 0);
	int handle = sceHttpSendRequest(request, postdata, postsize);
	int fh = sceIoOpen(file, SCE_O_WRONLY | SCE_O_CREAT, 0777);
	unsigned char data[16*1024];
	int read = 0;
	while ((read = sceHttpReadData(request, &data, sizeof(data))) > 0){
		int write = sceIoWrite(fh, data, read);
	}
	sceIoClose(fh);
	return 0;
}

static int lua_string(lua_State *L){
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
	if (argc < 2 || argc > 5) return luaL_error(L, "wrong number of arguments");
	#endif
	const char* url = luaL_checkstring(L,1);
	const char* file = luaL_checkstring(L,2);
	const char* headers = (argc >= 3) ? luaL_checkstring(L,3) : NULL;
	uint8_t method = (argc >= 4) ? luaL_checkinteger(L,4) : SCE_HTTP_METHOD_GET;
	const char* postdata = (argc >= 5) ? luaL_checkstring(L,5) : NULL;
	int postsize = (argc >= 5) ? strlen(postdata) : 0;
	int tpl = sceHttpCreateTemplate("lpp-vita app", 1, 1);	
	int conn = sceHttpCreateConnectionWithURL(tpl, url, 0);
	int request = sceHttpCreateRequestWithURL(conn, method, url, 0);
	int handle = sceHttpSendRequest(request, postdata, postsize);
	unsigned char *buffer = (unsigned char*)malloc(0x1000);
	int read = 0;
	int i = 0;
	int len = 0x1000;
	while ((read = sceHttpReadData(request, &buffer[i], len-i)) > 0){
		i += read;
		if (i >= len){
			len += 0x1000;
			buffer = (unsigned char*)realloc(buffer, len);
		}
	}
	buffer = (unsigned char*)realloc(buffer, i+1);
	buffer[i] = 0;
	lua_pushlstring(L,(const char*)buffer,i);
	free(buffer);
	return 0;
}

//Register our Network Functions
static const luaL_Reg Network_functions[] = {
  {"init",           lua_init},
  {"term",           lua_term},
  {"initFTP",        lua_initFTP},
  {"termFTP",        lua_termFTP},
  {"getIPAddress",   lua_getip},
  {"getMacAddress",  lua_getmac},
  {"isWifiEnabled",  lua_wifi},
  {"getWifiLevel",   lua_wifilv},
  {"downloadFile",   lua_download},
  {"requestString",  lua_string},
  {0, 0}
};

//Register our Socket Functions
static const luaL_Reg Socket_functions[] = {
  {"createServerSocket", lua_createserver},
  {"send",               lua_send},
  {"receive",            lua_recv},
  {"accept",             lua_accept},
  {"close",              lua_closeSock},
  {"connect",            lua_connect},
  {0, 0}
};

void luaNetwork_init(lua_State *L) {
	uint8_t GET_METHOD = SCE_HTTP_METHOD_GET;
	uint8_t POST_METHOD = SCE_HTTP_METHOD_POST;
	uint8_t HEAD_METHOD = SCE_HTTP_METHOD_HEAD;
	uint8_t OPTIONS_METHOD = SCE_HTTP_METHOD_OPTIONS;
	uint8_t PUT_METHOD = SCE_HTTP_METHOD_PUT;
	uint8_t DELETE_METHOD = SCE_HTTP_METHOD_DELETE;
	uint8_t TRACE_METHOD = SCE_HTTP_METHOD_TRACE;
	uint8_t CONNECT_METHOD = SCE_HTTP_METHOD_CONNECT;
	uint8_t UDP_SOCKET = SCE_NET_IPPROTO_UDP;
	uint8_t TCP_SOCKET = SCE_NET_IPPROTO_TCP;
	VariableRegister(L,UDP_SOCKET);
	VariableRegister(L,TCP_SOCKET);
	VariableRegister(L,GET_METHOD);
	VariableRegister(L,POST_METHOD);
	VariableRegister(L,HEAD_METHOD);
	VariableRegister(L,OPTIONS_METHOD);
	VariableRegister(L,PUT_METHOD);
	VariableRegister(L,DELETE_METHOD);
	VariableRegister(L,TRACE_METHOD);
	VariableRegister(L,CONNECT_METHOD);
	lua_newtable(L);
	luaL_setfuncs(L, Network_functions, 0);
	lua_setglobal(L, "Network");
	lua_newtable(L);
	luaL_setfuncs(L, Socket_functions, 0);
	lua_setglobal(L, "Socket");
}