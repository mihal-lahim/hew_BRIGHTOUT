#pragma once
#include <DirectXMath.h>
#include "GameObject.h"
#include "collision.h"

using namespace DirectX;

// 電柱クラス：GameObjectを継承
// ゲームワールドに配置される静的な電柱
class Pole : public GameObject
{
public:
    // コンストラクタ
    Pole(const XMFLOAT3& pos, float height = 4.0f, float radius = 0.2f);
    virtual ~Pole() = default;

    virtual void Update(double elapsedTime) override;  // 更新処理
    virtual void Draw() const override;                // 描画処理
    virtual AABB GetAABB() const override;            // 衝突判定用AABB

    // 電柱のプロパティを取得
    float GetHeight() const { return m_height; }
    float GetRadius() const { return m_radius; }
    
    // 電柱の上端位置を取得
    XMFLOAT3 GetTopPosition() const 
    { 
        return { m_Position.x, m_Position.y + m_height, m_Position.z }; 
    }

    // 点が電柱と衝突しているか判定
    bool CheckCollisionWithPoint(const XMFLOAT3& point) const;

    // 電柱ID管理
    void SetPoleID(int id) { m_poleID = id; }
    int GetPoleID() const { return m_poleID; }

private:
    float m_height;         // 電柱の高さ
    float m_radius;         // 電柱の半径
    float m_rotationSpeed;  // 回転速度（視覚効果用）
    double m_elapsedTime;   // 経過時間（エフェクト用）
    int m_poleID = -1;      // 電柱の識別ID
};
