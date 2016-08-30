#pragma once

#ifndef __IOCPMANAGER_H__
#define __IOCPMANAGER_H__

#include <vector>
#include <atomic>
#include "define.h"

class CSocketItem;
class CIOContext;
class CConnection;

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
	bool Connect(CConnection* pConn, std::string ip, int port);

	// ֹͣ
	void Stop();
private:
	// Ͷ��һ��Recv����
	bool PostRecv(CSocketItem *pSocketContext, CIOContext *pIoContext);

private:
	HANDLE m_hCompletePort;
	std::atomic<bool> m_bRun;
	//std::vector<CConnection*> m_ConnectionList;
};

#endif

