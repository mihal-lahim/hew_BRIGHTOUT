//////////////////////////////
//debugCamera.cpp
//Author: Hiroshi Kasiwagi
//Date: 2025/10/28
////////////////////////////////
#include "../include/debug_camera.h"
#include "../include/direct3D.h"
#include "../include/shader3d.h"
#include "../include/key_logger.h"
using namespace DirectX;

DebugCamera::DebugCamera(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT3& target)
	:m_position(position), m_up({ 0.0f,1.0f,0.0f })
{
	//�O���������
	//XMVECTOR�^�ɕϊ��@�������Ȃ��ƃx�N�g�����Z�ł��Ȃ�
	XMVECTOR xTarget = XMLoadFloat3(&target);
	XMVECTOR xPosition = XMLoadFloat3(&position);
	//���K�����đO�����x�N�g�������߂�
	XMVECTOR xFront = XMVector3Normalize(xTarget - xPosition);
	//XMFLOAT3�^�ɖ߂�
	XMStoreFloat3(&m_front, xFront);
	//��s�ŏ�����...
	//XMStoreFloat3(&m_front, XMVector3Normalize(XMLoadFloat3(&target)-XMLoadFloat3(&position)));

	//�K���ɉE�����x�N�g�������
	XMVECTOR xRight = XMVector3Cross(xPosition, { 0.0f,1.0f,0.0f });//�����鏇�ԂŌ������ς��
	xRight = XMVector3Normalize(xRight);	//�����̂Ȃ��x�N�g����Normalize���g���ƃG���[����
	//XMFLOAT3�^�ɖ߂�
	XMStoreFloat3(&m_right, xRight);


}

void DebugCamera::Update(double elapsed_time)
{
	//���x�ݒ�
	const float MOVE_SPEED = static_cast<float>(2.0f * elapsed_time);//�ړ����x
	const float ROTATION_SPEED = static_cast<float>(XMConvertToRadians(60.0f) * elapsed_time);//��]���x
	//XMVECTOR�ɕϊ�
	XMVECTOR xFront = XMLoadFloat3(&m_front);
	XMVECTOR xRight = XMLoadFloat3(&m_right);
	XMVECTOR xUp = XMLoadFloat3(&m_up);
	XMVECTOR xPosition = XMLoadFloat3(&m_position);

	//���Z�b�g
	if (KeyLogger_IsTrigger(KK_TAB))
	{
		XMVECTOR resetUp = { 0.0f,1.0f,0.0f };
		xUp = resetUp;
	}
	//�Y�[���A�E�g
	if (KeyLogger_IsPressed(KK_K))
	{
		m_fov += 0.1f;
	}
	//�Y�[���C��
	if (KeyLogger_IsPressed(KK_L))
	{
		m_fov -= 0.1f;
		if (m_fov <= 0)
		{
			m_fov = 0.1f;
		}
	}
	//�E��]
	if (KeyLogger_IsPressed(KK_RIGHT))
	{
		//XMMatrixRotationAxis()�ŉ�]���Ɖ�]�̑��x�ݒ�
		XMMATRIX rotation = XMMatrixRotationAxis(xUp, ROTATION_SPEED);
		xFront = XMVector3TransformNormal(xFront, rotation);
		xRight = XMVector3Cross(xUp, xFront);
		xFront = XMVector3Normalize(xFront);
		xRight = XMVector3Normalize(xRight);
	}
	//����]
	if (KeyLogger_IsPressed(KK_LEFT))
	{
		XMMATRIX rotation = XMMatrixRotationAxis(xUp, -ROTATION_SPEED);
		xFront = XMVector3TransformNormal(xFront, rotation);
		xRight = XMVector3Cross(xUp, xFront);
		xFront = XMVector3Normalize(xFront);
		xRight = XMVector3Normalize(xRight);
	}
	//���]
	if (KeyLogger_IsPressed(KK_UP))
	{
		XMMATRIX rotation = XMMatrixRotationAxis(xRight, -ROTATION_SPEED);
		xFront = XMVector3TransformNormal(xFront, rotation);
		xRight = XMVector3Cross(xUp, xFront);
		xFront = XMVector3Normalize(xFront);
		xRight = XMVector3Normalize(xRight);
	}
	//����]
	if (KeyLogger_IsPressed(KK_DOWN))
	{
		XMMATRIX rotation = XMMatrixRotationAxis(xRight, ROTATION_SPEED);
		xFront = XMVector3TransformNormal(xFront, rotation);
		xRight = XMVector3Cross(xUp, xFront);
		xFront = XMVector3Normalize(xFront);
		xRight = XMVector3Normalize(xRight);
	}
	//������݉E��]
	if (KeyLogger_IsPressed(KK_P))
	{
		XMMATRIX rotation = XMMatrixRotationAxis(xFront, ROTATION_SPEED);
		xUp = XMVector3TransformNormal(xUp, rotation);
		xRight = XMVector3Cross(xUp, xFront);
		xFront = XMVector3Normalize(xFront);
		xUp = XMVector3Normalize(xUp);
	}
	//������ݍ���]
	if (KeyLogger_IsPressed(KK_O))
	{
		XMMATRIX rotation = XMMatrixRotationAxis(xFront, -ROTATION_SPEED);
		xUp = XMVector3TransformNormal(xUp, rotation);
		xRight = XMVector3Cross(xUp, xFront);
		xFront = XMVector3Normalize(xFront);
		xUp = XMVector3Normalize(xUp);
	}
	//�O�i
	if (KeyLogger_IsPressed(KK_W))
	{
		xPosition += XMLoadFloat3(&m_front) * MOVE_SPEED;

	}
	//�E�ړ�
	if (KeyLogger_IsPressed(KK_D))
	{
		xPosition += XMLoadFloat3(&m_right) * MOVE_SPEED;
	}
	//���ړ�
	if (KeyLogger_IsPressed(KK_A))
	{
		xPosition -= XMLoadFloat3(&m_right) * MOVE_SPEED;
	}
	//���
	if (KeyLogger_IsPressed(KK_S))
	{
		xPosition -= XMLoadFloat3(&m_front) * MOVE_SPEED;
	}
	//�㉺�ړ�
	if (KeyLogger_IsPressed(KK_SPACE))
	{
		//���ړ�
		if (KeyLogger_IsPressed(KK_LEFTSHIFT))
		{
			xPosition -= XMVECTOR{ 0.0f, 1.0f, 0.0f } *MOVE_SPEED;
		}
		//��ړ�
		else
		{
			xPosition += XMVECTOR{ 0.0f, 1.0f, 0.0f } *MOVE_SPEED;
		}
	}
	//XMFLOAT3�ɖ߂�
	XMStoreFloat3(&m_position, xPosition);
	XMStoreFloat3(&m_front, xFront);
	XMStoreFloat3(&m_right, xRight);
	XMStoreFloat3(&m_up, xUp);

}

void DebugCamera::SetMatrix() const
{
	// �r���[�ϊ��s��̐ݒ�

	XMMATRIX mtxView = XMMatrixLookAtLH(
		XMLoadFloat3(&m_position),
		XMLoadFloat3(&m_position) + XMLoadFloat3(&m_front),
		XMLoadFloat3(&m_up));
	Shader3d_SetViewMatrix(mtxView);

	//���_�V�F�[�_�[�Ƀv���W�F�N�V�����ϊ��s���ݒ�
	// ��p90�x�A�A�X�y�N�g��͉�ʃT�C�Y����v�Z�A�O�[0.1�A��[1000
	//������
	float w = static_cast<float>(Direct3D_GetBackBufferWidth());
	float h = static_cast<float>(Direct3D_GetBackBufferHeight());
	XMMATRIX mtxProj = XMMatrixPerspectiveFovLH(m_fov, w / h, 0.1f, 1000.0f);
	Shader3d_SetProjectionMatrix(mtxProj);
}
