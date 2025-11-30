/*==============================================================================

   �O���b�h�̕`�� [grid.cpp]
														 Author : sumi rintarou
														 Date   : 2025/10/21
--------------------------------------------------------------------------------

==============================================================================*/
#include "../include/grid.h"
#include <d3d11.h>
#include "../include/direct3d.h"
#include <DirectXMath.h>
using namespace DirectX;
#include "../include/debug_ostream.h"
#include "../include/shader3d.h"
#include "../include/texture.h"

static ID3D11Buffer* g_pVertexBuffer = nullptr; // ���_�o�b�t�@
//static ID3D11ShaderResourceView* g_pTexture = nullptr;//2��3���Ƒ��₷�Ƃ��ɂ͂�������₵�Ė��O��ς���(1�ɂ�1��)

// ���ӁI�������ŊO������ݒ肳�����́BRelease�s�v�B
static ID3D11Device* g_pDevice = nullptr;
static ID3D11DeviceContext* g_pContext = nullptr;

// ���_��
static int g_Num_Vertex = 0; // �������X�g�̒��_��

// ���_�\����
struct Vertex3d
{
	XMFLOAT3 position; // ���_���W
	XMFLOAT4 color;    // �F
};

static int g_texId = -1;


void Grid_Initialize(int x_count, int z_count, float size)
{
	// �f�o�C�X�ƃf�o�C�X�R���e�L�X�g�̕ۑ�
	g_pDevice = Direct3D_GetDevice();
	g_pContext = Direct3D_GetContext();

	// ���_�����Z�o����
	const int X_V_COUNT = x_count + 1; // X�����̒��_��
	const int Z_V_COUNT = z_count + 1; // Z�����̒��_��
	g_Num_Vertex = (X_V_COUNT + Z_V_COUNT) * 2; // �������X�g�̒��_��

	// ���_�o�b�t�@����
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(Vertex3d) * g_Num_Vertex;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;// �A�N�Z�X�t���O��0�ɂ��邱�Ƃ�CPU�ɃA�N�Z�X�ł��Ȃ�����

	auto* pV = new Vertex3d[g_Num_Vertex];

	// �O���b�h���̒��_���̏�������
	float start_x = x_count * size * -0.5f;
	float end_x = x_count * size * 0.5f;
	float start_z = z_count * size * -0.5f;
	float end_z = z_count * size * 0.5f;
	int index = 0;

	for (int x = 0; x < X_V_COUNT; x++)
	{
		XMFLOAT4 color = x == X_V_COUNT / 2 ? XMFLOAT4{ 0.0f,0.0f,1.0f,1.0f } : XMFLOAT4{ 0.0f,1.0f,0.0f,1.0f };
		pV[index].position = { start_x + size * x,0.0f,start_z };
		pV[index].color = color;
		pV[index + 1].position = { start_x + size * x,0.0f,end_z };
		pV[index + 1].color = color;
		index += 2;
	}

	for (int z = 0; z < Z_V_COUNT; z++)
	{
		XMFLOAT4 color = z == Z_V_COUNT / 2 ? XMFLOAT4{ 0.0f,0.0f,1.0f,1.0f } : XMFLOAT4{ 0.0f,1.0f,0.0f,1.0f };
		pV[index].position = { start_x,0.0f,start_z + size * z };
		pV[index].color = color;
		pV[index + 1].position = { end_x,0.0f,start_z + size * z };
		pV[index + 1].color = color;
		index += 2;
	}


	// ���_�o�b�t�@�֗������ރf�[�^�̐ݒ�
	D3D11_SUBRESOURCE_DATA sd{};
	sd.pSysMem = pV;

	g_pDevice->CreateBuffer(&bd, &sd, &g_pVertexBuffer);

	delete[] pV;

	g_texId = Texture_Load(L"texture/siro.png");
}

void Grid_Finalize()
{
	SAFE_RELEASE(g_pVertexBuffer);
}

void Grid_Update(double elapsed_time)
{

}

void Grid_Draw()
{
	// �V�F�[�_�[��`��p�C�v���C���ɐݒ�
	Shader3d_Begin();

	// ���_�o�b�t�@��`��p�C�v���C���ɐݒ�
	UINT stride = sizeof(Vertex3d);
	UINT offset = 0;
	g_pContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);

	// ���_�V�F�[�_�[��world�ϊ��s���ݒ�@�P�ʍs��(1���|����Ƃ������Ƃŉ����l��ς��Ȃ��Ƃ�������)
	Shader3d_SetWorldMatrix(XMMatrixIdentity());

	// �v���~�e�B�u�g�|���W�ݒ�
	g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	// �e�N�X�`���̐ݒ�(��)
	Texture_SetTexture(g_texId);

	// �|���S���`�施�ߔ��s
	g_pContext->Draw(g_Num_Vertex, 0);
}



///*==============================================================================
//
//   �O���b�h�`�� [grid.cpp]
//                                                         Author : sumi rintarou
//                                                         Date   : 2025/10/21
//--------------------------------------------------------------------------------
//
//==============================================================================*/
//#include "../include/grid.h"
//#include <d3d11.h>
//#include "../include/direct3d.h"
//#include <DirectXMath.h>
//using namespace DirectX;
//#include "../include/debug_ostream.h"
//#include "../include/shader3d.h"
//#include <vector>
//
//// ���I�����ɕύX: ��`���폜���ē��I�ɒ��_�����Ǘ�
//static ID3D11Buffer* g_pVertexBuffer = nullptr; // ���_�o�b�t�@
//
//// ���ӁI�������ŊO������ݒ肳�����́BRelease�s�v�B
//static ID3D11Device* g_pDevice = nullptr;
//static ID3D11DeviceContext* g_pContext = nullptr;
//
//// ���_�\����
//struct Vertex3d
//{
//    XMFLOAT3 position; // ���_���W
//    XMFLOAT4 color;    // �F
//};
//
//// ���I�ɍ쐬����钸�_����ێ�
//static UINT g_NumVertex = 0;
//
//void Grid_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
//{
//    // �p�����[�^: �O���b�h�̃Z�����i1�ӂ�����̃}�X���j�� 1 �}�X�̑傫��
//    constexpr int GRID_CELLS = 10;    // 10x10 �̃}�X��
//    constexpr float CELL_SIZE = 1.0f; // 1 �}�X������̒����i�K�v�ɉ����ĕύX�j
//
//    // �f�o�C�X�ƃf�o�C�X�R���e�L�X�g�̃`�F�b�N
//    if (!pDevice || !pContext) {
//        hal::dout << "Grid_Initialize() : �^����ꂽ�f�o�C�X���R���e�L�X�g���s���ł�" << std::endl;
//        return;
//    }
//
//    // �f�o�C�X�ƃR���e�L�X�g��ۑ�
//    g_pDevice = pDevice;
//    g_pContext = pContext;
//
//    // ���������_�ɂ��邽�߂̃I�t�Z�b�g
//    float half = GRID_CELLS * CELL_SIZE * 0.5f;
//
//    // ���_���X�g���쐬 (�e������ GRID_CELLS+1 �{�̐��A1 �{������ 2 ���_)
//    std::vector<Vertex3d> vertices;
//    vertices.reserve((GRID_CELLS + 1) * 4);
//
//    XMFLOAT4 lineColor = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f); // �O���b�h�F�i�D�j
//
//    // Z �������ɕ��Ԑ��iX ���ɉ������F�قȂ� Z �̒l�� X ��L�΂��j
//    for (int i = 0; i <= GRID_CELLS; ++i) {
//        float z = -half + i * CELL_SIZE;
//        vertices.push_back({ XMFLOAT3(-half, 0.0f, z), lineColor });
//        vertices.push_back({ XMFLOAT3(half, 0.0f, z), lineColor });
//    }
//
//    // X �������ɕ��Ԑ��iZ ���ɉ������F�قȂ� X �̒l�� Z ��L�΂��j
//    for (int i = 0; i <= GRID_CELLS; ++i) {
//        float x = -half + i * CELL_SIZE;
//        vertices.push_back({ XMFLOAT3(x, 0.0f, -half), lineColor });
//        vertices.push_back({ XMFLOAT3(x, 0.0f,  half), lineColor });
//    }
//
//    // ���_�o�b�t�@�쐬
//    g_NumVertex = static_cast<UINT>(vertices.size());
//
//    D3D11_BUFFER_DESC bd = {};
//    bd.Usage = D3D11_USAGE_DEFAULT;
//    bd.ByteWidth = sizeof(Vertex3d) * g_NumVertex;
//    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
//    bd.CPUAccessFlags = 0;
//
//    D3D11_SUBRESOURCE_DATA sd{};
//    sd.pSysMem = vertices.data();
//
//    if (FAILED(g_pDevice->CreateBuffer(&bd, &sd, &g_pVertexBuffer))) {
//        hal::dout << "Grid_Initialize() : ���_�o�b�t�@�쐬�Ɏ��s���܂���" << std::endl;
//        g_pVertexBuffer = nullptr;
//        g_NumVertex = 0;
//    }
//}
//
//void Grid_Finalize()
//{
//    SAFE_RELEASE(g_pVertexBuffer);
//    g_NumVertex = 0;
//}
//
//void Grid_Update(double elapsed_time)
//{
//}
//
//void Grid_Draw()
//{
//    if (!g_pVertexBuffer || g_NumVertex == 0) return;
//
//    // �V�F�[�_�[��`��p�C�v���C���ɐݒ�
//    Shader3d_Begin();
//
//    // ���_�o�b�t�@��`��p�C�v���C���ɐݒ�
//    UINT stride = sizeof(Vertex3d);
//    UINT offset = 0;
//    g_pContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);
//
//    // ���[���h�s��i�P�ʍs��j
//    Shader3d_SetWorldMatrix(XMMatrixIdentity());
//
//    // �r���[�ݒ�i�K�v�ɉ����ĕύX�j
//    XMVECTOR eye = { 3.0f, 5.0f, -10.0f, 0.0f };
//    XMVECTOR target = { 0.0f, 0.0f, 0.0f, 0.0f };
//    XMVECTOR up = { 0.0f, 1.0f, 0.0f, 0.0f };
//    XMMATRIX mtxView = XMMatrixLookAtLH(eye, target, up);
//    Shader3d_SetViewMatrix(mtxView);
//
//    // �v���W�F�N�V����
//    float w = static_cast<float>(Direct3D_GetBackBufferWidth());
//    float h = static_cast<float>(Direct3D_GetBackBufferHeight());
//    XMMATRIX mtxProj = XMMatrixPerspectiveFovLH(1.0f, w / h, 0.1f, 1000.0f);
//    Shader3d_SetProjectionMatrix(mtxProj);
//
//    // ���`��
//    g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
//    g_pContext->Draw(g_NumVertex, 0);
//}
