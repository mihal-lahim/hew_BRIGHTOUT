#include "scene.h"
#include "title.h"
#include "game.h"
#include "result.h"

static Scene g_Scene = SCENE_GAME;// デバッグの時はこのシーンをゲームにしておく
static Scene g_SceneNext = g_Scene;

// 関数ポインタの型定義
typedef void (*FuncVoid)(void);
typedef void (*FuncDouble)(double);

static FuncVoid g_Initialize[]{
	Title_Initialize,
	Game_Initialize,
	Result_Initialize
};

static FuncVoid g_Finalize[]{
	Title_Finalize,
	Game_Finalize,
	Result_Finalize
};

static FuncDouble g_Update[]{
	Title_Update,
	Game_Update,
	Result_Update
};

static FuncVoid g_Draw[]{
	Title_Draw,
	Game_Draw,
	Result_Draw
};


void Scene_Initialize()
{
	g_Initialize[g_Scene]();
}

void Scene_Finalize()
{
	g_Finalize[g_Scene]();
}

void Scene_Update(double elapsed_time)
{
	g_Update[g_Scene](elapsed_time);
}

void Scene_Draw()
{
	g_Draw[g_Scene]();
}

void Scene_SetNextScene(Scene scene)
{
	g_SceneNext = scene;
}

void Scene_ChangeScene()
{
	if (g_Scene != g_SceneNext)
	{
		Scene_Finalize();// 現在のシーンの後片付け
		g_Scene = g_SceneNext;// 今のシーンを次のシーンとする

		Scene_Initialize();// 次のシーンの初期化
	}
}
