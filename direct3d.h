#ifndef  DIRECT3D_H
#define DIRECT3D_H

#include <Windows.h>
#include <d3d11.h>


// セーフリリースマクロ
#define SAFE_RELEASE(o) if (o) { (o)->Release(); o = NULL; }

bool Direct3D_Initialize(HWND hWnd); // Direct3Dの初期化
void Direct3D_Finalize(); // Direct3Dの終了処理

void Direct3D_Clear(); // バックバッファのクリア
void Direct3D_Present(); // バックバッファの表示

unsigned int Direct3D_GetBackBufferWidth();		// 画面の高さ
unsigned int Direct3D_GetBackBufferHeight();	// 画面の幅

ID3D11Device* Direct3D_GetDevice();// デバイスの取得
ID3D11DeviceContext* Direct3D_GetContext();// デバイスの取得

void SetViewport(int n);// お遊び用


enum AlphaBlendMode
{
	BLEND_TRANSPARENT,
	BLEND_ADD,
	BLEND_MAX
};

void Direct3D_SetAlphaBlend(AlphaBlendMode blend);

void Direct3D_SetDepthTest(bool bEnable);

#endif //DIRECT3D_H