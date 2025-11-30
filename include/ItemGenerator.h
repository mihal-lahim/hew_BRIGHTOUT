#pragma once
#include <vector>
#include <DirectXMath.h>
#include "Item.h"

class Map;
class Player;

using namespace DirectX;

// アイテムジェネレータクラス：定期的にアイテムを生成・更新・描画・管理
class ItemGenerator
{
public:
    ItemGenerator() = default;
    ~ItemGenerator();

    void Initialize();                                        // 初期化
    void GenerateItem(const XMFLOAT3& pos, ItemType type);   // 指定位置にアイテム生成
    void GenerateItemsOnPowerLines(const Map& map);          // マップの電線にランダムアイテム生成
    void Update(double elapsed);                              // 更新（タイマー管理と無効アイテム削除）
    void Draw() const;                                        // 描画
    void CheckPickup(Player* player);                        // プレイヤーとのピックアップ判定
    void RegisterItem(Item* item);                           // 外部から生成されたアイテムを登録（ItemGeneratorObject から呼び出される）

    std::vector<Item*>& GetItems() { return m_items; }
    size_t GetItemCount() const { return m_items.size(); }

private:
    std::vector<Item*> m_items;               // アクティブなアイテム
    double m_spawnTimer = 0.0;                // 生成タイマー
    double m_spawnInterval = 3.0;             // 生成間隔（秒）
    std::vector<XMFLOAT3> m_powerLinePositions; // 電線の座標
};
