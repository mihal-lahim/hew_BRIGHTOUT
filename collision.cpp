//////////////////////////////
// collision.cpp
//Author: Hiroshi Kasiwagi
//Date: 2025/11/18
//////////////////////////////
#include "collision.h"
#include <algorithm>

bool AABB::IsOverlap(const AABB& aabb)
{
	return GetMin().x < aabb.GetMax().x
		&& GetMax().x > aabb.GetMin().x
		&& GetMin().y < aabb.GetMax().y
		&& GetMax().y > aabb.GetMin().y
		&& GetMin().z < aabb.GetMax().z
		&& GetMax().z > aabb.GetMin().z;
}

Hit AABB::IsHit(const AABB& aabb)
{
	//“–‚½‚Á‚Ä‚È‚¯‚ê‚Î•Ô‚·
	if (!IsOverlap(aabb)) return{};

	//“–‚½‚Á‚½Žž
	float depth_x = std::min(GetMax().x, aabb.GetMax().x) - std::max(GetMin().x, aabb.GetMin().x);
	float depth_y = std::min(GetMax().y, aabb.GetMax().y) - std::max(GetMin().y, aabb.GetMin().y);
	float depth_z = std::min(GetMax().z, aabb.GetMax().z) - std::max(GetMin().z, aabb.GetMin().z);

	int shallow_axis = 0;//x:0,y:1,z:2

	if (depth_x < depth_y)
	{
		if (depth_x < depth_z)
		{
			shallow_axis = 0;
		}
		else
		{
			shallow_axis = depth_z;
		}
	}
	else
	{
		if (depth_y < depth_z)
		{
			shallow_axis = 1;
		}
		else
		{
			shallow_axis = 2;
		}
	}
}
