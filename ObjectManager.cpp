#include "ObjectManager.h"
#include "texture.h"
#include "Pole.h"
#include "PowerLine.h"
#include "ItemGeneratorObject.h"
#include "DebugAABB.h"
#include <algorithm>
#include <cmath>

// グローバルインスタンス定義
ObjectManager g_ObjectManager;

void ObjectManager::Initialize()
{
    Finalize();

    // ObjectManager は game.cpp から明示的にオブジェクトを追加されることを前提に
    // ここでは何も生成しない
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
        Pole* pole = dynamic_cast<Pole*>(obj.get());
        if (pole && pole->GetPoleID() == poleID) {
            return pole;
        }
    }
    return nullptr;
}

std::vector<class Pole*> ObjectManager::GetAllPoles()
{
    std::vector<Pole*> poles;
    for (const auto& obj : m_GameObjects) {
        Pole* pole = dynamic_cast<Pole*>(obj.get());
        if (pole) {
            poles.push_back(pole);
        }
    }
    return poles;
}

void ObjectManager::ConnectNearbyPoles()
{
    std::vector<Pole*> poles = GetAllPoles();
    
    // 各電柱について、最も近い電柱を見つけて電線でつなぐ
    for (size_t i = 0; i < poles.size(); ++i) {
        Pole* currentPole = poles[i];
        
        // 最も近い電柱を見つける
        float minDistance = FLT_MAX;
        Pole* nearestPole = nullptr;
        
        for (size_t j = 0; j < poles.size(); ++j) {
            if (i == j) continue;
            
            Pole* otherPole = poles[j];
            
            // 2つの電柱の上端位置の距離を計算
            DirectX::XMFLOAT3 pos1 = currentPole->GetTopPosition();
            DirectX::XMFLOAT3 pos2 = otherPole->GetTopPosition();
            
            float dx = pos2.x - pos1.x;
            float dy = pos2.y - pos1.y;
            float dz = pos2.z - pos1.z;
            
            float distance = std::sqrt(dx * dx + dy * dy + dz * dz);
            
            if (distance < minDistance) {
                minDistance = distance;
                nearestPole = otherPole;
            }
        }
        
        // 最も近い電柱が見つかった場合、電線でつなぐ
        if (nearestPole && minDistance < 30.0f) {  // 30ユニット以内
            DirectX::XMFLOAT3 startPos = currentPole->GetTopPosition();
            DirectX::XMFLOAT3 endPos = nearestPole->GetTopPosition();
            
            // 既に接続されているかチェック（重複防止）
            bool alreadyConnected = false;
            for (const auto& obj : m_GameObjects) {
                PowerLine* line = dynamic_cast<PowerLine*>(obj.get());
                if (line) {
                    DirectX::XMFLOAT3 lineStart = line->GetStartPosition();
                    DirectX::XMFLOAT3 lineEnd = line->GetEndPosition();
                    
                    // 接続が逆順でも一致したら重複と判定
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
            
            // 接続されていなければ電線を追加
            if (!alreadyConnected) {
                AddGameObject(std::make_unique<PowerLine>(startPos, endPos, 0.2f));
            }
        }
    }
}

std::vector<class ItemGeneratorObject*> ObjectManager::GetAllItemGenerators()
{
    std::vector<ItemGeneratorObject*> generators;
    for (const auto& obj : m_GameObjects) {
        ItemGeneratorObject* generator = dynamic_cast<ItemGeneratorObject*>(obj.get());
        if (generator) {
            generators.push_back(generator);
        }
    }
    return generators;
}

class ItemGeneratorObject* ObjectManager::GetItemGeneratorByID(int generatorID)
{
    for (const auto& obj : m_GameObjects) {
        ItemGeneratorObject* generator = dynamic_cast<ItemGeneratorObject*>(obj.get());
        if (generator && generator->GetGeneratorID() == generatorID) {
            return generator;
        }
    }
    return nullptr;
}

void ObjectManager::UpdateItemGenerators(double elapsedTime)
{
    std::vector<ItemGeneratorObject*> generators = GetAllItemGenerators();
    for (auto generator : generators) {
        if (generator) {
            generator->Update(elapsedTime);
        }
    }
}

void ObjectManager::DrawDebugAABBs() const
{
    if (!m_debugAABBEnabled)
        return;

    // すべてのゲームオブジェクトのAABBを描画
    for (const auto& obj : m_GameObjects)
    {
        if (obj)
        {
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

std::vector<Pole*> ObjectManager::GetAllPoles() const
{
    std::vector<Pole*> poles;
    for (const auto& obj : m_GameObjects)
    {
        Pole* pole = dynamic_cast<Pole*>(obj.get());
        if (pole)
        {
            poles.push_back(pole);
        }
    }
    return poles;
}

std::vector<PowerLine*> ObjectManager::GetAllPowerLines() const
{
    std::vector<PowerLine*> powerLines;
    for (const auto& obj : m_GameObjects)
    {
        PowerLine* line = dynamic_cast<PowerLine*>(obj.get());
        if (line)
        {
            powerLines.push_back(line);
        }
    }
    return powerLines;
}
