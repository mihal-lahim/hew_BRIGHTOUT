////////////////////////////////
//当たり判定
//Author : hioshi kasiwagi
//Date : 2025/11/18
/////////////////////////////////
#ifndef COLLISION_H
#define COLLISION_H

#include <DirectXmath.h>


class Hit
{
private:
	bool m_isHit{ false };//当たったかどうか
	DirectX::XMFLOAT3 m_normal{};//当たった面の法線
public:
	Hit() = default;
	Hit(bool isHit,const DirectX::XMFLOAT3& normal)
		:m_isHit(isHit), m_normal(normal)
	{
	}
	bool IsHit() const { return m_isHit; }
	const DirectX::XMFLOAT3& GetNormal() const { return m_normal; }
};

//前方宣言
class AABB;

class Collision
{
public:

	//引数のAABBと自分自身のAABBが重なっているかどうかを調べる
	virtual ~Collision() = default;
	virtual bool IsOverlap(const AABB& aabb) { return false; }
	virtual Hit IsHit(const AABB& aabb) { return {}; }
};

class AABB : public Collision
{
private:
	DirectX::XMFLOAT3 m_Min{};
	DirectX::XMFLOAT3 m_Max{};
	DirectX::XMFLOAT3 m_Center{};
public:
	AABB() = default;
	AABB(const DirectX::XMFLOAT3& min, const DirectX::XMFLOAT3& max)
		:m_Min(min), m_Max(max)
	{
		m_Center.x = (min.x + max.x) / 2.0f;
		m_Center.y = (min.y + max.y) / 2.0f;
		m_Center.z = (min.z + max.z) / 2.0f;
	}
	bool IsOverlap(const AABB& aabb) override;
	Hit IsHit(const AABB& aabb) override;

	const DirectX::XMFLOAT3& GetMin() const { return m_Min; }
	const DirectX::XMFLOAT3& GetMax() const { return m_Max; }
	const DirectX::XMFLOAT3& GetCenter() const { return m_Center; }
};



#endif //COLLISION_H