#include "item.h"
#include "player.h"
#include "cube.h"
#include "texture.h"
#include "model.h"
#include <cmath>

Item::Item(const XMFLOAT3& pos, ItemType type, float size)
    : GameObject(pos, nullptr, -1, { size, size, size }),
      m_size(size),
      m_active(true),
      m_effectTimer(0.0),
      m_type(type),
      m_textureID(-1)
{
    // タイプに応じてテクスチャをロード
    LoadTextureForType(type);
}

void Item::LoadTextureForType(ItemType type)
{
    // タイプごとにテクスチャファイルを指定してロード
    switch (type) {
        case ItemType::SpeedBoost:
            // 速度アップアイテム用テクスチャ（黄色系）
            m_textureID = Texture_Load(L"texture/God.png");
            if (m_textureID == -1) {
                // フォールバック：デフォルトテクスチャ
                m_textureID = Texture_Load(L"texture/siro.png");
            }
            break;

        case ItemType::ChargeTank:
            // 充電容量アイテム用テクスチャ（シアン系）
            m_textureID = Texture_Load(L"texture/kakutyoutanku.png");
            if (m_textureID == -1) {
                // フォールバック：デフォルトテクスチャ
                m_textureID = Texture_Load(L"texture/siro.png");
            }
            break;

        case ItemType::ElectricBoost:
            // 放電時間アイテム用テクスチャ（マゼンタ系）
            m_textureID = Texture_Load(L"texture/BRIGHTOUT_map_juuden.png");
            if (m_textureID == -1) {
                // フォールバック：デフォルトテクスチャ
                m_textureID = Texture_Load(L"texture/siro.png");
            }
            break;

        default:
            // 不明なタイプの場合
            m_textureID = Texture_Load(L"texture/siro.png");
            break;
    }
}

void Item::Update(double elapsed)
{
    if (!m_active) return;

    // 効果タイマー更新（必要に応じて）
    m_effectTimer += elapsed;

    // ゆっくり上下に浮遊する動き
    float bobAmount = static_cast<float>(std::sin(m_effectTimer * 2.0) * 0.3f);
    m_Position.y += static_cast<float>(bobAmount * elapsed);
}

DirectX::XMFLOAT4 Item::GetColor() const
{
    // アイテムのタイプに応じて色を返す
    switch (m_type) {
        case ItemType::SpeedBoost:
            return { 1.0f, 1.0f, 0.0f, 1.0f };  // 黄色：速度アップ
        case ItemType::ChargeTank:
            return { 0.0f, 1.0f, 1.0f, 1.0f };  // シアン：充電容量
        case ItemType::ElectricBoost:
            return { 1.0f, 0.0f, 1.0f, 1.0f };  // マゼンタ：放電時間
        default:
            return { 1.0f, 1.0f, 1.0f, 1.0f };  // 白：その他
    }
}

void Item::Draw() const
{
    if (!m_active) return;

    // モデルがあればモデル描画、なければテクスチャ描画
    if (m_pModel != nullptr) {
        DrawWithModel();
    } else {
        DrawWithTexture();
    }
}

void Item::DrawWithModel() const
{
    XMMATRIX scale = XMMatrixScaling(m_size, m_size, m_size);
    XMMATRIX translation = XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z);
    XMMATRIX world = scale * translation;
    
    ModelDraw(m_pModel, world);
}

void Item::DrawWithTexture() const
{
    // テクスチャを使用して描画
    {
        XMMATRIX scale = XMMatrixScaling(m_size, m_size, m_size);
        XMMATRIX translation = XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z);
        XMMATRIX world = scale * translation;
        
        // ロード済みテクスチャを設定
        if (m_textureID != -1) {
            Texture_SetTexture(m_textureID);
        }
        
        Cube_Draw(m_textureID, world);
    }
}

AABB Item::GetAABB() const
{
    XMFLOAT3 halfSize = { m_size * 0.5f, m_size * 0.5f, m_size * 0.5f };
    XMFLOAT3 min = {
        m_Position.x - halfSize.x,
        m_Position.y - halfSize.y,
        m_Position.z - halfSize.z
    };
    XMFLOAT3 max = {
        m_Position.x + halfSize.x,
        m_Position.y + halfSize.y,
        m_Position.z + halfSize.z
    };
    return AABB(min, max);
}

bool Item::CheckCollision(Player* player)
{
    if (!m_active || !player) return false;

    // プレイヤーの AABB と比較
    AABB playerAABB = player->GetAABB();
    AABB itemAABB = GetAABB();

    // AABB 衝突判定
    if (playerAABB.IsOverlap(itemAABB)) {
        m_active = false;  // アイテムを非アクティブにする
        return true;
    }
    return false;
}