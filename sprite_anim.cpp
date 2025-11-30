/*==============================================================================

   スプライトアニメの描画 [sprite_anim.cpp]
														 Author : sumi rintarou
														 Date   : 2025/06/13
--------------------------------------------------------------------------------

==============================================================================*/
#include "sprite.h"
#include "sprite_anim.h"
#include "system_timer.h";
#include <DirectXMath.h>

void AnimPattern::Draw(float dx, float dy,float dw,float dh,int pattern_num, DirectX::XMFLOAT4 color)
{
	//int pattern = pattern_num % m_pattern_max;

	Sprite_Draw(m_texid, dx, dy, dw, dh,
		m_start_position.x + m_pattern_size.x * (pattern_num % m_horizontal_pattern_max),
		m_start_position.y + m_pattern_size.y * (m_horizontal_pattern_max / m_horizontal_pattern_max), // ?
		m_pattern_size.x, m_pattern_size.y,color);
}

void AnimPatternPlayer::Update(double elapsed_time)
{
	m_accumulated_time += elapsed_time; // 累積時間を更新
	if (m_accumulated_time >= m_pAnimPattern->GetSecoundPerPattern())
	{
		m_pattern = (m_pattern + 1) % m_pAnimPattern->GetPatternMax(); // パターン番号を更新
		m_accumulated_time -= m_pAnimPattern->GetSecoundPerPattern();  // 累積時間をリセット
	}
}

// 再生機
void AnimPatternPlayer::Draw(float dx, float dy, float dw, float dh, DirectX::XMFLOAT4 color)
{
	m_pAnimPattern->Draw(dx, dy, dw, dh, m_pattern, color);
}
