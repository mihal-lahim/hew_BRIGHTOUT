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
    SetTag(GameObjectTag::POWER_LINE);
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
    // 電線を複数のセグメントで垂れた表現を実現
    XMVECTOR start = XMLoadFloat3(&m_startPos);
    XMVECTOR end = XMLoadFloat3(&m_endPos);
    XMVECTOR direction = end - start;
    float distance = XMVectorGetX(XMVector3Length(direction));

    if (distance < 0.001f) return; // 長さがほぼ0の場合はスキップ

    // セグメント数を最適化（距離が長いほど多くなる）
    // max 15 segments でバランスを取る
    const int segments = std::min(15, static_cast<int>(distance / (m_radius * 2.0f)) + 3);
    
    // 弛みの量（距離に応じて調整）
    float sag = distance * 0.08f; // 距離の8%程度垂れる

    // 棒の半径（細く表現）
    float wireRadius = m_radius * 0.3f;

    for (int i = 0; i < segments - 1; ++i) {
        float t0 = static_cast<float>(i) / (segments - 1);
        float t1 = static_cast<float>(i + 1) / (segments - 1);

        // パラメトリック曲線で各セグメントの開始点と終了点を計算
        XMVECTOR pos0 = start + direction * t0;
        XMVECTOR pos1 = start + direction * t1;

        // Y軸に弛みを追加（放物線状）
        float sag0 = sag * 4.0f * t0 * (1.0f - t0);
        float sag1 = sag * 4.0f * t1 * (1.0f - t1);

        pos0 = pos0 + XMVectorSet(0.0f, -sag0, 0.0f, 0.0f);
        pos1 = pos1 + XMVectorSet(0.0f, -sag1, 0.0f, 0.0f);

        // セグメント方向ベクトル
        XMVECTOR segDir = pos1 - pos0;
        float segLength = XMVectorGetX(XMVector3Length(segDir));

        if (segLength < 0.001f) continue;

        XMVECTOR segDirNorm = XMVector3Normalize(segDir);

        // 中点
        XMVECTOR midPos = pos0 + segDir * 0.5f;
        XMFLOAT3 mid;
        XMStoreFloat3(&mid, midPos);

        // 回転行列を計算（Z軸がセグメント方向になるように）
        XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
        XMVECTOR right = XMVector3Cross(up, segDirNorm);
        float rightLen = XMVectorGetX(XMVector3Length(right));

        if (rightLen > 0.001f) {
            right = XMVector3Normalize(right);
            up = XMVector3Cross(segDirNorm, right);
        } else {
            // Y軸が方向ベクトルと平行の場合、X軸を基準に
            right = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
            up = XMVector3Cross(segDirNorm, right);
            up = XMVector3Normalize(up);
        }

        // 回転行列を構成
        XMMATRIX rotation;
        rotation.r[0] = right;
        rotation.r[1] = up;
        rotation.r[2] = segDirNorm;
        rotation.r[3] = XMVectorSet(0, 0, 0, 1);

        // スケーリング（X, Y: 細い半径、Z: セグメント長）
        XMMATRIX scale = XMMatrixScaling(wireRadius, wireRadius, segLength);

        // 平行移動
        XMMATRIX translation = XMMatrixTranslation(mid.x, mid.y, mid.z);

        // ワールド行列
        XMMATRIX world = scale * rotation * translation;

        // セグメントを描画
        Cube_Draw(0, world);
    }

    // 始点と終点に球体を追加（接続部分を見栄えよく）
    XMMATRIX sphereScale = XMMatrixScaling(wireRadius * 2.0f, wireRadius * 2.0f, wireRadius * 2.0f);
    XMMATRIX sphereTransStart = XMMatrixTranslation(m_startPos.x, m_startPos.y, m_startPos.z);
    Cube_Draw(0, sphereScale * sphereTransStart);

    XMMATRIX sphereTransEnd = XMMatrixTranslation(m_endPos.x, m_endPos.y, m_endPos.z);
    Cube_Draw(0, sphereScale * sphereTransEnd);
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
    // 直線状の最小距離を計算（描画は曲線でも、当たり判定は直線）
    float distance = GetDistancePointToLineSegment(point);
    return distance <= m_radius;
}

DirectX::XMFLOAT3 PowerLine::GetClosestPointOnLine(const DirectX::XMFLOAT3& point) const
{
    // 直線上の最も近いポイントを返す（描画は曲線でも、当たり判定は直線）
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
