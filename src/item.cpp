#include "item.h"
#include "player.h"
#include "cube.h"
#include <cmath>

Item::Item(const XMFLOAT3& pos, ItemType type, float size)
    : GameObject(pos, nullptr, -1, { size, size, size }),
      m_size(size),
      m_active(true),
      m_effectTimer(0.0),
      m_type(type)
{
}

void Item::Update(double elapsed)
{
    if (!m_active) return;

    // 効果タイマー更新（必要に応じて）
    m_effectTimer += elapsed;

    // ゆっくり上下に浮遊する動き
    float bobAmount = static_cast<float>(std::sin(m_effectTimer * 2.0) * 0.3f);
    m_Position.y += static_cast<float>(bobAmount * elapsed);
}

DirectX::XMFLOAT4 Item::GetColor() const
{
    // アイテムのタイプに応じて色を返す
    switch (m_type) {
        case ItemType::SpeedBoost:
            return { 1.0f, 1.0f, 0.0f, 1.0f };  // 黄色：速度アップ
        case ItemType::ChargeTank:
            return { 0.0f, 1.0f, 1.0f, 1.0f };  // シアン：充電容量
        case ItemType::ElectricBoost:
            return { 1.0f, 0.0f, 1.0f, 1.0f };  // マゼンタ：放電時間
        default:
            return { 1.0f, 1.0f, 1.0f, 1.0f };  // 白：その他
    }
}

void Item::Draw() const
{
    if (!m_active) return;

    // キューブを描画（アイテムタイプに応じた色）
    {
        XMMATRIX scale = XMMatrixScaling(m_size, m_size, m_size);
        XMMATRIX translation = XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z);
        XMMATRIX world = scale * translation;
        
        // テクスチャID に応じて異なる描画（カラーマッピング）
        // ID 0：白（基本）/ ID 1-3：色付き
        int colorTextureId = 0;
        switch (m_type) {
            case ItemType::SpeedBoost:
                colorTextureId = 1;  // 黄色
                break;
            case ItemType::ChargeTank:
                colorTextureId = 2;  // シアン
                break;
            case ItemType::ElectricBoost:
                colorTextureId = 3;  // マゼンタ
                break;
        }
        
        Cube_Draw(colorTextureId, world);
    }
}

AABB Item::GetAABB() const
{
    XMFLOAT3 halfSize = { m_size * 0.5f, m_size * 0.5f, m_size * 0.5f };
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

bool Item::CheckCollision(Player* player)
{
    if (!m_active || !player) return false;

    // プレイヤーの AABB と比較
    AABB playerAABB = player->GetAABB();
    AABB itemAABB = GetAABB();

    // AABB 衝突判定
    if (playerAABB.IsOverlap(itemAABB)) {
        m_active = false;  // アイテムを非アクティブにする
        return true;
    }
    return false;
}