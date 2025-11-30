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

    // 電線ブロック取得用（仮のBlock型とPowerLine判定を追加）
    struct Block {
        enum Type {
            PowerLine,
            Other
        };
        Type type;
        DirectX::XMFLOAT3 position;
        Type GetType() const { return type; }
        DirectX::XMFLOAT3 GetPosition() const { return position; }
    };

    // 電線上のブロックリスト取得
    std::vector<Block> GetBlocks() const {
        std::vector<Block> blocks;
        // GroundTileから仮にPowerLineブロックを生成（実装は適宜修正）
        for (const auto& tile : m_GroundTiles) {
            // ここでは仮に全てPowerLineとして追加
            blocks.push_back(Block{ Block::PowerLine, tile.position });
        }
        return blocks;
    }
};

// グローバル Map インスタンス
extern Map g_MapInstance;

#endif // MAP_H