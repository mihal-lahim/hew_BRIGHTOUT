/*==============================================================================

   ÉâÉCÉgÇÃêßå‰ [light.h]
														 Author : sumi rintarou
														 Date   : 2025/11/06
--------------------------------------------------------------------------------

==============================================================================*/
#ifndef LIGHT_H
#define LIGHT_H

#include <DirectXMath.h>


void Light_Initialize();
void Light_Finalize();

void Light_SetAmbient(const DirectX::XMFLOAT4& color);
void Light_SetDiffuse(const DirectX::XMFLOAT3& color, const DirectX::XMFLOAT3& direction);// ïΩçsåıåπ

struct DiffuseLight
{
	DirectX::XMFLOAT4 color;
	DirectX::XMFLOAT4 direction;
};

void Light_SetDiffuse(const DiffuseLight& light);


#endif	// LIGHT_H