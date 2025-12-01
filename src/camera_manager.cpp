#include "camera_manager.h"
#include <algorithm>

size_t CameraManager::AddCamera(Camera* cam, const std::string& name)
{
	if (!cam) return npos;
	cameras_.push_back(cam);
	size_t idx = cameras_.size() - 1;
	if (!name.empty()) nameMap_[name] = idx;
	if (activeIndex_ == npos) activeIndex_ = idx;
	return idx;
}

bool CameraManager::RemoveCamera(Camera* cam)
{
	auto it = std::find(cameras_.begin(), cameras_.end(), cam);
	if (it == cameras_.end()) return false;
	size_t idx = std::distance(cameras_.begin(), it);
	return RemoveCameraAt(idx);
}

bool CameraManager::RemoveCameraAt(size_t index)
{
	if (index >= cameras_.size()) return false;
	cameras_.erase(cameras_.begin() + index);
	// nameMap_ ‚ÌƒGƒ“ƒgƒŠ‚Æ activeIndex_ ‚ğ’²®
	std::unordered_map<std::string, size_t> newMap;
	for (auto& p : nameMap_) {
		if (p.second == index) continue; // íœ‚³‚ê‚½‚à‚Ì
		size_t newIndex = p.second;
		if (p.second > index) newIndex = p.second - 1;
		newMap[p.first] = newIndex;
	}
	nameMap_.swap(newMap);
	if (cameras_.empty()) activeIndex_ = npos;
	else if (activeIndex_ == index) activeIndex_ = 0;
	else if (activeIndex_ > index) --activeIndex_;
	return true;
}

bool CameraManager::SetActiveByIndex(size_t index)
{
	if (index >= cameras_.size()) return false;
	activeIndex_ = index;
	return true;
}

bool CameraManager::SetActiveByName(const std::string& name)
{
	auto it = nameMap_.find(name);
	if (it == nameMap_.end()) return false;
	return SetActiveByIndex(it->second);
}

int CameraManager::Next()
{
	if (cameras_.empty()) return -1;
	if (activeIndex_ == npos) { activeIndex_ = 0; return static_cast<int>(activeIndex_); }
	activeIndex_ = (activeIndex_ + 1) % cameras_.size();
	return static_cast<int>(activeIndex_);
}

int CameraManager::Prev()
{
	if (cameras_.empty()) return -1;
	if (activeIndex_ == npos) { activeIndex_ = 0; return static_cast<int>(activeIndex_); }
	activeIndex_ = (activeIndex_ + cameras_.size() - 1) % cameras_.size();
	return static_cast<int>(activeIndex_);
}

Camera* CameraManager::GetActive() const
{
	if (activeIndex_ == npos || activeIndex_ >= cameras_.size()) return nullptr;
	return cameras_[activeIndex_];
}

void CameraManager::UpdateActive(double elapsed)
{
	Camera* cam = GetActive();
	if (cam) cam->Update(elapsed);
}

void CameraManager::ApplyActive() const
{
	Camera* cam = GetActive();
	if (cam) cam->SetMatrix();
}

size_t CameraManager::FindIndex(Camera* cam) const
{
	auto it = std::find(cameras_.begin(), cameras_.end(), cam);
	if (it == cameras_.end()) return npos;
	return std::distance(cameras_.begin(), it);
}

void CameraManager::Clear()
{
	cameras_.clear();
	nameMap_.clear();
	activeIndex_ = npos;
}
