#ifndef IMAGE_TOOLS_H
#define IMAGE_TOOLS_H

#include "core/dictionary.h"
#include "core/image.h"
#include "core/vector.h"

class ImageTools {
public:
	static Ref<Image> neighbor_tracing(const Image *p_src);
	static void fix_alpha_edges(Image *p_src);
	static void fix_tex_bleed(Image *p_src);
	static void normalmap_to_xy(Image *p_src);
	static void bumpmap_to_normalmap(Image *p_src, float p_bump_scale = 1.0);
	static Vector<Rect2> unpack_region(const Image *p_src, real_t p_distance_between_tiles_perc = 1, real_t p_minimum_tile_area_to_save_perc = 1, real_t p_alpha_threshold = 0.2, Ref<Image> p_debug_image = Ref<Image>());
	enum SeamlessAxis {
		FE_XY,
		FE_X,
		FE_Y,
	};
	enum SeamlessStampMode {
		FE_STAMPING,
		FE_SPLATMODE,
	};
	static Ref<Image> make_seamless(const Image *p_src, SeamlessStampMode p_stamp_mode = FE_STAMPING, real_t p_stamper_radius = 0.45, real_t p_stamp_density = 0.4, real_t p_hardness = 0.6, real_t p_stamp_noise_mask = 1, real_t p_randomize = 0.25, int p_stamp_rotate = 1, SeamlessAxis p_to_loop = FE_XY);
};

#endif // IMAGE_TOOLS_H
