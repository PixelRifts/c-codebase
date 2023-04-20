/* date = July 15th 2022 5:07 pm */

#ifndef INPUT_H
#define INPUT_H

// Common KeyCodes
#include "key_codes.h"

#if defined(PLATFORM_WIN)
#  include "impl/win32_key_code_translate.h"
#elif defined(PLATFORM_LINUX)
#  include "impl/x11_key_code_translate.h"
#endif

void __OS_InputKeyCallback(u8 key, i32 action);
b8   __OS_InputKeyCallbackCheckRepeat(u8 key, i32 action);
void __OS_InputButtonCallback(u8 button, i32 action);
void __OS_InputCursorPosCallback(f32 xpos, f32 ypos);
void __OS_InputScrollCallback(f32 xscroll, f32 yscroll);
void __OS_InputReset(void);

b32 OS_InputKey(u8 key);
b32 OS_InputKeyPressed(u8 key);
b32 OS_InputKeyReleased(u8 key);
b32 OS_InputKeyHeld(u8 key);
b32 OS_InputButton(u8 button);
b32 OS_InputButtonPressed(u8 button);
b32 OS_InputButtonReleased(u8 button);
f32 OS_InputGetMouseX();
f32 OS_InputGetMouseY();
f32 OS_InputGetMouseScrollX();
f32 OS_InputGetMouseScrollY();
f32 OS_InputGetMouseAbsoluteScrollX();
f32 OS_InputGetMouseAbsoluteScrollY();
f32 OS_InputGetMouseDX();
f32 OS_InputGetMouseDY();
f32 OS_InputGetMouseRecordedX();
f32 OS_InputGetMouseRecordedY();

#endif //INPUT_H
