/**
	@������Select I/Oģ��-�����
	@���ߣ�LYH66
*/

#include <WinSock2.h>
#include <WS2tcpip.h>
#include "stdio.h"

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "AdvApi32.lib")

#define MSGSIZE 255
#define MAXCON  3

DWORD WINAPI inputThread(LPVOID lpParameter);

char inputbuf[MSGSIZE] = {0};

int select_server()
{
	SOCKET sServer = INVALID_SOCKET;
	SOCKET sClient = INVALID_SOCKET;
	WSADATA wsadata;
	struct addrinfo *list = NULL;
	struct addrinfo hints;
	int iResult = 0;
	char recvbuf[MSGSIZE] = {0};
	int iAddrSize = sizeof(SOCKADDR_IN);
	struct sockaddr_in clientSockAddr;
	DWORD dwThreadId;
	fd_set freads;
	fd_set fwrites;
	timeval tv = {1, 0};

	// ��ʼ��windows socket
	iResult = WSAStartup(MAKEWORD(2, 2), &wsadata);
	if ( iResult != 0 )
	{
		printf("WSAStartup faild with ERROR: %ld", iResult);
		return 1;
	}
	
	// ������������ַ�Ͷ˿�
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_IP;
	hints.ai_flags = AI_PASSIVE;
	iResult = getaddrinfo("127.0.0.1", "12345", &hints, &list);
	if ( iResult == SOCKET_ERROR )
	{
		printf("getaddrinfo faild with error: %ld", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	// ����һ�����ڼ������׽���
	sServer = socket(list->ai_family, list->ai_socktype, list->ai_protocol);
	if ( sServer == INVALID_SOCKET )
	{
		printf("socket faild with error: %ld", WSAGetLastError());
		freeaddrinfo(list);
		WSACleanup();
		return 1;
	}
	
	// �Ѽ����׽������׽��ֵ�ַ������
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

	// �Ѽ����׽�������Ϊ����״̬
	iResult = listen(sServer, MAXCON);
	if ( iResult == SOCKET_ERROR )
	{
		printf("listen faild with error: %ld", WSAGetLastError());
		closesocket(sServer);
		WSACleanup();
		return 1;
	}
	
	// ��������
	sClient = accept(sServer, (struct sockaddr*)&clientSockAddr, &iAddrSize);
	if ( sClient == INVALID_SOCKET )
	{
		printf("accept faild with error: %ld", WSAGetLastError());
		closesocket(sServer);
		WSACleanup();
		return 1;
	}
	else
	{
		printf("Client[%s:%d] connected.\n", inet_ntoa( clientSockAddr.sin_addr), ntohs(clientSockAddr.sin_port));
		CreateThread(NULL, 0, inputThread, NULL, 0, &dwThreadId);
	}
	
	closesocket(sServer);

	while ( 1 )
	{
		// ����fd_set
		FD_ZERO(&freads);
		FD_ZERO(&fwrites);

		// ����fd_set
		FD_SET(sClient, &freads);
		FD_SET(sClient, &fwrites);

		// ��ѯ�׽��ֵ�״̬
		iResult = select(0, &freads, &fwrites, NULL, &tv);
		if ( iResult == SOCKET_ERROR )
		{
			printf("select faild with error: %ld\n", WSAGetLastError());
			closesocket(sClient);
			return 1;
		}else if ( iResult == 0)
		{
			continue;
		}
		
		if ( FD_ISSET(sClient, &fwrites) )
		{
			// ������Ϣ
			int inputbuflen = (int)strlen(inputbuf);
			if ( inputbuflen > 0)
			{
				iResult = send(sClient, inputbuf, inputbuflen + 1, 0);
				if ( iResult == SOCKET_ERROR )
				{
					printf("send faild with error:%ld", WSAGetLastError());
					closesocket(sClient);
					WSACleanup();
					return 1;
				}
				memset(inputbuf, 0, strlen(inputbuf) + 1);
			}
		}

		if ( FD_ISSET(sClient, &freads) )
		{
			// ������Ϣ
			iResult = recv(sClient, recvbuf, MSGSIZE, 0);
			if ( iResult == SOCKET_ERROR )
			{
				printf("recv faild with error:%ld", WSAGetLastError());
				closesocket(sClient);
				WSACleanup();
				return 1;
			}
			printf("\nserver say: %s\n", recvbuf);
		}
	}

	WSACleanup();
	return 0;
}

DWORD WINAPI inputThread(LPVOID lpParameter)
{
	char temp[MSGSIZE] = {0};
	while( 1 )
	{
		printf("intput: ");
		fgets(temp, MSGSIZE, stdin);
		temp[strlen(temp) + 1] = '\0';
		strcpy_s(inputbuf, strlen(temp) + 1, temp);

	}
	return 0;
}