#include "ObjectManager.h"
#include "texture.h"
#include "Pole.h"
#include "PowerLine.h"
#include "ItemGeneratorObject.h"
#include "DebugAABB.h"
#include "house.h"
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

void ObjectManager::ConnectNearbyPoles()
{
    // すべての電柱を集める
    std::vector<Pole*> poles;
    for (const auto& obj : m_GameObjects) {
        if (obj->GetTag() == GameObjectTag::POLE) {
            Pole* pole = static_cast<Pole*>(obj.get());
            if (pole) poles.push_back(pole);
        }
    }
    
    for (size_t i = 0; i < poles.size(); ++i) {
        Pole* currentPole = poles[i];
        float minDistance = FLT_MAX;
        Pole* nearestPole = nullptr;
        
        for (size_t j = 0; j < poles.size(); ++j) {
            if (i == j) continue;
            Pole* otherPole = poles[j];
            
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
        
        if (nearestPole && minDistance < 30.0f) {
            DirectX::XMFLOAT3 startPos = currentPole->GetTopPosition();
            DirectX::XMFLOAT3 endPos = nearestPole->GetTopPosition();
            
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
            
            if (!alreadyConnected) {
                AddGameObject(std::make_unique<PowerLine>(startPos, endPos, 0.2f));
            }
        }
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
