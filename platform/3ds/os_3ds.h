/**************************************************************************/
/*  os_3ds.h                                                              */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#ifndef OS_3DS_H
#define OS_3DS_H

#include "audio/audio_driver_3ds.h"
#include "core/os/input.h"
#include "core/os/os.h"
#include "main/input_default.h"
#include "servers/audio/audio_driver_dummy.h"
#include "servers/audio_server.h"
#include "servers/physics_2d/physics_2d_server_sw.h"
#include "servers/physics_2d/physics_2d_server_wrap_mt.h"
#include "servers/physics_server.h"
#include "servers/visual/rasterizer.h"
#include "servers/visual/visual_server_wrap_mt.h"
#include "servers/visual_server.h"

/**
	@author Thomas Edvalson <machin3@gmail.com>
*/

class OS_3DS : public OS {
	MainLoop *main_loop;
	VisualServer *visual_server;
	InputDefault *input;

	AudioServer *audio_server;
	AudioDriver3ds audio_driver;

	Point2i last_mouse_pos;
	VideoMode video_mode;

	bool use_vsync;

	uint64_t ticks_start;

protected:
	friend class Main;

	// functions used by main to initialize/deintialize the OS
	virtual int get_video_driver_count() const { return 1; }
	virtual const char *get_video_driver_name(int p_driver) const { return "citro3d"; }

	virtual VideoMode get_default_video_mode() const { return video_mode; }

	virtual int get_audio_driver_count() const { return 1; }
	virtual const char *get_audio_driver_name(int p_driver) const { return "ndsp"; }

	virtual void initialize_core();
	virtual Error initialize(const VideoMode &p_desired, int p_video_driver, int p_audio_driver);

	virtual void set_main_loop(MainLoop *p_main_loop);
	virtual void delete_main_loop();

	virtual void finalize();
	virtual void finalize_core();

	virtual bool _check_internal_feature_support(const String &p_feature) { return false; }

public:
	virtual void vprint(const char *p_format, va_list p_list, bool p_stderr = false);
	virtual void alert(const String &p_alert, const String &p_title = "ALERT!");
	virtual String get_stdin_string(bool p_block = true) { return ""; }

	virtual Point2 get_mouse_position() const { return last_mouse_pos; }
	virtual int get_mouse_button_state() const { return 0; }
	virtual void set_window_title(const String &p_title){};

	virtual void set_video_mode(const VideoMode &p_video_mode, int p_screen = 0) {}
	virtual VideoMode get_video_mode(int p_screen = 0) const { return video_mode; }
	virtual void get_fullscreen_mode_list(List<VideoMode> *p_list, int p_screen = 0) const {}

	virtual int get_current_video_driver() const { return 0; };

	virtual Size2 get_window_size() const { return Size2(400, 240); }

	virtual String get_executable_path() const { return "test"; }
	virtual Error execute(const String &p_path, const List<String> &p_arguments, bool p_blocking, ProcessID *r_child_id = 0, String *r_pipe = 0, int *r_exitcode = 0, bool read_stderr = false, Mutex *p_pipe_mutex = 0) { return FAILED; }
	virtual Error kill(const ProcessID &p_pid) { return FAILED; }

	virtual Error set_cwd(const String &p_cwd);

	virtual bool has_environment(const String &p_var) const { return false; }
	virtual String get_environment(const String &p_var) const { return ""; }
	virtual bool set_environment(const String &p_var, const String &p_value) const { return false; }

	virtual String get_name() const { return "3DS"; }

	virtual MainLoop *get_main_loop() const { return main_loop; }

	virtual Date get_date(bool local = false) const;
	virtual Time get_time(bool local = false) const;
	virtual TimeZoneInfo get_time_zone_info() const;
	virtual uint64_t get_unix_time() const;
	virtual uint64_t get_system_time_secs() const;

	virtual void delay_usec(uint32_t p_usec) const;
	virtual uint64_t get_ticks_usec() const;
	uint32_t get_ticks_msec() const;
	uint64_t get_splash_tick_msec() const;

	void set_frame_delay(uint32_t p_msec);
	uint32_t get_frame_delay() const;

	virtual bool can_draw() const { return true; }

	bool is_stdout_verbose() const;

	virtual void set_cursor_shape(CursorShape p_shape) {}

	virtual bool get_swap_ok_cancel() { return false; }
	virtual void swap_buffers();

	virtual int get_processor_count() const;

	virtual bool can_use_threads() const { return true; }

	virtual void set_use_vsync(bool p_enable) { use_vsync = p_enable; }
	virtual bool is_vsnc_enabled() const { return use_vsync; }

	void run();
	void processInput();

	OS_3DS();
	virtual ~OS_3DS();
};

#endif // OS_3DS_H
