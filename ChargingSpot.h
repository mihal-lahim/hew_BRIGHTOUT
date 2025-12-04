#ifndef CHARGING_SPOT_H
#define CHARGING_SPOT_H

#include "GameObject.h"
#include <DirectXMath.h>

// 充電スポットクラス
class ChargingSpot : public GameObject
{
public:
	ChargingSpot(const DirectX::XMFLOAT3& pos, float radius = 5.0f, float chargeRate = 20.0f, MODEL* model = nullptr);
	virtual ~ChargingSpot() = default;

	virtual void Update(double elapsed) override;
	virtual void Draw() const override;
	virtual AABB GetAABB() const override;

	// 充電スポットの範囲を取得
	float GetRadius() const { return m_radius; }

	// 充電レートを取得
	float GetChargeRate() const { return m_chargeRate; }

	// 充電レートを設定
	void SetChargeRate(float rate) { m_chargeRate = rate; }

	// 指定位置がスポット範囲内かチェック
	bool IsPlayerInRange(const DirectX::XMFLOAT3& playerPos) const;

private:
	float m_radius;       // 充電範囲（半径）
	float m_chargeRate = 5.0f;   // 1秒あたりの回復量
	float m_effectTimer;  // エフェクト用タイマー

	// スポットサイズ
	static constexpr float SPOT_SIZE = 1.0f;
};

#endif // CHARGING_SPOT_H
