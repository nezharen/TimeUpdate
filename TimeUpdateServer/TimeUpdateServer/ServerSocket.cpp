/*
 *ServerSocket.cpp
 *���ߣ��廪��ѧ  �ſ�
*/

#include <iostream>
#include <string>
#include <time.h>
#include "ServerSocket.h"
#define PORT_DEFAULT 8000						//Ĭ�϶˿ں�
#define TEST_DELAY_TIME_FREQUENCY 1000			//���������ӳ�ʱ���ѭ���Ĵ���
using std::cin;
using std::cout;
using std::endl;
using std::string;


void loadWinsocketLib();							//����Winsocket��̬���ӿ�
void getHostIP(char **hostIP);						//��ȡ������IP
void setPortNumber(unsigned short &port_number);	//���÷������˿ں�
DWORD WINAPI acceptClient(LPVOID lpParameter);		//�ӽ��̵��ú��������ڽ��ռ�����ͻ��˵�����
void updateTime(const SOCKET &client_socket);		//Ϊ�ͻ���ʱ��ͬ��
unsigned short getUnsignedShort();					//�����޷���short����
SYSTEMTIME getTimeNMilliSecondsLater(SYSTEMTIME current_time, const clock_t &n);	//����n����֮���ʱ��
WORD getTotalDays(const WORD &year, const WORD &month);								//���ĳ��ĳ���ж�����
void makeString(char *buf, const SYSTEMTIME &time);									//��ʱ�����򱣴����ַ�����
void addReverseNumber(char *buf, short &p, WORD number);							//��һ���ַ���ĩβ�������һ���������Կո�Ϊ���
WORD getNumber(char *buf, short &p);												//
WORD tenPowers(const short &n);														//����10��n��mi
bool getTime(char *buf, SYSTEMTIME &time);											//���ַ�������ȡʱ��
WORD getDifference(SYSTEMTIME x, SYSTEMTIME y);										//��������ʱ���ʱ���
int timeCompare(SYSTEMTIME x, SYSTEMTIME y);										//�Ƚ�����ʱ�������
void putOneNumber(char *buf, WORD number);											//��һ������ת��Ϊ�ַ���


ServerSocket::ServerSocket()
	: port_number(PORT_DEFAULT),
	  enabled(false)
{
	loadWinsocketLib();
	getHostIP(&hostIP);
	cout << "����IP��ַΪ" << hostIP << "��Ĭ�϶˿ں�Ϊ" << PORT_DEFAULT << "��" << endl;
}

ServerSocket::~ServerSocket()
{
	closesocket(server_socket);							//�رշ������׽���
	WSACleanup();										//�ر�Winsocket��̬���ӿ�
	cout << "�������׽����ѹرա�" << endl;
	system("pause");
}

void ServerSocket::setServerSocket()
{
	int err;

	//��������TCP/IP����ʽ�׽���
	setPortNumber(port_number);
	server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (server_socket == INVALID_SOCKET)
	{
		cout << "�����������׽���ʧ�ܣ�" << endl;
		exit(0);
	}

	//�󶨷�����IP��ַ���������׽���
	struct sockaddr_in sock_addr;
	sock_addr.sin_family = AF_INET;
	sock_addr.sin_port = htons(port_number);				//���÷������˿ں�
	sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);			//����IP��ַΪ����IP
	err = bind(server_socket, (sockaddr*)&sock_addr, sizeof(sock_addr));
	if (err != 0)
	{
		cout << "���׽��ֵ�������IPʧ�ܣ�" << endl;
		exit(0);
	}

	//���������׽����趨Ϊ����ģʽ
	err = listen(server_socket, 0);
	if (err != 0)
	{
		cout << "���������׽����趨Ϊ����ģʽʧ�ܣ�" << endl;
		exit(0);
	}

	cout << "�������׽������óɹ���" << endl;
	enabled = true;
}

void ServerSocket::createThread()
{
	DWORD id;
	HANDLE thread_handle = CreateThread(NULL, 0, acceptClient, this, 0, &id);		//����һ�����̵߳ȴ��ͻ��˵�����
	string cmd;

	//����ͨ����������exit��ֹ����
	while (true)
	{
		cin >> cmd;
		if (cmd == "exit")
		{
			enabled = false;														//���������׽�������Ϊ������
			break;
		}
	}
}

SOCKET ServerSocket::getServerSocket()
{
	return server_socket;
}

bool ServerSocket::getEnabled()
{
	return enabled;
}

unsigned short getUnsignedShort()
{
	unsigned short ans;

	while (true)
	{
		cin >> ans;
		if (cin.good())
			return ans;
		cout << "�����ʽ��������������(0~65535)��";
		cin.clear();
		cin.sync();
	}
}

void loadWinsocketLib()
{
	WSADATA wsadata;
	int err = WSAStartup(MAKEWORD(2, 0), &wsadata);									//����Winsocket��̬���ӿ�汾
	if (err != 0)
	{
		cout << "����Winsocket��ʧ�ܣ�" << endl;
		exit(0);
	}
}

void getHostIP(char **hostIP)
{
	char host_name[256];
	int err = gethostname(host_name, sizeof(host_name));
	if (err != 0)
	{
		cout << "��ȡ����IP��ַʧ�ܣ�" << endl;
		exit(0);
	}
	PHOSTENT hostinfo = gethostbyname(host_name);
	if (hostinfo == NULL)
	{
		cout << "��ȡ����IP��ַʧ�ܣ�" << endl;
		exit(0);
	}
	*hostIP = inet_ntoa(*(struct in_addr*)*hostinfo->h_addr_list);
}

void setPortNumber(unsigned short &port_number)
{
	cout << "�Ƿ�ı�˿ں�(Y/N)��";
	char ch;
	while (true)
	{
		std::cin >> ch;
		if (ch == 'N' || ch == 'n')
			break;
		if (ch == 'Y' || ch == 'y')
		{
			cout << "�������µĶ˿ں�(0~65535)��";
			port_number = getUnsignedShort();
			cout << "�˿ںű�����Ϊ" << port_number << "��" << endl;
			break;
		}
	}
}

DWORD WINAPI acceptClient(LPVOID lpParameter)
{
	SOCKET client_socket;
	ServerSocket *p_server_socket = (ServerSocket *)lpParameter;

	if (!p_server_socket->getEnabled())											//����������׽��ֲ��������˳�
		return (DWORD)0;
	client_socket = accept((p_server_socket)->getServerSocket(), NULL, NULL);	//���ܿͻ����׽��ֵ�����
	if (!p_server_socket->getEnabled())											//����������׽��ֲ��������˳�
		return (DWORD)0;

	//���߳��ѱ�ռ�ã�����һ���µ����̵߳ȴ��ͻ��˵�����
	DWORD id;
	HANDLE thread_handle = CreateThread(NULL, 0, acceptClient, lpParameter, 0, &id);
	
	if (client_socket == INVALID_SOCKET)
		cout << "���ܿͻ��˵���������ʧ�ܣ�" << endl;
	else
	{
		updateTime(client_socket);												//Ϊ�ͻ���ʱ��ͬ��
		closesocket(client_socket);												//�رտͻ����׽���
	}

	return (DWORD)0;															//���߳̽���
}

void updateTime(const SOCKET &client_socket)
{
	clock_t delay_time = 0;														//���������ӳ�ʱ��
	int err;
	char buf[256];
	SYSTEMTIME server_time, client_time;										//���������ͻ���ʱ��

	//���������ӳ�ʱ��
	for (short i = 1; i <= TEST_DELAY_TIME_FREQUENCY; i++)
	{
		delay_time -= clock();													//clock()����CPU������ʱ�䣨���룩
		GetLocalTime(&server_time);												//��ȡ��������ǰʱ��
		client_time = getTimeNMilliSecondsLater(server_time, delay_time);		//��ͻ��˷��͵�ʱ��=������ʱ��+�����ӳ�ʱ��
		makeString(buf, client_time);											//���ͻ���ʱ�䱣�����ַ�����
		err = send(client_socket, buf, sizeof(buf), 0);							//��ͻ��˷����ַ���
		if (err == SOCKET_ERROR)
		{
			cout << "��ͻ���ʱ��ͬ��ʧ�ܣ�" << endl;
			return;
		}
		err = recv(client_socket, buf, sizeof(buf), 0);							//���տͻ��˷��ص��ַ���
		delay_time += clock();													//�õ������ӳ�ʱ��
		if (err == SOCKET_ERROR)
		{
			cout << "��ͻ���ʱ��ͬ��ʧ�ܣ�" << endl;
			return;
		}
		if (strcmp(buf, "failed") == 0)
		{
			cout << "��ͻ���ʱ��ͬ��ʧ�ܣ�" << endl;
			return;
		}
	}
	delay_time /= (TEST_DELAY_TIME_FREQUENCY << 1);								//�õ������ӳ�ƽ��ʱ��
	
	//��ʽ����ʱ��ͬ��
	GetLocalTime(&server_time);
	client_time = getTimeNMilliSecondsLater(server_time, delay_time);
	makeString(buf, client_time);
	err = send(client_socket, buf, sizeof(buf), 0);
	if (err == SOCKET_ERROR)
	{
		cout << "��ͻ���ʱ��ͬ��ʧ�ܣ�" << endl;
		return;
	}
	err = recv(client_socket, buf, sizeof(buf), 0);
	if (err == SOCKET_ERROR)
	{
		cout << "δ�ܽ��յ��ͻ��˷��͵�ʱ��ͬ����֤���ݣ�" << endl;
		return;
	}
	if (strcmp(buf, "failed") == 0)
	{
		cout << "δ�ܽ��յ��ͻ��˷��͵�ʱ��ͬ����֤���ݣ�" << endl;
		return;
	}

	//ͬ���Ƿ�ɹ�����֤
	if (!getTime(buf, client_time))												//��ÿͻ��˷��ص�ʱ��
	{
		cout << "���յ��ͻ��˷��͵�ʱ��ͬ����֤���ݵ���ʽ����ȷ��" << endl;
		return;
	}
	client_time = getTimeNMilliSecondsLater(client_time, delay_time);			//���㿼�������ӳٵ�����¿ͻ������ڵ�ʱ��
	GetLocalTime(&server_time);													//��ȡ������ʱ��
	putOneNumber(buf, getDifference(server_time, client_time));					//���ͻ������������ʱ�������ַ�����

	//�����֤���
	cout << "��ͻ���ʱ��ͬ����ɣ���" << server_time.wYear << "��" << server_time.wMonth << "��" << server_time.wDay << "��"
		 << server_time.wHour << ":" << server_time.wMinute << ":" << server_time.wSecond << ":" << server_time.wMilliseconds 
		 << "�����Ϊ" << buf << "ms" << endl;
	err = send(client_socket, buf, sizeof(buf), 0);
	if (err == SOCKET_ERROR)
		cout << "�ͻ���δ�յ�ʱ��ͬ����֤�����" << endl;
}

SYSTEMTIME getTimeNMilliSecondsLater(SYSTEMTIME current_time, const clock_t &n)
{
	WORD totalDays;

	current_time.wMilliseconds += (WORD)n;
	if (current_time.wMilliseconds > 999)
	{
		current_time.wSecond += current_time.wMilliseconds /= 1000;
		current_time.wMilliseconds %= 1000;
		if (current_time.wSecond >59)
		{
			current_time.wMinute += current_time.wSecond / 60;
			current_time.wSecond %= 60;
			if (current_time.wMinute > 59)
			{
				current_time.wHour += current_time.wMinute / 60;
				current_time.wMinute %= 60;
				if (current_time.wHour > 23)
				{
					current_time.wDay += current_time.wHour / 24;
					current_time.wDayOfWeek += current_time.wHour / 24;
					if (current_time.wDayOfWeek > 6)
						current_time.wDayOfWeek %= 7;
					current_time.wHour %= 24;
					totalDays = getTotalDays(current_time.wYear, current_time.wMonth);
					while (current_time.wDay > totalDays)
					{
						current_time.wDay -= totalDays;
						current_time.wMonth ++;
						if (current_time.wMonth > 12)
						{
							current_time.wMonth = 1;
							current_time.wYear ++;
						}
						totalDays = getTotalDays(current_time.wYear, current_time.wMonth);
					}
				}
			}
		}
	}
	return current_time;
}

WORD getTotalDays(const WORD &year, const WORD &month)
{
	switch (month)
	{
	case 1:
	case 3:
	case 5:
	case 7:
	case 8:
	case 10:
	case 12:
		return (WORD)31;
	case 4:
	case 6:
	case 9:
	case 11:
		return (WORD)30;
	case 2:
		if (year % 400 == 0 || ((year % 100 != 0) && (year % 4 == 0)))
			return (WORD)29;
		else
			return (WORD)28;
	default:
		cout << "�����ڼ���ʱ��ʱ�������쳣��" << endl;
		return (WORD)0;
	}
}

void makeString(char *buf, const SYSTEMTIME &time)
{
	short p = 0;											//�ַ���λ��ָ��

	addReverseNumber(buf, p, time.wMilliseconds);
	addReverseNumber(buf, p, time.wSecond);
	addReverseNumber(buf, p, time.wMinute);
	addReverseNumber(buf, p, time.wHour);
	addReverseNumber(buf, p, time.wDayOfWeek);
	addReverseNumber(buf, p, time.wDay);
	addReverseNumber(buf, p, time.wMonth);
	addReverseNumber(buf, p, time.wYear);
	buf[p - 1] = '\0';
}

void addReverseNumber(char *buf, short &p, WORD number)
{
	if (number == 0)
	{
		buf[p] = '0';
		p++;
		buf[p] = ' ';
		p++;
		return;
	}
	while (number > 0)
	{
		buf[p] = '0' + number % 10;
		number /= 10;
		p++;
	}
	buf[p] = ' ';
	p++;
	return;
}

WORD getNumber(char *buf, short &p)
{
	WORD ans = 0;
	
	for (short i = 0; buf[p] != ' ' && buf[p] != '\0'; i++)
	{
		ans += (buf[p] - '0') * tenPowers(i);
		p++;
		if (p == 256)
			return -1;
	}
	p++;
	if (p == 256)
		return -1;
	return ans;
}

bool getTime(char *buf, SYSTEMTIME &time)
{
	short p = 0;								//�ַ���λ��ָ��
	time.wMilliseconds = getNumber(buf, p);
	if (time.wMilliseconds == -1)
		return false;
	time.wSecond = getNumber(buf, p);
	if (time.wSecond == -1)
		return false;
	time.wMinute = getNumber(buf, p);
	if (time.wMinute == -1)
		return false;
	time.wHour = getNumber(buf, p);
	if (time.wHour == -1)
		return false;
	time.wDayOfWeek = getNumber(buf, p);
	if (time.wDayOfWeek == -1)
		return false;
	time.wDay = getNumber(buf, p);
	if (time.wDay == -1)
		return false;
	time.wMonth = getNumber(buf, p);
	if (time.wMonth == -1)
		return false;
	time.wYear = getNumber(buf, p);
	if (time.wYear == -1)
		return false;
	return true;
}

WORD tenPowers(const short &n)
{
	WORD ans = 1;

	for (short i = 1; i <= n; i++)
		ans *= 10;
	return ans;
}

WORD getDifference(SYSTEMTIME x, SYSTEMTIME y)
{
	SYSTEMTIME t;

	if (timeCompare(x, y) < 0)
	{
		t = x;
		x = y;
		y = t;
	}
	for (WORD i = 0; ; i++)
	{
		if (timeCompare(x, y) == 0)
			return i;
		y = getTimeNMilliSecondsLater(y, 1);
	}
}

int timeCompare(SYSTEMTIME x, SYSTEMTIME y)
{
	if (x.wYear < y.wYear)
		return -1;
	else
		if (x.wYear > y.wYear)
			return 1;

	if (x.wMonth < y.wMonth)
		return -1;
	else
		if (x.wMonth > y.wMonth)
			return 1;

	if (x.wDay < y.wDay)
		return -1;
	else
		if (x.wDay > y.wDay)
			return 1;

	if (x.wHour < y.wHour)
		return -1;
	else
		if (x.wHour > y.wHour)
			return 1;

	if (x.wMinute < y.wMinute)
		return -1;
	else
		if (x.wMinute > y.wMinute)
			return 1;

	if (x.wSecond < y.wSecond)
		return -1;
	else
		if (x.wSecond > y.wSecond)
			return 1;

	if (x.wMilliseconds < y.wMilliseconds)
		return -1;
	else
		if (x.wMilliseconds > y.wMilliseconds)
			return 1;

	return 0;
}

void putOneNumber(char *buf, WORD number)
{
	short i, p;
	WORD t;

	for (i = 0; ; i++)
	{
		t = tenPowers(i);
		if (number / t < 10)
			break;
	}
	for (p = 0; i >= 0; p++)
	{
		buf[p] = '0' + number / t;
		number %= t;
		i--;
		t = tenPowers(i);
	}
	buf[p] = ' ';
	p++;
	buf[p] = '\0';
}
