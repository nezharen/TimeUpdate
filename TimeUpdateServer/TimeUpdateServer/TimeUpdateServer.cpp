/*
 *TimeUpdateServer.cpp
 *作者：清华大学  张凯
*/

//主程序
#include "ServerSocket.h"
int main()
{
	ServerSocket server_socket;

	server_socket.setServerSocket();		//建立服务器套接字
	server_socket.createThread();			//建立子线程等待客户端的连接
	return 0;
}
