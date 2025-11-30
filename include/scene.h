/*==============================================================================

   ÉVÅ[Éìä«óù[scene.h]
														 Author : sumi rintarou
														 Date   : 2025/09/17
--------------------------------------------------------------------------------

==============================================================================*/
#ifndef SCENE_H
#define SCENE_H

void Scene_Initialize();
void Scene_Finalize();
void Scene_Update(double elapsed_time);
void Scene_Draw();

enum Scene
{
	SCENE_TITLE,
	SCENE_GAME,
	SCENE_RESULT,
	SCENE_MAX
};

void Scene_SetNextScene(Scene scene);
void Scene_ChangeScene();

#endif // SCENE_H