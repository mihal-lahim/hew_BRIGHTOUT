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
#include "GameObject.h"
#include "controller.h"
#include "top_down_camera.h"
#include "collision.h"

struct MODEL; // forward declaration for model pointer
class Controller; // forward declaration
class TopDownCamera; // forward declaration
class Camera; // forward declaration for getter
class House; // forward declaration for house pointer

class Player : public GameObject
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
    DirectX::XMFLOAT3 direction_{ 0.0f,0.0f,1.0f };

    //体力
    float health_ = 100.0f;
    float maxHealth_ = 100.0f;

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
    DirectX::XMFLOAT3 aabbHalfSize{ 0.5f, 1.5f, 0.5f }; // 幅1m、高さ2m、奥行1m

	// 重力関連
	float velocityY_ = 0.0f; // Y軸速度（鉛直方向）
	bool isGrounded_ = false; // 地面に接しているかどうか
	static constexpr float GRAVITY = 9.8f; // 重力加速度 (units/sec^2)
	static constexpr float GROUND_LEVEL = 0.0f; // 地面の高さ

	// 衝突判定スキップ時間管理
	float skipCollisionTimer_ = 0.0f; // 衝突判定スキップ時間
	static constexpr float SKIP_COLLISION_DURATION = 0.2f; // 衝突判定をスキップする時間（秒）

	// 電線ダメージ管理
	float powerLineDamageTimer_ = 0.0f; // 電線ダメージを与えるまでの時間
	static constexpr float POWERLINE_DAMAGE_INTERVAL = 1.0f; // ダメージ間隔（秒）
	static constexpr float POWERLINE_DAMAGE_AMOUNT = 5.0f; // 1回のダメージ量

	// ハウスへの電気供給管理
	static constexpr float HOUSE_INTERACTION_RADIUS = 5.0f;  // ハウス相互作用距離
	static constexpr float ELECTRICITY_PER_HP = 1.0f;  // 体力1あたりの電気量（1:1の比率）
	static constexpr float ELECTRICITY_TRANSFER_RATE = 10.0f;  // 毎秒の固定供給量

	// 電気供給状態
	class House* m_supplyingHouse = nullptr;  // 現在供給中のハウス
	bool m_isSupplying = false;                // 供給中フラグ

public:
 Player();
 ~Player();
 explicit Player(const DirectX::XMFLOAT3& pos);

 Player(MODEL* model, MODEL* electricModel, const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3& dir);

 // 毎フレーム更新（経過時間は秒）
 void Update(double elapsedSec) override;

 // 移動：方向は正規化されていることを想定
 void Move(const DirectX::XMFLOAT3& dir, double elapsedSec);

 // ダッシュ開始/終了
 void StartDash();
 void StopDash();
 
 // ダメージ/回復
 void TakeDamage(float amount);
 void Heal(float amount);

 // コントローラ/カメラ管理
 void SetController(Controller* controller);
 Controller* GetController() const;

 // カメラを生成（target の y 成分を高さと解釈）
 void CreateCamera(const DirectX::XMFLOAT3& target);
 Camera* GetCamera() const; // return base Camera pointer for external use

 //位置取得（カメラ等が参照するためのアクセサ）
 const DirectX::XMFLOAT3& GetPosition() const { return m_Position; }

 //体力取得
 float GetHealth() const { return health_; }
 float GetMaxHealth() const { return maxHealth_; }

 //方向/速度設定
 void SetDirection(const DirectX::XMFLOAT3& dir) { direction_ = dir; }
 DirectX::XMFLOAT3 GetDirection() const { return direction_; }
 void SetBaseSpeed(float speed) { baseSpeed_ = speed; if (!isDashing_) currentSpeed_ = speed; }

 // AABB チェック用取得
 AABB GetAABB() const;

 //死亡判定
 bool IsAlive() const { return health_ > 0 && usePlayer; }

 // プレイヤーステート変更
 void ChangeState(Player::State newState);

 Player::State GetState() const { return state; }

 void SetElectricModel(MODEL* electricModel) { electricModel_ = electricModel; }

 // 衝突判定
 bool IsCollidingWith(const AABB& other) const { return GetAABB().IsOverlap(other); }

 // ジャンプ処理
 void Jump(float jumpForce = 5.0f);
 bool IsGrounded() const { return isGrounded_; }

 // 電柱周辺検出（電気状態への変身可能判定）
 bool IsNearPole() const;
 
 // 電線への自動スナップ（電気状態で移動）
 void SnapToNearestPowerLine();
 
 // 電気状態から人間に変化する際、電柱から跳ね返す
 void KnockbackFromPole();

 // ハウスへの電気供給
 void TransferElectricityToHouse(class House* house, double elapsedSec);

 // 最も近いハウスを取得
 class House* GetNearestHouse() const;

 // 供給中かどうかを判定
 bool IsSupplyingElectricity() const { return m_isSupplying; }
 
 // 供給中のハウスを取得
 class House* GetSupplyingHouse() const { return m_supplyingHouse; }

 // 供給開始/停止
 void StartSupplyingElectricity(class House* house);
 void StopSupplyingElectricity();

 // 描画処理
 void Draw() const;

private:
    // 衝突解決ヘルパー
    void ResolveCollisions(DirectX::XMFLOAT3& newPos, double elapsedSec);
    //AABB取得ヘルパー
    AABB GetAABBAt(const DirectX::XMFLOAT3& pos) const;

    // 電気状態変化時のリセット処理
    void ResetToElectricityState();

    // 電柱检测用の定数
    static constexpr float POLE_DETECTION_RADIUS = 3.0f;  
    static constexpr float POWER_LINE_SNAP_DISTANCE = 2.0f;
    
    // 電気状態から変化時の跳ね返り定数
    static constexpr float KNOCKBACK_DISTANCE = 3.0f;      // 電柱から跳ね返す距離（メートル）
    static constexpr float KNOCKBACK_JUMP_FORCE = 8.0f;    // 跳ね返し時の上昇初速度
    static constexpr float KNOCKBACK_HORIZONTAL_SPEED = 5.0f; // 水平方向の跳ね返し速度

};
#endif // !PLAYER_H#endif // !PLAYER_H#endif // !PLAYER_H#endif // !PLAYER_H#ifndef PLAYER_H