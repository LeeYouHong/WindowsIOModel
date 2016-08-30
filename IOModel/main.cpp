#include <WinSock.h>
#include <tchar.h>

#define PORT 5150
#define MSGSIZE 1024
#define WM_SOCKET (WM_USER+0)

#pragma comment(lib, "ws2_32.lib");

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{
	WNDCLASS wndClass;  
	TCHAR *ProviderClass = _T("AsyncSelect");  
	HWND hWnd; 
	MSG msg;

	wndClass.style = CS_HREDRAW | CS_VREDRAW;  
	wndClass.lpfnWndProc = (WNDPROC)WndProc;  
	wndClass.cbClsExtra = 0;  
	wndClass.cbWndExtra = 0;  
	wndClass.hInstance = NULL;  
	wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);  
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);  
	wndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);  
	wndClass.lpszMenuName = NULL;  
	wndClass.lpszClassName = ProviderClass;  

	if ( !RegisterClass(&wndClass) )  
	{  
		MessageBox(NULL, _T("This is program requires Windows NT!"), ProviderClass, MB_ICONERROR);
		return NULL;  
	}  

	hWnd = CreateWindow( 
		ProviderClass,  
		TEXT("AsyncSelect Model"),  
		WS_OVERLAPPEDWINDOW,  
		CW_USEDEFAULT,  
		CW_USEDEFAULT,  
		CW_USEDEFAULT,  
		CW_USEDEFAULT,  
		NULL,  
		NULL,  
		NULL,  
		NULL);

	ShowWindow(hWnd, iCmdShow);
	UpdateWindow(hWnd);

	while( GetMessage(&msg, NULL, 0, 0) )
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.wParam;
}

LRESULT CALLBACK WndProc( HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	WSADATA wsd;
	static SOCKET sListen;
	SOCKET sClient;
	SOCKADDR_IN local, client;
	int ret, iAddrSize = sizeof(client);
	char szMessage[MSGSIZE];

	switch ( message )
	{
	case WM_CREATE:
		WSAStartup(0x0202, &wsd);
		
		sListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		local.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
		local.sin_family = AF_INET;
		local.sin_port = htons(PORT);

		bind(sListen, (struct sockaddr*)&local, sizeof(local));

		listen(sListen, 3);

		WSAAsyncSelect(sListen, hwnd, WM_SOCKET, FD_ACCEPT);
		return 0;

	case WM_DESTROY:
		closesocket(sListen);
		WSACleanup();
		PostQuitMessage(0);
		return 0;

	case WM_SOCKET:
		if ( WSAGETSELECTERROR(lParam) )
		{
			closesocket(wParam);
			break;
		}

		switch( WSAGETSELECTEVENT(lParam) )
		{
		case FD_ACCEPT:
			sClient = accept(wParam, (struct sockaddr*)&client, &iAddrSize);
			
			WSAAsyncSelect(sClient, hwnd, WM_SOCKET, FD_READ | FD_CLOSE);
			break;

		case FD_READ:
			ret = recv(wParam, szMessage, MSGSIZE, 0);

			if ( ret == 0 || ret == SOCKET_ERROR && WSAGetLastError() == WSAECONNRESET )
			{
				closesocket(wParam);
			}
			else
			{
				szMessage[ret] = '\0';
				send(wParam, szMessage, strlen(szMessage), 0);
			}
			break;

		case FD_CLOSE:
			closesocket(wParam);
			break;
		}
		return 0;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}


