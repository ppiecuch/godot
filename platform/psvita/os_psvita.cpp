/*************************************************************************/
/*  os_psvita.cpp                                                        */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2022 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2022 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#include "os_psvita.h"

#include "drivers/unix/dir_access_unix.h"
#include "drivers/unix/file_access_unix.h"
#include "main/main.h"
#include "servers/visual/visual_server_raster.h"
#ifndef NO_THREADS
#include "servers/visual/visual_server_wrap_mt.h"
#endif

#ifdef __psp2__
#include <psp2/kernel/threadmgr.h>
#endif

#include <sys/time.h>

int OSPSVita::get_video_driver_count() const {
	return 1;
};

const char *OSPSVita::get_video_driver_name(int p_driver) const {
	return "GXM";
};

int OSPSVita::get_audio_driver_count() const {
	return 1;
};

const char *OSPSVita::get_audio_driver_name(int p_driver) const {
	return "SCE";
};

void OSPSVita::initialize_core() {
	setenv("HOME", "app0:", 1);

	FileAccess::make_default<FileAccessUnix>(FileAccess::ACCESS_RESOURCES);
	DirAccess::make_default<DirAccessUnix>(DirAccess::ACCESS_RESOURCES);
};

Error OSPSVita::initialize(const VideoMode &p_desired, int p_video_driver, int p_audio_driver) {
	OS::VideoMode vm;
	vm.resizable = false;
	vm.fullscreen = true;

	int value = 0;
	vm.width = value;
	vm.height = value;

	default_video_mode = vm;

	visual_server = memnew(VisualServerRaster);

	AudioDriverManager::initialize(p_audio_driver);

	visual_server->init();

	input = memnew(InputDefault);

	printf("%s: %i\n", __PRETTY_FUNCTION__, __LINE__);

	dual_mode = GLOBAL_DEF("psvita/dual_pad_mode", DUAL_MODE_DUAL_PAD);

	return OK;
};

void OSPSVita::release_rendering_thread(){

};

void OSPSVita::make_rendering_thread(){

};

void OSPSVita::swap_buffers(){

};

void OSPSVita::set_main_loop(MainLoop *p_main_loop) {
	main_loop = p_main_loop;
	input->set_main_loop(p_main_loop);
};

void OSPSVita::_mouse_button(Vector2 p_pos, bool p_pressed) {
	Ref<InputEventMouseButton> ev;
	ev.instance();

	ev->set_position(p_pos);
	ev->set_global_position(p_pos);

	//mouse_list.pressed[p_idx] = p_pressed;

	ev->set_button_index(BUTTON_LEFT);
	ev->set_doubleclick(false);
	ev->set_pressed(p_pressed);

	input->set_mouse_position(p_pos);

	input->parse_input_event(ev);
};

void OSPSVita::_mouse_motion(Vector2 p_pos, Vector2 p_rel) {
	Ref<InputEventMouseMotion> ev;
	ev.instance();

	ev->set_position(p_pos);
	ev->set_global_position(p_pos);
	ev->set_relative(p_rel);

	input->set_mouse_position(ev->get_position());

	ev->set_speed(input->get_last_mouse_speed());
	ev->set_button_mask(BUTTON_LEFT); // pressed

	input->parse_input_event(ev);
};

void OSPSVita::_screen_touch(int fid, Vector2 p_pos, bool p_pressed) {
	Ref<InputEventScreenTouch> ev;
	ev.instance();

	ev->set_index(fid);
	ev->set_pressed(true);
	ev->set_position(p_pos);

	input->parse_input_event(ev);
};

void OSPSVita::_screen_drag(int fid, Vector2 p_pos, Vector2 old_pos) {
	Ref<InputEventScreenDrag> ev;
	ev.instance();
	ev->set_index(fid);
	ev->set_position(p_pos);
	ev->set_relative(p_pos - old_pos);

	input->parse_input_event(ev);
};

void OSPSVita::_process_axis(int p_device, int p_axis, int p_value) {
	InputDefault::JoyAxis axis;
	axis.min = 1;
	axis.value = (float)p_value / 32768.0;

	input->joy_axis(p_device, p_axis, axis);
};

void OSPSVita::process_events(){

};

void OSPSVita::run() {
	if (!main_loop)
		return;

	main_loop->init();

	while (true) {
		process_events(); // get rid of pending events
		if (Main::iteration())
			break;
	};

	main_loop->finish();
};

void OSPSVita::delete_main_loop() {
	if (main_loop)
		memdelete(main_loop);
	main_loop = NULL;
};

void OSPSVita::finalize(){

};

void OSPSVita::finalize_core() {
	if (main_loop)
		memdelete(main_loop);
	main_loop = NULL;

	memdelete(input);

	visual_server->finish();
	memdelete(visual_server);
};

bool OSPSVita::_check_internal_feature_support(const String &p_feature) {
	if (p_feature == "mobile" || p_feature == "etc" || p_feature == "etc2") {
		//TODO support etc2 only if GLES3 driver is selected
		return true;
	}
#if defined(__aarch64__)
	if (p_feature == "arm64-v8a") {
		return true;
	}
#elif defined(__ARM_ARCH_7A__)
	if (p_feature == "armeabi-v7a" || p_feature == "armeabi") {
		return true;
	}
#elif defined(__arm__)
	if (p_feature == "armeabi") {
		return true;
	}
#endif
	return false;
};

String OSPSVita::get_stdin_string(bool p_block) {
	return "";
};

Point2 OSPSVita::get_mouse_position() const {
	return Point2();
};

int OSPSVita::get_mouse_button_state() const {
	return 0;
};

void OSPSVita::set_window_title(const String &p_title){

};

void OSPSVita::set_video_mode(const VideoMode &p_video_mode, int p_screen){

};

OS::VideoMode OSPSVita::get_video_mode(int p_screen) const {
	return default_video_mode;
};

Size2 OSPSVita::get_window_size() const {
	return Size2(default_video_mode.width, default_video_mode.height);
};

void OSPSVita::get_fullscreen_mode_list(List<VideoMode> *p_list, int p_screen) const {
	p_list->push_back(default_video_mode);
};

Error OSPSVita::execute(const String &p_path, const List<String> &p_arguments, bool p_blocking, ProcessID *r_child_id, String *r_pipe, int *r_exitcode, bool read_stderr, Mutex *p_pipe_mutex) {
	return ERR_UNAVAILABLE;
};

Error OSPSVita::kill(const ProcessID &p_pid) {
	return ERR_UNAVAILABLE;
};

bool OSPSVita::has_environment(const String &p_var) const {
	return false;
};

String OSPSVita::get_environment(const String &p_var) const {
	return "";
};

bool OSPSVita::set_environment(const String &p_var, const String &p_value) const {
	return false;
}

String OSPSVita::get_name() const {
	return "PSP2";
};

MainLoop *OSPSVita::get_main_loop() const {
	return main_loop;
};

OS::Date OSPSVita::get_date(bool utc) const {
	time_t t = time(NULL);
	struct tm *lt;
	if (utc)
		lt = gmtime(&t);
	else
		lt = localtime(&t);
	Date ret;
	ret.year = 1900 + lt->tm_year;
	// Index starting at 1 to match OS_Unix::get_date
	//   and Windows SYSTEMTIME and tm_mon follows the typical structure
	//   of 0-11, noted here: http://www.cplusplus.com/reference/ctime/tm/
	ret.month = (Month)(lt->tm_mon + 1);
	ret.day = lt->tm_mday;
	ret.weekday = (Weekday)lt->tm_wday;
	ret.dst = lt->tm_isdst;

	return ret;
};

OS::Time OSPSVita::get_time(bool utc) const {
	time_t t = time(NULL);
	struct tm *lt;
	if (utc)
		lt = gmtime(&t);
	else
		lt = localtime(&t);
	Time ret;
	ret.hour = lt->tm_hour;
	ret.min = lt->tm_min;
	ret.sec = lt->tm_sec;
	get_time_zone_info();
	return ret;
};

OS::TimeZoneInfo OSPSVita::get_time_zone_info() const {
	time_t t = time(NULL);
	struct tm *lt = localtime(&t);
	char name[16];
	strftime(name, 16, "%Z", lt);
	name[15] = 0;
	TimeZoneInfo ret;
	ret.name = name;

	char bias_buf[16];
	strftime(bias_buf, 16, "%z", lt);
	int bias;
	bias_buf[15] = 0;
	sscanf(bias_buf, "%d", &bias);

	// convert from ISO 8601 (1 minute=1, 1 hour=100) to minutes
	int hour = (int)bias / 100;
	int minutes = bias % 100;
	if (bias < 0)
		ret.bias = hour * 60 - minutes;
	else
		ret.bias = hour * 60 + minutes;

	return ret;
};

void OSPSVita::delay_usec(uint32_t p_usec) const {
	sceKernelDelayThread(p_usec);
};

uint64_t OSPSVita::get_ticks_usec() const {
	SceRtcTick ticks = { 0 };
	sceRtcGetCurrentTick(&ticks);

	uint64_t delta = ticks.tick - ticks_start.tick;
	uint64_t seconds = delta / ticks_per_sec;
	uint64_t remainder = delta - (seconds * ticks_per_sec);
	uint64_t millis = (remainder * 1000) / ticks_per_sec;
	uint64_t retval = seconds * 1000 + millis;
	return retval;
};

bool OSPSVita::can_draw() const {
	return true;
};

void OSPSVita::set_cursor_shape(CursorShape p_shape){

};

void OSPSVita::set_custom_mouse_cursor(const RES &p_cursor, CursorShape p_shape, const Vector2 &p_hotspot){

};

void OSPSVita::alert(const String &p_alert, const String &p_title) {
	printf("%s: %s\n", p_title.utf8().get_data(), p_alert.utf8().get_data());
};

String OSPSVita::get_resource_dir() const {
	return resource_dir;
};

OSPSVita::OSPSVita() {
	printf("*** creating OS object %p\n", this);
	main_loop = nullptr;

	sceRtcGetCurrentTick(&ticks_start);
	ticks_per_sec = sceRtcGetTickResolution();

	AudioDriverManager::add_driver(&driver_sce);

	resource_dir = "";
};

OSPSVita::~OSPSVita(){

};
