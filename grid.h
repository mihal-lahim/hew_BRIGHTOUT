/*==============================================================================

   ƒOƒŠƒbƒh‚Ì•`‰æ [grid.h]
														 Author : sumi rintarou
														 Date   : 2025/10/21
--------------------------------------------------------------------------------

==============================================================================*/
#ifndef GRID_H
#define GRID_H

#include <d3d11.h>

void Grid_Initialize(int x_count, int z_count, float size);

//void Grid_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void Grid_Finalize();
void Grid_Update(double elapsed_time);
void Grid_Draw();


#endif	// GRID_H