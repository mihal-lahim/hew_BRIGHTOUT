#include "../include/scene.h"
#include "../include/title.h"
#include "../include/game.h"
#include "../include/result.h"

static Scene g_Scene = SCENE_GAME;// �f�o�b�O�̎��͂��̃V�[�����Q�[���ɂ��Ă���
static Scene g_SceneNext = g_Scene;

// �֐��|�C���^�̌^��`
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
		Scene_Finalize();// ���݂̃V�[���̌�Еt��
		g_Scene = g_SceneNext;// ���̃V�[�������̃V�[���Ƃ���

		Scene_Initialize();// ���̃V�[���̏�����
	}
}
