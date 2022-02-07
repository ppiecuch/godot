/*************************************************************************/
/*  os_psvita.h                                                          */
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

#ifndef OS_PSVITA_H
#define OS_PSVITA_H

#include "core/os/os.h"
#include "core/version.h"

#include "audio/audio_driver_sce.h"
#include "main/input_default.h"
#include "servers/visual_server.h"

#ifdef __psp2__
#include <psp2/rtc.h>
#endif

class OSPSVita : public OS {
	enum {
		PAD_COUNT = 5,
	};

	enum DualMode {

		DUAL_MODE_SINGLE_PAD,
		DUAL_MODE_DUAL_REMOTE,
		DUAL_MODE_DUAL_PAD,
	};

	VisualServer *visual_server;

	VideoMode default_video_mode;
	MainLoop *main_loop;
	InputDefault *input;

	SceRtcTick ticks_start;
	int ticks_per_sec;

	String resource_dir;

	AudioDriverSCE driver_sce;

	void _mouse_button(Vector2 p_pos, bool p_pressed);
	void _mouse_motion(Vector2 p_pos, Vector2 p_rel);

	void _screen_touch(int fid, Vector2 p_pos, bool p_pressed);
	void _screen_drag(int fid, Vector2 p_pos, Vector2 old_pos);

	int dual_mode;

protected:
	// functions used by main to initialize/deintialize the OS
	virtual int get_video_driver_count() const;
	virtual const char *get_video_driver_name(int p_driver) const;
	virtual int get_current_video_driver() const { return 0; };

	virtual int get_audio_driver_count() const;
	virtual const char *get_audio_driver_name(int p_driver) const;

	virtual void initialize_core();

	virtual Error initialize(const VideoMode &p_desired, int p_video_driver, int p_audio_driver);

	virtual void set_main_loop(MainLoop *p_main_loop);
	virtual void delete_main_loop();

	virtual void finalize();
	virtual void finalize_core();

	virtual bool _check_internal_feature_support(const String &p_feature);

	void _process_axis(int p_device, int p_axis, int p_value);

	void process_events();

public:
	virtual String get_stdin_string(bool p_block = true);

	virtual Point2 get_mouse_position() const;
	virtual int get_mouse_button_state() const;

	virtual void set_window_title(const String &p_title);
	virtual void set_video_mode(const VideoMode &p_video_mode, int p_screen);
	virtual VideoMode get_video_mode(int p_screen) const;
	virtual void get_fullscreen_mode_list(List<VideoMode> *p_list, int p_screen) const;

	virtual Size2 get_window_size() const;

	virtual void release_rendering_thread();
	virtual void make_rendering_thread();
	virtual void swap_buffers();

	virtual Error execute(const String &p_path, const List<String> &p_arguments, bool p_blocking, ProcessID *r_child_id = NULL, String *r_pipe = NULL, int *r_exitcode = NULL, bool read_stderr = false, Mutex *p_pipe_mutex = NULL);
	virtual Error kill(const ProcessID &p_pid);

	virtual bool has_environment(const String &p_var) const;
	virtual String get_environment(const String &p_var) const;
	virtual bool set_environment(const String &p_var, const String &p_value) const;

	virtual String get_name() const;

	virtual MainLoop *get_main_loop() const;

	virtual Date get_date(bool local = false) const;
	virtual Time get_time(bool local = false) const;
	virtual TimeZoneInfo get_time_zone_info() const;

	virtual void delay_usec(uint32_t p_usec) const;
	virtual uint64_t get_ticks_usec() const;

	virtual bool can_draw() const;

	virtual void set_cursor_shape(CursorShape p_shape);
	virtual void set_custom_mouse_cursor(const RES &p_cursor, CursorShape p_shape, const Vector2 &p_hotspot);

	virtual String get_resource_dir() const;

	virtual void alert(const String &p_alert, const String &p_title = "ALERT!");

	void run();
	OSPSVita();
	virtual ~OSPSVita();
};

#endif // OS_PSVITA_H
