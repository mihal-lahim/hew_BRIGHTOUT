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
#include "debug_console.h"
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
#include "ChargingSpot.h"
#include "UI_house.h"
#include "scene.h"
#include "UI_Timer.h"
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
// UI: ハウス状態表示
static UIHouse* g_uiHouse = nullptr;
// プレイヤーの体力表示用 DebugText
static hal::DebugText* g_debugText = nullptr;

// アイテムジェネレーター
static ItemGenerator* g_itemGenerator = nullptr;

// ボタン指示UI
static ButtonHintUI* g_buttonHintUI = nullptr;

// タイマーUI
static UI_Timer* g_uiTimer = nullptr;

void Game_SetPlayerCount(int count)
{
	if (count < 1) count = 1;
	if (count > 3) count = 3;
	g_playerCount = count;
}

//============================================================================
// コントローラー初期化・プレイヤー数設定関数
//============================================================================
/**
 * @brief コントローラーを初期化し、接続されているコントローラー数からプレイヤー数を設定
 */
void Game_InitializeControllers()
{
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

	// 接続されているコントローラー数をカウント
	int connectedCount = 0;
	for (int i = 0; i < 3; ++i) {
		if (g_controllers[i]) {
			// コントローラーがデバイスに接続されているかチェック
			// XInputGetState()で成功（ERROR_SUCCESS）なら接続状態
			XINPUT_STATE state{};
			if (XInputGetState(i, &state) == ERROR_SUCCESS) {
				connectedCount++;
				hal::DebugConsole::GetInstance().LogFormat("[Controller] Slot %d connected", i);
			}
		}
	}

	// 接続されているコントローラー数に基づいてプレイヤー数を設定
	if (connectedCount > 0) {
		Game_SetPlayerCount(connectedCount);
		hal::DebugConsole::GetInstance().LogFormat("[Game] Player count set to: %d (from %d connected controllers)", g_playerCount, connectedCount);
	} else {
		// コントローラーが接続されていない場合はデフォルト（1プレイヤー）
		Game_SetPlayerCount(1);
		hal::DebugConsole::GetInstance().Log("[Game] No controllers connected. Default to 1 player");
	}
}

//============================================================================
// ハウス作成ヘルパー関数
//============================================================================
/**
 * @brief ハウスを作成して ObjectManager に追加
 * @param position ハウスの位置
 * @param scale ハウスのスケール（1.0f = 100%）
 * @param maxElectricity ハウスの最大電気量
 * @param model ハウスモデル（省略時は g_houseModel を使用）
 */
void CreateHouse(const DirectX::XMFLOAT3& position, float scale, float maxElectricity, struct MODEL* model = nullptr)
{
	if (!model) model = g_houseModel;
	
	auto house = std::make_unique<House>(
		position,
		model,
		maxElectricity
	);
	
	// スケールを設定
	house->SetScale(scale);
	
	g_ObjectManager.AddGameObject(std::move(house));
}

//============================================================================
// 電柱作成ヘルパー関数
//============================================================================
/**
 * @brief 電柱を作成して ObjectManager に追加
 * @param position 電柱の位置
 * @param height 電柱の高さ
 * @param radius 電柱の半径
 * @param poleID 電柱の ID
 */
void CreatePole(const DirectX::XMFLOAT3& position, float height, float radius, int& poleID)
{
	auto pole = std::make_unique<Pole>(position, height, radius);
	pole->SetPoleID(poleID++);
	g_ObjectManager.AddGameObject(std::move(pole));
}

//============================================================================
// アイテムジェネレータオブジェクト作成ヘルパー関数
//============================================================================
/**
 * @brief アイテムジェネレータオブジェクトを作成して ObjectManager に追加
 * @param position ジェネレータの位置
 * @param spawnRadius スポーン範囲（半径）
 * @param spawnInterval スポーン間隔（秒）
 * @param generatorID ジェネレータの ID
 */
void CreateItemGenerator(const DirectX::XMFLOAT3& position, float spawnRadius, float spawnInterval, int& generatorID)
{
	auto generator = std::make_unique<ItemGeneratorObject>(position, spawnRadius, spawnInterval);
	generator->SetGeneratorID(generatorID++);
	g_ObjectManager.AddGameObject(std::move(generator));
}

//============================================================================
// 充電スポット作成ヘルパー関数
//============================================================================
/**
 * @brief 充電スポットを作成して ObjectManager に追加
 * @param position 充電スポットの位置
 * @param chargeRadius 充電範囲（半径）
 * @param chargeRate 1秒あたりの回復量
 */
void CreateChargingSpot(const DirectX::XMFLOAT3& position, float chargeRadius, float chargeRate)
{
	auto chargingSpot = std::make_unique<ChargingSpot>(position, chargeRadius, chargeRate);
	g_ObjectManager.AddGameObject(std::move(chargingSpot));
}

//============================================================================
// ゲーム初期化
//============================================================================
void Game_Initialize()
{
	// デバッグコンソールの初期化
	hal::DebugConsole::GetInstance().Initialize(true);
	hal::DebugConsole::GetInstance().Success("=== Game Initialize Started ===");


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

	// 電柱を ObjectManager に追加（ヘルパー関数使用）
	int poleID = 0;
	CreatePole(DirectX::XMFLOAT3(-20.0f, 0.0f, -20.0f), 4.0f, 0.2f, poleID);
	CreatePole(DirectX::XMFLOAT3(20.0f, 0.0f, -20.0f), 4.0f, 0.2f, poleID);
	CreatePole(DirectX::XMFLOAT3(-20.0f, 0.0f, 20.0f), 4.0f, 0.2f, poleID);
	CreatePole(DirectX::XMFLOAT3(20.0f, 0.0f, 20.0f), 4.0f, 0.2f, poleID);
	CreatePole(DirectX::XMFLOAT3(0.0f, 0.0f, -20.0f), 4.0f, 0.2f, poleID);
	CreatePole(DirectX::XMFLOAT3(0.0f, 0.0f, 20.0f), 4.0f, 0.2f, poleID);

	// 電柱同士を電線で自動接続
	g_ObjectManager.ConnectNearbyPoles();

	// ハウスを ObjectManager に追加（ヘルパー関数使用）
	CreateHouse(DirectX::XMFLOAT3(-15.0f, 0.0f, -15.0f), 5.0f, 100.0f);
	CreateHouse(DirectX::XMFLOAT3(15.0f, 0.0f, 15.0f), 5.0f, 100.0f);

	// アイテムジェネレーターオブジェクトをフィールドに配置（ヘルパー関数使用）
	int generatorID = 0;
	CreateItemGenerator(DirectX::XMFLOAT3(-15.0f, 1.0f, 0.0f), 5.0f, 2.0f, generatorID);
	CreateItemGenerator(DirectX::XMFLOAT3(15.0f, 1.0f, 0.0f), 5.0f, 2.0f, generatorID);
	CreateItemGenerator(DirectX::XMFLOAT3(0.0f, 1.0f, -15.0f), 5.0f, 2.0f, generatorID);
	CreateItemGenerator(DirectX::XMFLOAT3(0.0f, 1.0f, 15.0f), 5.0f, 2.0f, generatorID);

	// 充電スポットを ObjectManager に追加（ヘルパー関数使用）
	CreateChargingSpot(DirectX::XMFLOAT3(-25.0f, 0.5f, 0.0f), 5.0f, 20.0f);
	CreateChargingSpot(DirectX::XMFLOAT3(25.0f, 0.5f, 0.0f), 5.0f, 20.0f);

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

	// UI ハウス状態表示の初期化
	g_uiHouse = new UIHouse();
	g_uiHouse->Initialize();
	UIManager::Add(g_uiHouse);

	// UI タイマーの初期化
	g_uiTimer = new UI_Timer();
	g_uiTimer->SetPosition( 20.0f, 50.0f );
	g_uiTimer->SetSize({ 150.0f, 30.0f });
	g_uiTimer->Initialize();
	UIManager::Add(g_uiTimer);

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

	// UI ハウス状態表示の終了化
	if (g_uiHouse) {
		UIManager::Remove(g_uiHouse);
		delete g_uiHouse;
		g_uiHouse = nullptr;
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
			if (g_player == g_players[i]) g_player = nullptr;
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

	// デバッグコンソールのシャットダウン
	hal::DebugConsole::GetInstance().Success("=== Game Finalized ===");
	hal::DebugConsole::GetInstance().Shutdown();
}

static double keika_time = 0.0;

void Game_Update(double elapsed_time)
{
	keika_time += elapsed_time;

	// コントローラーを更新
	for (int i = 0; i < 3; ++i) if (g_controllers[i]) g_controllers[i]->Update();

	// カメラ管理 - キーボードのみで操作
	// TAB キーでデバッグカメラと player0 カメラをトグル
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

	// C キーでカメラを順次切り替え
	if (KeyLogger_IsTrigger(KK_C)) {
		g_camMgr.Next();
	}

	Fade_Update(elapsed_time);
	Grid_Update(elapsed_time);

	// プレイヤーを更新
	for (int i = 0; i < g_playerCount; ++i) {
		if (g_players[i]) g_players[i]->Update(elapsed_time);
	}

	// アイテムジェネレーターの更新
	if (g_itemGenerator) {
		g_itemGenerator->Update(elapsed_time);

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

	// ハウスへの電気供給
	// Yボタンを押している間、継続的に給電
	if (g_players[0] && g_controllers[0]) {
		House* nearestHouse = g_players[0]->GetNearestHouse();

		// Yボタンを押している間は給電を継続
		if (g_controllers[0]->IsDown(Controller::BUTTON_Y)) {
			if (nearestHouse) {
				// ハウスが復旧していない場合のみ給電
				if (!nearestHouse->IsRepaired()) {
					// ハウスが範囲内なら給電を開始または継続
					if (!g_players[0]->IsSupplyingElectricity()) {
						g_players[0]->StartSupplyingElectricity(nearestHouse);
					}

					// 供給中なら継続的に電気を転送
					if (g_players[0]->IsSupplyingElectricity()) {
						House* supplyingHouse = g_players[0]->GetSupplyingHouse();
						if (supplyingHouse) {
							// プレイヤーが体力不足の場合は供給を停止
							if (g_players[0]->GetHealth() <= 0.0f) {
								g_players[0]->StopSupplyingElectricity();
							}
							else {
								g_players[0]->TransferElectricityToHouse(supplyingHouse, elapsed_time);
							}
						}
					}
				}
				else {
					// ハウスが既に復旧している場合は給電を停止
					if (g_players[0]->IsSupplyingElectricity()) {
						g_players[0]->StopSupplyingElectricity();
					}
				}
			}
		}
		else {
			// ボタンを離したら給電を停止
			if (g_players[0]->IsSupplyingElectricity()) {
				g_players[0]->StopSupplyingElectricity();
			}
		}
	}

	// 充電スポットでの体力回復
	// LEFT_SHOULDER ボタン（LB）を押している間、充電スポット範囲内で体力を継続回復
	if (g_players[0] && g_controllers[0]) {
		const auto& allObjects = g_ObjectManager.GetGameObjects();

		for (const auto& obj : allObjects) {
			if (obj->GetTag() == GameObjectTag::CHARGING_SPOT) {
				ChargingSpot* spot = static_cast<ChargingSpot*>(obj.get());
				if (!spot) continue;

				// プレイヤーが充電スポット範囲内かチェック
				if (spot->IsPlayerInRange(g_players[0]->GetPosition())) {
					spot->SetChargeRate(1.0f);
					// LEFT_SHOULDER ボタンを押している間、体力を継続回復
					if (g_controllers[0]->IsDown(Controller::BUTTON_LEFT_SHOULDER)) {
						float chargeAmount = spot->GetChargeRate();
						g_players[0]->Heal(chargeAmount);
					}
				}
			}
		}
	}

	// ハウスのリストを更新（毎フレーム）
	if (g_uiHouse) {
		std::vector<House*> houses;
		const auto& allObjects = g_ObjectManager.GetGameObjects();

		int repaired_count = 0;
		int total_house_count = 0;

		for (const auto& obj : allObjects) {
			if (obj->GetTag() == GameObjectTag::HOUSE) {
				House* house = static_cast<House*>(obj.get());
				if (house) {
					houses.push_back(house);
					total_house_count++;
					if (house->IsRepaired()) {
						repaired_count++;
					}
				}
			}
		}

		g_uiHouse->SetHouses(houses);

		// すべての家が復旧されたらリザルトシーンに遷移
		if (total_house_count > 0 && repaired_count == total_house_count) {
			Scene_SetNextScene(SCENE_RESULT);
		}
	}

	// === デバッグ表示：毎フレーム重要な状態情報をコンソールに出力 ===
	static float debugUpdateTimer = 0.0f;
	debugUpdateTimer += static_cast<float>(elapsed_time);

	// 0.5秒ごとにカーソルをリセットして上書き表示（スクロール履歴は保持）
	if (debugUpdateTimer >= 0.5f) {
		debugUpdateTimer = 0.0f;

		//コンソール画面のクリア
		hal::DebugConsole::GetInstance().ClearScreen();

		// === ヘッダー表示 ===
		DEBUG_LOG("========== GAME DEBUG CONSOLE ==========");
		DEBUG_LOGF("Time: %.1f sec", keika_time);
		DEBUG_LOG("");

		// プレイヤーの詳細情報
		if (g_players[0]) {
			XMFLOAT3 playerPos = g_players[0]->GetPosition();
			float playerHP = g_players[0]->GetHealth();
			float playerMaxHP = g_players[0]->GetMaxHealth();
			bool isSupplying = g_players[0]->IsSupplyingElectricity();
			XMFLOAT3 playerDir = g_players[0]->GetDirection();
			bool isGrounded = g_players[0]->IsGrounded();
			bool isNearPole = g_players[0]->IsNearPole();

			DEBUG_LOG("--- PLAYER INFO ---");
			DEBUG_LOGF("Position: (%.1f, %.1f, %.1f)", playerPos.x, playerPos.y, playerPos.z);
			DEBUG_LOGF("Direction: (%.2f, %.2f, %.2f)", playerDir.x, playerDir.y, playerDir.z);
			DEBUG_LOGF("HP: %.1f / %.1f", playerHP, playerMaxHP);
			DEBUG_LOGF("Grounded: %s | Near Pole: %s", isGrounded ? "YES" : "NO", isNearPole ? "YES" : "NO");
			DEBUG_LOGF("Supplying Electricity: %s", isSupplying ? "YES" : "NO");
			if (isSupplying) {
				House* supplyingHouse = g_players[0]->GetSupplyingHouse();
				if (supplyingHouse) {
					XMFLOAT3 housePos = supplyingHouse->GetPosition();
					DEBUG_LOGF("  -> House at (%.1f, %.1f, %.1f)", housePos.x, housePos.y, housePos.z);
				}
			}
			DEBUG_LOG("");
		}

		// ハウスの情報
		{
			const auto& allObjects = g_ObjectManager.GetGameObjects();
			int houseCount = 0;
			float totalElectricity = 0.0f;
			int repairedCount = 0;

			for (const auto& obj : allObjects) {
				if (obj->GetTag() == GameObjectTag::HOUSE) {
					House* house = static_cast<House*>(obj.get());
					if (house) {
						houseCount++;
						totalElectricity += house->GetElectricity();
						if (house->IsRepaired()) repairedCount++;
					}
				}
			}

			if (houseCount > 0) {
				DEBUG_LOG("--- HOUSES INFO ---");
				DEBUG_LOGF("Count: %d", houseCount);
				DEBUG_LOGF("Total Electricity: %.1f", totalElectricity);
				DEBUG_LOGF("Repaired: %d / %d", repairedCount, houseCount);

				// 個別の家の情報
				int houseIndex = 1;
				for (const auto& obj : allObjects) {
					if (obj->GetTag() == GameObjectTag::HOUSE) {
						House* house = static_cast<House*>(obj.get());
						if (house) {
							XMFLOAT3 housePos = house->GetPosition();
							DEBUG_LOGF("  House%d: %.1f/%.1f (%.1f, %.1f, %.1f) %s",
								houseIndex++,
								house->GetElectricity(), house->GetMaxElectricity(),
								housePos.x, housePos.y, housePos.z,
								house->IsRepaired() ? "[REPAIRED]" : "");
						}
					}
				}
				DEBUG_LOG("");
			}
		}

		// オブジェクト統計
		{
			const auto& allObjects = g_ObjectManager.GetGameObjects();
			int poleCount = 0, powerLineCount = 0, generatorCount = 0, chargingSpotCount = 0;

			for (const auto& obj : allObjects) {
				switch (obj->GetTag()) {
				case GameObjectTag::POLE: poleCount++; break;
				case GameObjectTag::POWER_LINE: powerLineCount++; break;
				case GameObjectTag::ITEM_GENERATOR: generatorCount++; break;
				case GameObjectTag::CHARGING_SPOT: chargingSpotCount++; break;
				default: break;
				}
			}

			DEBUG_LOG("--- SCENE OBJECTS ---");
			DEBUG_LOGF("Poles: %d | PowerLines: %d", poleCount, powerLineCount);
			DEBUG_LOGF("ItemGenerators: %d | ChargingSpots: %d", generatorCount, chargingSpotCount);
			DEBUG_LOG("========================================");
		}
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
			snprintf(healthText, sizeof(healthText), "HP: %.1f / %.1f", g_players[0]->GetHealth(), g_players[0]->GetMaxHealth());
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
