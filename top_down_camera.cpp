#include "top_down_camera.h"
#include "player.h"
#include "shader3d.h"
#include <DirectXMath.h>
#include "direct3d.h"
#include "debug_ostream.h"
#include <cmath>
#include <algorithm> // for std::min/std::max

using namespace DirectX;

// (VecSub, VecLen, etc. helper functions remain the same)
// ...

TopDownCamera::TopDownCamera(Player* owner, float height, float orthoWidth)
 : owner_(owner), height_(height), orthoWidth_(orthoWidth)
{
 // プレイヤーの初期向きに合わせてカメラの角度を初期化
 if (owner_) {
 DirectX::XMFLOAT3 initialDir = owner_->GetDirection();
 cameraAngle_ = std::atan2(initialDir.x, initialDir.z);
 }
}

void TopDownCamera::Update(double elapsed_time)
{
 if (!owner_) return;

 auto controller = owner_->GetController();
 if (!controller) return;

 auto rightStick = controller->GetStick(Controller::DIR_RIGHT);
 float dt = static_cast<float>(elapsed_time);

 //右スティックでカメラを操作
 if (rightStick.outside) {
 float stickX = static_cast<float>(rightStick.x) /32767.0f;
 float stickY = static_cast<float>(rightStick.y) /32767.0f;

 // 水平角度を更新（符号を反転）
 cameraAngle_ += stickX * TPSCameraSettings::SENSITIVITY_X * dt;

 // 垂直角度を更新（符号を反転）
 cameraPitch_ -= stickY * TPSCameraSettings::SENSITIVITY_Y * dt;
 // C++14互換のclamp
 cameraPitch_ = std::max(-0.5f, std::min(cameraPitch_,1.2f));
 }
 //角度を正規化
 if (cameraAngle_ >6.28318f) cameraAngle_ -=6.28318f;
 if (cameraAngle_ <0.0f) cameraAngle_ +=6.28318f;
}

void TopDownCamera::SetMatrix() const
{
 if (!owner_) return;

 XMFLOAT3 playerPos = owner_->GetPosition();
 XMFLOAT3 targetPos = { playerPos.x, playerPos.y + height_, playerPos.z };

 // 球面座標系でカメラの位置を計算
 float camX = targetPos.x + cameraDistance_ * std::sin(cameraAngle_) * std::cos(cameraPitch_);
 float camY = targetPos.y + cameraDistance_ * std::sin(cameraPitch_);
 float camZ = targetPos.z + cameraDistance_ * std::cos(cameraAngle_) * std::cos(cameraPitch_);
 XMFLOAT3 eye = { camX, camY, camZ };

 XMMATRIX view = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&targetPos), {0.0f,1.0f,0.0f });
 Shader3d_SetViewMatrix(view);

 // 投影行列（パースペクティブ）
 float w = static_cast<float>(Direct3D_GetBackBufferWidth());
 float h = static_cast<float>(Direct3D_GetBackBufferHeight());
 XMMATRIX proj = XMMatrixPerspectiveFovLH(XMConvertToRadians(60.0f), w / h,0.1f,1000.0f);
 Shader3d_SetProjectionMatrix(proj);
}