/*************************************************************************/
/*  os_3ds.cpp                                                           */
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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#include "core/os/memory.h"
#include "core/print_string.h"
#include "drivers/unix/dir_access_unix.h"
#include "drivers/unix/file_access_unix.h"
#include "servers/physics_server.h"
#include "servers/visual/visual_server_raster.h"
#ifndef NO_THREADS
#include "servers/visual/visual_server_wrap_mt.h"
#endif
#include "main/main.h"

#include "os_3ds.h"
#include "thread_3ds.h"
#include "video/rasterizer_citro3d.h"

#include "3ds_godot.h"
// Big stack thanks to CANVAS_ITEM_Z_MAX among other things
extern "C" {
u32 __stacksize__ = 1024 * 128;
}

static aptHookCookie apt_hook_cookie;

static void apt_hook_callback(APT_HookType hook, void *param) {
	if (hook == APTHOOK_ONRESTORE || hook == APTHOOK_ONWAKEUP) {
	}
}

OS_3DS::OS_3DS() :
		video_mode(800, 480, true, false, false) {
	gfxInitDefault();
	consoleInit(GFX_BOTTOM, NULL);

	aptHook(&apt_hook_cookie, apt_hook_callback, this);

	// 	set_low_processor_usage_mode(true);
	_render_thread_mode = RENDER_THREAD_UNSAFE;
	AudioDriverManager::add_driver(&audio_driver);

	use_vsync = true;
}

OS_3DS::~OS_3DS() {
	gfxExit();
}

void OS_3DS::run() {
	if (!main_loop)
		return;

	main_loop->init();

	while (aptMainLoop()) {
		processInput();

		if (hidKeysDown() & KEY_SELECT)
			break;

		if (Main::iteration())
			break;

		printf("fps:%f\n", Engine::get_singleton()->get_frames_per_second());
	}

	main_loop->finish();
}

void OS_3DS::initialize_core() {
	FileAccess::make_default<FileAccessUnix>(FileAccess::ACCESS_RESOURCES);
	FileAccess::make_default<FileAccessUnix>(FileAccess::ACCESS_USERDATA);
	FileAccess::make_default<FileAccessUnix>(FileAccess::ACCESS_FILESYSTEM);

	DirAccess::make_default<DirAccessUnix>(DirAccess::ACCESS_RESOURCES);
	DirAccess::make_default<DirAccessUnix>(DirAccess::ACCESS_USERDATA);
	DirAccess::make_default<DirAccessUnix>(DirAccess::ACCESS_FILESYSTEM);

#ifndef NO_NETWORK
// 	TCPServerPosix::make_default();
// 	StreamPeerTCPPosix::make_default();
// 	PacketPeerUDPPosix::make_default();
// 	IP_Unix::make_default();
#endif

	ticks_start = svcGetSystemTick();
}

Error OS_3DS::initialize(const VideoMode &p_desired, int p_video_driver, int p_audio_driver) {
	main_loop = NULL;

	visual_server = memnew(VisualServerRaster());

#ifndef NO_THREADS
	if (get_render_thread_mode() != RENDER_THREAD_UNSAFE) {
		visual_server = memnew(VisualServerWrapMT(visual_server, get_render_thread_mode() == RENDER_SEPARATE_THREAD));
	}
#endif

	AudioDriverManager::get_driver(0)->set_singleton();
	if (AudioDriverManager::get_driver(0)->init() != OK) {
		ERR_PRINT("Initializing audio failed.");
	}

	audio_server = memnew(AudioServer());
	audio_server->init();

	visual_server->init();

	input = memnew(InputDefault);

	return OK;
}

void OS_3DS::delete_main_loop() {
	if (main_loop)
		memdelete(main_loop);
	main_loop = NULL;
}

void OS_3DS::set_main_loop(MainLoop *p_main_loop) {
	main_loop = p_main_loop;
	input->set_main_loop(p_main_loop);
}

void OS_3DS::finalize() {
	if (main_loop)
		memdelete(main_loop);
	main_loop = NULL;

	memdelete(input);

	audio_server->finish();
	memdelete(audio_server);

	visual_server->finish();
	memdelete(visual_server);
}

void OS_3DS::finalize_core() {
}

void OS_3DS::vprint(const char *p_format, va_list p_list, bool p_stder) {
	if (p_stder) {
		vfprintf(stderr, p_format, p_list);
		fflush(stderr);
	} else {
		vprintf(p_format, p_list);
		fflush(stdout);
	}
}

void OS_3DS::alert(const String &p_alert, const String &p_title) {
	fprintf(stderr, "ERROR: %s\n", p_alert.utf8().get_data());
}

Error OS_3DS::set_cwd(const String &p_cwd) {
	printf("set cwd: %s", p_cwd.utf8().get_data());
	if (chdir(p_cwd.utf8().get_data()) != 0)
		return ERR_CANT_OPEN;

	return OK;
}

OS::Date OS_3DS::get_date(bool utc) const {
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
}

OS::Time OS_3DS::get_time(bool utc) const {
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
}

OS::TimeZoneInfo OS_3DS::get_time_zone_info() const {
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
}

void OS_3DS::delay_usec(uint32_t p_usec) const {
	svcSleepThread(1000ULL * p_usec);
}

#define TICKS_PER_SEC 268123480ULL
#define TICKS_PER_USEC 268

uint64_t OS_3DS::get_ticks_usec() const {
	return (svcGetSystemTick() - ticks_start) / TICKS_PER_USEC;
}

uint64_t OS_3DS::get_unix_time() const {
	return time(NULL);
}

uint64_t OS_3DS::get_system_time_secs() const {
	struct timeval tv_now;
	gettimeofday(&tv_now, NULL);
	//localtime(&tv_now.tv_usec);
	//localtime((const long *)&tv_now.tv_usec);
	return uint64_t(tv_now.tv_sec);
}

void OS_3DS::swap_buffers() {
	gfxSwapBuffersGpu();
	if (use_vsync)
		gspWaitForVBlank();
}

int OS_3DS::get_processor_count() const {
	return 1;
}

const int KEY_MAX = 16;
static u32 buttons[KEY_MAX] = {
	KEY_B,
	KEY_A,
	KEY_Y,
	KEY_X,
	KEY_L,
	KEY_R,
	KEY_ZL, // L2
	KEY_ZR, // R2
	KEY_TOUCH, // L3 substitute
	KEY_TOUCH, // R3 substitute
	KEY_SELECT,
	KEY_START,
	KEY_DUP,
	KEY_DDOWN,
	KEY_DLEFT,
	KEY_DRIGHT,
};

void OS_3DS::processInput() {
	hidScanInput();
	u32 kDown = hidKeysDown();
	u32 kUp = hidKeysUp();

	for (int i = 0; i < KEY_MAX; ++i) {
		if (buttons[i] & kDown)
			input->joy_button(0, i, true);
		else if (buttons[i] & kUp)
			input->joy_button(0, i, false);
	}
}
