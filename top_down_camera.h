#ifndef TOP_DOWN_CAMERA_H
#define TOP_DOWN_CAMERA_H
#include "camera.h"
#include <DirectXMath.h>

class Player;

// TPSゲームに適したカメラ設定
namespace TPSCameraSettings {
    constexpr float SENSITIVITY_X = 4.0f;         // 水平回転感度
    constexpr float SENSITIVITY_Y = 2.0f;         // 垂直（高さ）調整感度
    constexpr float FOLLOW_SPEED = 3.0f;          // 自動追従の速度
    constexpr float MIN_DISTANCE = 2.0f;          // プレイヤーとの最小距離
    constexpr float MAX_DISTANCE = 15.0f;         // プレイヤーとの最大距離
    constexpr float DEFAULT_DISTANCE = 8.0f;      // 初期距離
    constexpr float DEFAULT_HEIGHT_OFFSET = 2.0f; // プレイヤーの頭上からの高さオフセット
}

class TopDownCamera : public Camera
{
public:
    TopDownCamera(Player* owner, float height = TPSCameraSettings::DEFAULT_HEIGHT_OFFSET, float orthoWidth = 20.0f);
    void Update(double elapsed_time) override;
    void SetMatrix() const override;

    // カメラ角度を外部から参照可能に
    float GetCameraAngle() const { return cameraAngle_; }

private:
    Player* owner_ = nullptr; // プレイヤーへの参照
    float height_ = 10.0f;
    float orthoWidth_ = 20.0f;

    // カメラの独立した角度制御
    float cameraAngle_ = 0.0f;      // 現在のカメラ水平角度（ラジアン）
    float cameraPitch_ = 0.2f;      // 現在のカメラ垂直角度（ラジアン）
    float cameraDistance_ = TPSCameraSettings::DEFAULT_DISTANCE; // プレイヤーからの距離
};
#endif // TOP_DOWN_CAMERA_H