/* Brian Livesey - COMP4995 research project */

#ifndef ASSN1
#define ASSN1
#include "Assn1.h"
#endif
#include <iostream>
#include <sstream>
#include <string>
// #define _USE_MATH_DEFINES



#define BABOON "baboon.bmp"	// We don't speak of him anymore
#define LENA "lena.bmp"		// Nice lady but very 1972
#define SEA	 "seascape.bmp" 

// WndProc
long CALLBACK Assn1::WndProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam) {

	switch (uMessage) {
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

	case WM_MOUSEMOVE:
	{
		// Drag mouse
		if (wParam & MK_LBUTTON || wParam & MK_RBUTTON) {
			stopx = LOWORD(lParam);
			stopy = HIWORD(lParam);
		}
		return 0;
	}

	// Picking
	case WM_LBUTTONDOWN:
	{
		oldx = oldx + startx - stopx;
		oldy = oldy + starty - stopy;
		startx = LOWORD(lParam);
		starty = HIWORD(lParam);
		stopx = startx;
		stopy = starty;

		// compute the ray in view space given the clicked screen point
		bri::Ray ray = CalcPickingRay(LOWORD(lParam), HIWORD(lParam));
		// transform the ray to world space
		D3DXMATRIX view;
		g_pDevice->GetTransform(D3DTS_VIEW, &view);
		D3DXMATRIX viewInverse;
		D3DXMatrixInverse(&viewInverse, 0, &view);
		TransformRay(&ray, &viewInverse);

		// test for a hit against all loaded meshes
		for (int i = 0; i <= objIndex; ++i) {
			// D3DXVECTOR3 originPT(0, 0, 0);
			// D3DXVec3TransformCoord(&objectsList[i].boundSphere._center, &originPT, &objectsList[i].matTrans);
			if (RaySphereIntTest(&ray, &objectsList[i].boundSphere) && i != meshActive) {
				// meshActive = i;
				oldx = 0;
				oldy = 0;
				// Refresh mouse rotation matrix
				// D3DXMatrixRotationYawPitchRoll(&matWorld, (startx - stopx) / mouseFactor, (starty - stopy) / mouseFactor, 0);
				changeActiveMesh(i);
				break;
				// ::MessageBox(0, "Hit!", "HIT", 0);
			}
		}

		return 0;
	}

	// Click to start a line
	case WM_RBUTTONDOWN:
	{
		oldx = oldx + startx - stopx;
		oldy = oldy + starty - stopy;
		startx = LOWORD(lParam);
		starty = HIWORD(lParam);
		stopx = startx;
		stopy = starty;
		return 0;
	}

	// Keypress events
	case WM_KEYDOWN:
	{
		switch (wParam) {
			
		case 0x31: // '1' key pressed - select mesh 1
			// mesh1matrix *= matWorld;
			// mesh1active = true;
			meshActive = 0;
			// ::MessageBox(0, "Mesh 1", "Mesh 1 active!", 0);
			break;
		case 0x32: // '2' key pressed - select mesh 2
			// mesh1active = false;
			meshActive = 1;
			break;
		case 0x33: // '3' key pressed - toggle directional light
			light1on = !light1on;
			instance.g_pDevice->LightEnable(0, light1on);
			break;
		case 0x34: // '4' key pressed - toggle spot light
			light2on = !light2on;
			instance.g_pDevice->LightEnable(1, light2on);
			break;
		case 0x35: // '5' key pressed - toggle point light (moving)
			light3on = !light3on;
			instance.g_pDevice->LightEnable(2, light3on);
			break;
		case 0x36: // '6' key pressed - toggle ambient lighting
			light4on = !light4on;
			instance.g_pDevice->SetRenderState(D3DRS_AMBIENT, light4on ? 0x333333 : 0x000000);
			break;
		case 0x37: // '7' key - skipObjs
			skipObjs = (skipObjs + 1) % 11;
			break;
		case 0x38: // '8' key - reset camera
			vEyePt = D3DXVECTOR3(0.0f, 40.0f, -300.0f);
			// speed = 1;
			break;
		case 0x20: // SPACEBAR
			mirrorRotate = !mirrorRotate;
			break;
		case VK_BACK: // BACKSPACE
			snowOn = !snowOn;
			break;
		case VK_LEFT: // Left arrow - rotate view
			camAngle -= D3DX_PI / 180;
			break;
		case VK_RIGHT: // Right arrow - rotate view
			camAngle += D3DX_PI / 180;
			break;
		case VK_NEXT: // PageDown - speed up Camera forward movement
			speed *= 1.5;
			break;
		case VK_PRIOR: // PageUp - slow down Camera
			speed /= 1.5;
			break;
		case VK_DOWN: // Down arrow - rotate view
			camXAngle -= D3DX_PI / 180;
			break;
		case VK_UP: // Up arrow - rotate view
			camXAngle += D3DX_PI / 180;
			break;
		case VK_TAB:	// Tab = cycle projection matrixes
			matProjIndex = (matProjIndex + 1) % 5;
			break;
		}

	

	}

	default:
	{
		return DefWindowProc(hWnd, uMessage, wParam, lParam);
	}
	}
}


// Initialize stuff
int Assn1::GameInit() {
	HRESULT r = 0;//return values

	g_pD3D = Direct3DCreate9(D3D_SDK_VERSION);//COM object
	if (g_pD3D == NULL) {
		SetError("Could not create IDirect3D9 object");
		return E_FAIL;
	}

	// Configure Direct3D object
	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.Windowed = FALSE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;
	d3dpp.BackBufferCount = 1;
	d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
	d3dpp.MultiSampleQuality = 0;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8; // D3DFMT_D16;
	d3dpp.Flags = 0;
	d3dpp.BackBufferWidth = 1920; //  1024; //  640;
	d3dpp.BackBufferHeight = 1080; //  768; // 480;

	// Create the D3DDevice
	if (FAILED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, g_hWndMain, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &g_pDevice)))
	{
		return E_FAIL;
	}

	// Turn on the zbuffer
	g_pDevice->SetRenderState(D3DRS_ZENABLE, TRUE);


	//Set up lights

	// Directional light
	D3DLIGHT9 Light;
	ZeroMemory(&Light, sizeof(D3DLIGHT9));
	Light.Type = D3DLIGHT_DIRECTIONAL;
	Light.Diffuse.r = 1.0f;
	Light.Diffuse.g = 1.0f;
	Light.Diffuse.b = 1.0f;
	Light.Range = 100.0f;
	D3DXVECTOR3 vecDir;
	vecDir = D3DXVECTOR3(1.0f, 1.0f, 1.0f);
	D3DXVec3Normalize((D3DXVECTOR3*)&Light.Direction, &vecDir);
	//attach light structure to a Direct3D Lighting index

	g_pDevice->SetLight(0, &Light);


	// Spotlight
	D3DLIGHT9 Light2;
	ZeroMemory(&Light2, sizeof(D3DLIGHT9));
	Light2.Type = D3DLIGHT_SPOT;
	Light2.Diffuse.r = 0.2f;
	Light2.Diffuse.g = 0.2f;
	Light2.Diffuse.b = 1.0f;
	Light2.Range = 100.0f;
	Light2.Position = D3DXVECTOR3(2, 20, 0);
	Light2.Falloff = 1.0;
	Light2.Attenuation0 = 0.0;
	Light2.Attenuation1 = 5.0;
	Light2.Attenuation2 = 0.0;
	Light2.Theta =  10 * 2 * D3DX_PI / 360;
	Light2.Phi = 15 * 2 * D3DX_PI / 360;

	D3DXVECTOR3 vecDir2;
	vecDir2 = D3DXVECTOR3(0.0f, -1.0f, 0.0f);
	D3DXVec3Normalize((D3DXVECTOR3*)&Light2.Direction, &vecDir2);
	//attach Light2 structure to a Direct3D Light2ing index

	g_pDevice->SetLight(1, &Light2);


	// Point light (moving)
	ZeroMemory(&Light3, sizeof(D3DLIGHT9));
	Light3.Type = D3DLIGHT_POINT;
	Light3.Position = D3DXVECTOR3(0, 0, 0);
	Light3.Diffuse.r = 0.2f;
	Light3.Diffuse.g = 0.2f;
	Light3.Diffuse.b = 1.0f;
	Light3.Attenuation0 = 0.0;
	Light3.Attenuation1 = 1.0;
	Light3.Attenuation2 = 0.7;
	Light3.Range = 20.0f;
	D3DXVECTOR3 vecDir3;
	g_pDevice->SetLight(2, &Light3);

	//enable lights
	g_pDevice->LightEnable(0, TRUE);

	// Turn on lighting
	g_pDevice->SetRenderState(D3DRS_LIGHTING, TRUE);


	// Turn on ambient lighting 
	g_pDevice->SetRenderState(D3DRS_AMBIENT, 0x333333);
	
	// get backbuffer pointer
	r = g_pDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackSurf);
	if (FAILED(r)) {
		SetError("Couldn't get backbuffer");
	}
	// inexpensive copy operation initial config
	LoadBitmapToSurface(SEA, &backStretch, g_pDevice);
	D3DXLoadSurfaceFromSurface(pBackSurf, NULL, NULL, backStretch, NULL, NULL, D3DX_FILTER_TRIANGLE, 0);

	// Initialize matrices
	// D3DXMatrixIdentity(&mesh1matrix);
	// D3DXMATRIX mesh2scaling;
	// D3DXMATRIX mesh2trans;
	// D3DXMatrixScaling(&mesh2scaling, 0.005, 0.005, 0.005);
	// D3DXMatrixTranslation(&mesh2trans, 2, 0, 0);
	// mesh2matrix = mesh2scaling * mesh2trans;
	// mesh1active = true;

	return S_OK;
}

int Assn1::GameLoop() {

	Render();

	if (GetAsyncKeyState(VK_ESCAPE))
		PostQuitMessage(0);

	return S_OK;
}

int Assn1::GameShutdown() {
	//release resources. First display adapter because COM object created it, then COM object
	if (g_pDevice)
		g_pDevice->Release();

	if (g_pD3D)
		g_pD3D->Release();

	return S_OK;
}


//-----------------------------------------------------------------------------
// Name: InitGeometry()
// Desc: Load the mesh and build the material and texture arrays
//-----------------------------------------------------------------------------
HRESULT Assn1::InitGeometry()
{
	// Set initial camera position
	vEyePt = D3DXVECTOR3(0.0f, 40.0f, -300.0f);

	// Load meshes
	loadMesh("chair.x");
	loadMesh("chair.x");
	loadMesh("car2.x");

	/*
	// Big grid of chairs for mirroring, surrounding center point
	for (int i = 0; i < 9; ++i) {
		for (int j = 0; j < 9; ++j) {
			if (!(i <= 5 && i >= 3 && j <= 5 && j >= 3))
				createObject(0.25, &meshes[0], -12 + 3 * i, 0, -12 + 3 * j);
		}
	}
	*/

	/*
	// Bigger grid of chairs for mirroring, surrounding center point
	int chairsPerSide = 4;
	int chairSpacing = 6;
	for (int i = 0; i < chairsPerSide; ++i) {
		for (int j = 0; j < chairsPerSide; ++j) {
			if (!(i <= chairsPerSide / 2 + 1
				&& i >= chairsPerSide / 2 - 1
				&& j <= chairsPerSide / 2 + 1
				&& j >= chairsPerSide / 2 - 1))
				createObject(0.5, &meshes[0], -(chairsPerSide - 1) * chairSpacing / 2.0 + chairSpacing * i, 0, -(chairsPerSide - 1) * chairSpacing / 2.0 + chairSpacing * j);
		}
	}
	*/

	// Land tiles
	int chairsPerSide = 100;
	int chairSpacing = 5;
	for (int i = 0; i < chairsPerSide; ++i) {
		for (int j = 0; j < chairsPerSide; ++j) {
				createObject((rand() % 50 ) / 10.0, &meshes[0], -(chairsPerSide - 1) * chairSpacing / 2.0 + chairSpacing * i, 0, -(chairsPerSide - 1) * chairSpacing / 2.0 + chairSpacing * j);
		}
	}


	// Load textures
	D3DXCreateTextureFromFile(g_pDevice, "mirror.png", &texMirror);
	D3DXCreateTextureFromFile(g_pDevice, "grass1lo.bmp", &texGrass1);
	D3DXCreateTextureFromFile(g_pDevice, "grass2lo.bmp", &texGrass2);

	// Load materials
	/*
	::ZeroMemory(&matMirror, sizeof(matMirror));
	matMirror.Diffuse = D3DXCOLOR(0.3f, 0.3f, 0.3f, 1.0f); // grey
	matMirror.Ambient = D3DXCOLOR(0.3f, 0.3f, 0.3f, 1.0f);
	matMirror.Specular = D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f);
	matMirror.Emissive = D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f); // no emission
	matMirror.Power = 5.0f;
	*/
	matMirror = bri::WHITE_MTRL;

	// Load vertex buffer
	g_pDevice->CreateVertexBuffer(
		54 * sizeof(bri::Vertex),
		0, // usage
		bri::Vertex::FVF,
		D3DPOOL_MANAGED,
		&VB,
		0);

	bri::Vertex* v = 0;
	VB->Lock(0, 0, (void**)&v, 0);

	// ** floor & wall from demo unused ** 
	// floor
	v[0] = bri::Vertex(-7.5f, 0.0f, -10.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f);
	v[1] = bri::Vertex(-7.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f);
	v[2] = bri::Vertex(7.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f);

	v[3] = bri::Vertex(-7.5f, 0.0f, -10.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f);
	v[4] = bri::Vertex(7.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f);
	v[5] = bri::Vertex(7.5f, 0.0f, -10.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f);

	// wall
	v[6] = bri::Vertex(-7.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
	v[7] = bri::Vertex(-7.5f, 5.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
	v[8] = bri::Vertex(-2.5f, 5.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);

	v[9] = bri::Vertex(-7.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
	v[10] = bri::Vertex(-2.5f, 5.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);
	v[11] = bri::Vertex(-2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f);

	// Note: We leave gap in middle of walls for mirror

	v[12] = bri::Vertex(2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
	v[13] = bri::Vertex(2.5f, 5.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
	v[14] = bri::Vertex(7.5f, 5.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);

	v[15] = bri::Vertex(2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
	v[16] = bri::Vertex(7.5f, 5.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);
	v[17] = bri::Vertex(7.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f);

	// Mirror shifted down -2.5 manually! :P
	// mirror (front face)
	v[18] = bri::Vertex(-2.5f, -2.5f, -2.5f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
	v[19] = bri::Vertex(-2.5f, 2.5f, -2.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
	v[20] = bri::Vertex(2.5f, 2.5f, -2.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);

	v[21] = bri::Vertex(-2.5f, -2.5f, -2.5f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
	v[22] = bri::Vertex(2.5f, 2.5f, -2.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);
	v[23] = bri::Vertex(2.5f, -2.5f, -2.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f);

	// mirror left face
	v[24] = bri::Vertex(-2.5f, -2.5f, 2.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	v[25] = bri::Vertex(-2.5f, 2.5f, 2.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	v[26] = bri::Vertex(-2.5f, 2.5f, -2.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f);

	v[27] = bri::Vertex(-2.5f, -2.5f, 2.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	v[28] = bri::Vertex(-2.5f, 2.5, -2.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
	v[29] = bri::Vertex(-2.5f, -2.5f, -2.5f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f);

	// mirror (rear face)
	v[30] = bri::Vertex(2.5f, -2.5f, 2.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f);
	v[31] = bri::Vertex(2.5f, 2.5f, 2.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
	v[32] = bri::Vertex(-2.5f, 2.5f, 2.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f);

	v[33] = bri::Vertex(2.5f, -2.5f, 2.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f);
	v[34] = bri::Vertex(-2.5f, 2.5f, 2.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f);
	v[35] = bri::Vertex(-2.5f, -2.5f, 2.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);

	// mirror left face
	v[36] = bri::Vertex(2.5f, -2.5f, -2.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
	v[37] = bri::Vertex(2.5f, 2.5f, -2.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
	v[38] = bri::Vertex(2.5f, 2.5f, 2.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);

	v[39] = bri::Vertex(2.5f, -2.5f, -2.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
	v[40] = bri::Vertex(2.5f, 2.5f, 2.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	v[41] = bri::Vertex(2.5f, -2.5f, 2.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);

	// mirror (top face)
	v[42] = bri::Vertex(-2.5f, 2.5f, -2.5f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
	v[43] = bri::Vertex(-2.5f, 2.5f, 2.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
	v[44] = bri::Vertex(2.5f, 2.5f, 2.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);

	v[45] = bri::Vertex(-2.5f, 2.5f, -2.5f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
	v[46] = bri::Vertex(2.5f, 2.5f, 2.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);
	v[47] = bri::Vertex(2.5f, 2.5f, -2.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f);

	// mirror (bottom face)
	v[48] = bri::Vertex(2.5f, -2.5f, -2.5f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
	v[49] = bri::Vertex(2.5f, -2.5f, 2.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
	v[50] = bri::Vertex(-2.5f, -2.5f, 2.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);

	v[51] = bri::Vertex(2.5f, -2.5f, -2.5f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
	v[52] = bri::Vertex(-2.5f, -2.5f, 2.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);
	v[53] = bri::Vertex(-2.5f, -2.5f, -2.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f);

	VB->Unlock();

	/*
	// Particle system
	// d3d::BoundingBox *bbox();
	d3d::BoundingBox boundingBox;
	boundingBox._min = D3DXVECTOR3(-10.0f, -10.0f, -10.0f);
	boundingBox._max = D3DXVECTOR3(10.0f, 10.0f, 10.0f);
	snowSystem = new psys::Snow(&boundingBox, 1000);
	snowSystem->init(g_pDevice, "snowflake.dds");
	*/


	return S_OK;
}


void Assn1::createObject(float scaleFactor, bri::MeshObj* meshType, float x, float y, float z) {
	if (meshType != &meshes[0])
	{
		D3DXMatrixScaling(&objectsList[objIndex].matScaling, scaleFactor, scaleFactor, scaleFactor);
		objectsList[objIndex].boundSphere._radius = 0.75;
		// Initialize bounding-sphere for mesh
		D3DXVECTOR3 originPT(0, 0, 0);
		D3DXVec3TransformCoord(&objectsList[objIndex].boundSphere._center, &originPT, &objectsList[objIndex].matTrans);
	}
	else 
	{
		D3DXMatrixScaling(&objectsList[objIndex].matScaling, 1, scaleFactor, 1);
		D3DXMATRIX pushUp;
		D3DXMatrixTranslation(&pushUp, 0, 2.5, 0);
		objectsList[objIndex].matScaling = pushUp * objectsList[objIndex].matScaling;
	}
	objectsList[objIndex].mesh = meshType;
	D3DXMatrixTranslation(&objectsList[objIndex].matTrans, x, y, z);
	objectsList[objIndex].matNet = objectsList[objIndex].matScaling * objectsList[objIndex].matTrans;
	
	D3DXMatrixIdentity(&objectsList[objIndex].matRot);

	++objIndex;
}

HRESULT Assn1::loadMesh(std::string filename) {
	LPD3DXBUFFER pD3DXMtrlBuffer;

	// Load the mesh from the specified file
	if (FAILED(D3DXLoadMeshFromX(filename.c_str(), D3DXMESH_SYSTEMMEM,
		g_pDevice, NULL,
		&pD3DXMtrlBuffer, NULL, &meshes[meshIndex].numMaterials,
		&meshes[meshIndex].mesh)))
	{
		// If model is not in current folder, try parent folder

		// Wow, dealing with strings in VC++ is bloody horrible.  -BL
		if (FAILED(D3DXLoadMeshFromX((std::string("..\\") + filename).c_str(), D3DXMESH_SYSTEMMEM,
			g_pDevice, NULL,
			&pD3DXMtrlBuffer, NULL, &meshes[meshIndex].numMaterials,
			&meshes[meshIndex].mesh)))
		{
			MessageBox(NULL, "Could not find mesh file", filename.c_str(), MB_OK);
			return E_FAIL;
		}
	}


	// We need to extract the material properties and texture names from the 
	// pD3DXMtrlBuffer
	D3DXMATERIAL* d3dxMaterials = (D3DXMATERIAL*)pD3DXMtrlBuffer->GetBufferPointer();
	meshes[meshIndex].meshMaterials = new D3DMATERIAL9[meshes[meshIndex].numMaterials];
	meshes[meshIndex].meshTextures = new LPDIRECT3DTEXTURE9[meshes[meshIndex].numMaterials];

	for (DWORD i = 0; i < meshes[meshIndex].numMaterials; i++)
	{
		// Copy the material
		meshes[meshIndex].meshMaterials[i] = d3dxMaterials[i].MatD3D;

		// Set the ambient color for the material (D3DX does not do this)
		meshes[meshIndex].meshMaterials[i].Ambient = meshes[meshIndex].meshMaterials[i].Diffuse;

		meshes[meshIndex].meshTextures[i] = NULL;
		if (d3dxMaterials[i].pTextureFilename != NULL &&
			lstrlen(d3dxMaterials[i].pTextureFilename) > 0)
		{
			// Create the texture
			if (FAILED(D3DXCreateTextureFromFile(g_pDevice,
				d3dxMaterials[i].pTextureFilename,
				&meshes[meshIndex].meshTextures[i])))
			{
				// If texture is not in current folder, try parent folder
				const TCHAR* strPrefix = TEXT("..\\");
				const int lenPrefix = lstrlen(strPrefix);
				TCHAR strTexture[MAX_PATH];
				lstrcpyn(strTexture, strPrefix, MAX_PATH);
				lstrcpyn(strTexture + lenPrefix, d3dxMaterials[i].pTextureFilename, MAX_PATH - lenPrefix);
				// If texture is not in current folder, try parent folder
				if (FAILED(D3DXCreateTextureFromFile(g_pDevice,
					strTexture,
					&meshes[meshIndex].meshTextures[i])))
				{
					MessageBox(NULL, "Could not find texture map", "Meshes.exe", MB_OK);
				}
			}
		}
	}
	// Done with the material buffer
	pD3DXMtrlBuffer->Release();

	++meshIndex;
	return S_OK;
}


// Old render method from assignment 1 (still used - it calls Render2() for mesh stuff after it begins the scene)
int Assn1::Render() {
	HRESULT r;
	// LPDIRECT3DSURFACE9 pBackSurf = 0;
	if (!g_pDevice) {
		SetError("Cannot render because there is no device");
		return E_FAIL;
	}

	//clear the display arera with colour black, DON'T ignore stencil buffer (b/c that'd be stupid  -BL)
	g_pDevice->Clear(0, 0, D3DCLEAR_TARGET| D3DCLEAR_STENCIL, D3DCOLOR_XRGB(0, 0, 25), 1.0f, 0);

	// Inexpensive copy
	r = g_pDevice->StretchRect(backStretch, NULL, pBackSurf, NULL, D3DTEXF_NONE);
	if (FAILED(r)) {
		SetError("StretchRect failed");
	}

	
	g_pDevice->BeginScene();

	// Draw all meshes & geometry
	Render2();

	// Draw framerate count
	ID3DXFont *g_font = NULL;

	r = D3DXCreateFont(g_pDevice,     //D3D Device
		22,               //Font height
		0,                //Font width
		FW_NORMAL,        //Font Weight
		1,                //MipLevels
		false,            //Italic
		DEFAULT_CHARSET,  //CharSet
		OUT_DEFAULT_PRECIS, //OutputPrecision
		ANTIALIASED_QUALITY, //Quality
		DEFAULT_PITCH | FF_DONTCARE,//PitchAndFamily
		"Arial",          //pFacename
		&g_font);         //ppFont
	if (FAILED(r))
		SetError("did not create font");

	const char *fps_string;
	RECT font_rect;

	//frames per second
	static int frameCount = 0;
	static int frameRate = 0;
	INT64 freq;
	INT64 newCount;

	QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
	QueryPerformanceCounter((LARGE_INTEGER*)&newCount);
	if (timeStamp == 0) {
		timeStamp = newCount;
	}
	if (newCount - timeStamp > freq) {
		timeStamp = newCount;
		frameRate = frameCount;
		frameCount = 0;
	}
	else {
		++frameCount;
	}

	char fpsfps[10];
	sprintf_s(fpsfps, "%d", frameRate);
	int g_width = 200;
	int g_height = 75;
	SetRect(&font_rect, 0, 0, g_width, g_height);
	int font_height = g_font->DrawText(NULL,        
		fpsfps,					//pString
		-1,						//Count
		&font_rect,				//pRect
		DT_LEFT | DT_NOCLIP,	//Format,
		0xFFFFFFFF);			//Color
	// End draw framerate

	g_pDevice->EndScene();

	if (g_font) {
		g_font->Release();
		g_font = NULL;
	}

	// End draw text

	g_pDevice->Present(NULL, NULL, NULL, NULL);//swap over buffer to primary surface
	return S_OK;
}


// Render2() - Now with 100% more meshy goodness (this is the new render code for assignment 2, called from Render()
VOID Assn1::Render2()
{
	// Clear the backbuffer and the zbuffer (minus bb  -BL)
	// g_pDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
	g_pDevice->Clear(0, NULL, D3DCLEAR_ZBUFFER,
		D3DCOLOR_XRGB(0, 0, 255), 1.0f, 0);

	// Move my fancy moving light
	D3DLIGHT9 tempLight;
	g_pDevice->GetLight(2, &tempLight);
	float moveyLight = 50 - 100 * (timeGetTime() % 1000) / 1000.0;
	tempLight.Position = D3DXVECTOR3(0, 10, moveyLight);
	g_pDevice->SetLight(2, &tempLight);

	// Setup the world, view, and projection matrices
	SetupMatrices();

	// Render meshes

	// Note that objIndex is the *number* of objects not the index (sigh)
	for (int currentObj = 0; currentObj < objIndex; currentObj += skipObjs + 1) {
		if (objectsList[currentObj].mesh != &meshes[0]) {
			// Derive per-mesh transformation from previous state and new rotations applied
			// D3DXMATRIX matMesh = objectsList[currentMesh].matNet;

			if (currentObj == meshActive) {
				// objectsList[currentMesh].matRot = matWorld;
				objectsList[currentObj].matNet = matWorld * objectsList[currentObj].matRot * objectsList[currentObj].matScaling * objectsList[currentObj].matTrans;
			}

			g_pDevice->SetTransform(D3DTS_WORLD, &objectsList[currentObj].matNet);

			for (DWORD i = 0; i < objectsList[currentObj].mesh->numMaterials; i++)
			{
				// Set the material and texture for this subset
				g_pDevice->SetMaterial(&objectsList[currentObj].mesh->meshMaterials[i]);
				g_pDevice->SetTexture(0, objectsList[currentObj].mesh->meshTextures[i]);

				// Draw the mesh subset
				objectsList[currentObj].mesh->mesh->DrawSubset(i);			// Ok maybe we'll say "swing and a miss" for my naming on these two fields..
			}
		}
		else {	// Land square
			g_pDevice->SetTransform(D3DTS_WORLD, &objectsList[currentObj].matNet);
			g_pDevice->SetStreamSource(0, VB, 0, sizeof(bri::Vertex));
			g_pDevice->SetFVF(bri::Vertex::FVF);

			g_pDevice->SetMaterial(&matMirror);

			g_pDevice->SetTexture(0, texGrass2);
			g_pDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 18, 8);

			g_pDevice->SetTexture(0, texGrass1);
			g_pDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 42, 2);
		}
	}

	if (snowOn) {
		// Draw the physical mirror
		D3DXMATRIX I;
		D3DXMatrixIdentity(&I);
		g_pDevice->SetTransform(D3DTS_WORLD, &I);

		g_pDevice->SetStreamSource(0, VB, 0, sizeof(bri::Vertex));
		g_pDevice->SetFVF(bri::Vertex::FVF);

		/*
		// draw the floor
		g_pDevice->SetMaterial(&FloorMtrl);
		g_pDevice->SetTexture(0, FloorTex);
		g_pDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 2);

		// draw the walls
		g_pDevice->SetMaterial(&WallMtrl);
		g_pDevice->SetTexture(0, WallTex);
		g_pDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 6, 4);
		*/

		// draw the mirror
		g_pDevice->SetTransform(D3DTS_WORLD, &matrixMirror);

		g_pDevice->SetMaterial(&matMirror);
		g_pDevice->SetTexture(0, texMirror);
		// Front face
		// g_pDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 18, 2);
		// Left face
		// g_pDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 24, 2);

		g_pDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 18, 12);

		RenderMirror();
	}

	/*
	if (snowOn) {
		snowSystem->update(mirrorTicks);
		snowSystem->render();
	}
	*/
}



// Render mirrors
void Assn1::RenderMirror()
{
	// Setup planes for each mirror (move this to init later)
	D3DXPLANE plane[6] = { 
		D3DXPLANE(0.0f, 0.0f, 1.0f, 2.5f),  // xy plane
		D3DXPLANE(1.0f, 0.0f, 0.0f, 2.5f),
		D3DXPLANE(0.0f, 0.0f, -1.0f, 2.5f),
		D3DXPLANE(-1.0f, 0.0f, 0.0f, 2.5f),
		D3DXPLANE(0.0f, -1.0f, 0.0f, 2.5f),
		D3DXPLANE(0.0f, 1.0f, 0.0f, 2.5f),	
	};

	// Loop through once for each mirror-face
	activeMirrors = 0;

	for (int i = 0; i <= 5; ++i) {
		// Translate planes by mirror matrix
		D3DXPlaneTransform(&plane[i], &plane[i], &matrixMirror);
		if (D3DXPlaneDotNormal(&plane[i], &vEyePt) < 0) {				// Only render mirror-faces that face the camera
			++activeMirrors;
			// set up graphics device for stencilling

			// Clear stencil-buffer in-between drawing individual mirror-faces!
			g_pDevice->Clear(0, 0, D3DCLEAR_STENCIL, NULL, 1.0f, 0);
			g_pDevice->Clear(0, 0, D3DCLEAR_ZBUFFER, 0, 1.0f, 0);

			g_pDevice->SetRenderState(D3DRS_STENCILENABLE, true);
			g_pDevice->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS);
			g_pDevice->SetRenderState(D3DRS_STENCILREF, i+1);  // 0x1
			g_pDevice->SetRenderState(D3DRS_STENCILMASK, 0xffffffff);
			g_pDevice->SetRenderState(D3DRS_STENCILWRITEMASK, 0xffffffff);
			g_pDevice->SetRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP);
			g_pDevice->SetRenderState(D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP);
			g_pDevice->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_REPLACE);



			// disable writes to the depth and back buffers
			g_pDevice->SetRenderState(D3DRS_ZWRITEENABLE, false);
			g_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
			g_pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
			g_pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
			// draw the mirror to the stencil buffer
			g_pDevice->SetStreamSource(0, VB, 0, sizeof(bri::Vertex));
			g_pDevice->SetFVF(bri::Vertex::FVF);
			g_pDevice->SetMaterial(&matMirror);
			g_pDevice->SetTexture(0, texMirror);
			g_pDevice->SetTransform(D3DTS_WORLD, &matrixMirror);
			g_pDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 18 + 6 * i, 2);		// Draw mirror primitive (pulls from vertex-buffer)
			// re-enable depth writes
			g_pDevice->SetRenderState(D3DRS_ZWRITEENABLE, true);


			// Set stencil states for crop render
			g_pDevice->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_EQUAL);
			g_pDevice->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_KEEP);


			// position reflection
			D3DXMATRIX W, T, R;
			// D3DXPLANE plane(0.0f, 0.0f, 1.0f, 2.5f); // moved to array that holds planes for all mirror-faces  -BL
			D3DXMatrixReflect(&R, &plane[i]);
			
			// D3DXMatrixTranslation(&T,
			// 	coords.x,
			// 	coords.y,
			// 	coords.z);
			// W = T * R;
			


			// Clear depth buffer to disable re-render of mirror primaries
			g_pDevice->Clear(0, 0, D3DCLEAR_ZBUFFER, 0, 1.0f, 0);


			// Set device states for blending of mirrored image onto mirror surface
			g_pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
			g_pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);


			// g_pDevice->SetTransform(D3DTS_WORLD, &W);	// not Using this one, instead transform from render method  -BL
			g_pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW); // Moved up two lines

			// Turn on clipping plane to prevent spillage between mirror-faces
			g_pDevice->SetClipPlane(0, plane[i]);
			g_pDevice->SetRenderState(D3DRS_CLIPPLANEENABLE, D3DCLIPPLANE0);

			// Re-render objects in scene as reflections

			// Note that objIndex is the *number* of objects not the index (sigh)
			for (int currentObj = 0; currentObj < objIndex; ++currentObj) {
				// Derive per-mesh transformation from previous state and new rotations applied
				// D3DXMATRIX matMesh = objectsList[currentMesh].matNet;
				if (D3DXPlaneDotNormal(&plane[i], &objectsList[currentObj].boundSphere._center) < 0) {		// Check if object is on the mirror-side of reflection plane
					D3DXMatrixTranslation(&T,
						objectsList[currentObj].boundSphere._center.x,
						objectsList[currentObj].boundSphere._center.y,
						objectsList[currentObj].boundSphere._center.z);
					W = T * R;

					// Combine object's individual matrix w/ reflection matrix
					// D3DXMATRIX matMesh = objectsList[currentMesh].matNet * W;
					D3DXMATRIX matMesh;
					D3DXMatrixIdentity(&matMesh);
					if (currentObj == meshActive)
						matMesh *= matWorld;
					matMesh *= objectsList[currentObj].matScaling * objectsList[currentObj].matRot * W;
					g_pDevice->SetTransform(D3DTS_WORLD, &matMesh);

					for (DWORD i = 0; i < objectsList[currentObj].mesh->numMaterials; i++)
					{
						// Set the material and texture for this subset
						g_pDevice->SetMaterial(&objectsList[currentObj].mesh->meshMaterials[i]);
						g_pDevice->SetTexture(0, objectsList[currentObj].mesh->meshTextures[i]);

						// Draw the mesh subset
						objectsList[currentObj].mesh->mesh->DrawSubset(i);			// Ok maybe we'll say "swing and a miss" for my naming on these two fields..
					}
				}
			}


			// revert to non-stencil render state
			g_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
			g_pDevice->SetRenderState(D3DRS_STENCILENABLE, false);
			g_pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
			g_pDevice->SetRenderState(D3DRS_CLIPPLANEENABLE, false);
		}
	}
	// Set world matrix back to identity
	// Vanilla identity matrix
	D3DXMATRIX I;
	D3DXMatrixIdentity(&I);
	g_pDevice->SetTransform(D3DTS_WORLD, &I);
}



// Bitmap loader
int Assn1::LoadBitmapToSurface(char* PathName, LPDIRECT3DSURFACE9* ppSurface, LPDIRECT3DDEVICE9 pDevice) {
	HRESULT r;
	HBITMAP hBitmap;
	BITMAP Bitmap;

	hBitmap = (HBITMAP)LoadImage(NULL, PathName, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
	if (hBitmap == NULL) {
		SetError("Unable to load bitmap");
		return E_FAIL;
	}

	GetObject(hBitmap, sizeof(BITMAP), &Bitmap);
	DeleteObject(hBitmap);//we only needed it for the header info to create a D3D surface

	//create surface for bitmap
	// Using D3DPOOL_DEFAULT instead of D3DPOOL_SCRATCH as this affects our ability to call StretchRect() later
	r = pDevice->CreateOffscreenPlainSurface(Bitmap.bmWidth, Bitmap.bmHeight, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, ppSurface, NULL);

	if (FAILED(r)) {
		SetError("Unable to create surface for bitmap load");
		return E_FAIL;
	}
	//load bitmap onto surface
	r = D3DXLoadSurfaceFromFile(*ppSurface, NULL, NULL, PathName, NULL, D3DX_DEFAULT, 0, NULL);
	if (FAILED(r)) {
		SetError("Unable to laod file to surface");
		return E_FAIL;
	}

	return S_OK;
}



void Assn1::SetError(char* szFormat, ...) {
	char szBuffer[1024];
	va_list pArgList;

	va_start(pArgList, szFormat);

	_vsntprintf(szBuffer, sizeof(szBuffer) / sizeof(char), szFormat, pArgList);

	va_end(pArgList);

	OutputDebugString(szBuffer);
	OutputDebugString("\n");
	printf("error!!");
}


//-----------------------------------------------------------------------------
// Name: Cleanup()
// Desc: Releases all previously initialized objects
//-----------------------------------------------------------------------------
VOID Assn1::Cleanup()
{
	if (g_pMeshMaterials != NULL)
		delete[] g_pMeshMaterials;

	if (g_pMeshTextures)
	{
		for (DWORD i = 0; i < g_dwNumMaterials; i++)
		{
			if (g_pMeshTextures[i])
				g_pMeshTextures[i]->Release();
		}
		delete[] g_pMeshTextures;
	}
	if (g_pMesh != NULL)
		g_pMesh->Release();

	if (g_pDevice != NULL)
		g_pDevice->Release();

	if (g_pD3D != NULL)
		g_pD3D->Release();
}


// Camera rotation handler
VOID Assn1::rotateCamera(boolean clockwise) {
	int stepAngle = 20;
	matView *= *D3DXMatrixRotationY(NULL, clockwise ? -stepAngle : stepAngle);
	g_pDevice->SetTransform(D3DTS_VIEW, &matView);
}


//-----------------------------------------------------------------------------
// Name: SetupMatrices()
// Desc: Sets up the world, view, and projection transform matrices.
//-----------------------------------------------------------------------------
VOID Assn1::SetupMatrices()
{
	// For our world matrix, we will just leave it as the identity
	D3DXMATRIXA16 matWorldorig;
	// Initialize matrix to identity
	D3DXMatrixIdentity(&matWorldorig);
	D3DXMatrixRotationYawPitchRoll(&matWorld, (oldx + startx - stopx) / mouseFactor, (oldy + starty - stopy) / mouseFactor, 0);
	g_pDevice->SetTransform(D3DTS_WORLD, &matWorldorig);

	// Set up our view matrix. A view matrix can be defined given an eye point,
	// a point to lookat, and a direction for which way is up. Here, we set the
	// eye five units back along the z-axis and up three units, look at the 
	// origin, and define "up" to be in the y-direction.
	// vEyePt = D3DXVECTOR3(0.0f, 20.0f, -40.0f);

	// Matrices to handle camera rotation
	D3DXMATRIX matCamera, matCamera2;
	D3DXMATRIX matRotate;
	D3DXMATRIX matXRotate;
	D3DXMATRIX matScale;
	D3DXMatrixRotationY(&matRotate, camAngle);
	D3DXMatrixRotationX(&matXRotate, camXAngle);
	D3DXMatrixScaling(&matScale, scaleFactor, scaleFactor, scaleFactor);
	matCamera = matXRotate * matRotate * matScale;
	matCamera2 = matXRotate * matRotate;

	// Translate camera position vector
	D3DXVECTOR3 direction(0.0f, 0.0f, 0.1f);
	D3DXVECTOR3 moveDirection;
	D3DXVECTOR3 vUpVec(0.0f, 1.0f, 0.0f);
	direction *= speed;
	// D3DXVec3TransformCoord(&vEyePt, &vEyePt, &matCamera);
	D3DXMATRIXA16 matBomb;
	D3DXMatrixRotationX(&matBomb, D3DX_PI / 8);
	D3DXVec3TransformCoord(&moveDirection, &direction, &matCamera2);
	D3DXVec3TransformCoord(&direction, &direction, &matBomb);
	D3DXVec3TransformCoord(&direction, &direction, &matCamera2);
	// D3DXVECTOR3 bombSight(0.0f, -0.05f, 0.0f);						// Makes the camera look slighty lower than current heading (better visuals)
	D3DXVECTOR3 vLookatPt = vEyePt + direction; //  +bombSight;
	
	D3DXMatrixLookAtLH(&matView, &vEyePt, &vLookatPt, &vUpVec);

	g_pDevice->SetTransform(D3DTS_VIEW, &matView);
	vEyePt += moveDirection;

	// For the projection matrix, we set up a perspective transform (which
	// transforms geometry from 3D view space to 2D viewport space, with
	// a perspective divide making objects smaller in the distance). To build
	// a perpsective transform, we need the field of view (1/4 pi is common),
	// the aspect ratio, and the near and far clipping planes (which define at
	// what distances geometry should be no longer be rendered).
	D3DXMATRIXA16 matProj;
	double viewAngle, aspectRatio, nearPlane, farPlane;
	switch (matProjIndex) {
	case 0:
		viewAngle = D3DX_PI / 4;
		aspectRatio = 0.8;
		nearPlane = 1;
		farPlane = 200;
		break;
	case 1:
		viewAngle = D3DX_PI / 4;
		aspectRatio = 0.8;
		nearPlane = 1;
		farPlane = 300;
		break;
	case 2:
		viewAngle = D3DX_PI / 4;
		aspectRatio = 0.8;
		nearPlane = 1;
		farPlane = 100;
		break;
	case 3:
		viewAngle = D3DX_PI / 4;
		aspectRatio = 0.8;
		nearPlane = 200;
		farPlane = 400;
		break;
	case 4:
		viewAngle = D3DX_PI / 4;
		aspectRatio = 0.8;
		nearPlane = 1;
		farPlane = 500;
		break;
	}
	// D3DXMatrixPerspectiveFovLH(&matProj, D3DX_PI / 4, 1.0f, 1.0f, 100.0f);		// Original values
	D3DXMatrixPerspectiveFovLH(&matProj, viewAngle, aspectRatio, nearPlane, farPlane);
	g_pDevice->SetTransform(D3DTS_PROJECTION, &matProj);

	// Setup matrix for mirror-cube (slow rotation)
	// MMTIME stime;
	// timeGetSystemTime(&stime, sizeof(stime)); // Well that didn't work
	D3DXMATRIX pushDown, tempMirror;
	D3DXMatrixIdentity(&tempMirror);
	int ticksPerSec = 45;
	int cycleTicks = 10 * ticksPerSec;
	int rotsPerCycle = 1;
	double mirrorAngle = (mirrorTicks / (45 * 40.0)) * 2 * D3DX_PI; 
	double mirrorAngle2 = (1 + cos(((2 * cycleTicks - mirrorTicks % cycleTicks) / (double)(cycleTicks / 2)) * D3DX_PI - D3DX_PI)) * D3DX_PI * rotsPerCycle;
	D3DXMatrixRotationYawPitchRoll(&tempMirror, mirrorAngle, mirrorAngle2 * 1.2, mirrorAngle2 + D3DX_PI / 4);
	// D3DXMatrixRotationY(&tempMirror, mirrorAngle);

	// Any application of the translation matrix pushDown screws everything up royally
	D3DXMatrixTranslation(&pushDown, 0, -2.5, 0);
	// matrixMirror = pushDown * tempMirror;
	// matrixMirror = tempMirror * pushDown;
	matrixMirror = tempMirror;

	if (mirrorRotate) ++mirrorTicks;
}


bri::Ray Assn1::CalcPickingRay(int x, int y)
{
	float px = 0.0f;
	float py = 0.0f;
	D3DVIEWPORT9 vp;
	Assn1::g_pDevice->GetViewport(&vp);
	D3DXMATRIX proj;
	Assn1::g_pDevice->GetTransform(D3DTS_PROJECTION, &proj);
	px = (((2.0f*x) / vp.Width) - 1.0f) / proj(0, 0);
	py = (((-2.0f*y) / vp.Height) + 1.0f) / proj(1, 1);
	bri::Ray ray;
	ray._origin = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	ray._direction = D3DXVECTOR3(px, py, 1.0f);
	return ray;
}

void Assn1::TransformRay(bri::Ray* ray, D3DXMATRIX* T)
{
	// transform the ray's origin, w = 1.
	D3DXVec3TransformCoord(
		&ray->_origin,
		&ray->_origin,
		T);
	// transform the ray's direction, w = 0.
	D3DXVec3TransformNormal(
		&ray->_direction,
		&ray->_direction,
		T);
	// normalize the direction
	D3DXVec3Normalize(&ray->_direction, &ray->_direction);
}

bool Assn1::RaySphereIntTest(bri::Ray* ray,	bri::BoundingSphere* sphere)
{
	D3DXVECTOR3 v = ray->_origin - sphere->_center;
	float b = 2.0f * D3DXVec3Dot(&ray->_direction, &v);
	float c = D3DXVec3Dot(&v, &v) - (sphere->_radius * sphere->	_radius);
	// find the discriminant
	float discriminant = (b * b) - (4.0f * c);
	// test for imaginary number
	if (discriminant < 0.0f)
		return false;
	discriminant = sqrtf(discriminant);
	float s0 = (-b + discriminant) / 2.0f;
	float s1 = (-b - discriminant) / 2.0f;
	// if a solution is >= 0, then we intersected the sphere
	if (s0 >= 0.0f || s1 >= 0.0f)
		return true;
	return false;
}


void Assn1::changeActiveMesh(int i) {
	objectsList[meshActive].matRot = matWorld * objectsList[meshActive].matRot;
	meshActive = i;
}



Assn1::Assn1()
{
	startx = 0;
	starty = 0;
	stopx = 0;
	stopy = 0;

	D3DXVECTOR3 sphereCenter;
	sphereCenter.x = -2;
	sphereCenter.y = 0;
	sphereCenter.z = 0;

	Bsphere._center = sphereCenter;
	Bsphere._radius = 0.5;  
}


Assn1::~Assn1()
{
}

LPDIRECT3D9				Assn1::g_pD3D = 0;//COM object
LPDIRECT3DDEVICE9		Assn1::g_pDevice = 0;//graphics device
LPD3DXMESH              Assn1::g_pMesh = NULL; // Our mesh object in sysmem
D3DMATERIAL9*           Assn1::g_pMeshMaterials = NULL; // Materials for our mesh
LPDIRECT3DTEXTURE9*     Assn1::g_pMeshTextures = NULL; // Textures for our mesh
DWORD                   Assn1::g_dwNumMaterials = 0L;   // Number of mesh materials
LPD3DXMESH              Assn1::g_pMesh2 = NULL; // Our mesh object in sysmem
D3DMATERIAL9*           Assn1::g_pMeshMaterials2 = NULL; // Materials for our mesh
LPDIRECT3DTEXTURE9*     Assn1::g_pMeshTextures2 = NULL; // Textures for our mesh
DWORD                   Assn1::g_dwNumMaterials2 = 0L;   // Number of mesh materials
D3DXMATRIXA16			Assn1::matWorld;			// World matrix
D3DXMATRIXA16			Assn1::matView;			// View matrix
D3DXMATRIXA16			Assn1::matProj;			// Projection matrix
boolean					Assn1::mesh1active;		
D3DXMATRIX				Assn1::mesh1matrix;		// Net transformation matrix for mesh 1
D3DXMATRIX				Assn1::mesh2matrix;		// Net transformation matrix for mesh 2
float					Assn1::scaleFactor = 1.0;	// Camera scale factor
Assn1					Assn1::instance;			// Class instance pointer (so static WndProc can make calls)
boolean					Assn1::light1on = true;		// Directional light
boolean					Assn1::light2on = false;	// Spot light
boolean					Assn1::light3on = false;	// Point light (moving)
boolean					Assn1::light4on = true;		// Ambient
D3DLIGHT9				Assn1::Light3;				// Reference to moving point light object
double					Assn1::mouseFactor = 120;	// Pixels of mouse movement required to turn selected object once

// Assignment 3 stuff
bri::BoundingSphere		Assn1::Bsphere;				// Default testing bounding sphere
bri::Obj3d				Assn1::objectsList[MAX_OBJECTS];
const DWORD bri::Vertex::FVF = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1;
int						Assn1::meshIndex = 0;		// Index for last mesh added
int						Assn1::meshActive = 0;		// Index of active mesh
IDirect3DTexture9*		Assn1::texMirror;			// Texture for mirror
D3DMATERIAL9			Assn1::matMirror;			// Material for mirror
IDirect3DVertexBuffer9*	Assn1::VB = 0;
D3DXMATRIX				Assn1::matrixMirror;		// Transformation matrix for mirror
long					Assn1::mirrorTicks = 0;
D3DXVECTOR3				Assn1::vEyePt;
boolean					Assn1::mirrorRotate = true;
int						Assn1::activeMirrors;		// Track how many mirror faces are rendering currently
// psys::Snow*					Assn1::snowSystem;
boolean					Assn1::snowOn = false;		// Toggle snow particle system


HWND Assn1::g_hWndMain;//handle to main window
int Assn1::startx;		// Variables for mouse events
int Assn1::stopx;
int Assn1::starty;
int Assn1::stopy;
int Assn1::oldx;
int Assn1::oldy;
float Assn1::camAngle;	
float Assn1::camXAngle;

LPDIRECT3DSURFACE9 Assn1::pBackSurf;
LPDIRECT3DSURFACE9 Assn1::backStretch;


// Helper from textbook
D3DMATERIAL9 bri::InitMtrl(D3DXCOLOR a, D3DXCOLOR d, D3DXCOLOR s, D3DXCOLOR e, float p)
{
	D3DMATERIAL9 mtrl;
	mtrl.Ambient = a;
	mtrl.Diffuse = d;
	mtrl.Specular = s;
	mtrl.Emissive = e;
	mtrl.Power = p;
	return mtrl;
}

// Extras for research project
bri::MeshObj			Assn1::meshes[10];
int						Assn1::objIndex = 0;
IDirect3DTexture9*		Assn1::texGrass1;			// Texture for grass
IDirect3DTexture9*		Assn1::texGrass2;			// Texture for grass
IDirect3DTexture9*		Assn1::texEarth1;			// Texture for dirt/earth
IDirect3DTexture9*		Assn1::texEarth2;			// Texture for dirt/earth
double					Assn1::speed = 1.0;			// Camera speed
int						Assn1::matProjIndex = 0;			// Which projection matrix to use
int						Assn1::skipObjs = 0;		// Skip over some objects to speed up framerate