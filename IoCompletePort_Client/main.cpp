#include <vector>
#include <iostream>
#include <string>
#include "IOCPManager.h"
#include "MyConnection.h"
#include "Player.h"

#pragma comment(lib,"ws2_32.lib")

using namespace std;

int main()
{
	CIOCPManager iocpManager;

	iocpManager.Start();

	string ip = "172.31.0.112";
	int port = 9999;

	// 启动消息处理引擎
	s_MessageManager::Instance().Start();

	// 创建玩家
	for (int i = 0; i < 1; i++)
	{
		s_PlayerManager::Instance().AddPlayer(new CPlayer(i, i));
		s_MyConnManager::Instance().AddConnection(new CMyConnection(i));
	}

	// 建立连接
	for (auto ply : s_PlayerManager::Instance().m_PlayerList)
	{
		CMyConnection* pConn = s_MyConnManager::Instance().FindConn(ply->m_ConnectionID);
		if (nullptr == pConn)
		{
			cout << "Player no cid, pid=" << ply->m_PID << "cid=" << ply->m_ConnectionID << endl;
			continue;
		}

		bool ret = iocpManager.Connect(pConn, ip, port);
		if (!ret)
		{
			cout << "Create connection to [" << ip << ":" << port << "] failed." << endl;
			continue;
		}
	}

	string input;
	while (cin >> input)
	{
		if (input == "stop")
		{
			iocpManager.Stop();
			s_MessageManager::Instance().Stop();
			break;
		}
	}

	return 0;
}