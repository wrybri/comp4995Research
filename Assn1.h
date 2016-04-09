#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#include "basics.h"
#endif

#define MAX_OBJECTS 10100
#define SECTOR_SIZE 8
#define MAX_SECTORS 100
#define MAP_SIZE 200
#define BLOCK_WIDTH 5


namespace bri {
	struct Ray
	{
		D3DXVECTOR3 _origin;
		D3DXVECTOR3 _direction;
	};


	struct BoundingSphere
	{
	// BoundingSphere();  // wtf??  -BL
	D3DXVECTOR3 _center;
	float _radius;
	};


	struct MeshObj {
		LPD3DXMESH              mesh = NULL; // mesh object
		D3DMATERIAL9*           meshMaterials = NULL; // Materials for mesh
		LPDIRECT3DTEXTURE9*     meshTextures = NULL; // Textures for mesh
		DWORD                   numMaterials = 0L;   // Number of mesh materials
	};


	struct Obj3d {
		// boolean				active;
		D3DXMATRIX				matScaling;		// Default scaling matrix for mesh 1
		D3DXMATRIX				matTrans;		// Default translation matrix
		D3DXMATRIX				matRot;			// Default rotation matrix
		D3DXMATRIX				matNet;			// Net transformation matrix (for non-translations only)
		bri::BoundingSphere		boundSphere;		// Bounding sphere for object
		bri::MeshObj			*mesh;		// Mesh object to use
		int						type;		// Type of object (for land tiles)
	};

	struct Sector {
		bri::Obj3d* blocks[SECTOR_SIZE * SECTOR_SIZE];
		// int x;
		// int y;
	};


	struct Vertex
	{
		Vertex() {}
		Vertex(float x, float y, float z,
			float nx, float ny, float nz,
			float u, float v)
		{
			_x = x;  _y = y;  _z = z;
			_nx = nx; _ny = ny; _nz = nz;
			_u = u;  _v = v;
		}
		float _x, _y, _z;
		float _nx, _ny, _nz;
		float _u, _v;

		static const DWORD FVF;
	};
	
	// Had to move this out of .h b/c of multiple-declarations errors  -BL
	// const DWORD bri::Vertex::FVF = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1;

	//
	// Colors
	//

	const D3DXCOLOR      WHITE(D3DCOLOR_XRGB(255, 255, 255));
	const D3DXCOLOR      BLACK(D3DCOLOR_XRGB(0, 0, 0));
	const D3DXCOLOR        RED(D3DCOLOR_XRGB(255, 0, 0));
	const D3DXCOLOR      GREEN(D3DCOLOR_XRGB(0, 255, 0));
	const D3DXCOLOR       BLUE(D3DCOLOR_XRGB(0, 0, 255));
	const D3DXCOLOR     YELLOW(D3DCOLOR_XRGB(255, 255, 0));
	const D3DXCOLOR       CYAN(D3DCOLOR_XRGB(0, 255, 255));
	const D3DXCOLOR    MAGENTA(D3DCOLOR_XRGB(255, 0, 255));

	//
	// Materials
	//

	D3DMATERIAL9 InitMtrl(D3DXCOLOR a, D3DXCOLOR d, D3DXCOLOR s, D3DXCOLOR e, float p);

	const D3DMATERIAL9 WHITE_MTRL = InitMtrl(WHITE, WHITE, WHITE, BLACK, 2.0f);
	const D3DMATERIAL9 RED_MTRL = InitMtrl(RED, RED, RED, BLACK, 2.0f);
	const D3DMATERIAL9 GREEN_MTRL = InitMtrl(GREEN, GREEN, GREEN, BLACK, 2.0f);
	const D3DMATERIAL9 BLUE_MTRL = InitMtrl(BLUE, BLUE, BLUE, BLACK, 2.0f);
	const D3DMATERIAL9 YELLOW_MTRL = InitMtrl(YELLOW, YELLOW, YELLOW, BLACK, 2.0f);

}


class Assn1
{
public:
	Assn1();
	static long CALLBACK WndProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam);

	int GameInit();
	int GameLoop();
	int GameShutdown();
	int Render();
	int LoadBitmapToSurface(char* PathName, LPDIRECT3DSURFACE9* ppSurface, LPDIRECT3DDEVICE9 pDevice);

	// int InitDirect3DDevice(HWND hWndTarget, int Width, int Height, BOOL bWindowed, D3DFORMAT FullScreenFormat, LPDIRECT3D9 pD3D, LPDIRECT3DDEVICE9* ppDevice);
	static void SetError(char* szFormat, ...);
	// void Draw(int Pitch, DWORD* pData, int x1, int y1, int x2, int y2);

	// Assignment 2
	HRESULT InitGeometry();
	VOID Cleanup();
	VOID SetupMatrices();
	VOID Render2();
	VOID rotateCamera(boolean clockwise);

	// Assignment3
	static bool RaySphereIntTest(bri::Ray* ray, bri::BoundingSphere* sphere);
	static void TransformRay(bri::Ray* ray, D3DXMATRIX* T);
	static bri::Ray CalcPickingRay(int x, int y);
	static HRESULT loadMesh(std::string filename);
	static void changeActiveMesh(int i);
	static void RenderMirror();

	// Research
	static void createObject(float scaleFactor, bri::MeshObj* meshType, float x, float y, float z);
	static bri::Obj3d* createLandSquare(int landType, double height);
	static bri::Sector* createSector();
	static bri::Sector* findSector(int x, int y);


	~Assn1();

	// Globals
	static LPDIRECT3D9 g_pD3D;//COM object
	static LPDIRECT3DDEVICE9 g_pDevice;//graphics device
	static HWND g_hWndMain;//handle to main window
	INT64 timeStamp;
	static LPDIRECT3DSURFACE9 pBackSurf;
	static LPDIRECT3DSURFACE9 backStretch;
	static int startx, stopx, starty, stopy, oldx, oldy;
	static Assn1 instance;

	// Assignment 2
	static LPD3DXMESH              g_pMesh; // Our mesh object in sysmem
	static D3DMATERIAL9*           g_pMeshMaterials; // Materials for our mesh
	static LPDIRECT3DTEXTURE9*     g_pMeshTextures; // Textures for our mesh
	static DWORD                   g_dwNumMaterials;   // Number of mesh materials
	static LPD3DXMESH              g_pMesh2; // Our mesh object in sysmem
	static D3DMATERIAL9*           g_pMeshMaterials2; // Materials for our mesh
	static LPDIRECT3DTEXTURE9*     g_pMeshTextures2; // Textures for our mesh
	static DWORD                   g_dwNumMaterials2;   // Number of mesh materials
	static D3DXMATRIXA16			matWorld;			// World matrix
	static D3DXMATRIXA16			matView;			// View matrix
	static D3DXMATRIXA16			matProj;			// Projection matrix
	static float					camAngle;
	static float					camXAngle;
	static boolean					mesh1active;				// Mesh 1 active
	static D3DXMATRIX				mesh1matrix;
	static D3DXMATRIX				mesh2matrix;
	static float					scaleFactor;
	static boolean					light1on;
	static boolean					light2on;
	static boolean					light3on;
	static boolean					light4on;
	static  D3DLIGHT9				Light3;
	static double					mouseFactor;

	// Assignment 3 stuff
	static bri::BoundingSphere		Bsphere;
	static bri::Obj3d				objectsList[];
	static int						meshIndex;
	static int						meshActive;
	static IDirect3DTexture9*		texMirror;
	static D3DMATERIAL9				matMirror;
	static IDirect3DVertexBuffer9*	VB;
	static D3DXMATRIX				matrixMirror;
	static long						mirrorTicks;
	static D3DXVECTOR3				vEyePt;
	static boolean					mirrorRotate;
	static int						activeMirrors;
	// static psys::Snow				*snowSystem;
	static boolean					snowOn;

	// Research project extras
	static bri::MeshObj					meshes[];
	static int							objIndex;			// Number of objects currently in scene
	static IDirect3DTexture9*		texGrass1;
	static IDirect3DTexture9*		texGrass2;
	static IDirect3DTexture9*		texEarth1;
	static IDirect3DTexture9*		texEarth2;
	static double					speed;					// Camera forward speed
	static int						matProjIndex;
	static int						skipObjs;				// Skip over objects to render
	static bri::Sector*				map[];
	static bri::Sector*				sectorList[];
};