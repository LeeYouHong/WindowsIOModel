/**
	@描述：套接字模式-阻塞模式-客户端
	@作者：LYH66
*/

#include <WinSock2.h>
#include <WS2tcpip.h>
#include "stdio.h"

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "AdvApi32.lib")

#define MSGSIZE 255

DWORD WINAPI recvThread(LPVOID lpParameter);
DWORD WINAPI inputThread(LPVOID lpParameter);

char inputbuf[MSGSIZE] = {0};

int general_server()
{
	SOCKET sServer = INVALID_SOCKET;
	SOCKET sClient = INVALID_SOCKET;
	WSADATA wsadata;
	struct addrinfo *list = NULL;
	struct addrinfo hints;
	int iResult = 0;
	int iAddrSize = sizeof(SOCKADDR_IN);
	DWORD inputThreadId;
	DWORD recvThreadId;

	// 初始化windows socket
	iResult = WSAStartup(MAKEWORD(2, 2), &wsadata);
	if ( iResult != 0 )
	{
		printf("WSAStartup faild with ERROR: %ld", iResult);
		return 1;
	}
	
	// 解析服务器地址和端口
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_IP;
	hints.ai_flags = AI_PASSIVE;
	iResult = getaddrinfo("127.0.0.1", "27015", &hints, &list);
	if ( iResult == SOCKET_ERROR )
	{
		printf("getaddrinfo faild with error: %ld", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	// 创建一个用于监听的套接字
	sServer = socket(list->ai_family, list->ai_socktype, list->ai_protocol);
	if ( sServer == INVALID_SOCKET )
	{
		printf("socket faild with error: %ld", WSAGetLastError());
		freeaddrinfo(list);
		WSACleanup();
		return 1;
	}
	
	// 把监听套接字与套接字地址绑定起来
	iResult = bind(sServer, list->ai_addr, (int)list->ai_addrlen);
	if ( iResult == SOCKET_ERROR )
	{
		printf("bind faild with error: %ld", WSAGetLastError());
		freeaddrinfo(list);
		closesocket(sServer);
		WSACleanup();
		return 1;
	}
	
	freeaddrinfo(list);

	// 把监听套接字设置为监听状态
	iResult = listen(sServer, SOMAXCONN);
	if ( iResult == SOCKET_ERROR )
	{
		printf("listen faild with error: %ld", WSAGetLastError());
		closesocket(sServer);
		WSACleanup();
		return 1;
	}

	// 接受连接
	sClient = accept(sServer, NULL, NULL);
	if ( sClient == SOCKET_ERROR )
	{
		printf("accept faild with error: %ld", WSAGetLastError());
		closesocket(sServer);
		WSACleanup();
		return 1;
	}
	else
	{
		printf("Client connected.\n");
	}
	
	closesocket(sServer);

	// 创建标准输入进程
	CreateThread(NULL, 0, inputThread, NULL, 0, &inputThreadId);

	// 创建接收信息进程
	CreateThread(NULL, 0, recvThread, &sClient, 0, &recvThreadId);

	while( 1 )
	{
		int inputbuflen = (int)strlen(inputbuf);
		if ( inputbuflen > 0 )
		{
			// 发送信息
			iResult = send(sClient, inputbuf, inputbuflen + 1, 0);
			if ( iResult == SOCKET_ERROR )
			{
				printf("send faild with error: %ld", WSAGetLastError());
				closesocket(sClient);
				closesocket(sServer);
				WSACleanup();
				return 1;
			}
			memset(inputbuf, 0, MSGSIZE);
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
	closesocket(sServer);
	closesocket(sClient);
	WSACleanup();

	return 0;
}


DWORD WINAPI inputThread(LPVOID lpParameter)
{
	char temp[MSGSIZE] = {0};
	while ( 1 )
	{
		printf("input: ");
		fgets(temp, MSGSIZE, stdin);
		temp[strlen(temp)-1] = '\0';
		strcpy_s(inputbuf, strlen(temp) + 1, temp);
	}
	return 0;
}

DWORD WINAPI recvThread(LPVOID lpParameter)
{
	int iResult = 0;
	printf("recv");
	char recvbuf[MSGSIZE] = {0};
	SOCKET sClient = *(SOCKET*)lpParameter;

	while ( 1 )
	{
		// 接受消息
		iResult = recv(sClient, recvbuf, MSGSIZE, 0);
		if ( iResult == SOCKET_ERROR )
		{
			printf("recv faild with error:%ld\n", WSAGetLastError());
			closesocket(sClient);
			return 1;
		}
		printf("\nclient say:%s\n", recvbuf);
	}
	return 0;
}