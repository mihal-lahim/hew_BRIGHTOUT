#include <d3d11.h>
#include "direct3d.h"

// インターフェース(3つ)
static ID3D11Device* g_pDevice = nullptr;					// デバイス
static ID3D11DeviceContext* g_pDeviceContext = nullptr;		// コンテキスト
static IDXGISwapChain* g_pSwapChain = nullptr;				// スワップチェーン




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
	swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_SEQUENTIAL;			// スワップチェーンの効果
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

	//if (!configureBackBuffer())
	//{
	//	MessageBox(hWnd, "バックバッファの設定に失敗しました", "エラー", MB_OK);
	//	return false;
	//}
	//


	return false;
}

void Direct3D_Finalize()
{
}
