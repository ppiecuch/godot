#undef INCBIN_PREFIX
#define INCBIN_PREFIX
#define INCBIN_STYLE INCBIN_STYLE_SNAKE
#define INCBIN_SILENCE_BITCODE_WARNING

#include "misc/incbin.h"

INCBIN(bitmap_font, "benchmark/resources/atari-font.bmp");

INCBIN(texture_cube_ktx, "benchmark/resources/textures/cube.ktx");
INCBIN(texture_cube_png, "benchmark/resources/textures/cube.png");
INCBIN(texture_frog_ktx, "benchmark/resources/textures/frog.ktx");
INCBIN(texture_frog_png, "benchmark/resources/textures/frog.png");
INCBIN(texture_kid_ktx, "benchmark/resources/textures/kid.ktx");
INCBIN(texture_kid_png, "benchmark/resources/textures/kid.png");
INCBIN(texture_robot_ktx, "benchmark/resources/textures/robot.ktx");
INCBIN(texture_robot_png, "benchmark/resources/textures/robot.png");
INCBIN(texture_trex_ktx, "benchmark/resources/textures/trex.ktx");
INCBIN(texture_trex_png, "benchmark/resources/textures/trex.png");

INCBIN(model_cube_dat, "benchmark/resources/models/cube.dat");
INCBIN(model_from_dat, "benchmark/resources/models/frog.dat");
INCBIN(model_kid_dat, "benchmark/resources/models/kid.dat");
INCBIN(model_robot_dat, "benchmark/resources/models/robot.dat");
INCBIN(model_trex_dat, "benchmark/resources/models/trex.dat");

INCBIN(shader_flat_frag, "benchmark/resources/shaders/flat.shader");
INCBIN(shader_gouraud_frag, "benchmark/resources/shaders/gouraud.shader");
INCBIN(shader_phong_frag, "benchmark/resources/shaders/phong.shader");
INCBIN(shader_text_frag, "benchmark/resources/shaders/text.shader");
INCBIN(shader_untextured_gouraud_frag, "benchmark/resources/shaders/untextured-gouraud.shader");

INCBIN(postprocess_bloom_fs, "benchmark/resources/postprocess/bloom.fs");
INCBIN(postprocess_blur_fs, "benchmark/resources/postprocess/blur.fs");
INCBIN(postprocess_cross_hatching_fs, "benchmark/resources/postprocess/cross_hatching.fs");
INCBIN(postprocess_cross_stitching_fs, "benchmark/resources/postprocess/cross_stitching.fs");
INCBIN(postprocess_dream_vision_fs, "benchmark/resources/postprocess/dream_vision.fs");
INCBIN(postprocess_fisheye_fs, "benchmark/resources/postprocess/fisheye.fs");
INCBIN(postprocess_fxaa_frag_fs, "benchmark/resources/postprocess/fxaa.frag.fs");
INCBIN(postprocess_fxaa_vert_fs, "benchmark/resources/postprocess/fxaa.vert.fs");
INCBIN(postprocess_grayscale_fs, "benchmark/resources/postprocess/grayscale.fs");
INCBIN(postprocess_pixelizer_fs, "benchmark/resources/postprocess/pixelizer.fs");
INCBIN(postprocess_posterization_fs, "benchmark/resources/postprocess/posterization.fs");
INCBIN(postprocess_predator_fs, "benchmark/resources/postprocess/predator.fs");
INCBIN(postprocess_scanlines_fs, "benchmark/resources/postprocess/scanlines.fs");
INCBIN(postprocess_sobel_fs, "benchmark/resources/postprocess/sobel.fs");
