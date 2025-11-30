#pragma once
#include <DirectXMath.h>
#include "GameObject.h"
#include "collision.h"
#include <vector>

class Item;

using namespace DirectX;

// アイテムジェネレータオブジェクトクラス：GameObjectを継承
// フィールド上に配置され、周辺にアイテムをスポーンする
class ItemGeneratorObject : public GameObject
{
public:
    // コンストラクタ
    ItemGeneratorObject(const XMFLOAT3& pos, float spawnRadius = 5.0f, float spawnInterval = 3.0f);
    virtual ~ItemGeneratorObject();

    virtual void Update(double elapsedTime) override;  // 更新処理
    virtual void Draw() const override;                // 描画処理
    virtual AABB GetAABB() const override;            // 衝突判定用AABB

    // スポーン範囲内にアイテムを生成
    void SpawnItem();

    // プロパティ取得
    float GetSpawnRadius() const { return m_spawnRadius; }
    float GetSpawnInterval() const { return m_spawnInterval; }
    
    // 生成したアイテムを取得
    const std::vector<Item*>& GetSpawnedItems() const { return m_spawnedItems; }

    // ジェネレータのIDを管理
    void SetGeneratorID(int id) { m_generatorID = id; }
    int GetGeneratorID() const { return m_generatorID; }

private:
    float m_spawnRadius;        // スポーン範囲（半径）
    float m_spawnInterval;      // スポーン間隔（秒）
    double m_spawnTimer;        // スポーンタイマー
    std::vector<Item*> m_spawnedItems;  // このジェネレータが生成したアイテム
    int m_generatorID = -1;     // ジェネレータのID
    float m_visualRadius = 0.5f; // ビジュアル表現のサイズ
};
