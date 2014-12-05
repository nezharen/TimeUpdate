/*
 *ServerSocket.h
 *���ߣ��廪��ѧ  �ſ�
*/

#include <winsock2.h>
#pragma comment(lib, "WS2_32")			//����Winsocket��̬���ӿ�
#ifndef SERVER_SOCKET_H
#define SERVER_SOCKET_H

//��ServerSocket�����ڽ����������׽��ֲ���Ӧ�ͻ����׽���
class ServerSocket
{
public:
	ServerSocket();
	virtual ~ServerSocket();
	void setServerSocket();				//�����������׽���
	void createThread();				//�������̵߳ȴ��ͻ��˵�����
	SOCKET getServerSocket();			//��ȡ�������׽���
	bool getEnabled();					//��ȡ�������׽����Ƿ����
protected:
	SOCKET server_socket;				//�������׽���
	char *hostIP;						//������IP��ַ
	unsigned short port_number;			//�������˿ں�
	bool enabled;						//�������׽����Ƿ����
};

#endif
