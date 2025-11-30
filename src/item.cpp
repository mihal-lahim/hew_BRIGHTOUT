#include "../include/item.h"
#include "../include/player.h"
#include "../include/cube.h"
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

    // ���ʃ^�C�}�[�X�V�i�K�v�ɉ����āj
    m_effectTimer += elapsed;

    // �������㉺�ɕ��V���铮��
    float bobAmount = static_cast<float>(std::sin(m_effectTimer * 2.0) * 0.3f);
    m_Position.y += static_cast<float>(bobAmount * elapsed);
}

DirectX::XMFLOAT4 Item::GetColor() const
{
    // �A�C�e���̃^�C�v�ɉ����ĐF��Ԃ�
    switch (m_type) {
        case ItemType::SpeedBoost:
            return { 1.0f, 1.0f, 0.0f, 1.0f };  // ���F�F���x�A�b�v
        case ItemType::ChargeTank:
            return { 0.0f, 1.0f, 1.0f, 1.0f };  // �V�A���F�[�d�e��
        case ItemType::ElectricBoost:
            return { 1.0f, 0.0f, 1.0f, 1.0f };  // �}�[���^�F���d����
        default:
            return { 1.0f, 1.0f, 1.0f, 1.0f };  // ���F���̑�
    }
}

void Item::Draw() const
{
    if (!m_active) return;

    // �L���[�u��`��i�A�C�e���^�C�v�ɉ������F�j
    {
        XMMATRIX scale = XMMatrixScaling(m_size, m_size, m_size);
        XMMATRIX translation = XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z);
        XMMATRIX world = scale * translation;
        
        // �e�N�X�`��ID �ɉ����ĈقȂ�`��i�J���[�}�b�s���O�j
        // ID 0�F���i��{�j/ ID 1-3�F�F�t��
        int colorTextureId = 0;
        switch (m_type) {
            case ItemType::SpeedBoost:
                colorTextureId = 1;  // ���F
                break;
            case ItemType::ChargeTank:
                colorTextureId = 2;  // �V�A��
                break;
            case ItemType::ElectricBoost:
                colorTextureId = 3;  // �}�[���^
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

    // �v���C���[�� AABB �Ɣ�r
    AABB playerAABB = player->GetAABB();
    AABB itemAABB = GetAABB();

    // AABB �Փ˔���
    if (playerAABB.IsOverlap(itemAABB)) {
        m_active = false;  // �A�C�e�����A�N�e�B�u�ɂ���
        return true;
    }
    return false;
}