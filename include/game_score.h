/*==============================================================================

   スコアの管理と表示 [game_score.h]
														 Author : sumi rintarou
														 Date   : 2025/09/05
--------------------------------------------------------------------------------

==============================================================================*/
#ifndef GAME_SCORE_H
#define GAME_SCORE_H

void Score_Initialize(float x,float y,int dight);
void Score_Finalize();
void Score_Draw();
void Score_Update();

void Score_AddScore(int score);
int GetScore();

#endif // Game_SCORE_H
