#include "result.h"
#include "game_score.h"
#include "texture.h"
#include "sprite.h"
#include "fade.h"
#include "key_logger.h"
#include "Audio.h"
#include "scene.h"

static int g_TexTitId = -1;
static int g_TexPushAnyKeyId = -1;
static double g_AccumulatedTime = 0.0f;
static double g_KeyInputTime = 0.0f;

enum ResultState
{
	RESULT_STATE_FADE_IN,
	RESULT_STATE_KEYINPUT_WAIT,
	RESULT_STATE_KEYINPUT_ACTION,
	RESULT_STATE_FADE_OUT,
	RESULT_STATE_GAME_END,
	RESULT_STATE_STATE_MAX
};

static ResultState g_State = RESULT_STATE_FADE_IN;

void Result_Initialize()
{
	g_TexTitId = Texture_Load(L"texture/game_haikei.png");
	g_TexPushAnyKeyId = Texture_Load(L"texture/plzA.png");
	g_AccumulatedTime = 0.0f;
	g_State = RESULT_STATE_FADE_IN;

	Fade_Start(2.0f, true);
}

void Result_Finalize()
{
	Fade_Start(2.0f, false);
}

void Result_Update(double elapsed_time)
{
	g_AccumulatedTime += elapsed_time;

	switch (g_State)
	{
	case RESULT_STATE_FADE_IN:
		if (Fade_GetState() == FADE_STATE_FADE_IN_FINISHED)
		{
			g_State = RESULT_STATE_KEYINPUT_WAIT;
		}
		break;
	case RESULT_STATE_KEYINPUT_WAIT:
		if (KeyLogger_IsTrigger(KK_ENTER) || KeyLogger_IsTrigger(KK_A))
		{
			g_State = RESULT_STATE_KEYINPUT_ACTION;
			g_KeyInputTime = g_AccumulatedTime;
			// サウンド
		}
		break;
	case RESULT_STATE_KEYINPUT_ACTION:

		if (g_AccumulatedTime - g_KeyInputTime > 1.0f)
		{
			g_State = RESULT_STATE_FADE_OUT;
			Fade_Start(1.0f, false);
		}
		break;
	case RESULT_STATE_FADE_OUT:
		if (Fade_GetState() == FADE_STATE_FADE_OUT_FINISHED)
		{
			//ゲームシーンへ移行
			Scene_SetNextScene(SCENE_GAME);
		}
		break;
	}
}


void Result_Draw()
{
	Sprite_Draw(g_TexTitId, 0.0f, 0.0f, 1.0f);

	if (g_State != RESULT_STATE_FADE_IN)
	{
		float speed = g_State == RESULT_STATE_KEYINPUT_WAIT ? 4 : 16;

		float alpha = (sin(g_AccumulatedTime * speed) + 1.0) * 0.5;
		Sprite_Draw(g_TexPushAnyKeyId, 1920 / 2 - Texture_GetWidth(g_TexPushAnyKeyId) / 2, 500.0f, 1.0f, { 1.0f,1.0f,1.0f,alpha });
	}

	Score_Draw();
	//GetScore();
}
