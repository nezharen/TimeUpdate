#include <iostream>
#include <winsock2.h>
#pragma comment(lib, "WS2_32")
#define TEST_DELAY_TIME_FREQUENCY 1000
using std::cin;
using std::cout;
using std::endl;

void setServerSocket(SOCKET &server_socket);
void updateTime(const SOCKET &server_socket);
WORD getNumber(char *buf, short &p);
WORD tenPowers(const short &n);
void getTime(char *buf, SYSTEMTIME &time);
void makeString(char *buf, const SYSTEMTIME &time);
void addReverseNumber(char *buf, short &p, WORD number);

int main()
{
	SOCKET server_socket;
	setServerSocket(server_socket);
	updateTime(server_socket);
	closesocket(server_socket);
	WSACleanup();
	system("pause");
	return 0;
}

void setServerSocket(SOCKET &server_socket)
{
	char buf[256] = "127.0.0.1";
	unsigned short port = 8000;
	cout << "�����������IP��ַ��";
	cin >> buf;
	cout << "������������˿ںţ�";
	cin >> port;

	int err;
	WSADATA wsadata;
	err = WSAStartup(MAKEWORD(2, 0), &wsadata);
	if (err != 0)
	{
		cout << "����Winsocket��ʧ�ܣ�" << endl;
		system("pause");
		exit(0);
	}
	server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (server_socket == INVALID_SOCKET)
	{
		cout << "�����ͻ����׽���ʧ�ܣ�" << endl;
		WSACleanup();
		system("pause");
		exit(0);
	}

	struct sockaddr_in sock_addr;
	sock_addr.sin_family = AF_INET;
	sock_addr.sin_port = htons(port);
	sock_addr.sin_addr.s_addr = inet_addr(buf);
	err = connect(server_socket, (sockaddr*)&sock_addr, sizeof(sock_addr));
	if (err != 0)
	{
		cout << "���ӷ�����ʧ�ܣ�" << endl;
		closesocket(server_socket);
		WSACleanup();
		system("pause");
		exit(0);
	}

	cout << "���ӷ������ɹ�����ʼ����ʱ��ͬ����" << endl;
}

void updateTime(const SOCKET &server_socket)
{
	int err;
	char buf[256];

	for (short i = 1; i <= TEST_DELAY_TIME_FREQUENCY; i++)
	{
		err = recv(server_socket, buf, sizeof(buf), 0);
		if (err == SOCKET_ERROR)
		{
			cout << "�������ʱ��ͬ��ʧ�ܣ�" << endl;
			strcpy_s<256>(buf, "failed");
			err = send(server_socket, buf, sizeof(buf), 0);
			return;
		}
		err = send(server_socket, buf, sizeof(buf), 0);
		if (err == SOCKET_ERROR)
		{
			cout << "�������ʱ��ͬ��ʧ�ܣ�" << endl;
			strcpy_s<256>(buf, "failed");
			err = send(server_socket, buf, sizeof(buf), 0);
			return;
		}
	}
	err = recv(server_socket, buf, sizeof(buf), 0);
	if (err == SOCKET_ERROR)
	{
		cout << "�������ʱ��ͬ��ʧ�ܣ�" << endl;
		strcpy_s<256>(buf, "failed");
		err = send(server_socket, buf, sizeof(buf), 0);
		return;
	}

	SYSTEMTIME time;
	getTime(buf, time);
	err = SetLocalTime(&time);
	if (err == 0)
	{
		cout << "�޸�ϵͳʱ��ʧ�ܣ���ȷ�ϳ�������㹻��Ȩ�ޡ�" << endl;
		cout << "��ǰ������ʱ�䣺" << time.wYear << "��" << time.wMonth << "��" << time.wDay << "��"
		<< time.wHour << ":" << time.wMinute << ":" << time.wSecond << ":" << time.wMilliseconds << endl;
		strcpy_s<256>(buf, "failed");
		err = send(server_socket, buf, sizeof(buf), 0);
		return;
	}
	GetLocalTime(&time);
	makeString(buf, time);
	err = send(server_socket, buf, sizeof(buf), 0);
	if (err == SOCKET_ERROR)
	{
		cout << "δ�������������ʱ��ͬ����֤���ݣ�" << endl;
		strcpy_s<256>(buf, "failed");
		err = send(server_socket, buf, sizeof(buf), 0);
		return;
	}
	err = recv(server_socket, buf, sizeof(buf), 0);
	if (err == SOCKET_ERROR)
	{
		cout << "δ�ܽ��յ����������͵�ʱ��ͬ����֤�źţ�" << endl;
		strcpy_s<256>(buf, "failed");
		err = send(server_socket, buf, sizeof(buf), 0);
		return;
	}
	cout << "��ͻ���ʱ��ͬ����ɣ���" << time.wYear << "��" << time.wMonth << "��" << time.wDay << "��"
		 << time.wHour << ":" << time.wMinute << ":" << time.wSecond << ":" << time.wMilliseconds << "�����Ϊ" << buf << "ms" << endl;
}

WORD getNumber(char *buf, short &p)
{
	WORD ans = 0;
	
	for (short i = 0; buf[p] != ' ' && buf[p] != '\0'; i++)
	{
		ans += (buf[p] - '0') * tenPowers(i);
		p++;
	}
	p++;
	return ans;
}

void getTime(char *buf, SYSTEMTIME &time)
{
	short p = 0;
	time.wMilliseconds = getNumber(buf, p);
	time.wSecond = getNumber(buf, p);
	time.wMinute = getNumber(buf, p);
	time.wHour = getNumber(buf, p);
	time.wDayOfWeek = getNumber(buf, p);
	time.wDay = getNumber(buf, p);
	time.wMonth = getNumber(buf, p);
	time.wYear = getNumber(buf, p);
}

WORD tenPowers(const short &n)
{
	WORD ans = 1;

	for (short i = 1; i <= n; i++)
		ans *= 10;
	return ans;
}

void makeString(char *buf, const SYSTEMTIME &time)
{
	short p = 0;

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
