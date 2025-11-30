#pragma once
////////////////////////////
// UI_Charge.h
// UI チャージ表示クラス
//Author:hiroshi kasiwagi
//Date:2025/11/27
////////////////////////////
#ifndef UI_CHARGE_H
#define UI_CHARGE_H
#include "UI.h"
#include <DirectXMath.h>

class UI_Charge : public UI
{
public:
 UI_Charge() = default;
 ~UI_Charge() = default;

 // ライフサイクル
 void Initialize() override;
 void Update(double elapsedTime) override;
 void Draw() const override;

private:
 DirectX::XMFLOAT4 color_{1.0f,1.0f,1.0f,1.0f};
 float chargeAmount_ =0.0f; // current health
 int texBg_ = -1;
 int texFill_ = -1;
 // UI の表示サイズ（スクリーンピクセル）
 DirectX::XMFLOAT2 size_{200.0f,20.0f};
 // 内側パディング
 float padding_ =2.0f;
};
#endif // UI_CHARGE_H
