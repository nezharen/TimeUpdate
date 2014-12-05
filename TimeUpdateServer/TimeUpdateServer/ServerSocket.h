/*
 *ServerSocket.h
 *作者：清华大学  张凯
*/

#include <winsock2.h>
#pragma comment(lib, "WS2_32")			//导入Winsocket动态链接库
#ifndef SERVER_SOCKET_H
#define SERVER_SOCKET_H

//类ServerSocket，用于建立服务器套接字并响应客户端套接字
class ServerSocket
{
public:
	ServerSocket();
	virtual ~ServerSocket();
	void setServerSocket();				//建立服务器套接字
	void createThread();				//建立子线程等待客户端的连接
	SOCKET getServerSocket();			//获取服务器套接字
	bool getEnabled();					//获取服务器套接字是否可用
protected:
	SOCKET server_socket;				//服务器套接字
	char *hostIP;						//服务器IP地址
	unsigned short port_number;			//服务器端口号
	bool enabled;						//服务器套接字是否可用
};

#endif
