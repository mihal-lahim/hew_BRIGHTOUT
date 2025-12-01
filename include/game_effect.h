/*==============================================================================

   エフェクト制御[game_effect.h]
														 Author : sumi rintarou
														 Date   : 2025/09/19
--------------------------------------------------------------------------------

==============================================================================*/
#ifndef GAME_EFFECT_H
#define	GAME_EFFECT_H

#include <DirectXMath.h>


void Effect_Initialize();
void Effect_Finalize();
void Effect_Update(double elapsed_time);
void Effect_Draw();

void Effect_Create(const DirectX::XMFLOAT2& position,float size, double lifeTime,const DirectX::XMFLOAT4& color);

#endif // GAME_EFFECT_H
