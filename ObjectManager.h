#ifndef OBJECT_MANAGER_H
#define OBJECT_MANAGER_H

#include <vector>
#include <memory>
#include <DirectXMath.h>
#include "GameObject.h"
#include "debug_console.h"

class ObjectManager
{
private:
    std::vector<std::unique_ptr<GameObject>> m_GameObjects;

public:
    ObjectManager() = default;
    ~ObjectManager() = default;

    void Initialize();
    void Finalize();
    void Update(double elapsedTime);
    void Draw() const;

    // ゲームオブジェクトを取得するメソッド
    const std::vector<std::unique_ptr<GameObject>>& GetGameObjects() const { return m_GameObjects; }
    
    // ゲームオブジェクトを追加するメソッド
    void AddGameObject(std::unique_ptr<GameObject> obj)
    {
        if (!obj) return;
        
        // デバッグログ出力
        const char* tagName = "Unknown";
        switch (obj->GetTag()) {
            case GameObjectTag::POLE: tagName = "Pole"; break;
            case GameObjectTag::POWER_LINE: tagName = "PowerLine"; break;
            case GameObjectTag::HOUSE: tagName = "House"; break;
            case GameObjectTag::ITEM_GENERATOR: tagName = "ItemGenerator"; break;
            case GameObjectTag::CHARGING_SPOT: tagName = "ChargingSpot"; break;
            default: tagName = "Unknown"; break;
        }
        
        DEBUG_LOGF("[ObjectManager] Added object: %s (Total: %zu)", 
            tagName, m_GameObjects.size() + 1);
        
        m_GameObjects.push_back(std::move(obj));
    }

    // 電柱を管理するメソッド
    class Pole* GetPoleByID(int poleID);
    void ConnectNearbyPoles();  // 近い電柱同士を電線で接続

    // アイテムジェネレータオブジェクトを管理するメソッド
    class ItemGeneratorObject* GetItemGeneratorByID(int generatorID);

    // デバッグ用：すべてのAABBを描画
    void DrawDebugAABBs() const;
    void SetDebugAABBEnabled(bool enabled);
    bool IsDebugAABBEnabled() const;

    // オブジェクト作成ヘルパー関数
    void CreateHouse(const DirectX::XMFLOAT3& position, float scale, float maxElectricity, struct MODEL* model = nullptr);
    void CreatePole(const DirectX::XMFLOAT3& position, float height, float radius, int& poleID);
    void CreateItemGenerator(const DirectX::XMFLOAT3& position, float spawnRadius, float spawnInterval, int& generatorID);
    void CreateChargingSpot(const DirectX::XMFLOAT3& position, float chargeRadius, float chargeRate);

private:
    bool m_debugAABBEnabled = true;
};

// グローバルインスタンス
extern ObjectManager g_ObjectManager;

#endif // OBJECT_MANAGER_H
