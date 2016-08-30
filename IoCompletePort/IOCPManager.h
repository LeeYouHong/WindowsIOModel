#pragma once

#ifndef __IOCPMANAGER_H__
#define __IOCPMANAGER_H__

#include <vector>
#include <atomic>
#include "define.h"

class CSocketItem;
class CIOContext;

class CIOCPManager
{
public:
	CIOCPManager();
	~CIOCPManager();

public:
	// ��ʼ��
	void Start();

	// �����̴߳���
	static void Run(CIOCPManager* instance);

	// ��������
	bool SendData(CSocketItem* pSocketItem, char* sendBuf, int bufLength);

	// ����Զ��
	bool Connect(std::string ip, int port);

	// ֹͣ
	void Stop();
private:
	// Ͷ��һ��Recv����
	bool PostRecv(CSocketItem *pSocketContext, CIOContext *pIoContext);

private:
	HANDLE m_hCompletePort;
	std::atomic<bool> m_bRun;
	std::vector<CSocketItem*> m_SocketItemList;
};

#endif

