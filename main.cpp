/* Brian Livesey - COMP4995 Assignment 2 */

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#include "basics.h"
#endif
#ifndef ASSN1
#define ASSN1
#include "Assn1.h"
#endif


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pstrCmdLine, int iCmdShow){
	HWND hWnd;
	MSG msg;
	WNDCLASSEX wc;
	Assn1 ass1;

	static char strAppName[] = "Brian Livesey AKA A00896837 - Assignment 2";

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.lpfnWndProc = Assn1::WndProc;
	wc.hInstance = hInstance;
	wc.hbrBackground = (HBRUSH)GetStockObject(DKGRAY_BRUSH);
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hIconSm = LoadIcon(NULL, IDI_HAND);
	wc.hCursor = LoadCursor(NULL, IDC_CROSS);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = strAppName;

	RegisterClassEx(&wc);

	hWnd = CreateWindowEx(NULL,
		strAppName,
		strAppName,
		// WS_OVERLAPPEDWINDOW,
		WS_POPUP | WS_SYSMENU | WS_VISIBLE,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		512,512,
		NULL,
		NULL,
		hInstance,
		NULL);

	Assn1::g_hWndMain = hWnd;//set our global window handle
	Assn1::instance = ass1;

	ShowWindow(hWnd, iCmdShow);
	UpdateWindow(hWnd);
	
	if(FAILED(ass1.GameInit())){;//initialize Game
	Assn1::SetError("Initialization Failed");
	ass1.GameShutdown();
		return E_FAIL;
	}

	if (FAILED(ass1.InitGeometry())) {
		;//initialize Game
		Assn1::SetError("Geometry innit Failed");
		ass1.GameShutdown();
		return E_FAIL;
	}

	while(TRUE){
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)){
			if(msg.message == WM_QUIT)
				break;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else{
			ass1.GameLoop();
		}
	}
	ass1.GameShutdown();// clean up the game
	return msg.wParam;
}

