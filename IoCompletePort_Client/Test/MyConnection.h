#pragma once

#ifndef __MYCONNECTION_H__
#define __MYCONNECTION_H__

#include <vector>
#include "Connection.h"
#include "Singleton.h"

class CMyConnection : public CConnection
{
public:
	CMyConnection(int cid);
	virtual ~CMyConnection();

public:
	virtual void OnConnect() override;
	virtual void OnSend() override;
	virtual void OnRecv(char* data, int len) override;
	virtual void OnClose() override;

public:
	int m_Cid;
};

class CMyConnManager
{
public:
	void AddConnection(CMyConnection* pConn);

	CMyConnection* FindConn(int cid);

	std::vector<CMyConnection*> m_ConnList;
};

using s_MyConnManager = JuneNet::Singleton < CMyConnManager>;

#endif