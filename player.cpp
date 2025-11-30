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

// 入力反転フラグ（必要に応じて調整）
static constexpr bool INVERT_LS_X = true; // 左右が反転しているので X を反転
static constexpr bool INVERT_LS_Y = true; // 前後が反転しているので Y を反転

using namespace DirectX;

// プレイヤークラス実装
Player::Player()
{
}

Player::~Player() = default;

Player::Player(const XMFLOAT3& pos)
	: position_(pos)
{
}


Player::Player(MODEL* model, MODEL* electricModel, const XMFLOAT3& pos, const XMFLOAT3& dir)
	: model_(model), electricModel_(electricModel), position_(pos), direction_(dir)
{
	//体力
	health_ = maxHealth_ = 100;	
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
		//リスポーン
		if (usePlayer == false)
		{
			health_ = maxHealth_;
			position_ = XMFLOAT3(0.0f, 15.0f, 0.0f);
			usePlayer = true;
			
			// 状態をHUMANにリセット
			ChangeState(State::HUMAN);
			
			// 移動速度をリセット
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

	// 3. 移動と衝突処理
	XMFLOAT3 desiredMove = {
		horizontalMove.x * currentSpeed_ * static_cast<float>(elapsedSec),
		velocityY_ * static_cast<float>(elapsedSec),
		horizontalMove.z * currentSpeed_ * static_cast<float>(elapsedSec)
	};
	
	// 電気状態の場合は衝突判定をスキップして直接移動
	if (state == State::ELECTRICITY) {
		position_.x += desiredMove.x;
		position_.z += desiredMove.z;
		velocityY_ = 0.0f;
		isGrounded_ = true;
	} else if (skipCollisionTimer_ > 0.0f) {
		// 状態変更直後はスキップタイマーがある間、衝突判定をスキップ
		position_.x += desiredMove.x;
		position_.z += desiredMove.z;
		position_.y += desiredMove.y;
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
    // XZ平面（水平）の移動と衝突
    XMFLOAT3 newPos = position_;
    newPos.x += desiredMove.x;
    newPos.z += desiredMove.z;

    AABB playerAABB = GetAABBAt(newPos);
    bool collisionXZ = false;

    extern ObjectManager g_ObjectManager;
    for (const auto& obj : g_ObjectManager.GetGameObjects()) {
        if (playerAABB.IsOverlap(obj->GetAABB())) {
            collisionXZ = true;
            break;
        }
    }

    if (!collisionXZ) {
        position_ = newPos; // 水平移動を適用
    }

    // Y軸（垂直）の移動と衝突
    position_.y += desiredMove.y;
    playerAABB = GetAABB(); // 更新されたXZ位置でAABBを再計算
    bool collisionY = false;

    for (const auto& obj : g_ObjectManager.GetGameObjects()) {
        if (playerAABB.IsOverlap(obj->GetAABB())) {
            // オブジェクトの上にいるか、下から突き上げたか
            if (desiredMove.y < 0.0f) { // 落下中
                position_.y = obj->GetAABB().GetMax().y; // オブジェクトの天面にスナップ
                velocityY_ = 0.0f;
                isGrounded_ = true;
            } else if (desiredMove.y > 0.0f) { // 上昇中
                position_.y = obj->GetAABB().GetMin().y - aabbHalfSize.y * 2.0f; // オブジェクトの底面にスナップ
                velocityY_ = 0.0f;
            }
            collisionY = true;
            break;
        }
    }

    // 地面との最終チェック
    if (position_.y <= GROUND_LEVEL) {
        position_.y = GROUND_LEVEL;
        velocityY_ = 0.0f;
        isGrounded_ = true;
    } else if (!collisionY) {
        isGrounded_ = false;
    }
}

// 指定した位置でのAABBを計算するヘルパー
AABB Player::GetAABBAt(const DirectX::XMFLOAT3& pos) const
{
    float halfW = aabbHalfSize.x;
    float height = aabbHalfSize.y * 2.0f; // AABBの高さは全高
    DirectX::XMFLOAT3 min{ pos.x - halfW, pos.y, pos.z - halfW };
    DirectX::XMFLOAT3 max{ pos.x + halfW, pos.y + height, pos.z + halfW };
    return AABB(min, max);
}

// モデル描画（状態に応じてモデル切替）
void Player::Draw()
{
	MODEL* drawModel = (state == State::ELECTRICITY && electricModel_) ? electricModel_ : model_;
	if (!drawModel) return;
	// 回転はY軸のみを考慮して簡易的に作成
	float yaw = atan2f(direction_.x, direction_.z);
	XMMATRIX matRot = XMMatrixRotationY(yaw);
	XMMATRIX matTrans = XMMatrixTranslation(position_.x, position_.y, position_.z);
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
	// 地面に接している場合のみジャンプ可能
	if (isGrounded_) {
		velocityY_ = jumpForce;
		isGrounded_ = false;
	}
}

void Player::TakeDamage(int amount)
{
	health_ -= amount;
	if (health_ < 0) health_ = 0;
}

void Player::Heal(int amount)
{
	health_ += amount;
	if (health_ > maxHealth_) health_ = maxHealth_;
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
	DirectX::XMFLOAT3 min{ position_.x - halfW, position_.y, position_.z - halfW };
	DirectX::XMFLOAT3 max{ position_.x + halfW, position_.y + height, position_.z + halfW };
	return AABB(min, max);
}

// 電柱周辺検出メソッド
bool Player::IsNearPole() const
{
	extern ObjectManager g_ObjectManager;
	auto poles = g_ObjectManager.GetAllPoles();
	
	for (const auto& pole : poles) {
		if (!pole) continue;
		
		DirectX::XMFLOAT3 polePos = pole->GetPosition();
		float dx = polePos.x - position_.x;
		float dz = polePos.z - position_.z;
		// 水平距離のみで判定（高さは無視）
		float horizontalDistance = sqrtf(dx*dx + dz*dz);
		
		if (horizontalDistance <= POLE_DETECTION_RADIUS) {
			return true;
		}
	}
	return false;
}

// 最も近い電線にスナップするメソッド
void Player::SnapToNearestPowerLine()
{
	if (state != State::ELECTRICITY) return;

	extern ObjectManager g_ObjectManager;
	auto powerLines = g_ObjectManager.GetAllPowerLines();
	
	if (powerLines.empty()) return;

	float minDistance = FLT_MAX;
	DirectX::XMFLOAT3 snappedPos = position_;
	bool found = false;

	// すべての電線の中から最も近いポイントを探す
	for (const auto& line : powerLines) {
		if (!line) continue;

		// 電線上の最も近いポイントを取得
		DirectX::XMFLOAT3 closestPoint = line->GetClosestPointOnLine(position_);
		
		float dx = closestPoint.x - position_.x;
		float dy = closestPoint.y - position_.y;
		float dz = closestPoint.z - position_.z;
		
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

	// 最も近い電線にスナップ
	if (found) {
		position_ = snappedPos;
		isGrounded_ = true;
		velocityY_ = 0.0f;
	}
}

// 電気状態から人間に変化する際に電柱から跳ね返す
void Player::KnockbackFromPole()
{
	extern ObjectManager g_ObjectManager;
	auto poles = g_ObjectManager.GetAllPoles();
	
	if (poles.empty()) return;

	// 最も近い電柱を探す
	float minDistance = FLT_MAX;
	DirectX::XMFLOAT3 nearestPolePos = position_;

	for (const auto& pole : poles) {
		if (!pole) continue;

		DirectX::XMFLOAT3 polePos = pole->GetPosition();
		float dx = polePos.x - position_.x;
		float dz = polePos.z - position_.z;
		float horizontalDistance = sqrtf(dx * dx + dz * dz);

		if (horizontalDistance < minDistance) {
			minDistance = horizontalDistance;
			nearestPolePos = polePos;
		}
	}

	// 最も近い電柱からプレイヤーに向かう方向を計算
	float knockbackDx = position_.x - nearestPolePos.x;
	float knockbackDz = position_.z - nearestPolePos.z;
	float knockbackDist = sqrtf(knockbackDx * knockbackDx + knockbackDz * knockbackDz);

	if (knockbackDist > 0.001f) {
		// 正規化して方向を計算
		knockbackDx /= knockbackDist;
		knockbackDz /= knockbackDist;

		// 電柱から確実に離す（KNOCKBACK_DISTANCE = 3.0f）
		position_.x = nearestPolePos.x + knockbackDx * KNOCKBACK_DISTANCE;
		position_.z = nearestPolePos.z + knockbackDz * KNOCKBACK_DISTANCE;

		// 真上にジャンプ（水平方向の移動なし）
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
	}
}

// ハウスへの電気供給（ボタン操作で呼ばれる）
void Player::TransferElectricityToHouse(House* house, double elapsedSec)
{
	if (!house || health_ <= 0) return;

	// 供給量を計算（1秒あたりのレートから経過時間分を計算）
	float transferAmount = ELECTRICITY_TRANSFER_RATE * static_cast<float>(elapsedSec);
	
	// プレイヤーが持っている電気量（体力）の方が少ない場合はそれを上限にする
	if (health_ < transferAmount) {
		transferAmount = static_cast<float>(health_);
	}

	// ハウスに電気を供給
	house->ReceiveElectricity(transferAmount);
	
	// プレイヤーの体力から差し引く（電気を消費）
	TakeDamage(static_cast<int>(transferAmount));
}
