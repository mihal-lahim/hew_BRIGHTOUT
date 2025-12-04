#pragma once
#include <DirectXMath.h>
#include "GameObject.h"
#include "collision.h"

using namespace DirectX;

// ハウスクラス：GameObjectを継承
class House : public GameObject
{
public:
    House(const XMFLOAT3& pos, struct MODEL* model, float maxElectricity = 100.0f);
    virtual ~House() = default;

    virtual void Update(double elapsed) override;  // 毎フレーム更新
    virtual void Draw() const override;             // 描画
    virtual AABB GetAABB() const override;         // 衝突判定用AABB

    // 電気量管理
    float GetElectricity() const { return m_electricity; }
    float GetMaxElectricity() const { return m_maxElectricity; }
    
    // 復旧状態判定（満タンか）
    bool IsRepaired() const { return m_electricity >= m_maxElectricity; }
    
    // 復旧状態設定
    void SetRepaired(bool repaired);

    // プレイヤーとの距離をチェック
    float GetDistanceToPlayer(const XMFLOAT3& playerPos) const;
    
    // プレイヤーが近い範囲内かチェック
    bool IsPlayerNearby(const XMFLOAT3& playerPos, float radius = 5.0f) const;

    // 電気供給を受け取るメソッドを追加
    void ReceiveElectricity(float amount);

    // スケール設定・取得
    void SetScale(float scale) { m_scale = scale; }
    float GetScale() const { return m_scale; }

private:
    struct MODEL* m_model;              // ハウスモデル
    float m_electricity;                // 現在の電気量
    float m_maxElectricity;             // 最大電気量
    bool m_isRepaired;                  // 復旧状態フラグ
    float m_effectTimer;                // 視覚効果用タイマー
    float m_scale = 1.0f;               // スケール（デフォルト1.0f）
    
    // ハウスサイズ（AABB計算用）
    static constexpr float HOUSE_SIZE = 2.0f;
    
    // 当たり判定縮小係数（モデルの表示サイズより小さい当たり判定を設定）
    static constexpr float AABB_SCALE_FACTOR = 0.5f;
};



