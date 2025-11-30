/*==============================================================================

   キーボード入力 [key_logger.h]
														 Author : sumi rintarou
														 Date   : 2025/06/27
--------------------------------------------------------------------------------

==============================================================================*/

#ifndef KEY_LOGGER_H
#define KEY_LOGGER_H
#include "keyboard.h"

void KeyLogger_Initialize();
void KeyLogger_Update();
bool KeyLogger_IsPressed(Keyboard_Keys key);
bool KeyLogger_IsTrigger(Keyboard_Keys key);
bool KeyLogger_IsRelease(Keyboard_Keys key);


#endif // KEY_LOGGER_H
