/*==============================================================================

 ゲームの制御 [game.cpp]
 Author : hiroshi kasiwagi
 Date :2025/10/22
--------------------------------------------------------------------------------

==============================================================================*/
#include "game.h"
#include "fade.h"
#include "grid.h"
#include "direct3d.h"
#include "map.h"
#include "light.h"
#include "model.h"
#include "debug_camera.h"
#include "camera.h"
#include "camera_manager.h"
#include <DirectXMath.h>
#include "controller.h"
#include "debug_ostream.h"
#include "player.h"
#include "key_logger.h"
#include "UI_Charge.h"
#include "cube.h"
#include "debug_text.h"
#include "ObjectManager.h"
#include "ItemGenerator.h"
#include "PowerLine.h"
#include "Pole.h"
#include "ItemGeneratorObject.h"
#include "DebugAABB.h"
#include "house.h"
#include "button_hint_ui.h"
#include <array>
#include <string>
#include <cstdio>
using namespace DirectX;


static CameraManager g_camMgr;
static DebugCamera g_debugCamera({ 0.0f,1.0f,-5.0f }, { 0.0f,0.0f,0.0f });
static MODEL* g_pKirby{};
static MODEL* g_test{};
static MODEL* g_ball{};
static MODEL* g_houseModel{};  // ハウスモデル

// コントローラー: 最大3人サポート
static Controller* g_controllers[3] = { nullptr, nullptr, nullptr };

// プレイヤー: 最大3人対応
static Player* g_players[3] = { nullptr, nullptr, nullptr };

// UI向けのグローバルプレイヤーポインタ
Player* g_player = nullptr;
// デフォルト1プレイヤー
static int g_playerCount = 1;

// UI: チャージゲージ（現在は単一インスタンス）
static UI_Charge* g_uiCharge = nullptr;
// プレイヤーの体力表示用 DebugText
static hal::DebugText* g_debugText = nullptr;

// アイテムジェネレーター
static ItemGenerator* g_itemGenerator = nullptr;

// ボタン指示UI
static ButtonHintUI* g_buttonHintUI = nullptr;

void Game_SetPlayerCount(int count)
{
	if (count < 1) count = 1;
	if (count > 3) count = 3;
	g_playerCount = count;
}

//============================================================================
// ゲーム初期化
//============================================================================
void Game_Initialize()
{
	Game_SetPlayerCount(1);


	Grid_Initialize(10, 10, 1.0f);
	g_MapInstance.Initialize();
	g_ObjectManager.Initialize();
	Light_Initialize();
	Fade_Start(1.0f, true);

	// デバッグAABB描画の初期化
	DebugAABB::Initialize(Direct3D_GetDevice(), Direct3D_GetContext());
	g_pKirby = ModelLoad("model/kirby.fbx", 0.1f, false);
	g_test = ModelLoad("model/test.fbx", 0.1f, false);
	g_ball = ModelLoad("model/ball.fbx", 0.1f, false);

	// ハウスモデルのロード
	g_houseModel = ModelLoad("model/aruhula_ie.fbx", 0.1f, false);  // ※ ハウスの適切なモデルに置き換え

	// 電柱を ObjectManager に追加し、IDを設定
	int poleID = 0;

	auto pole1 = std::make_unique<Pole>(
		DirectX::XMFLOAT3(-10.0f, 0.0f, 0.0f),
		4.0f, 0.2f
	);
	pole1->SetPoleID(poleID++);
	g_ObjectManager.AddGameObject(std::move(pole1));

	auto pole2 = std::make_unique<Pole>(
		DirectX::XMFLOAT3(10.0f, 0.0f, 0.0f),
		4.0f, 0.2f
	);
	pole2->SetPoleID(poleID++);
	g_ObjectManager.AddGameObject(std::move(pole2));

	auto pole3 = std::make_unique<Pole>(
		DirectX::XMFLOAT3(0.0f, 0.0f, -10.0f),
		4.0f, 0.2f
	);
	pole3->SetPoleID(poleID++);
	g_ObjectManager.AddGameObject(std::move(pole3));

	auto pole4 = std::make_unique<Pole>(
		DirectX::XMFLOAT3(0.0f, 0.0f, 10.0f),
		4.0f, 0.2f
	);
	pole4->SetPoleID(poleID++);
	g_ObjectManager.AddGameObject(std::move(pole4));

	auto pole5 = std::make_unique<Pole>(
		DirectX::XMFLOAT3(-10.0f, 0.0f, 10.0f),
		4.0f, 0.2f
	);
	pole5->SetPoleID(poleID++);
	g_ObjectManager.AddGameObject(std::move(pole5));

	auto pole6 = std::make_unique<Pole>(
		DirectX::XMFLOAT3(10.0f, 0.0f, 10.0f),
		4.0f, 0.2f
	);
	pole6->SetPoleID(poleID++);
	g_ObjectManager.AddGameObject(std::move(pole6));

	// 電柱同士を電線で自動接続
	g_ObjectManager.ConnectNearbyPoles();

	// ハウスを ObjectManager に追加
	auto house1 = std::make_unique<House>(
		DirectX::XMFLOAT3(-5.0f, 0.0f, -5.0f),
		g_houseModel,
		100.0f  // 最大電気量
	);
	g_ObjectManager.AddGameObject(std::move(house1));

	auto house2 = std::make_unique<House>(
		DirectX::XMFLOAT3(5.0f, 0.0f, 5.0f),
		g_houseModel,
		100.0f
	);
	g_ObjectManager.AddGameObject(std::move(house2));

	// アイテムジェネレータオブジェクトをフィールドに配置
	int generatorID = 0;

	auto generator1 = std::make_unique<ItemGeneratorObject>(
		DirectX::XMFLOAT3(-8.0f, 1.0f, -8.0f),
		5.0f,   // スポーン範囲
		2.0f    // スポーン間隔
	);
	generator1->SetGeneratorID(generatorID++);
	g_ObjectManager.AddGameObject(std::move(generator1));

	auto generator2 = std::make_unique<ItemGeneratorObject>(
		DirectX::XMFLOAT3(8.0f, 1.0f, -8.0f),
		5.0f,
		2.0f
	);
	generator2->SetGeneratorID(generatorID++);
	g_ObjectManager.AddGameObject(std::move(generator2));

	auto generator3 = std::make_unique<ItemGeneratorObject>(
		DirectX::XMFLOAT3(-8.0f, 1.0f, 8.0f),
		5.0f,
		2.0f
	);
	generator3->SetGeneratorID(generatorID++);
	g_ObjectManager.AddGameObject(std::move(generator3));

	auto generator4 = std::make_unique<ItemGeneratorObject>(
		DirectX::XMFLOAT3(8.0f, 1.0f, 8.0f),
		5.0f,
		2.0f
	);
	generator4->SetGeneratorID(generatorID++);
	g_ObjectManager.AddGameObject(std::move(generator4));

	// コントローラーを動的確保し、各スロットごとにコンストラクタで初期化
	for (int i = 0; i < 3; ++i) {
		if (!g_controllers[i]) g_controllers[i] = new Controller(i, 16);
	}

	// 各コントローラーのコールバック設定
	for (int i = 0; i < 3; ++i) {
		if (g_controllers[i]) {
			g_controllers[i]->SetOnConnected([i](int pad) { hal::dout << "Controller connected: " << pad << " (slot " << i << ")" << std::endl; });
			g_controllers[i]->SetOnDisconnected([i](int pad) { hal::dout << "Controller disconnected: " << pad << " (slot " << i << ")" << std::endl; });
			g_controllers[i]->SetOnButtonPress([i](int pad, Controller::Button btn) { hal::dout << "Pad " << pad << " Button pressed: " << static_cast<int>(btn) << " (slot " << i << ")" << std::endl; });
		}
	}

	// 要求された数だけプレイヤーを生成（少し間隔を開けて配置）
	for (int i = 0; i < g_playerCount; ++i) {
		float x = static_cast<float>(i * 2) - static_cast<float>(g_playerCount - 1); // 原点付近に分散配置
		XMFLOAT3 pos = { x,16.0f,0.0f };
		// ★ 修正点: プレイヤーの初期向きをZ軸負方向に変更
		g_players[i] = new Player(g_pKirby, g_ball, pos, { 0.0f,10.0f,-1.0f });
		// コントローラーを割り当て（スロット i）
		g_players[i]->SetController(g_controllers[i]);
		// UI 用の互換グローバル（player0）を設定
		if (i == 0) g_player = g_players[i];
		// 各プレイヤーにカメラを作成
		g_players[i]->CreateCamera({ 0.0f,1.0f,0.0f });
	}

	// カメラマネージャーの設定
	g_camMgr.AddCamera(&g_debugCamera, "debug");
	// 各プレイヤーのカメラを固有名で登録
	for (int i = 0; i < g_playerCount; ++i) {
		if (g_players[i] && g_players[i]->GetCamera()) {
			std::string name = std::string("player") + std::to_string(i);
			g_camMgr.AddCamera(g_players[i]->GetCamera(), name.c_str());
		}
	}
	// デフォルトはデバッグカメラに設定
	g_camMgr.SetActiveByName("debug");

	// UI 初期化
	g_uiCharge = new UI_Charge();
	g_uiCharge->SetPosition({ 20.0f,20.0f });
	g_uiCharge->SetSize({ 200.0f,24.0f });
	g_uiCharge->Initialize();
	UIManager::Add(g_uiCharge);

	// UI 用のグローバルプレイヤーポインタを設定
	g_player = g_players[0];

	// アイテムジェネレーターの初期化
	g_itemGenerator = new ItemGenerator();
	g_itemGenerator->Initialize();
	g_itemGenerator->GenerateItemsOnPowerLines(g_MapInstance);

	// ボタンヒント UI の初期化
	g_buttonHintUI = new ButtonHintUI(Direct3D_GetDevice(), Direct3D_GetContext(), Direct3D_GetBackBufferWidth(), Direct3D_GetBackBufferHeight());
}

void Game_Finalize()
{
	ModelRelease(g_pKirby);

	Light_Finalize();
	Grid_Finalize();
	g_MapInstance.Finalize();
	g_ObjectManager.Finalize();

	// デバッグAABB描画の終了処理
	DebugAABB::Finalize();

	if (g_uiCharge) {
		UIManager::Remove(g_uiCharge);
		delete g_uiCharge;
		g_uiCharge = nullptr;
	}

	// アイテムジェネレーターの終了処理
	if (g_itemGenerator) {
		delete g_itemGenerator;
		g_itemGenerator = nullptr;
	}

	// ボタンヒント UI の終了処理
	if (g_buttonHintUI) {
		delete g_buttonHintUI;
		g_buttonHintUI = nullptr;
	}

	// プレイヤーを削除
	for (int i = 0; i < 3; ++i) {
		if (g_players[i]) {
			if (g_player == g_players[i]) g_player = nullptr; // グローバルと一致する場合はクリア
			delete g_players[i];
			g_players[i] = nullptr;
		}
	}

	// コントローラーを解放（スレッド停止・Join を行う）
	for (int i = 0; i < 3; ++i) {
		if (g_controllers[i]) {
			// 実行中のスレッドがあれば停止してから join
			g_controllers[i]->Stop();
			g_controllers[i]->Join();
			delete g_controllers[i];
			g_controllers[i] = nullptr;
		}
	}

	// グローバルプレイヤーポインタをリセット
	g_player = nullptr;
}

static double keika_time = 0.0;

void Game_Update(double elapsed_time)
{
	keika_time += elapsed_time;

	// キーボードのトリガー状態を更新
	KeyLogger_Update();

	// コントローラーを更新
	for (int i = 0; i < 3; ++i) if (g_controllers[i]) g_controllers[i]->Update();

	// カメラ管理
	// Tabでデバッグカメラと player0 カメラをトグル
	if (KeyLogger_IsTrigger(KK_TAB)) {
		Camera* current = g_camMgr.GetActive();
		if (current == &g_debugCamera) {
			if (g_players[0] && g_players[0]->GetCamera()) {
				g_camMgr.SetActiveByName("player0");
			}
		}
		else {
			g_camMgr.SetActiveByName("debug");
		}
	}
	else {
		// コントローラーの X ボタンでカメラを順次切替
		if (g_controllers[0] && g_controllers[0]->WasPressed(Controller::BUTTON_X)) {
			g_camMgr.Next();
		}
		else {
			for (int i = 1; i < 3; ++i) {
				if (g_controllers[i] && g_controllers[i]->WasPressed(Controller::BUTTON_X)) { g_camMgr.Next(); break; }
			}
		}
	}

	g_camMgr.UpdateActive(elapsed_time);

	Fade_Update(elapsed_time);
	Grid_Update(elapsed_time);

	// プレイヤーを更新
	for (int i = 0; i < g_playerCount; ++i) {
		if (g_players[i]) g_players[i]->Update(elapsed_time);
	}

	// アイテムジェネレーターの更新
	if (g_itemGenerator) {
		g_itemGenerator->Update(elapsed_time);

		// ItemGeneratorObject が生成したアイテムを登録
		auto generators = g_ObjectManager.GetAllItemGenerators();
		for (auto generator : generators) {
			if (generator) {
				const auto& items = generator->GetSpawnedItems();
				for (auto item : items) {
					// すでに登録されているかチェック（簡易的）
					bool alreadyRegistered = false;
					for (auto registeredItem : g_itemGenerator->GetItems()) {
						if (registeredItem == item) {
							alreadyRegistered = true;
							break;
						}
					}
					if (!alreadyRegistered) {
						g_itemGenerator->RegisterItem(item);
					}
				}
			}
		}

		// プレイヤーのピックアップ判定
		for (int i = 0; i < g_playerCount; ++i) {
			if (g_players[i]) {
				g_itemGenerator->CheckPickup(g_players[i]);
			}
		}
	}

	// ボタンヒント UI の更新
	if (g_buttonHintUI && g_players[0]) {
		g_buttonHintUI->Update(g_players[0], elapsed_time);
	}

	// UI 更新
	UIManager::UpdateAll(elapsed_time);
}

void Game_Draw()
{
	// マルチプレイヤー時は分割表示
	const float SCREEN_WIDTH = static_cast<float>(Direct3D_GetBackBufferWidth());
	const float SCREEN_HEIGHT = static_cast<float>(Direct3D_GetBackBufferHeight());

	if (g_playerCount <= 1) {
		// シングルカメラパス:既にアクティブなカメラを使用。アクティブなカメラが無い場合のみデフォルトを設定
		if (!g_camMgr.GetActive()) {
			if (g_players[0] && g_players[0]->GetCamera()) {
				g_camMgr.SetActiveByName("player0");
			}
			else {
				g_camMgr.SetActiveByName("debug");
			}
		}
		g_camMgr.ApplyActive();

		Direct3D_SetDepthTest(true);

		Grid_Draw();

		XMFLOAT3 direction;
		{
			XMVECTOR dirVec = XMVectorSet(-1.0f, -1.5f, 1.0f, 0.0f);
			XMStoreFloat3(&direction, dirVec);
		}
		Light_SetDiffuse({ 0.7f,0.7f,0.7f }, direction);

		Light_SetAmbient({ 0.2f,0.1f,0.1f,1.0f });
		ModelDraw(g_pKirby, XMMatrixTranslation(0.0f, 2.0f, 0.0f));

		Light_SetAmbient({ 0.2f,0.1f,0.1f,1.0f });
		ModelDraw(g_test, XMMatrixIdentity());

		Light_SetAmbient({ 0.1f,0.1f,0.1f,1.0f });
		g_MapInstance.Draw();
		g_ObjectManager.Draw(); // g_MapInstance.Draw() の後に追加

		// アイテムジェネレーターを描画
		if (g_itemGenerator) {
			Light_SetAmbient({ 0.5f,0.5f,0.5f,1.0f });
			g_itemGenerator->Draw();
		}

		Light_SetAmbient({ 0.3f,0.3f,0.3f,1.0f });
		for (int i = 0; i < g_playerCount; ++i) {
			if (g_players[i]) {
				g_players[i]->Draw();

				// ★ 追加: プレイヤーの向きを可視化するデバッグキューブを描画
				Player* p = g_players[i];
				XMFLOAT3 playerPos = p->GetPosition();
				XMFLOAT3 playerDir = p->GetDirection();

				// プレイヤーの少し前の位置を計算
				float offset = 1.0f; // プレイヤーからの距離
				XMVECTOR posVec = XMLoadFloat3(&playerPos);
				XMVECTOR dirVec = XMLoadFloat3(&playerDir);
				XMVECTOR cubePosVec = posVec + dirVec * offset;

				// キューブのワールド行列を作成（小さく表示）
				XMMATRIX scale = XMMatrixScaling(0.2f, 0.2f, 0.2f);
				XMMATRIX translation = XMMatrixTranslationFromVector(cubePosVec);
				XMMATRIX world = scale * translation;

				// デバッグキューブを描画
				Cube_Draw(0, world);
			}
		}

		// デバッグ用AABB描画
		//g_ObjectManager.DrawDebugAABBs();
		Direct3D_SetDepthTest(false);

		//2D UI 描画
		UIManager::DrawAll();




		// プレイヤーの体力を DebugText で左下に表示
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
			g_debugText->SetText(healthText, { 1.0f, 1.0f, 1.0f, 1.0f }); // 白色で表示
			g_debugText->Draw();
		}

		// ボタンヒント UI を描画
		if (g_buttonHintUI) {
			g_buttonHintUI->Draw();
		}

		return;
	}

	// 複数プレイヤー: ビューポートを設定して各プレイヤーのビューを描画
	ID3D11DeviceContext* ctx = Direct3D_GetContext();

	// 各プレイヤーのカメラでフル3Dシーンをレンダリング
	for (int i = 0; i < g_playerCount; ++i) {
		if (!g_players[i] || !g_players[i]->GetCamera()) continue;

		D3D11_VIEWPORT vp = {};
		if (g_playerCount == 2) {
			// 横並び
			vp.TopLeftX = static_cast<FLOAT>(i * (SCREEN_WIDTH * 0.5f));
			vp.TopLeftY = 0.0f;
			vp.Width = SCREEN_WIDTH * 0.5f;
			vp.Height = SCREEN_HEIGHT;
		}
		else if (g_playerCount == 3) {
			if (i == 0) {
				// 左半分を縦いっぱい
				vp.TopLeftX = 0.0f;
				vp.TopLeftY = 0.0f;
				vp.Width = SCREEN_WIDTH * 0.5f;
				vp.Height = SCREEN_HEIGHT;
			}
			else {
				//右側を上下に分割
				vp.TopLeftX = SCREEN_WIDTH * 0.5f;
				vp.Width = SCREEN_WIDTH * 0.5f;
				vp.Height = SCREEN_HEIGHT * 0.5f;
				vp.TopLeftY = (i == 1) ? 0.0f : SCREEN_HEIGHT * 0.5f;
			}
		}
		else {
			// フォールバック: フルスクリーン
			vp.TopLeftX = 0.0f; vp.TopLeftY = 0.0f; vp.Width = SCREEN_WIDTH; vp.Height = SCREEN_HEIGHT;
		}
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;

		ctx->RSSetViewports(1, &vp);

		// このプレイヤーのカメラをアクティブにする
		std::string name = std::string("player") + std::to_string(i);
		g_camMgr.SetActiveByName(name.c_str());
		g_camMgr.ApplyActive();

		Direct3D_SetDepthTest(true);

		// シーン描画
		Grid_Draw();

		XMFLOAT3 direction;
		{
			XMVECTOR dirVec = XMVectorSet(-1.0f, -1.5f, 1.0f, 0.0f);
			XMStoreFloat3(&direction, dirVec);
		}
		Light_SetDiffuse({ 0.7f,0.7f,0.7f }, direction);

		Light_SetAmbient({ 0.2f,0.1f,0.1f,1.0f });
		ModelDraw(g_pKirby, XMMatrixTranslation(0.0f, 2.0f, 0.0f));

		Light_SetAmbient({ 0.2f,0.1f,0.1f,1.0f });
		ModelDraw(g_test, XMMatrixIdentity());

		Light_SetAmbient({ 0.1f,0.1f,0.1f,1.0f });
		g_MapInstance.Draw();
		g_ObjectManager.Draw(); // g_MapInstance.Draw() の後に追加

		// アイテムジェネレーターを描画
		if (g_itemGenerator) {
			Light_SetAmbient({ 0.5f,0.5f,0.5f,1.0f });
			g_itemGenerator->Draw();
		}

		Light_SetAmbient({ 0.3f,0.3f,0.3f,1.0f });
		for (int j = 0; j < g_playerCount; ++j) {
			if (g_players[j]) g_players[j]->Draw();
		}

		Direct3D_SetDepthTest(false);

		// 必要なら個別ビュー向けの HUD をここで描画（省略中）
	}

	// フルビューポートに戻す
	D3D11_VIEWPORT vpFull = {};
	vpFull.TopLeftX = 0.0f; vpFull.TopLeftY = 0.0f;
	vpFull.Width = SCREEN_WIDTH; vpFull.Height = SCREEN_HEIGHT;
	vpFull.MinDepth = 0.0f; vpFull.MaxDepth = 1.0f;
	ctx->RSSetViewports(1, &vpFull);

	// デバッグ用AABB描画（マルチプレイヤー時）
	Direct3D_SetDepthTest(true);
	g_ObjectManager.DrawDebugAABBs();
	Direct3D_SetDepthTest(false);

	// フルスクリーンでグローバル UI を一度描画
	UIManager::DrawAll();
}
