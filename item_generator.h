#pragma once
#include <vector>
#include "item.h"
#include "player.h"
#include "map.h"

// アイテム生成管理クラス：電線上にアイテムを生成・更新・描画・回収処理を管理
class ItemGenerator
{
public:
    void GenerateItem(const XMFLOAT3& pos);                  // 指定位置にアイテム生成
    void GenerateItemsOnPowerLines(const Map& map);          // マップの電線上に初期アイテム生成
    void Update(double elapsed);                              // 更新（タイマー管理と自動生成）
    void Draw();                                             // 描画
    void CheckPickup(Player* player);                        // プレイヤー回収判定

private:
    std::vector<Item*> m_items;               // 生成済みアイテム
    double m_spawnTimer = 0.0;                // 生成タイマー
    std::vector<XMFLOAT3> m_powerLinePositions; // 電線の座標
};