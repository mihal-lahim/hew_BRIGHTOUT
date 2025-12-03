#include "ItemGeneratorObject.h"
#include "Item.h"
#include "cube.h"
#include "ItemGenerator.h"
#include <cmath>
#include <cstdlib>
#include <ctime>

ItemGeneratorObject::ItemGeneratorObject(const XMFLOAT3& pos, float spawnRadius, float spawnInterval)
    : GameObject(pos, nullptr, -1, { spawnRadius * 0.2f, spawnRadius * 0.2f, spawnRadius * 0.2f }),
      m_spawnRadius(spawnRadius),
      m_spawnInterval(spawnInterval),
      m_spawnTimer(0.0),
      m_visualRadius(0.5f)
{
    SetTag(GameObjectTag::ITEM_GENERATOR);
    // ワールドシード初期化
    static bool initialized = false;
    if (!initialized) {
        srand(static_cast<unsigned>(time(nullptr)));
        initialized = true;
    }
}

ItemGeneratorObject::~ItemGeneratorObject()
{
    // 生成したアイテムはItemGeneratorが管理するため、ここでは削除しない
    m_spawnedItems.clear();
}

void ItemGeneratorObject::Update(double elapsedTime)
{
    // スポーンタイマーを更新
    m_spawnTimer += elapsedTime;

    // スポーン間隔に達したかチェック
    if (m_spawnTimer >= m_spawnInterval) {
        m_spawnTimer = 0.0;
        SpawnItem();
    }

    // 無効なアイテムを削除
    auto it = m_spawnedItems.begin();
    while (it != m_spawnedItems.end()) {
        if (!(*it)->IsActive()) {
            it = m_spawnedItems.erase(it);
        } else {
            ++it;
        }
    }
}

void ItemGeneratorObject::Draw() const
{
    // ジェネレータ本体の描画（回転対応）
    XMMATRIX scale = XMMatrixScaling(m_visualRadius, m_visualRadius, m_visualRadius);
    XMMATRIX rotation = XMMatrixRotationY(m_Rotation.y);
    XMMATRIX translation = XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z);
    XMMATRIX world = scale * rotation * translation;

    // ジェネレータを黄色のテクスチャで描画（テクスチャID 4 を使用）
    Cube_Draw(4, world);

    // スポーン範囲を視覚化するための外部の円形描画
    {
        float rangeSize = m_spawnRadius * 0.3f;
        XMMATRIX rangeScale = XMMatrixScaling(rangeSize, 0.1f, rangeSize);
        XMMATRIX rangeRotation = XMMatrixRotationY(m_Rotation.y);
        XMMATRIX rangeTranslation = XMMatrixTranslation(m_Position.x, m_Position.y + 0.1f, m_Position.z);
        XMMATRIX rangeWorld = rangeScale * rangeRotation * rangeTranslation;

        // スポーン範囲を茶色で描画
        Cube_Draw(0, rangeWorld);
    }
}

AABB ItemGeneratorObject::GetAABB() const
{
    XMFLOAT3 halfSize = { m_visualRadius, m_visualRadius, m_visualRadius };
    XMFLOAT3 min = {
        m_Position.x - halfSize.x,
        m_Position.y - halfSize.y,
        m_Position.z - halfSize.z
    };
    XMFLOAT3 max = {
        m_Position.x + halfSize.x,
        m_Position.y + halfSize.y,
        m_Position.z + halfSize.z
    };
    return AABB(min, max);
}

void ItemGeneratorObject::SpawnItem()
{
    // スポーン範囲内のランダムな位置を計算
    float randomAngle = static_cast<float>(rand() % 360) * 3.14159f / 180.0f;
    float randomDistance = static_cast<float>(rand() % static_cast<int>(m_spawnRadius * 100)) / 100.0f;

    XMFLOAT3 itemPos = {
        m_Position.x + randomDistance * std::cos(randomAngle),
        m_Position.y,  // 地面と同じ高さに配置
        m_Position.z + randomDistance * std::sin(randomAngle)
    };

    // ランダムなアイテムタイプを選択
    ItemType itemType = static_cast<ItemType>(rand() % 3);

    // アイテムを生成
    Item* newItem = new Item(itemPos, itemType, 0.5f);
    m_spawnedItems.push_back(newItem);
}
