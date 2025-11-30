#include "PowerLine.h"
#include "cube.h"
#include <cmath>
#include <algorithm>

PowerLine::PowerLine(const XMFLOAT3& startPos, const XMFLOAT3& endPos, float radius)
    : GameObject(startPos, nullptr, -1, { radius, radius, radius }),
      m_startPos(startPos),
      m_endPos(endPos),
      m_radius(radius),
      m_electricityIntensity(1.0f)
{
    // 中点を GameObject の位置に設定
    m_Position.x = (startPos.x + endPos.x) * 0.5f;
    m_Position.y = (startPos.y + endPos.y) * 0.5f;
    m_Position.z = (startPos.z + endPos.z) * 0.5f;
}

void PowerLine::Update(double elapsedTime)
{
    // 電線は静的なので特に更新処理はない
    // 必要に応じてエフェクトなどを追加可能
}

void PowerLine::Draw() const
{
    // 開始点から終了点まで複数のキューブを描画して電線を表現
    XMVECTOR start = XMLoadFloat3(&m_startPos);
    XMVECTOR end = XMLoadFloat3(&m_endPos);
    XMVECTOR direction = end - start;
    float distance = XMVectorGetX(XMVector3Length(direction));

    // 電線の分割数（距離に応じて調整）
    int segments = static_cast<int>(distance / (m_radius * 2.0f)) + 1;

    for (int i = 0; i < segments; ++i) {
        float t = (segments > 1) ? static_cast<float>(i) / (segments - 1) : 0.0f;
        
        XMVECTOR segmentPos = start + direction * t;
        XMFLOAT3 pos;
        XMStoreFloat3(&pos, segmentPos);

        // キューブを描画
        XMMATRIX scale = XMMatrixScaling(m_radius, m_radius, m_radius);
        XMMATRIX translation = XMMatrixTranslation(pos.x, pos.y, pos.z);
        XMMATRIX world = scale * translation;

        Cube_Draw(0, world);  // テクスチャID 0（白いキューブ）
    }
}

AABB PowerLine::GetAABB() const
{
    // 電線全体を覆う AABB を計算
    float minX = std::min(m_startPos.x, m_endPos.x) - m_radius;
    float maxX = std::max(m_startPos.x, m_endPos.x) + m_radius;
    float minY = std::min(m_startPos.y, m_endPos.y) - m_radius;
    float maxY = std::max(m_startPos.y, m_endPos.y) + m_radius;
    float minZ = std::min(m_startPos.z, m_endPos.z) - m_radius;
    float maxZ = std::max(m_startPos.z, m_endPos.z) + m_radius;

    return AABB(
        { minX, minY, minZ },
        { maxX, maxY, maxZ }
    );
}

float PowerLine::GetDistancePointToLineSegment(const XMFLOAT3& point) const
{
    XMVECTOR p = XMLoadFloat3(&point);
    XMVECTOR a = XMLoadFloat3(&m_startPos);
    XMVECTOR b = XMLoadFloat3(&m_endPos);

    XMVECTOR ap = p - a;
    XMVECTOR ab = b - a;
    float ab_length_sq = XMVectorGetX(XMVector3LengthSq(ab));

    if (ab_length_sq < 0.0001f) {
        // 開始点と終了点が同じ場合
        return XMVectorGetX(XMVector3Length(ap));
    }

    float t = XMVectorGetX(XMVector3Dot(ap, ab)) / ab_length_sq;
    t = std::max(0.0f, std::min(1.0f, t));

    XMVECTOR closest = a + ab * t;
    XMVECTOR dist_vec = p - closest;
    return XMVectorGetX(XMVector3Length(dist_vec));
}

bool PowerLine::CheckCollisionWithPoint(const XMFLOAT3& point) const
{
    float distance = GetDistancePointToLineSegment(point);
    return distance <= m_radius;
}

DirectX::XMFLOAT3 PowerLine::GetClosestPointOnLine(const DirectX::XMFLOAT3& point) const
{
    XMVECTOR p = XMLoadFloat3(&point);
    XMVECTOR a = XMLoadFloat3(&m_startPos);
    XMVECTOR b = XMLoadFloat3(&m_endPos);

    XMVECTOR ap = p - a;
    XMVECTOR ab = b - a;
    float ab_length_sq = XMVectorGetX(XMVector3LengthSq(ab));

    if (ab_length_sq < 0.0001f) {
        // 開始点と終了点が同じ場合は開始点を返す
        return m_startPos;
    }

    float t = XMVectorGetX(XMVector3Dot(ap, ab)) / ab_length_sq;
    t = std::max(0.0f, std::min(1.0f, t));

    XMVECTOR closest = a + ab * t;
    DirectX::XMFLOAT3 result;
    XMStoreFloat3(&result, closest);
    return result;
}
