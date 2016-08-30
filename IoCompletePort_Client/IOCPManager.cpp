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
	//��ʼ��winsock
	WSADATA wsaData;
	int nResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (nResult != NO_ERROR)
	{
		std::cout << "��ʼ���׽���ʧ�ܣ�" << std::endl;
		return;
	}

	m_hCompletePort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
	if (m_hCompletePort == NULL)
	{
		std::cout << "��ɶ˿ڴ���ʧ��" << std::endl;
		return;
	}

	// ��ȡCPU��Ŀ
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	int rsThreadNum = si.dwNumberOfProcessors * 2 + 2;

	// ���������߳�
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
			sprintf_s(szDebug, 255, "�ȴ���ɶ˿ڳ���%d", GetLastError());
			std::cout << szDebug << std::endl;
			
			// �˳������߳�
			if (GetLastError() == ERROR_INVALID_HANDLE)
			{
				return;
			}
		}
		else
		{
			// ��ȡ����Ĳ���
			CIOContext* pIoContext = CONTAINING_RECORD(pOverlapped, CIOContext, m_Overlapped);
			switch (pIoContext->m_OpType)
			{
			case enOperation_CONNECT:	// �������֪ͨ
			{
				//// ���͵�һ������
				//instance->SendData(pSocketItem, "Hello", sizeof("Hello"));

				// ���ӳɹ�����
				pSocketItem->m_pConnection->OnConnect();

				//Ͷ�ݵ�һ��recv����
				CIOContext* pNewIOContext = pSocketItem->GetFreeIoContext();
				instance->PostRecv(pSocketItem, pIoContext);
				break;
			}
			case enOperation_RECV:	// ���ݽ������֪ͨ
			{
				// �ж��׽����Ƿ�ر�
				if (0 == dwBytesTransfered)
				{
					pSocketItem->m_pConnection->OnClose();
					continue;
				}
				//��ӡ�յ�������
				//std::cout << "�յ���Ϣ:" << pIoContext->m_wsaBuf.buf << std::endl;
				DWORD dwFlags = 0;

				// ���ֵ���Ϣ����
				pSocketItem->m_pConnection->OnRecv(pIoContext->m_wsaBuf.buf, dwBytesTransfered);

				//pSocketItem->m_RecvBuffer.WriteBuf(pIoContext->m_wsaBuf.buf, pIoContext->m_wsaBuf.len);

				//Ͷ����һ��Recv����
				CIOContext* pNewIOContext = pSocketItem->GetFreeIoContext();
				instance->PostRecv(pSocketItem, pIoContext);

				//pSocketItem->Close();
				break;
			}
			case enOperation_SEND:	// ���ݷ������֪ͨ
			{
				// �ж��׽����Ƿ�ر�
				if (0 == dwBytesTransfered)
				{
					pSocketItem->m_pConnection->OnClose();
					continue;
				}

				// ����������ɴ���
				pSocketItem->m_pConnection->OnSend();

				pSocketItem->FreeIOContext(pIoContext);
			}
			break;
			default:
				//��Ӧ��ִ�е�����
				std::cout << "RSThread�е� pIoContext->m_OpType �����쳣." << std::endl;
				break;
			}
		}
	}
}

bool CIOCPManager::SendData(CSocketItem* pSocketItem, char* sendBuf, int bufLength)
{
	//��������io������
	CIOContext *pNewIoContext = pSocketItem->GetFreeIoContext();
	pNewIoContext->m_OpType = enOperation_SEND;
	CopyMemory(pNewIoContext->m_wsaBuf.buf, sendBuf, bufLength);
	pNewIoContext->m_wsaBuf.len = bufLength;

	DWORD dwTrancferred;

	int iRetCode = WSASend(pSocketItem->m_Socket, &pNewIoContext->m_wsaBuf, 1, &dwTrancferred, 0, &pNewIoContext->m_Overlapped, NULL);
	if ((iRetCode == SOCKET_ERROR) && (WSAGetLastError() != WSA_IO_PENDING))
	{
		std::cout << "�������ݸ�����˳���:" << WSAGetLastError() << std::endl;
		return false;
	}
	return true;
}

bool CIOCPManager::Connect(CConnection* pNewConn, std::string ip, int port)
{
	LPFN_CONNECTEX lpfnConnectEx = NULL;
	DWORD dwBytes = 0;
	GUID GuidConnectEx = WSAID_CONNECTEX;

	// ���������͵Ŀͻ����׽���
	SOCKET hClientSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (hClientSocket == INVALID_SOCKET)
	{
		std::cout << "�����׽��ִ���ʧ��:" << WSAGetLastError() << std::endl;
		return false;
	}

	// ��Ҫ���׽���
	SOCKADDR_IN localAddr;
	ZeroMemory(&localAddr, sizeof(SOCKADDR_IN));
	localAddr.sin_family = AF_INET;
	if (SOCKET_ERROR == ::bind(hClientSocket, (sockaddr*)&localAddr, sizeof(SOCKADDR_IN)))
	{
		std::cout << "�󶨱����׽���ʧ��:" << WSAGetLastError() << std::endl;
		return false;
	}

	// ��ȡConnectEx�ĺ���ָ��
	if (SOCKET_ERROR == WSAIoctl(hClientSocket, SIO_GET_EXTENSION_FUNCTION_POINTER, &GuidConnectEx, sizeof(GuidConnectEx),
		&lpfnConnectEx, sizeof(lpfnConnectEx), &dwBytes, 0, 0))
	{
		std::cout << "��ȡConnectEx����ָ�����:" << WSAGetLastError() << std::endl;
		return false;
	}

	// �����׽�����
	pNewConn->m_SocketItem.m_Socket = hClientSocket;

	// ����IO������
	CIOContext* pIoContext = pNewConn->m_SocketItem.GetFreeIoContext();
	pIoContext->m_OpType = enOperation_CONNECT;

	//����ɶ˿�
	HANDLE hTemp = CreateIoCompletionPort((HANDLE)pNewConn->m_SocketItem.m_Socket, m_hCompletePort, (DWORD)&pNewConn->m_SocketItem, 0);
	if (hTemp == NULL)
	{
		pNewConn->m_SocketItem.Close();
		SAFE_DEL(pNewConn);
		std::cout << "����ɶ˿�ʧ��:" << WSAGetLastError() << std::endl;
		return false;
	}

	// ��������Ŀ��
	SOCKADDR_IN addrPeer;
	ZeroMemory(&addrPeer, sizeof(SOCKADDR_IN));
	addrPeer.sin_family = AF_INET;
	inet_pton(AF_INET, ip.c_str(), &addrPeer.sin_addr.s_addr);
	addrPeer.sin_port = htons(port);

	// ���ӵ�Զ��
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

			std::cout << "ConnectExʧ��:" << nErrorCode << std::endl;
			return false;
		}
	}

	// ��������
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
	//��ʼ������
	DWORD dwFlags = 0;
	DWORD dwBytes = 0;

	pIoContext->m_OpType = enOperation_RECV;

	int nBytesRecv = WSARecv(pSocketContext->m_Socket, &pIoContext->m_wsaBuf, 1, &dwBytes, &dwFlags, &pIoContext->m_Overlapped, NULL);

	// �������ֵ���󣬲��Ҵ���Ĵ��벢����Pending�Ļ����Ǿ�˵������ص�����ʧ����
	if ((SOCKET_ERROR == nBytesRecv) && (WSA_IO_PENDING != WSAGetLastError()))
	{
		std::cout << "Ͷ��WSARecvʧ�ܣ���," << GetLastError() << std::endl;
		return false;
	}

	return true;
}

