/*==============================================================================

   スプライトアニメのヘッダー [sprite_anim.h]
														 Author : sumi rintarou
														 Date   : 2025/06/13
--------------------------------------------------------------------------------

==============================================================================*/
#ifndef SPRITEANIM_ANIM_H
#define SPRITEANIM_ANIM_H
#include <DirectXMath.h>

class AnimPattern
{
private:
	int m_texid;
	int m_pattern_max;
	int m_horizontal_pattern_max;
	double m_second_per_pattern;
	DirectX::XMUINT2 m_start_position;
	DirectX::XMUINT2 m_pattern_size;		// x→w y→hのこと
	bool m_is_loop;
public:
	AnimPattern() = delete;
	AnimPattern(int texid, int pattern_max,int horizontal_pattern_max, double second_per_pattern, DirectX::XMUINT2 start_position, DirectX::XMUINT2 pattern_size, bool is_loop = true)
		: m_texid(texid)
		, m_pattern_max(pattern_max)
		, m_horizontal_pattern_max(horizontal_pattern_max)
		, m_second_per_pattern(second_per_pattern)
		, m_start_position(start_position)
		, m_pattern_size(pattern_size)
		, m_is_loop(is_loop)
	{
	}

	int GetPatternMax() { return m_pattern_max;}
	double GetSecoundPerPattern() { return m_second_per_pattern; }
	bool IsLoop() { return m_is_loop; }
	void Draw(float dx, float dy,float dw,float dh, int pattern_num ,DirectX::XMFLOAT4 color = { 1.0f,1.0f,1.0f,1.0f });
};

class AnimPatternPlayer
{
private:
	int m_pattern = 0;
	double m_accumulated_time = 0.0f;		// 累積時間
	AnimPattern* m_pAnimPattern = nullptr;	// 再生すべきパターン番号

public:
	AnimPatternPlayer() = delete;
	AnimPatternPlayer(AnimPattern* pAnimPattern)
		:m_pAnimPattern(pAnimPattern) {
	}

	void Update(double elapsed_time);
		
	void Draw(float dx, float dy, float dw, float dh, DirectX::XMFLOAT4 color = { 1.0f,1.0f,1.0f,1.0f });
};

#endif // SPRITEANIM_ANIM_H

