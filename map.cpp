/*==============================================================================

 マップ制御 [map.cpp] - Block を道路・草地・電線・電柱・建物に変更
 Author : sumi rintarou
 Date :2025/11/04
--------------------------------------------------------------------------------

==============================================================================*/
#include "map.h"
#include "cube.h"
#include "texture.h"

using namespace DirectX;

// グローバル Map インスタンス定義
Map g_MapInstance;

void Map::Initialize()
{
	Finalize();

	// 地面用テクスチャ読込
	int groundTex[4] = {
		Texture_Load(L"texture/map_hidariue.png"),
		Texture_Load(L"texture/map_migiue.png"),
		Texture_Load(L"texture/map_hidarishita.png"),
		Texture_Load(L"texture/map_migishita.png")
	};

	// 原点中心に4つのタイル配置
	float tileSize = 25.0f; // タイルサイズを大きくする
	m_GroundTiles.clear();
	m_GroundTiles.reserve(4);

	// 左上
	m_GroundTiles.emplace_back(GroundTile{ {-tileSize * 0.5f, 0.0f, tileSize * 0.5f}, groundTex[0], tileSize, tileSize });
	// 右上
	m_GroundTiles.emplace_back(GroundTile{ {tileSize * 0.5f, 0.0f, tileSize * 0.5f}, groundTex[1], tileSize, tileSize });
	// 左下
	m_GroundTiles.emplace_back(GroundTile{ {-tileSize * 0.5f, 0.0f, -tileSize * 0.5f}, groundTex[2], tileSize, tileSize });
	// 右下
	m_GroundTiles.emplace_back(GroundTile{ {tileSize * 0.5f, 0.0f, -tileSize * 0.5f}, groundTex[3], tileSize, tileSize });
}

void Map::Finalize()
{
	m_GroundTiles.clear();
}

void Map::Draw() const
{
	for (const auto& tile : m_GroundTiles) {
		// 地面は厚さ 0.1f でスケール
		// キューブは中心から上下に0.05f ずつ広がるため、底面がY=0になるようにオフセット
		XMMATRIX mtxScale = XMMatrixScaling(tile.width, 0.1f, tile.depth);
		XMMATRIX mtxTrans = XMMatrixTranslation(tile.position.x, tile.position.y - 0.05f, tile.position.z);

		XMMATRIX world = mtxScale * mtxTrans;
		Cube_Draw(tile.textureId, world);
	}
}

