#pragma once
#include <DirectXMath.h>
#include <d3d11.h>
#include "collision.h"

// デバッグ用AABB描画モジュール
namespace DebugAABB
{
    // 初期化と終了処理
    void Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    void Finalize();
    
    // AABBをワイヤーフレーム描画
    void DrawAABB(const AABB& aabb, const DirectX::XMFLOAT4& color = { 0.0f, 1.0f, 0.0f, 1.0f });
    
    // 複数のAABBを描画
    void DrawAABBs(const AABB* aabbs, size_t count, const DirectX::XMFLOAT4& color = { 0.0f, 1.0f, 0.0f, 1.0f });
    
    // デバッグ表示のOn/Off
    void SetEnabled(bool enabled);
    bool IsEnabled();
}
