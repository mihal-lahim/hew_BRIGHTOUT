/*==============================================================================

   �|���S���`�� [polygon.cpp]
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
#include "../include/debug_ostream.h"


static constexpr int NUM_VERTEX = 4; // ���_��


static ID3D11Buffer* g_pVertexBuffer = nullptr; // ���_�o�b�t�@
//static ID3D11ShaderResourceView* g_pTexture = nullptr;//2��3���Ƒ��₷�Ƃ��ɂ͂�������₵�Ė��O��ς���(1�ɂ�1��)


// ���ӁI�������ŊO������ݒ肳�����́BRelease�s�v�B
static ID3D11Device* g_pDevice = nullptr;
static ID3D11DeviceContext* g_pContext = nullptr;


//�V��
#define SAIDAI_X	5000.0f
#define SAIDAI_Y	5000.0f
#define ZOUKA_X		5.0f
#define ZOUKA_Y		5.0f
float sizeX = SAIDAI_X;
float sizeY = SAIDAI_Y;
bool hantenX = false;
bool hantenY = false;

// ���_�\����
struct Vertex
{
	XMFLOAT3 position; // ���_���W
	XMFLOAT4 color;    // ���_�F
	XMFLOAT2 texcoord; // �e�N�X�`�����W
};


void Polygon_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	// �f�o�C�X�ƃf�o�C�X�R���e�L�X�g�̃`�F�b�N
	if (!pDevice || !pContext) {
		hal::dout << "Polygon_Initialize() : �^����ꂽ�f�o�C�X���R���e�L�X�g���s���ł�" << std::endl;
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

void Polygon_Finalize(void)
{
	//SAFE_RELEASE(g_pTexture);
	SAFE_RELEASE(g_pVertexBuffer);
}

void Polygon_Draw(void)
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


	// �R�T�C���Ȃǂ��g�����ꍇ
	static float a = 0.0f;
	a += 0.01f; // ���l�����������₷
	//float W = 512.0f;// * (cos(a) + 1);	// �|���S���̕�
	//float H = 512.0f;// * (sin(a) + 1);	// �|���S���̍���

	//float X = 64.0f * (cos(a) + 5);		// �|���S���̕�
	//float Y = 64.0f * (sin(a) + 5);		// �|���S���̍���

	// ��]������
	float W = 512.0f;						// �|���S���̕�
	float H = 512.0f;						// �|���S���̍���

	float X = 64.0f * (cos(a) * 7 + 10);	// �|���S���̕�
	float Y = 64.0f * (sin(a) * 7 + 3);		// �|���S���̍���

	v[0].position = { X,     Y,     0.0f };
	v[1].position = { W + X  , Y,     0.0f };
	v[2].position = { X,     H + Y  , 0.0f };
	v[3].position = { W + X  , H + Y  , 0.0f };


	//���B
	static float c = 0.0f;
	v[0].texcoord = { 0.0f + c, 0.0f + c };
	v[1].texcoord = { 2.0f + c, 0.0f + c };
	v[2].texcoord = { 0.0f + c, 2.0f + c };
	v[3].texcoord = { 2.0f + c, 2.0f + c };
	c += 0.01f;

	//float pay = 3.14;
	// ���_�o�b�t�@�̃��b�N������
	g_pContext->Unmap(g_pVertexBuffer, 0);

	// ���_�o�b�t�@��`��p�C�v���C���ɐݒ�
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	g_pContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);

	// ���_�V�F�[�_�[��World�ϊ��s���ݒ�
	Shader_SetWorldMatrix(XMMatrixIdentity());

	// ���_�V�F�[�_�[�ɕϊ��s���ݒ�
	Shader_SetProjectionMatrix(XMMatrixOrthographicOffCenterLH(0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 0.0f, 1.0f));
	Shader_SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });

	//�l�p�`
	g_pContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// �|���S���`�施�ߔ��s
	g_pContext->Draw(NUM_VERTEX, 0);
}
