#pragma once

#ifndef __CONNECTION_H__
#define __CONNECTION_H__

#include "SocketItem.h"

class ISocketEventHandler;

class CConnection
{
public:
	CConnection();
	virtual ~CConnection();

public:
	virtual void OnConnect();
	virtual void OnSend();
	virtual void OnRecv(char* data, int len);
	virtual void OnClose();

	void SetSocketEventHandler(ISocketEventHandler* pHandler);
public:
	CSocketItem m_SocketItem;

private:
	ISocketEventHandler* m_ISocketHandler;
};

#endif