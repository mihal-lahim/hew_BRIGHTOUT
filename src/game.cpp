/*==============================================================================

 �Q�[���̐��� [game.cpp]
 Author : hiroshi kasiwagi
 Date :2025/10/22
--------------------------------------------------------------------------------

==============================================================================*/
#include "../include/game.h"
#include "../include/fade.h"
#include "../include/grid.h"
#include "../include/direct3d.h"
#include "../include/map.h"
#include "../include/light.h"
#include "../include/model.h"
#include "../include/debug_camera.h"
#include "../include/camera.h"
#include "../include/camera_manager.h"
#include <DirectXMath.h>
#include "../include/controller.h"
#include "../include/debug_ostream.h"
#include "../include/player.h"
#include "../include/key_logger.h"
#include "../include/UI_Charge.h"
#include "../include/cube.h"
#include "../include/debug_text.h"
#include "../include/ObjectManager.h" // �ǉ�
#include <array>
#include <string>
#include <cstdio>
using namespace DirectX;


static CameraManager g_camMgr;
static DebugCamera g_debugCamera({0.0f,1.0f,-5.0f }, {0.0f,0.0f,0.0f });
static MODEL* g_pKirby{};
static MODEL* g_test{};
static MODEL* g_ball{};

// �R���g���[���[: �ő�3�l�T�|�[�g
static Controller* g_controllers[3] = { nullptr, nullptr, nullptr };

// �v���C���[: �ő�3�l�Ή�
static Player* g_players[3] = { nullptr, nullptr, nullptr };

// UI�����̃O���[�o���v���C���[�|�C���^
Player* g_player = nullptr;
// �f�t�H���g1�v���C���[
static int g_playerCount =1; 

// UI: �`���[�W�Q�[�W�i���݂͒P��C���X�^���X�j
static UI_Charge* g_uiCharge = nullptr;
// �v���C���[�̗͕̑\���p DebugText
static hal::DebugText* g_debugText = nullptr;

void Game_SetPlayerCount(int count)
{
	if (count < 1) count = 1;
	if (count > 3) count = 3;
	g_playerCount = count;
}

//============================================================================
// �Q�[��������
//============================================================================
void Game_Initialize()
{
	Game_SetPlayerCount(1);


	Grid_Initialize(10,10,1.0f);
	g_MapInstance.Initialize();
	g_ObjectManager.Initialize(); // �ǉ�
	Light_Initialize();
	Fade_Start(1.0f, true);

	g_pKirby = ModelLoad("model/kirby.fbx",0.1f, false);
	g_test = ModelLoad("model/test.fbx",0.1f, false);
	g_ball = ModelLoad("model/ball.fbx",0.1f, false);

	// �R���g���[���[�𓮓I�m�ۂ��A�e�X���b�g���ƂɃR���X�g���N�^�ŏ�����
	for (int i =0; i <3; ++i) {
		if (!g_controllers[i]) g_controllers[i] = new Controller(i,16);
	}

	// �e�R���g���[���[�̃R�[���o�b�N�ݒ�
	for (int i =0; i <3; ++i) {
		if (g_controllers[i]) {
			g_controllers[i]->SetOnConnected([i](int pad) { hal::dout << "Controller connected: " << pad << " (slot " << i << ")" << std::endl; });
			g_controllers[i]->SetOnDisconnected([i](int pad) { hal::dout << "Controller disconnected: " << pad << " (slot " << i << ")" << std::endl; });
			g_controllers[i]->SetOnButtonPress([i](int pad, Controller::Button btn) { hal::dout << "Pad " << pad << " Button pressed: " << static_cast<int>(btn) << " (slot " << i << ")" << std::endl; });
		}
	}

	// �v�����ꂽ�������v���C���[�𐶐��i�����Ԋu���J���Ĕz�u�j
	for (int i =0; i < g_playerCount; ++i) {
		float x = static_cast<float>(i *2) - static_cast<float>(g_playerCount -1); // ���_�t�߂ɕ��U�z�u
		XMFLOAT3 pos = { x,16.0f,0.0f };
        // �� �C���_: �v���C���[�̏���������Z���������ɕύX
		g_players[i] = new Player(g_pKirby,g_ball, pos, { 0.0f,10.0f,-1.0f });
		// �R���g���[���[�����蓖�āi�X���b�g i�j
		g_players[i]->SetController(g_controllers[i]);
		// UI �p�̌݊��O���[�o���iplayer0�j��ݒ�
		if (i ==0) g_player = g_players[i];
		// �e�v���C���[�ɃJ�������쐬
		g_players[i]->CreateCamera({0.0f,1.0f,0.0f });
	}

	// �J�����}�l�[�W���[�̐ݒ�
	g_camMgr.AddCamera(&g_debugCamera, "debug");
	// �e�v���C���[�̃J�������ŗL���œo�^
	for (int i =0; i < g_playerCount; ++i) {
		if (g_players[i] && g_players[i]->GetCamera()) {
			std::string name = std::string("player") + std::to_string(i);
			g_camMgr.AddCamera(g_players[i]->GetCamera(), name.c_str());
		}
	}
	// �f�t�H���g�̓f�o�b�O�J�����ɐݒ�
	g_camMgr.SetActiveByName("debug");

	// UI ������
	g_uiCharge = new UI_Charge();
	g_uiCharge->SetPosition({20.0f,20.0f });
	g_uiCharge->SetSize({200.0f,24.0f });
	g_uiCharge->Initialize();
	UIManager::Add(g_uiCharge);

	// UI �p�̃O���[�o���v���C���[�|�C���^��ݒ�
	g_player = g_players[0];
}

void Game_Finalize()
{
	ModelRelease(g_pKirby);

	Light_Finalize();
	Grid_Finalize();
	g_MapInstance.Finalize();
	g_ObjectManager.Finalize(); // �ǉ�

	if (g_uiCharge) {
		UIManager::Remove(g_uiCharge);
		delete g_uiCharge;
		g_uiCharge = nullptr;
	}

	// �v���C���[���폜
	for (int i =0; i <3; ++i) {
		if (g_players[i]) {
			if (g_player == g_players[i]) g_player = nullptr; // �O���[�o���ƈ�v����ꍇ�̓N���A
			delete g_players[i];
			g_players[i] = nullptr;
		}
	}

	// �R���g���[���[������i�X���b�h��~�EJoin ���s���j
	for (int i =0; i <3; ++i) {
		if (g_controllers[i]) {
			// ���s���̃X���b�h������Β�~���Ă��� join
			g_controllers[i]->Stop();
			g_controllers[i]->Join();
			delete g_controllers[i];
			g_controllers[i] = nullptr;
		}
	}

	// �O���[�o���v���C���[�|�C���^�����Z�b�g
	g_player = nullptr;
}

static double keika_time =0.0;

void Game_Update(double elapsed_time)
{
	keika_time += elapsed_time;

	// �L�[�{�[�h�̃g���K�[��Ԃ��X�V
	KeyLogger_Update();

	// �R���g���[���[���X�V
	for (int i =0; i <3; ++i) if (g_controllers[i]) g_controllers[i]->Update();

	// �J�����Ǘ�
	// Tab�Ńf�o�b�O�J������ player0 �J�������g�O��
	if (KeyLogger_IsTrigger(KK_TAB)) {
		Camera* current = g_camMgr.GetActive();
		if (current == &g_debugCamera) {
			if (g_players[0] && g_players[0]->GetCamera()) {
				g_camMgr.SetActiveByName("player0");
			}
		} else {
			g_camMgr.SetActiveByName("debug");
		}
	} else {
		// �R���g���[���[�� X �{�^���ŃJ�����������ؑ�
		if (g_controllers[0] && g_controllers[0]->WasPressed(Controller::BUTTON_X)) {
			g_camMgr.Next();
		} else {
			for (int i =1; i <3; ++i) {
				if (g_controllers[i] && g_controllers[i]->WasPressed(Controller::BUTTON_X)) { g_camMgr.Next(); break; }
			}
		}
	}

	g_camMgr.UpdateActive(elapsed_time);

	Fade_Update(elapsed_time);
	Grid_Update(elapsed_time);

	// �v���C���[���X�V
	for (int i =0; i < g_playerCount; ++i) {
		if (g_players[i]) g_players[i]->Update(elapsed_time);
	}

	// UI �X�V
	UIManager::UpdateAll(elapsed_time);
}

void Game_Draw()
{
	// �}���`�v���C���[���͕����\��
	const float SCREEN_WIDTH = static_cast<float>(Direct3D_GetBackBufferWidth());
	const float SCREEN_HEIGHT = static_cast<float>(Direct3D_GetBackBufferHeight());

	if (g_playerCount <= 1) {
		// �V���O���J�����p�X:���ɃA�N�e�B�u�ȃJ�������g�p�B�A�N�e�B�u�ȃJ�����������ꍇ�̂݃f�t�H���g��ݒ�
		if (!g_camMgr.GetActive()) {
			if (g_players[0] && g_players[0]->GetCamera()) {
				g_camMgr.SetActiveByName("player0");
			} else {
				g_camMgr.SetActiveByName("debug");
			}
		}
		g_camMgr.ApplyActive();

		Direct3D_SetDepthTest(true);

		Grid_Draw();

		XMFLOAT3 direction;
		{
			XMVECTOR dirVec = XMVectorSet(-1.0f, -1.5f,1.0f,0.0f);
			XMStoreFloat3(&direction, dirVec);
		}
		Light_SetDiffuse({0.7f,0.7f,0.7f }, direction);

		Light_SetAmbient({0.2f,0.1f,0.1f,1.0f });
		ModelDraw(g_pKirby, XMMatrixTranslation(0.0f,2.0f,0.0f));

		Light_SetAmbient({0.2f,0.1f,0.1f,1.0f });
		ModelDraw(g_test, XMMatrixIdentity());

		Light_SetAmbient({0.1f,0.1f,0.1f,1.0f });
		g_MapInstance.Draw();
		g_ObjectManager.Draw(); // g_MapInstance.Draw() �̌�ɒǉ�

		Light_SetAmbient({0.3f,0.3f,0.3f,1.0f });
		for (int i = 0; i < g_playerCount; ++i) {
			if (g_players[i]) {
				g_players[i]->Draw();

                // �� �ǉ�: �v���C���[�̌�������������f�o�b�O�L���[�u��`��
                Player* p = g_players[i];
                XMFLOAT3 playerPos = p->GetPosition();
                XMFLOAT3 playerDir = p->GetDirection();

                // �v���C���[�̏����O�̈ʒu���v�Z
                float offset = 1.0f; // �v���C���[����̋���
                XMVECTOR posVec = XMLoadFloat3(&playerPos);
                XMVECTOR dirVec = XMLoadFloat3(&playerDir);
                XMVECTOR cubePosVec = posVec + dirVec * offset;

                // �L���[�u�̃��[���h�s����쐬�i�������\���j
                XMMATRIX scale = XMMatrixScaling(0.2f, 0.2f, 0.2f);
                XMMATRIX translation = XMMatrixTranslationFromVector(cubePosVec);
                XMMATRIX world = scale * translation;

                // �f�o�b�O�L���[�u��`��
                Cube_Draw(0,world);
            }
		}

		Direct3D_SetDepthTest(false);

		//2D UI �`��
		UIManager::DrawAll();

		// �v���C���[�̗̑͂� DebugText �ō����ɕ\��
		if (!g_debugText && g_players[0]) {
			ID3D11Device* pDevice = Direct3D_GetDevice();
			ID3D11DeviceContext* pContext = Direct3D_GetContext();
			UINT width = Direct3D_GetBackBufferWidth();
			UINT height = Direct3D_GetBackBufferHeight();
			g_debugText = new hal::DebugText(pDevice, pContext, L"texture/consolab_ascii_512.png", width, height, 10.0f, height - 30.0f, 1, 32);
		}

		if (g_debugText && g_players[0]) {
			g_debugText->Clear();
			char healthText[64];
			snprintf(healthText, sizeof(healthText), "HP: %d / %d", g_players[0]->GetHealth(), g_players[0]->GetMaxHealth());
			g_debugText->SetText(healthText, { 1.0f, 1.0f, 1.0f, 1.0f }); // ���F�ŕ\��
			g_debugText->Draw();
		}
		return;
	}

	// �����v���C���[: �r���[�|�[�g��ݒ肵�Ċe�v���C���[�̃r���[��`��
	ID3D11DeviceContext* ctx = Direct3D_GetContext();

	// �e�v���C���[�̃J�����Ńt��3D�V�[���������_�����O
	for (int i =0; i < g_playerCount; ++i) {
		if (!g_players[i] || !g_players[i]->GetCamera()) continue;

		D3D11_VIEWPORT vp = {};
		if (g_playerCount ==2) {
			// ������
			vp.TopLeftX = static_cast<FLOAT>(i * (SCREEN_WIDTH *0.5f));
			vp.TopLeftY =0.0f;
			vp.Width = SCREEN_WIDTH *0.5f;
			vp.Height = SCREEN_HEIGHT;
		}
		else if (g_playerCount ==3) {
			if (i ==0) {
				// ���������c�����ς�
				vp.TopLeftX =0.0f;
				vp.TopLeftY =0.0f;
				vp.Width = SCREEN_WIDTH *0.5f;
				vp.Height = SCREEN_HEIGHT;
			} else {
				//�E�����㉺�ɕ���
				vp.TopLeftX = SCREEN_WIDTH *0.5f;
				vp.Width = SCREEN_WIDTH *0.5f;
				vp.Height = SCREEN_HEIGHT *0.5f;
				vp.TopLeftY = (i ==1) ?0.0f : SCREEN_HEIGHT *0.5f;
			}
		}
		else {
			// �t�H�[���o�b�N: �t���X�N���[��
			vp.TopLeftX =0.0f; vp.TopLeftY =0.0f; vp.Width = SCREEN_WIDTH; vp.Height = SCREEN_HEIGHT;
		}
		vp.MinDepth =0.0f;
		vp.MaxDepth =1.0f;

		ctx->RSSetViewports(1, &vp);

		// ���̃v���C���[�̃J�������A�N�e�B�u�ɂ���
		std::string name = std::string("player") + std::to_string(i);
		g_camMgr.SetActiveByName(name.c_str());
		g_camMgr.ApplyActive();

		Direct3D_SetDepthTest(true);

		// �V�[���`��
		Grid_Draw();

		XMFLOAT3 direction;
		{
			XMVECTOR dirVec = XMVectorSet(-1.0f, -1.5f,1.0f,0.0f);
			XMStoreFloat3(&direction, dirVec);
		}
		Light_SetDiffuse({0.7f,0.7f,0.7f}, direction);

		Light_SetAmbient({0.2f,0.1f,0.1f,1.0f});
		ModelDraw(g_pKirby, XMMatrixTranslation(0.0f,2.0f,0.0f));

		Light_SetAmbient({0.2f,0.1f,0.1f,1.0f});
		ModelDraw(g_test, XMMatrixIdentity());

		Light_SetAmbient({0.1f,0.1f,0.1f,1.0f});
		g_MapInstance.Draw();
		g_ObjectManager.Draw(); // g_MapInstance.Draw() �̌�ɒǉ�

		Light_SetAmbient({0.3f,0.3f,0.3f,1.0f});
		for (int j =0; j < g_playerCount; ++j) {
			if (g_players[j]) g_players[j]->Draw();
		}

		Direct3D_SetDepthTest(false);

		// �K�v�Ȃ�ʃr���[������ HUD �������ŕ`��i�ȗ����j
	}

	// �t���r���[�|�[�g�ɖ߂�
	D3D11_VIEWPORT vpFull = {};
	vpFull.TopLeftX =0.0f; vpFull.TopLeftY =0.0f;
	vpFull.Width = SCREEN_WIDTH; vpFull.Height = SCREEN_HEIGHT;
	vpFull.MinDepth =0.0f; vpFull.MaxDepth =1.0f;
	ctx->RSSetViewports(1, &vpFull);

	// �t���X�N���[���ŃO���[�o�� UI ����x�`��
	UIManager::DrawAll();
}
