#include <d3d11.h>
#include "../include/direct3d.h"
#include "../include/debug_ostream.h"

//#pragma comment(lib, "d3d11.lib")	// �O���V���{���������N���邽�߂̋L�q(Vi�ł����g���Ȃ�)
// #pragma comment(lib, "dxgi.lib")	// DXGI���C�u�����̃����N�i�K�v�ɉ����āj
#if defined(DEBUG) || defined(_DEBUG)
#pragma comment(lib, "DirectXTex_Debug.lib")	
#else
#pragma comment(lib, "DirectXTex_Release.lib")	
#endif

// �C���^�[�t�F�[�X(3��)
static ID3D11Device* g_pDevice = nullptr;					// �f�o�C�X
static ID3D11DeviceContext* g_pDeviceContext = nullptr;		// �R���e�L�X�g
static IDXGISwapChain* g_pSwapChain = nullptr;				// �X���b�v�`�F�[��

static ID3D11BlendState* g_BlendStateMultiply = nullptr;		// �u�����h�X�e�[�g
static ID3D11BlendState* g_BlendStateAdd = nullptr;		// �u�����h�X�e�[�g

static ID3D11DepthStencilState* g_pDepthStencilStateDepthDisable = nullptr;
static ID3D11DepthStencilState* g_pDepthStencilStateDepthEnable = nullptr;

// �o�b�N�o�b�t�@�̊֘A
static ID3D11RenderTargetView* g_pRenderTargetView = nullptr;	// �����_�[�^�[�Q�b�g�r���[
static ID3D11Texture2D* g_pDepthStencilBuffer = nullptr;		// �f�v�X�X�e���V���o�b�t�@
static ID3D11DepthStencilView* g_pDepthStencilView = nullptr;	// �f�v�X�X�e���V���r���[

static D3D11_TEXTURE2D_DESC g_BackBufferDesc{};					// �o�b�N�o�b�t�@
//static D3D11_VIEWPORT g_Viewport{};							// �r���[�|�[�g

static D3D11_VIEWPORT g_Viewport[2]{};					// �r���[�|�[�g

static bool configureBackBuffer();	// �o�b�N�o�b�t�@�̐ݒ�
static void releaseBackBuffer();	// �o�b�N�o�b�t�@�̉��


bool Direct3D_Initialize(HWND hWnd)
{
	//�f�o�C�X�A�X���b�v�`�F�[���A�R���e�L�X�g����
	DXGI_SWAP_CHAIN_DESC swap_chain_desc{};
	swap_chain_desc.Windowed = TRUE;	// �E�B���h�E���[�h
	swap_chain_desc.BufferCount = 2;	// �o�b�t�@�̐�
	// swap_chain_desc.BufferDesc.Width = 0;
	// swap_chain_desc.BufferDesc.Heicht = 0;
	// ���E�B���h�E�T�C�Y�ɍ��킹�Ď����I�ɐݒ肳���
	swap_chain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;		// �o�b�t�@�̃t�H�[�}�b�g
	swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;		// �o�b�t�@�̎g�p���@
	swap_chain_desc.SampleDesc.Count = 1;								// �}���`�T���v�����O�̐�
	swap_chain_desc.SampleDesc.Quality = 0;								// �}���`�T���v�����O�̕i��
	//swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;	// �X���b�v�`�F�[���̌��� ��������R�����g�A�E�g�����FPS���オ��void Direct3D_Present()��(1.0)�ɂ��� //�x���`�}�[�N�Œ������
	swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_SEQUENTIAL;
	swap_chain_desc.OutputWindow = hWnd;								// �E�B���h�E�n���h��

	UINT device_flags = 0;
#if defined (DEBUG) || defined(_DEBUG)
	device_flags |= D3D11_CREATE_DEVICE_DEBUG;	// �f�o�b�O�p�t���O
#endif
	D3D_FEATURE_LEVEL levels[] = {
		D3D_FEATURE_LEVEL_11_1,//�n�[�h�E�F�A�̃��x���ɂ���ĕς��̂Ŕ��f����
		D3D_FEATURE_LEVEL_11_0
	};

	D3D_FEATURE_LEVEL feature_level = D3D_FEATURE_LEVEL_11_0;

	HRESULT hr = D3D11CreateDeviceAndSwapChain(
		nullptr,						// �A�_�v�^
		D3D_DRIVER_TYPE_HARDWARE,		// �h���C�o�̎��
		nullptr,						// �\�t�g�E�F�A�h���C�o
		device_flags,					// �t���O
		levels,							// �@�\���x��
		ARRAYSIZE(levels),				// �@�\���x���̐�
		D3D11_SDK_VERSION,				// SDK�o�[�W����
		&swap_chain_desc,				// �X���b�v�`�F�[���̐ݒ�
		&g_pSwapChain,					// �X���b�v�`�F�[��
		&g_pDevice,						// �f�o�C�X
		&feature_level,					// �@�\���x��
		&g_pDeviceContext				// �R���e�L�X�g
	);

	if (FAILED(hr))
	{
		MessageBox(hWnd, "Direct3D�̏������Ɏ��s���܂���", "�G���[", MB_OK);
		return false;
	}

	configureBackBuffer();
	if (!configureBackBuffer())
	{
		MessageBox(hWnd, "�o�b�N�o�b�t�@�̐ݒ�Ɏ��s���܂���", "�G���[", MB_OK);
		return false;
	}

	return true;
}

void Direct3D_Finalize()
{
	SAFE_RELEASE(g_pDepthStencilStateDepthEnable);
	SAFE_RELEASE(g_pDepthStencilStateDepthDisable);
	SAFE_RELEASE(g_BlendStateMultiply);
	SAFE_RELEASE(g_BlendStateAdd);

	releaseBackBuffer();

	SAFE_RELEASE(g_pSwapChain);
	SAFE_RELEASE(g_pDeviceContext);
	SAFE_RELEASE(g_pDevice);
}

void Direct3D_Clear()
{
	float clear_color[4] = { 0.1f, 0.2f, 0.4f, 1.0f }; // RGBA �w�i�̐F
	g_pDeviceContext->ClearRenderTargetView(g_pRenderTargetView, clear_color);//�N���A���Ă��������Ɩ��߂��Ă���
	g_pDeviceContext->ClearDepthStencilView(g_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);//���̐F�ŃN���A���Ă��������Ɩ��߂��Ă���@ 1.0f�[���A0�ɂ���ƕ`�悳��Ȃ�
	//g_pDeviceContext�̓R�}���h �ǂ����ɂ��ߑ����Ă���

	//�����_�[�^�[�Q�b�g�r���[�ƃf�v�X�X�e���V���r���[�̐ݒ�
	g_pDeviceContext->OMSetRenderTargets(1, &g_pRenderTargetView, g_pDepthStencilView);
}

void Direct3D_Present()
{
	// �X���b�v�`�F�[���̕\��
	g_pSwapChain->Present(1, 0);
}

unsigned int Direct3D_GetBackBufferWidth()
{
	return g_BackBufferDesc.Width;
}

unsigned int Direct3D_GetBackBufferHeight()
{
	return g_BackBufferDesc.Height;
}

ID3D11Device* Direct3D_GetDevice()
{
	return g_pDevice;
}

ID3D11DeviceContext* Direct3D_GetContext()
{
	return g_pDeviceContext;
}

// ���V�їp
void SetViewport(int n)
{
	g_pDeviceContext->RSSetViewports(1, &g_Viewport[n]);
}

void Direct3D_SetAlphaBlend(AlphaBlendMode blend)
{
	float blend_factor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	ID3D11BlendState* arg{};

	switch (blend)
	{
	case BLEND_TRANSPARENT:
		arg = g_BlendStateMultiply;
		break;
	case BLEND_ADD:
		arg = g_BlendStateAdd;
		break;
	}

	g_pDeviceContext->OMSetBlendState(arg, blend_factor, 0xffffffff);// �ݒ�
}

void Direct3D_SetDepthTest(bool bEnable)
{
	auto state = bEnable ? g_pDepthStencilStateDepthEnable : g_pDepthStencilStateDepthDisable;

	g_pDeviceContext->OMSetDepthStencilState(state, NULL);// �ݒ�
}

bool configureBackBuffer()
{
	HRESULT hr;

	ID3D11Texture2D* back_buffer_pointer = nullptr;

	// �o�b�N�o�b�t�@�̎擾
	hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&back_buffer_pointer);

	if (FAILED(hr)) {
		hal::dout << "�o�b�N�o�b�t�@�̎擾�Ɏ��s���܂���" << std::endl;
		return false;
	}

	// �o�b�N�o�b�t�@�̃����_�[�^�[�Q�b�g�r���[�̐���
	hr = g_pDevice->CreateRenderTargetView(back_buffer_pointer, nullptr, &g_pRenderTargetView);

	if (FAILED(hr)) {
		back_buffer_pointer->Release();
		hal::dout << "�o�b�N�o�b�t�@�̃����_�[�^�[�Q�b�g�r���[�̐����Ɏ��s���܂���" << std::endl;
		return false;
	}

	// �o�b�N�o�b�t�@�̏�ԁi���j���擾
	back_buffer_pointer->GetDesc(&g_BackBufferDesc);

	back_buffer_pointer->Release(); // �o�b�N�o�b�t�@�̃|�C���^�͕s�v�Ȃ̂ŉ��

	// �f�v�X�X�e���V���o�b�t�@�̐���
	D3D11_TEXTURE2D_DESC depth_stencil_desc{};
	depth_stencil_desc.Width = g_BackBufferDesc.Width;
	depth_stencil_desc.Height = g_BackBufferDesc.Height;
	depth_stencil_desc.MipLevels = 1;
	depth_stencil_desc.ArraySize = 1;
	depth_stencil_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depth_stencil_desc.SampleDesc.Count = 1;
	depth_stencil_desc.SampleDesc.Quality = 0;
	depth_stencil_desc.Usage = D3D11_USAGE_DEFAULT;
	depth_stencil_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depth_stencil_desc.CPUAccessFlags = 0;
	depth_stencil_desc.MiscFlags = 0;
	hr = g_pDevice->CreateTexture2D(&depth_stencil_desc, nullptr, &g_pDepthStencilBuffer);

	if (FAILED(hr)) {
		hal::dout << "�f�v�X�X�e���V���o�b�t�@�̐����Ɏ��s���܂���" << std::endl;
		return false;
	}

	// �f�v�X�X�e���V���r���[�̐���
	D3D11_DEPTH_STENCIL_VIEW_DESC depth_stencil_view_desc{};
	depth_stencil_view_desc.Format = depth_stencil_desc.Format;
	depth_stencil_view_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depth_stencil_view_desc.Texture2D.MipSlice = 0;
	depth_stencil_view_desc.Flags = 0;
	hr = g_pDevice->CreateDepthStencilView(g_pDepthStencilBuffer, &depth_stencil_view_desc, &g_pDepthStencilView);

	if (FAILED(hr)) {
		hal::dout << "�f�v�X�X�e���V���r���[�̐����Ɏ��s���܂���" << std::endl;
		return false;
	}

	// �r���[�|�[�g�̐ݒ�
	g_Viewport[0].TopLeftX = 0.0f;
	g_Viewport[0].TopLeftY = 0.0f;
	g_Viewport[0].Width = (FLOAT)g_BackBufferDesc.Width;
	g_Viewport[0].Height = (FLOAT)g_BackBufferDesc.Height;
	g_Viewport[0].MinDepth = 0.0f;
	g_Viewport[0].MaxDepth = 1.0f;


	////���V�їp(�����ƉE���ŕ�����)
	//g_Viewport[0].TopLeftX = 0.0f;
	//g_Viewport[0].TopLeftY = 0.0f;
	//g_Viewport[0].Width = (FLOAT)g_BackBufferDesc.Width * 0.5;
	//g_Viewport[0].Height = (FLOAT)g_BackBufferDesc.Height;
	//g_Viewport[0].MinDepth = 0.0f;
	//g_Viewport[0].MaxDepth = 1.0f;

	//g_Viewport[1].TopLeftX = (FLOAT)g_BackBufferDesc.Width * 0.5;
	//g_Viewport[1].TopLeftY = 0.0f;
	//g_Viewport[1].Width = (FLOAT)g_BackBufferDesc.Width * 0.5;
	//g_Viewport[1].Height = (FLOAT)g_BackBufferDesc.Height;
	//g_Viewport[1].MinDepth = 0.0f;
	//g_Viewport[1].MaxDepth = 1.0f;

	g_pDeviceContext->RSSetViewports(1, &g_Viewport[0]);

	// �u�����h�X�e�[�g�ݒ�
	D3D11_BLEND_DESC bd = {};
	bd.AlphaToCoverageEnable = FALSE;
	bd.IndependentBlendEnable = FALSE;
	bd.RenderTarget[0].BlendEnable = TRUE;
	bd.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	bd.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	bd.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	bd.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	bd.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	bd.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	bd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	g_pDevice->CreateBlendState(&bd, &g_BlendStateMultiply);// �쐬

	// ���Z�����p�̍쐬
	bd.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	g_pDevice->CreateBlendState(&bd, &g_BlendStateAdd);// �쐬

	Direct3D_SetAlphaBlend(BLEND_TRANSPARENT);// �f�t�H���g

	float blend_factor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	g_pDeviceContext->OMSetBlendState(g_BlendStateMultiply, blend_factor, 0xffffffff);// �ݒ�

	//�[�x�X�e���V���X�e�[�g�ݒ�(���s�����)
	D3D11_DEPTH_STENCIL_DESC dsd = {};
	dsd.DepthFunc = D3D11_COMPARISON_LESS;
	dsd.StencilEnable = FALSE;
	//dsd.DepthEnable = FALSE;	//�����ɂ���
	//dsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;

	//g_pDevice->CreateDepthStencilState(&dsd, &g_pDepthStencilStateDepthDisable);

	dsd.DepthEnable = TRUE;
	dsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;

	g_pDevice->CreateDepthStencilState(&dsd, &g_pDepthStencilStateDepthDisable);

	g_pDeviceContext->OMSetDepthStencilState(g_pDepthStencilStateDepthDisable, NULL);


	return true;
}

void releaseBackBuffer()
{
	if (g_pRenderTargetView) {
		g_pRenderTargetView->Release();
		g_pRenderTargetView = nullptr;
	}

	if (g_pDepthStencilBuffer) {
		g_pDepthStencilBuffer->Release();
		g_pDepthStencilBuffer = nullptr;
	}

	if (g_pDepthStencilView) {
		g_pDepthStencilView->Release();
		g_pDepthStencilView = nullptr;
	}
}

