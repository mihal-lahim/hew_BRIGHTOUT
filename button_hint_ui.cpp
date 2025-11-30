#include "button_hint_ui.h"
#include "debug_text.h"
#include "player.h"

ButtonHintUI::ButtonHintUI(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, UINT screenWidth, UINT screenHeight)
	: m_debugText(nullptr), m_isVisible(false), m_displayTimer(0.0f), m_fadeOutDuration(10.0f), m_screenWidth(screenWidth), m_screenHeight(screenHeight)
{
	// 画面中央に DebugText を作成
	float centerX = static_cast<float>(screenWidth * 0.5f - 300.0f);  // テキスト幅を考慮
	float centerY = static_cast<float>(screenHeight * 0.5f - 20.0f);  // テキスト高さを考慮
	
	m_debugText = new hal::DebugText(pDevice, pContext, L"texture/consolab_ascii_512.png", screenWidth, screenHeight, centerX, centerY, 1, 256);
}

ButtonHintUI::~ButtonHintUI()
{
	if (m_debugText) {
		delete m_debugText;
		m_debugText = nullptr;
	}
}

void ButtonHintUI::ShowHint(const std::string& buttonName, const std::string& action)
{
	m_currentHint = buttonName + " : " + action;
	m_isVisible = true;
	m_displayTimer = 0.0f;
}

void ButtonHintUI::HideHint()
{
	m_isVisible = false;
}

void ButtonHintUI::Update(Player* player, double elapsed)
{
	if (!player) return;

	// プレイヤーが電柱の近くにいるか確認
	bool canTransform = player->IsNearPole();
	
	// 変身可能な状態の指示を表示
	if (canTransform) {
		// HUMAN 状態と ELECTRICITY 状態を区別する
		int stateValue = static_cast<int>(player->GetState());
		
		if (stateValue == 0) {  // HUMAN state
			// HUMAN 状態 → ELECTRICITY への変身
			ShowHint("press B ", "Change elect");
		} else {  // ELECTRICITY state
			// ELECTRICITY 状態 → HUMAN への変身
			ShowHint("press B", "change human");
		}
	} else {
		// 電柱の近くにいない場合は非表示
		m_isVisible = false;
		m_displayTimer = 0.0f;
	}

	// 表示中のテキストの透明度を計算
	if (m_isVisible) {
		m_displayTimer += static_cast<float>(elapsed);

		// 表示時間終了で自動非表示
		if (m_displayTimer > m_fadeOutDuration) {
			m_isVisible = false;
			m_displayTimer = 0.0f;
		}
	}
}

void ButtonHintUI::Draw()
{
	if (!m_isVisible || !m_debugText) return;

	m_debugText->Clear();
	
	// フェード効果を計算
	float alpha = 1.0f;
	if (m_displayTimer > m_fadeOutDuration * 0.8f) {
		// 最後の20%でフェードアウト
		float fadeTime = m_displayTimer - m_fadeOutDuration * 0.8f;
		alpha = 1.0f - (fadeTime / (m_fadeOutDuration * 0.2f));
		if (alpha < 0.0f) alpha = 0.0f;
	}

	DirectX::XMFLOAT4 color = { 1.0f, 1.0f, 0.0f, alpha };  // 黄色で表示
	m_debugText->SetText(m_currentHint.c_str(), color);
	m_debugText->Draw();
}
