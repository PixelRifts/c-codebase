/* date = September 29th 2021 10:03 am */

#ifndef VMATH_H
#define VMATH_H

#include "defines.h"

#define EPSILON 0.001f
#define PI 3.141592653589f
#define PHI 1.61803399f
#define HALF_PI 1.570796326794f
#define DEG_TO_RAD 0.0174532925f
#define RAD_TO_DEG 57.2957795131
#define FLOAT_MAX 340282346638528859811704183484516925440.0000000000000000
#define FLOAT_MIN -FLOAT_MAX

#define Color_Red vec4_init(0.8f, 0.3f, 0.2f, 1.f)
#define Color_Green vec4_init(0.2f, 0.8f, 0.3f, 1.f)
#define Color_Blue vec4_init(0.3f, 0.2f, 0.8f, 1.f)
#define Color_Magenta vec4_init(0.8f, 0.3f, 0.7f, 1.f)
#define Color_Cyan vec4_init(0.3f, 0.8f, 0.7f, 1.f)
#define Color_Yellow vec4_init(0.8f, 0.7f, 0.3f, 1.f)
#define Color_PureGreen vec4_init(0.0f, 1.0f, 0.0f, 1.f)
#define Color_PureRed vec4_init(1.0f, 0.0f, 0.0f, 1.f)
#define Color_PureBlue vec4_init(0.0f, 0.0f, 1.0f, 1.f)
#define Color_White vec4_init(1.0f, 1.0f, 1.0f, 1.f)

typedef struct vec2 { f32 x; f32 y;               } vec2;
typedef struct vec3 { f32 x; f32 y; f32 z;        } vec3;
typedef struct vec4 { f32 x; f32 y; f32 z; f32 w; } vec4;

typedef struct mat3 { f32 a[3*3]; } mat3;
typedef struct mat4 { f32 a[4*4]; } mat4;

typedef struct rect { f32 x; f32 y; f32 w; f32 h; } rect;
typedef struct quat { f32 s; f32 i; f32 j; f32 k; } quat;

//~ Math Utilities

void animate_f32exp(f32* val, f32 target, f32 speed, f32 dt);

typedef u32 axis2;
enum {
	axis2_x,
	axis2_y,
	axis2_count,
};

typedef u32 axis3;
enum {
	axis3_x,
	axis3_y,
	axis3_count,
};

//~ Inline Initializers

static inline vec2 vec2_init(f32 x, f32 y) { return (vec2) { x, y }; }
static inline vec3 vec3_init(f32 x, f32 y, f32 z) { return (vec3) { x, y, z }; }
static inline vec4 vec4_init(f32 x, f32 y, f32 z, f32 w) { return (vec4) { x, y, z, w }; }
static inline rect rect_init(f32 x, f32 y, f32 w, f32 h) { return (rect) { x, y, w, h }; }
static inline quat quat_init(f32 s, f32 i, f32 j, f32 k) { return (quat) { s, i, j, k }; }

static inline u16 mat3_idx(u16 x, u16 y) { return y * 3 + x; }
static inline u16 mat4_idx(u16 x, u16 y) { return y * 4 + x; }

static inline f64 radians(f32 deg) { return (f64) (deg * DEG_TO_RAD); }
static inline f32 degrees(f64 rad) { return (f32) (rad * RAD_TO_DEG); }

//~ Vector Functions

vec2 vec2_add(vec2 a, vec2 b);
vec2 vec2_sub(vec2 a, vec2 b);
vec2 vec2_scale(vec2 a, f32 s);
vec2 vec2_clamp(vec2 vec, rect quad);

vec3 vec3_add(vec3 a, vec3 b);
vec3 vec3_sub(vec3 a, vec3 b);
vec3 vec3_scale(vec3 a, f32 s);

vec4 vec4_add(vec4 a, vec4 b);
vec4 vec4_sub(vec4 a, vec4 b);
vec4 vec4_scale(vec4 a, f32 s);

vec3 vec3_mul(vec3 a, mat3 m);
vec4 vec4_mul(vec4 a, mat4 m);

//~ Matrix Functions

mat3 mat3_identity();
mat4 mat4_identity();

mat3 mat3_mul(mat3 a, mat3 b);
void mat3_set(mat3* mat, mat3 o);

mat3 mat3_translate(vec2 v);
mat3 mat3_rotate(f32 r);
mat3 mat3_scalev(vec2 s);
mat3 mat3_scalef(f32 s);

mat4 mat4_mul(mat4 a, mat4 b);
void mat4_set(mat4* mat, mat4 o);
mat4 mat4_transpose(mat4 a);

mat4 mat4_translate(vec3 v);
mat4 mat4_scale(vec3 v);
mat4 mat4_rotX(f32 deg);
mat4 mat4_rotY(f32 deg);
mat4 mat4_rotZ(f32 deg);
mat4 mat4_ortho(f32 left, f32 right, f32 top, f32 bottom, f32 near, f32 far);
mat4 mat4_perspective(f32 fov, f32 aspect_ratio, f32 near, f32 far);

//~ Quaternion Functions

quat quat_identity();

quat quat_mul(quat a, quat b);
f32  quat_length(quat q);
quat quat_norm(quat q);
quat quat_rotate_axis(quat q, f32 x, f32 y, f32 z, f32 a);
quat quat_from_euler(f32 yaw, f32 pitch, f32 roll);
mat4 quat_to_rotation_mat(quat q);

//~ Rect Functions

b8   rect_contains_point(rect a, vec2 p);
b8   rect_overlaps(rect a, rect b);
b8   rect_contained_by_rect(rect a, rect b);
rect rect_get_overlap(rect a, rect b);
rect rect_uv_cull(rect quad, rect uv, rect cull_quad);

#endif //VMATH_H
