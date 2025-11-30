#include "../include/PowerLine.h"
#include "../include/cube.h"
#include <cmath>
#include <algorithm>

PowerLine::PowerLine(const XMFLOAT3& startPos, const XMFLOAT3& endPos, float radius)
    : GameObject(startPos, nullptr, -1, { radius, radius, radius }),
      m_startPos(startPos),
      m_endPos(endPos),
      m_radius(radius),
      m_electricityIntensity(1.0f)
{
    // ���_�� GameObject �̈ʒu�ɐݒ�
    m_Position.x = (startPos.x + endPos.x) * 0.5f;
    m_Position.y = (startPos.y + endPos.y) * 0.5f;
    m_Position.z = (startPos.z + endPos.z) * 0.5f;
}

void PowerLine::Update(double elapsedTime)
{
    // �d���͐ÓI�Ȃ̂œ��ɍX�V�����͂Ȃ�
    // �K�v�ɉ����ăG�t�F�N�g�Ȃǂ�ǉ��\
}

void PowerLine::Draw() const
{
    // �J�n�_����I���_�܂ŕ����̃L���[�u��`�悵�ēd����\��
    XMVECTOR start = XMLoadFloat3(&m_startPos);
    XMVECTOR end = XMLoadFloat3(&m_endPos);
    XMVECTOR direction = end - start;
    float distance = XMVectorGetX(XMVector3Length(direction));

    // �d���̕������i�����ɉ����Ē����j
    int segments = static_cast<int>(distance / (m_radius * 2.0f)) + 1;

    for (int i = 0; i < segments; ++i) {
        float t = (segments > 1) ? static_cast<float>(i) / (segments - 1) : 0.0f;
        
        XMVECTOR segmentPos = start + direction * t;
        XMFLOAT3 pos;
        XMStoreFloat3(&pos, segmentPos);

        // �L���[�u��`��
        XMMATRIX scale = XMMatrixScaling(m_radius, m_radius, m_radius);
        XMMATRIX translation = XMMatrixTranslation(pos.x, pos.y, pos.z);
        XMMATRIX world = scale * translation;

        Cube_Draw(0, world);  // �e�N�X�`��ID 0�i�����L���[�u�j
    }
}

AABB PowerLine::GetAABB() const
{
    // �d���S�̂𕢂� AABB ���v�Z
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
        // �J�n�_�ƏI���_�������ꍇ
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
        // �J�n�_�ƏI���_�������ꍇ�͊J�n�_��Ԃ�
        return m_startPos;
    }

    float t = XMVectorGetX(XMVector3Dot(ap, ab)) / ab_length_sq;
    t = std::max(0.0f, std::min(1.0f, t));

    XMVECTOR closest = a + ab * t;
    DirectX::XMFLOAT3 result;
    XMStoreFloat3(&result, closest);
    return result;
}
