//////////////////////////////
// UI_Timer.cpp
// タイマーUI: ゲーム開始時に設定した時間をカウントダウン表示
// Author: hiroshi kasiwagi
// Date: 2025/11/27
//////////////////////////////

#include "UI_Timer.h"
#include "texture.h"
#include "sprite.h"
#include "direct3d.h"
#include <cmath>
#include <algorithm>

// DrawNumber()関数の宣言（UI_Charge.cppに定義されている）
namespace {
    void DrawNumber(int texId, int number, float x, float y, float h)
    {
        if (texId < 0) return;
        
        // digit sizing
        float digitH = h;
        float digitW = digitH * 0.7f;

        // source tile size in sprite
        int srcW = (int)(digitH * 0.7f);
        int srcH = (int)digitH;
        int srcY = 100;

        // 負数の場合は0として扱う
        if (number < 0) number = 0;

        // 0の場合は0を描画して終了（テクスチャは「1234567890」なので0は9番目）
        if (number == 0) {
            int srcX = 9 * srcW;  // 0は9番目の位置
            Sprite_Draw(texId, x, y, digitW, digitH, srcX, srcY, srcW, srcH);
            return;
        }

        // 数値を文字列に変換
        char buf[16];
        int len = 0;
        int n = number;
        
        // 数字を逆順で配列に格納
        while (n > 0 && len < 15) {
            buf[len++] = '0' + (n % 10);
            n /= 10;
        }
        
        if (len == 0) return;

        // 逆順を正順に描画
        float px = x;
        for (int i = len - 1; i >= 0; --i) {
            int digit = buf[i] - '0';
            // テクスチャが「1234567890」の順なため、0は9番目、1は0番目...9は8番目
            int srcX;
            if (digit == 0) {
                srcX = 9 * srcW;  // 0は9番目
            } else {
                srcX = (digit - 1) * srcW;  // 1～9は0～8番目
            }
            Sprite_Draw(texId, px, y, digitW, digitH, srcX, srcY, srcW, srcH);
            px += digitW;
        }
    }

    // コロン「:」を描画する関数
    void DrawColon(int texId, float x, float y, float h)
    {
        if (texId < 0) return;
        
        float digitH = h;
        float digitW = digitH * 0.7f;
        
        // コロンのテクスチャ座標（10番目の位置）
        int srcW = (int)(digitH * 0.7f);
        int srcH = (int)digitH;
        int srcY = 100;
        int colonSrcX = 10 * srcW;  // コロンのX座標
        
        Sprite_Draw(texId, x, y, digitW, digitH, colonSrcX, srcY, srcW, srcH);
    }
}

UI_Timer::UI_Timer()
    : remainingTime_(0.0f),
      totalTime_(0.0f),
      numberTexture_(-1),
      posX_(1920.0f / 2.0f - 80.0f),  // 画面中央
      posY_(50.0f),
      digitHeight_(60.0f)
{
}

void UI_Timer::Initialize()
{
    // 数字テクスチャを読み込み
    if (numberTexture_ < 0) {
        numberTexture_ = Texture_Load(L"texture/brightout_number_k.png");
    }
}

void UI_Timer::Update(double elapsedTime)
{
    // カウントダウン
    if (remainingTime_ > 0.0f) {
        remainingTime_ -= static_cast<float>(elapsedTime);
        if (remainingTime_ < 0.0f) {
            remainingTime_ = 0.0f;
        }
    }
}

void UI_Timer::Draw() const
{
    //if (numberTexture_ < 0) return;

    //// 残り時間を秒単位で整数化
    //int totalSeconds = static_cast<int>(std::ceil(remainingTime_));
    //
    //// 分と秒に分割
    //int minutes = totalSeconds / 60;
    //int seconds = totalSeconds % 60;
    //
    //// 分を描画
    //DrawNumber(numberTexture_, minutes, posX_, posY_, digitHeight_);
    //
    //// コロンの位置を計算
    //float colonX = posX_ + digitHeight_ * 0.7f * 2 + 5.0f;  // 2桁の数字分 + 少し余白
    //
    //// コロン「:」を描画
    //DrawColon(numberTexture_, colonX, posY_, digitHeight_);
    //
    //// 秒を描画する位置を計算
    //float secondsX = colonX + digitHeight_ * 0.7f + 5.0f;
    //
    //// 秒の十の位を描画
    //int secondsTens = seconds / 10;
    //DrawNumber(numberTexture_, secondsTens, secondsX, posY_, digitHeight_);
    //
    //// 秒の一の位を描画
    //int secondsOnes = seconds % 10;
    //float secondsOnesX = secondsX + digitHeight_ * 0.7f;
    //DrawNumber(numberTexture_, secondsOnes, secondsOnesX, posY_, digitHeight_);
}

void UI_Timer::SetTime(float timeSeconds)
{
    remainingTime_ = std::max(0.0f, timeSeconds);
    totalTime_ = remainingTime_;
}

void UI_Timer::SetPosition(float x, float y)
{
    posX_ = x;
    posY_ = y;
}

void UI_Timer::SetDigitHeight(float digitHeight)
{
    digitHeight_ = std::max(1.0f, digitHeight);
}
