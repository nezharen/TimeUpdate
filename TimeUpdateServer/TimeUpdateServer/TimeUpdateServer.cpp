/*
 *TimeUpdateServer.cpp
 *���ߣ��廪��ѧ  �ſ�
*/

//������
#include "ServerSocket.h"
int main()
{
	ServerSocket server_socket;

	server_socket.setServerSocket();		//�����������׽���
	server_socket.createThread();			//�������̵߳ȴ��ͻ��˵�����
	return 0;
}
