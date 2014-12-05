/*
 *ServerSocket.cpp
 *作者：清华大学  张凯
*/

#include <iostream>
#include <string>
#include <time.h>
#include "ServerSocket.h"
#define PORT_DEFAULT 8000						//默认端口号
#define TEST_DELAY_TIME_FREQUENCY 1000			//测试网络延迟时间的循环的次数
using std::cin;
using std::cout;
using std::endl;
using std::string;


void loadWinsocketLib();							//加载Winsocket动态链接库
void getHostIP(char **hostIP);						//获取服务器IP
void setPortNumber(unsigned short &port_number);	//设置服务器端口号
DWORD WINAPI acceptClient(LPVOID lpParameter);		//子进程调用函数，用于接收及处理客户端的连接
void updateTime(const SOCKET &client_socket);		//为客户端时间同步
unsigned short getUnsignedShort();					//读入无符号short整数
SYSTEMTIME getTimeNMilliSecondsLater(SYSTEMTIME current_time, const clock_t &n);	//计算n毫秒之后的时间
WORD getTotalDays(const WORD &year, const WORD &month);								//获得某年某月有多少天
void makeString(char *buf, const SYSTEMTIME &time);									//将时间逆序保存在字符串中
void addReverseNumber(char *buf, short &p, WORD number);							//向一个字符串末尾逆序加入一个整数，以空格为间隔
WORD getNumber(char *buf, short &p);												//
WORD tenPowers(const short &n);														//计算10的n次mi
bool getTime(char *buf, SYSTEMTIME &time);											//从字符串中提取时间
WORD getDifference(SYSTEMTIME x, SYSTEMTIME y);										//计算两个时间的时间差
int timeCompare(SYSTEMTIME x, SYSTEMTIME y);										//比较两个时间的早晚
void putOneNumber(char *buf, WORD number);											//把一个数字转化为字符串


ServerSocket::ServerSocket()
	: port_number(PORT_DEFAULT),
	  enabled(false)
{
	loadWinsocketLib();
	getHostIP(&hostIP);
	cout << "本机IP地址为" << hostIP << "，默认端口号为" << PORT_DEFAULT << "。" << endl;
}

ServerSocket::~ServerSocket()
{
	closesocket(server_socket);							//关闭服务器套接字
	WSACleanup();										//关闭Winsocket动态链接库
	cout << "服务器套接字已关闭。" << endl;
	system("pause");
}

void ServerSocket::setServerSocket()
{
	int err;

	//建立基于TCP/IP的流式套接字
	setPortNumber(port_number);
	server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (server_socket == INVALID_SOCKET)
	{
		cout << "建立服务器套接字失败！" << endl;
		exit(0);
	}

	//绑定服务器IP地址到服务器套接字
	struct sockaddr_in sock_addr;
	sock_addr.sin_family = AF_INET;
	sock_addr.sin_port = htons(port_number);				//设置服务器端口号
	sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);			//设置IP地址为本机IP
	err = bind(server_socket, (sockaddr*)&sock_addr, sizeof(sock_addr));
	if (err != 0)
	{
		cout << "绑定套接字到服务器IP失败！" << endl;
		exit(0);
	}

	//将服务器套接字设定为监听模式
	err = listen(server_socket, 0);
	if (err != 0)
	{
		cout << "将服务器套接字设定为监听模式失败！" << endl;
		exit(0);
	}

	cout << "服务器套接字设置成功！" << endl;
	enabled = true;
}

void ServerSocket::createThread()
{
	DWORD id;
	HANDLE thread_handle = CreateThread(NULL, 0, acceptClient, this, 0, &id);		//建立一个子线程等待客户端的连接
	string cmd;

	//可以通过输入命令exit终止程序
	while (true)
	{
		cin >> cmd;
		if (cmd == "exit")
		{
			enabled = false;														//将服务器套接字设置为不可用
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
		cout << "输入格式有误，请重新输入(0~65535)：";
		cin.clear();
		cin.sync();
	}
}

void loadWinsocketLib()
{
	WSADATA wsadata;
	int err = WSAStartup(MAKEWORD(2, 0), &wsadata);									//设置Winsocket动态链接库版本
	if (err != 0)
	{
		cout << "加载Winsocket库失败！" << endl;
		exit(0);
	}
}

void getHostIP(char **hostIP)
{
	char host_name[256];
	int err = gethostname(host_name, sizeof(host_name));
	if (err != 0)
	{
		cout << "获取本机IP地址失败！" << endl;
		exit(0);
	}
	PHOSTENT hostinfo = gethostbyname(host_name);
	if (hostinfo == NULL)
	{
		cout << "获取本机IP地址失败！" << endl;
		exit(0);
	}
	*hostIP = inet_ntoa(*(struct in_addr*)*hostinfo->h_addr_list);
}

void setPortNumber(unsigned short &port_number)
{
	cout << "是否改变端口号(Y/N)：";
	char ch;
	while (true)
	{
		std::cin >> ch;
		if (ch == 'N' || ch == 'n')
			break;
		if (ch == 'Y' || ch == 'y')
		{
			cout << "请输入新的端口号(0~65535)：";
			port_number = getUnsignedShort();
			cout << "端口号被设置为" << port_number << "。" << endl;
			break;
		}
	}
}

DWORD WINAPI acceptClient(LPVOID lpParameter)
{
	SOCKET client_socket;
	ServerSocket *p_server_socket = (ServerSocket *)lpParameter;

	if (!p_server_socket->getEnabled())											//如果服务器套接字不可用则退出
		return (DWORD)0;
	client_socket = accept((p_server_socket)->getServerSocket(), NULL, NULL);	//接受客户端套接字的连接
	if (!p_server_socket->getEnabled())											//如果服务器套接字不可用则退出
		return (DWORD)0;

	//此线程已被占用，建立一个新的子线程等待客户端的连接
	DWORD id;
	HANDLE thread_handle = CreateThread(NULL, 0, acceptClient, lpParameter, 0, &id);
	
	if (client_socket == INVALID_SOCKET)
		cout << "接受客户端的连接请求失败！" << endl;
	else
	{
		updateTime(client_socket);												//为客户端时间同步
		closesocket(client_socket);												//关闭客户端套接字
	}

	return (DWORD)0;															//此线程结束
}

void updateTime(const SOCKET &client_socket)
{
	clock_t delay_time = 0;														//保存网络延迟时间
	int err;
	char buf[256];
	SYSTEMTIME server_time, client_time;										//服务器、客户端时间

	//计算网络延迟时间
	for (short i = 1; i <= TEST_DELAY_TIME_FREQUENCY; i++)
	{
		delay_time -= clock();													//clock()返回CPU运行总时间（毫秒）
		GetLocalTime(&server_time);												//获取服务器当前时间
		client_time = getTimeNMilliSecondsLater(server_time, delay_time);		//向客户端发送的时间=服务器时间+网络延迟时间
		makeString(buf, client_time);											//将客户端时间保存在字符串中
		err = send(client_socket, buf, sizeof(buf), 0);							//向客户端发送字符串
		if (err == SOCKET_ERROR)
		{
			cout << "与客户端时间同步失败！" << endl;
			return;
		}
		err = recv(client_socket, buf, sizeof(buf), 0);							//接收客户端返回的字符串
		delay_time += clock();													//得到网络延迟时间
		if (err == SOCKET_ERROR)
		{
			cout << "与客户端时间同步失败！" << endl;
			return;
		}
		if (strcmp(buf, "failed") == 0)
		{
			cout << "与客户端时间同步失败！" << endl;
			return;
		}
	}
	delay_time /= (TEST_DELAY_TIME_FREQUENCY << 1);								//得到网络延迟平均时间
	
	//正式进行时间同步
	GetLocalTime(&server_time);
	client_time = getTimeNMilliSecondsLater(server_time, delay_time);
	makeString(buf, client_time);
	err = send(client_socket, buf, sizeof(buf), 0);
	if (err == SOCKET_ERROR)
	{
		cout << "与客户端时间同步失败！" << endl;
		return;
	}
	err = recv(client_socket, buf, sizeof(buf), 0);
	if (err == SOCKET_ERROR)
	{
		cout << "未能接收到客户端发送的时间同步验证数据！" << endl;
		return;
	}
	if (strcmp(buf, "failed") == 0)
	{
		cout << "未能接收到客户端发送的时间同步验证数据！" << endl;
		return;
	}

	//同步是否成功的验证
	if (!getTime(buf, client_time))												//获得客户端返回的时间
	{
		cout << "接收到客户端发送的时间同步验证数据但格式不正确！" << endl;
		return;
	}
	client_time = getTimeNMilliSecondsLater(client_time, delay_time);			//计算考虑网络延迟的情况下客户端现在的时间
	GetLocalTime(&server_time);													//获取服务器时间
	putOneNumber(buf, getDifference(server_time, client_time));					//将客户端与服务器的时间差保存在字符串中

	//输出验证结果
	cout << "与客户端时间同步完成，在" << server_time.wYear << "年" << server_time.wMonth << "月" << server_time.wDay << "日"
		 << server_time.wHour << ":" << server_time.wMinute << ":" << server_time.wSecond << ":" << server_time.wMilliseconds 
		 << "，误差为" << buf << "ms" << endl;
	err = send(client_socket, buf, sizeof(buf), 0);
	if (err == SOCKET_ERROR)
		cout << "客户端未收到时间同步验证结果！" << endl;
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
		cout << "程序在计算时间时出现了异常！" << endl;
		return (WORD)0;
	}
}

void makeString(char *buf, const SYSTEMTIME &time)
{
	short p = 0;											//字符串位置指针

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
	short p = 0;								//字符串位置指针
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
