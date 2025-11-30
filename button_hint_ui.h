#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <string>

namespace hal
{
	class DebugText;
}

class Player;
class House;

// ボタン指示 UI クラス
class ButtonHintUI
{
public:
	ButtonHintUI(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, UINT screenWidth, UINT screenHeight);
	~ButtonHintUI();

	void Update(Player* player, double elapsed);
	void Draw();

	// 画面中央にボタン指示を表示
	void ShowHint(const std::string& buttonName, const std::string& action);
	
	// ボタン指示を非表示
	void HideHint();

private:
	hal::DebugText* m_debugText;
	std::string m_currentHint;
	bool m_isVisible;
	float m_displayTimer;
	float m_fadeOutDuration;
	
	UINT m_screenWidth;
	UINT m_screenHeight;
};
