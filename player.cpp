#include "player.h"
#include <algorithm>
#include <cmath>
#include <DirectXMath.h>
#include "model.h"
#include "controller.h"
#include "top_down_camera.h"
#include "map.h"
#include "ObjectManager.h" // 追加

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
	// ダッシュ状態の更新
	if (isDashing_) {
		dashTimeRemaining_ -= static_cast<float>(elapsedSec);
		if (dashTimeRemaining_ <= 0.0f) {
			isDashing_ = false;
			currentSpeed_ = (state == State::ELECTRICITY) ? (baseSpeed_ * electricSpeedmul) : baseSpeed_;
			dashTimeRemaining_ = 0.0f;
		}
	}

	// 1. 入力に基づいて水平方向の移動ベクトルを決定
	XMFLOAT3 horizontalMove = { 0.0f, 0.0f, 0.0f };
	if (controller_) {
		// 状態切替（Bボタン押下で切替）
		if (controller_->WasPressed(Controller::BUTTON_B)) {
			if (state == State::HUMAN) state = State::ELECTRICITY; else state = State::HUMAN;
			if (!isDashing_) {
				currentSpeed_ = (state == State::ELECTRICITY) ? (baseSpeed_ * electricSpeedmul) : baseSpeed_;
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
		}
		
	}

	// 2. 重力を適用（毎フレーム速度を加算）
	if (!isGrounded_) {
		velocityY_ -= GRAVITY * static_cast<float>(elapsedSec);
	}

	// 3. 移動と衝突解決
	XMFLOAT3 desiredMove = {
		horizontalMove.x * currentSpeed_ * static_cast<float>(elapsedSec),
		velocityY_ * static_cast<float>(elapsedSec),
		horizontalMove.z * currentSpeed_ * static_cast<float>(elapsedSec)
	};
	ResolveCollisions(desiredMove, elapsedSec);

	// 4. 向きを更新
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

// Player::Move は現在使われていないため、空にするか削除
void Player::Move(const XMFLOAT3& dir, double elapsedSec)
{
	// このロジックは Player::Update と ResolveCollisions に統合されました
}
