/**
	@��������ɶ˿�����
			ӵ�н����̡߳���д�߳�
	@���ߣ�LYH66
*/

// winsock 2 ��ͷ�ļ��Ϳ�
#include <winsock2.h>
#include <MSWSock.h>
#include <iostream>
#include <vector>
#include "process.h"
#include "IOCPModel.h"

#pragma comment(lib,"ws2_32.lib")

using namespace std;

//�̲߳���
typedef struct
{
	HANDLE hCompletionPort;
	bool	bRun;
}tagThreadData;

#define MYDELETE(obj)	if((obj) != NULL) {delete (obj); (obj) = NULL;}

/////////////////////////ȫ�ַ���///////////////////////////////////
//����˽����߳�
unsigned __stdcall AccepteThread(LPVOID pThreadData);

//����˶�д�߳�
unsigned __stdcall RSThread(LPVOID pThreadData);

//Ͷ��һ��Recv����
bool PostRecv(CSocketItem *pSocketContext, CIoContext *pIoContext);

//�������ݻؿͻ���
bool SendToClient(CSocketItem *pSocketContext, CIoContext *pIoContext);

//ȫ�ֱ���
std::vector<CSocketItem *> g_ArraySocketItem;

int main()
{
	HANDLE hCompletionPort;
	HANDLE hRSThread[8];
	WSADATA wsaData;
	bool	bRunRsThread[8];
	bool	bRunAccepted;

	try
	{
		//��ʼ��winsock
		int nResult;
		nResult = WSAStartup(MAKEWORD(2,2), &wsaData);
		if (nResult != NO_ERROR)
		{
			cout<<"��ʼ���׽���ʧ�ܣ�"<<endl;
			return -1;
		}

		hCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
		if ( hCompletionPort == NULL )
		{
			throw TEXT("��ɶ˿ڴ���ʧ��");
		}

		tagThreadData ThreadData;
		ZeroMemory(&ThreadData, sizeof(tagThreadData));
		ThreadData.hCompletionPort = hCompletionPort;

		//������д�߳�
		for (int i = 0; i < 8; i++)
		{
			bRunRsThread[i] = true;
			ThreadData.bRun = bRunRsThread[i];
			hRSThread[i] = (HANDLE)_beginthreadex(NULL,0,RSThread,&ThreadData, 0, NULL);
			if (hRSThread[i] == INVALID_HANDLE_VALUE)
			{
				char msg[200];
				sprintf_s(msg, 200, "%d�Ŷ�д�̴߳���ʧ��", i);
				cout<<msg<<endl;
			}
		}

		//�ȴ�����
		bRunAccepted = true;
		ThreadData.bRun = bRunAccepted;
		AccepteThread(&ThreadData);
	}
	catch(...)
	{
				
	}
	return 0;
}

unsigned __stdcall AccepteThread(LPVOID pThreadData)
{
	SOCKADDR_IN			SocketAddrServer, SocketAddrClient;
	SOCKET				hConnectedSocket = INVALID_SOCKET;
	int					nBufferSize = sizeof(SocketAddrClient);
	SOCKET				hListenSocket = INVALID_SOCKET;
	WORD				wListenPort = 12345;

	//��ȡ�̲߳���
	tagThreadData	*pParam = (tagThreadData*)pThreadData;
	HANDLE hIoCompletePort = pParam->hCompletionPort;
	bool bRun = pParam->bRun;

	//���������˿�
	memset(&SocketAddrServer, 0, sizeof(SocketAddrServer));
	SocketAddrServer.sin_addr.s_addr = INADDR_ANY;
	SocketAddrServer.sin_family = AF_INET;
	SocketAddrServer.sin_port = htons(wListenPort);

	hListenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (hListenSocket == INVALID_SOCKET) throw TEXT("�����׽��ִ���ʧ��");
	int iErrorCode = bind(hListenSocket, (SOCKADDR*)&SocketAddrServer, sizeof(SocketAddrServer));
	if (iErrorCode == SOCKET_ERROR) throw TEXT("�˿ڱ�ռ�ã��˿ڰ�ʧ��");
	iErrorCode = listen(hListenSocket, 2000);
	if (iErrorCode == SOCKET_ERROR) throw TEXT("�˿ڱ�ռ�ã��˿ڰ�ʧ��");
	while ( bRun )
	{
		try
		{
			hConnectedSocket = WSAAccept(hListenSocket, (SOCKADDR*) &SocketAddrClient, &nBufferSize, NULL, NULL);
			if (hConnectedSocket == INVALID_SOCKET)
			{
				cout<<"WSAAccept���ص���һ����Ч��Socket"<<endl;
				return 0;
			}

			//�����׽���������
			CSocketItem *pNewSocketItem = new CSocketItem;
			pNewSocketItem->m_Socket = hConnectedSocket;

			//����ɶ˿�
			HANDLE hTemp = CreateIoCompletionPort((HANDLE)pNewSocketItem->m_Socket, hIoCompletePort, (DWORD)pNewSocketItem, 0);
			if (hTemp == NULL)
			{
				cout<<"����ɶ˿�ʧ��"<<endl;
				delete pNewSocketItem;
				return -1;
			}

			//��SocketContext�ӽ�ȫ�ֵ�SocketContext������
			g_ArraySocketItem.push_back(pNewSocketItem);

			//����IO������
			CIoContext	*pNewIoContext = pNewSocketItem->GetNewIoContext();
			pNewIoContext->m_OpType	= enOperation_RECV;

			//Ͷ�ݽ�����������
			DWORD	dwBytes = 0;
			DWORD   dwFlags = 0;
			WSABUF  *pWbuf = &pNewIoContext->m_wsaBuf;
			OVERLAPPED *pOverlapped = &pNewIoContext->m_Overlapped;

			//Ͷ�ݵ�һ��recv����
			int nBytesRecv = WSARecv(pNewSocketItem->m_Socket, pWbuf, 1, &dwBytes, &dwFlags, pOverlapped, NULL);
			if (nBytesRecv == SOCKET_ERROR && (WSA_IO_PENDING != WSAGetLastError()))
			{
				cout<<"Ͷ�ݵ�һ��WSARecvʧ��,������룺"<<WSAGetLastError()<<endl;
				return -1;
			}
		}
		catch(...)
		{
			cout<<"�������ӻ����ɶ˿��쳣"<<endl;
			closesocket(hConnectedSocket);
		}
	}
	
	return 0;
}

unsigned __stdcall RSThread(LPVOID lpParam)
{
	tagThreadData			*pParam = (tagThreadData*)lpParam;
	HANDLE					hCompletePort = pParam->hCompletionPort;
	bool					bRun = pParam->bRun;

	OVERLAPPED				*pOverlapped = NULL;
	CSocketItem				*pSocketItem = NULL;
	DWORD					 dwBytesTransfered = 0;

	while (bRun)
	{
		BOOL bReturn = GetQueuedCompletionStatus(hCompletePort, &dwBytesTransfered, (PULONG_PTR)&pSocketItem, &pOverlapped, INFINITE);
		if ( !bReturn )
		{
			char szDebug[255];
			sprintf_s(szDebug, 255, "�ȴ���ɶ˿ڳ���%d", GetLastError());
			cout<<szDebug<<endl;
		}
		else
		{
			// ��ȡ����Ĳ���
			CIoContext* pIoContext = CONTAINING_RECORD(pOverlapped, CIoContext, m_Overlapped);  
			switch( pIoContext->m_OpType )
			{
			case enOperation_RECV:
				{
					//��ӡ�յ�������
					cout<<"�յ���Ϣ:"<<pIoContext->m_wsaBuf.buf<<endl;
					//ԭ���������ݸ��ͻ���
					SendToClient(pSocketItem, pIoContext);
					//Ͷ����һ��Recv����
					PostRecv(pSocketItem, pIoContext);
					break;
				}
			case enOperation_SEND:
				{
					cout<<"�������"<<endl;
				}
				break;
			default:
				//��Ӧ��ִ�е�����
				cout<<"RSThread�е� pIoContext->m_OpType �����쳣.\n"<<endl;
				break;
			}
		}
	}
	return 0;
}

bool PostRecv(CSocketItem *pSocketContext, CIoContext *pIoContext)
{
	//��ʼ������
	DWORD dwFlags = 0;
	DWORD dwBytes = 0;

	pIoContext->m_OpType = enOperation_RECV;

	int nBytesRecv = WSARecv(pSocketContext->m_Socket, &pIoContext->m_wsaBuf, 1, &dwBytes, &dwFlags, &pIoContext->m_Overlapped, NULL);
	
	// �������ֵ���󣬲��Ҵ���Ĵ��벢����Pending�Ļ����Ǿ�˵������ص�����ʧ����
	if ((SOCKET_ERROR == nBytesRecv) && (WSA_IO_PENDING != WSAGetLastError()))
	{
		cout<<"Ͷ�ݵ�һ��WSARecvʧ�ܣ���"<<endl;
		return false;
	}
	return true;
}

bool SendToClient( CSocketItem *pSocketContext, CIoContext *pIoContext)
{
	//��������io������
	CIoContext *pNewIoContext = pSocketContext->GetNewIoContext();
	pNewIoContext->m_OpType = enOperation_SEND;
	CopyMemory(pNewIoContext->m_wsaBuf.buf, pIoContext->m_wsaBuf.buf, pIoContext->m_wsaBuf.len);

	DWORD dwTrancferred;

	int iRetCode = WSASend(pSocketContext->m_Socket, &pNewIoContext->m_wsaBuf, 1, &dwTrancferred, 0, &pNewIoContext->m_Overlapped, NULL);
	if ((iRetCode == SOCKET_ERROR) && (WSAGetLastError() != WSA_IO_PENDING))
	{
		cout<<"����ԭ�����ݸ��ͻ��˳���:"<<WSAGetLastError()<<endl;
		return false;
	}
	return true;
}
