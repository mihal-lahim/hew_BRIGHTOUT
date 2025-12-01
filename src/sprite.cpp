/*==============================================================================

   スプライト描画 [Sprite.cpp]
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
#include "sprite.h"
#include "texture.h"
#include "debug_ostream.h"


static constexpr int NUM_VERTEX = 4; // 頂点数


static ID3D11Buffer* g_pVertexBuffer = nullptr; // 頂点バッファ
//static ID3D11ShaderResourceView* g_pTexture = nullptr;//2枚3枚と増やすときにはこれも増やして名前を変える(1個につき1枚)


// 注意！初期化で外部から設定されるもの。Release不要。
static ID3D11Device* g_pDevice = nullptr;
static ID3D11DeviceContext* g_pContext = nullptr;

// 頂点構造体
struct Vertex
{
	XMFLOAT3 position; // 頂点座標
	XMFLOAT4 color;    // 頂点色
	XMFLOAT2 texcoord; // テクスチャ座標
};


void Sprite_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
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
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(Vertex) * NUM_VERTEX;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	g_pDevice->CreateBuffer(&bd, NULL, &g_pVertexBuffer);
}

void Sprite_Finalize(void)
{
	//SAFE_RELEASE(g_pTexture);
	SAFE_RELEASE(g_pVertexBuffer);
}

void Sprite_Draw(int texid, float x, float y, float w,float h, int tx, int ty, int tw, int th,float angle,DirectX::XMFLOAT4 color)
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

	constexpr float W = 512.0f;
	constexpr float H = 512.0f;

	// 画面の左上から右下に向かう線分を描画する
	v[0].position = { -0.5f, -0.5f, 0.0f };
	v[1].position = { +0.5f, -0.5f, 0.0f };
	v[2].position = { -0.5f, +0.5f, 0.0f };
	v[3].position = { +0.5f, +0.5f, 0.0f };

	// テクスチャ座標の設定
	float tsw = (float)Texture_GetWidth(texid);
	float tsh = (float)Texture_GetHeight(texid);

	//色		　{R,G,B,A}
	v[0].color = color;
	v[1].color = color;
	v[2].color = color;
	v[3].color = color;


	float u0 = tx / tsw;
	float v0 = ty / tsh;
	float u1 = (tx + tw) / tsw;
	float v1 = (ty + th) / tsh;

	v[0].texcoord = { u0 , v0 };
	v[1].texcoord = { u1 , v0 };
	v[2].texcoord = { u0 , v1 };
	v[3].texcoord = { u1 , v1 };

	float pay = 3.14;
	// 頂点バッファのロックを解除
	g_pContext->Unmap(g_pVertexBuffer, 0);

	// 頂点バッファを描画パイプラインに設定
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	g_pContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);

	// 頂点シェーダーにWorld変換行列を設定
	XMMATRIX s = XMMatrixScaling(w, h, 1.0f);
	XMMATRIX r = XMMatrixRotationZ(angle);//XMMATRIX←演算をする場合
	XMMATRIX t = XMMatrixTranslation(x,y,0.0f);
	Shader_SetWorldMatrix(s * r * t);//合成

	// 頂点シェーダーにプロジェクション変換行列を設定
	Shader_SetProjectionMatrix(XMMatrixOrthographicOffCenterLH(0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 0.0f, 1.0f));

	Shader_SetColor(color);

	// 画面の中央が(0,0)の設定
	//Shader_SetMatrix(XMMatrixOrthographicOffCenterLH(-SCREEN_WIDTH * 0.5, SCREEN_WIDTH * 0.5, SCREEN_HEIGHT * 0.5, -SCREEN_HEIGHT * 0.5, 0.0f, 1.0f));

	 //プリミティブトポロジ設定
	g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	Texture_SetTexture(texid);

	//四角形
	g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// ポリゴン描画命令発行
	g_pContext->Draw(NUM_VERTEX, 0);

}

void Sprite_Draw(int texid, float x, float y, float w, float h, int tx, int ty, int tw, int th, DirectX::XMFLOAT4 color)
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


	v[0].position = { x,     y,     0.0f };
	v[1].position = { w + x, y,     0.0f };
	v[2].position = { x,     h + y, 0.0f };
	v[3].position = { w + x, h + y, 0.0f };

	// テクスチャ座標の設定
	float tsw = (float)Texture_GetWidth(texid);
	float tsh = (float)Texture_GetHeight(texid);

	//色		　{R,G,B,A}
	v[0].color = color;
	v[1].color = color;
	v[2].color = color;
	v[3].color = color;


	float u0 = tx / tsw;
	float v0 = ty / tsh;
	float u1 = (tx + tw) / tsw;
	float v1 = (ty + th) / tsh;

	v[0].texcoord = { u0 , v0 };
	v[1].texcoord = { u1 , v0 };
	v[2].texcoord = { u0 , v1 };
	v[3].texcoord = { u1 , v1 };

	// 頂点バッファのロックを解除
	g_pContext->Unmap(g_pVertexBuffer, 0);

	// 頂点バッファを描画パイプラインに設定
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	g_pContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);

	// 頂点シェーダーにWorld変換行列を設定
	Shader_SetWorldMatrix(XMMatrixIdentity());

	// 頂点シェーダーにプロジェクション変換行列を設定
	Shader_SetProjectionMatrix(XMMatrixOrthographicOffCenterLH(0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 0.0f, 1.0f));

	Shader_SetColor(color);

	// 画面の中央が(0,0)の設定
	//Shader_SetMatrix(XMMatrixOrthographicOffCenterLH(-SCREEN_WIDTH * 0.5, SCREEN_WIDTH * 0.5, SCREEN_HEIGHT * 0.5, -SCREEN_HEIGHT * 0.5, 0.0f, 1.0f));

	 //プリミティブトポロジ設定
	g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	Texture_SetTexture(texid);

	//三角形の内側を塗りつぶす
	//g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//四角形
	g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// ポリゴン描画命令発行
	g_pContext->Draw(NUM_VERTEX, 0);

}

//簡易版　アングル無し
void Sprite_Draw(int texid, float x, float y,float zoom,XMFLOAT4 color)
{
	// シェーダーを描画パイプラインに設定
	Shader_Begin();

	// 頂点バッファをロックする GPUがはいれなくなる
	D3D11_MAPPED_SUBRESOURCE msr;
	g_pContext->Map(g_pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);

	// 頂点バッファへの仮想ポインタを取得
	Vertex* v = (Vertex*)msr.pData;

	// 頂点情報を書き込み
	const float SCREEN_WIDTH = (float)Direct3D_GetBackBufferWidth();
	const float SCREEN_HEIGHT = (float)Direct3D_GetBackBufferHeight();

	//テクスチャ全体の幅と高さ
	float tsw = (float)Texture_GetWidth(texid);
	float tsh = (float)Texture_GetHeight(texid);

	//正方形の座標
	v[0].position = { x,       y,        0.0f }; // 左下
	v[1].position = { (x + tsw) * zoom, y,        0.0f }; // 右下
	v[2].position = { x,       (y + tsh) * zoom, 0.0f };;// 左上
	v[3].position = { (x + tsw) * zoom, (y + tsh) * zoom, 0.0f }; // 右下

	//頂点の色
	v[0].color = color; // 白
	v[1].color = color; // 白
	v[2].color = color; // 白
	v[3].color = color; // 白

	//テクスチャの範囲
	v[0].texcoord = { 0.0f, 0.0f };
	v[1].texcoord = { 1.0f, 0.0f };
	v[2].texcoord = { 0.0f, 1.0f };
	v[3].texcoord = { 1.0f, 1.0f };

	// 頂点バッファのロックを解除 GPUが動けるようにする
	g_pContext->Unmap(g_pVertexBuffer, 0);

	// 頂点バッファを描画パイプラインに設定
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	g_pContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);


	//頂点シェーダーにworld変換行列を設定　単位行列　
	Shader_SetWorldMatrix(XMMatrixIdentity());

	// 頂点シェーダーに変換行列を設定　一番上下左右の座標を決める 座標系をつくる
	Shader_SetProjectionMatrix(XMMatrixOrthographicOffCenterLH(
		0.0f,    //左
		SCREEN_WIDTH,    //右
		SCREEN_HEIGHT,    //下
		0.0f, //上
		0.0f,
		1.0f));

	//色とアルファ値（透明度）を設定
	Shader_SetColor(color);

	// プリミティブトポロジ設定  ストリップ化
	g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	//テクスチャを設定
	Texture_SetTexture(texid);

	// スプライト描画命令発行
	g_pContext->Draw(NUM_VERTEX, 0);
}



//void Sprite_Draw(int texid, float x, float y, float w, float h, int tx, int ty, int tw, int th,float bairitu, DirectX::XMFLOAT4 color)
//{
//	// シェーダーを描画パイプラインに設定
//	Shader_Begin();
//
//	// 頂点バッファをロックする
//	D3D11_MAPPED_SUBRESOURCE msr;
//	g_pContext->Map(g_pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
//
//	// 頂点バッファへの仮想ポインタを取得
//	Vertex* v = (Vertex*)msr.pData;
//
//	// 頂点情報を書き込み
//	const float SCREEN_WIDTH = (float)Direct3D_GetBackBufferWidth();
//	const float SCREEN_HEIGHT = (float)Direct3D_GetBackBufferHeight();
//
//
//	//正方形の座標
//	v[0].position = { x,            y,        0.0f };	// 左下
//	v[1].position = { x  * bairitu, y,   0.0f };		// 右下
//	v[2].position = { x,            y * bairitu, 0.0f };// 左上
//	v[3].position = { x  * bairitu, y * bairitu, 0.0f };// 右下
//
//	// テクスチャ座標の設定
//	float tsw = (float)Texture_GetWidth(texid);
//	float tsh = (float)Texture_GetHeight(texid);
//
//	//色		　{R,G,B,A}
//	v[0].color = color;
//	v[1].color = color;
//	v[2].color = color;
//	v[3].color = color;
//
//
//	float u0 = tx / tsw;
//	float v0 = ty / tsh;
//	float u1 = (tx + tw) / tsw;
//	float v1 = (ty + th) / tsh;
//
//	v[0].texcoord = { u0 , v0 };
//	v[1].texcoord = { u1 , v0 };
//	v[2].texcoord = { u0 , v1 };
//	v[3].texcoord = { u1 , v1 };
//
//	// 頂点バッファのロックを解除
//	g_pContext->Unmap(g_pVertexBuffer, 0);
//
//	// 頂点バッファを描画パイプラインに設定
//	UINT stride = sizeof(Vertex);
//	UINT offset = 0;
//	g_pContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);
//
//	// 頂点シェーダーにWorld変換行列を設定
//	Shader_SetWorldMatrix(XMMatrixIdentity());
//
//	// 頂点シェーダーにプロジェクション変換行列を設定
//	Shader_SetProjectionMatrix(XMMatrixOrthographicOffCenterLH(0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 0.0f, 1.0f));
//
//	Shader_SetColor(color);
//
//	// 画面の中央が(0,0)の設定
//	//Shader_SetMatrix(XMMatrixOrthographicOffCenterLH(-SCREEN_WIDTH * 0.5, SCREEN_WIDTH * 0.5, SCREEN_HEIGHT * 0.5, -SCREEN_HEIGHT * 0.5, 0.0f, 1.0f));
//
//	 //プリミティブトポロジ設定
//	g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
//
//	Texture_SetTexture(texid);
//
//	//三角形の内側を塗りつぶす
//	//g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//
//	//四角形
//	g_pContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
//
//	// ポリゴン描画命令発行
//	g_pContext->Draw(NUM_VERTEX, 0);
//
//}



