#include "../include/ItemGenerator.h"
#include "../include/player.h"
#include <cstdlib>
#include <ctime>

ItemGenerator::~ItemGenerator()
{
    // ���ׂẴA�C�e�����N���[���A�b�v
    for (auto item : m_items) {
        delete item;
    }
    m_items.clear();
}

void ItemGenerator::Initialize()
{
    m_spawnTimer = 0.0;
    m_items.clear();
    m_powerLinePositions.clear();

    // �����_���V�[�h��������
    srand(static_cast<unsigned>(time(nullptr)));
}

void ItemGenerator::GenerateItem(const XMFLOAT3& pos, ItemType type)
{
    // �V�����A�C�e���𐶐�
    Item* newItem = new Item(pos, type, 0.5f);
    m_items.push_back(newItem);
}

void ItemGenerator::RegisterItem(Item* item)
{
    // �O�����琶�����ꂽ�A�C�e����o�^�iItemGeneratorObject ����Ăяo�����j
    if (item) {
        m_items.push_back(item);
    }
}

void ItemGenerator::GenerateItemsOnPowerLines(const Map& map)
{
    // �d�����W��������
    m_powerLinePositions.clear();
    
    // �d���̒��_�ʒu�ɃA�C�e����z�u
    m_powerLinePositions.push_back({ -0.0f, 5.0f, 0.0f });   // �����d��1�̒��_
    m_powerLinePositions.push_back({ 0.0f, 5.0f, 0.0f });    // �����d���̒��_
    m_powerLinePositions.push_back({ 0.0f, 8.0f, 10.0f });   // �����d��2�̒��_
    
    // �e�d���ʒu�̎��ӂɃA�C�e���𐶐�
    for (const auto& linePos : m_powerLinePositions) {
        // �A�C�e���𒼐ړd���ʒu�ɔz�u
        ItemType randomType = static_cast<ItemType>(rand() % 3);
        GenerateItem(linePos, randomType);
    }
}

void ItemGenerator::Update(double elapsed)
{
    // �e�A�C�e�����X�V
    for (auto item : m_items) {
        if (item && item->IsActive()) {
            item->Update(elapsed);
        }
    }

    // �����ȃA�C�e�����폜
    auto it = m_items.begin();
    while (it != m_items.end()) {
        if (!(*it)->IsActive()) {
            delete *it;
            it = m_items.erase(it);
        } else {
            ++it;
        }
    }

    // ����I�ɐV�����A�C�e���𐶐�
    m_spawnTimer += elapsed;
    if (m_spawnTimer >= m_spawnInterval) {
        m_spawnTimer = 0.0;
        
        // �����_���Ȉʒu�ɐV�����A�C�e���𐶐�
        float randomX = static_cast<float>(rand() % 20 - 10);  // -10 ���� 10
        float randomZ = static_cast<float>(rand() % 20 - 10);
        
        XMFLOAT3 randomPos = { randomX, 2.0f, randomZ };
        ItemType randomType = static_cast<ItemType>(rand() % 3);
        GenerateItem(randomPos, randomType);
    }
}

void ItemGenerator::Draw() const
{
    for (auto item : m_items) {
        if (item && item->IsActive()) {
            item->Draw();
        }
    }
}

void ItemGenerator::CheckPickup(Player* player)
{
    if (!player) return;

    for (auto item : m_items) {
        if (item && item->IsActive()) {
            if (item->CheckCollision(player)) {
                // �A�C�e�����s�b�N�A�b�v�����Ƃ��̏���
                switch (item->GetType()) {
                    case ItemType::SpeedBoost:
                        // �v���C���[�̑��x���ꎞ�I�ɏグ��i������j
                        player->SetBaseSpeed(6.0f);
                        break;
                    case ItemType::ChargeTank:
                        // �v���C���[�̗̑͂���
                        player->Heal(25);
                        break;
                    case ItemType::ElectricBoost:
                        // ���d���Ԃ𑝉��i�����\��j
                        break;
                }
            }
        }
    }
}
