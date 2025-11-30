//////////////////////////////
//debugCamera.h
//Author: Hiroshi Kasiwagi
//Date: 2025/10/28
////////////////////////////////
#ifndef DEBUG_CAMERA_H
#define DEBUG_CAMERA_H
#include "camera.h"
#include <DirectXMath.h>

class DebugCamera : public Camera
{
private:
	DirectX::XMFLOAT3 m_front{};
	DirectX::XMFLOAT3 m_right{};
	DirectX::XMFLOAT3 m_up{};
	DirectX::XMFLOAT3 m_position{};
	float m_fov{ 1.0f };
public:

	DebugCamera(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT3& target);
	void Update(double elapsed_time)override;//キー入力による更新
	void SetMatrix() const override;
};

#endif // DEBUG_CAMERA_H