#include <vector>
#include <iostream>
#include <string>
#include "IOCPManager.h"

#pragma comment(lib,"ws2_32.lib")

using namespace std;

int main()
{
	CIOCPManager iocpManager;

	iocpManager.Start();

	iocpManager.Connect("172.31.0.112", 12345);

	string input;
	while (cin >> input)
	{
		if (input == "stop")
		{
			iocpManager.Stop();
			break;
		}
	}

	return 0;
}