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
    Pole(const XMFLOAT3& pos, MODEL* model = nullptr, float height = 4.0f, float radius = 0.2f);
    virtual ~Pole() = default;

    virtual void Update(double elapsedTime) override;  // 更新処理
    virtual void Draw() const override;                // 描画処理
    virtual AABB GetAABB() const override;            // 衝突判定用AABB

    // 電柱のプロパティ取得
    float GetHeight() const { return m_height; }
    float GetRadius() const { return m_radius; }
    
    // 電柱の頂上位置を取得
    XMFLOAT3 GetTopPosition() const 
    { 
        return { m_Position.x, m_Position.y + m_height, m_Position.z }; 
    }

    // 点と電柱が衝突しているか判定
    bool CheckCollisionWithPoint(const XMFLOAT3& point) const;

    // 電柱ID管理
    void SetPoleID(int id) { m_poleID = id; }
    int GetPoleID() const { return m_poleID; }

    // モデル設定
    void SetModel(MODEL* model) { m_pModel = model; }
    MODEL* GetModel() const { return m_pModel; }

private:
    float m_height;         // 電柱の高さ
    float m_radius;         // 電柱の半径
    float m_rotationSpeed;  // 回転速度（エフェクト用）
    double m_elapsedTime;   // 経過時間（エフェクト用）
    int m_poleID = -1;      // 電柱の識別ID
};
