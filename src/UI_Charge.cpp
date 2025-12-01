//////////////////////////////
// UI.h
// UI Šî’êƒNƒ‰ƒX‚Æ UI ŠÇ—ƒNƒ‰ƒX
//Author:hiroshi kasiwagi
//Date:2025/11/27
//////////////////////////////

#include "UI_Charge.h"
#include "player.h"
#include "texture.h"
#include "sprite.h"
#include "direct3d.h"
#include <algorithm>

extern Player* g_player; // defined in game.cpp

static int g_NumberTex = -1;

// helper to draw an integer using the project's digit sprite sheet
static void DrawNumber(int texId, int number, float x, float y, float h)
{
	if (texId < 0) return;
	// digit sizing
	float digitH = h;
	float digitW = digitH * 0.7f; // adjust to fit

	// source tile height in sprite (approx)
	int srcH = (int)digitH;
	int srcW = (int)digitW;
	int srcY = 100; // as used elsewhere in project

	if (number == 0) {
		int d = 0;
		int srcX = d * srcW;
		Sprite_Draw(texId, x, y, digitW, digitH, srcX, srcY, srcW, srcH);
		return;
	}

	char buf[16];
	int len = 0;
	int n = number;
	while (n > 0 && len < 15) {
		buf[len++] = '0' + (n % 10);
		n /= 10;
	}
	if (len == 0) return;

	float px = x;
	for (int i = len - 1; i >= 0; --i) {
		int d = buf[i] - '0';
		int srcX = d * srcW;
		Sprite_Draw(texId, px, y, digitW, digitH, srcX, srcY, srcW, srcH);
		px += digitW;
	}
}

void UI_Charge::Initialize()
{
	// ƒeƒNƒXƒ`ƒƒ“Ç‚İ‚İi”wŒi‚Æ“h‚èj
	texBg_ = Texture_Load(L"texture/ui_charge_bg.png");
	texFill_ = Texture_Load(L"texture/ui_charge_fill.png");
	if (g_NumberTex < 0) g_NumberTex = Texture_Load(L"texture/ScoreNumberB.png");
}

void UI_Charge::Update(double elapsedTime)
{
	if (g_player) {
		int h = g_player->GetHealth();
		int mh = g_player->GetMaxHealth();
		if (mh <= 0) mh = 1;
		chargeAmount_ = static_cast<float>(h) / static_cast<float>(mh);
	}
}

void UI_Charge::Draw() const
{
	//// Draw at bottom-left with margin
	//const float margin = 10.0f;
	//const float screenW = (float)Direct3D_GetBackBufferWidth();
	//const float screenH = (float)Direct3D_GetBackBufferHeight();

	//float x = margin; // left margin
	//float h = size_.y;
	//float w = size_.x;
	//float y = screenH - h - margin; // bottom-left origin

	//// ”wŒi‚ğ•`‚­
	//if (texBg_ >= 0) Sprite_Draw(texBg_, x, y, w, h, 0, 0, (int)w, (int)h);

	////[“U‚ğ•`‚­
	//float fillRatio = std::max(0.0f, std::min(1.0f, chargeAmount_));
	//float innerW = w - padding_ * 2.0f;
	//float drawW = innerW * fillRatio;
	//if (texFill_ >= 0) {
	//	if (drawW > 1.0f) {
	//		Sprite_Draw(texFill_, x + padding_, y + padding_, drawW, h - padding_ * 2.0f, 0, 0, (int)drawW, (int)(h - padding_ * 2.0f));
	//	}
	//	else {
	//		Sprite_Draw(texFill_, x + padding_, y + padding_, 1.0f, h - padding_ * 2.0f, 0, 0, 1, (int)(h - padding_ * 2.0f));
	//	}
	//}

	//// ”’l•\¦: current / max ‚ğƒQ[ƒW‰E‘¤‚É•\¦
	//if (g_player && g_NumberTex >= 0) {
	//	int cur = g_player->GetHealth();
	//	int mx = g_player->GetMaxHealth();
	//	float numH = h; // digit height
	//	float numX = x + w + 8.0f; // to the right of gauge
	//	float numY = y;
	//	DrawNumber(g_NumberTex, cur, numX, numY, numH);
	//	// small gap
	//	float afterCurX = numX + (numH * 0.7f) * (cur == 0 ? 1 : (int)log10(std::max(1, cur)) + 1) + 4.0f;
	//	DrawNumber(g_NumberTex, mx, afterCurX, numY, numH);
	//}
}
