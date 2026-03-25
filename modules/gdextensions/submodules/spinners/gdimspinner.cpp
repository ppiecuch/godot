/**************************************************************************/
/*  gdimspinner.cpp                                                       */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/

#include "gdimspinner.h"

#include "imspinner_imgui.h"

// Include auto-generated spinner variants
#include "imspinner/imspinner_generated.h"

#include "common/gd_core.h"
#include "core/reference.h"

// --- Spinner Node ---

void Spinner::_draw_spinner() {
	ERR_FAIL_COND(!_imgui_wnd);
	ImGui::SetCurrentWindow(_imgui_wnd);

	// Center the spinner on the node's origin (0,0)
	// SpinnerBegin computes centre = CursorPos + (radius, radius),
	// so offset CursorPos so that centre lands at (0, 0).
	_imgui_wnd->DC.CursorPos = Vector2(-_radius, -_radius);

	// Build config from node properties
	SpinnerConfig cfg;
	cfg.radius = _radius;
	cfg.thickness = _thickness;
	cfg.speed = _speed;
	cfg.color = ImColor(_color);
	cfg.bg_color = ImColor(_bg_color);
	cfg.angle = _angle;
	cfg.dots = _dots;
	cfg.mode = _mode;

	draw_spinner_variant(spinner_variant, _anim, cfg);
}

void Spinner::_notification(int p_notification) {
	switch (p_notification) {
		case NOTIFICATION_DRAW: {
			_draw_spinner();
		} break;
		case NOTIFICATION_INTERNAL_PROCESS: {
			if (spinner_active) {
				update();
			}
		} break;
	}
}

// --- Properties ---

void Spinner::set_spinner_active(bool p_active) {
	if (spinner_active != p_active) {
		spinner_active = p_active;
		set_process_internal(p_active);
		update();
	}
}

bool Spinner::get_spinner_active() const { return spinner_active; }

void Spinner::set_spinner_variant(int p_variant) {
	if (p_variant >= 0 && p_variant < SPINNER_VARIANT_COUNT && spinner_variant != p_variant) {
		spinner_variant = p_variant;
		update();
	}
}

int Spinner::get_spinner_variant() const { return spinner_variant; }

void Spinner::set_radius(real_t p_radius) {
	_radius = MAX(p_radius, 1.0f);
	update();
}

real_t Spinner::get_radius() const { return _radius; }

void Spinner::set_thickness(real_t p_thickness) {
	_thickness = MAX(p_thickness, 0.5f);
	update();
}

real_t Spinner::get_thickness() const { return _thickness; }

void Spinner::set_speed(real_t p_speed) {
	_speed = MAX(p_speed, 0.0f);
	_anim->velocity = _speed > 0 ? _speed / 4.0f : 1.0f;
	update();
}

real_t Spinner::get_speed() const { return _speed; }

void Spinner::set_color(const Color &p_color) {
	_color = p_color;
	update();
}

Color Spinner::get_color() const { return _color; }

void Spinner::set_bg_color(const Color &p_color) {
	_bg_color = p_color;
	update();
}

Color Spinner::get_bg_color() const { return _bg_color; }

void Spinner::set_angle(real_t p_angle) {
	_angle = p_angle;
	update();
}

real_t Spinner::get_angle() const { return _angle; }

void Spinner::set_dots(int p_dots) {
	_dots = MAX(p_dots, 1);
	update();
}

int Spinner::get_dots() const { return _dots; }

void Spinner::set_mode(int p_mode) {
	_mode = MAX(p_mode, 0);
	update();
}

int Spinner::get_mode() const { return _mode; }

int Spinner::get_variant_count() const { return SPINNER_VARIANT_COUNT; }

String Spinner::get_variant_name(int p_variant) const {
	ERR_FAIL_INDEX_V(p_variant, SPINNER_VARIANT_COUNT, "");
	return String(SPINNER_VARIANT_NAMES[p_variant]);
}

String Spinner::get_spinner_name() const {
	return get_variant_name(spinner_variant);
}

// --- Bindings ---

void Spinner::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_spinner_variant", "variant"), &Spinner::set_spinner_variant);
	ClassDB::bind_method(D_METHOD("get_spinner_variant"), &Spinner::get_spinner_variant);
	ClassDB::bind_method(D_METHOD("set_spinner_active", "active"), &Spinner::set_spinner_active);
	ClassDB::bind_method(D_METHOD("get_spinner_active"), &Spinner::get_spinner_active);

	ClassDB::bind_method(D_METHOD("set_radius", "radius"), &Spinner::set_radius);
	ClassDB::bind_method(D_METHOD("get_radius"), &Spinner::get_radius);
	ClassDB::bind_method(D_METHOD("set_thickness", "thickness"), &Spinner::set_thickness);
	ClassDB::bind_method(D_METHOD("get_thickness"), &Spinner::get_thickness);
	ClassDB::bind_method(D_METHOD("set_speed", "speed"), &Spinner::set_speed);
	ClassDB::bind_method(D_METHOD("get_speed"), &Spinner::get_speed);
	ClassDB::bind_method(D_METHOD("set_color", "color"), &Spinner::set_color);
	ClassDB::bind_method(D_METHOD("get_color"), &Spinner::get_color);
	ClassDB::bind_method(D_METHOD("set_bg_color", "color"), &Spinner::set_bg_color);
	ClassDB::bind_method(D_METHOD("get_bg_color"), &Spinner::get_bg_color);
	ClassDB::bind_method(D_METHOD("set_angle", "angle"), &Spinner::set_angle);
	ClassDB::bind_method(D_METHOD("get_angle"), &Spinner::get_angle);
	ClassDB::bind_method(D_METHOD("set_dots", "dots"), &Spinner::set_dots);
	ClassDB::bind_method(D_METHOD("get_dots"), &Spinner::get_dots);
	ClassDB::bind_method(D_METHOD("set_mode", "mode"), &Spinner::set_mode);
	ClassDB::bind_method(D_METHOD("get_mode"), &Spinner::get_mode);

	ClassDB::bind_method(D_METHOD("get_variant_count"), &Spinner::get_variant_count);
	ClassDB::bind_method(D_METHOD("get_variant_name", "variant"), &Spinner::get_variant_name);
	ClassDB::bind_method(D_METHOD("get_spinner_name"), &Spinner::get_spinner_name);

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "active"), "set_spinner_active", "get_spinner_active");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "variant", PROPERTY_HINT_RANGE,
						 "0," + itos(SPINNER_VARIANT_COUNT - 1)),
			"set_spinner_variant", "get_spinner_variant");

	ADD_GROUP("Style", "");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "radius", PROPERTY_HINT_RANGE, "1,200,0.5"), "set_radius", "get_radius");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "thickness", PROPERTY_HINT_RANGE, "0.5,50,0.5"), "set_thickness", "get_thickness");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "speed", PROPERTY_HINT_RANGE, "0,50,0.1"), "set_speed", "get_speed");
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "color"), "set_color", "get_color");
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "bg_color"), "set_bg_color", "get_bg_color");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "angle", PROPERTY_HINT_RANGE, "0,6.28,0.01"), "set_angle", "get_angle");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "dots", PROPERTY_HINT_RANGE, "1,64"), "set_dots", "get_dots");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "mode", PROPERTY_HINT_RANGE, "0,10"), "set_mode", "get_mode");
}

Spinner::Spinner() {
	spinner_active = true;
	spinner_variant = SPINNER_RAINBOW;
	_radius = 16.0f;
	_thickness = 2.0f;
	_speed = 4.0f;
	_color = Color(1, 1, 1, 1);
	_bg_color = Color(1, 1, 1, 0.5f);
	_angle = Math_PI;
	_dots = 12;
	_mode = 0;
	_anim = memnew(SpinnerAnimState);
	_imgui_wnd = memnew(ImGuiWindow(this));
	set_process_internal(true);
}

Spinner::~Spinner() {
	memdelete(_anim);
	memdelete(_imgui_wnd);
}

// =========================================================================
// Tests
// =========================================================================

#ifdef DOCTEST
#include "doctest/doctest.h"

TEST_SUITE("[[spinners]] Spinner properties") {
	TEST_CASE("[spinners] default values") {
		Spinner s;
		CHECK(s.get_spinner_variant() == 0);
		CHECK(s.get_spinner_active());
		CHECK(s.get_radius() == doctest::Approx(16.0f));
		CHECK(s.get_thickness() == doctest::Approx(2.0f));
		CHECK(s.get_speed() == doctest::Approx(4.0f));
		CHECK(s.get_dots() == 12);
		CHECK(s.get_mode() == 0);
	}

	TEST_CASE("[spinners] set variant") {
		Spinner s;
		s.set_spinner_variant(5);
		CHECK(s.get_spinner_variant() == 5);
	}

	TEST_CASE("[spinners] variant clamped to valid range") {
		Spinner s;
		s.set_spinner_variant(-1);
		CHECK(s.get_spinner_variant() == 0); // unchanged
		s.set_spinner_variant(99999);
		CHECK(s.get_spinner_variant() == 0); // unchanged
	}

	TEST_CASE("[spinners] set radius") {
		Spinner s;
		s.set_radius(32.0f);
		CHECK(s.get_radius() == doctest::Approx(32.0f));
	}

	TEST_CASE("[spinners] radius minimum clamped") {
		Spinner s;
		s.set_radius(0.0f);
		CHECK(s.get_radius() >= 1.0f);
	}

	TEST_CASE("[spinners] set thickness") {
		Spinner s;
		s.set_thickness(5.0f);
		CHECK(s.get_thickness() == doctest::Approx(5.0f));
	}

	TEST_CASE("[spinners] set speed") {
		Spinner s;
		s.set_speed(8.0f);
		CHECK(s.get_speed() == doctest::Approx(8.0f));
	}

	TEST_CASE("[spinners] set color") {
		Spinner s;
		s.set_color(Color(1, 0, 0));
		CHECK(s.get_color().r == doctest::Approx(1.0f));
		CHECK(s.get_color().g == doctest::Approx(0.0f));
	}

	TEST_CASE("[spinners] set bg_color") {
		Spinner s;
		s.set_bg_color(Color(0, 1, 0, 0.3f));
		CHECK(s.get_bg_color().g == doctest::Approx(1.0f));
		CHECK(s.get_bg_color().a == doctest::Approx(0.3f));
	}

	TEST_CASE("[spinners] set dots") {
		Spinner s;
		s.set_dots(8);
		CHECK(s.get_dots() == 8);
	}

	TEST_CASE("[spinners] dots minimum clamped") {
		Spinner s;
		s.set_dots(0);
		CHECK(s.get_dots() >= 1);
	}

	TEST_CASE("[spinners] set angle") {
		Spinner s;
		s.set_angle(1.57f);
		CHECK(s.get_angle() == doctest::Approx(1.57f));
	}
}

TEST_SUITE("[[spinners]] Generated integration") {
	TEST_CASE("[spinners] variant count") {
		CHECK(SPINNER_VARIANT_COUNT > 100);
		CHECK(SPINNER_VARIANT_COUNT < 300);
	}

	TEST_CASE("[spinners] variant names non-empty") {
		for (int i = 0; i < SPINNER_VARIANT_COUNT; i++) {
			CHECK(SPINNER_VARIANT_NAMES[i] != nullptr);
			CHECK(strlen(SPINNER_VARIANT_NAMES[i]) > 0);
		}
	}

	TEST_CASE("[spinners] get_variant_name") {
		Spinner s;
		CHECK(s.get_variant_count() == SPINNER_VARIANT_COUNT);
		CHECK(s.get_variant_name(0) == "SPINNER_RAINBOW");
	}

	TEST_CASE("[spinners] SpinnerConfig defaults") {
		SpinnerConfig cfg;
		CHECK(cfg.radius == doctest::Approx(16.f));
		CHECK(cfg.thickness == doctest::Approx(2.f));
		CHECK(cfg.speed == doctest::Approx(4.f));
		CHECK(cfg.dots == 12);
		CHECK(cfg.mode == 0);
	}

	TEST_CASE("[spinners] SpinnerAnimState defaults") {
		SpinnerAnimState anim;
		CHECK(anim.velocity == doctest::Approx(1.f));
		CHECK(anim.hue == 0);
	}

	TEST_CASE("[spinners] first enum is SPINNER_RAINBOW") {
		CHECK(SPINNER_RAINBOW == 0);
	}

	TEST_CASE("[spinners] known variants exist") {
		// Check some well-known spinners are in the enum
		CHECK(SPINNER_BOUNCE_BALL >= 0);
		CHECK(SPINNER_CLOCK >= 0);
		CHECK(SPINNER_PULSAR >= 0);
		CHECK(SPINNER_DOTS >= 0);
		CHECK(SPINNER_ANG >= 0);
	}
}

TEST_SUITE("[[spinners]] ImGui adapter layer") {
	TEST_CASE("[spinners] ImColor construction from floats") {
		ImColor c(1.0f, 0.5f, 0.25f, 0.8f);
		CHECK(c.Value.r == doctest::Approx(1.0f));
		CHECK(c.Value.g == doctest::Approx(0.5f));
		CHECK(c.Value.b == doctest::Approx(0.25f));
		CHECK(c.Value.a == doctest::Approx(0.8f));
	}

	TEST_CASE("[spinners] ImColor construction from ints") {
		ImColor c(255, 128, 0, 255);
		CHECK(c.Value.r == doctest::Approx(1.0f));
		CHECK(c.Value.g == doctest::Approx(128.0f / 255.0f).epsilon(0.01));
		CHECK(c.Value.b == doctest::Approx(0.0f));
	}

	TEST_CASE("[spinners] ImColor HSV") {
		ImColor c = ImColor::HSV(0.0f, 1.0f, 1.0f);
		// HSV(0, 1, 1) = red
		CHECK(c.Value.r == doctest::Approx(1.0f));
		CHECK(c.Value.g < 0.1f);
		CHECK(c.Value.b < 0.1f);
	}

	TEST_CASE("[spinners] ImColor to Godot Color conversion") {
		ImColor c(0.5f, 0.6f, 0.7f, 0.8f);
		Color gc = c;
		CHECK(gc.r == doctest::Approx(0.5f));
		CHECK(gc.g == doctest::Approx(0.6f));
		CHECK(gc.b == doctest::Approx(0.7f));
		CHECK(gc.a == doctest::Approx(0.8f));
	}

	TEST_CASE("[spinners] ImColor to ImU32 conversion") {
		ImColor c(1.0f, 0.0f, 0.0f, 1.0f);
		ImU32 u = c;
		CHECK(u != 0);
	}

	TEST_CASE("[spinners] ImGui::GetTime returns positive") {
		float t = ImGui::GetTime();
		CHECK(t >= 0.0f);
	}

	TEST_CASE("[spinners] ImGui::GetCurrentContext not null") {
		CHECK(ImGui::GetCurrentContext() != nullptr);
	}

	TEST_CASE("[spinners] ImGui::CalcTextSize returns positive") {
		ImVec2 sz = ImGui::CalcTextSize("Hello");
		CHECK(sz.x > 0);
		CHECK(sz.y > 0);
	}

	TEST_CASE("[spinners] ImGui::ColorConvertRGBtoHSV") {
		float h, s, v;
		ImGui::ColorConvertRGBtoHSV(1, 0, 0, h, s, v);
		CHECK(s == doctest::Approx(1.0f));
		CHECK(v == doctest::Approx(1.0f));
	}

	TEST_CASE("[spinners] ImGuiStorage get/set float") {
		ImGuiStorage store;
		store.SetFloat(42, 3.14f);
		CHECK(store.GetFloat(42, 0.0f) == doctest::Approx(3.14f));
	}

	TEST_CASE("[spinners] ImGuiStorage get/set int") {
		ImGuiStorage store;
		store.SetInt(99, 777);
		CHECK(store.GetInt(99, 0) == 777);
	}

	TEST_CASE("[spinners] ImGuiStorage default values") {
		ImGuiStorage store;
		CHECK(store.GetFloat(12345, -1.0f) == doctest::Approx(-1.0f));
		CHECK(store.GetInt(12345, -99) == -99);
	}

	TEST_CASE("[spinners] ImMin/ImMax/ImClamp") {
		CHECK(ImMin(3, 5) == 3);
		CHECK(ImMax(3, 5) == 5);
		CHECK(ImClamp(7, 0, 5) == 5);
		CHECK(ImClamp(-1, 0, 5) == 0);
		CHECK(ImClamp(3, 0, 5) == 3);
	}

	TEST_CASE("[spinners] ImFont has valid embedded font") {
		ImFont font;
		CHECK(font._gdFont.is_valid());
		CHECK(font._gdFont->get_size() == 12);
	}

	TEST_CASE("[spinners] ImFontGlyph has AdvanceX") {
		ImFont font;
		ImFontGlyph *g = font.FindGlyph('A');
		CHECK(g != nullptr);
		CHECK(g->AdvanceX > 0);
	}
}

#endif // DOCTEST
