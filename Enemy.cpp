/////////////////////////
//Enemy.cpp[敵クラス実装]
//Author : Hiroshi.Kasiwagi
//Date : 2025/12/04
/////////////////////////
#include "Enemy.h"
#include "player.h"
#include "model.h"
#include <DirectXMath.h>
#include <cstdlib>
#include <cmath>

using namespace DirectX;

// ===========================
// コンストラクタ・デストラクタ
// ===========================

Enemy::Enemy()
	: GameObject(DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f)),
	health(100),
	maxHealth(100),
	speed(PATROL_SPEED),
	position(0.0f, 0.0f, 0.0f),
	direction(0.0f, 0.0f, 1.0f),
	model(nullptr),
	currentState(State::PATROL),
	targetPlayer(nullptr),
	patrolTimer(0.0f),
	patrolTarget(0.0f, 0.0f, 0.0f)
{
}

Enemy::Enemy(const DirectX::XMFLOAT3& pos)
	: GameObject(pos),
	health(100),
	maxHealth(100),
	speed(PATROL_SPEED),
	position(pos),
	direction(0.0f, 0.0f, 1.0f),
	model(nullptr),
	currentState(State::PATROL),
	targetPlayer(nullptr),
	patrolTimer(0.0f),
	patrolTarget(GenerateRandomPatrolTarget())
{
}

Enemy::Enemy(const DirectX::XMFLOAT3& pos, MODEL* modelPtr, float maxHealthVal)
	: GameObject(pos, modelPtr),
	health(static_cast<int>(maxHealthVal)),
	maxHealth(static_cast<int>(maxHealthVal)),
	speed(PATROL_SPEED),
	position(pos),
	direction(0.0f, 0.0f, 1.0f),
	model(modelPtr),
	currentState(State::PATROL),
	targetPlayer(nullptr),
	patrolTimer(0.0f),
	patrolTarget(GenerateRandomPatrolTarget())
{
	m_pModel = modelPtr;
}

Enemy::~Enemy()
{
	// model は外部で管理されるため、ここでは削除しない
}

// ===========================
// 更新処理
// ===========================

void Enemy::Update(double elapsedSec)
{
	if (!IsAlive()) return;

	// プレイヤー検知判定
	CheckPlayerDetection();

	// 状態に応じた更新
	switch (currentState)
	{
	case State::PATROL:
		UpdatePatrol(elapsedSec);
		break;
	case State::CHASE:
		UpdateChase(elapsedSec);
		break;
	case State::DEAD:
		// 死亡状態では更新なし
		break;
	}

	// ワールド座標を更新
	m_Position = position;
}

// ===========================
// 徘徊処理
// ===========================

void Enemy::UpdatePatrol(double elapsedSec)
{
	patrolTimer += static_cast<float>(elapsedSec);

	// 新しい徘徊目標を生成するタイミング
	if (patrolTimer >= PATROL_UPDATE_TIME)
	{
		patrolTimer = 0.0f;
		patrolTarget = GenerateRandomPatrolTarget();
	}

	// 徘徊目標に向かって移動
	MoveToward(patrolTarget, PATROL_SPEED, elapsedSec);
}

// ===========================
// 追跡処理
// ===========================

void Enemy::UpdateChase(double elapsedSec)
{
	if (!targetPlayer)
	{
		currentState = State::PATROL;
		return;
	}

	// プレイヤーまでの距離をチェック
	float distToPlayer = GetDistanceToPlayer();

	// プレイヤーを喪失した場合
	if (distToPlayer > LOSE_TARGET_RANGE)
	{
		currentState = State::PATROL;
		patrolTimer = 0.0f;
		patrolTarget = GenerateRandomPatrolTarget();
		return;
	}

	// プレイヤーの位置に向かって移動
	XMFLOAT3 playerPos = targetPlayer->GetPosition();
	MoveToward(playerPos, CHASE_SPEED, elapsedSec);
}

// ===========================
// プレイヤー検知判定
// ===========================

void Enemy::CheckPlayerDetection()
{
	// 複数プレイヤーが設定されている場合は、最も近いプレイヤーを自動選択
	if (!targetPlayers.empty())
	{
		Player* closestPlayer = nullptr;
		float minDistance = FLT_MAX;

		// すべてのプレイヤーをチェックして最も近いものを見つける
		for (Player* player : targetPlayers)
		{
			if (!player || !player->IsAlive()) continue;

			XMFLOAT3 playerPos = player->GetPosition();
			XMVECTOR posVec = XMLoadFloat3(&position);
			XMVECTOR playerVec = XMLoadFloat3(&playerPos);
			XMVECTOR diff = playerVec - posVec;
			float distance = XMVectorGetX(XMVector3Length(diff));

			if (distance < minDistance)
			{
				minDistance = distance;
				closestPlayer = player;
			}
		}

		// 最も近いプレイヤーをターゲットに設定
		targetPlayer = closestPlayer;
	}

	if (!targetPlayer || !targetPlayer->IsAlive())
	{
		if (currentState == State::CHASE)
		{
			currentState = State::PATROL;
		}
		return;
	}

	float distToPlayer = GetDistanceToPlayer();

	// プレイヤーが検知範囲内に入った
	if (distToPlayer < DETECTION_RANGE && currentState != State::CHASE)
	{
		currentState = State::CHASE;
	}
	// プレイヤーが検知範囲外に出た
	else if (distToPlayer > LOSE_TARGET_RANGE && currentState == State::CHASE)
	{
		currentState = State::PATROL;
		patrolTimer = 0.0f;
		patrolTarget = GenerateRandomPatrolTarget();
	}
}

// ===========================
// AI ヘルパー関数
// ===========================

DirectX::XMFLOAT3 Enemy::GenerateRandomPatrolTarget()
{
	// マップの境界内でランダムな位置を生成
	float randomX = static_cast<float>((rand() % 100) - 50) * 0.6f; // -30～30 の範囲
	float randomZ = static_cast<float>((rand() % 100) - 50) * 0.6f;

	// 境界内に収める
	if (randomX > MAP_BOUNDARY) randomX = MAP_BOUNDARY;
	if (randomX < -MAP_BOUNDARY) randomX = -MAP_BOUNDARY;
	if (randomZ > MAP_BOUNDARY) randomZ = MAP_BOUNDARY;
	if (randomZ < -MAP_BOUNDARY) randomZ = -MAP_BOUNDARY;

	return XMFLOAT3(randomX, position.y, randomZ);
}

DirectX::XMFLOAT3 Enemy::GetRandomDirection()
{
	float angle = static_cast<float>(rand() % 360) * (3.14159f / 180.0f);
	return XMFLOAT3(std::sin(angle), 0.0f, std::cos(angle));
}

float Enemy::GetDistanceToPlayer() const
{
	if (!targetPlayer) return FLT_MAX;

	XMFLOAT3 playerPos = targetPlayer->GetPosition();
	XMVECTOR pos1 = XMLoadFloat3(&position);
	XMVECTOR pos2 = XMLoadFloat3(&playerPos);
	XMVECTOR diff = pos2 - pos1;

	return XMVectorGetX(XMVector3Length(diff));
}

void Enemy::MoveToward(const DirectX::XMFLOAT3& target, float moveSpeed, double elapsedSec)
{
	// 目標方向を計算
	XMVECTOR posVec = XMLoadFloat3(&position);
	XMVECTOR targetVec = XMLoadFloat3(&target);
	XMVECTOR dirVec = targetVec - posVec;
	XMVECTOR length = XMVector3Length(dirVec);

	float dist = XMVectorGetX(length);

	// 目標に到着している場合はスキップ
	if (dist < 0.5f)
	{
		return;
	}

	// 方向を正規化
	XMVECTOR normalizedDir = XMVector3Normalize(dirVec);
	XMStoreFloat3(&direction, normalizedDir);

	// 移動距離を計算
	float moveDistance = moveSpeed * static_cast<float>(elapsedSec);

	// 新しい位置を計算
	XMVECTOR newPos = posVec + normalizedDir * moveDistance;
	XMStoreFloat3(&position, newPos);

	// マップ境界チェック
	if (position.x > MAP_BOUNDARY) position.x = MAP_BOUNDARY;
	if (position.x < -MAP_BOUNDARY) position.x = -MAP_BOUNDARY;
	if (position.z > MAP_BOUNDARY) position.z = MAP_BOUNDARY;
	if (position.z < -MAP_BOUNDARY) position.z = -MAP_BOUNDARY;
}

// ===========================
// 描画処理
// ===========================

void Enemy::Draw() const
{
	if (!model) return;

	XMMATRIX world = XMMatrixTranslation(position.x, position.y, position.z);
	ModelDraw(model, world);
}

// ===========================
// 当たり判定処理
// ===========================

AABB Enemy::GetAABB() const
{
	return GetAABBAt(position);
}

AABB Enemy::GetAABBAt(const DirectX::XMFLOAT3& pos) const
{
	// AABBの半分のサイズ
	float halfWidth = AABB_WIDTH / 2.0f;
	float halfDepth = AABB_DEPTH / 2.0f;

	// Min と Max を計算
	DirectX::XMFLOAT3 min(pos.x - halfWidth, pos.y, pos.z - halfDepth);
	DirectX::XMFLOAT3 max(pos.x + halfWidth, pos.y + AABB_HEIGHT, pos.z + halfDepth);

	return AABB(min, max);
}

// ===========================
// ダメージ・回復処理
// ===========================

void Enemy::SetHealth(int newHealth)
{
	health = newHealth;
	if (health <= 0)
	{
		currentState = State::DEAD;
	}
}

void Enemy::TakeDamage(int damage)
{
	health -= damage;
	if (health <= 0)
	{
		health = 0;
		currentState = State::DEAD;
	}
}

void Enemy::Heal(int amount)
{
	health += amount;
	if (health > maxHealth)
	{
		health = maxHealth;
	}
}
