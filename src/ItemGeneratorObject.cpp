#include "../include/ItemGeneratorObject.h"
#include "../include/Item.h"
#include "../include/cube.h"
#include "../include/ItemGenerator.h"
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
    // �����_���V�[�h��������
    static bool initialized = false;
    if (!initialized) {
        srand(static_cast<unsigned>(time(nullptr)));
        initialized = true;
    }
}

ItemGeneratorObject::~ItemGeneratorObject()
{
    // ���������A�C�e����ItemGenerator���Ǘ����邽�߁A�����ł͍폜���Ȃ�
    m_spawnedItems.clear();
}

void ItemGeneratorObject::Update(double elapsedTime)
{
    // �X�|�[���^�C�}�[���X�V
    m_spawnTimer += elapsedTime;

    // �X�|�[���Ԋu�ɒB�������`�F�b�N
    if (m_spawnTimer >= m_spawnInterval) {
        m_spawnTimer = 0.0;
        SpawnItem();
    }

    // �����ȃA�C�e�����폜
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
    // �W�F�l���[�^�{�̂�\���i�����������́j
    XMMATRIX scale = XMMatrixScaling(m_visualRadius, m_visualRadius, m_visualRadius);
    XMMATRIX translation = XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z);
    XMMATRIX world = scale * translation;

    // �W�F�l���[�^��ΐF�̃e�N�X�`���ŕ`��i�e�N�X�`��ID 4 ���g�p�j
    Cube_Draw(4, world);

    // �X�|�[���͈͂�\�����邽�߂̊O���̉~��`��
    {
        float rangeSize = m_spawnRadius * 0.3f;
        XMMATRIX rangeScale = XMMatrixScaling(rangeSize, 0.1f, rangeSize);
        XMMATRIX rangeTranslation = XMMatrixTranslation(m_Position.x, m_Position.y + 0.1f, m_Position.z);
        XMMATRIX rangeWorld = rangeScale * rangeTranslation;

        // �X�|�[���͈͂𔖂��D�F�ŕ`��
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
    // �X�|�[���͈͓��̃����_���Ȉʒu���v�Z
    float randomAngle = static_cast<float>(rand() % 360) * 3.14159f / 180.0f;
    float randomDistance = static_cast<float>(rand() % static_cast<int>(m_spawnRadius * 100)) / 100.0f;

    XMFLOAT3 itemPos = {
        m_Position.x + randomDistance * std::cos(randomAngle),
        m_Position.y + 1.0f,  // �������߂ɔz�u
        m_Position.z + randomDistance * std::sin(randomAngle)
    };

    // �����_���ȃA�C�e���^�C�v��I��
    ItemType itemType = static_cast<ItemType>(rand() % 3);

    // �A�C�e���𐶐�
    Item* newItem = new Item(itemPos, itemType, 0.5f);
    m_spawnedItems.push_back(newItem);
}
