#ifndef OBJECT_MANAGER_H
#define OBJECT_MANAGER_H

#include <vector>
#include <memory>
#include <DirectXMath.h>
#include "GameObject.h"
#include "debug_console.h"

// フォワード宣言
class Player;
class Enemy;
class Controller;

class ObjectManager
{
private:
    std::vector<std::unique_ptr<GameObject>> m_GameObjects;
    float m_poleConnectionDistance = 30.0f;  // 電柱接続の最大距離（デフォルト: 30.0f）

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
            case GameObjectTag::ENEMY: tagName = "Enemy"; break;
            default: tagName = "Unknown"; break;
        }
        
        DEBUG_LOGF("[ObjectManager] Added object: %s (Total: %zu)", 
            tagName, m_GameObjects.size() + 1);
        
        m_GameObjects.push_back(std::move(obj));
    }

    // 電柱を管理するメソッド
    class Pole* GetPoleByID(int poleID);
    void ConnectPolesByID(int poleID1, int poleID2);  // 指定されたID の2つの電柱を手動で接続
    
    // 電柱接続距離を設定するメソッド
    void SetPoleConnectionDistance(float distance) { m_poleConnectionDistance = distance; }
    float GetPoleConnectionDistance() const { return m_poleConnectionDistance; }

    // アイテムジェネレータオブジェクトを管理するメソッド
    class ItemGeneratorObject* GetItemGeneratorByID(int generatorID);

    // デバッグ用：すべてのAABBを描画
    void DrawDebugAABBs() const;
    void SetDebugAABBEnabled(bool enabled);
    bool IsDebugAABBEnabled() const;

    // オブジェクト作成ヘルパー関数
    // rotationY: Y軸回転角度（単位：0～360度、デフォルト0度）
    void CreateHouse(const DirectX::XMFLOAT3& position, float scale, float maxElectricity, 
                     MODEL* model = nullptr, float rotationY = 0.0f);
    void CreatePole(const DirectX::XMFLOAT3& position, float height, float radius, int& poleID, 
                    MODEL* model = nullptr, float rotationY = 0.0f);
    void CreateItemGenerator(const DirectX::XMFLOAT3& position, float spawnRadius, float spawnInterval, 
                            int& generatorID, float rotationY = 0.0f);
    void CreateChargingSpot(const DirectX::XMFLOAT3& position, float chargeRadius, float chargeRate, 
                           MODEL* model = nullptr, float rotationY = 0.0f);
    
    // Player を ObjectManager に追加して返す
    Player* CreatePlayer(int playerId, const DirectX::XMFLOAT3& pos,
                        MODEL* model, MODEL* electricModel,
                        const DirectX::XMFLOAT3& dir, class Controller* controller);
    
    // Enemy を ObjectManager に追加して返す
    Enemy* CreateEnemy(const DirectX::XMFLOAT3& position, MODEL* model, float maxHealth);

private:
    bool m_debugAABBEnabled = true;
};

// グローバルインスタンス
extern ObjectManager g_ObjectManager;

#endif // OBJECT_MANAGER_H
