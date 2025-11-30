#include "../include/player.h"
#include <algorithm>
#include <cmath>
#include <DirectXMath.h>
#include "../include/model.h"
#include "../include/controller.h"
#include "../include/top_down_camera.h"
#include "../include/map.h"
#include "../include/ObjectManager.h" // �ǉ�
#include "../include/Pole.h"
#include "../include/PowerLine.h"

// ���͔��]�t���O�i�K�v�ɉ����Ē����j
static constexpr bool INVERT_LS_X = true; // ���E�����]���Ă���̂� X �𔽓]
static constexpr bool INVERT_LS_Y = true; // �O�オ���]���Ă���̂� Y �𔽓]

using namespace DirectX;

// �v���C���[�N���X����
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
	//�̗�
	health_ = maxHealth_ = 100;	
	usePlayer = true;

	
}

// ���t���[���X�V�i�_�b�V���p�����Ԃ̊Ǘ��A���͏����j
void Player::Update(double elapsedSec)
{
	// �_�b�V����Ԃ̍X�V
	if (isDashing_) {
		dashTimeRemaining_ -= static_cast<float>(elapsedSec);
		if (dashTimeRemaining_ <= 0.0f) {
			isDashing_ = false;
			currentSpeed_ = (state == State::ELECTRICITY) ? (baseSpeed_ * electricSpeedmul) : baseSpeed_;
			dashTimeRemaining_ = 0.0f;
		}
	}

	// �Փ˔���X�L�b�v�^�C�}�[�̍X�V
	if (skipCollisionTimer_ > 0.0f) {
		skipCollisionTimer_ -= static_cast<float>(elapsedSec);
	}

	// 1. ���͂Ɋ�Â��Đ��������̈ړ��x�N�g��������
	XMFLOAT3 horizontalMove = { 0.0f, 0.0f, 0.0f };
	if (controller_) {
		// �d�C�{�^�� (B �{�^���ŕϐg)
		if (controller_->WasPressed(Controller::BUTTON_B)) {
			if (state == State::HUMAN) {
				// HUMAN -> ELECTRICITY: �d���߂��K�{
				if (IsNearPole()) {
					ChangeState(State::ELECTRICITY);
					// �d�C��Ԃւ̃��Z�b�g����
					ResetToElectricityState();
				}
			} else {
				// ELECTRICITY -> HUMAN: �d���߂��K�{
				if (IsNearPole()) {
					ChangeState(State::HUMAN);
					if (!isDashing_) {
						currentSpeed_ = baseSpeed_;
					}
					// �d�����璵�˕Ԃ�����
					KnockbackFromPole();
					// 0.5�b�ԁA�Փ˔�����X�L�b�v
					skipCollisionTimer_ = SKIP_COLLISION_DURATION;
				}
			}
		}

		// �X�e�B�b�N����
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

		// �_�b�V���J�n�i�d�C��Ԃ̂݁j
		if (controller_->WasPressed(Controller::BUTTON_A) && state == State::ELECTRICITY && !isDashing_) {
			StartDash();
		}
        // �f�o�b�O�p�_���[�W
		if (controller_->WasPressed(Controller::BUTTON_RIGHT_SHOULDER)) {
			TakeDamage(10);
		}
		//���S�p����
		if(!IsAlive())
		{
			usePlayer = false;
		}
		//���X�|�[��
		if (usePlayer == false)
		{
			health_ = maxHealth_;
			position_ = XMFLOAT3(0.0f, 15.0f, 0.0f);
			usePlayer = true;
			ChangeState(State::HUMAN);
		}
		
	}

	// 2. �d�͂�K�p�i���t���[�����x�����Z�j
	if (!isGrounded_) {
		velocityY_ -= GRAVITY * static_cast<float>(elapsedSec);
	}

	// 3. �ړ��ƏՓˏ���
	XMFLOAT3 desiredMove = {
		horizontalMove.x * currentSpeed_ * static_cast<float>(elapsedSec),
		velocityY_ * static_cast<float>(elapsedSec),
		horizontalMove.z * currentSpeed_ * static_cast<float>(elapsedSec)
	};
	
	// �d�C��Ԃ̏ꍇ�͏Փ˔�����X�L�b�v���Ē��ڈړ�
	if (state == State::ELECTRICITY) {
		position_.x += desiredMove.x;
		position_.z += desiredMove.z;
		velocityY_ = 0.0f;
		isGrounded_ = true;
	} else if (skipCollisionTimer_ > 0.0f) {
		// ��ԕύX����̓X�L�b�v�^�C�}�[������ԁA�Փ˔�����X�L�b�v
		position_.x += desiredMove.x;
		position_.z += desiredMove.z;
		position_.y += desiredMove.y;
	} else {
		// HUMAN��Ԃ͒ʏ�̏Փ˔����K�p
		ResolveCollisions(desiredMove, elapsedSec);
	}

	// 4. �d�C��Ԃ̏ꍇ�A�d���ɃX�i�b�v�����ė����h�~
	if (state == State::ELECTRICITY) {
		SnapToNearestPowerLine();
	}

	// 5. �����̍X�V
	if (horizontalMove.x != 0.0f || horizontalMove.z != 0.0f) {
		direction_ = horizontalMove;
	}
}

// �Փˉ���
void Player::ResolveCollisions(DirectX::XMFLOAT3& desiredMove, double elapsedSec)
{
    // XZ���ʁi�����j�̈ړ��ƏՓ�
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
        position_ = newPos; // �����ړ���K�p
    }

    // Y���i�����j�̈ړ��ƏՓ�
    position_.y += desiredMove.y;
    playerAABB = GetAABB(); // �X�V���ꂽXZ�ʒu��AABB���Čv�Z
    bool collisionY = false;

    for (const auto& obj : g_ObjectManager.GetGameObjects()) {
        if (playerAABB.IsOverlap(obj->GetAABB())) {
            // �I�u�W�F�N�g�̏�ɂ��邩�A������˂��グ����
            if (desiredMove.y < 0.0f) { // ������
                position_.y = obj->GetAABB().GetMax().y; // �I�u�W�F�N�g�̓V�ʂɃX�i�b�v
                velocityY_ = 0.0f;
                isGrounded_ = true;
            } else if (desiredMove.y > 0.0f) { // �㏸��
                position_.y = obj->GetAABB().GetMin().y - aabbHalfSize.y * 2.0f; // �I�u�W�F�N�g�̒�ʂɃX�i�b�v
                velocityY_ = 0.0f;
            }
            collisionY = true;
            break;
        }
    }

    // �n�ʂƂ̍ŏI�`�F�b�N
    if (position_.y <= GROUND_LEVEL) {
        position_.y = GROUND_LEVEL;
        velocityY_ = 0.0f;
        isGrounded_ = true;
    } else if (!collisionY) {
        isGrounded_ = false;
    }
}

// �w�肵���ʒu�ł�AABB���v�Z����w���p�[
AABB Player::GetAABBAt(const DirectX::XMFLOAT3& pos) const
{
    float halfW = aabbHalfSize.x;
    float height = aabbHalfSize.y * 2.0f; // AABB�̍����͑S��
    DirectX::XMFLOAT3 min{ pos.x - halfW, pos.y, pos.z - halfW };
    DirectX::XMFLOAT3 max{ pos.x + halfW, pos.y + height, pos.z + halfW };
    return AABB(min, max);
}

// ���f���`��i��Ԃɉ����ă��f���ؑցj
void Player::Draw()
{
	MODEL* drawModel = (state == State::ELECTRICITY && electricModel_) ? electricModel_ : model_;
	if (!drawModel) return;
	// ��]��Y���݂̂��l�����ĊȈՓI�ɍ쐬
	float yaw = atan2f(direction_.x, direction_.z);
	XMMATRIX matRot = XMMatrixRotationY(yaw);
	XMMATRIX matTrans = XMMatrixTranslation(position_.x, position_.y, position_.z);
	XMMATRIX world = matRot * matTrans;

	ModelDraw(drawModel, world);
}

void Player::StartDash()
{
	// �d�C��Ԃ̂݃_�b�V���\
	if (state != State::ELECTRICITY || isDashing_) return;
	isDashing_ = true;
	dashTimeRemaining_ = dashDuration_;
	// �d�C��Ԃ̃_�b�V�����x = �ʏ�d�C���x * dashMultiplier_
	currentSpeed_ = baseSpeed_ * electricSpeedmul * dashMultiplier_;
}

void Player::StopDash()
{
	if (isDashing_) {
		isDashing_ = false;
		dashTimeRemaining_ = 0.0f;
		//������͏�Ԃɉ������ʏ푬�x
		currentSpeed_ = (state == State::ELECTRICITY) ? (baseSpeed_ * electricSpeedmul) : baseSpeed_;
	}
}

void Player::Jump(float jumpForce)
{
	// �n�ʂɐڂ��Ă���ꍇ�̂݃W�����v�\
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

// �J���������i������ target.y �𗘗p�j
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

// �d�����ӌ��o���\�b�h
bool Player::IsNearPole() const
{
	extern ObjectManager g_ObjectManager;
	auto poles = g_ObjectManager.GetAllPoles();
	
	for (const auto& pole : poles) {
		if (!pole) continue;
		
		DirectX::XMFLOAT3 polePos = pole->GetPosition();
		float dx = polePos.x - position_.x;
		float dz = polePos.z - position_.z;
		// ���������݂̂Ŕ���i�����͖����j
		float horizontalDistance = sqrtf(dx*dx + dz*dz);
		
		if (horizontalDistance <= POLE_DETECTION_RADIUS) {
			return true;
		}
	}
	return false;
}

// �ł��߂��d���ɃX�i�b�v���郁�\�b�h
void Player::SnapToNearestPowerLine()
{
	if (state != State::ELECTRICITY) return;

	extern ObjectManager g_ObjectManager;
	auto powerLines = g_ObjectManager.GetAllPowerLines();
	
	if (powerLines.empty()) return;

	float minDistance = FLT_MAX;
	DirectX::XMFLOAT3 snappedPos = position_;
	bool found = false;

	// ���ׂĂ̓d���̒�����ł��߂��|�C���g��T��
	for (const auto& line : powerLines) {
		if (!line) continue;

		// �d����̍ł��߂��|�C���g���擾
		DirectX::XMFLOAT3 closestPoint = line->GetClosestPointOnLine(position_);
		
		float dx = closestPoint.x - position_.x;
		float dy = closestPoint.y - position_.y;
		float dz = closestPoint.z - position_.z;
		
		// ���������Ɛ��������𕪕�
		float horizontalDist = sqrtf(dx * dx + dz * dz);
		float verticalDist = fabsf(dy);
		
		// �����������͈͓��Ȃ�A���������Ɋւ�炸�X�i�b�v�Ώۂɂ���
		// ��������: 2m�ȓ��A��������: 5m�ȓ�
		if (horizontalDist <= POWER_LINE_SNAP_DISTANCE && verticalDist <= 5.0f) {
			// 3���������ōł��߂����̂�I��
			float distance = sqrtf(dx * dx + dy * dy + dz * dz);
			
			if (distance < minDistance) {
				minDistance = distance;
				snappedPos = closestPoint;
				found = true;
			}
		}
	}

	// �ł��߂��d���ɃX�i�b�v
	if (found) {
		position_ = snappedPos;
		isGrounded_ = true;
		velocityY_ = 0.0f;
	}
}

// �d�C��Ԃ���l�Ԃɕω�����ۂɓd�����璵�˕Ԃ�
void Player::KnockbackFromPole()
{
	extern ObjectManager g_ObjectManager;
	auto poles = g_ObjectManager.GetAllPoles();
	
	if (poles.empty()) return;

	// �ł��߂��d����T��
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

	// �ł��߂��d������v���C���[�Ɍ������������v�Z
	float knockbackDx = position_.x - nearestPolePos.x;
	float knockbackDz = position_.z - nearestPolePos.z;
	float knockbackDist = sqrtf(knockbackDx * knockbackDx + knockbackDz * knockbackDz);

	if (knockbackDist > 0.001f) {
		// ���K�����ĕ������v�Z
		knockbackDx /= knockbackDist;
		knockbackDz /= knockbackDist;

		// �d������m���ɗ����iKNOCKBACK_DISTANCE = 3.0f�j
		position_.x = nearestPolePos.x + knockbackDx * KNOCKBACK_DISTANCE;
		position_.z = nearestPolePos.z + knockbackDz * KNOCKBACK_DISTANCE;

		// �^��ɃW�����v�i���������̈ړ��Ȃ��j
		velocityY_ = KNOCKBACK_JUMP_FORCE;
		isGrounded_ = false;
	}
}

// �d�C��Ԃւ̕ω����Ɋe���Ԃ����Z�b�g
void Player::ResetToElectricityState()
{
	// ���x�����Z�b�g
	velocityY_ = 0.0f;
	isGrounded_ = true;
	isDashing_ = false;
	dashTimeRemaining_ = 0.0f;

	// �d�C��Ԃ̑��x��ݒ�
	currentSpeed_ = baseSpeed_ * electricSpeedmul;

	// �ł��߂��d���ɃX�i�b�v
	SnapToNearestPowerLine();
}

// �v���C���[�X�e�[�g�ύX���\�b�h
void Player::ChangeState(Player::State newState)
{
	state = newState;
}
