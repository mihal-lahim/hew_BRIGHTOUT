#pragma once
#include <DirectXMath.h>
#include "GameObject.h"
#include "collision.h"

using namespace DirectX;

// アイテムの種類
enum class ItemType {
    SpeedBoost,    // 移動速度増加
    ChargeTank,    // 充電容量
    ElectricBoost  // 放電時間付与
};

// アイテムクラス：GameObjectを継承
class Item : public GameObject
{
public:
    Item(const XMFLOAT3& pos, ItemType type, float size = 0.5f) ;
    virtual ~Item() = default;

    virtual void Update(double elapsed) override;  // 効果時間更新
    virtual void Draw() const override;             // 描画
    virtual AABB GetAABB() const override;         // 衝突判定用AABB

    bool CheckCollision(class Player* player);     // プレイヤーとの衝突判定
    bool IsActive() const { return m_active; }
    ItemType GetType() const { return m_type; }

    // アイテムタイプに応じた色を取得
    DirectX::XMFLOAT4 GetColor() const;

    // テクスチャID取得
    int GetTextureID() const { return m_textureID; }

    // モデル設定
    void SetModel(MODEL* model) { m_pModel = model; }

private:
    float m_size;          // 描画サイズ
    bool m_active;         // キャラクターが有効か
    double m_effectTimer;  // 効果時間（必要に応じて使用）
    ItemType m_type;       // アイテムタイプ
    int m_textureID;       // テクスチャID（-1は未ロード）

    // タイプに応じてテクスチャをロード
    void LoadTextureForType(ItemType type);

    // モデル描画用ヘルパー
    void DrawWithModel() const;
    void DrawWithTexture() const;
};