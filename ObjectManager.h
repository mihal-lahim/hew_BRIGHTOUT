#ifndef OBJECT_MANAGER_H
#define OBJECT_MANAGER_H

#include <vector>
#include <memory>
#include "GameObject.h"

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

private:
    bool m_debugAABBEnabled = true;
};

// グローバルインスタンス
extern ObjectManager g_ObjectManager;

#endif // OBJECT_MANAGER_H