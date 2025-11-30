//=======================================
// 
// Author	:�p ꣑��N
// Date		:2025/10/16
//
//=======================================
#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN //�Â��E�B���h�E�Y�̃t�@�C�����g��Ȃ��悤�ɔ�΂�����
#include<Windows.h>
#include <algorithm>
#include "../include/system_timer.h";
#include "../include/direct3d.h"
#include "../include/shader.h"
#include "../include/shader3d.h"
#include "../include/texture.h"
#include "../include/cube.h"
#include "../include/grid.h"
#include "../include/sprite.h"
#include "../include/sprite_anim.h"
#include "../include/debug_ostream.h"
#include "../include/debug_text.h"
#include "../include/keyboard.h"
#include "../include/mouse.h"
#include "../include/key_logger.h"
#include "../include/Audio.h"
#include "../include/fade.h"
#include <sstream>

#include "../include/scene.h"
#include "../include/game.h"


// �Q�[���p�b�h�p
#include <Xinput.h>
#pragma comment(lib,"xinput.lib")

#include "../include/controller.h"

using namespace DirectX;

//=======================================
// �E�B���h�E���
//=======================================
static constexpr char WINDOW_CLASS[] = "GameWindow";	//���C���E�B���h�E��
static constexpr char TITLE[] = "�Q�[���E�B���h�E";		//����̃^�C�g���o�[�̃e�L�X�g
// constexpr�̓R���p�C��������Ƃ��ɒ萔������
// const�̓r���h���ɒ萔������

//=======================================
// �E�B���h�E�v���V�[�W���@�v���g�^�C�v�錾
//=======================================
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

//=======================================
// ���C��
//=======================================
int APIENTRY WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
	(void)CoInitializeEx(nullptr, COINIT_MULTITHREADED);

	//DPI�X�P�[�����O
	SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

	//�E�B���h�E�N���X�̓o�^
	WNDCLASSEX wcex{};											// �\����	{}������Ɩ����Ƃł͈Ⴄ�@�Ȃ��ꍇ�͂��݂������Ă��܂��@����ƒ��g�����������Ă����@WNDCLASSEX wcex = {};�ł��悢
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.lpfnWndProc = WndProc;									// �E�B���h�E�v���V�[�W���Ƃ������̂�o�^���Ȃ��Ƃ����Ȃ�(�֐��|�C���^)
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);			// �ǂ�ȃA�C�R���ɂ��邩�����߂Ă���
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);				// �J�[�\���̊G��ς����肷��ꏊ
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);			// �E�B���h�E���o�������̔w�i��ς��邱�Ƃ��ł���
	wcex.lpszMenuName = nullptr;								// ���j���[�͍��Ȃ��@�{���͂���Ȃ���ŏ��������Ă��邽��
	wcex.lpszClassName = WINDOW_CLASS;							// �E�B���h�E�N���X����WINDOW_CLASS�������o��
	wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);	// �X���[���A�C�R����o�^�ł���

	RegisterClassEx(&wcex);										// Register�͓o�^�Ƃ����Ӗ� wcex�̖��O�œo�^


	//�N���C�A���g�̈�̃T�C�Y���������Z�`
	RECT window_rect = { 0,0,1920,1080 };

	//�E�B���h�E�̃X�^�C��
	DWORD window_style = WS_EX_OVERLAPPEDWINDOW ^ (WS_THICKFRAME | WS_MAXIMIZEBOX);
	//�w�肵���N���C�A���g�̈���m�ۂ��邽�߂ɐV���ȒZ�`���W���v�Z
	AdjustWindowRect(&window_rect, window_style, FALSE);

	//�E�B���h�E�̕��ƍ������Z�o
	int window_width = window_rect.right - window_rect.left;
	int window_height = window_rect.bottom - window_rect.top;

	//�v���C�}�����j�^�[�̉�ʉ𑜓x�擾
	int desktop_width = GetSystemMetrics(SM_CXSCREEN);
	int desktop_height = GetSystemMetrics(SM_CYSCREEN);

	// �f�X�N�g�b�v�̐^�񒆂ɃE�B���h�E�����������悤�ɍ��W���v�Z
	// ������������A�f�X�N�g�b�v���E�B���h�E���傫���ꍇ�͍���ɕ\��
	int window_x = std::max((desktop_width - window_width) / 2, 0);
	int window_y = std::max((desktop_height - window_height) / 2, 0);

	//���C���E�B���h�E�̍쐬
	HWND hWnd = CreateWindow(	//hWnd�͕Ԃ�l�@�n���h���@���ʎq
		WINDOW_CLASS,
		TITLE,								  	//"����������"�ł��ǂ�
		WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME,	//�E�B���h�E�X�^�C����ς�����t���O
		//window_style,						  	//�t���O���Ǘ����āA�ő剻�����Ȃ��悤�ɂł���
		window_x,				  				//�E�B���h�E�̏������WX�����߂�@CW_USEDEFAULT�͓K���Ȉʒu�ɐݒ肵�Ă����
		window_y,							  	//�E�B���h�E�̏������WY�����߂�
		window_width,						  	//�E�B���h�E�̕�
		window_height,			  			  	//�E�B���h�E�̍���
		nullptr,	  						  	//�e�̃E�B���h�E�n���h�������Ƃ��@nullptr�͐e�����Ȃ����ߓ���Ă���
		nullptr,	  						  	//���j���[
		hInstance,
		nullptr
	);

	// �V�X�e���^�C�}�[�̏�����
	SystemTimer_Initialize();

	// �}�E�X�̏�����
	Mouse_Initialize(hWnd);

	//�}�E�X�̃J�[�\��
	Mouse_SetVisible(false);

	// �L�[���K�[�̏�����
	KeyLogger_Initialize();

	// �I�[�f�B�I�̏�����
	InitAudio();

	ShowWindow(hWnd, nCmdShow);	//�E�B���h�E�\��
	UpdateWindow(hWnd);			//�E�B���h�E�̕`��̍X�V

	// �e�평����
	if (!Direct3D_Initialize(hWnd))
	{
		PostQuitMessage(0);//���b�Z�[�W��\��
		Direct3D_Finalize();
		Sprite_Finalize();

		return 0;//�������Ɏ��s�����̂ŏI��
	}
	else
	{
		if (!Shader_Initialize(Direct3D_GetDevice(), Direct3D_GetContext()))
		{
			PostQuitMessage(0);//���b�Z�[�W��\��
		}
		else
		{
			Texture_Initialize(Direct3D_GetDevice(), Direct3D_GetContext());
			Sprite_Initialize(Direct3D_GetDevice(), Direct3D_GetContext());		//�X�v���C�g�̏�����

			//�t�F�[�h�̏�����
			Fade_Initialize();

			// �L���[�u�̏�����
			Cube_Initialize(Direct3D_GetDevice(), Direct3D_GetContext());

			// �V�[���̏�����
			Scene_Initialize();
		}
	}

	Shader3d_Initialize(Direct3D_GetDevice(), Direct3D_GetContext());

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	hal::DebugText dt(Direct3D_GetDevice(), Direct3D_GetContext(),
		L"texture/consolab_ascii_512.png",
		Direct3D_GetBackBufferWidth(), Direct3D_GetBackBufferHeight(),
		0.0f, 0.0f,
		0, 0,
		0.0f, 0.0f
	);

	//���Ԍv���p
	double exec_last_time = 0.0f;
	double fps_last_time = 0.0f;
	double current_time = 0.0f;
	double fps = 0.0f;
	ULONG frame_count = 0;

	//���s���Ԃ�FPS�v�����Ԃ̏�����
	exec_last_time = fps_last_time = SystemTimer_GetAbsoluteTime();

	//�Q�[�����[�v
	MSG msg;
	do {
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))// �E�B���h�E���b�Z�[�W�����Ă�����
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else// �Q�[���̏���
		{
			current_time = SystemTimer_GetAbsoluteTime();		// �V�X�e�����Ԃ��擾
			double elapsed_time = current_time - fps_last_time;	//	FPS�v���p�̍X�V���Ԃ��v�Z
			if (elapsed_time >= 1.0) // 1�b�o�߂�����
			{
				fps = frame_count / elapsed_time;
				fps_last_time = current_time;		// FPS�v�����Ԃ�ۑ�
				frame_count = 0;					//�J�����g���N���A
			}

			elapsed_time = current_time - exec_last_time;
			if ((elapsed_time) >= (1 / 60.0)) // 1/60�b���ƂɎ��s
			{
				exec_last_time = current_time;		// ��������������ۑ�

				// �}�E�X
				Mouse_State ms{};
				Mouse_GetState(&ms);

				Direct3D_Clear();
				SetViewport(0);

				// �Q�[���p�b�h�p
				XINPUT_STATE xs{};
				XInputGetState(0, &xs);//�ǂ̃p�b�h�̓��͂��󂯎�肽�����������ɂ����@������

				// �U�� �U����ON/OFF������
				XINPUT_VIBRATION xv{};
				if (xs.Gamepad.wButtons & XINPUT_GAMEPAD_A)
				{
					xv.wLeftMotorSpeed = 65535;
					xv.wRightMotorSpeed = 65535;
					XInputSetState(0, &xv);
				}
				else
				{
					xv.wLeftMotorSpeed = 0;
					xv.wRightMotorSpeed = 0;
					XInputSetState(0, &xv);
				}
				KeyLogger_Update();// �L�[�{�[�h

				Fade_Update(elapsed_time);
				Scene_Update(elapsed_time);


				Direct3D_Clear();
				SetViewport(0);

				Scene_Draw();
				Fade_Draw();// �t�F�[�h�̕`��

#if defined (DEBUG) || defined(_DEBUG)
				// FPS��\������
				std::stringstream ss;
				ss << "fps:" << fps << std::endl;
				dt.SetText(ss.str().c_str());
				dt.Draw();	// FPS�̕`��
				dt.Clear();	// FPS�̃N���A
#endif
				Direct3D_Present();

				frame_count++;	// �t���[���J�E���g�𑝂₷
				Scene_ChangeScene();// �V�[���̕ύX�̊m�F
			}
		}
	} while (msg.message != WM_QUIT);

	Scene_Finalize();		// �V�[���̏I������

	Fade_Finalize();		// �t�F�[�h�̏I������
	Cube_Finalize();		// �L���[�u�̏I������
	Shader_Finalize();		// �V�F�[�_�[�̏I������
	Shader3d_Finalize();	// �V�F�[�_�[3d�̏I������
	Texture_Finalize();		// �e�N�X�`���̏I������
	Direct3D_Finalize();	// Direct3D�̏I������
	Sprite_Finalize();		// �X�v���C�g�̏I������
	Mouse_Finalize();		// �}�E�X�̏I������
	UninitAudio();			// �I�[�f�B�I�̏I������

	return (int)msg.wParam;	// ���b�Z�[�W��wParam��Ԃ�
}
//=======================================
// �E�B���h�E�v���V�[�W��
//=======================================
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_ACTIVATEAPP:
		Keyboard_ProcessMessage(message, wParam, lParam);
		Mouse_ProcessMessage(message, wParam, lParam);
		break;
	case WM_INPUT:
	case WM_MOUSEMOVE:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_MOUSEWHEEL:
	case WM_XBUTTONDOWN:
	case WM_XBUTTONUP:
	case WM_MOUSEHOVER:
		Mouse_ProcessMessage(message, wParam, lParam);
		break;
	case WM_DESTROY:			//�E�B���h�E�̔j�����b�Z�[�W�@�������O���ƃE�B���h�E�����Ȃ��Ȃ邽�ߕK�{
		PostQuitMessage(0);		//WM_QUIT���b�Z�[�W�̑��M
		break;

	case WM_CLOSE:// �E�B���h�E��������Ƃ��ɖ{���ɕ��邩�ǂ����̔��ʂ�����
		if (MessageBox(hWnd, "�{���ɕ��Ă���낵���ł����H", "�m�F", MB_OKCANCEL | MB_DEFBUTTON2) == IDOK)
		{
			DestroyWindow(hWnd);//�w��̃E�B���h�E��WM_DESTROY���b�Z�[�W�𑗂�
		}
		break;//DefWindowProc�֐��Ƀ��b�Z�[�W�𗬂����I�����邱�Ƃɂ���ĉ����Ȃ��������Ƃɂ���
	case WM_KEYDOWN:// WM_KEYDOWN�ɂ̓}�E�X�̃{�^����L�[�{�[�h�̃L�[�������ꂽ�Ƃ��Ɏ󂯎�郁�b�Z�[�W
		if (wParam == VK_ESCAPE)// wParam�ɂ͉����ꂽ�L�[�̉��z�L�[�ƌĂ΂��L�[�R�[�h���i�[����Ă���
		{
			SendMessage(hWnd, WM_CLOSE, 0, 0);// WM_CLOSE���b�Z�[�W�̑��M
		}
	case WM_SYSKEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYUP:
		Keyboard_ProcessMessage(message, wParam, lParam);
		break;
	default:
		//�ʏ�̃��b�Z�[�W�����͂��̊֐��ɔC����
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}




