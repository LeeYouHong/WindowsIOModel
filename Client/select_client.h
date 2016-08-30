/**
	@������Select I/Oģ��-�ͻ���
	@���ߣ�LYH66
*/

#include <WinSock2.h>
#include <WS2tcpip.h>
#include "stdio.h"
#include <iostream>
#include <string>
#include "Global.h"
using namespace std;

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "AdvApi32.lib")

#define MSGSIZE 255

DWORD WINAPI inputThread(LPVOID lpParameter);
DWORD SendData(SOCKET hSocket, WORD wMainCmdID, WORD wSubCmdID, void * pData, WORD wDataSize);
DWORD SendDataBuffer(SOCKET hSocket, void * pBuffer, WORD wSendSize);
DWORD SendDetectCommandToServer(SOCKET hSocket);
DWORD SendRegisterCommandToServer(SOCKET hSocket, char *strAccount, const char *strPasswd);
DWORD SendLogonCommandToServer(SOCKET hSocket, const char *strAccount, const char *strPasswd);
DWORD ParserInput(std::string input, char name[], char message[]);
DWORD ParserInput(std::string &input, string &name, string &message);
WORD SendMessageSingle(SOCKET hSocket, const char *strSender, const char *strRecver, const char *strMessage);
WORD SendMessageAll(SOCKET hSocket, const char *strSender, const char *strMessage);

char inputbuf[MSGSIZE] = {0};
SOCKET sClient = INVALID_SOCKET;
string Account;
string strInput;

int select_client()
{
	WSADATA wsaData;

	struct addrinfo hints;
	struct addrinfo *list = NULL;
	struct addrinfo *temp = NULL;
	char szMessage[MSGSIZE];
	int iResult = 0;
	char sendbuf[MSGSIZE]= {0};
	char recvbuf[MSGSIZE] = {0};
	int recvbuflen = MSGSIZE;
	fd_set fwrites;
	fd_set freads;
	timeval tv={1, 0};
	DWORD inputThreadId;
	// ��ʼ��Windows Socket��
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	// �����������˿ں�IP
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

	//���͵�½��
	printf("input you name:");
    cin>>Account;
	SendLogonCommandToServer(sClient, Account.c_str(), "123456");


	CreateThread(NULL, 0, inputThread, NULL, 0, &inputThreadId)	;
	while( 1 )
	{
		// �����׽���
		FD_ZERO(&fwrites);
		FD_ZERO(&freads);

		// �����׽���
		FD_SET(sClient, &fwrites);
		FD_SET(sClient, &freads);

		// ��ѯ�׽��ֵ�״̬
		iResult = select(sClient, &freads, &fwrites, NULL, &tv);
		if ( iResult == SOCKET_ERROR )
		{
			closesocket(sClient);
			WSACleanup();
			return 1;
		}else if( iResult == 0 )  // ʱ�䵽��
		{
			continue;
		}
		
		if ( FD_ISSET(sClient, &fwrites) )
		{
			// ������Ϣ
			int inputbuflen = (int)strlen(inputbuf);
			if ( inputbuflen > 0)
			{
				//iResult = send(sClient, inputbuf, inputbuflen + 1, 0);
				SendMessageSingle(sClient, "liyouhong", "yufei", inputbuf);
				//if ( iResult == SOCKET_ERROR )
				//{
				//	printf("send faild with error:%ld", WSAGetLastError());
				//	closesocket(sClient);
				//	WSACleanup();
				//	return 1;
				//}
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
		printf("intput: ");
		//fgets(temp, MSGSIZE, stdin);
		//temp[strlen(temp) + 1] = '\0';
		//strcpy_s(inputbuf, strlen(temp) + 1, temp);
		//std::cin>>str;
		std::string name;
		std::string message;
		string input;
		cout<<"input you message with name:message"<<endl;
		cin>>input;
		ParserInput(input, name, message);
		SendMessageSingle(sClient, Account.c_str(), name.c_str(), message.c_str());
	}
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
	printf("%sTo%s����%s�ɹ�", strSender, strRecver, strMessage);
	return 0;
}


DWORD SendData(SOCKET hSocket, WORD wMainCmdID, WORD wSubCmdID, void * pData, WORD wDataSize)
{
	//Ч��״̬
	if (hSocket==INVALID_SOCKET) return false;

	//Ч���С
	if (wDataSize>SOCKET_PACKET) return false;

	//��������
	BYTE cbDataBuffer[SOCKET_BUFFER];
	CMD_Head * pHead=(CMD_Head *)cbDataBuffer;
	pHead->CommandInfo.wMainCmdID=wMainCmdID;
	pHead->CommandInfo.wSubCmdID=wSubCmdID;

	//��д��Ϣͷ
	pHead->CmdInfo.cbCheckCode=0;
	pHead->CmdInfo.wPacketSize=sizeof(CMD_Head)+wDataSize;
	pHead->CmdInfo.cbVersion=SOCKET_VER;

	if (wDataSize>0)
	{
		CopyMemory(pHead+1,pData,wDataSize);
	}

	WORD wSendSize = sizeof(CMD_Head) + wDataSize;
	//��������
	return SendDataBuffer(hSocket, cbDataBuffer,wSendSize);
}

DWORD SendDataBuffer(SOCKET hSocket, void * pBuffer, WORD wSendSize)
{
	//��������
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

DWORD SendRegisterCommandToServer(SOCKET hSocket, const char *strAccount, const char *strPasswd)
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

DWORD SendLogonCommandToServer(SOCKET hSocket, const char *strAccount, const char *strPasswd)
{
	BYTE cbBuffer[SOCKET_PACKET];
	CMD_GP_LogonByAccounts * pRegisterAccounts=(CMD_GP_LogonByAccounts *)cbBuffer;
	memset(pRegisterAccounts,0,sizeof(CMD_GP_LogonByAccounts));
	pRegisterAccounts->dwPlazaVersion = 0;
	lstrcpynA(pRegisterAccounts->szPassWord, strPasswd,CountArray(pRegisterAccounts->szPassWord));
	lstrcpynA(pRegisterAccounts->szAccounts, strAccount, CountArray(pRegisterAccounts->szAccounts));

	SendData(hSocket, MDM_GP_LOGON, SUB_GP_LOGON_ACCOUNTS, cbBuffer, sizeof(CMD_GP_LogonByAccounts));
	printf("���͵�½����ɹ�");
	return 0;
}

DWORD ParserInput(std::string input, char name[], char message[])
{
	std::string::size_type index1 = input.find_first_of(':');
	std::string cmd = input.substr(0, index1);
	std::string strMessage = input.substr(index1 + 1, input.size());
	return 0;
}

DWORD ParserInput(std::string &input, string &name, string &message)
{
	std::string::size_type index1 = input.find_first_of(':');
	name = input.substr(0, index1);
	message = input.substr(index1 + 1, input.size());
	return 0;
}

WORD SendMessageAll(SOCKET hSocket, const char *strSender, const char *strMessage)
{
     return 0;
}