#include "../include/ObjectManager.h"
#include "../include/texture.h" // �e�N�X�`��ID�擾�p

// �O���[�o���C���X�^���X��`
ObjectManager g_ObjectManager;

void ObjectManager::Initialize()
{
    Finalize();

    // �e�N�X�`���̎��O�ǂݍ��݁i��j
    int poleTexId = Texture_Load(L"texture/pole.png");
    int buildingTexId = Texture_Load(L"texture/building.png");

    // �d���̔z�u
    for (int z = 0; z < 5; ++z) {
        for (int x = 0; x < 5; ++x) {
            if ((x % 2 == 0) && (z % 2 == 0)) { // �a��ɔz�u
                m_GameObjects.emplace_back(std::make_unique<GameObject>(
                    DirectX::XMFLOAT3(x * 8.0f - 16.0f, 0.0f, z * 8.0f - 16.0f),
                    nullptr,
                    poleTexId,
                    DirectX::XMFLOAT3(0.2f, 4.0f, 0.2f)
                ));
            }
        }
    }

    // �����̔z�u
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
