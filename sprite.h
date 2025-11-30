#pragma once
/*==============================================================================

   ƒXƒvƒ‰ƒCƒg•`‰æ [sprite.h]
														 Author : Youhei Sato
														 Date   : 2025/05/15
--------------------------------------------------------------------------------

==============================================================================*/
#ifndef SPRITE_H
#define SPRITE_H
#include <DirectXMath.h>

#include "DirectXTex.h"
using namespace DirectX;

#include <d3d11.h>

void Sprite_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void Sprite_Finalize(void);
//void Sprite_Draw(int texid, float x, float y, DirectX::XMFLOAT4 color = { 1.0f, 1.0f, 1.0f, 1.0f });

//void Sprite_Draw(int texid, float x, float y, XMFLOAT4 color = { 1.0f, 1.0f, 1.0f, 1.0f });// ŠÈˆÕ”Å
void Sprite_Draw(int texid, float x, float y, float zoom = 1.0f, XMFLOAT4 color = { 1.0f, 1.0f, 1.0f, 1.0f });//”{—¦‚ ‚èŠÈˆÕ”Å
void Sprite_Draw(int texid, float x, float y, float w,float h, int tx, int ty, int tw, int th,DirectX::XMFLOAT4 color = { 1.0f, 1.0f, 1.0f, 1.0f });
void Sprite_Draw(int texid, float x, float y, float w, float h, int tx, int ty, int tw, int th, float angle, DirectX::XMFLOAT4 color = { 1.0f, 1.0f, 1.0f, 1.0f });
//void Sprite_Draw(int texid, float x, float y, float w, float h, int tx, int ty, int tw, int th, float bairitu, DirectX::XMFLOAT4 color = { 1.0f, 1.0f, 1.0f, 1.0f });



#endif // SPRITE_H
