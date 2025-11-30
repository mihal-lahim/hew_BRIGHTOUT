

#ifndef GAME_OBJECT_H
#define GAME_OBJECT_H

#include <DirectXMath.h>
#include "collision.h"

struct MODEL; // モデル構造体の前方宣言

// ゲーム内のすべての静的オブジェクトの基底クラス
class GameObject
{
protected:
    DirectX::XMFLOAT3 m_Position{};
    DirectX::XMFLOAT3 m_Scale{ 1.0f, 1.0f, 1.0f };
    DirectX::XMFLOAT3 m_Rotation{}; // オイラー角
    MODEL* m_pModel = nullptr;
    int m_TextureId = -1; // テクスチャID（モデルがない場合）

public:
    GameObject(const DirectX::XMFLOAT3& pos, MODEL* model = nullptr, int textureId = -1, const DirectX::XMFLOAT3& scale = { 1.0f, 1.0f, 1.0f })
        : m_Position(pos), m_pModel(model), m_TextureId(textureId), m_Scale(scale) {}
    virtual ~GameObject() = default;

    virtual void Update(double elapsedTime) {}
    virtual void Draw() const;

    virtual AABB GetAABB() const;

    const DirectX::XMFLOAT3& GetPosition() const { return m_Position; }
};

#endif // GAME_OBJECT_H
