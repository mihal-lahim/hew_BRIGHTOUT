/*==============================================================================

   キューブの描画 [cube.cpp]
														 Author : sumi rintarou
														 Date   : 2025/10/16
--------------------------------------------------------------------------------

==============================================================================*/
#include "cube.h"
#include <d3d11.h>
#include "direct3d.h"
#include "texture.h"
#include <DirectXMath.h>
using namespace DirectX;
#include "debug_ostream.h"
#include "shader3d.h"

static constexpr int NUM_VERTEX = 4 * 6;	// 頂点数
static constexpr int NUM_INDEX = 36;		// インデックス数

static ID3D11Buffer* g_pVertexBuffer = nullptr; // 頂点バッファ
static ID3D11Buffer* g_pIndexBuffer = nullptr; // インデックスバッファ

// 注意！初期化で外部から設定されるもの。Release不要。
static ID3D11Device* g_pDevice = nullptr;
static ID3D11DeviceContext* g_pContext = nullptr;

// 頂点構造体
struct Vertex3d
{
	XMFLOAT3 position; // 頂点座標
	XMFLOAT3 normal;   // 法線
	XMFLOAT4 color;    // 色
	XMFLOAT2 texcoord; // テクスチャ座標
};

static Vertex3d g_CubeVertex[] = {

	//正面 Z <-
	{{-0.5f, 0.5f,-0.5f}, { 0.0f, 0.0f,-1.0f}, {1.0f,1.0f,1.0f,1.0f},{ 0.0f ,0.0f}},
	{{ 0.5f, 0.5f,-0.5f}, { 0.0f, 0.0f,-1.0f}, {1.0f,1.0f,1.0f,1.0f},{ 1.0f ,0.0f}},
	{{-0.5f,-0.5f,-0.5f}, { 0.0f, 0.0f,-1.0f}, {1.0f,1.0f,1.0f,1.0f},{ 0.0f ,1.0f}},
	{{ 0.5f,-0.5f,-0.5f}, { 0.0f, 0.0f,-1.0f}, {1.0f,1.0f,1.0f,1.0f},{ 1.0f ,1.0f}},

	//右面 X ->
	{{ 0.5f, 0.5f,-0.5f}, { 1.0f, 0.0f, 0.0f}, {1.0f,1.0f,1.0f,1.0f},{ 0.0f ,0.0f}},
	{{ 0.5f, 0.5f, 0.5f}, { 1.0f, 0.0f, 0.0f}, {1.0f,1.0f,1.0f,1.0f},{ 1.0f ,0.0f}},
	{{ 0.5f,-0.5f,-0.5f}, { 1.0f, 0.0f, 0.0f}, {1.0f,1.0f,1.0f,1.0f},{ 0.0f ,1.0f}},
	{{ 0.5f,-0.5f, 0.5f}, { 1.0f, 0.0f, 0.0f}, {1.0f,1.0f,1.0f,1.0f},{ 1.0f ,1.0f}},

	//背面 Z<-
	{{ 0.5f, 0.5f, 0.5f}, { 0.0f, 0.0f, 1.0f}, {1.0f,1.0f,1.0f,1.0f},{ 0.0f ,0.0f}},
	{{-0.5f, 0.5f, 0.5f}, { 0.0f, 0.0f, 1.0f}, {1.0f,1.0f,1.0f,1.0f},{ 1.0f ,0.0f}},
	{{ 0.5f,-0.5f, 0.5f}, { 0.0f, 0.0f, 1.0f}, {1.0f,1.0f,1.0f,1.0f},{ 0.0f ,1.0f}},
	{{-0.5f,-0.5f, 0.5f}, { 0.0f, 0.0f, 1.0f}, {1.0f,1.0f,1.0f,1.0f},{ 1.0f ,1.0f}},

	//左面 X <-
	{{-0.5f, 0.5f, 0.5f}, { -1.0f, 0.0f, 0.0f}, {1.0f,1.0f,1.0f,1.0f},{ 0.0f ,0.0f}},
	{{-0.5f, 0.5f,-0.5f}, { -1.0f, 0.0f, 0.0f}, {1.0f,1.0f,1.0f,1.0f},{ 1.0f ,0.0f}},
	{{-0.5f,-0.5f, 0.5f}, { -1.0f, 0.0f, 0.0f}, {1.0f,1.0f,1.0f,1.0f},{ 0.0f ,1.0f}},
	{{-0.5f,-0.5f,-0.5f}, { -1.0f, 0.0f, 0.0f}, {1.0f,1.0f,1.0f,1.0f},{ 1.0f ,1.0f}},

	//上面 Y ->
	{{-0.5f, 0.5f, 0.5f}, { 0.0f, 1.0f, 0.0f}, {1.0f,1.0f,1.0f,1.0f},{ 0.0f ,0.0f}},
	{{ 0.5f, 0.5f, 0.5f}, { 0.0f, 1.0f, 0.0f}, {1.0f,1.0f,1.0f,1.0f},{ 1.0f ,0.0f}},
	{{-0.5f, 0.5f,-0.5f}, { 0.0f, 1.0f, 0.0f}, {1.0f,1.0f,1.0f,1.0f},{ 0.0f ,1.0f}},
	{{ 0.5f, 0.5f,-0.5f}, { 0.0f, 1.0f, 0.0f}, {1.0f,1.0f,1.0f,1.0f},{ 1.0f ,1.0f}},

	//下面 Y <-
	{{-0.5f,-0.5f,-0.5f}, { 0.0f,-1.0f, 0.0f}, {1.0f,1.0f,1.0f,1.0f},{ 0.0f ,0.0f}},
	{{ 0.5f,-0.5f,-0.5f}, { 0.0f,-1.0f, 0.0f}, {1.0f,1.0f,1.0f,1.0f},{ 1.0f ,0.0f}},
	{{-0.5f,-0.5f, 0.5f}, { 0.0f,-1.0f, 0.0f}, {1.0f,1.0f,1.0f,1.0f},{ 0.0f ,1.0f}},
	{{ 0.5f,-0.5f, 0.5f}, { 0.0f,-1.0f, 0.0f}, {1.0f,1.0f,1.0f,1.0f},{ 1.0f ,1.0f}},
};

static unsigned short g_CubeIndex[] = {
	 0,  1,  2,  1,  3,  2,
	 4,  5,  6,  5,  7,  6,
	 8,  9, 10,  9, 11, 10,
	12, 13, 14, 13, 15, 14,
	16, 17, 18, 17, 19, 18,
	20, 21, 22, 21, 23, 22,
};

void Cube_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	// デバイスとデバイスコンテキストのチェック
	if (!pDevice || !pContext) {
		hal::dout << "Sprite_Initialize() : 与えられたデバイスかコンテキストが不正です" << std::endl;
		return;
	}

	// デバイスとデバイスコンテキストの保存
	g_pDevice = pDevice;
	g_pContext = pContext;

	// 頂点バッファ生成
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(Vertex3d) * NUM_VERTEX;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;// アクセスフラグを0にすることでCPUにアクセスできなくする

	// 頂点バッファへ流し込むデータの設定
	D3D11_SUBRESOURCE_DATA sd{};
	sd.pSysMem = g_CubeVertex;

	g_pDevice->CreateBuffer(&bd, &sd, &g_pVertexBuffer);

	// インデックスバッファ作成
	bd.ByteWidth = sizeof(unsigned short) * NUM_INDEX;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;

	// 頂点バッファへ流し込むデータの設定
	sd.pSysMem = g_CubeIndex;

	g_pDevice->CreateBuffer(&bd, &sd, &g_pIndexBuffer);

}

void Cube_Finalize()
{
	SAFE_RELEASE(g_pIndexBuffer);
	SAFE_RELEASE(g_pVertexBuffer);
}

void Cube_Draw(const int textrue, const XMMATRIX& mtxWorld)
{
	// シェーダーを描画パイプラインに設定
	Shader3d_Begin();

	// 頂点バッファを描画パイプラインに設定
	UINT stride = sizeof(Vertex3d);
	UINT offset = 0;

	// プリミティブトポロジ設定
	g_pContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);

	// 頂点インデックスを描画をパイプラインに設定
	g_pContext->IASetIndexBuffer(g_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	// ワールド変換行列を計算
	Shader3d_SetWorldMatrix(mtxWorld);

	// プリミティブトポロジ設定
	g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//テクスチャの設定
	Texture_SetTexture(textrue);
	

	// マテリアルカラーの設定
	Shader3d_SetMaterialDiffuse({ 1.0f,1.0f,1.0f,1.0f });

	// ポリゴン描画命令発行
	g_pContext->DrawIndexed(NUM_INDEX, 0, 0);
}
