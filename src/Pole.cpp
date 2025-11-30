#include "../include/Pole.h"
#include "../include/cube.h"
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
    // �d���͐ÓI�Ȃ̂œ��ɍX�V�����͂Ȃ�
    // �K�v�ɉ����ăG�t�F�N�g�Ȃǂ�ǉ��\
    m_elapsedTime += elapsedTime;
}

void Pole::Draw() const
{
    // �d����`��i�����̃L���[�u���X�^�b�N���ĕ\���j
    
    // �����F�����x�[�X
    {
        XMMATRIX scale = XMMatrixScaling(m_radius * 1.5f, m_radius, m_radius * 1.5f);
        XMMATRIX translation = XMMatrixTranslation(m_Position.x, m_Position.y + m_radius, m_Position.z);
        XMMATRIX world = scale * translation;
        Cube_Draw(0, world);
    }

    // �����F���C������
    {
        XMMATRIX scale = XMMatrixScaling(m_radius, m_height * 0.8f, m_radius);
        XMMATRIX translation = XMMatrixTranslation(m_Position.x, m_Position.y + m_height * 0.5f, m_Position.z);
        XMMATRIX world = scale * translation;
        Cube_Draw(0, world);
    }

    // �㕔�F��[
    {
        XMMATRIX scale = XMMatrixScaling(m_radius * 0.8f, m_radius * 2.0f, m_radius * 0.8f);
        XMMATRIX translation = XMMatrixTranslation(m_Position.x, m_Position.y + m_height - m_radius, m_Position.z);
        XMMATRIX world = scale * translation;
        Cube_Draw(0, world);
    }
}

AABB Pole::GetAABB() const
{
    // �d���S�̂𕢂� AABB ���v�Z
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
    // �_���d���̉~���͈͓��ɂ��邩����
    
    // Y���̔���
    if (point.y < m_Position.y || point.y > m_Position.y + m_height) {
        return false;
    }

    // XZ���ʂ̋������v�Z�i�~������j
    float dx = point.x - m_Position.x;
    float dz = point.z - m_Position.z;
    float distanceSq = dx * dx + dz * dz;
    float radiusSq = m_radius * m_radius;

    return distanceSq <= radiusSq;
}
