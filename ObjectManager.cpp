#include "ObjectManager.h"
#include "texture.h"
#include "Pole.h"
#include "PowerLine.h"
#include "ItemGeneratorObject.h"
#include "DebugAABB.h"
#include "house.h"
#include "ChargingSpot.h"
#include "Enemy.h"
#include "Player.h"
#include "debug_console.h"
#include <algorithm>
#include <cmath>

// グローバルインスタンス定義
ObjectManager g_ObjectManager;

void ObjectManager::Initialize()
{
    Finalize();
}

void ObjectManager::Finalize()
{
    m_GameObjects.clear();
}

void ObjectManager::Update(double elapsedTime)
{
    for (const auto& obj : m_GameObjects) {
        obj->Update(elapsedTime);
    }
}

void ObjectManager::Draw() const
{
    for (const auto& obj : m_GameObjects) {
        obj->Draw();
    }
}

class Pole* ObjectManager::GetPoleByID(int poleID)
{
    for (const auto& obj : m_GameObjects) {
        if (obj->GetTag() == GameObjectTag::POLE) {
            Pole* pole = static_cast<Pole*>(obj.get());
            if (pole && pole->GetPoleID() == poleID) {
                return pole;
            }
        }
    }
    return nullptr;
}

void ObjectManager::ConnectPolesByID(int poleID1, int poleID2)
{
    // 指定されたID の2つの電柱を取得
    Pole* pole1 = GetPoleByID(poleID1);
    Pole* pole2 = GetPoleByID(poleID2);
    
    // 両方の電柱が見つからない場合はエラーログを出力して終了
    if (!pole1 || !pole2) {
        if (!pole1) {
            hal::DebugConsole::GetInstance().LogFormat("[ObjectManager] Error: Pole with ID %d not found", poleID1);
        }
        if (!pole2) {
            hal::DebugConsole::GetInstance().LogFormat("[ObjectManager] Error: Pole with ID %d not found", poleID2);
        }
        return;
    }
    
    DirectX::XMFLOAT3 startPos = pole1->GetTopPosition();
    DirectX::XMFLOAT3 endPos = pole2->GetTopPosition();
    
    // 既に接続済みかチェック
    bool alreadyConnected = false;
    for (const auto& obj : m_GameObjects) {
        if (obj->GetTag() == GameObjectTag::POWER_LINE) {
            PowerLine* line = static_cast<PowerLine*>(obj.get());
            if (line) {
                DirectX::XMFLOAT3 lineStart = line->GetStartPosition();
                DirectX::XMFLOAT3 lineEnd = line->GetEndPosition();
                
                float dx1 = lineStart.x - startPos.x;
                float dy1 = lineStart.y - startPos.y;
                float dz1 = lineStart.z - startPos.z;
                float dist1 = std::sqrt(dx1 * dx1 + dy1 * dy1 + dz1 * dz1);
                
                float dx2 = lineEnd.x - endPos.x;
                float dy2 = lineEnd.y - endPos.y;
                float dz2 = lineEnd.z - endPos.z;
                float dist2 = std::sqrt(dx2 * dx2 + dy2 * dy2 + dz2 * dz2);
                
                if (dist1 < 0.1f && dist2 < 0.1f) {
                    alreadyConnected = true;
                    break;
                }
            }
        }
    }
    
    // 未接続の場合は電線を作成して接続
    if (!alreadyConnected) {
        AddGameObject(std::make_unique<PowerLine>(startPos, endPos, 0.2f));
        hal::DebugConsole::GetInstance().LogFormat("[ObjectManager] Connected Pole %d to Pole %d", poleID1, poleID2);
    } else {
        hal::DebugConsole::GetInstance().LogFormat("[ObjectManager] Poles %d and %d are already connected", poleID1, poleID2);
    }
}

class ItemGeneratorObject* ObjectManager::GetItemGeneratorByID(int generatorID)
{
    for (const auto& obj : m_GameObjects) {
        if (obj->GetTag() == GameObjectTag::ITEM_GENERATOR) {
            ItemGeneratorObject* generator = static_cast<ItemGeneratorObject*>(obj.get());
            if (generator && generator->GetGeneratorID() == generatorID) {
                return generator;
            }
        }
    }
    return nullptr;
}

void ObjectManager::DrawDebugAABBs() const
{
    if (!m_debugAABBEnabled)
        return;

    for (const auto& obj : m_GameObjects) {
        if (obj) {
            AABB aabb = obj->GetAABB();
            DebugAABB::DrawAABB(aabb, { 0.0f, 1.0f, 0.0f, 1.0f });
        }
    }
}

void ObjectManager::SetDebugAABBEnabled(bool enabled)
{
    m_debugAABBEnabled = enabled;
    DebugAABB::SetEnabled(enabled);
}

bool ObjectManager::IsDebugAABBEnabled() const
{
    return m_debugAABBEnabled;
}

//============================================================================
// オブジェクト作成ヘルパー関数の実装
//============================================================================

void ObjectManager::CreateHouse(const DirectX::XMFLOAT3& position, float scale, float maxElectricity, struct MODEL* model, float rotationY)
{
    if (!model) return;
    
    // 度数法（0-360度）をラジアン角に変換
    float rotationRad = rotationY * 3.14159f / 180.0f;
    
    auto house = std::make_unique<House>(position, model, maxElectricity);
    house->SetScale(scale);
    house->SetRotation({ 0.0f, rotationRad, 0.0f });
    AddGameObject(std::move(house));
}

void ObjectManager::CreatePole(const DirectX::XMFLOAT3& position, float height, float radius, int& poleID, MODEL* model, float rotationY)
{
    // 度数法（0-360度）をラジアン角に変換
    float rotationRad = rotationY * 3.14159f / 180.0f;
    
    auto pole = std::make_unique<Pole>(position, model, height, radius);
    pole->SetPoleID(poleID++);
    pole->SetRotation({ 0.0f, rotationRad, 0.0f });
    AddGameObject(std::move(pole));
}

void ObjectManager::CreateItemGenerator(const DirectX::XMFLOAT3& position, float spawnRadius, float spawnInterval, int& generatorID, float rotationY)
{
    // 度数法（0-360度）をラジアン角に変換
    float rotationRad = rotationY * 3.14159f / 180.0f;
    
    auto generator = std::make_unique<ItemGeneratorObject>(position, spawnRadius, spawnInterval);
    generator->SetGeneratorID(generatorID++);
    generator->SetRotation({ 0.0f, rotationRad, 0.0f });
    AddGameObject(std::move(generator));
}

void ObjectManager::CreateChargingSpot(const DirectX::XMFLOAT3& position, float chargeRadius, float chargeRate, MODEL* model, float rotationY)
{
    // 度数法（0-360度）をラジアンに変換
    float rotationRad = rotationY * 3.14159f / 180.0f;
    
    auto chargingSpot = std::make_unique<ChargingSpot>(position, chargeRadius, chargeRate, model);
    chargingSpot->SetRotation({ 0.0f, rotationRad, 0.0f });
    AddGameObject(std::move(chargingSpot));
}

// Player を ObjectManager に追加して返す
Player* ObjectManager::CreatePlayer(int playerId, const DirectX::XMFLOAT3& pos,
                                   MODEL* model, MODEL* electricModel,
                                   const DirectX::XMFLOAT3& dir, Controller* controller)
{
    auto player = std::make_unique<Player>(model, electricModel, pos, dir);
    player->SetTag(GameObjectTag::PLAYER);
    
    // コントローラーを割り当て
    if (controller) {
        player->SetController(controller);
    }
    
    // ポインタを取得（AddGameObject の前）
    Player* pPlayer = player.get();
    
    // ObjectManager に追加
    AddGameObject(std::move(player));
    
    DEBUG_LOGF("[ObjectManager] Created Player: ID=%d | Pos=(%.1f, %.1f, %.1f) | HP=100", 
        playerId, pos.x, pos.y, pos.z);
    
    return pPlayer;
}

// Enemy を ObjectManager に追加して返す
Enemy* ObjectManager::CreateEnemy(const DirectX::XMFLOAT3& position, MODEL* model, float maxHealth)
{
    auto enemy = std::make_unique<Enemy>(position, model, maxHealth);
    enemy->SetTag(GameObjectTag::ENEMY);
    
    // ポインタを取得（AddGameObject の前）
    Enemy* pEnemy = enemy.get();
    
    // ObjectManager に追加
    AddGameObject(std::move(enemy));
    
    DEBUG_LOGF("[ObjectManager] Created Enemy: Pos=(%.1f, %.1f, %.1f) | MaxHealth=%.0f", 
        position.x, position.y, position.z, maxHealth);
    
    return pEnemy;
}
