#pragma once

#include "UI.h"

/**
 * @brief タイマーUI: ゲーム開始時に設定した時間をカウントダウン表示
 * 
 * 使用例:
 *   UI_Timer* timer = new UI_Timer();
 *   timer->Initialize();
 *   timer->SetTime(300.0f);  // 300秒でカウント開始
 *   UIManager::Add(timer);
 */
class UI_Timer : public UI
{
public:
    UI_Timer();
    virtual ~UI_Timer() = default;

    // UI基本インターフェース
    virtual void Initialize() override;
    virtual void Update(double elapsedTime) override;
    virtual void Draw() const override;

    /**
     * @brief カウントダウンを開始する時間を設定（秒単位）
     * @param timeSeconds カウントダウンする時間（秒）
     */
    void SetTime(float timeSeconds);

    /**
     * @brief 残り時間を取得
     * @return 残り時間（秒）
     */
    float GetRemainingTime() const { return remainingTime_; }

    /**
     * @brief カウントダウンが終了したかチェック
     * @return true: 時間切れ, false: カウント中
     */
    bool IsTimeUp() const { return remainingTime_ <= 0.0f; }

    /**
     * @brief 表示位置を設定
     * @param x X座標
     * @param y Y座標
     */
    void SetPosition(float x, float y);

    /**
     * @brief 表示サイズを設定
     * @param digitHeight 数字1文字の高さ
     */
    void SetDigitHeight(float digitHeight);

private:
    float remainingTime_;    // 残り時間（秒）
    float totalTime_;        // 開始時の設定時間
    int numberTexture_;      // 数字テクスチャID
    float posX_;             // 表示X座標
    float posY_;             // 表示Y座標
    float digitHeight_;      // 数字1文字の高さ
};
