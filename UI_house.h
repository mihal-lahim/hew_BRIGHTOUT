#pragma once

#include "UI.h"
#include <DirectXMath.h>
#include <vector>

// フォワード宣言
class House;
struct MODEL;
namespace hal
{
    class DebugText;
}

// 家UI: 現在存在する家の数、電気量、復旧状態を表示
class UIHouse : public UI
{
public:
    UIHouse();
    virtual ~UIHouse();

    // ライフサイクル
    virtual void Initialize() override;
    virtual void Update(double elapsedTime) override;
    virtual void Draw() const override;

    // 家のリスト更新
    void SetHouses(const std::vector<House*>& houses);

protected:
    virtual void OnShow() override;
    virtual void OnHide() override;

private:
    std::vector<House*> m_houses;           // 表示対象の家リスト
    hal::DebugText* m_debugText = nullptr;  // デバッグテキスト表示用
    
    // テキスト表示用のメンバ
    float m_updateTimer = 0.0f;
    static constexpr float TEXT_UPDATE_INTERVAL = 0.1f;  // テキスト更新間隔（秒）
};
