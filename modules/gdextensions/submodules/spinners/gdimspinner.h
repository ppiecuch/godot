/**************************************************************************/
/*  gdimspinner.h                                                         */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/

#ifndef GDSPINNER_H
#define GDSPINNER_H

#include "scene/2d/node_2d.h"

// Forward declarations
struct ImGuiWindow;
struct SpinnerConfig;
struct SpinnerAnimState;

class Spinner : public Node2D {
	GDCLASS(Spinner, Node2D);

	ImGuiWindow *_imgui_wnd;

	int spinner_variant;
	bool spinner_active;

	// Configurable parameters (exposed to GDScript)
	real_t _radius;
	real_t _thickness;
	real_t _speed;
	Color _color;
	Color _bg_color;
	real_t _angle;
	int _dots;
	int _mode;

	// Animation state
	SpinnerAnimState *_anim;

	void _draw_spinner();

protected:
	static void _bind_methods();
	void _notification(int p_notification);

public:
	void set_spinner_active(bool p_active);
	bool get_spinner_active() const;

	void set_spinner_variant(int p_variant);
	int get_spinner_variant() const;

	void set_radius(real_t p_radius);
	real_t get_radius() const;

	void set_thickness(real_t p_thickness);
	real_t get_thickness() const;

	void set_speed(real_t p_speed);
	real_t get_speed() const;

	void set_color(const Color &p_color);
	Color get_color() const;

	void set_bg_color(const Color &p_color);
	Color get_bg_color() const;

	void set_angle(real_t p_angle);
	real_t get_angle() const;

	void set_dots(int p_dots);
	int get_dots() const;

	void set_mode(int p_mode);
	int get_mode() const;

	int get_variant_count() const;
	String get_variant_name(int p_variant) const;
	String get_spinner_name() const;

	Spinner();
	~Spinner();
};

#endif // GDSPINNER_H
