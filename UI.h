//////////////////////////////
// UI.h
// UI 基底クラスと UI 管理クラス
//Author:hiroshi kasiwagi
//Date:2025/11/27
//////////////////////////////

#ifndef UI_H
#define UI_H



#include <DirectXMath.h>
#include <vector>
#include <algorithm>

class UI
{
public:
 UI() = default;
 virtual ~UI() = default;

 // ライフサイクル
 virtual void Initialize() {}
 virtual void Update(double elapsedTime) {}
 virtual void Draw() const {}

 // 表示 / 有効フラグ
 void SetVisible(bool v) { visible_ = v; if (visible_) OnShow(); else OnHide(); }
 bool IsVisible() const { return visible_; }

 void SetEnabled(bool e) { enabled_ = e; }
 bool IsEnabled() const { return enabled_; }

 //位置/サイズ（スクリーン座標）
 void SetPosition(const DirectX::XMFLOAT2& p) { position_ = p; }
 DirectX::XMFLOAT2 GetPosition() const { return position_; }

 void SetSize(const DirectX::XMFLOAT2& s) { size_ = s; }
 DirectX::XMFLOAT2 GetSize() const { return size_; }

 // Zオーダー（描画順）
 void SetZOrder(int z) { zOrder_ = z; }
 int GetZOrder() const { return zOrder_; }

protected:
 // 表示に伴うフック
 virtual void OnShow() {}
 virtual void OnHide() {}

private:
 bool visible_ = true;
 bool enabled_ = true;
 DirectX::XMFLOAT2 position_{0.0f,0.0f};
 DirectX::XMFLOAT2 size_{100.0f,30.0f};
 int zOrder_ =0;
};

// シンプルな UI 管理クラス（シングルトン風、静的アクセス）
class UIManager
{
public:
 static void Add(UI* ui)
 {
 if (!ui) return;
 Get().list_.push_back(ui);
 SortByZ();
 }

 static void Remove(UI* ui)
 {
 if (!ui) return;
 auto &v = Get().list_;
 v.erase(std::remove(v.begin(), v.end(), ui), v.end());
 }

 static void InitializeAll()
 {
 for (auto ui : Get().list_) if (ui) ui->Initialize();
 }

 static void UpdateAll(double elapsed)
 {
 for (auto ui : Get().list_) if (ui && ui->IsEnabled()) ui->Update(elapsed);
 }

 static void DrawAll()
 {
 for (auto ui : Get().list_) if (ui && ui->IsVisible()) ui->Draw();
 }

 static void Clear()
 {
 Get().list_.clear();
 }

private:
 UIManager() = default;
 ~UIManager() = default;

 static UIManager& Get()
 {
 static UIManager instance;
 return instance;
 }

 static void SortByZ()
 {
 auto &v = Get().list_;
 std::sort(v.begin(), v.end(), [](const UI* a, const UI* b){
 if (!a) return false;
 if (!b) return true;
 return a->GetZOrder() < b->GetZOrder();
 });
 }

 std::vector<UI*> list_;
}; 
#endif // !UI_H