/////////////////////////////////////////
//controller.cpp[コントローラー制御]
//Author: hiroshi kasiwagi
//Date: 2025/11/26
/////////////////////////////////////////

#include "controller.h"

#include <thread>
#include <chrono>
#include <cmath>
#include <utility>

#pragma comment(lib, "Xinput.lib")

// ボタン名配列（外部表示で使用）
const std::pair<Controller::Button, const char*> Controller::g_buttonList[Controller::BUTTON_COUNT] = {
	{ Controller::BUTTON_A, "A" },
	{ Controller::BUTTON_B, "B" },
	{ Controller::BUTTON_X, "X" },
	{ Controller::BUTTON_Y, "Y" },
	{ Controller::BUTTON_LEFT_SHOULDER, "LB" },
	{ Controller::BUTTON_RIGHT_SHOULDER, "RB" },
	{ Controller::BUTTON_START, "Start" },
	{ Controller::BUTTON_BACK, "Back" },
	{ Controller::BUTTON_DPAD_UP, "DPadUp" },
	{ Controller::BUTTON_DPAD_DOWN, "DPadDown" },
	{ Controller::BUTTON_DPAD_LEFT, "DPadLeft" },
	{ Controller::BUTTON_DPAD_RIGHT, "DPadRight" }
};

// コンストラクタ（デフォルト値は互換性維持）
Controller::Controller(int controllerNumber, int pollMs)
	: controllerNumber_(controllerNumber),
	pollMs_(pollMs),
	triggerUseBoolMode_(true),
	stickUseBoolMode_(false),
	triggerThreshold_(10),
	stickDeadZone_(7849),
	triggerDeltaForValueMode_(6),
	stickDeltaForValueMode_(800),
	running_(false)
{
	// 初期化: 配列をゼロ初期化（簡潔に）
	for (size_t p = 0; p < PAD_COUNT; ++p) {
		for (size_t b = 0; b < BUTTON_COUNT; ++b) {
			hasPressTime_[p][b] = false;
			lastHoldTime_[p][b] = 0.0;
			pressedThisFrame_[p][b] = false;
			releasedThisFrame_[p][b] = false;
			pressTime_[p][b] = std::chrono::steady_clock::time_point{};
		}
		prev_[p] = {};
		cur_[p] = {};
	}
}

// コールバック設定（ムーブで保持）
void Controller::SetOnConnected(std::function<void(int)> cb) { onConnected_ = std::move(cb); }
void Controller::SetOnDisconnected(std::function<void(int)> cb) { onDisconnected_ = std::move(cb); }
void Controller::SetOnButtonPress(std::function<void(int, Button)> cb) { onButtonPress_ = std::move(cb); }
void Controller::SetOnButtonRelease(std::function<void(int, Button, double)> cb) { onButtonRelease_ = std::move(cb); }
void Controller::SetOnTriggerChange(std::function<void(int, Direction, const TriggerInfo&)> cb) { onTriggerChange_ = std::move(cb); }
void Controller::SetOnStickChange(std::function<void(int, Direction, const StickInfo&)> cb) { onStickChange_ = std::move(cb); }

// ボタンの配列インデックスを返す（存在しなければ -1）
int Controller::FindButtonIndex(Button btn) const
{
	for (size_t i = 0; i < BUTTON_COUNT; ++i) {
		if (g_buttonList[i].first == btn) return static_cast<int>(i);
	}
	return -1;
}

// 内部: 押下時間の計測を完了して秒を返す（lastHoldTime_ をセット）
double Controller::GetButtonHoldTimeInternal(int pad, Button button)
{
	if (pad < 0 || pad >= static_cast<int>(PAD_COUNT)) return 0.0;
	int idx = FindButtonIndex(button);
	if (idx < 0) return 0.0;
	if (!hasPressTime_[pad][idx]) return 0.0;
	auto now = std::chrono::steady_clock::now();
	auto start = pressTime_[pad][idx];
	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
	hasPressTime_[pad][idx] = false;
	double sec = static_cast<double>(ms) / 1000.0;
	lastHoldTime_[pad][idx] = sec;
	return sec;
}

// 内部: 押下時間の計測開始を記録
void Controller::RecordButtonPressTimeInternal(int pad, Button button)
{
	if (pad < 0 || pad >= static_cast<int>(PAD_COUNT)) return;
	int idx = FindButtonIndex(button);
	if (idx < 0) return;
	pressTime_[pad][idx] = std::chrono::steady_clock::now();
	hasPressTime_[pad][idx] = true;
}

// XINPUT_STATE ベースのクエリ関数（簡潔）
bool Controller::IsButtonPressedState(const XINPUT_STATE& state, Button button) const
{
	return (state.Gamepad.wButtons & static_cast<WORD>(button)) != 0;
}

// トリガー・スティック状態クエリ
Controller::TriggerInfo Controller::QueryTriggerState(const XINPUT_STATE& st, Direction dir, int threshold) const
{
	int v = (dir == DIR_LEFT) ? st.Gamepad.bLeftTrigger : st.Gamepad.bRightTrigger;
	return { v, v > threshold };
}

// スティック状態クエリ
Controller::StickInfo Controller::QueryStickState(const XINPUT_STATE& st, Direction dir, int deadzone) const
{
	SHORT x = (dir == DIR_LEFT) ? st.Gamepad.sThumbLX : st.Gamepad.sThumbRX;
	SHORT y = (dir == DIR_LEFT) ? st.Gamepad.sThumbLY : st.Gamepad.sThumbRY;
	int mag = static_cast<int>(std::hypot(static_cast<double>(x), static_cast<double>(y)));
	bool out = (std::abs(static_cast<int>(x)) > deadzone) || (std::abs(static_cast<int>(y)) > deadzone);
	return { x, y, mag, out };
}

// 互換用: ブロッキング実行（RunLoop を回す）
void Controller::Run() { RunLoop(); }

// スレッド開始
void Controller::Start()
{
	if (running_) return;
	running_ = true;
	thread_ = std::thread(&Controller::RunLoop, this);
}

void Controller::Stop() { running_ = false; }

void Controller::Join() { if (thread_.joinable()) thread_.join(); }

// Update: フレーム単位で呼ぶことを想定した簡潔な処理
void Controller::Update()
{
	int pad = controllerNumber_;
	// フレームフラグをクリア
	for (size_t b = 0; b < BUTTON_COUNT; ++b) {
		pressedThisFrame_[pad][b] = false;
		releasedThisFrame_[pad][b] = false;
	}

	XINPUT_STATE st{};
	DWORD rc = XInputGetState(static_cast<DWORD>(pad), &st);
	bool connected = (rc == ERROR_SUCCESS);

	// 接続/切断イベント（前回状態と比較）
	if (connected && !prev_[pad].connected) { if (onConnected_) onConnected_(pad); }
	else if (!connected && prev_[pad].connected) { if (onDisconnected_) onDisconnected_(pad); }

	if (!connected) {
		cur_[pad] = {};
		cur_[pad].connected = false;
		prev_[pad] = cur_[pad];
		return;
	}

	cur_[pad].connected = true;

	// ボタン更新（簡潔にループ処理しイベント発行）
	for (size_t i = 0; i < BUTTON_COUNT; ++i) {
		Button btn = g_buttonList[i].first;
		bool nowDown = IsButtonPressedState(st, btn);
		cur_[pad].btns[i] = nowDown;
		if (nowDown && !prev_[pad].btns[i]) {
			RecordButtonPressTimeInternal(pad, btn);
			pressedThisFrame_[pad][i] = true;
			if (onButtonPress_) onButtonPress_(pad, btn);
		}
		else if (!nowDown && prev_[pad].btns[i]) {
			double holdSec = GetButtonHoldTimeInternal(pad, btn);
			releasedThisFrame_[pad][i] = true;
			if (onButtonRelease_) onButtonRelease_(pad, btn, holdSec);
		}
	}

	// トリガー更新（変更時のみイベント）
	TriggerInfo curL = QueryTriggerState(st, DIR_LEFT, triggerThreshold_);
	TriggerInfo curR = QueryTriggerState(st, DIR_RIGHT, triggerThreshold_);
	if (triggerUseBoolMode_) {
		if (curL.above != prev_[pad].leftTrigger.above && onTriggerChange_) onTriggerChange_(pad, DIR_LEFT, curL);
		if (curR.above != prev_[pad].rightTrigger.above && onTriggerChange_) onTriggerChange_(pad, DIR_RIGHT, curR);
	}
	else {
		if (std::abs(curL.value - prev_[pad].leftTrigger.value) > triggerDeltaForValueMode_ && onTriggerChange_) onTriggerChange_(pad, DIR_LEFT, curL);
		if (std::abs(curR.value - prev_[pad].rightTrigger.value) > triggerDeltaForValueMode_ && onTriggerChange_) onTriggerChange_(pad, DIR_RIGHT, curR);
	}
	cur_[pad].leftTrigger = curL;
	cur_[pad].rightTrigger = curR;

	// スティック更新（変更時のみイベント）
	StickInfo curLT = QueryStickState(st, DIR_LEFT, stickDeadZone_);
	StickInfo curRT = QueryStickState(st, DIR_RIGHT, stickDeadZone_);
	if (stickUseBoolMode_) {
		if (curLT.outside != prev_[pad].leftStick.outside && onStickChange_) onStickChange_(pad, DIR_LEFT, curLT);
		if (curRT.outside != prev_[pad].rightStick.outside && onStickChange_) onStickChange_(pad, DIR_RIGHT, curRT);
	}
	else {
		if ((std::abs(curLT.x - prev_[pad].leftStick.x) > stickDeltaForValueMode_ ||
			std::abs(curLT.y - prev_[pad].leftStick.y) > stickDeltaForValueMode_ ||
			std::abs(curLT.magnitude - prev_[pad].leftStick.magnitude) > stickDeltaForValueMode_) && onStickChange_) {
			onStickChange_(pad, DIR_LEFT, curLT);
		}
		if ((std::abs(curRT.x - prev_[pad].rightStick.x) > stickDeltaForValueMode_ ||
			std::abs(curRT.y - prev_[pad].rightStick.y) > stickDeltaForValueMode_ ||
			std::abs(curRT.magnitude - prev_[pad].rightStick.magnitude) > stickDeltaForValueMode_) && onStickChange_) {
			onStickChange_(pad, DIR_RIGHT, curRT);
		}
	}
	cur_[pad].leftStick = curLT;
	cur_[pad].rightStick = curRT;

	// prev を更新して次フレーム判定へ
	prev_[pad] = cur_[pad];
}

// RunLoop: Start によって内部スレッドで回る（簡潔）
void Controller::RunLoop()
{
	if (controllerNumber_ < 0 || controllerNumber_ >= static_cast<int>(PAD_COUNT)) return;
	running_ = true;
	while (running_) {
		Update();
		std::this_thread::sleep_for(std::chrono::milliseconds(pollMs_));
	}
}

// Update 後に呼べるクエリ関数（簡潔）
bool Controller::IsDown(Button btn) const
{
	int idx = FindButtonIndex(btn);
	if (idx < 0) return false;
	return cur_[controllerNumber_].btns[idx];
}

bool Controller::WasPressed(Button btn) const
{
	int idx = FindButtonIndex(btn);
	if (idx < 0) return false;
	return pressedThisFrame_[controllerNumber_][idx];
}

bool Controller::WasReleased(Button btn) const
{
	int idx = FindButtonIndex(btn);
	if (idx < 0) return false;
	return releasedThisFrame_[controllerNumber_][idx];
}

double Controller::GetLastHoldTime(Button btn) const
{
	int idx = FindButtonIndex(btn);
	if (idx < 0) return 0.0;
	return lastHoldTime_[controllerNumber_][idx];
}

Controller::TriggerInfo Controller::GetTrigger(Direction dir) const
{
	return (dir == DIR_LEFT) ? cur_[controllerNumber_].leftTrigger : cur_[controllerNumber_].rightTrigger;
}

Controller::StickInfo Controller::GetStick(Direction dir) const
{
	return (dir == DIR_LEFT) ? cur_[controllerNumber_].leftStick : cur_[controllerNumber_].rightStick;
}
