/////////////////////////
//Enemy.h[敵クラス]
//Author : Hiroshi.Kasiwagi
//Date : 2025/12/04
/////////////////////////
#ifndef ENEMY_H
#define ENEMY_H

#include "GameObject.h" 
#include "collision.h"
#include <DirectXMath.h>
#include <vector>

class Player; // forward declaration

class Enemy : public GameObject
{
public:
	// 敵の状態
	enum class State
	{
		PATROL,      // 徘徊
		CHASE,       // 追跡
		DEAD         // 死亡
	};

	Enemy();
	explicit Enemy(const DirectX::XMFLOAT3& pos);
	Enemy(const DirectX::XMFLOAT3& pos, MODEL* model, float maxHealth = 100.0f);
	~Enemy();

	// 毎フレーム更新
	void Update(double elapsedSec) override;

	// 描画
	void Draw() const override;

	// ゲッター
	int GetHealth() const { return health; }
	float GetSpeed() const { return speed; }
	const DirectX::XMFLOAT3& GetPosition() const { return position; }
	const DirectX::XMFLOAT3& GetDirection() const { return direction; }
	State GetState() const { return currentState; }
	bool IsAlive() const { return health > 0 && currentState != State::DEAD; }

	// 当たり判定関連
	AABB GetAABB() const;
	AABB GetAABBAt(const DirectX::XMFLOAT3& pos) const;
	bool IsCollidingWith(const AABB& other) const { return GetAABB().IsOverlap(other); }

	// セッター
	void SetHealth(int newHealth);
	void SetSpeed(float newSpeed) { speed = newSpeed; }
	void SetPosition(const DirectX::XMFLOAT3& newPos) { position = newPos; }
	void TakeDamage(int damage);
	void Heal(int amount);

	// プレイヤー設定
	void SetTargetPlayer(Player* player) { targetPlayer = player; }
	void SetPlayerList(const std::vector<Player*>& players) { targetPlayers = players; }

private:
	int health;                           // 敵の体力
	int maxHealth;                        // 最大体力
	float speed;                          // 敵の移動速度
	DirectX::XMFLOAT3 position;           // 敵の位置
	DirectX::XMFLOAT3 direction;          // 敵の向き
	MODEL* model;                         // 敵のモデル

	// AI 関連
	State currentState;                   // 現在の状態
	Player* targetPlayer;                 // ターゲットプレイヤー（後方互換性）
	std::vector<Player*> targetPlayers;   // 複数プレイヤーのリスト

	// 徘徊用パラメータ
	float patrolTimer;                    // 徘徊のタイマー
	DirectX::XMFLOAT3 patrolTarget;       // 徘徊の目標位置
	static constexpr float PATROL_UPDATE_TIME = 3.0f;  // 徘徊目標更新間隔（秒）
	static constexpr float PATROL_SPEED = 2.0f;        // 徘徊時の速度
	static constexpr float MAP_BOUNDARY = 30.0f;       // マップの境界

	// 追跡用パラメータ
	static constexpr float DETECTION_RANGE = 15.0f;    // プレイヤー検知範囲（メートル）
	static constexpr float CHASE_SPEED = 4.0f;         // 追跡時の速度
	static constexpr float LOSE_TARGET_RANGE = 20.0f;  // ターゲット喪失範囲

	// 衝突判定用AABB
	static constexpr float AABB_WIDTH = 0.8f;
	static constexpr float AABB_HEIGHT = 1.8f;
	static constexpr float AABB_DEPTH = 0.8f;

	// プライベート関数
	void UpdatePatrol(double elapsedSec);
	void UpdateChase(double elapsedSec);
	void CheckPlayerDetection();
	DirectX::XMFLOAT3 GenerateRandomPatrolTarget();
	DirectX::XMFLOAT3 GetRandomDirection();
	float GetDistanceToPlayer() const;
	void MoveToward(const DirectX::XMFLOAT3& target, float moveSpeed, double elapsedSec);
};

#endif // !ENEMY_H