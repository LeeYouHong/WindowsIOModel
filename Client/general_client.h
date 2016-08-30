/**
	@描述：套接字模式-阻塞模式-客户端
	@作者：LYH66
*/

#include <WinSock2.h>
#include <WS2tcpip.h>
#include "stdio.h"
#include "Global.h"

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "AdvApi32.lib")

#define MSGSIZE 255

DWORD WINAPI recvThread(LPVOID lpParameter);
DWORD WINAPI inputThread(LPVOID lpParameter);
DWORD SendData(SOCKET hSocket, WORD wMainCmdID, WORD wSubCmdID, void * pData, WORD wDataSize);
DWORD SendDataBuffer(SOCKET hSocket, void * pBuffer, WORD wSendSize);
DWORD SendDetectCommandToServer(SOCKET hSocket);
DWORD SendRegisterCommandToServer(SOCKET hSocket, char *strAccount, char *strPasswd);
DWORD SendLogonCommandToServer(SOCKET hSocket, char *strAccount, char *strPasswd);
WORD SendMessageSingle(SOCKET hSocket, const char *strSender, const char *strRecver, const char *strMessage);

char inputbuf[MSGSIZE] = {0};

int general_client()
{
	WSADATA wsaData;
	SOCKET sClient = INVALID_SOCKET;
	struct addrinfo hints;
	struct addrinfo *list = NULL;
	struct addrinfo *temp = NULL;
	int iResult = 0;
	DWORD inputThreadId;
	DWORD recvThreadId;

	// 初始化Windows Socket库
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	// 解析服务器端口和IP
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	iResult = getaddrinfo("192.168.56.101", "12345", &hints, &list);
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
	
	//SendLogonCommandToServer(sClient, "yufei", "123456");
	//SendRegisterCommandToServer(sClient, "liyou", "123456");
	//SendDetectCommandToServer(sClient);
	// 创建标准输入进程
	CreateThread(NULL, 0, inputThread, NULL, 0, &inputThreadId);

	// 创建接收信息进程
	CreateThread(NULL, 0, recvThread, &sClient, 0, &recvThreadId);
    //Sleep(1000);
	//SendMessageSingle(sClient, "yufei", "liyouhong", "hello");
	//while( 1 )
	//{
	//	// 发送消息
	//	int inputbuflen = (int)strlen(inputbuf);
	//	if ( inputbuflen > 0 )
	//	{
	//		iResult = send(sClient, inputbuf, inputbuflen + 1, 0);

	//		if ( iResult == SOCKET_ERROR )
	//		{
	//			printf("send faild with error:%ld", WSAGetLastError());
	//			closesocket(sClient);
	//			WSACleanup();
	//			return 1;
	//		}
	//		memset(inputbuf, 0, MSGSIZE);
	//	}
	//}
	
	int input;
	scanf("%d", &input);

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
	while ( 1 )
	{
		printf("input: ");
		fgets(temp, MSGSIZE, stdin);
		temp[strlen(temp) - 1] = '\0';
		strcpy_s(inputbuf, strlen(temp) + 1, temp);
	}
	return 0;
}

DWORD WINAPI recvThread(LPVOID lpParameter)
{
	int iResult = 0;
	char recvbuf[MSGSIZE] = {0};
	SOCKET sClient = *(SOCKET*)lpParameter;
   printf("recv\n");
	while ( 1 )
	{
		// 接受消息
		iResult = recv(sClient, recvbuf, MSGSIZE, 0);
		printf("recv size:%d", iResult);
		if ( iResult == SOCKET_ERROR )
		{
			printf("recv faild with error:%ld\n", WSAGetLastError());
			closesocket(sClient);
			return 1;
		}
		printf("\nserver say:%s\n", recvbuf);
	}
	return 0;
}

DWORD SendData(SOCKET hSocket, WORD wMainCmdID, WORD wSubCmdID, void * pData, WORD wDataSize)
{
	//效验状态
	if (hSocket==INVALID_SOCKET) return false;

	//效验大小
	if (wDataSize>SOCKET_PACKET) return false;

	//构造数据
	BYTE cbDataBuffer[SOCKET_BUFFER];
	CMD_Head * pHead=(CMD_Head *)cbDataBuffer;
	pHead->CommandInfo.wMainCmdID=wMainCmdID;
	pHead->CommandInfo.wSubCmdID=wSubCmdID;

	//填写信息头
	pHead->CmdInfo.cbCheckCode=0;
	pHead->CmdInfo.wPacketSize=sizeof(CMD_Head)+wDataSize;
	pHead->CmdInfo.cbVersion=SOCKET_VER;

	if (wDataSize>0)
	{
		CopyMemory(pHead+1,pData,wDataSize);
	}

	WORD wSendSize = sizeof(CMD_Head) + wDataSize;
	//发送数据
	return SendDataBuffer(hSocket, cbDataBuffer,wSendSize);
}

DWORD SendDataBuffer(SOCKET hSocket, void * pBuffer, WORD wSendSize)
{
	//发送数据
	WORD wSended=0;
	while (wSended<wSendSize)
	{
		int iErrorCode=send(hSocket,(char *)pBuffer+wSended,wSendSize-wSended,0);
		if (iErrorCode==SOCKET_ERROR)
		{
			if (WSAGetLastError()==WSAEWOULDBLOCK)
			{
				return true;
			}
			return false;
		}
		wSended+=iErrorCode;
	}
	return true;
}

DWORD SendDetectCommandToServer( SOCKET hSocket )
{
  BYTE cbDataBuffer[SOCKET_PACKET + sizeof(CMD_Head)];

  WORD wDataSize = 0;

  SendData(hSocket, MDM_KN_COMMAND,SUB_KN_DETECT_SOCKET,cbDataBuffer,wDataSize);

  return 0;
}

DWORD SendRegisterCommandToServer(SOCKET hSocket, char *strAccount, char *strPasswd)
{
   BYTE cbBuffer[SOCKET_PACKET];
   CMD_GP_RegisterAccounts * pRegisterAccounts=(CMD_GP_RegisterAccounts *)cbBuffer;
   memset(pRegisterAccounts,0,sizeof(CMD_GP_RegisterAccounts));
   pRegisterAccounts->dwPlazaVersion = 0;
   pRegisterAccounts->cbGender = 0;
   lstrcpynA(pRegisterAccounts->szPassWord, strPasswd,CountArray(pRegisterAccounts->szPassWord));
   lstrcpynA(pRegisterAccounts->szAccounts, strAccount, CountArray(pRegisterAccounts->szAccounts));

   SendData(hSocket, MDM_GP_LOGON, SUB_GP_REGISTER_ACCOUNTS, cbBuffer, sizeof(CMD_GP_RegisterAccounts));
   return 0;
}

DWORD SendLogonCommandToServer(SOCKET hSocket, char *strAccount, char *strPasswd)
{
	BYTE cbBuffer[SOCKET_PACKET];
	CMD_GP_LogonByAccounts * pRegisterAccounts=(CMD_GP_LogonByAccounts *)cbBuffer;
	memset(pRegisterAccounts,0,sizeof(CMD_GP_LogonByAccounts));
	pRegisterAccounts->dwPlazaVersion = 0;
	lstrcpynA(pRegisterAccounts->szPassWord, strPasswd,CountArray(pRegisterAccounts->szPassWord));
	lstrcpynA(pRegisterAccounts->szAccounts, strAccount, CountArray(pRegisterAccounts->szAccounts));

	SendData(hSocket, MDM_GP_LOGON, SUB_GP_LOGON_ACCOUNTS, cbBuffer, sizeof(CMD_GP_LogonByAccounts));
	printf("发送登陆命令成功");
	return 0;
}

WORD SendMessageSingle(SOCKET hSocket, const char *strSender, const char *strRecver, const char *strMessage)
{
	BYTE cbBuffer[SOCKET_BUFFER];
	CMD_GP_ChatBySingle *pChat = (CMD_GP_ChatBySingle *)cbBuffer;
	memset(pChat, 0, sizeof(CMD_GP_ChatBySingle));
	lstrcpynA(pChat->szSender, strSender, sizeof(pChat->szSender));
	lstrcpynA(pChat->szRecver, strRecver, sizeof(pChat->szRecver));
	lstrcpynA(pChat->szMessage, strMessage, sizeof(pChat->szMessage));
	
	SendData(hSocket, MDM_GP_CHAT, SUB_GP_CHAT_SINGLE, cbBuffer, sizeof(CMD_GP_ChatBySingle));
	printf("%sTo%s发送%s成功", strSender, strRecver, strMessage);
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
		//strcpy_s(inputbuf, strlen(temp) + 1, temp);
		SendMessageSingle(sClient, "yufei", "liyouhong", temp);
	}
	return 0;
}