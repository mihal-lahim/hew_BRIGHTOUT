#include "../include/ObjectManager.h"
#include "../include/texture.h" // テクスチャID取得用

// グローバルインスタンス定義
ObjectManager g_ObjectManager;

void ObjectManager::Initialize()
{
    Finalize();

    // テクスチャの事前読み込み（例）
    int poleTexId = Texture_Load(L"texture/pole.png");
    int buildingTexId = Texture_Load(L"texture/building.png");

    // 電柱の配置
    for (int z = 0; z < 5; ++z) {
        for (int x = 0; x < 5; ++x) {
            if ((x % 2 == 0) && (z % 2 == 0)) { // 疎らに配置
                m_GameObjects.emplace_back(std::make_unique<GameObject>(
                    DirectX::XMFLOAT3(x * 8.0f - 16.0f, 0.0f, z * 8.0f - 16.0f),
                    nullptr,
                    poleTexId,
                    DirectX::XMFLOAT3(0.2f, 4.0f, 0.2f)
                ));
            }
        }
    }

    // 建物の配置
    m_GameObjects.emplace_back(std::make_unique<GameObject>(
        DirectX::XMFLOAT3(15.0f, 0.0f, 15.0f),
        nullptr,
        buildingTexId,
        DirectX::XMFLOAT3(5.0f, 10.0f, 5.0f)
    ));
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
