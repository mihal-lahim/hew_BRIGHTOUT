/*==============================================================================

   グリッドの描画 [grid.cpp]
														 Author : sumi rintarou
														 Date   : 2025/10/21
--------------------------------------------------------------------------------

==============================================================================*/
#include "grid.h"
#include <d3d11.h>
#include "direct3d.h"
#include <DirectXMath.h>
using namespace DirectX;
#include "debug_ostream.h"
#include "shader3d.h"
#include "texture.h"

static ID3D11Buffer* g_pVertexBuffer = nullptr; // 頂点バッファ
//static ID3D11ShaderResourceView* g_pTexture = nullptr;//2枚3枚と増やすときにはこれも増やして名前を変える(1個につき1枚)

// 注意！初期化で外部から設定されるもの。Release不要。
static ID3D11Device* g_pDevice = nullptr;
static ID3D11DeviceContext* g_pContext = nullptr;

// 頂点数
static int g_Num_Vertex = 0; // 線分リストの頂点数

// 頂点構造体
struct Vertex3d
{
	XMFLOAT3 position; // 頂点座標
	XMFLOAT4 color;    // 色
};

static int g_texId = -1;


void Grid_Initialize(int x_count, int z_count, float size)
{
	// デバイスとデバイスコンテキストの保存
	g_pDevice = Direct3D_GetDevice();
	g_pContext = Direct3D_GetContext();

	// 頂点数を算出する
	const int X_V_COUNT = x_count + 1; // X方向の頂点数
	const int Z_V_COUNT = z_count + 1; // Z方向の頂点数
	g_Num_Vertex = (X_V_COUNT + Z_V_COUNT) * 2; // 線分リストの頂点数

	// 頂点バッファ生成
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(Vertex3d) * g_Num_Vertex;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;// アクセスフラグを0にすることでCPUにアクセスできなくする

	auto* pV = new Vertex3d[g_Num_Vertex];

	// グリッド線の頂点情報の書き込み
	float start_x = x_count * size * -0.5f;
	float end_x = x_count * size * 0.5f;
	float start_z = z_count * size * -0.5f;
	float end_z = z_count * size * 0.5f;
	int index = 0;

	for (int x = 0; x < X_V_COUNT; x++)
	{
		XMFLOAT4 color = x == X_V_COUNT / 2 ? XMFLOAT4{ 0.0f,0.0f,1.0f,1.0f } : XMFLOAT4{ 0.0f,1.0f,0.0f,1.0f };
		pV[index].position = { start_x + size * x,0.0f,start_z };
		pV[index].color = color;
		pV[index + 1].position = { start_x + size * x,0.0f,end_z };
		pV[index + 1].color = color;
		index += 2;
	}

	for (int z = 0; z < Z_V_COUNT; z++)
	{
		XMFLOAT4 color = z == Z_V_COUNT / 2 ? XMFLOAT4{ 0.0f,0.0f,1.0f,1.0f } : XMFLOAT4{ 0.0f,1.0f,0.0f,1.0f };
		pV[index].position = { start_x,0.0f,start_z + size * z };
		pV[index].color = color;
		pV[index + 1].position = { end_x,0.0f,start_z + size * z };
		pV[index + 1].color = color;
		index += 2;
	}


	// 頂点バッファへ流し込むデータの設定
	D3D11_SUBRESOURCE_DATA sd{};
	sd.pSysMem = pV;

	g_pDevice->CreateBuffer(&bd, &sd, &g_pVertexBuffer);

	delete[] pV;

	g_texId = Texture_Load(L"texture/siro.png");
}

void Grid_Finalize()
{
	SAFE_RELEASE(g_pVertexBuffer);
}

void Grid_Update(double elapsed_time)
{

}

void Grid_Draw()
{
	// シェーダーを描画パイプラインに設定
	Shader3d_Begin();

	// 頂点バッファを描画パイプラインに設定
	UINT stride = sizeof(Vertex3d);
	UINT offset = 0;
	g_pContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);

	// 頂点シェーダーにworld変換行列を設定　単位行列(1を掛けるということで何も値を変えないということ)
	Shader3d_SetWorldMatrix(XMMatrixIdentity());

	// プリミティブトポロジ設定
	g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	// テクスチャの設定(白)
	Texture_SetTexture(g_texId);

	// ポリゴン描画命令発行
	g_pContext->Draw(g_Num_Vertex, 0);
}



///*==============================================================================
//
//   グリッド描画 [grid.cpp]
//                                                         Author : sumi rintarou
//                                                         Date   : 2025/10/21
//--------------------------------------------------------------------------------
//
//==============================================================================*/
//#include "grid.h"
//#include <d3d11.h>
//#include "direct3d.h"
//#include <DirectXMath.h>
//using namespace DirectX;
//#include "debug_ostream.h"
//#include "shader3d.h"
//#include <vector>
//
//// 動的生成に変更: 定義を削除して動的に頂点数を管理
//static ID3D11Buffer* g_pVertexBuffer = nullptr; // 頂点バッファ
//
//// 注意！初期化で外部から設定されるもの。Release不要。
//static ID3D11Device* g_pDevice = nullptr;
//static ID3D11DeviceContext* g_pContext = nullptr;
//
//// 頂点構造体
//struct Vertex3d
//{
//    XMFLOAT3 position; // 頂点座標
//    XMFLOAT4 color;    // 色
//};
//
//// 動的に作成される頂点数を保持
//static UINT g_NumVertex = 0;
//
//void Grid_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
//{
//    // パラメータ: グリッドのセル数（1辺あたりのマス数）と 1 マスの大きさ
//    constexpr int GRID_CELLS = 10;    // 10x10 のマス目
//    constexpr float CELL_SIZE = 1.0f; // 1 マスあたりの長さ（必要に応じて変更）
//
//    // デバイスとデバイスコンテキストのチェック
//    if (!pDevice || !pContext) {
//        hal::dout << "Grid_Initialize() : 与えられたデバイスかコンテキストが不正です" << std::endl;
//        return;
//    }
//
//    // デバイスとコンテキストを保存
//    g_pDevice = pDevice;
//    g_pContext = pContext;
//
//    // 中央を原点にするためのオフセット
//    float half = GRID_CELLS * CELL_SIZE * 0.5f;
//
//    // 頂点リストを作成 (各方向に GRID_CELLS+1 本の線、1 本あたり 2 頂点)
//    std::vector<Vertex3d> vertices;
//    vertices.reserve((GRID_CELLS + 1) * 4);
//
//    XMFLOAT4 lineColor = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f); // グリッド色（灰）
//
//    // Z 軸方向に並ぶ線（X 軸に沿う線：異なる Z の値で X を伸ばす）
//    for (int i = 0; i <= GRID_CELLS; ++i) {
//        float z = -half + i * CELL_SIZE;
//        vertices.push_back({ XMFLOAT3(-half, 0.0f, z), lineColor });
//        vertices.push_back({ XMFLOAT3(half, 0.0f, z), lineColor });
//    }
//
//    // X 軸方向に並ぶ線（Z 軸に沿う線：異なる X の値で Z を伸ばす）
//    for (int i = 0; i <= GRID_CELLS; ++i) {
//        float x = -half + i * CELL_SIZE;
//        vertices.push_back({ XMFLOAT3(x, 0.0f, -half), lineColor });
//        vertices.push_back({ XMFLOAT3(x, 0.0f,  half), lineColor });
//    }
//
//    // 頂点バッファ作成
//    g_NumVertex = static_cast<UINT>(vertices.size());
//
//    D3D11_BUFFER_DESC bd = {};
//    bd.Usage = D3D11_USAGE_DEFAULT;
//    bd.ByteWidth = sizeof(Vertex3d) * g_NumVertex;
//    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
//    bd.CPUAccessFlags = 0;
//
//    D3D11_SUBRESOURCE_DATA sd{};
//    sd.pSysMem = vertices.data();
//
//    if (FAILED(g_pDevice->CreateBuffer(&bd, &sd, &g_pVertexBuffer))) {
//        hal::dout << "Grid_Initialize() : 頂点バッファ作成に失敗しました" << std::endl;
//        g_pVertexBuffer = nullptr;
//        g_NumVertex = 0;
//    }
//}
//
//void Grid_Finalize()
//{
//    SAFE_RELEASE(g_pVertexBuffer);
//    g_NumVertex = 0;
//}
//
//void Grid_Update(double elapsed_time)
//{
//}
//
//void Grid_Draw()
//{
//    if (!g_pVertexBuffer || g_NumVertex == 0) return;
//
//    // シェーダーを描画パイプラインに設定
//    Shader3d_Begin();
//
//    // 頂点バッファを描画パイプラインに設定
//    UINT stride = sizeof(Vertex3d);
//    UINT offset = 0;
//    g_pContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);
//
//    // ワールド行列（単位行列）
//    Shader3d_SetWorldMatrix(XMMatrixIdentity());
//
//    // ビュー設定（必要に応じて変更）
//    XMVECTOR eye = { 3.0f, 5.0f, -10.0f, 0.0f };
//    XMVECTOR target = { 0.0f, 0.0f, 0.0f, 0.0f };
//    XMVECTOR up = { 0.0f, 1.0f, 0.0f, 0.0f };
//    XMMATRIX mtxView = XMMatrixLookAtLH(eye, target, up);
//    Shader3d_SetViewMatrix(mtxView);
//
//    // プロジェクション
//    float w = static_cast<float>(Direct3D_GetBackBufferWidth());
//    float h = static_cast<float>(Direct3D_GetBackBufferHeight());
//    XMMATRIX mtxProj = XMMatrixPerspectiveFovLH(1.0f, w / h, 0.1f, 1000.0f);
//    Shader3d_SetProjectionMatrix(mtxProj);
//
//    // 線描画
//    g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
//    g_pContext->Draw(g_NumVertex, 0);
//}
