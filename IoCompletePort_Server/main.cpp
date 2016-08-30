/**
	@描述：完成端口例子
			拥有接听线程、读写线程
	@作者：LYH66
*/

// winsock 2 的头文件和库
#include <winsock2.h>
#include <MSWSock.h>
#include <iostream>
#include <vector>
#include "process.h"
#include "IOCPModel.h"

#pragma comment(lib,"ws2_32.lib")

using namespace std;

//线程参数
typedef struct
{
	HANDLE hCompletionPort;
	bool	bRun;
}tagThreadData;

#define MYDELETE(obj)	if((obj) != NULL) {delete (obj); (obj) = NULL;}

/////////////////////////全局方法///////////////////////////////////
//服务端接受线程
unsigned __stdcall AccepteThread(LPVOID pThreadData);

//服务端读写线程
unsigned __stdcall RSThread(LPVOID pThreadData);

//投递一个Recv请求
bool PostRecv(CSocketItem *pSocketContext, CIoContext *pIoContext);

//发送数据回客户端
bool SendToClient(CSocketItem *pSocketContext, CIoContext *pIoContext);

//全局变量
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
		//初始化winsock
		int nResult;
		nResult = WSAStartup(MAKEWORD(2,2), &wsaData);
		if (nResult != NO_ERROR)
		{
			cout<<"初始化套接字失败！"<<endl;
			return -1;
		}

		hCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
		if ( hCompletionPort == NULL )
		{
			throw TEXT("完成端口创建失败");
		}

		tagThreadData ThreadData;
		ZeroMemory(&ThreadData, sizeof(tagThreadData));
		ThreadData.hCompletionPort = hCompletionPort;

		//建立读写线程
		for (int i = 0; i < 8; i++)
		{
			bRunRsThread[i] = true;
			ThreadData.bRun = bRunRsThread[i];
			hRSThread[i] = (HANDLE)_beginthreadex(NULL,0,RSThread,&ThreadData, 0, NULL);
			if (hRSThread[i] == INVALID_HANDLE_VALUE)
			{
				char msg[200];
				sprintf_s(msg, 200, "%d号读写线程创建失败", i);
				cout<<msg<<endl;
			}
		}

		//等待连接
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

	//提取线程参数
	tagThreadData	*pParam = (tagThreadData*)pThreadData;
	HANDLE hIoCompletePort = pParam->hCompletionPort;
	bool bRun = pParam->bRun;

	//建立监听端口
	memset(&SocketAddrServer, 0, sizeof(SocketAddrServer));
	SocketAddrServer.sin_addr.s_addr = INADDR_ANY;
	SocketAddrServer.sin_family = AF_INET;
	SocketAddrServer.sin_port = htons(wListenPort);

	hListenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (hListenSocket == INVALID_SOCKET) throw TEXT("监听套接字创建失败");
	int iErrorCode = bind(hListenSocket, (SOCKADDR*)&SocketAddrServer, sizeof(SocketAddrServer));
	if (iErrorCode == SOCKET_ERROR) throw TEXT("端口被占用，端口绑定失败");
	iErrorCode = listen(hListenSocket, 2000);
	if (iErrorCode == SOCKET_ERROR) throw TEXT("端口被占用，端口绑定失败");
	while ( bRun )
	{
		try
		{
			hConnectedSocket = WSAAccept(hListenSocket, (SOCKADDR*) &SocketAddrClient, &nBufferSize, NULL, NULL);
			if (hConnectedSocket == INVALID_SOCKET)
			{
				cout<<"WSAAccept返回的是一个无效的Socket"<<endl;
				return 0;
			}

			//设置套接字上下文
			CSocketItem *pNewSocketItem = new CSocketItem;
			pNewSocketItem->m_Socket = hConnectedSocket;

			//绑定完成端口
			HANDLE hTemp = CreateIoCompletionPort((HANDLE)pNewSocketItem->m_Socket, hIoCompletePort, (DWORD)pNewSocketItem, 0);
			if (hTemp == NULL)
			{
				cout<<"绑定完成端口失败"<<endl;
				delete pNewSocketItem;
				return -1;
			}

			//把SocketContext加进全局的SocketContext管理器
			g_ArraySocketItem.push_back(pNewSocketItem);

			//建立IO上下文
			CIoContext	*pNewIoContext = pNewSocketItem->GetNewIoContext();
			pNewIoContext->m_OpType	= enOperation_RECV;

			//投递接受数据请求
			DWORD	dwBytes = 0;
			DWORD   dwFlags = 0;
			WSABUF  *pWbuf = &pNewIoContext->m_wsaBuf;
			OVERLAPPED *pOverlapped = &pNewIoContext->m_Overlapped;

			//投递第一个recv请求
			int nBytesRecv = WSARecv(pNewSocketItem->m_Socket, pWbuf, 1, &dwBytes, &dwFlags, pOverlapped, NULL);
			if (nBytesRecv == SOCKET_ERROR && (WSA_IO_PENDING != WSAGetLastError()))
			{
				cout<<"投递第一个WSARecv失败,错误代码："<<WSAGetLastError()<<endl;
				return -1;
			}
		}
		catch(...)
		{
			cout<<"接受连接或绑定完成端口异常"<<endl;
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
			sprintf_s(szDebug, 255, "等待完成端口出错：%d", GetLastError());
			cout<<szDebug<<endl;
		}
		else
		{
			// 读取传入的参数
			CIoContext* pIoContext = CONTAINING_RECORD(pOverlapped, CIoContext, m_Overlapped);  
			switch( pIoContext->m_OpType )
			{
			case enOperation_RECV:
				{
					//打印收到的数据
					cout<<"收到消息:"<<pIoContext->m_wsaBuf.buf<<endl;
					//原样返回数据给客户端
					SendToClient(pSocketItem, pIoContext);
					//投递下一个Recv请求
					PostRecv(pSocketItem, pIoContext);
					break;
				}
			case enOperation_SEND:
				{
					cout<<"发送完成"<<endl;
				}
				break;
			default:
				//不应该执行到这里
				cout<<"RSThread中的 pIoContext->m_OpType 参数异常.\n"<<endl;
				break;
			}
		}
	}
	return 0;
}

bool PostRecv(CSocketItem *pSocketContext, CIoContext *pIoContext)
{
	//初始化变量
	DWORD dwFlags = 0;
	DWORD dwBytes = 0;

	pIoContext->m_OpType = enOperation_RECV;

	int nBytesRecv = WSARecv(pSocketContext->m_Socket, &pIoContext->m_wsaBuf, 1, &dwBytes, &dwFlags, &pIoContext->m_Overlapped, NULL);
	
	// 如果返回值错误，并且错误的代码并非是Pending的话，那就说明这个重叠请求失败了
	if ((SOCKET_ERROR == nBytesRecv) && (WSA_IO_PENDING != WSAGetLastError()))
	{
		cout<<"投递第一个WSARecv失败！！"<<endl;
		return false;
	}
	return true;
}

bool SendToClient( CSocketItem *pSocketContext, CIoContext *pIoContext)
{
	//建立发送io上下文
	CIoContext *pNewIoContext = pSocketContext->GetNewIoContext();
	pNewIoContext->m_OpType = enOperation_SEND;
	CopyMemory(pNewIoContext->m_wsaBuf.buf, pIoContext->m_wsaBuf.buf, pIoContext->m_wsaBuf.len);

	DWORD dwTrancferred;

	int iRetCode = WSASend(pSocketContext->m_Socket, &pNewIoContext->m_wsaBuf, 1, &dwTrancferred, 0, &pNewIoContext->m_Overlapped, NULL);
	if ((iRetCode == SOCKET_ERROR) && (WSAGetLastError() != WSA_IO_PENDING))
	{
		cout<<"发送原样数据给客户端出错:"<<WSAGetLastError()<<endl;
		return false;
	}
	return true;
}
