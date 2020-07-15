#include "blitter.h"

_Blitter *_Blitter::singleton = NULL;

void _Blitter::_bind_methods() {
    ClassDB::bind_method(D_METHOD("calculate_new_rect_from_mins_max", "p_base_size", "p_mins", "p_max"), &_Blitter::calculate_new_rect_from_mins_max);
    ClassDB::bind_method(D_METHOD("calculate_new_size_from_mins_max", "p_base_size", "p_mins", "p_max"), &_Blitter::calculate_new_size_from_mins_max);
	ClassDB::bind_method(D_METHOD("blit_rect_modulate", "p_src:Image", "p_src_rect", "p_dest_point", "p_modulate"), &_Blitter::blit_rect_modulate);
	ClassDB::bind_method(D_METHOD("blit_rect_modulate_inverted_alpha", "p_src:Image", "p_src_rect", "p_dest_point", "p_modulate"), &_Blitter::blit_rect_modulate_inverted_alpha);
	ClassDB::bind_method(D_METHOD("blit_rect_modulate_inverted_alpha_translucent", "p_src:Image", "p_src_rect", "p_dest_point", "p_modulate"), &_Blitter::blit_rect_modulate_inverted_alpha_translucent);
}

_Blitter *_Blitter::get_singleton() {
	return singleton;
}

Rect2 _Blitter::calculate_new_rect_from_mins_max(const Vector2 &p_base_size, const Vector2 &p_mins, const Vector2 &p_max) {
	return Blitter::calculate_new_rect_from_mins_max(p_base_size, p_mins, p_max);
}

Vector2 _Blitter::calculate_new_size_from_mins_max(const Vector2 &p_base_size, const Vector2 &p_mins, const Vector2 &p_max) {
	return Blitter::calculate_new_size_from_mins_max(p_base_size, p_mins, p_max);
}

Ref<Image> _Blitter::blit_rect_modulate(const Ref<Image> p_src, const Rect2& p_src_rect, const Ref<Image> p_dest, const Point2& p_dest_point, const Color &p_modulate) {
	return Blitter::blit_rect<true, false, false>(p_src, p_src_rect, p_dest, p_dest_point, p_modulate);
}

Ref<Image> _Blitter::blit_rect_modulate_inverted_alpha(const Ref<Image> p_src, const Rect2& p_src_rect, const Ref<Image> p_dest, const Point2& p_dest_point, const Color &p_modulate) {
	return Blitter::blit_rect<true, true, false>(p_src, p_src_rect, p_dest, p_dest_point, p_modulate);
}

Ref<Image> _Blitter::blit_rect_modulate_inverted_alpha_translucent(const Ref<Image> p_src, const Rect2& p_src_rect, const Ref<Image> p_dest, const Point2& p_dest_point, const Color &p_modulate) {
	return Blitter::blit_rect<true, true, true>(p_src, p_src_rect, p_dest, p_dest_point, p_modulate);
}

_Blitter::_Blitter() {
	singleton = this;
}
