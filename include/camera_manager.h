#pragma once
/*
 CameraManager - 複数のカメラインスタンスを管理し、アクティブカメラを切り替える
 Author: GitHub Copilot (generated)
 Date:2025/11/27
*/

#include "camera.h"
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>

class CameraManager
{
public:
 CameraManager() = default;
 ~CameraManager() = default;

 // カメラポインタを登録する（任意の名前付き）。Manager は所有権を持たない。
 // 登録に成功するとカメラのインデックスを返す。
 size_t AddCamera(Camera* cam, const std::string& name = "");

 // ポインタまたはインデックスでカメラを削除する。削除に成功すれば true を返す。
 bool RemoveCamera(Camera* cam);
 bool RemoveCameraAt(size_t index);

 // インデックスや名前でアクティブカメラを設定する。成功なら true を返す。
 bool SetActiveByIndex(size_t index);
 bool SetActiveByName(const std::string& name);

 // 次/前のカメラに切り替える（循環）。成功すると新しいアクティブインデックスを返す。
 int Next();
 int Prev();

 // アクティブカメラのポインタを取得（nullptr の場合あり）
 Camera* GetActive() const;

 // アクティブカメラを更新する
 void UpdateActive(double elapsed);

 // アクティブカメラの SetMatrix を呼ぶ
 void ApplyActive() const;

 // 指定したカメラポインタのインデックスを返す、見つからなければ npos
 static const size_t npos = static_cast<size_t>(-1);
 size_t FindIndex(Camera* cam) const;

 // 登録をクリアするユーティリティ
 void Clear();

private:
 std::vector<Camera*> cameras_;
 std::unordered_map<std::string, size_t> nameMap_;
 size_t activeIndex_{ npos };
};
