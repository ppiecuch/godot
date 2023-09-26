
#undef INCBIN_PREFIX
#define INCBIN_PREFIX
#define INCBIN_STYLE INCBIN_STYLE_SNAKE
#define INCBIN_SILENCE_BITCODE_WARNING

#include "misc/incbin.h"

#define ROOT "modules/gdextensions/submodules/environment/proc_rocks/generators/procrockgen/"

INCBIN(json_1, ROOT "demo/example/1.json");
INCBIN(json_2, ROOT "demo/example/2.json");
INCBIN(json_3, ROOT "demo/example/3.json");
INCBIN(json_4, ROOT "demo/example/4.json");
INCBIN(json_5, ROOT "demo/example/5.json");
INCBIN(json_6, ROOT "demo/example/6.json");
INCBIN(json_7, ROOT "demo/example/7.json");
INCBIN(json_8, ROOT "demo/example/8.json");
INCBIN(json_9, ROOT "demo/example/9.json");
INCBIN(json_10, ROOT "demo/example/10.json");
INCBIN(json_11, ROOT "demo/example/11.json");
INCBIN(json_12, ROOT "demo/example/12.json");
INCBIN(json_granite_custom, ROOT "demo/example/granite_custom.json");

INCBIN(gravel_albedo_jpg, ROOT "demo/textures/gravel/albedo.jpg");
INCBIN(gravel_ambientOcc_jpg, ROOT "demo/textures/gravel/ambientOcc.jpg");
INCBIN(gravel_displacement_jpg, ROOT "demo/textures/gravel/displacement.jpg");
INCBIN(gravel_normals_jpg, ROOT "demo/textures/gravel/normals.jpg");
INCBIN(gravel_roughness_jpg, ROOT "demo/textures/gravel/roughness.jpg");
INCBIN(moss_albedo_jpg, ROOT "demo/textures/mossy/albedo.jpg");
INCBIN(moss_ambientOcc_jpg, ROOT "demo/textures/mossy/ambientOcc.jpg");
INCBIN(moss_displacement_jpg, ROOT "demo/textures/mossy/displacement.jpg");
INCBIN(moss_normals_jpg, ROOT "demo/textures/mossy/normals.jpg");
INCBIN(moss_roughness_jpg, ROOT "demo/textures/mossy/roughness.jpg");
INCBIN(rock_jpg, ROOT "demo/textures/rock/rock.jpg");
