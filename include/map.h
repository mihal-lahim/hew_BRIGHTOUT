/*==============================================================================

 ゴルフコースの制御 [map.h]
 Author : sumi rintarou
 Date :2025/11/04
--------------------------------------------------------------------------------

==============================================================================*/
#ifndef MAP_H
#define MAP_H

#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>

// 地面タイル用構造体
struct GroundTile
{
    DirectX::XMFLOAT3 position;
    int textureId;
    float width;
    float depth;
};

class Map
{
private:
    std::vector<GroundTile> m_GroundTiles;

public:
    Map() = default;
    ~Map() = default;

    void Initialize();
    void Finalize();
    void Draw() const;
};

// グローバル Map インスタンス
extern Map g_MapInstance;

#endif // MAP_H