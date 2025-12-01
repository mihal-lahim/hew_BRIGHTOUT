#include "DebugAABB.h"
#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>

using namespace DirectX;

namespace DebugAABB
{
    // 頂点構造体
    struct SimpleVertex
    {
        XMFLOAT3 position;
        XMFLOAT4 color;
    };

    // グローバル変数
    static ID3D11Device* g_pDevice = nullptr;
    static ID3D11DeviceContext* g_pContext = nullptr;
    static ID3D11InputLayout* g_pInputLayout = nullptr;
    static ID3D11VertexShader* g_pVertexShader = nullptr;
    static ID3D11PixelShader* g_pPixelShader = nullptr;
    static ID3D11Buffer* g_pVertexBuffer = nullptr;
    static ID3D11Buffer* g_pIndexBuffer = nullptr;
    static ID3D11Buffer* g_pConstantBuffer = nullptr;
    static bool g_bEnabled = true;
    static const UINT VERTEX_BUFFER_SIZE = 1000;
    static const UINT INDEX_BUFFER_SIZE = 2000;

    void Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    {
        g_pDevice = pDevice;
        g_pContext = pContext;

        // 簡単な頂点シェーダー
        const char* vsCode = R"(
            cbuffer TransformBuffer : register(b0)
            {
                float4x4 view;
                float4x4 projection;
            };

            struct VS_INPUT
            {
                float3 position : POSITION;
                float4 color : COLOR0;
            };

            struct PS_INPUT
            {
                float4 position : SV_POSITION;
                float4 color : COLOR0;
            };

            PS_INPUT main(VS_INPUT input)
            {
                PS_INPUT output;
                float4 pos = float4(input.position, 1.0f);
                pos = mul(pos, view);
                pos = mul(pos, projection);
                output.position = pos;
                output.color = input.color;
                return output;
            }
        )";

        // 簡単なピクセルシェーダー
        const char* psCode = R"(
            struct PS_INPUT
            {
                float4 position : SV_POSITION;
                float4 color : COLOR0;
            };

            float4 main(PS_INPUT input) : SV_TARGET
            {
                return input.color;
            }
        )";

        // 入力レイアウトの定義
        D3D11_INPUT_ELEMENT_DESC layout[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
        };

        // 頂点バッファの作成
        D3D11_BUFFER_DESC bufferDesc;
        ZeroMemory(&bufferDesc, sizeof(bufferDesc));
        bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
        bufferDesc.ByteWidth = sizeof(SimpleVertex) * VERTEX_BUFFER_SIZE;
        bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        pDevice->CreateBuffer(&bufferDesc, nullptr, &g_pVertexBuffer);

        // インデックスバッファの作成
        bufferDesc.ByteWidth = sizeof(UINT) * INDEX_BUFFER_SIZE;
        bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        pDevice->CreateBuffer(&bufferDesc, nullptr, &g_pIndexBuffer);

        // 定数バッファの作成
        bufferDesc.ByteWidth = sizeof(XMMATRIX) * 2;
        bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        pDevice->CreateBuffer(&bufferDesc, nullptr, &g_pConstantBuffer);
    }

    void Finalize()
    {
        if (g_pVertexBuffer) g_pVertexBuffer->Release();
        if (g_pIndexBuffer) g_pIndexBuffer->Release();
        if (g_pConstantBuffer) g_pConstantBuffer->Release();
        if (g_pInputLayout) g_pInputLayout->Release();
        if (g_pVertexShader) g_pVertexShader->Release();
        if (g_pPixelShader) g_pPixelShader->Release();
    }

    void DrawAABB(const AABB& aabb, const XMFLOAT4& color)
    {
        if (!g_bEnabled || !g_pContext)
            return;

        // AABBの8つの頂点を作成
        XMFLOAT3 min = aabb.GetMin();
        XMFLOAT3 max = aabb.GetMax();

        SimpleVertex vertices[8] =
        {
            // 最小コーナー
            { XMFLOAT3(min.x, min.y, min.z), color },  // 0
            { XMFLOAT3(max.x, min.y, min.z), color },  // 1
            { XMFLOAT3(max.x, max.y, min.z), color },  // 2
            { XMFLOAT3(min.x, max.y, min.z), color },  // 3
            // 最大コーナー
            { XMFLOAT3(min.x, min.y, max.z), color },  // 4
            { XMFLOAT3(max.x, min.y, max.z), color },  // 5
            { XMFLOAT3(max.x, max.y, max.z), color },  // 6
            { XMFLOAT3(min.x, max.y, max.z), color }   // 7
        };

        // ワイヤーフレームのインデックス（12本の辺）
        UINT indices[24] =
        {
            0, 1, 1, 2, 2, 3, 3, 0,  // 手前の面
            4, 5, 5, 6, 6, 7, 7, 4,  // 奥の面
            0, 4, 1, 5, 2, 6, 3, 7   // 繋ぎの辺
        };

        // 頂点バッファにデータをコピー
        if (g_pVertexBuffer)
        {
            D3D11_MAPPED_SUBRESOURCE mappedResource;
            if (SUCCEEDED(g_pContext->Map(g_pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
            {
                CopyMemory(mappedResource.pData, vertices, sizeof(vertices));
                g_pContext->Unmap(g_pVertexBuffer, 0);
            }
        }

        // インデックスバッファにデータをコピー
        if (g_pIndexBuffer)
        {
            D3D11_MAPPED_SUBRESOURCE mappedResource;
            if (SUCCEEDED(g_pContext->Map(g_pIndexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
            {
                CopyMemory(mappedResource.pData, indices, sizeof(indices));
                g_pContext->Unmap(g_pIndexBuffer, 0);
            }
        }

        // 頂点バッファを設定
        UINT stride = sizeof(SimpleVertex);
        UINT offset = 0;
        g_pContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);
        g_pContext->IASetIndexBuffer(g_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

        // ラインストリップで描画
        g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
        g_pContext->DrawIndexed(24, 0, 0);
    }

    void DrawAABBs(const AABB* aabbs, size_t count, const XMFLOAT4& color)
    {
        if (!g_bEnabled || !aabbs)
            return;

        for (size_t i = 0; i < count; ++i)
        {
            DrawAABB(aabbs[i], color);
        }
    }

    void SetEnabled(bool enabled)
    {
        g_bEnabled = enabled;
    }

    bool IsEnabled()
    {
        return g_bEnabled;
    }
}
