/*==============================================================================

   ポリゴン描画 [polygon.cpp]
														 Author : Youhei Sato
														 Date   : 2025/05/15
--------------------------------------------------------------------------------

==============================================================================*/
#include <d3d11.h>
#include <DirectXMath.h>
#include "DirectXTex.h"
using namespace DirectX;
#include "direct3d.h"
#include "shader.h"
#include "debug_ostream.h"


static constexpr int NUM_VERTEX = 4; // 頂点数


static ID3D11Buffer* g_pVertexBuffer = nullptr; // 頂点バッファ
//static ID3D11ShaderResourceView* g_pTexture = nullptr;//2枚3枚と増やすときにはこれも増やして名前を変える(1個につき1枚)


// 注意！初期化で外部から設定されるもの。Release不要。
static ID3D11Device* g_pDevice = nullptr;
static ID3D11DeviceContext* g_pContext = nullptr;


//遊び
#define SAIDAI_X	5000.0f
#define SAIDAI_Y	5000.0f
#define ZOUKA_X		5.0f
#define ZOUKA_Y		5.0f
float sizeX = SAIDAI_X;
float sizeY = SAIDAI_Y;
bool hantenX = false;
bool hantenY = false;

// 頂点構造体
struct Vertex
{
	XMFLOAT3 position; // 頂点座標
	XMFLOAT4 color;    // 頂点色
	XMFLOAT2 texcoord; // テクスチャ座標
};


void Polygon_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	// デバイスとデバイスコンテキストのチェック
	if (!pDevice || !pContext) {
		hal::dout << "Polygon_Initialize() : 与えられたデバイスかコンテキストが不正です" << std::endl;
		return;
	}

	// デバイスとデバイスコンテキストの保存
	g_pDevice = pDevice;
	g_pContext = pContext;

	// 頂点バッファ生成
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(Vertex) * NUM_VERTEX;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	g_pDevice->CreateBuffer(&bd, NULL, &g_pVertexBuffer);
}

void Polygon_Finalize(void)
{
	//SAFE_RELEASE(g_pTexture);
	SAFE_RELEASE(g_pVertexBuffer);
}

void Polygon_Draw(void)
{
	// シェーダーを描画パイプラインに設定
	Shader_Begin();

	// 頂点バッファをロックする
	D3D11_MAPPED_SUBRESOURCE msr;
	g_pContext->Map(g_pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);

	// 頂点バッファへの仮想ポインタを取得
	Vertex* v = (Vertex*)msr.pData;

	// 頂点情報を書き込み
	const float SCREEN_WIDTH = (float)Direct3D_GetBackBufferWidth();
	const float SCREEN_HEIGHT = (float)Direct3D_GetBackBufferHeight();


	// コサインなどを使った場合
	static float a = 0.0f;
	a += 0.01f; // 数値を少しずつ増やす
	//float W = 512.0f;// * (cos(a) + 1);	// ポリゴンの幅
	//float H = 512.0f;// * (sin(a) + 1);	// ポリゴンの高さ

	//float X = 64.0f * (cos(a) + 5);		// ポリゴンの幅
	//float Y = 64.0f * (sin(a) + 5);		// ポリゴンの高さ

	// 回転させる
	float W = 512.0f;						// ポリゴンの幅
	float H = 512.0f;						// ポリゴンの高さ

	float X = 64.0f * (cos(a) * 7 + 10);	// ポリゴンの幅
	float Y = 64.0f * (sin(a) * 7 + 3);		// ポリゴンの高さ

	v[0].position = { X,     Y,     0.0f };
	v[1].position = { W + X  , Y,     0.0f };
	v[2].position = { X,     H + Y  , 0.0f };
	v[3].position = { W + X  , H + Y  , 0.0f };


	//増殖
	static float c = 0.0f;
	v[0].texcoord = { 0.0f + c, 0.0f + c };
	v[1].texcoord = { 2.0f + c, 0.0f + c };
	v[2].texcoord = { 0.0f + c, 2.0f + c };
	v[3].texcoord = { 2.0f + c, 2.0f + c };
	c += 0.01f;

	//float pay = 3.14;
	// 頂点バッファのロックを解除
	g_pContext->Unmap(g_pVertexBuffer, 0);

	// 頂点バッファを描画パイプラインに設定
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	g_pContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);

	// 頂点シェーダーにWorld変換行列を設定
	Shader_SetWorldMatrix(XMMatrixIdentity());

	// 頂点シェーダーに変換行列を設定
	Shader_SetProjectionMatrix(XMMatrixOrthographicOffCenterLH(0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 0.0f, 1.0f));
	Shader_SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });

	//四角形
	g_pContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// ポリゴン描画命令発行
	g_pContext->Draw(NUM_VERTEX, 0);
}
