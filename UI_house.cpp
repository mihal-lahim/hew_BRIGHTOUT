//////////////////////////////
// UI_house.cpp
// 家の状態表示UI
//Author: Game Dev
//Date: 2025/01/XX
//////////////////////////////

#include "UI_house.h"
#include "house.h"
#include "debug_text.h"
#include "direct3d.h"
#include <cstdio>
#include <algorithm>
#include <sstream>

UIHouse::UIHouse()
{
}

UIHouse::~UIHouse()
{
    if (m_debugText) {
        delete m_debugText;
        m_debugText = nullptr;
    }
}

void UIHouse::Initialize()
{
    // デバッグテキスト初期化
    if (!m_debugText) {
        ID3D11Device* pDevice = Direct3D_GetDevice();
        ID3D11DeviceContext* pContext = Direct3D_GetContext();
        UINT width = Direct3D_GetBackBufferWidth();
        UINT height = Direct3D_GetBackBufferHeight();
        
        // 画面右上に表示（スクリーン座標）
        float offsetX = width - 420.0f;
        float offsetY = 10.0f;
        
        try {
            m_debugText = new hal::DebugText(
                pDevice, pContext,
                L"texture/consolab_ascii_512.png",
                width, height,
                offsetX, offsetY,
                15,   // maxLine（余裕を持たせる）
                60    // maxCharactersPerLine
            );
        } catch (...) {
            // テクスチャロード失敗時のフォールバック
            m_debugText = nullptr;
        }
    }
    
    m_updateTimer = 0.0f;
}

void UIHouse::Update(double elapsedTime)
{
    //m_updateTimer += elapsedTime;
    //
    //// 定期的にテキストを更新
    //if (m_updateTimer >= TEXT_UPDATE_INTERVAL) {
    //    m_updateTimer = 0.0f;
    //    
    //    // テキスト生成
    //    if (m_debugText) {
    //        m_debugText->Clear();
    //        
    //        // 家がない場合
    //        if (m_houses.empty()) {
    //            m_debugText->SetText("=== HOUSES ===", { 0.0f, 1.0f, 1.0f, 1.0f });
    //            m_debugText->SetText("No houses found", { 1.0f, 0.0f, 0.0f, 1.0f });
    //            return;
    //        }
    //        
    //        // タイトル
    //        m_debugText->SetText("=== HOUSES ===", { 0.0f, 1.0f, 1.0f, 1.0f });
    //        
    //        // 家の総数
    //        char totalText[64];
    //        snprintf(totalText, sizeof(totalText), "Total: %zu", m_houses.size());
    //        m_debugText->SetText(totalText, { 1.0f, 1.0f, 1.0f, 1.0f });
    //        
    //        // 各家の情報
    //        int repaired = 0;
    //        float totalElectricity = 0.0f;
    //        float maxTotalElectricity = 0.0f;
    //        
    //        for (size_t i = 0; i < m_houses.size(); ++i) {
    //            House* house = m_houses[i];
    //            if (!house) continue;
    //            
    //            float electricity = house->GetElectricity();
    //            float maxElectricity = house->GetMaxElectricity();
    //            bool isRepaired = house->IsRepaired();
    //            
    //            totalElectricity += electricity;
    //            maxTotalElectricity += maxElectricity;
    //            
    //            if (isRepaired) repaired++;
    //            
    //            // 各家の状態表示
    //            char houseText[96];
    //            snprintf(houseText, sizeof(houseText),
    //                "House%zu: %.0f/%.0f",
    //                i + 1,
    //                electricity,
    //                maxElectricity
    //            );
    //            
    //            // 復旧状態で色分け
    //            DirectX::XMFLOAT4 color = isRepaired ? 
    //                DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) :  // 緑：復旧完了
    //                DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f);   // 黄：復旧中
    //            
    //            m_debugText->SetText(houseText, color);
    //        }
    //        
    //        // 空行
    //        m_debugText->SetText("", { 1.0f, 1.0f, 1.0f, 1.0f });
    //        
    //        // 統計情報
    //        char statsText[96];
    //        snprintf(statsText, sizeof(statsText),
    //            "Repaired: %d/%zu",
    //            repaired,
    //            m_houses.size()
    //        );
    //        m_debugText->SetText(statsText, { 0.0f, 1.0f, 1.0f, 1.0f });
    //        
    //        if (maxTotalElectricity > 0.0f) {
    //            char totalElecText[96];
    //            snprintf(totalElecText, sizeof(totalElecText),
    //                "Total Elec: %.0f/%.0f",
    //                totalElectricity,
    //                maxTotalElectricity
    //            );
    //            m_debugText->SetText(totalElecText, { 1.0f, 1.0f, 1.0f, 1.0f });
    //            
    //            // 回復率
    //            float recoveryRate = (totalElectricity / maxTotalElectricity) * 100.0f;
    //            char rateText[64];
    //            snprintf(rateText, sizeof(rateText),
    //                "Recovery: %.1f%%",
    //                recoveryRate
    //            );
    //            
    //            DirectX::XMFLOAT4 rateColor = (recoveryRate >= 100.0f) ?
    //                DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) :  // 緑：完全復旧
    //                DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);   // 赤：復旧中
    //            
    //            m_debugText->SetText(rateText, rateColor);
    //        }
    //    }
    //}
}

void UIHouse::Draw() const
{
    if (m_debugText && IsVisible()) {
        m_debugText->Draw();
    }
    
    // フォールバック: DebugText が初期化されていない場合のログ
    if (!m_debugText && IsVisible()) {
        // Note: 実際のアプリケーションでは、ここにデバッグログ出力を追加
        // hal::dout << "UIHouse: DebugText not initialized" << std::endl;
    }
}

void UIHouse::SetHouses(const std::vector<House*>& houses)
{
    m_houses = houses;
}

void UIHouse::OnShow()
{
    // 表示時の処理（必要に応じて）
}

void UIHouse::OnHide()
{
    // 非表示時の処理（必要に応じて）
}
