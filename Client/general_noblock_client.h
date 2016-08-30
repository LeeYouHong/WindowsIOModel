/**
	@������������ʽ-������ģʽ-�ͻ���
	@���ߣ�LYH66
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

	// ��ʼ��Windows Socket��
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	// �����������˿ں�IP
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
		// ����һ���׽���	
		sClient = socket(temp->ai_family, temp->ai_socktype, temp->ai_protocol);
		if ( sClient == INVALID_SOCKET )
		{
			printf("socket faild with error: %ld\n", WSAGetLastError());
			freeaddrinfo(list);
			WSACleanup();
			return 1;
		}

		// ���ӵ������
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
	
	// ��ֹû�����ӳɹ�
	if ( sClient == INVALID_SOCKET )
	{
		printf("Unable to connect to server!\n");
		WSACleanup();
		return 1;
	}
	
	printf(" connect success!\n");

	// ���÷�����
	iResult = ioctlsocket(sClient, FIONBIO, &noblock);
	if ( iResult == SOCKET_ERROR )
	{
		closesocket(sClient);
		WSACleanup();
		return 1;
	}
	
	// ����һ����׼������߳�
	CreateThread(NULL, 0, inputThread, NULL, 0, &dwThreadId);

	while( 1 )
	{
		// ������Ϣ
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

		// ������Ϣ
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

	// �ر�����
	iResult = shutdown(sClient, SD_SEND);
	if ( iResult == SOCKET_ERROR )
	{
		printf("shutdown faild with error: %ld", WSAGetLastError());
		closesocket(sClient);
		WSACleanup();
		return 1;
	}

	// ɨβ
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