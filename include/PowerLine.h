#pragma once
#include <DirectXMath.h>
#include "GameObject.h"
#include "collision.h"

using namespace DirectX;

// 電線クラス：GameObjectを継承
// プレイヤーがこれに接触すると放電する
class PowerLine : public GameObject
{
public:
    PowerLine(const XMFLOAT3& startPos, const XMFLOAT3& endPos, float radius = 0.2f);
    virtual ~PowerLine() = default;

    virtual void Update(double elapsedTime) override;  // 更新処理
    virtual void Draw() const override;                // 描画処理
    virtual AABB GetAABB() const override;            // 衝突判定用AABB

    // 電線の両端の位置を取得
    const XMFLOAT3& GetStartPosition() const { return m_startPos; }
    const XMFLOAT3& GetEndPosition() const { return m_endPos; }

    // 電線の半径を取得
    float GetRadius() const { return m_radius; }

    // プレイヤーが電線に接触しているか判定
    bool CheckCollisionWithPoint(const XMFLOAT3& point) const;

    // 点から線分上の最も近いポイントを取得
    DirectX::XMFLOAT3 GetClosestPointOnLine(const DirectX::XMFLOAT3& point) const;

private:
    XMFLOAT3 m_startPos;    // 電線の開始位置
    XMFLOAT3 m_endPos;      // 電線の終了位置
    float m_radius;         // 電線の半径（判定用）
    float m_electricityIntensity = 1.0f;  // 放電の強度

    // 点から線分までの最短距離を計算
    float GetDistancePointToLineSegment(const XMFLOAT3& point) const;
};
