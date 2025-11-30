#include "Pole.h"
#include "cube.h"
#include <cmath>
#include <algorithm>

Pole::Pole(const XMFLOAT3& pos, float height, float radius)
    : GameObject(pos, nullptr, -1, { radius, height, radius }),
      m_height(height),
      m_radius(radius),
      m_rotationSpeed(0.0f),
      m_elapsedTime(0.0)
{
}

void Pole::Update(double elapsedTime)
{
    // 電柱は静的なので特に更新処理はない
    // 必要に応じてエフェクトなどを追加可能
    m_elapsedTime += elapsedTime;
}

void Pole::Draw() const
{
    // 電柱を描画（複数のキューブをスタックして表現）
    
    // 下部：太いベース
    {
        XMMATRIX scale = XMMatrixScaling(m_radius * 1.5f, m_radius, m_radius * 1.5f);
        XMMATRIX translation = XMMatrixTranslation(m_Position.x, m_Position.y + m_radius, m_Position.z);
        XMMATRIX world = scale * translation;
        Cube_Draw(0, world);
    }

    // 中央：メイン部分
    {
        XMMATRIX scale = XMMatrixScaling(m_radius, m_height * 0.8f, m_radius);
        XMMATRIX translation = XMMatrixTranslation(m_Position.x, m_Position.y + m_height * 0.5f, m_Position.z);
        XMMATRIX world = scale * translation;
        Cube_Draw(0, world);
    }

    // 上部：先端
    {
        XMMATRIX scale = XMMatrixScaling(m_radius * 0.8f, m_radius * 2.0f, m_radius * 0.8f);
        XMMATRIX translation = XMMatrixTranslation(m_Position.x, m_Position.y + m_height - m_radius, m_Position.z);
        XMMATRIX world = scale * translation;
        Cube_Draw(0, world);
    }
}

AABB Pole::GetAABB() const
{
    // 電柱全体を覆う AABB を計算
    XMFLOAT3 halfSize = { m_radius, m_height * 0.5f, m_radius };
    XMFLOAT3 min = {
        m_Position.x - halfSize.x,
        m_Position.y,
        m_Position.z - halfSize.z
    };
    XMFLOAT3 max = {
        m_Position.x + halfSize.x,
        m_Position.y + m_height,
        m_Position.z + halfSize.z
    };
    return AABB(min, max);
}

bool Pole::CheckCollisionWithPoint(const XMFLOAT3& point) const
{
    // 点が電柱の円柱範囲内にあるか判定
    
    // Y軸の判定
    if (point.y < m_Position.y || point.y > m_Position.y + m_height) {
        return false;
    }

    // XZ平面の距離を計算（円柱判定）
    float dx = point.x - m_Position.x;
    float dz = point.z - m_Position.z;
    float distanceSq = dx * dx + dz * dz;
    float radiusSq = m_radius * m_radius;

    return distanceSq <= radiusSq;
}
