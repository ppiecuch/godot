#ifndef VAT_ANIMATION_TRACK_H
#define VAT_ANIMATION_TRACK_H

#include "core/reference.h"

class VATAnimationTrack : public Reference {
	GDCLASS(VATAnimationTrack, Reference);

	String name;
	int start_frame;
	int end_frame;
	int framerate;
	bool is_looping;

protected:
	static void _bind_methods();

public:
	void set_track(const String &p_name, int p_start, int p_end, int p_framerate, bool p_loop);

	void set_track_name(const String &p_name);
	String get_track_name() const;

	void set_start_frame(int p_frame);
	int get_start_frame() const;

	void set_end_frame(int p_frame);
	int get_end_frame() const;

	void set_framerate(int p_framerate);
	int get_framerate() const;

	void set_is_looping(bool p_loop);
	bool get_is_looping() const;

	String to_string();

	VATAnimationTrack();
};

#endif // VAT_ANIMATION_TRACK_H
