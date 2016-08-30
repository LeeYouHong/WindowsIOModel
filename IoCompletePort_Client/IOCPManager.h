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
	// 初始化
	void Start();

	// 工作线程处理
	static void Run(CIOCPManager* instance);

	// 发送数据
	bool SendData(CSocketItem* pSocketItem, char* sendBuf, int bufLength);

	// 连接远端
	bool Connect(CConnection* pConn, std::string ip, int port);

	// 停止
	void Stop();
private:
	// 投递一个Recv请求
	bool PostRecv(CSocketItem *pSocketContext, CIOContext *pIoContext);

private:
	HANDLE m_hCompletePort;
	std::atomic<bool> m_bRun;
	//std::vector<CConnection*> m_ConnectionList;
};

#endif

