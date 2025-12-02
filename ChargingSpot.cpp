#include "ChargingSpot.h"
#include "cube.h"
#include <cmath>
#include <DirectXMath.h>

using namespace DirectX;

ChargingSpot::ChargingSpot(const XMFLOAT3& pos, float radius, float chargeRate)
	: GameObject(pos, nullptr, -1, { SPOT_SIZE, SPOT_SIZE, SPOT_SIZE }),
	  m_radius(radius),
	  m_chargeRate(chargeRate),
	  m_effectTimer(0.0f)
{
	SetTag(GameObjectTag::CHARGING_SPOT);
}

void ChargingSpot::Update(double elapsed)
{
	// エフェクトタイマー更新
	m_effectTimer += static_cast<float>(elapsed);
	if (m_effectTimer > 2.0f) {
		m_effectTimer = 0.0f;
	}
}

void ChargingSpot::Draw() const
{
	// 充電スポットを視覚化するキューブを描画
	// 中心の小さなキューブ
	{
		XMMATRIX scale = XMMatrixScaling(SPOT_SIZE, SPOT_SIZE, SPOT_SIZE);
		XMMATRIX translation = XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z);
		XMMATRIX world = scale * translation;
		Cube_Draw(1, world);  // テクスチャID 1（青色を想定）
	}

	// 充電範囲を示す外側のリング（透視のため複数の立方体で表現）
	{
		float ringRadius = m_radius;
		int segments = 8;
		for (int i = 0; i < segments; ++i) {
			float angle = (2.0f * 3.14159f * i) / segments;
			float x = m_Position.x + ringRadius * std::cos(angle);
			float z = m_Position.z + ringRadius * std::sin(angle);

			XMMATRIX scale = XMMatrixScaling(0.3f, 0.5f, 0.3f);
			XMMATRIX translation = XMMatrixTranslation(x, m_Position.y, z);
			XMMATRIX world = scale * translation;
			Cube_Draw(2, world);  // テクスチャID 2（補助色）
		}
	}
}

AABB ChargingSpot::GetAABB() const
{
	// スポット自体のAABB（視覚的な表現用）
	float halfSize = SPOT_SIZE * 0.5f;
	XMFLOAT3 min = {
		m_Position.x - halfSize,
		m_Position.y - halfSize,
		m_Position.z - halfSize
	};
	XMFLOAT3 max = {
		m_Position.x + halfSize,
		m_Position.y + halfSize,
		m_Position.z + halfSize
	};
	return AABB(min, max);
}

bool ChargingSpot::IsPlayerInRange(const XMFLOAT3& playerPos) const
{
	float dx = playerPos.x - m_Position.x;
	float dz = playerPos.z - m_Position.z;
	float distance = std::sqrt(dx * dx + dz * dz);
	
	return distance <= m_radius;
}
