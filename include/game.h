/*==============================================================================

   ゲーム.h [game.h]
														 Author : sumi rintarou
														 Date   : 2025/06/13
--------------------------------------------------------------------------------

==============================================================================*/
#ifndef GAME_H
#define GAME_H

void Game_Initialize();
void Game_Finalize();
void Game_Update(double elapsed_time);
void Game_Draw();

// プレイヤー数を設定（Game_Initialize より前に呼ぶ）
void Game_SetPlayerCount(int count);

#endif // GAME_H
