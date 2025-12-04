#ifndef GAME_OBJECT_H
#define GAME_OBJECT_H

#include <DirectXMath.h>
#include "collision.h"
#include <string>

struct MODEL; // モデル構造体の前方宣言

// ゲームオブジェクトのタグ定義
enum class GameObjectTag
{
    UNKNOWN = 0,
    POLE = 1,
    POWER_LINE = 2,
    HOUSE = 3,
    ITEM_GENERATOR = 4,
    CHARGING_SPOT = 5,
    ENEMY = 6,
    PLAYER = 7,
    // 必要に応じてタグを追加
};

// ゲーム内のすべての静的オブジェクトの基底クラス
class GameObject
{
protected:
    DirectX::XMFLOAT3 m_Position{};
    DirectX::XMFLOAT3 m_Scale{ 1.0f, 1.0f, 1.0f };
    DirectX::XMFLOAT3 m_Rotation{}; // オイラー角
    MODEL* m_pModel = nullptr;
    int m_TextureId = -1; // テクスチャID（モデルがない場合）
    GameObjectTag m_Tag = GameObjectTag::UNKNOWN;  // オブジェクトのタグ

public:
    GameObject(const DirectX::XMFLOAT3& pos, MODEL* model = nullptr, int textureId = -1, const DirectX::XMFLOAT3& scale = { 1.0f, 1.0f, 1.0f })
        : m_Position(pos), m_pModel(model), m_TextureId(textureId), m_Scale(scale) {}
    virtual ~GameObject() = default;

    virtual void Update(double elapsedTime) {}
    virtual void Draw() const;

    virtual AABB GetAABB() const;

    const DirectX::XMFLOAT3& GetPosition() const { return m_Position; }

    // 回転取得・設定
    const DirectX::XMFLOAT3& GetRotation() const { return m_Rotation; }
    void SetRotation(const DirectX::XMFLOAT3& rotation) { m_Rotation = rotation; }

    // タグ関連のメソッド
    void SetTag(GameObjectTag tag) { m_Tag = tag; }
    GameObjectTag GetTag() const { return m_Tag; }
    bool HasTag(GameObjectTag tag) const { return m_Tag == tag; }
};

#endif // GAME_OBJECT_H
