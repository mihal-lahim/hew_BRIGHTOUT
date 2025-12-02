#include "house.h"
#include "player.h"
#include "model.h"
#include "debug_console.h"
#include <cmath>

House::House(const XMFLOAT3& pos, MODEL* model, float maxElectricity)
    : GameObject(pos, model, -1, { HOUSE_SIZE, HOUSE_SIZE, HOUSE_SIZE }),
      m_model(model),
      m_electricity(0.0f),
      m_maxElectricity(maxElectricity),
      m_isRepaired(false),
      m_effectTimer(0.0f)
{
    SetTag(GameObjectTag::HOUSE);
}

void House::Update(double elapsed)
{
    // 毎フレーム更新
    m_effectTimer += static_cast<float>(elapsed);
    
    // 復旧状態を自動更新
    if (m_electricity >= m_maxElectricity) {
        m_isRepaired = true;
        m_electricity = m_maxElectricity;  // 上限をオーバーしないように
    } else {
        m_isRepaired = false;
    }
}


void House::Draw() const
{
    if (!m_model) return;

    // ハウスの描画
    XMMATRIX scale = XMMatrixScaling(1.0f, 1.0f, 1.0f);
    XMMATRIX translation = XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z);
    XMMATRIX world = scale * translation;

    ModelDraw(m_model, world);
}

AABB House::GetAABB() const
{
    float halfSize = HOUSE_SIZE * 0.5f;
    XMFLOAT3 min = {
        m_Position.x - halfSize,
        m_Position.y - halfSize,
        m_Position.z - halfSize
    };
    XMFLOAT3 max = {
        m_Position.x + halfSize,
        m_Position.y + halfSize,
        m_Position.z + halfSize
    };
    return AABB(min, max);
}

void House::SetRepaired(bool repaired)
{
    m_isRepaired = repaired;
    if (repaired) {
        m_electricity = m_maxElectricity;
    }
}

float House::GetDistanceToPlayer(const XMFLOAT3& playerPos) const
{
    float dx = m_Position.x - playerPos.x;
    float dy = m_Position.y - playerPos.y;
    float dz = m_Position.z - playerPos.z;
    
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}

bool House::IsPlayerNearby(const XMFLOAT3& playerPos, float radius) const
{
    return GetDistanceToPlayer(playerPos) <= radius;
}

void House::ReceiveElectricity(float amount)
{
    m_electricity += amount;
    if (m_electricity > m_maxElectricity) {
        m_electricity = m_maxElectricity;
	}
    
    // デバッグログ出力
    //DEBUG_LOGF("[House] Received electricity: +%.1f (Current: %.1f/%.1f)",amount, m_electricity, m_maxElectricity);
}
