//--------------------------------------------------------------------------------------
// File: main.cpp
//
// This application demonstrates animation using matrix transformations
//
// http://msdn.microsoft.com/en-us/library/windows/apps/ff729722.aspx
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#define _XM_NO_INTRINSICS_

#include "main.h"

DirectX::XMFLOAT4 g_EyePosition(0.0f, 0, -0, 1.0f);
XMFLOAT4 LightPosition(g_EyePosition);
//--------------------------------------------------------------------------------------
// Forward declarations
//--------------------------------------------------------------------------------------
HRESULT		InitWindow(HINSTANCE hInstance, int nCmdShow);
HRESULT		InitDevice();
HRESULT		InitMesh();
HRESULT		InitWorld(int width, int height);
void		CleanupDevice();
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
void		Render();
void KeyboardInput();
void ImGuiRender();
void Update();
HRESULT CreateTerrainGridHM();
HRESULT CreateTerrainDiamondSquare();
HRESULT CreateTerrainFaultFormation();

void DiamondStep(int sideLength);
void SquareStep(int sideLength);
void average(int x, int y, int sideLength);
void GenerationDiamondSquare();
//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
HINSTANCE               g_hInst = nullptr;
HWND                    g_hWnd = nullptr;
D3D_DRIVER_TYPE         g_driverType = D3D_DRIVER_TYPE_NULL;
D3D_FEATURE_LEVEL       g_featureLevel = D3D_FEATURE_LEVEL_11_0;
ID3D11Device* g_pd3dDevice = nullptr;
ID3D11Device1* g_pd3dDevice1 = nullptr;
ID3D11DeviceContext* g_pImmediateContext = nullptr;
ID3D11DeviceContext1* g_pImmediateContext1 = nullptr;
IDXGISwapChain* g_pSwapChain = nullptr;
IDXGISwapChain1* g_pSwapChain1 = nullptr;
ID3D11RenderTargetView* g_pRenderTargetView = nullptr;
ID3D11Texture2D* g_pDepthStencil = nullptr;
ID3D11DepthStencilView* g_pDepthStencilView = nullptr;
ID3D11VertexShader* g_pVertexShader = nullptr;

ID3D11PixelShader* g_pPixelShader = nullptr;

ID3D11InputLayout* g_pVertexLayout = nullptr;

ID3D11HullShader* g_pHullShader = nullptr;

ID3D11DomainShader* g_pDomainShader = nullptr;

ID3D11Buffer* g_pConstantBuffer = nullptr;

ID3D11Buffer* g_pLightConstantBuffer = nullptr;

ID3D11RasterizerState* g_pWireFrame;
ID3D11SamplerState* g_pSamplerState = nullptr;

ID3D11Buffer* g_pGridVertexBuffer = nullptr;
ID3D11Buffer* g_pGridIndexBuffer = nullptr;

ID3D11Buffer* g_pGridDSVertexBuffer = nullptr;
ID3D11Buffer* g_pGridDSIndexBuffer = nullptr;

ID3D11Buffer* g_pGridFFVertexBuffer = nullptr;
ID3D11Buffer* g_pGridFFIndexBuffer = nullptr;

ID3D11Buffer* g_pTerrainMaterialBuffer = nullptr;
MaterialPropertiesConstantBuffer	m_material;

// Camera
XMMATRIX                g_View;
XMMATRIX                g_Projection;
Camera* g_pCurrentCamera = nullptr;
Camera* g_pCamera0 = nullptr;
float currentPosZ = -2.0f;
float currentPosX = 0.0f;
float currentPosY = 0.0f;
float rotationX = 0.0f;
float rotationY = 0.0f;

int						g_viewWidth;
int						g_viewHeight;

bool isWireFrame = false;

DrawableGameObject		g_GameObject;

// Terrain
UINT rows;
UINT columns;
int totalCells;
UINT totalFaces;
int totalVertices;
int depth;
int width;
// Width & Depth
float dx;
float dz;
//Texture Coords
float du;
float dv;

// Terrain Object
XMFLOAT4X4 g_Terrian;

// Terrain Textures
ID3D11ShaderResourceView* g_pTextureGrass = nullptr;
ID3D11ShaderResourceView* g_pTextureStone = nullptr;
ID3D11ShaderResourceView* g_pTextureSnow = nullptr;
ID3D11ShaderResourceView* g_pDispacementMap = nullptr;

XMFLOAT3 terrainPos = XMFLOAT3(0.0f, -5.0f, 0.0f);
XMFLOAT3 terrainRot = XMFLOAT3(0.0f, 0.0f, 0.0f);

int terrainID;
float terrainHeight = 4.0f;
float terrainBias = 0.0f;

bool diamondSquare = false;
bool faultFormation = false;
int randomSeed = 100;

int faultIterations = 10;

int imGUIRoughness = 15;


//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	if (FAILED(InitWindow(hInstance, nCmdShow)))
		return 0;

	if (FAILED(InitDevice()))
	{
		CleanupDevice();
		return 0;
	}

	// Main message loop
	MSG msg = { 0 };
	while (WM_QUIT != msg.message)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			Update();
			KeyboardInput();
			Render();
		}
	}

	CleanupDevice();

	return (int)msg.wParam;
}


//--------------------------------------------------------------------------------------
// Register class and create window
//--------------------------------------------------------------------------------------
HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow)
{
	// Register class
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, (LPCTSTR)IDI_TUTORIAL1);
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = nullptr;
	wcex.lpszClassName = L"TutorialWindowClass";
	wcex.hIconSm = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_TUTORIAL1);
	if (!RegisterClassEx(&wcex))
		return E_FAIL;

	// Create window
	g_hInst = hInstance;
	RECT rc = { 0, 0, 1280, 720 };

	g_viewWidth = 1280;
	g_viewHeight = 720;

	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
	g_hWnd = CreateWindow(L"TutorialWindowClass", L"Direct3D 11 Tutorial 5",
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
		CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance,
		nullptr);
	if (!g_hWnd)
		return E_FAIL;

	ShowWindow(g_hWnd, nCmdShow);

	return S_OK;
}


//--------------------------------------------------------------------------------------
// Helper for compiling shaders with D3DCompile
//
// With VS 11, we could load up prebuilt .cso files instead...
//--------------------------------------------------------------------------------------
HRESULT CompileShaderFromFile(const WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
	HRESULT hr = S_OK;

	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
	// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
	// Setting this flag improves the shader debugging experience, but still allows 
	// the shaders to be optimized and to run exactly the way they will run in 
	// the release configuration of this program.
	dwShaderFlags |= D3DCOMPILE_DEBUG;

	// Disable optimizations to further improve shader debugging
	dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	ID3DBlob* pErrorBlob = nullptr;
	hr = D3DCompileFromFile(szFileName, nullptr, nullptr, szEntryPoint, szShaderModel,
		dwShaderFlags, 0, ppBlobOut, &pErrorBlob);
	if (FAILED(hr))
	{
		if (pErrorBlob)
		{
			OutputDebugStringA(reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer()));
			pErrorBlob->Release();
		}
		return hr;
	}
	if (pErrorBlob) pErrorBlob->Release();

	return S_OK;
}


//--------------------------------------------------------------------------------------
// Create Direct3D device and swap chain
//--------------------------------------------------------------------------------------
HRESULT InitDevice()
{
	HRESULT hr = S_OK;

	RECT rc;
	GetClientRect(g_hWnd, &rc);
	UINT width = rc.right - rc.left;
	UINT height = rc.bottom - rc.top;

	UINT createDeviceFlags = 0;
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT numDriverTypes = ARRAYSIZE(driverTypes);

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		g_driverType = driverTypes[driverTypeIndex];
		hr = D3D11CreateDevice(nullptr, g_driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
			D3D11_SDK_VERSION, &g_pd3dDevice, &g_featureLevel, &g_pImmediateContext);

		if (hr == E_INVALIDARG)
		{
			// DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
			hr = D3D11CreateDevice(nullptr, g_driverType, nullptr, createDeviceFlags, &featureLevels[1], numFeatureLevels - 1,
				D3D11_SDK_VERSION, &g_pd3dDevice, &g_featureLevel, &g_pImmediateContext);
		}

		if (SUCCEEDED(hr))
			break;
	}
	if (FAILED(hr))
		return hr;

	// Obtain DXGI factory from device (since we used nullptr for pAdapter above)
	IDXGIFactory1* dxgiFactory = nullptr;
	{
		IDXGIDevice* dxgiDevice = nullptr;
		hr = g_pd3dDevice->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgiDevice));
		if (SUCCEEDED(hr))
		{
			IDXGIAdapter* adapter = nullptr;
			hr = dxgiDevice->GetAdapter(&adapter);
			if (SUCCEEDED(hr))
			{
				hr = adapter->GetParent(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&dxgiFactory));
				adapter->Release();
			}
			dxgiDevice->Release();
		}
	}
	if (FAILED(hr))
		return hr;

	// Create swap chain
	IDXGIFactory2* dxgiFactory2 = nullptr;
	hr = dxgiFactory->QueryInterface(__uuidof(IDXGIFactory2), reinterpret_cast<void**>(&dxgiFactory2));
	if (dxgiFactory2)
	{
		// DirectX 11.1 or later
		hr = g_pd3dDevice->QueryInterface(__uuidof(ID3D11Device1), reinterpret_cast<void**>(&g_pd3dDevice1));
		if (SUCCEEDED(hr))
		{
			(void)g_pImmediateContext->QueryInterface(__uuidof(ID3D11DeviceContext1), reinterpret_cast<void**>(&g_pImmediateContext1));
		}

		DXGI_SWAP_CHAIN_DESC1 sd = {};
		sd.Width = width;
		sd.Height = height;
		sd.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;//  DXGI_FORMAT_R16G16B16A16_FLOAT;////DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.BufferCount = 1;

		hr = dxgiFactory2->CreateSwapChainForHwnd(g_pd3dDevice, g_hWnd, &sd, nullptr, nullptr, &g_pSwapChain1);
		if (SUCCEEDED(hr))
		{
			hr = g_pSwapChain1->QueryInterface(__uuidof(IDXGISwapChain), reinterpret_cast<void**>(&g_pSwapChain));
		}

		dxgiFactory2->Release();
	}
	else
	{
		// DirectX 11.0 systems
		DXGI_SWAP_CHAIN_DESC sd = {};
		sd.BufferCount = 1;
		sd.BufferDesc.Width = width;
		sd.BufferDesc.Height = height;
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferDesc.RefreshRate.Numerator = 60;
		sd.BufferDesc.RefreshRate.Denominator = 1;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.OutputWindow = g_hWnd;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.Windowed = TRUE;

		hr = dxgiFactory->CreateSwapChain(g_pd3dDevice, &sd, &g_pSwapChain);
	}

	// Note this tutorial doesn't handle full-screen swapchains so we block the ALT+ENTER shortcut
	dxgiFactory->MakeWindowAssociation(g_hWnd, DXGI_MWA_NO_ALT_ENTER);

	dxgiFactory->Release();

	if (FAILED(hr))
		return hr;

	// Create a render target view
	ID3D11Texture2D* pBackBuffer = nullptr;
	hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer));
	if (FAILED(hr))
		return hr;

	hr = g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_pRenderTargetView);
	pBackBuffer->Release();
	if (FAILED(hr))
		return hr;

	// Create depth stencil texture
	D3D11_TEXTURE2D_DESC descDepth = {};
	descDepth.Width = width;
	descDepth.Height = height;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;
	hr = g_pd3dDevice->CreateTexture2D(&descDepth, nullptr, &g_pDepthStencil);
	if (FAILED(hr))
		return hr;

	// Create the depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
	descDSV.Format = descDepth.Format;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
	hr = g_pd3dDevice->CreateDepthStencilView(g_pDepthStencil, &descDSV, &g_pDepthStencilView);
	if (FAILED(hr))
		return hr;

	g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView, g_pDepthStencilView);

	// Setup the viewport
	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT)width;
	vp.Height = (FLOAT)height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	g_pImmediateContext->RSSetViewports(1, &vp);

	D3D11_RASTERIZER_DESC rs = {};
	ZeroMemory(&rs, sizeof(D3D11_RASTERIZER_DESC));
	rs.FillMode = D3D11_FILL_WIREFRAME;
	rs.CullMode = D3D11_CULL_BACK;
	hr = g_pd3dDevice->CreateRasterizerState(&rs, &g_pWireFrame);

	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	g_pd3dDevice->CreateSamplerState(&sampDesc, &g_pSamplerState);

	m_material.Material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_material.Material.Specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_material.Material.SpecularPower = 32.0f;
	m_material.Material.UseTexture = true;

	// Create the material constant buffer
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(MaterialPropertiesConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	hr = g_pd3dDevice->CreateBuffer(&bd, nullptr, &g_pTerrainMaterialBuffer);
	if (FAILED(hr))
		return hr;

	hr = CreateDDSTextureFromFile(g_pd3dDevice, L"grass.dds", nullptr, &g_pTextureGrass);
	hr = CreateDDSTextureFromFile(g_pd3dDevice, L"stone.dds", nullptr, &g_pTextureStone);
	hr = CreateDDSTextureFromFile(g_pd3dDevice, L"snow.dds", nullptr, &g_pTextureSnow);
	hr = CreateDDSTextureFromFile(g_pd3dDevice, L"HeightMap.dds", nullptr, &g_pDispacementMap);

	hr = InitMesh();
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"Failed to initialise mesh.", L"Error", MB_OK);
		return hr;
	}

	hr = InitWorld(width, height);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"Failed to initialise world.", L"Error", MB_OK);
		return hr;
	}

	hr = g_GameObject.initMesh(g_pd3dDevice, g_pImmediateContext);
	if (FAILED(hr))
		return hr;

	return S_OK;
}

// ***************************************************************************************
// InitMesh
// ***************************************************************************************

HRESULT		InitMesh()
{
	// Compile the vertex shader
	ID3DBlob* pVSBlob = nullptr;
	HRESULT hr = CompileShaderFromFile(L"shader.fx", "VS", "vs_5_0", &pVSBlob);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return hr;
	}

	// Create the vertex shader
	hr = g_pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &g_pVertexShader);
	if (FAILED(hr))
	{
		pVSBlob->Release();
		return hr;
	}

	// Define the input layout
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = ARRAYSIZE(layout);

	// Create the input layout
	hr = g_pd3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(),
		pVSBlob->GetBufferSize(), &g_pVertexLayout);
	pVSBlob->Release();
	if (FAILED(hr))
		return hr;

	// Set the input layout
	g_pImmediateContext->IASetInputLayout(g_pVertexLayout);

	// Compile the Hull Shader
	ID3DBlob* pHSBlob = nullptr;
	hr = CompileShaderFromFile(L"shader.fx", "HSMAIN", "hs_5_0", &pHSBlob);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return hr;
	}

	// Create the hull shader
	hr = g_pd3dDevice->CreateHullShader(pHSBlob->GetBufferPointer(), pHSBlob->GetBufferSize(), nullptr, &g_pHullShader);
	if (FAILED(hr))
	{
		pVSBlob->Release();
		return hr;
	}

	// Compile the Domain Shader
	ID3DBlob* pDSBlob = nullptr;
	hr = CompileShaderFromFile(L"shader.fx", "DSMAIN", "ds_5_0", &pHSBlob);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return hr;
	}

	// Create the domain shader
	hr = g_pd3dDevice->CreateDomainShader(pHSBlob->GetBufferPointer(), pHSBlob->GetBufferSize(), nullptr, &g_pDomainShader);
	if (FAILED(hr))
	{
		pVSBlob->Release();
		return hr;
	}

	// Compile the pixel shader
	ID3DBlob* pPSBlob = nullptr;
	hr = CompileShaderFromFile(L"shader.fx", "PS", "ps_4_0", &pPSBlob);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return hr;
	}

	// Create the pixel shader
	hr = g_pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &g_pPixelShader);
	pPSBlob->Release();
	if (FAILED(hr))
		return hr;


	// Create the constant buffer
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	hr = g_pd3dDevice->CreateBuffer(&bd, nullptr, &g_pConstantBuffer);
	if (FAILED(hr))
		return hr;



	// Create the light constant buffer
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(LightPropertiesConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	hr = g_pd3dDevice->CreateBuffer(&bd, nullptr, &g_pLightConstantBuffer);
	if (FAILED(hr))
		return hr;


	return hr;
}

// ***************************************************************************************
// InitWorld
// ***************************************************************************************
HRESULT		InitWorld(int width, int height)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();

	ImGui_ImplWin32_Init(g_hWnd);
	ImGui_ImplDX11_Init(g_pd3dDevice, g_pImmediateContext);
	ImGui::StyleColorsClassic();

	g_pCamera0 = new Camera(XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), g_viewWidth, g_viewHeight, 0.01f, 10000.0f);
	g_pCurrentCamera = g_pCamera0;
	g_pCurrentCamera->SetView();
	g_pCurrentCamera->SetProjection();
	g_pCurrentCamera->SetProjectionView();

	CreateTerrainGridHM();
	CreateTerrainDiamondSquare();
	CreateTerrainFaultFormation();

	return S_OK;
}


//--------------------------------------------------------------------------------------
// Clean up the objects we've created
//--------------------------------------------------------------------------------------
void CleanupDevice()
{
	g_GameObject.cleanup();

	// Remove any bound render target or depth/stencil buffer
	ID3D11RenderTargetView* nullViews[] = { nullptr };
	g_pImmediateContext->OMSetRenderTargets(_countof(nullViews), nullViews, nullptr);

	if (g_pImmediateContext) g_pImmediateContext->ClearState();
	// Flush the immediate context to force cleanup
	if (g_pImmediateContext1) g_pImmediateContext1->Flush();
	g_pImmediateContext->Flush();

	if (g_pLightConstantBuffer)
		g_pLightConstantBuffer->Release();
	if (g_pVertexLayout) g_pVertexLayout->Release();
	if (g_pConstantBuffer) g_pConstantBuffer->Release();
	if (g_pVertexShader) g_pVertexShader->Release();
	if (g_pPixelShader) g_pPixelShader->Release();
	if (g_pDepthStencil) g_pDepthStencil->Release();
	if (g_pDepthStencilView) g_pDepthStencilView->Release();
	if (g_pRenderTargetView) g_pRenderTargetView->Release();
	if (g_pSwapChain1) g_pSwapChain1->Release();
	if (g_pSwapChain) g_pSwapChain->Release();
	if (g_pImmediateContext1) g_pImmediateContext1->Release();
	if (g_pImmediateContext) g_pImmediateContext->Release();
	if (g_pHullShader) g_pHullShader->Release();
	if (g_pDomainShader) g_pDomainShader->Release();
	if (g_pWireFrame) g_pWireFrame->Release();

	ID3D11Debug* debugDevice = nullptr;
	g_pd3dDevice->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&debugDevice));

	if (g_pd3dDevice1) g_pd3dDevice1->Release();
	if (g_pd3dDevice) g_pd3dDevice->Release();

	// handy for finding dx memory leaks
	debugDevice->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);

	if (debugDevice)
		debugDevice->Release();
}


//--------------------------------------------------------------------------------------
// Called every time the application receives a message
//--------------------------------------------------------------------------------------
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
	{
		return true;
	}

	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_LBUTTONDOWN:
	{
		int xPos = GET_X_LPARAM(lParam);
		int yPos = GET_Y_LPARAM(lParam);
		break;
	}
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

		// Note that this tutorial does not handle resizing (WM_SIZE) requests,
		// so we created the window without the resize border.

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

void DiamondStep(int sideLength)
{
	srand(randomSeed);

	int halfSide = sideLength / 2;

	for (int y = 0; y < terrainSizeHeight / (sideLength - 1); y++)
	{
		for (int x = 0; x < terrainSizeWidth / (sideLength - 1); x++)
		{
			int center_x = x * (sideLength - 1) + halfSide;
			int center_y = y * (sideLength - 1) + halfSide;

			int avg = (map[x * (sideLength - 1)][y * (sideLength - 1)] +
				map[x * (sideLength - 1)][(y + 1) * (sideLength - 1)] +
				map[(x + 1) * (sideLength - 1)][y * (sideLength - 1)] +
				map[(x + 1) * (sideLength - 1)][(y + 1) * (sideLength - 1)]) / 4.0f;

			map[center_x][center_y] = avg + rand() % roughness + -roughness;
		}
	}

}

void SquareStep(int sideLength)
{
	int halfLength = sideLength / 2;

	for (int y = 0; y < terrainSizeHeight / (sideLength - 1); y++)
	{
		for (int x = 0; x < terrainSizeWidth / (sideLength - 1); x++)
		{
			// Top
			average(x * (sideLength - 1) + halfLength, y * (sideLength - 1), sideLength);
			// Right
			average((x + 1) * (sideLength - 1), y * (sideLength - 1) + halfLength,
				sideLength);
			// Bottom
			average(x * (sideLength - 1) + halfLength, (y + 1) * (sideLength - 1), sideLength);
			// Left
			average(x * (sideLength - 1), y * (sideLength - 1) + halfLength, sideLength);
		}
	}

}

void average(int x, int y, int sideLength)
{
	srand(randomSeed);

	float counter = 0;
	float accumulator = 0;

	int halfSide = sideLength / 2;

	if (x != 0)
	{
		counter += 1.0f;
		accumulator += map[y][x - halfSide];
	}
	if (y != 0)
	{
		counter += 1.0f;
		accumulator += map[y - halfSide][x];
	}
	if (x != terrainSizeWidth - 1)
	{
		counter += 1.0f;
		accumulator += map[y][x + halfSide];
	}
	if (y != terrainSizeHeight - 1)
	{
		counter += 1.0f;
		accumulator += map[y + halfSide][x];
	}

	map[y][x] = (accumulator / counter) + rand() % roughness + -roughness;
}

void GenerationDiamondSquare()
{
	srand(randomSeed);

	int sideLength = terrainSizeWidth / 2;

	map[0][0] = rand() % roughness + -roughness;
	map[0][sideLength - 1] = rand() % roughness + -roughness;
	map[sideLength - 1][0] = rand() % roughness + -roughness;
	map[sideLength - 1][sideLength - 1] = rand() % roughness + -roughness;

	DiamondStep(terrainSizeWidth);
	SquareStep(terrainSizeWidth);

	roughness /= 2;

	while (sideLength >= 2)
	{
		DiamondStep(sideLength + 1);
		SquareStep(sideLength + 1);
		sideLength /= 2;

		if (roughness >= 2)
		{
			roughness /= 2;
		}
	}
}

HRESULT CreateTerrainGridHM()
{
	HRESULT hr;

	// Grid Generation
	rows = terrainSizeWidth;
	columns = terrainSizeHeight;

	float halfDepth = 0.5f * columns;
	float halfWidth = 0.5f * rows;

	rows = rows - 1;
	columns = columns - 1;
	totalCells = rows * columns;
	totalFaces = rows * columns * 2;
	totalVertices = rows * columns;

	dx = rows / (columns);
	dz = columns / (rows);

	du = 1.0f / (columns);
	dv = 1.0f / (rows);

	std::vector<SimpleVertex> v(totalVertices);

	// Vertex Generation
	for (UINT i = 0; i < rows; ++i)
	{
		float z = halfDepth - i * dz;
		for (UINT j = 0; j < columns; ++j)
		{
			float x = j * dx + (-dx * 0.5);
			float y = 0.0f;

			v[i * columns + j].Pos = XMFLOAT3(x, y, z);
			v[i * columns + j].Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);

			v[i * columns + j].TexCoord.x = j * du;
			v[i * columns + j].TexCoord.y = i * dv;
		}
	}

	std::vector<WORD> indices(totalFaces * 3);

	int k = 0;
	// Index Generation
	for (UINT i = 0; i < rows - 1; ++i)
	{
		for (UINT j = 0; j < columns - 1; ++j)
		{
			indices[k] = i * columns + j;
			indices[k + 1] = i * columns + j + 1;
			indices[k + 2] = (i + 1) * columns + j;
			indices[k + 3] = (i + 1) * columns + j;
			indices[k + 4] = i * columns + j + 1;
			indices[k + 5] = (i + 1) * columns + j + 1;

			k += 6;
		}
	}

	// Vertex Buffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(SimpleVertex) * totalVertices;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = v.data();
	hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pGridVertexBuffer);

	if (FAILED(hr))
		return hr;

	// Index Buffer
	D3D11_BUFFER_DESC bd2;
	ZeroMemory(&bd2, sizeof(bd2));

	bd2.Usage = D3D11_USAGE_DEFAULT;
	bd2.ByteWidth = sizeof(WORD) * indices.size();
	bd2.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd2.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData2;
	ZeroMemory(&InitData2, sizeof(InitData2));
	InitData2.pSysMem = indices.data();
	hr = g_pd3dDevice->CreateBuffer(&bd2, &InitData2, &g_pGridIndexBuffer);

	if (FAILED(hr))
		return hr;


	return hr;
}

HRESULT CreateTerrainDiamondSquare()
{
	HRESULT hr;

	GenerationDiamondSquare();

	// Grid Generation
	rows = terrainSizeWidth;
	columns = terrainSizeHeight;

	float halfDepth = 0.5f * columns;
	float halfWidth = 0.5f * rows;

	rows = rows - 1;
	columns = columns - 1;
	totalCells = rows * columns;
	totalFaces = rows * columns * 2;
	totalVertices = rows * columns;

	dx = rows / (columns);
	dz = columns / (rows);

	du = 1.0f / (columns);
	dv = 1.0f / (rows);

	std::vector<SimpleVertex> v(totalVertices);

	// Vertex Generation
	for (UINT i = 0; i < rows; ++i)
	{
		float z = halfDepth - i * dz;
		for (UINT j = 0; j < columns; ++j)
		{
			float x = j * dx + (-dx * 0.5);
			float y = map[i][j];

			v[i * columns + j].Pos = XMFLOAT3(x, y, z);
			v[i * columns + j].Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);

			v[i * columns + j].TexCoord.x = j * du;
			v[i * columns + j].TexCoord.y = i * dv;
		}
	}

	std::vector<WORD> indices(totalFaces * 3);

	int k = 0;
	// Index Generation
	for (UINT i = 0; i < rows - 1; ++i)
	{
		for (UINT j = 0; j < columns - 1; ++j)
		{
			indices[k] = i * columns + j;
			indices[k + 1] = i * columns + j + 1;
			indices[k + 2] = (i + 1) * columns + j;
			indices[k + 3] = (i + 1) * columns + j;
			indices[k + 4] = i * columns + j + 1;
			indices[k + 5] = (i + 1) * columns + j + 1;

			k += 6;
		}
	}

	// Vertex Buffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(SimpleVertex) * totalVertices;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = v.data();
	hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pGridDSVertexBuffer);

	if (FAILED(hr))
		return hr;

	// Index Buffer
	D3D11_BUFFER_DESC bd2;
	ZeroMemory(&bd2, sizeof(bd2));

	bd2.Usage = D3D11_USAGE_DEFAULT;
	bd2.ByteWidth = sizeof(WORD) * indices.size();
	bd2.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd2.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData2;
	ZeroMemory(&InitData2, sizeof(InitData2));
	InitData2.pSysMem = indices.data();
	hr = g_pd3dDevice->CreateBuffer(&bd2, &InitData2, &g_pGridDSIndexBuffer);

	if (FAILED(hr))
		return hr;


	return hr;
}

HRESULT CreateTerrainFaultFormation()
{
	HRESULT hr;

	srand(randomSeed);
	// Grid Generation
	rows = terrainSizeWidth;
	columns = terrainSizeHeight;

	float halfDepth = 0.5f * columns;
	float halfWidth = 0.5f * rows;

	rows = rows - 1;
	columns = columns - 1;
	totalCells = rows * columns;
	totalFaces = rows * columns * 2;
	totalVertices = rows * columns;

	dx = rows / columns;
	dz = columns / rows;

	du = 1.0f / columns;
	dv = 1.0f / rows;

	std::vector<SimpleVertex> v(totalVertices);

	int index;
	float y = 0.0f;
	// Vertex Generation
	for (UINT i = 0; i < rows; ++i)
	{
		float z = halfDepth - i * dz;
		for (UINT j = 0; j < columns; ++j)
		{

			for (int iter = 0; iter < faultIterations; ++iter)
			{
				float xLine1, yLine1, xLine2, yLine2;

				float m = 0.0f, b = 0.0f;

				xLine1 = terrainSizeWidth * 0.1f + rand() % terrainSizeWidth * 0.8f;
				yLine1 = (rand() % 2 == 0) ? terrainSizeHeight - 1 : 0;

				float randomPoint1 = 0;

				while (randomPoint1 == 0.0f || xLine2 == xLine1)
				{
					randomPoint1 = rand() % roughness + -roughness;
					xLine2 = xLine1 + randomPoint1;
				}

				float randomPoint2 = 0;
				while (randomPoint2 == 0.0f)
				{
					randomPoint2 = rand() % roughness + -roughness;
					yLine2 = yLine1 + randomPoint2;
				}

				m = (yLine2 - yLine1) / (xLine2 - xLine1);
				b = yLine1 - (xLine1 * m);

				float H1 = rand() % roughness + -roughness;

				float H2 = H1 * 0.5f;


				float x = j * dx + -dx * 0.5;

				y = 0.0f;

				bool eq = j > i * m + b;

				if (eq)
				{
					y += H1;
				}

				y -= H2;

				v[i * columns + j].Pos = XMFLOAT3(x, y, z);
				v[i * columns + j].Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);

				v[i * columns + j].TexCoord.x = j * du;
				v[i * columns + j].TexCoord.y = i * dv;
			}
		}
	}

	std::vector<WORD> indices(totalFaces * 3);

	int k = 0;
	// Index Generation
	for (UINT i = 0; i < rows - 1; ++i)
	{
		for (UINT j = 0; j < columns - 1; ++j)
		{
			indices[k] = i * columns + j;
			indices[k + 1] = i * columns + j + 1;
			indices[k + 2] = (i + 1) * columns + j;
			indices[k + 3] = (i + 1) * columns + j;
			indices[k + 4] = i * columns + j + 1;
			indices[k + 5] = (i + 1) * columns + j + 1;

			k += 6;
		}
	}

	// Vertex Buffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(SimpleVertex) * totalVertices;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = v.data();
	hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pGridFFVertexBuffer);

	if (FAILED(hr))
		return hr;

	// Index Buffer
	D3D11_BUFFER_DESC bd2;
	ZeroMemory(&bd2, sizeof(bd2));

	bd2.Usage = D3D11_USAGE_DEFAULT;
	bd2.ByteWidth = sizeof(WORD) * indices.size();
	bd2.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd2.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData2;
	ZeroMemory(&InitData2, sizeof(InitData2));
	InitData2.pSysMem = indices.data();
	hr = g_pd3dDevice->CreateBuffer(&bd2, &InitData2, &g_pGridFFIndexBuffer);

	if (FAILED(hr))
		return hr;


	return hr;
}

void setupLightForRender()
{
	Light light;
	light.Enabled = static_cast<int>(true);
	light.LightType = PointLight;
	light.Color = XMFLOAT4(Colors::White);
	light.SpotAngle = XMConvertToRadians(45.0f);
	light.ConstantAttenuation = 1.0f;
	light.LinearAttenuation = 1;
	light.QuadraticAttenuation = 1;

	// set up the light
	light.Position = LightPosition;
	XMVECTOR LightDirection = XMVectorSet(-LightPosition.x, -LightPosition.y, -LightPosition.z, 0.0f);
	LightDirection = XMVector3Normalize(LightDirection);
	XMStoreFloat4(&light.Direction, LightDirection);

	LightPropertiesConstantBuffer lightProperties;
	lightProperties.EyePosition = LightPosition;
	lightProperties.Lights[0] = light;
	g_pImmediateContext->UpdateSubresource(g_pLightConstantBuffer, 0, nullptr, &lightProperties, 0, 0);
}

void Update()
{
	g_pCurrentCamera->SetPosition(XMFLOAT3(currentPosX - sin(rotationX), currentPosY - sin(rotationY), currentPosZ - cos(rotationX)));
	g_pCurrentCamera->SetLookAt(XMFLOAT3(currentPosX, currentPosY, currentPosZ));
	g_pCurrentCamera->SetView();

	g_pCurrentCamera->SetView();
	g_pCurrentCamera->SetProjection();
	g_pCurrentCamera->SetProjectionView();

	XMMATRIX object;
	object = (XMMatrixRotationX(terrainRot.x) * XMMatrixRotationY(terrainRot.y) * XMMatrixRotationZ(terrainRot.z)) * XMMatrixTranslation(terrainPos.x, terrainPos.y, terrainPos.z);


	XMStoreFloat4x4(&g_Terrian, object);
}

void KeyboardInput()
{
	// Wire Frame Input

	if (isWireFrame == true)
	{
		g_pImmediateContext->RSSetState(g_pWireFrame);
	}
	if (isWireFrame == false)
	{
		g_pImmediateContext->RSSetState(nullptr);
	}

	// Camera Input
	float mCameraSpeed = 0.000008f;

	// Forward
	if (GetAsyncKeyState('W'))
	{
		currentPosZ += mCameraSpeed * cos(rotationX);
		currentPosX += mCameraSpeed * sin(rotationX);
		currentPosY += mCameraSpeed * sin(rotationY);
	}
	// Backwards
	if (GetAsyncKeyState('S'))
	{
		currentPosZ -= mCameraSpeed * cos(rotationX);
		currentPosX -= mCameraSpeed * sin(rotationX);
		currentPosY += mCameraSpeed * sin(rotationY);
	}

	// Right
	if (GetAsyncKeyState('D'))
	{
		rotationX += mCameraSpeed;
	}
	// Left
	if (GetAsyncKeyState('A'))
	{
		rotationX -= mCameraSpeed;
	}

	// Up
	if (GetAsyncKeyState('E'))
	{
		rotationY += mCameraSpeed * cos(rotationY);
	}
	// Down
	if (GetAsyncKeyState('Q'))
	{
		rotationY -= mCameraSpeed * cos(rotationY);
	}

	if (rotationY > 1.5f)
	{
		rotationY = 1.5f;
	}
	if (rotationY < -1.5f)
	{
		rotationY = -1.5f;
	}

}

float calculateDeltaTime()
{
	// Update our time
	static float deltaTime = 0.0f;
	static ULONGLONG timeStart = 0;
	ULONGLONG timeCur = GetTickCount64();
	if (timeStart == 0)
		timeStart = timeCur;
	deltaTime = (timeCur - timeStart) / 1000.0f;
	timeStart = timeCur;

	float FPS60 = 1.0f / 60.0f;
	static float cummulativeTime = 0;

	// cap the framerate at 60 fps 
	cummulativeTime += deltaTime;
	if (cummulativeTime >= FPS60) {
		cummulativeTime = cummulativeTime - FPS60;
	}
	else {
		return 0;
	}

	return deltaTime;
}

void ImGuiRender()
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	ImGui::Begin("Debug Window");

	ImGui::SetWindowSize(ImVec2(500.0f, 500.0f));

	if (ImGui::CollapsingHeader("Camera"))
	{
		ImGui::Text("Camera Position");
		ImGui::DragFloat("Camera Pos X", &currentPosX, 0.05f);
		ImGui::DragFloat("Camera Pos Y", &currentPosY, 0.05f);
		ImGui::DragFloat("Camera Pos Z", &currentPosZ, 0.05f);
		ImGui::Text("Camera Rotation");
		ImGui::DragFloat("Rotate on the X Axis", &rotationX, 0.05f);
		ImGui::DragFloat("Rotate on the Y Axis", &rotationY, 0.05f);
	}
	if (ImGui::CollapsingHeader("Light"))
	{
		ImGui::Text("Light Position");
		ImGui::DragFloat("Light Pos X", &LightPosition.x, 0.05f);
		ImGui::DragFloat("Light Pos Y", &LightPosition.y, 0.05f);
		ImGui::DragFloat("Light Pos Z", &LightPosition.z, 0.05f);
	}
	if (ImGui::CollapsingHeader("Util"))
	{
		ImGui::Checkbox("Wire Frame Mode", &isWireFrame);
	}
	if (ImGui::CollapsingHeader("Terrain"))
	{

		ImGui::DragFloat3("Terain Position", &terrainPos.x);
		ImGui::DragFloat3("Terain Rotation", &terrainRot.x, 0.01f);
		ImGui::DragFloat("Terrain Height", &terrainHeight);
		ImGui::DragFloat("Terrain Bias", &terrainBias);

		if (ImGui::CollapsingHeader("Static Terrain"))
		{
			if (ImGui::Button("Grid"))
			{
				terrainID = 0;
			}
			if (ImGui::Button("Displacement Map"))
			{
				terrainID = 1;
			}
		}
		if (ImGui::CollapsingHeader("Procedural Terrain"))
		{
			ImGui::DragInt("Terrain Seed: ", &randomSeed);
			ImGui::DragInt("Roughness", &imGUIRoughness, 1.0f, 1.0f, 1000.0f);

			ImGui::Checkbox("Diamond Square Algorithm", &diamondSquare);
			if (ImGui::Button("Generate Diamond Square") && diamondSquare == true && faultFormation == false)
			{
				roughness = imGUIRoughness;
				CreateTerrainDiamondSquare();
			}
			ImGui::Checkbox("Fault Formation Algorithm", &faultFormation);
			ImGui::DragInt("Fault Iterations", &faultIterations);
			if (ImGui::Button("Generate Fault Formation") && faultFormation == true && diamondSquare == false)
			{
				roughness = imGUIRoughness;
				CreateTerrainFaultFormation();
			}

		}
	}
	ImGui::End();
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}


//--------------------------------------------------------------------------------------
// Render a frame
//--------------------------------------------------------------------------------------
void Render()
{
	float t = calculateDeltaTime(); // capped at 60 fps
	if (t == 0.0f)
		return;

	// Clear the back buffer
	g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, Colors::MidnightBlue);
	g_pImmediateContext->ClearDepthStencilView(g_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;

	ConstantBuffer cb1;
	XMMATRIX myTerrain = XMLoadFloat4x4(&g_Terrian);
	cb1.mWorld = XMMatrixTranspose(myTerrain);
	cb1.mView = XMMatrixTranspose(XMLoadFloat4x4(g_pCurrentCamera->GetView()));
	cb1.mProjection = XMMatrixTranspose(XMLoadFloat4x4(g_pCurrentCamera->GetProjection()));
	cb1.terrainID = terrainID;
	cb1.terrainHeight = terrainHeight;
	cb1.terrainBias = terrainBias;
	cb1.vOutputColor = XMFLOAT4(1, 1, 1, 0);

	g_pImmediateContext->UpdateSubresource(g_pConstantBuffer, 0, nullptr, &cb1, 0, 0);
	g_pImmediateContext->UpdateSubresource(g_pTerrainMaterialBuffer, 0, nullptr, &m_material, 0, 0);

	setupLightForRender();





	if (diamondSquare == true)
	{
		g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pGridDSVertexBuffer, &stride, &offset);
		g_pImmediateContext->IASetIndexBuffer(g_pGridDSIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
	}
	else if (faultFormation == true)
	{
		g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pGridFFVertexBuffer, &stride, &offset);
		g_pImmediateContext->IASetIndexBuffer(g_pGridFFIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
	}
	else
	{
		g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pGridVertexBuffer, &stride, &offset);
		g_pImmediateContext->IASetIndexBuffer(g_pGridIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
	}

	g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);

	// Vertex Shader Stage
	g_pImmediateContext->VSSetShader(g_pVertexShader, nullptr, 0);
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantBuffer);
	g_pImmediateContext->VSSetConstantBuffers(1, 1, &g_pTerrainMaterialBuffer);
	g_pImmediateContext->VSSetConstantBuffers(2, 1, &g_pLightConstantBuffer);

	// Hull Shader Stage
	g_pImmediateContext->HSSetShader(g_pHullShader, nullptr, 0);

	// Tessellation (Hidden)

	// Domain Shader Stage
	g_pImmediateContext->DSSetShader(g_pDomainShader, nullptr, 0);
	g_pImmediateContext->DSSetConstantBuffers(0, 1, &g_pConstantBuffer);
	g_pImmediateContext->DSSetSamplers(0, 1, &g_pSamplerState);
	g_pImmediateContext->DSSetShaderResources(3, 1, &g_pDispacementMap);

	// Geometry Shader

	// Pixel Shader Stage
	g_pImmediateContext->PSSetShader(g_pPixelShader, nullptr, 0);
	g_pImmediateContext->PSSetConstantBuffers(1, 1, &g_pTerrainMaterialBuffer);
	g_pImmediateContext->PSSetConstantBuffers(2, 1, &g_pLightConstantBuffer);

	g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTextureGrass);
	g_pImmediateContext->PSSetShaderResources(1, 1, &g_pTextureStone);
	g_pImmediateContext->PSSetShaderResources(2, 1, &g_pTextureSnow);
	g_pImmediateContext->PSSetShaderResources(3, 1, &g_pDispacementMap);
	g_pImmediateContext->PSSetSamplers(0, 1, &g_pSamplerState);

	g_pImmediateContext->DrawIndexed(totalFaces * 3, 0, 0);

	g_pImmediateContext->Flush();

	ImGuiRender();

	g_pSwapChain->Present(0, 0);
}



