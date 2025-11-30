/////////////////////////////
// player.h[プレイヤークラス]
//Author : hiroshi kasiwagi
//Date :2025/11/27
////////////////////////////////
#ifndef PLAYER_H
#define PLAYER_H

#include <DirectXMath.h>
#include <array>
#include <memory>
#include "controller.h"
#include "top_down_camera.h"
#include "collision.h"

struct MODEL; // forward declaration for model pointer
class Controller; // forward declaration
class TopDownCamera; // forward declaration
class Camera; // forward declaration for getter

class Player
{
private:
    // モデル（オプション、外部で管理されるポインタ）
    struct MODEL* model_ = nullptr;
    //電気状態用モデル
    struct MODEL* electricModel_ = nullptr;

    // コントローラ（非所有、外部で作成して渡す）
    Controller* controller_ = nullptr;

    // カメラ（プレイヤーが管理・所有）
    std::unique_ptr<TopDownCamera> camera_;

    //位置と向き
    DirectX::XMFLOAT3 position_{};
    DirectX::XMFLOAT3 direction_{ 0.0f,0.0f,1.0f };

    //体力
    int health_ = 100;
    int maxHealth_ = 100;

    //死亡判定
	bool usePlayer = true;

    // 移動速度
    float baseSpeed_ = 3.0f; // 通常移動速度 (units/sec)
    float dashMultiplier_ = 2.0f; // ダッシュ時の倍率
    float currentSpeed_ = 3.0f;
	float electricSpeedmul = 3.0f; //電気状態の速度倍率

    // ダッシュ管理
    bool isDashing_ = false;
    float dashDuration_ = 0.6f; // ダッシュ継続時間（秒）
    float dashTimeRemaining_ = 0.0f;

    //プレイヤーのステート
    enum class State {
        HUMAN,
        ELECTRICITY,
    } state = State::HUMAN;

    // 衝突判定用AABBのサイズ（ローカル空間）
    DirectX::XMFLOAT3 aabbHalfSize{ 0.5f, 1.0f, 0.5f }; // 幅1m、高さ2m、奥行1m

	// 重力関連
	float velocityY_ = 0.0f; // Y軸速度（鉛直方向）
	bool isGrounded_ = false; // 地面に接しているかどうか
	static constexpr float GRAVITY = 9.8f; // 重力加速度 (units/sec^2)
	static constexpr float GROUND_LEVEL = 0.0f; // 地面の高さ

public:
 Player();
 ~Player();
 explicit Player(const DirectX::XMFLOAT3& pos);

 Player(MODEL* model, MODEL* electricModel, const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3& dir);

 // 毎フレーム更新（経過時間は秒）
 void Update(double elapsedSec);

 // 移動：方向は正規化されていることを想定
 void Move(const DirectX::XMFLOAT3& dir, double elapsedSec);

 // 描画
 void Draw();

 // ダッシュ開始/終了
 void StartDash();
 void StopDash();

 // ダメージ/回復
 void TakeDamage(int amount);
 void Heal(int amount);

 // コントローラ/カメラ管理
 void SetController(Controller* controller);
 Controller* GetController() const;

 // カメラを生成（target の y 成分を高さと解釈）
 void CreateCamera(const DirectX::XMFLOAT3& target);
 Camera* GetCamera() const; // return base Camera pointer for external use

 //位置取得（カメラ等が参照するためのアクセサ）
 const DirectX::XMFLOAT3& GetPosition() const { return position_; }

 //体力取得
 int GetHealth() const { return health_; }
 int GetMaxHealth() const { return maxHealth_; }

 //方向/速度設定
 void SetDirection(const DirectX::XMFLOAT3& dir) { direction_ = dir; }
 DirectX::XMFLOAT3 GetDirection() const { return direction_; }
 void SetBaseSpeed(float speed) { baseSpeed_ = speed; if (!isDashing_) currentSpeed_ = speed; }

 // AABB チェック用取得
 AABB GetAABB() const;

 //死亡判定
 bool IsAlive() const { return health_ > 0 && usePlayer; }

 // プレイヤーステート変更
 void ChangeState(Player::State newState) { state = newState; }

 Player::State GetState() const { return state; }

void SetElectricModel(MODEL* electricModel) { electricModel_ = electricModel; }

// 衝突判定
bool IsCollidingWith(const AABB& other) const { return GetAABB().IsOverlap(other); }

// ジャンプ処理
void Jump(float jumpForce = 5.0f);
bool IsGrounded() const { return isGrounded_; }

private:
    // 衝突解決ヘルパー
    void ResolveCollisions(DirectX::XMFLOAT3& newPos, double elapsedSec);
	//AABB取得ヘルパー
	AABB GetAABBAt(const DirectX::XMFLOAT3& pos) const;


};
#endif // !PLAYER_H