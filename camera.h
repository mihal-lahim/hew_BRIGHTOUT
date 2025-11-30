/*==============================================================================

   ÉJÉÅÉâ [camera.h]
														 Author : sumi rintarou
														 Date   : 2025/10/28
--------------------------------------------------------------------------------

==============================================================================*/
#ifndef CAMERA_H
#define CAMERA_H

class Camera
{
public:
	virtual ~Camera() = default;

	virtual void Update(double elapsed_time) {};
	virtual void SetMatrix() const = 0;
};

#endif // CAMERA_H