/////////////////////////////////////////
//controller.h[コントローラー制御]
//Author: hiroshi kasiwagi
//Date: 2025/11/26
/////////////////////////////////////////
#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <windows.h>
#include <Xinput.h>
#include <array>
#include <chrono>
#include <utility>
#include <thread>
#include <atomic>
#include <functional>

/*
 Controller クラス概要（簡潔）
 - 1 インスタンス = 1 コントローラー (controllerNumber)
 - ゲームループで毎フレーム `Update()` を呼ぶことで状態が更新される。
 - または `Start()` で内部スレッドを回し続けることもできる（Start/Stop/Join）。
 - 表示や処理はコールバックで外側に通知する（スレッドはコール元に注意）。
 - public クエリ: IsDown / WasPressed / WasReleased / GetLastHoldTime / GetTrigger / GetStick
 - スレッド安全: 公開 API は内部状態を参照するが、同じインスタンスを複数スレッドから
   同時に操作する場合は上位で同期すること。
*/

class Controller 
{
public:
    // ------------------ ボタン定義 ------------------
    enum Button : WORD {
        BUTTON_A             = XINPUT_GAMEPAD_A,
        BUTTON_B             = XINPUT_GAMEPAD_B,
        BUTTON_X             = XINPUT_GAMEPAD_X,
        BUTTON_Y             = XINPUT_GAMEPAD_Y,
        BUTTON_LEFT_SHOULDER = XINPUT_GAMEPAD_LEFT_SHOULDER,
        BUTTON_RIGHT_SHOULDER= XINPUT_GAMEPAD_RIGHT_SHOULDER,
        BUTTON_START         = XINPUT_GAMEPAD_START,
        BUTTON_BACK          = XINPUT_GAMEPAD_BACK,
        BUTTON_DPAD_UP       = XINPUT_GAMEPAD_DPAD_UP,
        BUTTON_DPAD_DOWN     = XINPUT_GAMEPAD_DPAD_DOWN,
        BUTTON_DPAD_LEFT     = XINPUT_GAMEPAD_DPAD_LEFT,
        BUTTON_DPAD_RIGHT    = XINPUT_GAMEPAD_DPAD_RIGHT
    };

    // 左右の区別（スティック／トリガー）
    enum Direction: bool {
        DIR_LEFT = false,
        DIR_RIGHT = true
    };

    // トリガー・スティック情報（軽量）
    struct TriggerInfo {
        int value;   // 0..255
        bool above;  // 閾値を超えているか
    };

    struct StickInfo {
        SHORT x;
        SHORT y;
        int magnitude; // sqrt(x^2 + y^2)
        bool outside;  // デッドゾーン外か
    };

    // コンストラクタ
    // controllerNumber: XInput のパッド番号 (0..3)
    // pollMs: Run() が内部でスリープするミリ秒 (Update を使う場合は無視)
    explicit Controller(int controllerNumber = 0, int pollMs = 100);

    // スレッド制御（必要なら Start で内部スレッドを回す）
    void Start();
    void Stop();
    void Join();

    // フレーム単位の更新（ゲームループから毎フレーム呼ぶ）
    // Update を使う場合は Start/RunLoop を使わないでください（重複呼び出し注意）
    void Update();

    // コントローラー番号取得
    int GetControllerNumber() const { return controllerNumber_; }

    // 状態クエリ（Update 後に呼ぶ）
    bool IsDown(Button btn) const;
    bool WasPressed(Button btn) const;
    bool WasReleased(Button btn) const;
    double GetLastHoldTime(Button btn) const;

    TriggerInfo GetTrigger(Direction dir) const;
    StickInfo GetStick(Direction dir) const;

    // 互換用：ブロッキング実行（内部で Update を連続呼び）
    void Run();

    // --- イベントコールバック（表示やゲームロジックはここで受け取る） ---
    // 注意: コールバックは Update/RunLoop のコンテキスト（スレッド）から呼ばれる
    void SetOnConnected(std::function<void(int)> cb);
    void SetOnDisconnected(std::function<void(int)> cb);
    void SetOnButtonPress(std::function<void(int, Button)> cb);
    void SetOnButtonRelease(std::function<void(int, Button, double)> cb);
    void SetOnTriggerChange(std::function<void(int, Direction, const TriggerInfo&)> cb);
    void SetOnStickChange(std::function<void(int, Direction, const StickInfo&)> cb);

private:
    static const size_t BUTTON_COUNT = 12;
    static const size_t PAD_COUNT = 4;

    int controllerNumber_;
    int pollMs_;
    bool triggerUseBoolMode_;
    bool stickUseBoolMode_;
    const int triggerThreshold_;
    const int stickDeadZone_;
    const int triggerDeltaForValueMode_;
    const int stickDeltaForValueMode_;

    struct PadPrev {
        std::array<bool, BUTTON_COUNT> btns{};
        TriggerInfo leftTrigger{0,false};
        TriggerInfo rightTrigger{0,false};
        StickInfo leftStick{0,0,0,false};
        StickInfo rightStick{0,0,0,false};
        bool connected = false;
    };

    // 前フレーム / 現在フレーム
    std::array<PadPrev, PAD_COUNT> prev_;
    std::array<PadPrev, PAD_COUNT> cur_;

    // 押下時間管理
    std::array<std::array<std::chrono::steady_clock::time_point, BUTTON_COUNT>, PAD_COUNT> pressTime_;
    std::array<std::array<bool, BUTTON_COUNT>, PAD_COUNT> hasPressTime_;
    std::array<std::array<double, BUTTON_COUNT>, PAD_COUNT> lastHoldTime_;
    std::array<std::array<bool, BUTTON_COUNT>, PAD_COUNT> pressedThisFrame_;
    std::array<std::array<bool, BUTTON_COUNT>, PAD_COUNT> releasedThisFrame_;

    // 定義は実装ファイルに置く
    static const std::pair<Button, const char*> g_buttonList[BUTTON_COUNT];

    // 内部ユーティリティ（簡潔に）
    int FindButtonIndex(Button btn) const;
    double GetButtonHoldTimeInternal(int pad, Button button);
    void RecordButtonPressTimeInternal(int pad, Button button);

    // XINPUT_STATE ベースのクエリ
    bool IsButtonPressedState(const XINPUT_STATE &state, Button button) const;
    TriggerInfo QueryTriggerState(const XINPUT_STATE &st, Direction dir, int threshold) const;
    StickInfo QueryStickState(const XINPUT_STATE &st, Direction dir, int deadzone) const;

    // 実行ループ（Start が使う）
    void RunLoop();

    // スレッド管理
    std::thread thread_;
    std::atomic<bool> running_;

    // イベントコールバック
    std::function<void(int)> onConnected_;
    std::function<void(int)> onDisconnected_;
    std::function<void(int, Button)> onButtonPress_;
    std::function<void(int, Button, double)> onButtonRelease_;
    std::function<void(int, Direction, const TriggerInfo&)> onTriggerChange_;
    std::function<void(int, Direction, const StickInfo&)> onStickChange_;
};

#endif // CONTROLLER_H
