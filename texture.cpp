#include "texture.h"
#include <string>
#include "direct3d.h"
#include "DirectXTex.h"
#include <sstream>
#include <Windows.h>
using namespace DirectX;
static constexpr int TEXTURE_MAX =1024;//テクスチャの最大数

struct Texture
{
	std::wstring filename;//実ファイルパス
	unsigned int width;
	unsigned int height;

	ID3D11ShaderResourceView* pTexture = nullptr;
};

static Texture g_Textures[TEXTURE_MAX];

// デバイス参照
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
	//既に読み込まれているかチェック
	for (int i =0; i < TEXTURE_MAX; i++)
	{
		if (!g_Textures[i].pTexture) continue;

		if (g_Textures[i].filename == pFileName)
		{
			return i;
		}
	}

	// 空きスロットへ読み込み
	for (int i =0; i < TEXTURE_MAX; i++)
	{
		if (g_Textures[i].pTexture) continue;

		// テクスチャ読み込み
		TexMetadata metadata;
		ScratchImage image;

		HRESULT hr = LoadFromWICFile(pFileName, WIC_FLAGS_NONE, &metadata, image);
		if (FAILED(hr)) {
			// 現在のワーキングディレクトリを取得してメッセージに含める
			wchar_t curDir[MAX_PATH] = {};
			GetCurrentDirectoryW(MAX_PATH, curDir);
			std::wstringstream ss;
			ss << L"Texture load failed: \n" << pFileName << L"\n";
			ss << L"Working directory: \n" << curDir << L"\n";
			ss << L"HRESULT:0x" << std::hex << hr;
			std::wstring msg = ss.str();
			MessageBoxW(nullptr, msg.c_str(), L"Texture Load Error", MB_OK | MB_ICONERROR);
			// 出力デバッグにも表示
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
			MessageBox(nullptr, "画像の読み込みに失敗しました", "エラー", MB_OK | MB_ICONERROR);
			break;
		}

		// ファイル名を保存
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
		// エラーの場合はここにデバッグ表示を入れる
		return;
	}

	// テクスチャ設定
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
