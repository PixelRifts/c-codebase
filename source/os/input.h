/* date = July 15th 2022 5:07 pm */

#ifndef INPUT_H
#define INPUT_H

#define Input_Press 1
#define Input_Release 2
#define Input_Repeat 3

#ifdef PLATFORM_WIN
#  include "impl/win32_key_codes.h"
#else
#  error "Not Implemented YET"
#endif

void __OS_InputKeyCallback(u8 key, i32 action);
void __OS_InputButtonCallback(i32 button, i32 action);
void __OS_InputCursorPosCallback(f32 xpos, f32 ypos);
void __OS_InputScrollCallback(f32 xscroll, f32 yscroll);
void __OS_InputReset();

b32 OS_InputKey(i32 key);
b32 OS_InputKeyPressed(i32 key);
b32 OS_InputKeyReleased(i32 key);
b32 OS_InputKeyHeld(i32 key);
b32 OS_InputButton(i32 button);
b32 OS_InputButtonPressed(i32 button);
b32 OS_InputButtonReleased(i32 button);
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
