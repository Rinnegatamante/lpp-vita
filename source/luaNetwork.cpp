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
void* net_memory = NULL;
char vita_ip[16];

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
    if (argc != 0) return luaL_error(L, "wrong number of arguments");
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
    if (argc != 0) return luaL_error(L, "wrong number of arguments");
    ftpvita_fini();
    return 0;
}

static int lua_getip(lua_State *L) {
    int argc = lua_gettop(L);
    if (argc != 0) return luaL_error(L, "wrong number of arguments");
	SceNetCtlInfo info;
    if (sceNetCtlInetGetInfo(SCE_NETCTL_INFO_GET_IP_ADDRESS, &info) < 0) strcpy(vita_ip, "127.0.0.1");
	else strcpy(vita_ip, info.ip_address);
	lua_pushstring(L, vita_ip);
    return 1;
}

static int lua_getmac(lua_State *L) {
    int argc = lua_gettop(L);
    if (argc != 0) return luaL_error(L, "wrong number of arguments");
	SceNetEtherAddr mac;
	char macAddress[32];
	sceNetGetMacAddress(&mac, 0);	
	sprintf(macAddress, "%02X:%02X:%02X:%02X:%02X:%02X", mac.data[0], mac.data[1], mac.data[2], mac.data[3], mac.data[4], mac.data[5]);
	lua_pushstring(L, macAddress);
    return 1;
}

static int lua_initSock(lua_State *L) {
    int argc = lua_gettop(L);
    if (argc != 0) return luaL_error(L, "wrong number of arguments");
	int ret = sceNetShowNetstat();
	SceNetInitParam initparam;
	if (ret == SCE_NET_ERROR_ENOTINIT) {
		net_memory = malloc(NET_INIT_SIZE);
		initparam.memory = net_memory;
		initparam.size = NET_INIT_SIZE;
		initparam.flags = 0;
		ret = sceNetInit(&initparam);
	}
	sceNetCtlInit();
    return 0;
}

static int lua_termSock(lua_State *L) {
    int argc = lua_gettop(L);
    if (argc != 0) return luaL_error(L, "wrong number of arguments");
	sceNetCtlTerm();
	sceNetTerm();
	if (net_memory != NULL) free(net_memory);
    return 0;
}

static int lua_createserver(lua_State *L) {
    int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING
		if (argc != 1) return luaL_error(L, "Socket.createServerSocket(port) takes one argument.");
	#endif
	int port = luaL_checkinteger(L, 1);

	Socket* my_socket = (Socket*) malloc(sizeof(Socket));
	my_socket->serverSocket = true;

	my_socket->sock = sceNetSocket("Socket", SCE_NET_AF_INET, SCE_NET_SOCK_STREAM, 0);
	#ifndef SKIP_ERROR_HANDLING
		if (my_socket->sock <= 0) return luaL_error(L, "invalid socket.");
	#endif

	int _true = 1;
	SceNetSockaddrIn addrTo;
	addrTo.sin_family = SCE_NET_AF_INET;
	addrTo.sin_port = sceNetHtons(port);
	addrTo.sin_addr.s_addr = 0;

	int err = sceNetBind(my_socket->sock, (SceNetSockaddr*)&addrTo, sizeof(addrTo));
	#ifndef SKIP_ERROR_HANDLING
		if (err != 0) return luaL_error(L, "bind error.");
	#endif

	sceNetSetsockopt(my_socket->sock, SCE_NET_SOL_SOCKET, SCE_NET_SO_NBIO, &_true, sizeof(_true));

	err = sceNetListen(my_socket->sock, 1);
	#ifndef SKIP_ERROR_HANDLING
		if (err != 0) return luaL_error(L, "listen error.");
	#endif
	
	my_socket->magic = 0xDEADDEAD;
	lua_pushinteger(L,(uint32_t)my_socket);
return 1;
}

static int lua_send(lua_State *L)
{
	int argc = lua_gettop(L);
	#ifndef SKIP_ERROR_HANDLING	
		if (argc != 2) return luaL_error(L, "wrong number of arguments");
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
		if (argc != 2)  return luaL_error(L, "wrong number of arguments");
	#endif
	
	// Getting arguments
	char *host = (char*)luaL_checkstring(L, 1);
	int port = luaL_checkinteger(L, 2);
	char port_str[8];
	sprintf(port_str,"%i",port);
	
	// Allocating Socket memblock
	Socket* my_socket = (Socket*) malloc(sizeof(Socket));
	my_socket->serverSocket = false;
	my_socket->magic = 0xDEADDEAD;
	
	// Creating socket
	my_socket->sock = sceNetSocket("Socket", SCE_NET_AF_INET, SCE_NET_SOCK_STREAM, 0);
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
	
	// Connecting to the server
	int res = sceNetConnect(my_socket->sock, (SceNetSockaddr*)&addrTo, sizeof(SceNetSockaddrIn));
	#ifndef SKIP_ERROR_HANDLING
		if(res < 0){
			sceNetSocketClose(my_socket->sock);
			free(my_socket);
			return luaL_error(L, "Failed connecting to the server.");
		}
	#endif
	
	// Setting socket options
	int _true = 1;
	sceNetSetsockopt(my_socket->sock, SCE_NET_SOL_SOCKET, SCE_NET_SO_NBIO, &_true, sizeof(_true));	
	
	lua_pushinteger(L, (uint32_t)my_socket);
	return 1;
}

//Register our Network Functions
static const luaL_Reg Network_functions[] = {
  {"initFTP",				lua_initFTP},
  {"termFTP",				lua_termFTP},
  {"getIPAddress",			lua_getip},
  {"getMacAddress",			lua_getmac},
  {0, 0}
};

//Register our Socket Functions
static const luaL_Reg Socket_functions[] = {
  {"init",					lua_initSock},
  {"term",					lua_termSock},
  {"createServerSocket",	lua_createserver},
  {"send",					lua_send},
  {"receive",				lua_recv},
  {"accept",				lua_accept},
  {"close",					lua_closeSock},
  {"connect",				lua_connect},
  {0, 0}
};

void luaNetwork_init(lua_State *L) {
	lua_newtable(L);
	luaL_setfuncs(L, Network_functions, 0);
	lua_setglobal(L, "Network");
	lua_newtable(L);
	luaL_setfuncs(L, Socket_functions, 0);
	lua_setglobal(L, "Socket");
}