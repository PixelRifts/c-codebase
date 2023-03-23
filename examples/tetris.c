
//~
//
// Tetris Example (Followed OLC's tutorial and translated to c)
// Optional Layers Required: Render2D
//
//~

#include "defines.h"
#include "os/os.h"
#include "os/window.h"
#include "os/input.h"
#include "base/tctx.h"
#include "core/backend.h"
#include "core/resources.h"

#include "opt/render_2d.h"

#include <stdlib.h>

Array_Prototype(i32_array, i32);
Array_Impl(i32_array, i32);

string_array tetrominos = {0};
i32 field_width = 12;
i32 field_height = 18;
u8* field;

f32 xoff = 50;
f32 yoff = 50;
f32 block_size = 20;

i32 index_with_rotate(i32 x, i32 y, i32 r) {
	switch (r % 4) {
		case 0: return y * 4 + x;
		case 1: return 12 + y - (x * 4);
		case 2: return 15 - (y * 4) - x;
		case 3: return 3 - y + (x * 4);
	}
	return 0;
}

b8 piece_fit_test(u8 id, i32 r, i32 pos_x, i32 pos_y) {
	for (i32 y = 0; y < 4; y++) {
		for (i32 x = 0; x < 4; x++) {
			i32 piece_index = index_with_rotate(x, y, r);
			
			i32 field_index = (pos_y + y) * field_width + (pos_x + x);
			if (pos_x + x >= 0 && pos_x + x < field_width &&
				pos_y + y >= 0 && pos_y + y < field_height) {
				if (tetrominos.elems[id].str[piece_index] == 'X' && field[field_index] != 0)
					return false;
			}
		}
	}
	return true;
}

int main() {
	OS_Init();
	
	ThreadContext context = {0};
	tctx_init(&context);
	
	M_Arena* global_arena = arena_make();
	
	OS_Window* window = OS_WindowCreate(1080, 720, str_lit("This should work"));
	B_BackendInit(window);
	OS_WindowShow(window);
	
	R2D_Renderer renderer = {0};
	R2D_Init(window, &renderer);
	
	vec4 colors[] = {
		(vec4) {0},
		Color_Red,
		Color_Green,
		Color_Blue,
		Color_Cyan,
		Color_Magenta,
		Color_Yellow,
		Color_PureRed,
		(vec4) {0},
		Color_White,
	};
	
	string_array_add(&tetrominos, str_lit("..X." "..X." "..X." "..X."));
	string_array_add(&tetrominos, str_lit("..X." ".XX." ".X.." "...."));
	string_array_add(&tetrominos, str_lit(".X.." ".XX." "..X." "...."));
	string_array_add(&tetrominos, str_lit("...." ".XX." ".XX." "...."));
	string_array_add(&tetrominos, str_lit("..X." ".XX." "..X." "...."));
	string_array_add(&tetrominos, str_lit("...." ".XX." "..X." "..X."));
	string_array_add(&tetrominos, str_lit("...." ".XX." ".X.." ".X.."));
	
	field = arena_alloc(global_arena, sizeof(u8) * field_width * field_height);
	
	for (u32 y = 0; y < field_height; y++) {
		for (u32 x = 0; x < field_width; x++) {
			field[y * field_width + x] =
			(x == 0 || x == field_width - 1 || y == field_height - 1)
				? 9 : 0;
		}
	}
	
	srand(OS_TimeMicrosecondsNow());
	
	b8 game_over = false;
	i32 current_piece_id = rand() % 7;
	i32 current_rotation = 0;
	i32 current_x = (field_width / 2) - 1, current_y = 0;
	
	f32 start, end, dt;
	start = OS_TimeMicrosecondsNow();
	
	f32 speed = 2;
	f32 speed_acc = 0;
	
	while (OS_WindowIsOpen(window) && !game_over) {
		OS_PollEvents();
		
		end = OS_TimeMicrosecondsNow();
		dt = (end - start) / 1e6;
		start = OS_TimeMicrosecondsNow();
		
		current_x -= (OS_InputKeyPressed(Input_Key_LeftArrow) || OS_InputKeyHeld(Input_Key_LeftArrow)) && piece_fit_test(current_piece_id, current_rotation, current_x - 1, current_y);
		current_x += (OS_InputKeyPressed(Input_Key_RightArrow) || OS_InputKeyHeld(Input_Key_RightArrow)) && piece_fit_test(current_piece_id, current_rotation, current_x + 1, current_y);
		current_y += 
		(OS_InputKeyPressed(Input_Key_DownArrow) || OS_InputKeyHeld(Input_Key_DownArrow)) && piece_fit_test(current_piece_id, current_rotation, current_x, current_y + 1);
		current_rotation += (OS_InputKeyPressed('Z') || OS_InputKeyHeld('Z')) && piece_fit_test(current_piece_id, current_rotation + 1, current_x, current_y);
		
		speed_acc += dt;
		if (speed_acc >= speed) {
			if (piece_fit_test(current_piece_id, current_rotation, current_x, current_y + 1))
				current_y++;
			else {
				for (u32 y = 0; y < 4; y++) {
					for (u32 x = 0; x < 4; x++) {
						if (tetrominos.elems[current_piece_id]
							.str[index_with_rotate(x, y, current_rotation)] == 'X') {
							field[(current_y + y) * field_width + (current_x + x)] = current_piece_id+1;
						}
					}
				}
				
				i32_array fixed_lines = {0};
				for (u32 y = 0; y < 4; y++) {
					if (current_y + y < field_height - 1) {
						b8 line_destroyed = true;
						for (u32 x = 1; x < field_width - 1; x++) {
							line_destroyed &= field[(current_y + y) * field_width + x] != 0;
						}
						if (line_destroyed) {
							for (u32 x = 1; x < field_width - 1; x++) {
								field[(current_y + y) * field_width + x] = 0;
							}
							i32_array_add(&fixed_lines, current_y + y);
						}
					}
				}
				Iterate(fixed_lines,  i) {
					i32 line = fixed_lines.elems[i];
					for (u32 x = 1; x < field_width - 1; x++) {
						for (i32 y = line; y > 0; y--) {
							field[y * field_width + x] = field[(y - 1) * field_width + x];
						}
						field[x] = 0;
					}
				}
				i32_array_free(&fixed_lines);
				
				current_x = (field_width / 2) - 1, current_y = 0;
				current_piece_id = rand() % 7;
				current_rotation = 0;
				
				if (!piece_fit_test(current_piece_id, current_rotation, current_x, current_y + 1)) {
					game_over = true;
				} else {
					speed = Clamp(0.3, speed - 0.1, 3);
				}
			}
			
			speed_acc = 0;
		}
		
		R_Clear(BufferMask_Color);
		
		R2D_BeginDraw(&renderer);
		for (u32 y = 0; y < field_height; y++) {
			for (u32 x = 0; x < field_width; x++) {
				if (field[y * field_width + x])
					R2D_DrawQuadC(&renderer,
								  (rect) { xoff + x * block_size, yoff + y * block_size, block_size, block_size }, colors[field[y * field_width + x]]);
			}
		}
		
		for (u32 y = 0; y < 4; y++) {
			for (u32 x = 0; x < 4; x++) {
				if (tetrominos.elems[current_piece_id]
					.str[index_with_rotate(x, y, current_rotation)] == 'X')
					R2D_DrawQuadC(&renderer,
								  (rect) { xoff + (current_x + x) * block_size, yoff + (current_y + y) * block_size, block_size, block_size },
								  colors[current_piece_id + 1]);
			}
		}
		R2D_EndDraw(&renderer);
		
		B_BackendSwapchainNext(window);
	}
	
	R2D_Free(&renderer);
	
	B_BackendFree(window);
	OS_WindowClose(window);
	
	arena_free(global_arena);
	
	tctx_free(&context);
}
