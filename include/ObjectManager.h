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

    const std::vector<std::unique_ptr<GameObject>>& GetGameObjects() const { return m_GameObjects; }
};

// グローバルインスタンス
extern ObjectManager g_ObjectManager;

#endif // OBJECT_MANAGER_H