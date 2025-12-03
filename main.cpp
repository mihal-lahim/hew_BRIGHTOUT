//=======================================
// 
// Author	:角 凜太朗
// Date		:2025/10/16
//
//=======================================
#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN //古いウィンドウズのファイルを使わないように飛ばすため
#include<Windows.h>
#include <algorithm>
#include "system_timer.h";
#include "direct3d.h"
#include "shader.h"
#include "shader3d.h"
#include "texture.h"
#include "cube.h"
#include "grid.h"
#include "sprite.h"
#include "sprite_anim.h"
#include "debug_ostream.h"
#include "debug_text.h"
#include "keyboard.h"
#include "mouse.h"
#include "key_logger.h"
#include "Audio.h"
#include "fade.h"
#include <sstream>

#include "scene.h"
#include "game.h"


// ゲームパッド用
#include <Xinput.h>
#pragma comment(lib,"xinput.lib")

#include "controller.h"

using namespace DirectX;

//=======================================
// ウィンドウ情報
//=======================================
static constexpr char WINDOW_CLASS[] = "GameWindow";	//メインウィンドウ名
static constexpr char TITLE[] = "ゲームウィンドウ";		//左上のタイトルバーのテキスト
// constexprはコンパイルをするときに定数化する
// constはビルド時に定数化する

//=======================================
// ウィンドウプロシージャ　プロトタイプ宣言
//=======================================
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

//=======================================
// メイン
//=======================================
int APIENTRY WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
	(void)CoInitializeEx(nullptr, COINIT_MULTITHREADED);

	//DPIスケーリング
	SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

	//ウィンドウクラスの登録
	WNDCLASSEX wcex{};											// 構造体	{}があると無いとでは違う　ない場合はごみが入ってしまう　あると中身を初期化してくれる　WNDCLASSEX wcex = {};でもよい
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.lpfnWndProc = WndProc;									// ウィンドウプロシージャというものを登録しないといけない(関数ポインタ)
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);			// どんなアイコンにするかを決めている
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);				// カーソルの絵を変えたりする場所
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);			// ウィンドウが出来た時の背景を変えることができる
	wcex.lpszMenuName = nullptr;								// メニューは作らない　本来はいらない上で初期化しているため
	wcex.lpszClassName = WINDOW_CLASS;							// ウィンドウクラス名でWINDOW_CLASSを引き出す
	wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);	// スモールアイコンを登録できる

	RegisterClassEx(&wcex);										// Registerは登録という意味 wcexの名前で登録


	//クライアント領域のサイズを持った短形
	RECT window_rect = { 0,0,1920 * 0.75,1080 * 0.75 };

	//ウィンドウのスタイル
	DWORD window_style = WS_EX_OVERLAPPEDWINDOW ^ (WS_THICKFRAME | WS_MAXIMIZEBOX);
	//指定したクライアント領域を確保するために新たな短形座標を計算
	AdjustWindowRect(&window_rect, window_style, FALSE);

	//ウィンドウの幅と高さを算出
	int window_width = window_rect.right - window_rect.left;
	int window_height = window_rect.bottom - window_rect.top;

	//プライマリモニターの画面解像度取得
	int desktop_width = GetSystemMetrics(SM_CXSCREEN);
	int desktop_height = GetSystemMetrics(SM_CYSCREEN);

	// デスクトップの真ん中にウィンドウが生成されるように座標を計算
	// ただし万が一、デスクトップよりウィンドウが大きい場合は左上に表示
	int window_x = std::max((desktop_width - window_width) / 2, 0);
	int window_y = std::max((desktop_height - window_height) / 2, 0);

	//メインウィンドウの作成
	HWND hWnd = CreateWindow(	//hWndは返り値　ハンドル　識別子
		WINDOW_CLASS,
		TITLE,								  	//"あいうえお"でも良い
		WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME,	//ウィンドウスタイルを変えられるフラグ
		//window_style,						  	//フラグを管理して、最大化させないようにできる
		window_x,				  				//ウィンドウの初期座標Xを決める　CW_USEDEFAULTは適当な位置に設定してくれる
		window_y,							  	//ウィンドウの初期座標Yを決める
		window_width,						  	//ウィンドウの幅
		window_height,			  			  	//ウィンドウの高さ
		nullptr,	  						  	//親のウィンドウハンドルを作るとき　nullptrは親がいないため入れている
		nullptr,	  						  	//メニュー
		hInstance,
		nullptr
	);

	// システムタイマーの初期化
	SystemTimer_Initialize();

	// マウスの初期化
	Mouse_Initialize(hWnd);

	//マウスのカーソル
	Mouse_SetVisible(true);

	// キーロガーの初期化
	KeyLogger_Initialize();

	// オーディオの初期化
	InitAudio();

	ShowWindow(hWnd, nCmdShow);	//ウィンドウ表示
	UpdateWindow(hWnd);			//ウィンドウの描画の更新

	// テ平線の初期化
	if (!Direct3D_Initialize(hWnd))
	{
		PostQuitMessage(0);//メッセージを表示
		Direct3D_Finalize();
		Sprite_Finalize();

		return 0;//初期化に失敗したので終了
	}
	else
	{
		if (!Shader_Initialize(Direct3D_GetDevice(), Direct3D_GetContext()))
		{
			PostQuitMessage(0);//メッセージを表示
		}
		else
		{
			Texture_Initialize(Direct3D_GetDevice(), Direct3D_GetContext());
			Sprite_Initialize(Direct3D_GetDevice(), Direct3D_GetContext());		//スプライトの初期化

			//フェードの初期化
			Fade_Initialize();

			// キューブの初期化
			Cube_Initialize(Direct3D_GetDevice(), Direct3D_GetContext());

			// シーンの初期化
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

	//時間計測用
	double exec_last_time = 0.0f;
	double fps_last_time = 0.0f;
	double current_time = 0.0f;
	double fps = 0.0f;
	ULONG frame_count = 0;

	//実行時間とFPS計測時間の初期化
	exec_last_time = fps_last_time = SystemTimer_GetAbsoluteTime();

	//ゲームループ
	MSG msg;
	do {
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))// ウィンドウメッセージが来ていたら
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else// ゲームの処理
		{
			current_time = SystemTimer_GetAbsoluteTime();		// システム時間を取得
			double elapsed_time = current_time - fps_last_time;	//	FPS計測用の更新時間を計算
			if (elapsed_time >= 1.0) // 1秒経過したら
			{
				fps = frame_count / elapsed_time;
				fps_last_time = current_time;		// FPS計測時間を保存
				frame_count = 0;					//カレントをクリア
			}

			elapsed_time = current_time - exec_last_time;
			if ((elapsed_time) >= (1 / 60.0)) // 1/60秒ごとに実行
			{
				exec_last_time = current_time;		// 処理した時刻を保存

				// マウス
				Mouse_State ms{};
				Mouse_GetState(&ms);

				Direct3D_Clear();
				SetViewport(0);

				// ゲームパッド用
				XINPUT_STATE xs{};
				XInputGetState(0, &xs);//どのパッドの入力を受け取りたいかを引数にいれる　第二引数

				// 振動 振動のON/OFFを書く
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
				KeyLogger_Update();// キーボード

				Fade_Update(elapsed_time);
				Scene_Update(elapsed_time);


				Direct3D_Clear();
				SetViewport(0);

				Scene_Draw();
				Fade_Draw();// フェードの描画

#if defined (DEBUG) || defined(_DEBUG)
				// FPSを表示する
				std::stringstream ss;
				ss << "fps:" << fps << std::endl;
				dt.SetText(ss.str().c_str());
				dt.Draw();	// FPSの描画
				dt.Clear();	// FPSのクリア
#endif
				Direct3D_Present();

				frame_count++;	// フレームカウントを増やす
				Scene_ChangeScene();// シーンの変更の確認
			}
		}
	} while (msg.message != WM_QUIT);

	Scene_Finalize();		// シーンの終了処理

	Fade_Finalize();		// フェードの終了処理
	Cube_Finalize();		// キューブの終了処理
	Shader_Finalize();		// シェーダーの終了処理
	Shader3d_Finalize();	// シェーダー3dの終了処理
	Texture_Finalize();		// テクスチャの終了処理
	Direct3D_Finalize();	// Direct3Dの終了処理
	Sprite_Finalize();		// スプライトの終了処理
	Mouse_Finalize();		// マウスの終了処理
	UninitAudio();			// オーディオの終了処理

	return (int)msg.wParam;	// メッセージのwParamを返す
}
//=======================================
// ウィンドウプロシージャ
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
	case WM_DESTROY:			//ウィンドウの破棄メッセージ　ここを外すとウィンドウを閉じれなくなるため必須
		PostQuitMessage(0);		//WM_QUITメッセージの送信
		break;

	case WM_CLOSE:// ウィンドウが閉じられるときに本当に閉じるかどうかの判別をする
		if (MessageBox(hWnd, "本当に閉じてもよろしいですか？", "確認", MB_OKCANCEL | MB_DEFBUTTON2) == IDOK)
		{
			DestroyWindow(hWnd);//指定のウィンドウにWM_DESTROYメッセージを送る
		}
		break;//DefWindowProc関数にメッセージを流さず終了することによって何もなかったことにする
	case WM_KEYDOWN:// WM_KEYDOWNにはマウスのボタンやキーボードのキーが押されたときに受け取るメッセージ
		if (wParam == VK_ESCAPE)// wParamには押されたキーの仮想キーと呼ばれるキーコードが格納されている
		{
			SendMessage(hWnd, WM_CLOSE, 0, 0);// WM_CLOSEメッセージの送信
		}
	case WM_SYSKEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYUP:
		Keyboard_ProcessMessage(message, wParam, lParam);
		break;
	default:
		//通常のメッセージ処理はこの関数に任せる
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}







