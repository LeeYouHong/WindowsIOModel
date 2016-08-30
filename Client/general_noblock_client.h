/**
	@描述：基本方式-非阻塞模式-客户端
	@作者：LYH66
*/

#include <WinSock2.h>
#include <WS2tcpip.h>
#include "stdio.h"

#define MSGSIZE 255

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "AdvApi32.lib")

DWORD WINAPI inputThread(LPVOID lpParameter);

char inputbuf[MSGSIZE] = {0};

int general_noblock_client()
{
	WSADATA wsaData;
	SOCKET sClient = INVALID_SOCKET;
	struct addrinfo hints;
	struct addrinfo *list = NULL;
	struct addrinfo *temp = NULL;
	int iResult = 0;
	char recvbuf[MSGSIZE] = {0};
	u_long noblock = 1;
	DWORD dwThreadId;

	// 初始化Windows Socket库
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	// 解析服务器端口和IP
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	iResult = getaddrinfo("127.0.0.1", "27015", &hints, &list);
	if ( iResult != 0)
	{
		printf("getaddrinfo error : %d", WSAGetLastError());
		return 1;
	}
	
	for (temp = list; temp != NULL; temp = temp->ai_next)
	{
		// 创建一个套接字	
		sClient = socket(temp->ai_family, temp->ai_socktype, temp->ai_protocol);
		if ( sClient == INVALID_SOCKET )
		{
			printf("socket faild with error: %ld\n", WSAGetLastError());
			freeaddrinfo(list);
			WSACleanup();
			return 1;
		}

		// 连接到服务端
		iResult = connect(sClient, temp->ai_addr, (int)temp->ai_addrlen);
		if ( iResult == SOCKET_ERROR )
		{
			printf("connect faild with error:%ld\n", WSAGetLastError());
			closesocket(sClient);
			sClient = INVALID_SOCKET;
			continue;
		}

		break;
	}
	
	freeaddrinfo(list);
	
	// 防止没有连接成功
	if ( sClient == INVALID_SOCKET )
	{
		printf("Unable to connect to server!\n");
		WSACleanup();
		return 1;
	}
	
	printf(" connect success!\n");

	// 设置非阻塞
	iResult = ioctlsocket(sClient, FIONBIO, &noblock);
	if ( iResult == SOCKET_ERROR )
	{
		closesocket(sClient);
		WSACleanup();
		return 1;
	}
	
	// 创建一个标准输入的线程
	CreateThread(NULL, 0, inputThread, NULL, 0, &dwThreadId);

	while( 1 )
	{
		// 发送消息
		int inputbuflen = (int)strlen(inputbuf);
		if ( inputbuflen > 0 )
		{
			iResult = send(sClient, inputbuf, inputbuflen + 1, 0);
			if ( iResult == SOCKET_ERROR )
			{
				printf("send faild with error:%ld", WSAGetLastError());
				closesocket(sClient);
				WSACleanup();
				return 1;
			}
			memset(inputbuf, 0, MSGSIZE);
		}

		// 接受消息
		iResult = recv(sClient, recvbuf, MSGSIZE, 0);
		if ( iResult == SOCKET_ERROR )
		{
			if ( WSAGetLastError() != WSAEWOULDBLOCK )
			{

			
				printf("recv faild with error:%ld\n", WSAGetLastError());
				closesocket(sClient);
				WSACleanup();
				return 1;
			}
		}
		if ( iResult > 0 )
		{
			printf("\nserver say:%s\n", recvbuf);
		}	
	}

	// 关闭连接
	iResult = shutdown(sClient, SD_SEND);
	if ( iResult == SOCKET_ERROR )
	{
		printf("shutdown faild with error: %ld", WSAGetLastError());
		closesocket(sClient);
		WSACleanup();
		return 1;
	}

	// 扫尾
	closesocket(sClient);
	WSACleanup();

	return 0;
}

DWORD WINAPI inputThread(LPVOID lpParameter)
{
	char temp[MSGSIZE] = {0};
	while( 1 )
	{
		printf("input: ");
		fgets(temp, MSGSIZE, stdin);
		temp[strlen(temp)-1] = '\0';
		strcpy_s(inputbuf, strlen(temp) + 1, temp);
	}
	return 0;
}