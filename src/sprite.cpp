/*==============================================================================

   �X�v���C�g�`�� [Sprite.cpp]
														 Author : Youhei Sato
														 Date   : 2025/05/15
--------------------------------------------------------------------------------

==============================================================================*/
#include <d3d11.h>
#include <DirectXMath.h>
#include "../include/DirectXTex.h"
using namespace DirectX;
#include "../include/direct3d.h"
#include "../include/shader.h"
#include "../include/sprite.h"
#include "../include/texture.h"
#include "../include/debug_ostream.h"


static constexpr int NUM_VERTEX = 4; // ���_��


static ID3D11Buffer* g_pVertexBuffer = nullptr; // ���_�o�b�t�@
//static ID3D11ShaderResourceView* g_pTexture = nullptr;//2��3���Ƒ��₷�Ƃ��ɂ͂�������₵�Ė��O��ς���(1�ɂ�1��)


// ���ӁI�������ŊO������ݒ肳�����́BRelease�s�v�B
static ID3D11Device* g_pDevice = nullptr;
static ID3D11DeviceContext* g_pContext = nullptr;

// ���_�\����
struct Vertex
{
	XMFLOAT3 position; // ���_���W
	XMFLOAT4 color;    // ���_�F
	XMFLOAT2 texcoord; // �e�N�X�`�����W
};


void Sprite_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	// �f�o�C�X�ƃf�o�C�X�R���e�L�X�g�̃`�F�b�N
	if (!pDevice || !pContext) {
		hal::dout << "Sprite_Initialize() : �^����ꂽ�f�o�C�X���R���e�L�X�g���s���ł�" << std::endl;
		return;
	}

	// �f�o�C�X�ƃf�o�C�X�R���e�L�X�g�̕ۑ�
	g_pDevice = pDevice;
	g_pContext = pContext;

	// ���_�o�b�t�@����
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(Vertex) * NUM_VERTEX;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	g_pDevice->CreateBuffer(&bd, NULL, &g_pVertexBuffer);
}

void Sprite_Finalize(void)
{
	//SAFE_RELEASE(g_pTexture);
	SAFE_RELEASE(g_pVertexBuffer);
}

void Sprite_Draw(int texid, float x, float y, float w,float h, int tx, int ty, int tw, int th,float angle,DirectX::XMFLOAT4 color)
{
	// �V�F�[�_�[��`��p�C�v���C���ɐݒ�
	Shader_Begin();

	// ���_�o�b�t�@�����b�N����
	D3D11_MAPPED_SUBRESOURCE msr;
	g_pContext->Map(g_pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);

	// ���_�o�b�t�@�ւ̉��z�|�C���^���擾
	Vertex* v = (Vertex*)msr.pData;

	// ���_������������
	const float SCREEN_WIDTH = (float)Direct3D_GetBackBufferWidth();
	const float SCREEN_HEIGHT = (float)Direct3D_GetBackBufferHeight();

	constexpr float W = 512.0f;
	constexpr float H = 512.0f;

	// ��ʂ̍��ォ��E���Ɍ�����������`�悷��
	v[0].position = { -0.5f, -0.5f, 0.0f };
	v[1].position = { +0.5f, -0.5f, 0.0f };
	v[2].position = { -0.5f, +0.5f, 0.0f };
	v[3].position = { +0.5f, +0.5f, 0.0f };

	// �e�N�X�`�����W�̐ݒ�
	float tsw = (float)Texture_GetWidth(texid);
	float tsh = (float)Texture_GetHeight(texid);

	//�F		�@{R,G,B,A}
	v[0].color = color;
	v[1].color = color;
	v[2].color = color;
	v[3].color = color;


	float u0 = tx / tsw;
	float v0 = ty / tsh;
	float u1 = (tx + tw) / tsw;
	float v1 = (ty + th) / tsh;

	v[0].texcoord = { u0 , v0 };
	v[1].texcoord = { u1 , v0 };
	v[2].texcoord = { u0 , v1 };
	v[3].texcoord = { u1 , v1 };

	float pay = 3.14;
	// ���_�o�b�t�@�̃��b�N������
	g_pContext->Unmap(g_pVertexBuffer, 0);

	// ���_�o�b�t�@��`��p�C�v���C���ɐݒ�
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	g_pContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);

	// ���_�V�F�[�_�[��World�ϊ��s���ݒ�
	XMMATRIX s = XMMatrixScaling(w, h, 1.0f);
	XMMATRIX r = XMMatrixRotationZ(angle);//XMMATRIX�����Z������ꍇ
	XMMATRIX t = XMMatrixTranslation(x,y,0.0f);
	Shader_SetWorldMatrix(s * r * t);//����

	// ���_�V�F�[�_�[�Ƀv���W�F�N�V�����ϊ��s���ݒ�
	Shader_SetProjectionMatrix(XMMatrixOrthographicOffCenterLH(0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 0.0f, 1.0f));

	Shader_SetColor(color);

	// ��ʂ̒�����(0,0)�̐ݒ�
	//Shader_SetMatrix(XMMatrixOrthographicOffCenterLH(-SCREEN_WIDTH * 0.5, SCREEN_WIDTH * 0.5, SCREEN_HEIGHT * 0.5, -SCREEN_HEIGHT * 0.5, 0.0f, 1.0f));

	 //�v���~�e�B�u�g�|���W�ݒ�
	g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	Texture_SetTexture(texid);

	//�l�p�`
	g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// �|���S���`�施�ߔ��s
	g_pContext->Draw(NUM_VERTEX, 0);

}

void Sprite_Draw(int texid, float x, float y, float w, float h, int tx, int ty, int tw, int th, DirectX::XMFLOAT4 color)
{
	// �V�F�[�_�[��`��p�C�v���C���ɐݒ�
	Shader_Begin();

	// ���_�o�b�t�@�����b�N����
	D3D11_MAPPED_SUBRESOURCE msr;
	g_pContext->Map(g_pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);

	// ���_�o�b�t�@�ւ̉��z�|�C���^���擾
	Vertex* v = (Vertex*)msr.pData;

	// ���_������������
	const float SCREEN_WIDTH = (float)Direct3D_GetBackBufferWidth();
	const float SCREEN_HEIGHT = (float)Direct3D_GetBackBufferHeight();


	v[0].position = { x,     y,     0.0f };
	v[1].position = { w + x, y,     0.0f };
	v[2].position = { x,     h + y, 0.0f };
	v[3].position = { w + x, h + y, 0.0f };

	// �e�N�X�`�����W�̐ݒ�
	float tsw = (float)Texture_GetWidth(texid);
	float tsh = (float)Texture_GetHeight(texid);

	//�F		�@{R,G,B,A}
	v[0].color = color;
	v[1].color = color;
	v[2].color = color;
	v[3].color = color;


	float u0 = tx / tsw;
	float v0 = ty / tsh;
	float u1 = (tx + tw) / tsw;
	float v1 = (ty + th) / tsh;

	v[0].texcoord = { u0 , v0 };
	v[1].texcoord = { u1 , v0 };
	v[2].texcoord = { u0 , v1 };
	v[3].texcoord = { u1 , v1 };

	// ���_�o�b�t�@�̃��b�N������
	g_pContext->Unmap(g_pVertexBuffer, 0);

	// ���_�o�b�t�@��`��p�C�v���C���ɐݒ�
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	g_pContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);

	// ���_�V�F�[�_�[��World�ϊ��s���ݒ�
	Shader_SetWorldMatrix(XMMatrixIdentity());

	// ���_�V�F�[�_�[�Ƀv���W�F�N�V�����ϊ��s���ݒ�
	Shader_SetProjectionMatrix(XMMatrixOrthographicOffCenterLH(0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 0.0f, 1.0f));

	Shader_SetColor(color);

	// ��ʂ̒�����(0,0)�̐ݒ�
	//Shader_SetMatrix(XMMatrixOrthographicOffCenterLH(-SCREEN_WIDTH * 0.5, SCREEN_WIDTH * 0.5, SCREEN_HEIGHT * 0.5, -SCREEN_HEIGHT * 0.5, 0.0f, 1.0f));

	 //�v���~�e�B�u�g�|���W�ݒ�
	g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	Texture_SetTexture(texid);

	//�O�p�`�̓�����h��Ԃ�
	//g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//�l�p�`
	g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// �|���S���`�施�ߔ��s
	g_pContext->Draw(NUM_VERTEX, 0);

}

//�ȈՔŁ@�A���O������
void Sprite_Draw(int texid, float x, float y,float zoom,XMFLOAT4 color)
{
	// �V�F�[�_�[��`��p�C�v���C���ɐݒ�
	Shader_Begin();

	// ���_�o�b�t�@�����b�N���� GPU���͂���Ȃ��Ȃ�
	D3D11_MAPPED_SUBRESOURCE msr;
	g_pContext->Map(g_pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);

	// ���_�o�b�t�@�ւ̉��z�|�C���^���擾
	Vertex* v = (Vertex*)msr.pData;

	// ���_������������
	const float SCREEN_WIDTH = (float)Direct3D_GetBackBufferWidth();
	const float SCREEN_HEIGHT = (float)Direct3D_GetBackBufferHeight();

	//�e�N�X�`���S�̂̕��ƍ���
	float tsw = (float)Texture_GetWidth(texid);
	float tsh = (float)Texture_GetHeight(texid);

	//�����`�̍��W
	v[0].position = { x,       y,        0.0f }; // ����
	v[1].position = { (x + tsw) * zoom, y,        0.0f }; // �E��
	v[2].position = { x,       (y + tsh) * zoom, 0.0f };;// ����
	v[3].position = { (x + tsw) * zoom, (y + tsh) * zoom, 0.0f }; // �E��

	//���_�̐F
	v[0].color = color; // ��
	v[1].color = color; // ��
	v[2].color = color; // ��
	v[3].color = color; // ��

	//�e�N�X�`���͈̔�
	v[0].texcoord = { 0.0f, 0.0f };
	v[1].texcoord = { 1.0f, 0.0f };
	v[2].texcoord = { 0.0f, 1.0f };
	v[3].texcoord = { 1.0f, 1.0f };

	// ���_�o�b�t�@�̃��b�N������ GPU��������悤�ɂ���
	g_pContext->Unmap(g_pVertexBuffer, 0);

	// ���_�o�b�t�@��`��p�C�v���C���ɐݒ�
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	g_pContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);


	//���_�V�F�[�_�[��world�ϊ��s���ݒ�@�P�ʍs��@
	Shader_SetWorldMatrix(XMMatrixIdentity());

	// ���_�V�F�[�_�[�ɕϊ��s���ݒ�@��ԏ㉺���E�̍��W�����߂� ���W�n������
	Shader_SetProjectionMatrix(XMMatrixOrthographicOffCenterLH(
		0.0f,    //��
		SCREEN_WIDTH,    //�E
		SCREEN_HEIGHT,    //��
		0.0f, //��
		0.0f,
		1.0f));

	//�F�ƃA���t�@�l�i�����x�j��ݒ�
	Shader_SetColor(color);

	// �v���~�e�B�u�g�|���W�ݒ�  �X�g���b�v��
	g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	//�e�N�X�`����ݒ�
	Texture_SetTexture(texid);

	// �X�v���C�g�`�施�ߔ��s
	g_pContext->Draw(NUM_VERTEX, 0);
}



//void Sprite_Draw(int texid, float x, float y, float w, float h, int tx, int ty, int tw, int th,float bairitu, DirectX::XMFLOAT4 color)
//{
//	// �V�F�[�_�[��`��p�C�v���C���ɐݒ�
//	Shader_Begin();
//
//	// ���_�o�b�t�@�����b�N����
//	D3D11_MAPPED_SUBRESOURCE msr;
//	g_pContext->Map(g_pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
//
//	// ���_�o�b�t�@�ւ̉��z�|�C���^���擾
//	Vertex* v = (Vertex*)msr.pData;
//
//	// ���_������������
//	const float SCREEN_WIDTH = (float)Direct3D_GetBackBufferWidth();
//	const float SCREEN_HEIGHT = (float)Direct3D_GetBackBufferHeight();
//
//
//	//�����`�̍��W
//	v[0].position = { x,            y,        0.0f };	// ����
//	v[1].position = { x  * bairitu, y,   0.0f };		// �E��
//	v[2].position = { x,            y * bairitu, 0.0f };// ����
//	v[3].position = { x  * bairitu, y * bairitu, 0.0f };// �E��
//
//	// �e�N�X�`�����W�̐ݒ�
//	float tsw = (float)Texture_GetWidth(texid);
//	float tsh = (float)Texture_GetHeight(texid);
//
//	//�F		�@{R,G,B,A}
//	v[0].color = color;
//	v[1].color = color;
//	v[2].color = color;
//	v[3].color = color;
//
//
//	float u0 = tx / tsw;
//	float v0 = ty / tsh;
//	float u1 = (tx + tw) / tsw;
//	float v1 = (ty + th) / tsh;
//
//	v[0].texcoord = { u0 , v0 };
//	v[1].texcoord = { u1 , v0 };
//	v[2].texcoord = { u0 , v1 };
//	v[3].texcoord = { u1 , v1 };
//
//	// ���_�o�b�t�@�̃��b�N������
//	g_pContext->Unmap(g_pVertexBuffer, 0);
//
//	// ���_�o�b�t�@��`��p�C�v���C���ɐݒ�
//	UINT stride = sizeof(Vertex);
//	UINT offset = 0;
//	g_pContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);
//
//	// ���_�V�F�[�_�[��World�ϊ��s���ݒ�
//	Shader_SetWorldMatrix(XMMatrixIdentity());
//
//	// ���_�V�F�[�_�[�Ƀv���W�F�N�V�����ϊ��s���ݒ�
//	Shader_SetProjectionMatrix(XMMatrixOrthographicOffCenterLH(0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 0.0f, 1.0f));
//
//	Shader_SetColor(color);
//
//	// ��ʂ̒�����(0,0)�̐ݒ�
//	//Shader_SetMatrix(XMMatrixOrthographicOffCenterLH(-SCREEN_WIDTH * 0.5, SCREEN_WIDTH * 0.5, SCREEN_HEIGHT * 0.5, -SCREEN_HEIGHT * 0.5, 0.0f, 1.0f));
//
//	 //�v���~�e�B�u�g�|���W�ݒ�
//	g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
//
//	Texture_SetTexture(texid);
//
//	//�O�p�`�̓�����h��Ԃ�
//	//g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//
//	//�l�p�`
//	g_pContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
//
//	// �|���S���`�施�ߔ��s
//	g_pContext->Draw(NUM_VERTEX, 0);
//
//}



