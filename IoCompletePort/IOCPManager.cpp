#include "IOCPManager.h"

#include <iostream>
#include <thread>
#include "SocketItem.h"

CIOCPManager::CIOCPManager()
	:m_bRun(false)
	, m_hCompletePort(nullptr)
{

}

CIOCPManager::~CIOCPManager()
{
	m_bRun = false;

	for (auto item : m_SocketItemList)
	{
		SAFE_DEL(item);
	}
	m_SocketItemList.clear();
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
		}
		else
		{
			// ��ȡ����Ĳ���
			CIOContext* pIoContext = CONTAINING_RECORD(pOverlapped, CIOContext, m_Overlapped);
			switch (pIoContext->m_OpType)
			{
			case enOperation_CONNECT:	// �������֪ͨ
			{
				std::cout << "���ӳɹ�" << std::endl;
				// ���͵�һ������
				instance->SendData(pSocketItem, "Hello", sizeof("Hello"));

				//Ͷ�ݵ�һ��recv����
				instance->PostRecv(pSocketItem, pIoContext);
				break;
			}
			case enOperation_RECV:	// ���ݽ������֪ͨ
			{
				//��ӡ�յ�������
				std::cout << "�յ���Ϣ:" << pIoContext->m_wsaBuf.buf << std::endl;

				pSocketItem->m_RecvBuffer.WriteBuf(pIoContext->m_wsaBuf.buf, pIoContext->m_wsaBuf.len);

				// ������Ϣ,TODO....

				//Ͷ����һ��Recv����
				instance->PostRecv(pSocketItem, pIoContext);
				break;
			}
			case enOperation_SEND:	// ���ݷ������֪ͨ
			{
				std::cout << "�������" << std::endl;

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

	DWORD dwTrancferred;

	int iRetCode = WSASend(pSocketItem->m_Socket, &pNewIoContext->m_wsaBuf, 1, &dwTrancferred, 0, &pNewIoContext->m_Overlapped, NULL);
	if ((iRetCode == SOCKET_ERROR) && (WSAGetLastError() != WSA_IO_PENDING))
	{
		std::cout << "�������ݸ�����˳���:" << WSAGetLastError() << std::endl;
		return false;
	}
	return true;
}

bool CIOCPManager::Connect(std::string ip, int port)
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
	CSocketItem *pNewSocketItem = new CSocketItem();
	pNewSocketItem->m_Socket = hClientSocket;

	// ����IO������
	CIOContext* pIoContext = pNewSocketItem->GetFreeIoContext();
	pIoContext->m_OpType = enOperation_CONNECT;

	//����ɶ˿�
	HANDLE hTemp = CreateIoCompletionPort((HANDLE)pNewSocketItem->m_Socket, m_hCompletePort, (DWORD)pNewSocketItem, 0);
	if (hTemp == NULL)
	{
		delete pNewSocketItem;
		std::cout << "����ɶ˿�ʧ��:" << WSAGetLastError() << std::endl;
		return false;
	}

	//��SocketContext�ӽ�ȫ�ֵ�SocketContext������
	m_SocketItemList.push_back(pNewSocketItem);

	// ��������Ŀ��
	SOCKADDR_IN addrPeer;
	ZeroMemory(&addrPeer, sizeof(SOCKADDR_IN));
	addrPeer.sin_family = AF_INET;
	inet_pton(AF_INET, ip.c_str(), &addrPeer.sin_addr.s_addr);
	addrPeer.sin_port = htons(12345);

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
			std::cout << "ConnectExʧ��:" << nErrorCode << std::endl;
			return false;
		}
	}

	return true;
}

void CIOCPManager::Stop()
{
	m_bRun = false;
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
		std::cout << "Ͷ�ݵ�һ��WSARecvʧ�ܣ���" << std::endl;
		return false;
	}

	return true;
}

