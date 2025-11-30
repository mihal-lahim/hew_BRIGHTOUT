#include "../include/texture.h"
#include <string>
#include "../include/direct3d.h"
#include "../include/DirectXTex.h"
#include <sstream>
#include <Windows.h>
using namespace DirectX;
static constexpr int TEXTURE_MAX =1024;//�e�N�X�`���̍ő吔

struct Texture
{
	std::wstring filename;//���t�@�C���p�X
	unsigned int width;
	unsigned int height;

	ID3D11ShaderResourceView* pTexture = nullptr;
};

static Texture g_Textures[TEXTURE_MAX];

// �f�o�C�X�Q��
static ID3D11Device* g_pDevice = nullptr;
static ID3D11DeviceContext* g_pContext = nullptr;

void Texture_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	g_pDevice = pDevice;
	g_pContext = pContext;
}

void Texture_Finalize()
{
	Texture_AllRelease();
}

int Texture_Load(const wchar_t* pFileName, bool bMipMap)
{
	//���ɓǂݍ��܂�Ă��邩�`�F�b�N
	for (int i =0; i < TEXTURE_MAX; i++)
	{
		if (!g_Textures[i].pTexture) continue;

		if (g_Textures[i].filename == pFileName)
		{
			return i;
		}
	}

	// �󂫃X���b�g�֓ǂݍ���
	for (int i =0; i < TEXTURE_MAX; i++)
	{
		if (g_Textures[i].pTexture) continue;

		// �e�N�X�`���ǂݍ���
		TexMetadata metadata;
		ScratchImage image;

		HRESULT hr = LoadFromWICFile(pFileName, WIC_FLAGS_NONE, &metadata, image);
		if (FAILED(hr)) {
			// ���݂̃��[�L���O�f�B���N�g�����擾���ă��b�Z�[�W�Ɋ܂߂�
			wchar_t curDir[MAX_PATH] = {};
			GetCurrentDirectoryW(MAX_PATH, curDir);
			std::wstringstream ss;
			ss << L"Texture load failed: \n" << pFileName << L"\n";
			ss << L"Working directory: \n" << curDir << L"\n";
			ss << L"HRESULT:0x" << std::hex << hr;
			std::wstring msg = ss.str();
			MessageBoxW(nullptr, msg.c_str(), L"Texture Load Error", MB_OK | MB_ICONERROR);
			// �o�̓f�o�b�O�ɂ��\��
			OutputDebugStringW(msg.c_str());
			return -1;
		}

		g_Textures[i].width = (unsigned int)metadata.width;
		g_Textures[i].height = (unsigned int)metadata.height;

		if (bMipMap)
		{
			ScratchImage mipChain;
			GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), TEX_FILTER_DEFAULT,0, mipChain);

			image = std::move(mipChain);
			metadata = image.GetMetadata();
		}

		hr = CreateShaderResourceView(g_pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Textures[i].pTexture);

		if (FAILED(hr))
		{
			MessageBox(nullptr, "�摜�̓ǂݍ��݂Ɏ��s���܂���", "�G���[", MB_OK | MB_ICONERROR);
			break;
		}

		// �t�@�C������ۑ�
		g_Textures[i].filename = pFileName;

		return i;
	}
	return -1;
}

void Texture_AllRelease()
{
	for (Texture& t : g_Textures)
	{
		SAFE_RELEASE(t.pTexture);
	}
}

void Texture_SetTexture(int texture_id)
{
	if (texture_id <0)
	{
		// �G���[�̏ꍇ�͂����Ƀf�o�b�O�\��������
		return;
	}

	// �e�N�X�`���ݒ�
	g_pContext->PSSetShaderResources(0,1, &g_Textures[texture_id].pTexture);
}

DirectX::XMUINT2 Texture_GetSize(int texture_id)
{
	return { g_Textures[texture_id].width, g_Textures[texture_id].height };
}

const unsigned int& Texture_GetWidth(int texture_id)
{
	return g_Textures[texture_id].width;
}

const unsigned int& Texture_GetHeight(int texture_id)
{
	return g_Textures[texture_id].height;
}
