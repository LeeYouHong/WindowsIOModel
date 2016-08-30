#include "IOCPManager.h"

#include <iostream>
#include <thread>
#include "SocketItem.h"
#include "Connection.h"

CIOCPManager::CIOCPManager()
	:m_bRun(false)
	, m_hCompletePort(nullptr)
{

}

CIOCPManager::~CIOCPManager()
{
	m_bRun = false;

/*	for (auto item : m_ConnectionList)
	{
		SAFE_DEL(item);
	}
	m_ConnectionList.clear()*/;
}

void CIOCPManager::Start()
{
	//初始化winsock
	WSADATA wsaData;
	int nResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (nResult != NO_ERROR)
	{
		std::cout << "初始化套接字失败！" << std::endl;
		return;
	}

	m_hCompletePort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
	if (m_hCompletePort == NULL)
	{
		std::cout << "完成端口创建失败" << std::endl;
		return;
	}

	// 获取CPU数目
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	int rsThreadNum = si.dwNumberOfProcessors * 2 + 2;

	// 创建工作线程
	m_bRun = true;
	for (int i = 0; i < rsThreadNum; ++i)
	{
		std::thread workProc(Run, this);
		workProc.detach();
	}
}

void CIOCPManager::Run(CIOCPManager* instance)
{
	OVERLAPPED				*pOverlapped = NULL;
	CSocketItem				*pSocketItem = NULL;
	DWORD					 dwBytesTransfered = 0;

	while (instance->m_bRun)
	{
		BOOL bReturn = GetQueuedCompletionStatus(instance->m_hCompletePort, &dwBytesTransfered, (PULONG_PTR)&pSocketItem, &pOverlapped, INFINITE);
		if (!bReturn)
		{
			char szDebug[255];
			sprintf_s(szDebug, 255, "等待完成端口出错：%d", GetLastError());
			std::cout << szDebug << std::endl;
			
			// 退出工作线程
			if (GetLastError() == ERROR_INVALID_HANDLE)
			{
				return;
			}
		}
		else
		{
			// 读取传入的参数
			CIOContext* pIoContext = CONTAINING_RECORD(pOverlapped, CIOContext, m_Overlapped);
			switch (pIoContext->m_OpType)
			{
			case enOperation_CONNECT:	// 连接完成通知
			{
				//// 发送第一条数据
				//instance->SendData(pSocketItem, "Hello", sizeof("Hello"));

				// 连接成功处理
				pSocketItem->m_pConnection->OnConnect();

				//投递第一个recv请求
				CIOContext* pNewIOContext = pSocketItem->GetFreeIoContext();
				instance->PostRecv(pSocketItem, pIoContext);
				break;
			}
			case enOperation_RECV:	// 数据接收完成通知
			{
				// 判断套接字是否关闭
				if (0 == dwBytesTransfered)
				{
					pSocketItem->m_pConnection->OnClose();
					continue;
				}
				//打印收到的数据
				//std::cout << "收到消息:" << pIoContext->m_wsaBuf.buf << std::endl;
				DWORD dwFlags = 0;

				// 接手到消息处理
				pSocketItem->m_pConnection->OnRecv(pIoContext->m_wsaBuf.buf, dwBytesTransfered);

				//pSocketItem->m_RecvBuffer.WriteBuf(pIoContext->m_wsaBuf.buf, pIoContext->m_wsaBuf.len);

				//投递下一个Recv请求
				CIOContext* pNewIOContext = pSocketItem->GetFreeIoContext();
				instance->PostRecv(pSocketItem, pIoContext);

				//pSocketItem->Close();
				break;
			}
			case enOperation_SEND:	// 数据发送完成通知
			{
				// 判断套接字是否关闭
				if (0 == dwBytesTransfered)
				{
					pSocketItem->m_pConnection->OnClose();
					continue;
				}

				// 发送数据完成处理
				pSocketItem->m_pConnection->OnSend();

				pSocketItem->FreeIOContext(pIoContext);
			}
			break;
			default:
				//不应该执行到这里
				std::cout << "RSThread中的 pIoContext->m_OpType 参数异常." << std::endl;
				break;
			}
		}
	}
}

bool CIOCPManager::SendData(CSocketItem* pSocketItem, char* sendBuf, int bufLength)
{
	//建立发送io上下文
	CIOContext *pNewIoContext = pSocketItem->GetFreeIoContext();
	pNewIoContext->m_OpType = enOperation_SEND;
	CopyMemory(pNewIoContext->m_wsaBuf.buf, sendBuf, bufLength);
	pNewIoContext->m_wsaBuf.len = bufLength;

	DWORD dwTrancferred;

	int iRetCode = WSASend(pSocketItem->m_Socket, &pNewIoContext->m_wsaBuf, 1, &dwTrancferred, 0, &pNewIoContext->m_Overlapped, NULL);
	if ((iRetCode == SOCKET_ERROR) && (WSAGetLastError() != WSA_IO_PENDING))
	{
		std::cout << "发送数据给服务端出错:" << WSAGetLastError() << std::endl;
		return false;
	}
	return true;
}

bool CIOCPManager::Connect(CConnection* pNewConn, std::string ip, int port)
{
	LPFN_CONNECTEX lpfnConnectEx = NULL;
	DWORD dwBytes = 0;
	GUID GuidConnectEx = WSAID_CONNECTEX;

	// 创建连接型的客户端套接字
	SOCKET hClientSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (hClientSocket == INVALID_SOCKET)
	{
		std::cout << "监听套接字创建失败:" << WSAGetLastError() << std::endl;
		return false;
	}

	// 需要绑定套接字
	SOCKADDR_IN localAddr;
	ZeroMemory(&localAddr, sizeof(SOCKADDR_IN));
	localAddr.sin_family = AF_INET;
	if (SOCKET_ERROR == ::bind(hClientSocket, (sockaddr*)&localAddr, sizeof(SOCKADDR_IN)))
	{
		std::cout << "绑定本地套接字失败:" << WSAGetLastError() << std::endl;
		return false;
	}

	// 获取ConnectEx的函数指针
	if (SOCKET_ERROR == WSAIoctl(hClientSocket, SIO_GET_EXTENSION_FUNCTION_POINTER, &GuidConnectEx, sizeof(GuidConnectEx),
		&lpfnConnectEx, sizeof(lpfnConnectEx), &dwBytes, 0, 0))
	{
		std::cout << "获取ConnectEx函数指针错误:" << WSAGetLastError() << std::endl;
		return false;
	}

	// 创建套接字项
	pNewConn->m_SocketItem.m_Socket = hClientSocket;

	// 创建IO上下文
	CIOContext* pIoContext = pNewConn->m_SocketItem.GetFreeIoContext();
	pIoContext->m_OpType = enOperation_CONNECT;

	//绑定完成端口
	HANDLE hTemp = CreateIoCompletionPort((HANDLE)pNewConn->m_SocketItem.m_Socket, m_hCompletePort, (DWORD)&pNewConn->m_SocketItem, 0);
	if (hTemp == NULL)
	{
		pNewConn->m_SocketItem.Close();
		SAFE_DEL(pNewConn);
		std::cout << "绑定完成端口失败:" << WSAGetLastError() << std::endl;
		return false;
	}

	// 设置连接目标
	SOCKADDR_IN addrPeer;
	ZeroMemory(&addrPeer, sizeof(SOCKADDR_IN));
	addrPeer.sin_family = AF_INET;
	inet_pton(AF_INET, ip.c_str(), &addrPeer.sin_addr.s_addr);
	addrPeer.sin_port = htons(port);

	// 连接到远端
	int nLen = sizeof(addrPeer);
	PVOID lpSendBuffer = NULL;
	DWORD dwSendDataLength = 0;
	DWORD dwBytesSent = 0;

	BOOL bResult = lpfnConnectEx(hClientSocket,
		(sockaddr*)&addrPeer,
		nLen,
		lpSendBuffer,
		dwSendDataLength,
		&dwBytesSent,
		&pIoContext->m_Overlapped
		);

	if (!bResult)
	{
		int nErrorCode = WSAGetLastError();
		if (nErrorCode != ERROR_IO_PENDING)
		{
			pNewConn->m_SocketItem.Close();
			SAFE_DEL(pNewConn);

			std::cout << "ConnectEx失败:" << nErrorCode << std::endl;
			return false;
		}
	}

	// 保存连接
	//m_ConnectionList.push_back(pNewConn);

	return true;
}

void CIOCPManager::Stop()
{
	m_bRun = false;
	CloseHandle(m_hCompletePort);
}

bool CIOCPManager::PostRecv(CSocketItem *pSocketContext, CIOContext *pIoContext)
{
	//初始化变量
	DWORD dwFlags = 0;
	DWORD dwBytes = 0;

	pIoContext->m_OpType = enOperation_RECV;

	int nBytesRecv = WSARecv(pSocketContext->m_Socket, &pIoContext->m_wsaBuf, 1, &dwBytes, &dwFlags, &pIoContext->m_Overlapped, NULL);

	// 如果返回值错误，并且错误的代码并非是Pending的话，那就说明这个重叠请求失败了
	if ((SOCKET_ERROR == nBytesRecv) && (WSA_IO_PENDING != WSAGetLastError()))
	{
		std::cout << "投递WSARecv失败！！," << GetLastError() << std::endl;
		return false;
	}

	return true;
}

