/*==============================================================================

   フェード処理[fade.cpp]
														 Author : sumi rintarou
														 Date   : 2025/09/10
--------------------------------------------------------------------------------

==============================================================================*/

#include "fade.h"
#include "sprite.h"
#include "texture.h"
using namespace DirectX;
#include "direct3d.h"

static FadeState g_FadeState = FADE_STATE_NONE;
static double g_FadeTime = 0.0f;
static double g_AccumuLatedTime = 0.0f;
static double g_FadeStartTime = 0.0f;
static XMFLOAT4 g_FadeColor = { 0.0f,0.0f,0.0f,0.0f };
static int g_TexId = -1;

void Fade_Initialize()
{
	FadeState g_FadeState = FADE_STATE_NONE;
	g_AccumuLatedTime = 0.0f;
	g_TexId = Texture_Load(L"texture/siro.png");
}

void Fade_Finalize()
{
}

void Fade_Update(double elapsed_time)
{
	// 時間計測とステートの管理
	if (g_FadeState == FADE_STATE_NONE ||
		g_FadeState == FADE_STATE_FADE_IN_FINISHED ||
		g_FadeState == FADE_STATE_FADE_OUT_FINISHED)
	{
		return;
	}

	double lifeTime = g_AccumuLatedTime - g_FadeStartTime;

	// 透明度の計算
	float alpha = 0.0f;
	if (g_FadeTime == 0.0f)
	{
		alpha = 1.0f;
	}
	else
	{
		alpha = static_cast<float>(lifeTime / g_FadeTime);
	}

	g_FadeColor.w = g_FadeState == FADE_STATE_FADE_IN ? (1.0f - alpha) : alpha;


	if (g_FadeTime <= lifeTime)
	{
		g_FadeState = g_FadeState == FADE_STATE_FADE_IN ? FADE_STATE_FADE_IN_FINISHED : FADE_STATE_FADE_OUT_FINISHED;
	}

	g_AccumuLatedTime += elapsed_time;
}

void Fade_Draw()
{
	if (g_FadeState == FADE_STATE_NONE ||
		g_FadeState == FADE_STATE_FADE_IN_FINISHED)
	{
		return;
	}

	Sprite_Draw(g_TexId, 0.0f, 0.0f,
		Direct3D_GetBackBufferWidth(),
		Direct3D_GetBackBufferHeight(),
		0, 0, 8, 8,
		g_FadeColor);
}

void Fade_Start(double fade_time, bool isFadeIn, DirectX::XMFLOAT4 fade_color)
{
	g_FadeTime = fade_time;
	g_FadeState = isFadeIn ? FADE_STATE_FADE_IN : FADE_STATE_FADE_OUT;
	g_FadeStartTime = g_AccumuLatedTime;
	g_FadeColor = fade_color;
}

FadeState Fade_GetState()
{
	return g_FadeState;
}
