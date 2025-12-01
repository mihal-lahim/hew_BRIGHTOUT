#include <d3d11.h>
#include "direct3d.h"
#include "debug_ostream.h"

//#pragma comment(lib, "d3d11.lib")	// 外部シンボルをリンクするための記述(Viでしか使えない)
// #pragma comment(lib, "dxgi.lib")	// DXGIライブラリのリンク（必要に応じて）
#if defined(DEBUG) || defined(_DEBUG)
#pragma comment(lib, "DirectXTex_Debug.lib")	
#else
#pragma comment(lib, "DirectXTex_Release.lib")	
#endif

// インターフェース(3つ)
static ID3D11Device* g_pDevice = nullptr;					// デバイス
static ID3D11DeviceContext* g_pDeviceContext = nullptr;		// コンテキスト
static IDXGISwapChain* g_pSwapChain = nullptr;				// スワップチェーン

static ID3D11BlendState* g_BlendStateMultiply = nullptr;		// ブレンドステート
static ID3D11BlendState* g_BlendStateAdd = nullptr;		// ブレンドステート

static ID3D11DepthStencilState* g_pDepthStencilStateDepthDisable = nullptr;
static ID3D11DepthStencilState* g_pDepthStencilStateDepthEnable = nullptr;

// バックバッファの関連
static ID3D11RenderTargetView* g_pRenderTargetView = nullptr;	// レンダーターゲットビュー
static ID3D11Texture2D* g_pDepthStencilBuffer = nullptr;		// デプスステンシルバッファ
static ID3D11DepthStencilView* g_pDepthStencilView = nullptr;	// デプスステンシルビュー

static D3D11_TEXTURE2D_DESC g_BackBufferDesc{};					// バックバッファ
//static D3D11_VIEWPORT g_Viewport{};							// ビューポート

static D3D11_VIEWPORT g_Viewport[2]{};					// ビューポート

static bool configureBackBuffer();	// バックバッファの設定
static void releaseBackBuffer();	// バックバッファの解放


bool Direct3D_Initialize(HWND hWnd)
{
	//デバイス、スワップチェーン、コンテキスト生成
	DXGI_SWAP_CHAIN_DESC swap_chain_desc{};
	swap_chain_desc.Windowed = TRUE;	// ウィンドウモード
	swap_chain_desc.BufferCount = 2;	// バッファの数
	// swap_chain_desc.BufferDesc.Width = 0;
	// swap_chain_desc.BufferDesc.Heicht = 0;
	// →ウィンドウサイズに合わせて自動的に設定される
	swap_chain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;		// バッファのフォーマット
	swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;		// バッファの使用方法
	swap_chain_desc.SampleDesc.Count = 1;								// マルチサンプリングの数
	swap_chain_desc.SampleDesc.Quality = 0;								// マルチサンプリングの品質
	//swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;	// スワップチェーンの効果 ←これをコメントアウトするとFPSが上がるvoid Direct3D_Present()を(1.0)にして //ベンチマーク固定をする
	swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_SEQUENTIAL;
	swap_chain_desc.OutputWindow = hWnd;								// ウィンドウハンドル

	UINT device_flags = 0;
#if defined (DEBUG) || defined(_DEBUG)
	device_flags |= D3D11_CREATE_DEVICE_DEBUG;	// デバッグ用フラグ
#endif
	D3D_FEATURE_LEVEL levels[] = {
		D3D_FEATURE_LEVEL_11_1,//ハードウェアのレベルによって変わるので判断する
		D3D_FEATURE_LEVEL_11_0
	};

	D3D_FEATURE_LEVEL feature_level = D3D_FEATURE_LEVEL_11_0;

	HRESULT hr = D3D11CreateDeviceAndSwapChain(
		nullptr,						// アダプタ
		D3D_DRIVER_TYPE_HARDWARE,		// ドライバの種類
		nullptr,						// ソフトウェアドライバ
		device_flags,					// フラグ
		levels,							// 機能レベル
		ARRAYSIZE(levels),				// 機能レベルの数
		D3D11_SDK_VERSION,				// SDKバージョン
		&swap_chain_desc,				// スワップチェーンの設定
		&g_pSwapChain,					// スワップチェーン
		&g_pDevice,						// デバイス
		&feature_level,					// 機能レベル
		&g_pDeviceContext				// コンテキスト
	);

	if (FAILED(hr))
	{
		MessageBox(hWnd, "Direct3Dの初期化に失敗しました", "エラー", MB_OK);
		return false;
	}

	configureBackBuffer();
	if (!configureBackBuffer())
	{
		MessageBox(hWnd, "バックバッファの設定に失敗しました", "エラー", MB_OK);
		return false;
	}

	return true;
}

void Direct3D_Finalize()
{
	SAFE_RELEASE(g_pDepthStencilStateDepthEnable);
	SAFE_RELEASE(g_pDepthStencilStateDepthDisable);
	SAFE_RELEASE(g_BlendStateMultiply);
	SAFE_RELEASE(g_BlendStateAdd);

	releaseBackBuffer();

	SAFE_RELEASE(g_pSwapChain);
	SAFE_RELEASE(g_pDeviceContext);
	SAFE_RELEASE(g_pDevice);
}

void Direct3D_Clear()
{
	float clear_color[4] = { 0.1f, 0.2f, 0.4f, 1.0f }; // RGBA 背景の色
	g_pDeviceContext->ClearRenderTargetView(g_pRenderTargetView, clear_color);//クリアしてくださいと命令している
	g_pDeviceContext->ClearDepthStencilView(g_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);//この色でクリアしてくださいと命令している　 1.0f深さ、0にすると描画されない
	//g_pDeviceContextはコマンド どこかにため続けている

	//レンダーターゲットビューとデプスステンシルビューの設定
	g_pDeviceContext->OMSetRenderTargets(1, &g_pRenderTargetView, g_pDepthStencilView);
}

void Direct3D_Present()
{
	// スワップチェーンの表示
	g_pSwapChain->Present(1, 0);
}

unsigned int Direct3D_GetBackBufferWidth()
{
	return g_BackBufferDesc.Width;
}

unsigned int Direct3D_GetBackBufferHeight()
{
	return g_BackBufferDesc.Height;
}

ID3D11Device* Direct3D_GetDevice()
{
	return g_pDevice;
}

ID3D11DeviceContext* Direct3D_GetContext()
{
	return g_pDeviceContext;
}

// お遊び用
void SetViewport(int n)
{
	g_pDeviceContext->RSSetViewports(1, &g_Viewport[n]);
}

void Direct3D_SetAlphaBlend(AlphaBlendMode blend)
{
	float blend_factor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	ID3D11BlendState* arg{};

	switch (blend)
	{
	case BLEND_TRANSPARENT:
		arg = g_BlendStateMultiply;
		break;
	case BLEND_ADD:
		arg = g_BlendStateAdd;
		break;
	}

	g_pDeviceContext->OMSetBlendState(arg, blend_factor, 0xffffffff);// 設定
}

void Direct3D_SetDepthTest(bool bEnable)
{
	auto state = bEnable ? g_pDepthStencilStateDepthEnable : g_pDepthStencilStateDepthDisable;

	g_pDeviceContext->OMSetDepthStencilState(state, NULL);// 設定
}

bool configureBackBuffer()
{
	HRESULT hr;

	ID3D11Texture2D* back_buffer_pointer = nullptr;

	// バックバッファの取得
	hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&back_buffer_pointer);

	if (FAILED(hr)) {
		hal::dout << "バックバッファの取得に失敗しました" << std::endl;
		return false;
	}

	// バックバッファのレンダーターゲットビューの生成
	hr = g_pDevice->CreateRenderTargetView(back_buffer_pointer, nullptr, &g_pRenderTargetView);

	if (FAILED(hr)) {
		back_buffer_pointer->Release();
		hal::dout << "バックバッファのレンダーターゲットビューの生成に失敗しました" << std::endl;
		return false;
	}

	// バックバッファの状態（情報）を取得
	back_buffer_pointer->GetDesc(&g_BackBufferDesc);

	back_buffer_pointer->Release(); // バックバッファのポインタは不要なので解放

	// デプスステンシルバッファの生成
	D3D11_TEXTURE2D_DESC depth_stencil_desc{};
	depth_stencil_desc.Width = g_BackBufferDesc.Width;
	depth_stencil_desc.Height = g_BackBufferDesc.Height;
	depth_stencil_desc.MipLevels = 1;
	depth_stencil_desc.ArraySize = 1;
	depth_stencil_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depth_stencil_desc.SampleDesc.Count = 1;
	depth_stencil_desc.SampleDesc.Quality = 0;
	depth_stencil_desc.Usage = D3D11_USAGE_DEFAULT;
	depth_stencil_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depth_stencil_desc.CPUAccessFlags = 0;
	depth_stencil_desc.MiscFlags = 0;
	hr = g_pDevice->CreateTexture2D(&depth_stencil_desc, nullptr, &g_pDepthStencilBuffer);

	if (FAILED(hr)) {
		hal::dout << "デプスステンシルバッファの生成に失敗しました" << std::endl;
		return false;
	}

	// デプスステンシルビューの生成
	D3D11_DEPTH_STENCIL_VIEW_DESC depth_stencil_view_desc{};
	depth_stencil_view_desc.Format = depth_stencil_desc.Format;
	depth_stencil_view_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depth_stencil_view_desc.Texture2D.MipSlice = 0;
	depth_stencil_view_desc.Flags = 0;
	hr = g_pDevice->CreateDepthStencilView(g_pDepthStencilBuffer, &depth_stencil_view_desc, &g_pDepthStencilView);

	if (FAILED(hr)) {
		hal::dout << "デプスステンシルビューの生成に失敗しました" << std::endl;
		return false;
	}

	// ビューポートの設定
	g_Viewport[0].TopLeftX = 0.0f;
	g_Viewport[0].TopLeftY = 0.0f;
	g_Viewport[0].Width = (FLOAT)g_BackBufferDesc.Width;
	g_Viewport[0].Height = (FLOAT)g_BackBufferDesc.Height;
	g_Viewport[0].MinDepth = 0.0f;
	g_Viewport[0].MaxDepth = 1.0f;


	////お遊び用(左側と右側で分ける)
	//g_Viewport[0].TopLeftX = 0.0f;
	//g_Viewport[0].TopLeftY = 0.0f;
	//g_Viewport[0].Width = (FLOAT)g_BackBufferDesc.Width * 0.5;
	//g_Viewport[0].Height = (FLOAT)g_BackBufferDesc.Height;
	//g_Viewport[0].MinDepth = 0.0f;
	//g_Viewport[0].MaxDepth = 1.0f;

	//g_Viewport[1].TopLeftX = (FLOAT)g_BackBufferDesc.Width * 0.5;
	//g_Viewport[1].TopLeftY = 0.0f;
	//g_Viewport[1].Width = (FLOAT)g_BackBufferDesc.Width * 0.5;
	//g_Viewport[1].Height = (FLOAT)g_BackBufferDesc.Height;
	//g_Viewport[1].MinDepth = 0.0f;
	//g_Viewport[1].MaxDepth = 1.0f;

	g_pDeviceContext->RSSetViewports(1, &g_Viewport[0]);

	// ブレンドステート設定
	D3D11_BLEND_DESC bd = {};
	bd.AlphaToCoverageEnable = FALSE;
	bd.IndependentBlendEnable = FALSE;
	bd.RenderTarget[0].BlendEnable = TRUE;
	bd.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	bd.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	bd.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	bd.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	bd.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	bd.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	bd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	g_pDevice->CreateBlendState(&bd, &g_BlendStateMultiply);// 作成

	// 加算合成用の作成
	bd.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	g_pDevice->CreateBlendState(&bd, &g_BlendStateAdd);// 作成

	Direct3D_SetAlphaBlend(BLEND_TRANSPARENT);// デフォルト

	float blend_factor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	g_pDeviceContext->OMSetBlendState(g_BlendStateMultiply, blend_factor, 0xffffffff);// 設定

	//深度ステンシルステート設定(奥行き情報)
	D3D11_DEPTH_STENCIL_DESC dsd = {};
	dsd.DepthFunc = D3D11_COMPARISON_LESS;
	dsd.StencilEnable = FALSE;
	//dsd.DepthEnable = FALSE;	//無効にする
	//dsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;

	//g_pDevice->CreateDepthStencilState(&dsd, &g_pDepthStencilStateDepthDisable);

	dsd.DepthEnable = TRUE;
	dsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;

	g_pDevice->CreateDepthStencilState(&dsd, &g_pDepthStencilStateDepthDisable);

	g_pDeviceContext->OMSetDepthStencilState(g_pDepthStencilStateDepthDisable, NULL);


	return true;
}

void releaseBackBuffer()
{
	if (g_pRenderTargetView) {
		g_pRenderTargetView->Release();
		g_pRenderTargetView = nullptr;
	}

	if (g_pDepthStencilBuffer) {
		g_pDepthStencilBuffer->Release();
		g_pDepthStencilBuffer = nullptr;
	}

	if (g_pDepthStencilView) {
		g_pDepthStencilView->Release();
		g_pDepthStencilView = nullptr;
	}
}

