#include "vat_animation_track.h"

#ifdef DOCTEST
#include "doctest/doctest.h"
#else
#define DOCTEST_CONFIG_DISABLE
#endif

void VATAnimationTrack::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_track", "name", "start_frame", "end_frame", "framerate", "is_looping"), &VATAnimationTrack::set_track);

	ClassDB::bind_method(D_METHOD("set_track_name", "name"), &VATAnimationTrack::set_track_name);
	ClassDB::bind_method(D_METHOD("get_track_name"), &VATAnimationTrack::get_track_name);

	ClassDB::bind_method(D_METHOD("set_start_frame", "frame"), &VATAnimationTrack::set_start_frame);
	ClassDB::bind_method(D_METHOD("get_start_frame"), &VATAnimationTrack::get_start_frame);

	ClassDB::bind_method(D_METHOD("set_end_frame", "frame"), &VATAnimationTrack::set_end_frame);
	ClassDB::bind_method(D_METHOD("get_end_frame"), &VATAnimationTrack::get_end_frame);

	ClassDB::bind_method(D_METHOD("set_framerate", "framerate"), &VATAnimationTrack::set_framerate);
	ClassDB::bind_method(D_METHOD("get_framerate"), &VATAnimationTrack::get_framerate);

	ClassDB::bind_method(D_METHOD("set_is_looping", "looping"), &VATAnimationTrack::set_is_looping);
	ClassDB::bind_method(D_METHOD("get_is_looping"), &VATAnimationTrack::get_is_looping);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "track_name"), "set_track_name", "get_track_name");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "start_frame"), "set_start_frame", "get_start_frame");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "end_frame"), "set_end_frame", "get_end_frame");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "framerate"), "set_framerate", "get_framerate");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "is_looping"), "set_is_looping", "get_is_looping");
}

void VATAnimationTrack::set_track(const String &p_name, int p_start, int p_end, int p_framerate, bool p_loop) {
	name = p_name;
	start_frame = p_start;
	end_frame = p_end;
	framerate = p_framerate;
	is_looping = p_loop;
}

void VATAnimationTrack::set_track_name(const String &p_name) { name = p_name; }
String VATAnimationTrack::get_track_name() const { return name; }

void VATAnimationTrack::set_start_frame(int p_frame) { start_frame = p_frame; }
int VATAnimationTrack::get_start_frame() const { return start_frame; }

void VATAnimationTrack::set_end_frame(int p_frame) { end_frame = p_frame; }
int VATAnimationTrack::get_end_frame() const { return end_frame; }

void VATAnimationTrack::set_framerate(int p_framerate) { framerate = p_framerate; }
int VATAnimationTrack::get_framerate() const { return framerate; }

void VATAnimationTrack::set_is_looping(bool p_loop) { is_looping = p_loop; }
bool VATAnimationTrack::get_is_looping() const { return is_looping; }

String VATAnimationTrack::to_string() {
	return "Animation Track: " + name +
			" startFrame: " + itos(start_frame) +
			" endFrame: " + itos(end_frame) +
			" framerate: " + itos(framerate) +
			" isLooping: " + String(is_looping ? "true" : "false");
}

VATAnimationTrack::VATAnimationTrack() {
	start_frame = 0;
	end_frame = 0;
	framerate = 24;
	is_looping = true;
}

#ifdef DOCTEST

TEST_CASE("[VATAnimationTrack] default constructor") {
	VATAnimationTrack track;
	CHECK(track.get_track_name() == "");
	CHECK(track.get_start_frame() == 0);
	CHECK(track.get_end_frame() == 0);
	CHECK(track.get_framerate() == 24);
	CHECK(track.get_is_looping() == true);
}

TEST_CASE("[VATAnimationTrack] set_track") {
	VATAnimationTrack track;
	track.set_track("Walk", 0, 29, 30, true);

	CHECK(track.get_track_name() == "Walk");
	CHECK(track.get_start_frame() == 0);
	CHECK(track.get_end_frame() == 29);
	CHECK(track.get_framerate() == 30);
	CHECK(track.get_is_looping() == true);

	SUBCASE("overwrite with different values") {
		track.set_track("Run", 30, 59, 24, false);
		CHECK(track.get_track_name() == "Run");
		CHECK(track.get_start_frame() == 30);
		CHECK(track.get_end_frame() == 59);
		CHECK(track.get_framerate() == 24);
		CHECK(track.get_is_looping() == false);
	}
}

TEST_CASE("[VATAnimationTrack] individual setters") {
	VATAnimationTrack track;

	SUBCASE("set_track_name") {
		track.set_track_name("Idle");
		CHECK(track.get_track_name() == "Idle");
		track.set_track_name("Attack");
		CHECK(track.get_track_name() == "Attack");
	}

	SUBCASE("set_start_frame") {
		track.set_start_frame(10);
		CHECK(track.get_start_frame() == 10);
		track.set_start_frame(0);
		CHECK(track.get_start_frame() == 0);
	}

	SUBCASE("set_end_frame") {
		track.set_end_frame(100);
		CHECK(track.get_end_frame() == 100);
	}

	SUBCASE("set_framerate") {
		track.set_framerate(60);
		CHECK(track.get_framerate() == 60);
		track.set_framerate(12);
		CHECK(track.get_framerate() == 12);
	}

	SUBCASE("set_is_looping") {
		CHECK(track.get_is_looping() == true); // default
		track.set_is_looping(false);
		CHECK(track.get_is_looping() == false);
		track.set_is_looping(true);
		CHECK(track.get_is_looping() == true);
	}
}

TEST_CASE("[VATAnimationTrack] to_string") {
	VATAnimationTrack track;
	track.set_track("Death", 60, 89, 24, false);

	String s = track.to_string();
	CHECK(s.find("Death") != -1);
	CHECK(s.find("60") != -1);
	CHECK(s.find("89") != -1);
	CHECK(s.find("24") != -1);
	CHECK(s.find("false") != -1);
}

TEST_CASE("[VATAnimationTrack] multiple tracks independence") {
	VATAnimationTrack track_a;
	VATAnimationTrack track_b;

	track_a.set_track("Walk", 0, 29, 30, true);
	track_b.set_track("Run", 30, 59, 24, false);

	// Verify they don't share state
	CHECK(track_a.get_track_name() == "Walk");
	CHECK(track_b.get_track_name() == "Run");
	CHECK(track_a.get_start_frame() == 0);
	CHECK(track_b.get_start_frame() == 30);
	CHECK(track_a.get_is_looping() == true);
	CHECK(track_b.get_is_looping() == false);
}

TEST_CASE("[VATAnimationTrack] Ref<VATAnimationTrack> usage") {
	Ref<VATAnimationTrack> track;
	track.instance();
	track->set_track("Idle", 0, 47, 24, true);

	CHECK(track.is_valid());
	CHECK(track->get_track_name() == "Idle");
	CHECK(track->get_end_frame() == 47);

	SUBCASE("copy ref shares same object") {
		Ref<VATAnimationTrack> track2 = track;
		CHECK(track2->get_track_name() == "Idle");
		track2->set_track_name("Modified");
		CHECK(track->get_track_name() == "Modified");
	}

	SUBCASE("null ref") {
		Ref<VATAnimationTrack> null_track;
		CHECK(null_track.is_null());
	}
}

TEST_CASE("[VATAnimationTrack] edge case frame values") {
	VATAnimationTrack track;

	SUBCASE("zero-length animation (single frame)") {
		track.set_track("SingleFrame", 5, 5, 24, false);
		CHECK(track.get_start_frame() == 5);
		CHECK(track.get_end_frame() == 5);
	}

	SUBCASE("large frame numbers") {
		track.set_track("Long", 0, 9999, 60, true);
		CHECK(track.get_end_frame() == 9999);
		CHECK(track.get_framerate() == 60);
	}
}

#endif // DOCTEST
