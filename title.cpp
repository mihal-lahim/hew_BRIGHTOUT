///////////////////////////////
// Title Scene Module
// Author: hiroshi kasiwagi
// Date: 2025/11/27
///////////////////////////////

#include "title.h"
#include "key_logger.h"
#include "texture.h"
#include "sprite.h"
#include "fade.h"
#include "Audio.h"
#include "direct3d.h"
#include "math.h"
#include "scene.h"
#include "controller.h"
static int g_TexTitId = -1;
static int g_TexPushAnyKeyId = -1;
static double g_AccumulatedTime = 0.0f;
static double g_KeyInputTime = 0.0f;


enum TitleState
{
	TITLE_STATE_FADE_IN,
	TITLE_STATE_KEYINPUT_WAIT,
	TITLE_STATE_KEYINPUT_ACTION,
	TITLE_STATE_FADE_OUT,
	TITLE_STATE_STATE_MAX
};

static TitleState g_State = TITLE_STATE_FADE_IN;

static Controller* g_Controller = nullptr;

void Title_Initialize()
{

	g_TexTitId = Texture_Load(L"texture/game_haikei.png");
	g_TexPushAnyKeyId = Texture_Load(L"texture/plzA.png");
	g_AccumulatedTime = 0.0f;
	g_State = TITLE_STATE_FADE_IN;

	Fade_Start(2.0f, true);
	
	g_Controller = new Controller(0);
};

void Title_Finalize()
{
	Fade_Start(2.0f, false);
	delete g_Controller;
	g_Controller = nullptr;
}


void Title_Update(double elapsed_time)
{
	g_Controller->Update();
	g_AccumulatedTime += elapsed_time;

	switch (g_State)
	{
	case TITLE_STATE_FADE_IN:
		if (Fade_GetState() == FADE_STATE_FADE_IN_FINISHED)
		{
			g_State = TITLE_STATE_KEYINPUT_WAIT;
		}
		break;
	case TITLE_STATE_KEYINPUT_WAIT:
		if (KeyLogger_IsTrigger(KK_ENTER) || KeyLogger_IsTrigger(KK_A))
		{
			g_State = TITLE_STATE_KEYINPUT_ACTION;
			g_KeyInputTime = g_AccumulatedTime;
			// サウンド
		}
		//Aボタン、スタートボタンでも反応
		if(g_Controller->WasPressed(Controller::BUTTON_A) || g_Controller->WasPressed(Controller::BUTTON_START))
		{
			g_State = TITLE_STATE_KEYINPUT_ACTION;
			
		}
		break;
	case TITLE_STATE_KEYINPUT_ACTION:

		if ( g_AccumulatedTime - g_KeyInputTime > 1.0f)
		{
			g_State = TITLE_STATE_FADE_OUT;
			Fade_Start(1.0f, false);
		}
		break;
	case TITLE_STATE_FADE_OUT:
		if (Fade_GetState() == FADE_STATE_FADE_OUT_FINISHED)
		{
			//ゲームシーンへ移行
			Scene_SetNextScene(SCENE_GAME);
		}
		break;
	}
}

void Title_Draw()
{
	Sprite_Draw(g_TexTitId, 0.0f, 0.0f,1.0f);

	if (g_State != TITLE_STATE_FADE_IN)
	{
		float speed = g_State == TITLE_STATE_KEYINPUT_WAIT ? 4 : 16;

		float alpha = (sin(g_AccumulatedTime * speed) + 1.0) * 0.5;
		Sprite_Draw(g_TexPushAnyKeyId, 1920 / 2 - Texture_GetWidth(g_TexPushAnyKeyId)/ 2, 500.0f,1.0f, { 1.0f,1.0f,1.0f,alpha });
	}
}