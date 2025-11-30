/*==============================================================================

   スコアの管理と表示 [game_score.cpp]
														 Author : sumi rintarou
														 Date   : 2025/09/05
--------------------------------------------------------------------------------

==============================================================================*/
#include "game_score.h"
#include "texture.h"
#include "sprite.h"
#include <algorithm>


static int g_ScoreTexId = -1;
static int g_Score = 0;
static int g_Dight = 0;
static float g_Dx = 0.0f, g_Dy = 0.0f;
static int g_CountStopValue = 0;
static int g_ShowScore = 0;
static int g_gazou_waku = 0;

void numberDraw(int number, float x, float y);

void Score_Initialize(float x, float y, int dight)
{
	g_ScoreTexId = Texture_Load(L"texture/suuretu_02.png");
	g_gazou_waku = Texture_Load(L"texture/gazou_waku.png");

	g_Score = 0;
	g_Dight = dight;
	g_Dx = x;
	g_Dy = y;
	g_ShowScore = 0;

	//カンスト値
	g_CountStopValue = 1;
	for (int i = 0; i < g_Dight; i++)
	{
		g_CountStopValue *= 10;
	}
	g_CountStopValue--;

}

void Score_Finalize()
{
}

void Score_Draw()
{
	// 枠の描画
	Sprite_Draw(g_gazou_waku, -17, -95, 666, 375, 0, 0, 666, 375);// 枠

	int temp = std::min(g_Score, g_CountStopValue);

	for (int i = 0; i < g_Dight; i++)
	{
		int dd = g_Dight - 1 - i;
		int n = temp % 10;
		numberDraw(n, g_Dx + 50 * dd, g_Dy);
		temp /= 10;
	}
}

void Score_Update()
{
	if (g_ShowScore < g_Score)
	{
		g_ShowScore++;
	}
}

void Score_AddScore(int score)
{
	g_ShowScore = g_Score;
	g_Score += score;
}

void numberDraw(int number, float x, float y)
{
	Sprite_Draw(g_ScoreTexId, x, y, 700 / 10, 437 / 5, number * 69, 100, 700 / 10, 700 / 10);// 灰

	//Sprite_Draw(g_ScoreTexId, x, y, 700 / 10, 437 / 5, number * 69, 270, 700 / 10, 700 / 10);// 赤
}

int GetScore()
{
	return g_Score;
}