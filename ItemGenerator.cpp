#include "ItemGenerator.h"
#include "player.h"
#include <cstdlib>
#include <ctime>

ItemGenerator::~ItemGenerator()
{
    // すべてのアイテムをクリーンアップ
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

    // ランダムシードを初期化
    srand(static_cast<unsigned>(time(nullptr)));
}

MODEL* ItemGenerator::GetModelForType(ItemType type) const
{
    switch (type) {
        case ItemType::SpeedBoost:
            return m_speedBoostModel;
        case ItemType::ChargeTank:
            return m_chargeTankModel;
        case ItemType::ElectricBoost:
            return m_electricBoostModel;
        default:
            return nullptr;
    }
}

void ItemGenerator::GenerateItem(const XMFLOAT3& pos, ItemType type)
{
    // 新しいアイテムを生成
    Item* newItem = new Item(pos, type, 0.5f);
    // タイプに応じたモデルを設定
    newItem->SetModel(GetModelForType(type));
    m_items.push_back(newItem);
}

void ItemGenerator::RegisterItem(Item* item)
{
    // 外部から生成されたアイテムを登録（ItemGeneratorObject から呼び出される）
    if (item) {
        // タイプに応じたモデルを設定
        item->SetModel(GetModelForType(item->GetType()));
        m_items.push_back(item);
    }
}

void ItemGenerator::GenerateItemsOnPowerLines(const Map& map)
{
    // 電線座標を初期化
    m_powerLinePositions.clear();
    
    // 電線の中点位置にアイテムを配置
    m_powerLinePositions.push_back({ -0.0f, 5.0f, 0.0f });   // 水平電線1の中点
    m_powerLinePositions.push_back({ 0.0f, 5.0f, 0.0f });    // 鉛直電線の中点
    m_powerLinePositions.push_back({ 0.0f, 8.0f, 10.0f });   // 水平電線2の中点
    
    // 各電線位置の周辺にアイテムを生成
    for (const auto& linePos : m_powerLinePositions) {
        // アイテムを直接電線位置に配置
        ItemType randomType = static_cast<ItemType>(rand() % 3);
        GenerateItem(linePos, randomType);
    }
}

void ItemGenerator::Update(double elapsed)
{
    // 各アイテムを更新
    for (auto item : m_items) {
        if (item && item->IsActive()) {
            item->Update(elapsed);
        }
    }

    // 無効なアイテムを削除
    auto it = m_items.begin();
    while (it != m_items.end()) {
        if (!(*it)->IsActive()) {
            delete *it;
            it = m_items.erase(it);
        } else {
            ++it;
        }
    }

    // 定期的に新しいアイテムを生成
    m_spawnTimer += elapsed;
    if (m_spawnTimer >= m_spawnInterval) {
        m_spawnTimer = 0.0;
        
        // ランダムな位置に新しいアイテムを生成
        float randomX = static_cast<float>(rand() % 20 - 10);  // -10 から 10
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
                // アイテムをピックアップしたときの処理
                switch (item->GetType()) {
                    case ItemType::SpeedBoost:
                        // プレイヤーの速度を一時的に上げる（実装例）
                        player->SetBaseSpeed(6.0f);
                        break;
                    case ItemType::ChargeTank:
                        // プレイヤーの体力を回復
                        player->Heal(25);
                        break;
                    case ItemType::ElectricBoost:
                        // 放電時間を増加（実装予定）
                        break;
                }
            }
        }
    }
}
