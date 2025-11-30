/*==============================================================================

   ÉLÉÖÅ[ÉuÇÃï`âÊ [cube.h]
														 Author : sumi rintarou
														 Date   : 2025/10/16
--------------------------------------------------------------------------------

==============================================================================*/
#ifndef CUBE_H
#define CUBE_H

#include <d3d11.h>
#include <DirectXMath.h>

void Cube_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void Cube_Finalize();
void Cube_Draw(const int texture,const DirectX::XMMATRIX& mtxWorld);


#endif	// CUBE_H