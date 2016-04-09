#define WIN32_LEAN_AND_MEAN

#include "basics.h"

long CALLBACK WndProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam){

	switch(uMessage){
	case WM_CREATE:
		{
			return 0;
		}
	case WM_PAINT:
		{
			ValidateRect(hWnd, NULL);//basically saying - yeah we took care of any paint msg without any overhead
			return 0;
		}
	case WM_DESTROY:
		{
			PostQuitMessage(0);
			return 0;
		}
	default:
		{
			return DefWindowProc(hWnd, uMessage, wParam, lParam);
		}
	}
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pstrCmdLine, int iCmdShow){
	HWND hWnd;
	MSG msg;
	WNDCLASSEX wc;

	static char strAppName[] = "First Windows App, Zen Style";

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.lpfnWndProc = WndProc;
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
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		512,512,
		NULL,
		NULL,
		hInstance,
		NULL);

	g_hWndMain = hWnd;//set our global window handle

	ShowWindow(hWnd, iCmdShow);
	UpdateWindow(hWnd);
	
	if(FAILED(GameInit())){;//initialize Game
		SetError("Initialization Failed");
		GameShutdown();
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
			GameLoop();
		}
	}
	GameShutdown();// clean up the game
	return msg.wParam;
}

int GameInit(){
	HRESULT r = 0;//return values

	g_pD3D = Direct3DCreate8(D3D_SDK_VERSION);//COM object
	if( g_pD3D == NULL){
		SetError("Could not create IDirect3D8 object");
		return E_FAIL;
	}

	r = InitDirect3DDevice(g_hWndMain, 640, 480, FALSE, D3DFMT_X8R8G8B8, g_pD3D, &g_pDevice);
	if(FAILED(r)){//FAILED is a macro that returns false if return value is a failure - safer than using value itself
		SetError("Initialization of the device failed");
		return E_FAIL;
	}

	return S_OK;
}
int GameLoop(){

	Render();

	if (GetAsyncKeyState(VK_ESCAPE))
		PostQuitMessage(0);

	return S_OK;
}

int GameShutdown(){
	//release resources. First display adapter because COM object created it, then COM object
	if(g_pDevice)
		g_pDevice->Release();

	if(g_pD3D)
		g_pD3D->Release();

	return S_OK;
}

int Render(){
	HRESULT r;
//	D3DLOCKED_RECT LockedRect;//locked area of display memory(buffer really) we are drawing to
	LPDIRECT3DSURFACE8 pBackSurf = 0;
	if(!g_pDevice){
		SetError("Cannot render because there is no device");
		return E_FAIL;
	}

	//clear the display arera with colour black, ignore stencil buffer
	g_pDevice->Clear(0,0,D3DCLEAR_TARGET, D3DCOLOR_XRGB(0,0,25), 1.0f, 0);

	//get pointer to backbuffer
	r=g_pDevice->GetBackBuffer(0,D3DBACKBUFFER_TYPE_MONO, &pBackSurf);
	if(FAILED(r)){
		SetError("Couldn't get backbuffer");
	}
/*
	//get a lock on the surface
	r=pBackSurf->LockRect(&LockedRect, NULL, 0);
	if(FAILED(r)){
		SetError("Could not lock the back buffer");
	}

	DWORD* pData = (DWORD*)(LockedRect.pBits);
	//DRAW CODE GOES HERE - use pData
	Draw(LockedRect.Pitch, pData);

 */
	LPDIRECT3DSURFACE8 pSurface = 0;

	r=LoadBitmapToSurface("baboon.bmp",
		&pSurface, g_pDevice);
	if(FAILED(r)){
		SetError("could not load bitmap surface");
	}

	D3DSURFACE_DESC d3dsd;
	pSurface->GetDesc(&d3dsd);//get info about surface

	POINT DestPoint = {0, 0};
	RECT rect = {0,0, d3dsd.Width, d3dsd.Height};//source dimensions

//	SetError("size x=%d size y=%d format=%d",d3dsd.Width, d3dsd.Height, d3dsd.Format);

//	r=g_pDevice->CopyRects(pSurface, &rect, 1, pBackSurf, &DestPoint);//copy surface to buffer (like a bitblt)

	r=D3DXLoadSurfaceFromSurface(pBackSurf, NULL, NULL, pSurface, NULL, &rect, D3DX_FILTER_TRIANGLE,0);
//	r=D3DXLoadSurfaceFromSurface(pBackSurf, NULL, NULL, pSurface, NULL, &rect, D3DX_FILTER_POINT, 0);
	if(ERROR(r))
		SetError("did not copy surface");

//	SimpleBitmapDraw("baboon.bmp", pBackSurf, 10,10);
	pSurface->Release();
	pSurface = 0;

//	pBackSurf->UnlockRect();
//	pData = 0;

//	pBackSurf->Release();//release lock
//	pBackSurf = 0;

	g_pDevice->Present(NULL, NULL, NULL, NULL);//swap over buffer to primary surface
	return S_OK;
}

void Draw(int Pitch, DWORD* pData){
	DWORD Offset = 100*Pitch/4 + 200;
	pData[Offset] = D3DCOLOR_XRGB(255,0,0);

	//SimpleBitmapDraw();
}
void SimpleBitmapDraw(char* PathName, LPDIRECT3DSURFACE8 pBackSurf, int dpx, int dpy){
	LPDIRECT3DSURFACE8 pSurface = 0;

	LoadBitmapToSurface(PathName, &pSurface, g_pDevice);

	D3DSURFACE_DESC d3dsd;
	pSurface->GetDesc(&d3dsd);//get info about surface

	POINT DestPoint = {dpx, dpy};
	RECT rect = {0,0, d3dsd.Width, d3dsd.Height};//source dimensions

	g_pDevice->CopyRects(pSurface, &rect, 1, pBackSurf, &DestPoint);//copy surface to buffer (like a bitblt)

//	pSurface->Release();
//	pSurface = 0;

//	pBackSurf->Release();
//	pBackSurf = 0;
//
//	g_pDevice->Present(NULL, NULL, NULL, NULL);//put it on the primary surface
}

int LoadBitmapToSurface(char* PathName, LPDIRECT3DSURFACE8* ppSurface, LPDIRECT3DDEVICE8 pDevice){
	HRESULT r;
	HBITMAP hBitmap;
	BITMAP Bitmap;

	hBitmap = (HBITMAP)LoadImage(NULL, PathName, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
	if(hBitmap == NULL){
		SetError("Unable to load bitmap");
		return E_FAIL;
	}

	GetObject(hBitmap, sizeof(BITMAP), &Bitmap);
	DeleteObject(hBitmap);//we only needed it for the header info to create a D3D surface

	//create surface for bitmap
	r=pDevice->CreateImageSurface(Bitmap.bmWidth, Bitmap.bmHeight, D3DFMT_X8R8G8B8, ppSurface);
	if(FAILED(r)){
		SetError("Unable to create surface for bitmap load");
		return E_FAIL;
	}

	//load bitmap onto surface
	r = D3DXLoadSurfaceFromFile(*ppSurface, NULL, NULL, PathName, NULL, D3DX_FILTER_NONE, 0, NULL);
	if(FAILED(r)){
		SetError("Unable to laod file to surface");
		return E_FAIL;
	}

	return S_OK;
}
