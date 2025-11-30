/*==============================================================================

   �L���[�u�̕`�� [cube.cpp]
														 Author : sumi rintarou
														 Date   : 2025/10/16
--------------------------------------------------------------------------------

==============================================================================*/
#include "../include/cube.h"
#include <d3d11.h>
#include "../include/direct3d.h"
#include "../include/texture.h"
#include <DirectXMath.h>
using namespace DirectX;
#include "../include/debug_ostream.h"
#include "../include/shader3d.h"

static constexpr int NUM_VERTEX = 4 * 6;	// ���_��
static constexpr int NUM_INDEX = 36;		// �C���f�b�N�X��

static ID3D11Buffer* g_pVertexBuffer = nullptr; // ���_�o�b�t�@
static ID3D11Buffer* g_pIndexBuffer = nullptr; // �C���f�b�N�X�o�b�t�@

// ���ӁI�������ŊO������ݒ肳�����́BRelease�s�v�B
static ID3D11Device* g_pDevice = nullptr;
static ID3D11DeviceContext* g_pContext = nullptr;

// ���_�\����
struct Vertex3d
{
	XMFLOAT3 position; // ���_���W
	XMFLOAT3 normal;   // �@��
	XMFLOAT4 color;    // �F
	XMFLOAT2 texcoord; // �e�N�X�`�����W
};

static Vertex3d g_CubeVertex[] = {

	//���� Z <-
	{{-0.5f, 0.5f,-0.5f}, { 0.0f, 0.0f,-1.0f}, {1.0f,1.0f,1.0f,1.0f},{ 0.0f ,0.0f}},
	{{ 0.5f, 0.5f,-0.5f}, { 0.0f, 0.0f,-1.0f}, {1.0f,1.0f,1.0f,1.0f},{ 1.0f ,0.0f}},
	{{-0.5f,-0.5f,-0.5f}, { 0.0f, 0.0f,-1.0f}, {1.0f,1.0f,1.0f,1.0f},{ 0.0f ,1.0f}},
	{{ 0.5f,-0.5f,-0.5f}, { 0.0f, 0.0f,-1.0f}, {1.0f,1.0f,1.0f,1.0f},{ 1.0f ,1.0f}},

	//�E�� X ->
	{{ 0.5f, 0.5f,-0.5f}, { 1.0f, 0.0f, 0.0f}, {1.0f,1.0f,1.0f,1.0f},{ 0.0f ,0.0f}},
	{{ 0.5f, 0.5f, 0.5f}, { 1.0f, 0.0f, 0.0f}, {1.0f,1.0f,1.0f,1.0f},{ 1.0f ,0.0f}},
	{{ 0.5f,-0.5f,-0.5f}, { 1.0f, 0.0f, 0.0f}, {1.0f,1.0f,1.0f,1.0f},{ 0.0f ,1.0f}},
	{{ 0.5f,-0.5f, 0.5f}, { 1.0f, 0.0f, 0.0f}, {1.0f,1.0f,1.0f,1.0f},{ 1.0f ,1.0f}},

	//�w�� Z<-
	{{ 0.5f, 0.5f, 0.5f}, { 0.0f, 0.0f, 1.0f}, {1.0f,1.0f,1.0f,1.0f},{ 0.0f ,0.0f}},
	{{-0.5f, 0.5f, 0.5f}, { 0.0f, 0.0f, 1.0f}, {1.0f,1.0f,1.0f,1.0f},{ 1.0f ,0.0f}},
	{{ 0.5f,-0.5f, 0.5f}, { 0.0f, 0.0f, 1.0f}, {1.0f,1.0f,1.0f,1.0f},{ 0.0f ,1.0f}},
	{{-0.5f,-0.5f, 0.5f}, { 0.0f, 0.0f, 1.0f}, {1.0f,1.0f,1.0f,1.0f},{ 1.0f ,1.0f}},

	//���� X <-
	{{-0.5f, 0.5f, 0.5f}, { -1.0f, 0.0f, 0.0f}, {1.0f,1.0f,1.0f,1.0f},{ 0.0f ,0.0f}},
	{{-0.5f, 0.5f,-0.5f}, { -1.0f, 0.0f, 0.0f}, {1.0f,1.0f,1.0f,1.0f},{ 1.0f ,0.0f}},
	{{-0.5f,-0.5f, 0.5f}, { -1.0f, 0.0f, 0.0f}, {1.0f,1.0f,1.0f,1.0f},{ 0.0f ,1.0f}},
	{{-0.5f,-0.5f,-0.5f}, { -1.0f, 0.0f, 0.0f}, {1.0f,1.0f,1.0f,1.0f},{ 1.0f ,1.0f}},

	//��� Y ->
	{{-0.5f, 0.5f, 0.5f}, { 0.0f, 1.0f, 0.0f}, {1.0f,1.0f,1.0f,1.0f},{ 0.0f ,0.0f}},
	{{ 0.5f, 0.5f, 0.5f}, { 0.0f, 1.0f, 0.0f}, {1.0f,1.0f,1.0f,1.0f},{ 1.0f ,0.0f}},
	{{-0.5f, 0.5f,-0.5f}, { 0.0f, 1.0f, 0.0f}, {1.0f,1.0f,1.0f,1.0f},{ 0.0f ,1.0f}},
	{{ 0.5f, 0.5f,-0.5f}, { 0.0f, 1.0f, 0.0f}, {1.0f,1.0f,1.0f,1.0f},{ 1.0f ,1.0f}},

	//���� Y <-
	{{-0.5f,-0.5f,-0.5f}, { 0.0f,-1.0f, 0.0f}, {1.0f,1.0f,1.0f,1.0f},{ 0.0f ,0.0f}},
	{{ 0.5f,-0.5f,-0.5f}, { 0.0f,-1.0f, 0.0f}, {1.0f,1.0f,1.0f,1.0f},{ 1.0f ,0.0f}},
	{{-0.5f,-0.5f, 0.5f}, { 0.0f,-1.0f, 0.0f}, {1.0f,1.0f,1.0f,1.0f},{ 0.0f ,1.0f}},
	{{ 0.5f,-0.5f, 0.5f}, { 0.0f,-1.0f, 0.0f}, {1.0f,1.0f,1.0f,1.0f},{ 1.0f ,1.0f}},
};

static unsigned short g_CubeIndex[] = {
	 0,  1,  2,  1,  3,  2,
	 4,  5,  6,  5,  7,  6,
	 8,  9, 10,  9, 11, 10,
	12, 13, 14, 13, 15, 14,
	16, 17, 18, 17, 19, 18,
	20, 21, 22, 21, 23, 22,
};

void Cube_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
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
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(Vertex3d) * NUM_VERTEX;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;// �A�N�Z�X�t���O��0�ɂ��邱�Ƃ�CPU�ɃA�N�Z�X�ł��Ȃ�����

	// ���_�o�b�t�@�֗������ރf�[�^�̐ݒ�
	D3D11_SUBRESOURCE_DATA sd{};
	sd.pSysMem = g_CubeVertex;

	g_pDevice->CreateBuffer(&bd, &sd, &g_pVertexBuffer);

	// �C���f�b�N�X�o�b�t�@�쐬
	bd.ByteWidth = sizeof(unsigned short) * NUM_INDEX;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;

	// ���_�o�b�t�@�֗������ރf�[�^�̐ݒ�
	sd.pSysMem = g_CubeIndex;

	g_pDevice->CreateBuffer(&bd, &sd, &g_pIndexBuffer);

}

void Cube_Finalize()
{
	SAFE_RELEASE(g_pIndexBuffer);
	SAFE_RELEASE(g_pVertexBuffer);
}

void Cube_Draw(const int textrue, const XMMATRIX& mtxWorld)
{
	// �V�F�[�_�[��`��p�C�v���C���ɐݒ�
	Shader3d_Begin();

	// ���_�o�b�t�@��`��p�C�v���C���ɐݒ�
	UINT stride = sizeof(Vertex3d);
	UINT offset = 0;

	// �v���~�e�B�u�g�|���W�ݒ�
	g_pContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);

	// ���_�C���f�b�N�X��`����p�C�v���C���ɐݒ�
	g_pContext->IASetIndexBuffer(g_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	// ���[���h�ϊ��s����v�Z
	Shader3d_SetWorldMatrix(mtxWorld);

	// �v���~�e�B�u�g�|���W�ݒ�
	g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//�e�N�X�`���̐ݒ�
	Texture_SetTexture(textrue);

	// �}�e���A���J���[�̐ݒ�
	Shader3d_SetMaterialDiffuse({ 1.0f,1.0f,1.0f,1.0f });

	// �|���S���`�施�ߔ��s
	g_pContext->DrawIndexed(NUM_INDEX, 0, 0);
}
