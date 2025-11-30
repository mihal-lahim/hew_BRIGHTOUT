//////////////////////////////
//debugCamera.cpp
//Author: Hiroshi Kasiwagi
//Date: 2025/10/28
////////////////////////////////
#include "debug_camera.h"
#include "direct3D.h"
#include "shader3d.h"
#include "key_logger.h"
using namespace DirectX;

DebugCamera::DebugCamera(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT3& target)
	:m_position(position), m_up({ 0.0f,1.0f,0.0f })
{
	//前方向を作る
	//XMVECTOR型に変換　こうしないとベクトル演算できない
	XMVECTOR xTarget = XMLoadFloat3(&target);
	XMVECTOR xPosition = XMLoadFloat3(&position);
	//正規化して前方向ベクトルを求める
	XMVECTOR xFront = XMVector3Normalize(xTarget - xPosition);
	//XMFLOAT3型に戻す
	XMStoreFloat3(&m_front, xFront);
	//一行で書くと...
	//XMStoreFloat3(&m_front, XMVector3Normalize(XMLoadFloat3(&target)-XMLoadFloat3(&position)));

	//適当に右方向ベクトルを作る
	XMVECTOR xRight = XMVector3Cross(xPosition, { 0.0f,1.0f,0.0f });//かける順番で向きが変わる
	xRight = XMVector3Normalize(xRight);	//長さのないベクトルにNormalizeを使うとエラーする
	//XMFLOAT3型に戻す
	XMStoreFloat3(&m_right, xRight);


}

void DebugCamera::Update(double elapsed_time)
{
	//速度設定
	const float MOVE_SPEED = static_cast<float>(2.0f * elapsed_time);//移動速度
	const float ROTATION_SPEED = static_cast<float>(XMConvertToRadians(60.0f) * elapsed_time);//回転速度
	//XMVECTORに変換
	XMVECTOR xFront = XMLoadFloat3(&m_front);
	XMVECTOR xRight = XMLoadFloat3(&m_right);
	XMVECTOR xUp = XMLoadFloat3(&m_up);
	XMVECTOR xPosition = XMLoadFloat3(&m_position);

	//リセット
	if (KeyLogger_IsTrigger(KK_TAB))
	{
		XMVECTOR resetUp = { 0.0f,1.0f,0.0f };
		xUp = resetUp;
	}
	//ズームアウト
	if (KeyLogger_IsPressed(KK_K))
	{
		m_fov += 0.1f;
	}
	//ズームイン
	if (KeyLogger_IsPressed(KK_L))
	{
		m_fov -= 0.1f;
		if (m_fov <= 0)
		{
			m_fov = 0.1f;
		}
	}
	//右回転
	if (KeyLogger_IsPressed(KK_RIGHT))
	{
		//XMMatrixRotationAxis()で回転軸と回転の速度設定
		XMMATRIX rotation = XMMatrixRotationAxis(xUp, ROTATION_SPEED);
		xFront = XMVector3TransformNormal(xFront, rotation);
		xRight = XMVector3Cross(xUp, xFront);
		xFront = XMVector3Normalize(xFront);
		xRight = XMVector3Normalize(xRight);
	}
	//左回転
	if (KeyLogger_IsPressed(KK_LEFT))
	{
		XMMATRIX rotation = XMMatrixRotationAxis(xUp, -ROTATION_SPEED);
		xFront = XMVector3TransformNormal(xFront, rotation);
		xRight = XMVector3Cross(xUp, xFront);
		xFront = XMVector3Normalize(xFront);
		xRight = XMVector3Normalize(xRight);
	}
	//上回転
	if (KeyLogger_IsPressed(KK_UP))
	{
		XMMATRIX rotation = XMMatrixRotationAxis(xRight, -ROTATION_SPEED);
		xFront = XMVector3TransformNormal(xFront, rotation);
		xRight = XMVector3Cross(xUp, xFront);
		xFront = XMVector3Normalize(xFront);
		xRight = XMVector3Normalize(xRight);
	}
	//下回転
	if (KeyLogger_IsPressed(KK_DOWN))
	{
		XMMATRIX rotation = XMMatrixRotationAxis(xRight, ROTATION_SPEED);
		xFront = XMVector3TransformNormal(xFront, rotation);
		xRight = XMVector3Cross(xUp, xFront);
		xFront = XMVector3Normalize(xFront);
		xRight = XMVector3Normalize(xRight);
	}
	//きりもみ右回転
	if (KeyLogger_IsPressed(KK_P))
	{
		XMMATRIX rotation = XMMatrixRotationAxis(xFront, ROTATION_SPEED);
		xUp = XMVector3TransformNormal(xUp, rotation);
		xRight = XMVector3Cross(xUp, xFront);
		xFront = XMVector3Normalize(xFront);
		xUp = XMVector3Normalize(xUp);
	}
	//きりもみ左回転
	if (KeyLogger_IsPressed(KK_O))
	{
		XMMATRIX rotation = XMMatrixRotationAxis(xFront, -ROTATION_SPEED);
		xUp = XMVector3TransformNormal(xUp, rotation);
		xRight = XMVector3Cross(xUp, xFront);
		xFront = XMVector3Normalize(xFront);
		xUp = XMVector3Normalize(xUp);
	}
	//前進
	if (KeyLogger_IsPressed(KK_W))
	{
		xPosition += XMLoadFloat3(&m_front) * MOVE_SPEED;

	}
	//右移動
	if (KeyLogger_IsPressed(KK_D))
	{
		xPosition += XMLoadFloat3(&m_right) * MOVE_SPEED;
	}
	//左移動
	if (KeyLogger_IsPressed(KK_A))
	{
		xPosition -= XMLoadFloat3(&m_right) * MOVE_SPEED;
	}
	//後退
	if (KeyLogger_IsPressed(KK_S))
	{
		xPosition -= XMLoadFloat3(&m_front) * MOVE_SPEED;
	}
	//上下移動
	if (KeyLogger_IsPressed(KK_SPACE))
	{
		//下移動
		if (KeyLogger_IsPressed(KK_LEFTSHIFT))
		{
			xPosition -= XMVECTOR{ 0.0f, 1.0f, 0.0f } *MOVE_SPEED;
		}
		//上移動
		else
		{
			xPosition += XMVECTOR{ 0.0f, 1.0f, 0.0f } *MOVE_SPEED;
		}
	}
	//XMFLOAT3に戻す
	XMStoreFloat3(&m_position, xPosition);
	XMStoreFloat3(&m_front, xFront);
	XMStoreFloat3(&m_right, xRight);
	XMStoreFloat3(&m_up, xUp);

}

void DebugCamera::SetMatrix() const
{
	// ビュー変換行列の設定

	XMMATRIX mtxView = XMMatrixLookAtLH(
		XMLoadFloat3(&m_position),
		XMLoadFloat3(&m_position) + XMLoadFloat3(&m_front),
		XMLoadFloat3(&m_up));
	Shader3d_SetViewMatrix(mtxView);

	//頂点シェーダーにプロジェクション変換行列を設定
	// 画角90度、アスペクト比は画面サイズから計算、前端0.1、後端1000
	//視錐台
	float w = static_cast<float>(Direct3D_GetBackBufferWidth());
	float h = static_cast<float>(Direct3D_GetBackBufferHeight());
	XMMATRIX mtxProj = XMMatrixPerspectiveFovLH(m_fov, w / h, 0.1f, 1000.0f);
	Shader3d_SetProjectionMatrix(mtxProj);
}
