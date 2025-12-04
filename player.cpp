#include "player.h"
#include <algorithm>
#include <cmath>
#include <DirectXMath.h>
#include "model.h"
#include "controller.h"
#include "top_down_camera.h"
#include "map.h"
#include "ObjectManager.h"
#include "Pole.h"
#include "PowerLine.h"
#include "house.h"
#include "debug_console.h"

// 入力反転フラグ（必要に応じて調整）
static constexpr bool INVERT_LS_X = true; // 左右が反転しているので X を反転
static constexpr bool INVERT_LS_Y = true; // 前後が反転しているので Y を反転

using namespace DirectX;

// プレイヤークラス実装
Player::~Player() = default;

Player::Player()
	: GameObject(DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f))
	, model_(nullptr)
	, electricModel_(nullptr)
	, direction_(0.0f, 0.0f, 1.0f)
	, health_(100.0f)
	, maxHealth_(100.0f)
	, usePlayer(true)
{
}

Player::Player(const XMFLOAT3& pos)
	: GameObject(pos)
{
}


Player::Player(MODEL* model, MODEL* electricModel, const XMFLOAT3& pos, const XMFLOAT3& dir)
	: GameObject(pos, model)
	, model_(model)
	, electricModel_(electricModel)
	, direction_(dir)
{
	health_ = maxHealth_ = 100.0f;	
	usePlayer = true;

	
}

// 毎フレーム更新（ダッシュ継続時間の管理、入力処理）
void Player::Update(double elapsedSec)
{
	// 1. ダッシュ時間の更新
	if (isDashing_) {
		dashTimeRemaining_ -= static_cast<float>(elapsedSec);
		if (dashTimeRemaining_ <= 0.0f) {
			isDashing_ = false;
			currentSpeed_ = (state == State::ELECTRICITY) ? (baseSpeed_ * electricSpeedmul) : baseSpeed_;
			dashTimeRemaining_ = 0.0f;
		}
	}

	// 衝突判定スキップタイマーの更新
	if (skipCollisionTimer_ > 0.0f) {
		skipCollisionTimer_ -= static_cast<float>(elapsedSec);
	}

	// カメラを更新（右スティックの入力処理を含む）
	if (camera_) {
		camera_->Update(elapsedSec);
	}

	// 電線ダメージ処理：電気状態の間、定期的にダメージを受ける
	if (state == State::ELECTRICITY) {
		powerLineDamageTimer_ -= static_cast<float>(elapsedSec);
		if (powerLineDamageTimer_ <= 0.0f) {
			TakeDamage(POWERLINE_DAMAGE_AMOUNT);
			powerLineDamageTimer_ = POWERLINE_DAMAGE_INTERVAL; // タイマーをリセット
		}
	}

	// 1. 入力に基づいて水平方向の移動ベクトルを決定
	XMFLOAT3 horizontalMove = { 0.0f, 0.0f, 0.0f };
	if (controller_) {
		// 電気ボタン (B ボタンで変身)
		if (controller_->WasPressed(Controller::BUTTON_B)) {
			if (state == State::HUMAN) {
				// HUMAN -> ELECTRICITY: 電柱近く必須
				if (IsNearPole()) {
					ChangeState(State::ELECTRICITY);
					// 電気状態へのリセット処理
					ResetToElectricityState();
				}
			} else {
				// ELECTRICITY -> HUMAN: 電柱近く必須
				if (IsNearPole()) {
					ChangeState(State::HUMAN);
					if (!isDashing_) {
					currentSpeed_ = baseSpeed_;
					}
					// 電柱から跳ね返す処理
					KnockbackFromPole();
					// 0.5秒間、衝突判定をスキップ
					skipCollisionTimer_ = SKIP_COLLISION_DURATION;
				}
			}
		}

		// スティック入力
		auto left = controller_->GetStick(Controller::DIR_LEFT);
		if (left.outside) {
			float cameraAngle = camera_ ? camera_->GetCameraAngle() : 0.0f;
			float stickX = static_cast<float>(left.x) * (INVERT_LS_X ? -1.0f : 1.0f);
			float stickY = static_cast<float>(left.y) * (INVERT_LS_Y ? -1.0f : 1.0f);

			XMVECTOR camForward = XMVectorSet(sinf(cameraAngle), 0.0f, cosf(cameraAngle), 0.0f);
			XMVECTOR camRight = XMVectorSet(cosf(cameraAngle), 0.0f, -sinf(cameraAngle), 0.0f);
			XMVECTOR moveVec = XMVector3Normalize(XMVectorAdd(XMVectorScale(camRight, stickX), XMVectorScale(camForward, stickY)));
			XMStoreFloat3(&horizontalMove, moveVec);
		}

		// ダッシュ開始（電気状態のみ）
		if (controller_->WasPressed(Controller::BUTTON_A) && state == State::ELECTRICITY && !isDashing_) {
			StartDash();
		}
        // デバッグ用ダメージ
		if (controller_->WasPressed(Controller::BUTTON_RIGHT_SHOULDER)) {
			TakeDamage(10);
		}
		//死亡用判定
		if(!IsAlive())
		{
			usePlayer = false;
		}
		//リスポーン処理
		if (usePlayer == false)
		{
			health_ = maxHealth_;
			m_Position = XMFLOAT3(0.0f, 15.0f, 0.0f);
			usePlayer = true;
			
			// 状態をHUMANにリセット
			ChangeState(State::HUMAN);
			
			// 移動速度を設定
			currentSpeed_ = baseSpeed_;
			
			// ダッシュ状態をリセット
			isDashing_ = false;
			dashTimeRemaining_ = 0.0f;
			
			// その他の状態をリセット
			velocityY_ = 0.0f;
			isGrounded_ = false;
			powerLineDamageTimer_ = 0.0f;
			skipCollisionTimer_ = 0.0f;
		}
		
	}

	// 2. 重力を適用（毎フレーム速度を加算）
	if (!isGrounded_) {
		velocityY_ -= GRAVITY * static_cast<float>(elapsedSec);
	}

	// 3. 移動と衝突判定
	XMFLOAT3 desiredMove = {
		horizontalMove.x * currentSpeed_ * static_cast<float>(elapsedSec),
		velocityY_ * static_cast<float>(elapsedSec),
		horizontalMove.z * currentSpeed_ * static_cast<float>(elapsedSec)
	};
	
	// 電気状態の場合は衝突判定をスキップして直進
	if (state == State::ELECTRICITY) {
		m_Position.x += desiredMove.x;
		m_Position.z += desiredMove.z;
		velocityY_ = 0.0f;
		isGrounded_ = true;
	} else if (skipCollisionTimer_ > 0.0f) {
		// 状態変化直後のスキップタイマー期間、衝突判定をスキップ
		m_Position.x += desiredMove.x;
		m_Position.z += desiredMove.z;
		m_Position.y += desiredMove.y;
	} else {
		// HUMAN状態は通常の衝突判定を適用
		ResolveCollisions(desiredMove, elapsedSec);
	}

	// 4. 電気状態の場合、電線にスナップさせて落下防止
	if (state == State::ELECTRICITY) {
		SnapToNearestPowerLine();
	}

	// 5. 向きの更新
	if (horizontalMove.x != 0.0f || horizontalMove.z != 0.0f) {
		direction_ = horizontalMove;
	}
}

// 衝突解決
void Player::ResolveCollisions(DirectX::XMFLOAT3& desiredMove, double elapsedSec)
{
    // XZ平面（水平）の移動と衝突判定
    XMFLOAT3 newPos = m_Position;
    newPos.x += desiredMove.x;
    newPos.z += desiredMove.z;

    AABB playerAABB = GetAABBAt(newPos);
    bool collisionXZ = false;

    extern ObjectManager g_ObjectManager;
    for (const auto& obj : g_ObjectManager.GetGameObjects()) {
        // HUMAN状態の際、PowerLineとの衝突をスキップ
        if (state == State::HUMAN && obj->GetTag() == GameObjectTag::POWER_LINE) {
            continue;
        }
        // プレイヤー自身は衝突判定から除外
        if (obj->GetTag() == GameObjectTag::PLAYER) {
            continue;
        }
        
        if (playerAABB.IsOverlap(obj->GetAABB())) {
            collisionXZ = true;
            break;
        }
    }

    if (!collisionXZ) {
        m_Position = newPos; // 衝突なければ移動を適用
    }

    // Y軸（垂直）の移動と衝突判定
    m_Position.y += desiredMove.y;
    playerAABB = GetAABB(); // 更新されたXZ位置でAABBを再計算
    bool collisionY = false;

    for (const auto& obj : g_ObjectManager.GetGameObjects()) {
        // HUMAN状態の際、PowerLineとの衝突をスキップ
        if (state == State::HUMAN && obj->GetTag() == GameObjectTag::POWER_LINE) {
            continue;
        }
        // プレイヤー自身は衝突判定から除外
        if (obj->GetTag() == GameObjectTag::PLAYER) {
            continue;
        }
        
        if (playerAABB.IsOverlap(obj->GetAABB())) {
            // オブジェクトの上にあるか、下にあるかで分ける
            if (desiredMove.y < 0.0f) { // 落下時
                m_Position.y = obj->GetAABB().GetMax().y; // オブジェクトの上面にスナップ
                velocityY_ = 0.0f;
                isGrounded_ = true;
            } else if (desiredMove.y > 0.0f) { // 上昇時
                m_Position.y = obj->GetAABB().GetMin().y - aabbHalfSize.y * 2.0f; // オブジェクトの下面にスナップ
                velocityY_ = 0.0f;
            }
            collisionY = true;
            break;
        }
    }

    // 地面との最終チェック
    if (m_Position.y <= GROUND_LEVEL) {
        m_Position.y = GROUND_LEVEL;
        velocityY_ = 0.0f;
        isGrounded_ = true;
    } else if (!collisionY) {
        isGrounded_ = false;
    }
}

// 指定した位置でのAABBを計算する補助関数
AABB Player::GetAABBAt(const DirectX::XMFLOAT3& pos) const
{
    float halfW = aabbHalfSize.x;
    float height = aabbHalfSize.y * 2.0f; // AABBの高さは全て
    DirectX::XMFLOAT3 min{ pos.x - halfW, pos.y - 0.5f, pos.z - halfW };
    DirectX::XMFLOAT3 max{ pos.x + halfW, pos.y + height, pos.z + halfW };
    return AABB(min, max);
}

// プレイヤー描画（状態に応じてモデルを選択）
void Player::Draw() const
{
	MODEL* drawModel = (state == State::ELECTRICITY && electricModel_) ? electricModel_ : model_;
	if (!drawModel) return;
	// 回転とY軸のみ向きの値で作成
	float yaw = atan2f(direction_.x, direction_.z);
	XMMATRIX matRot = XMMatrixRotationY(yaw);
	XMMATRIX matTrans = XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z);
	XMMATRIX world = matRot * matTrans;

	ModelDraw(drawModel, world);
}

void Player::StartDash()
{
	// 電気状態のみダッシュ可能
	if (state != State::ELECTRICITY || isDashing_) return;
	isDashing_ = true;
	dashTimeRemaining_ = dashDuration_;
	// 電気状態のダッシュ速度 = 通常電気速度 * dashMultiplier_
	currentSpeed_ = baseSpeed_ * electricSpeedmul * dashMultiplier_;
}

void Player::StopDash()
{
	if (isDashing_) {
		isDashing_ = false;
		dashTimeRemaining_ = 0.0f;
		//解除後は状態に応じた通常速度
		currentSpeed_ = (state == State::ELECTRICITY) ? (baseSpeed_ * electricSpeedmul) : baseSpeed_;
	}
}

void Player::Jump(float jumpForce)
{
	// 地面に接しているい場合のみジャンプ可能
	if (isGrounded_) {
		velocityY_ = jumpForce;
		isGrounded_ = false;
	}
}

void Player::TakeDamage(float amount)
{
	health_ -= amount;
	if (health_ < 0.0f) health_ = 0.0f;
	
	const char* stateName = (state == State::ELECTRICITY) ? "ELECTRICITY" : "HUMAN";
	//DEBUG_LOGF("[TakeDamage] State=%s | Pos=(%.1f, %.1f, %.1f) | HP=%.1f/%.1f | Damage=%.2f", stateName, m_Position.x, m_Position.y, m_Position.z, health_, maxHealth_, amount);
}

void Player::Heal(float amount)
{
	health_ += amount;
	if (health_ > maxHealth_) health_ = maxHealth_;
	
	const char* stateName = (state == State::ELECTRICITY) ? "ELECTRICITY" : "HUMAN";
	//DEBUG_LOGF("[Heal] State=%s | Pos=(%.1f, %.1f, %.1f) | HP=%.1f/%.1f | Healed=%.2f", stateName, m_Position.x, m_Position.y, m_Position.z, health_, maxHealth_, amount);
}

void Player::SetController(Controller* controller)
{
	controller_ = controller;
}

Controller* Player::GetController() const
{
	return controller_;
}

// カメラ生成（高さは target.y を利用）
void Player::CreateCamera(const XMFLOAT3& target)
{
	float height = target.y;
	camera_.reset(new TopDownCamera(this, height, 20.0f));
}

Camera* Player::GetCamera() const
{
	return camera_.get();
}

AABB Player::GetAABB() const
{
	float halfW = aabbHalfSize.x;
	float height = aabbHalfSize.y;
	DirectX::XMFLOAT3 min{ m_Position.x - halfW, m_Position.y, m_Position.z - halfW };
	DirectX::XMFLOAT3 max{ m_Position.x + halfW, m_Position.y + height, m_Position.z + halfW };
	return AABB(min, max);
}

// 電柱周辺検出メソッド
bool Player::IsNearPole() const
{
	extern ObjectManager g_ObjectManager;
	const auto& allObjects = g_ObjectManager.GetGameObjects();
	
	for (const auto& obj : allObjects) {
		if (obj->GetTag() == GameObjectTag::POLE) {
			Pole* pole = static_cast<Pole*>(obj.get());
			if (!pole) continue;
			
			DirectX::XMFLOAT3 polePos = pole->GetPosition();
			float dx = polePos.x - m_Position.x;
			float dz = polePos.z - m_Position.z;
			// 水平距離のみで判定（垂直は無視）
			float horizontalDistance = sqrtf(dx*dx + dz*dz);
			
			if (horizontalDistance <= POLE_DETECTION_RADIUS) {
				return true;
			}
		}
	}
	return false;
}

// 最も近い電線にスナップするメソッド
void Player::SnapToNearestPowerLine()
{
	if (state != State::ELECTRICITY) return;

	extern ObjectManager g_ObjectManager;

	float minDistance = FLT_MAX;
	DirectX::XMFLOAT3 snappedPos = m_Position;
	bool found = false;

	// すべての電線の中から最も近いポイントを探す
	const auto& allObjects = g_ObjectManager.GetGameObjects();
	for (const auto& obj : allObjects) {
		if (obj->GetTag() == GameObjectTag::POWER_LINE) {
			PowerLine* line = static_cast<PowerLine*>(obj.get());
			if (!line) continue;

			// 電線上の最も近いポイントを取得
			DirectX::XMFLOAT3 closestPoint = line->GetClosestPointOnLine(m_Position);
			
			float dx = closestPoint.x - m_Position.x;
			float dy = closestPoint.y - m_Position.y;
			float dz = closestPoint.z - m_Position.z;
			
			// 水平距離と垂直距離を分別
			float horizontalDist = sqrtf(dx * dx + dz * dz);
			float verticalDist = fabsf(dy);
			
			// 水平距離が範囲内なら、垂直距離に関わらずスナップ対象にする
			// 水平距離: 2m以内、垂直距離: 5m以内
			if (horizontalDist <= POWER_LINE_SNAP_DISTANCE && verticalDist <= 5.0f) {
				// 3次元距離で最も近いものを選ぶ
				float distance = sqrtf(dx * dx + dy * dy + dz * dz);
				
				if (distance < minDistance) {
					minDistance = distance;
					snappedPos = closestPoint;
					found = true;
				}
			}
		}
	}

	// 最も近い電線にスナップ
	if (found) {
		m_Position = snappedPos;
		isGrounded_ = true;
		velocityY_ = 0.0f;
	}
}

// 電気状態から人間状態に変化する際に電柱から跳ね返す
void Player::KnockbackFromPole()
{
	extern ObjectManager g_ObjectManager;
	const auto& allObjects = g_ObjectManager.GetGameObjects();
	
	Pole* nearestPole = nullptr;
	float minDistance = FLT_MAX;
	
	for (const auto& obj : allObjects) {
		if (obj->GetTag() == GameObjectTag::POLE) {
			Pole* pole = static_cast<Pole*>(obj.get());
			if (!pole) continue;

			DirectX::XMFLOAT3 polePos = pole->GetPosition();
			float dx = polePos.x - m_Position.x;
			float dz = polePos.z - m_Position.z;
			float horizontalDistance = sqrtf(dx * dx + dz * dz);

			if (horizontalDistance < minDistance) {
				minDistance = horizontalDistance;
				nearestPole = pole;
			}
		}
	}

	if (!nearestPole) return;

	// 最も近い電柱からプレイヤーへの方向ベクトルを計算
	DirectX::XMFLOAT3 nearestPolePos = nearestPole->GetPosition();
	float knockbackDx = m_Position.x - nearestPolePos.x;
	float knockbackDz = m_Position.z - nearestPolePos.z;
	float knockbackDist = sqrtf(knockbackDx * knockbackDx + knockbackDz * knockbackDz);

	if (knockbackDist > 0.001f) {
		// 正規化して方向を計算
		knockbackDx /= knockbackDist;
		knockbackDz /= knockbackDist;

		// 電柱から確定に跳ね返す（KNOCKBACK_DISTANCE = 3.0f）
		m_Position.x = nearestPolePos.x + knockbackDx * KNOCKBACK_DISTANCE;
		m_Position.z = nearestPolePos.z + knockbackDz * KNOCKBACK_DISTANCE;

		// 同時にジャンプ（水平方向の移動なし）
		velocityY_ = KNOCKBACK_JUMP_FORCE;
		isGrounded_ = false;
	}
}

// 電気状態への変化時に各種状態をリセット
void Player::ResetToElectricityState()
{
	// 速度をリセット
	velocityY_ = 0.0f;
	isGrounded_ = true;
	isDashing_ = false;
	dashTimeRemaining_ = 0.0f;

	// 電気状態の速度を設定
	currentSpeed_ = baseSpeed_ * electricSpeedmul;

	// ダメージタイマーをリセット：電気状態に変わった直後はダメージを受けないようにする
	powerLineDamageTimer_ = POWERLINE_DAMAGE_INTERVAL;

	// 最寄りの電線にスナップ
	SnapToNearestPowerLine();
}

// プレイヤーステート変更メソッド
void Player::ChangeState(Player::State newState)
{
	if (state == newState) return; // 状態が変わらない場合は処理しない

	state = newState;

	// HUMAN状態に戻る場合はタイマーをリセット
	if (newState == State::HUMAN) {
		powerLineDamageTimer_ = 0.0f;
		DEBUG_LOGF("[ChangeState] HUMAN | Pos=(%.1f, %.1f, %.1f) | HP=%.1f/%.1f", 
			m_Position.x, m_Position.y, m_Position.z, health_, maxHealth_);
	} else if (newState == State::ELECTRICITY) {
		DEBUG_LOGF("[ChangeState] ELECTRICITY | Pos=(%.1f, %.1f, %.1f) | HP=%.1f/%.1f", 
			m_Position.x, m_Position.y, m_Position.z, health_, maxHealth_);
	}
}

// ハウスへの電気供給（ボタン操作で呼ばれる）
// プレイヤーが近くにいるハウスに電気を供給する関数
void Player::TransferElectricityToHouse(House* house, double elapsedSec)
{
	if (!house || health_ <= 0) return;

	// 毎秒の固定供給量（ELECTRICITY_TRANSFER_RATE で調整可能）
	float transferAmount = ELECTRICITY_TRANSFER_RATE * static_cast<float>(elapsedSec);
	
	// プレイヤーが持っている体力の方が少ない場合はそれを上限にする
	if (health_ < transferAmount) {
		transferAmount = static_cast<float>(health_);
	}

	

	// ハウスに電気を供給（体力をそのまま電気に変換）
	house->ReceiveElectricity(transferAmount);
	
	// プレイヤーの体力から差し引く（電気を消費）
// 体力を減少させて電気を供給
	TakeDamage(transferAmount);
	
}

// 最も近いハウスを取得
class House* Player::GetNearestHouse() const
{
	extern ObjectManager g_ObjectManager;
	const auto& allObjects = g_ObjectManager.GetGameObjects();
	
	House* nearestHouse = nullptr;
	float minDistance = FLT_MAX;
	
	for (const auto& obj : allObjects) {
		if (obj->GetTag() == GameObjectTag::HOUSE) {
			House* house = static_cast<House*>(obj.get());
			if (!house) continue;
			
			float distance = house->GetDistanceToPlayer(m_Position);
			
			if (distance < minDistance && distance <= HOUSE_INTERACTION_RADIUS) {
				minDistance = distance;
				nearestHouse = house;
			}
		}
	}
	return nearestHouse;
}

// 給電開始
void Player::StartSupplyingElectricity(House* house)
{
	if (!house || m_isSupplying) return;
	m_supplyingHouse = house;
	m_isSupplying = true;

	const char* stateName = (state == State::ELECTRICITY) ? "ELECTRICITY" : "HUMAN";
	DEBUG_LOGF("[StartSupply] State=%s | Pos=(%.1f, %.1f, %.1f) | HP=%.1f/%.1f | House=(%.1f, %.1f, %.1f)", 
		stateName, m_Position.x, m_Position.y, m_Position.z, health_, maxHealth_,
		house->GetPosition().x, house->GetPosition().y, house->GetPosition().z);
}

// 給電停止
void Player::StopSupplyingElectricity()
{
	m_isSupplying = false;
	m_supplyingHouse = nullptr;

	const char* stateName = (state == State::ELECTRICITY) ? "ELECTRICITY" : "HUMAN";
	DEBUG_LOGF("[StopSupply] State=%s | Pos=(%.1f, %.1f, %.1f) | HP=%.1f/%.1f", 
		stateName, m_Position.x, m_Position.y, m_Position.z, health_, maxHealth_);
}
