// kate: replace-tabs on; tab-indents on; tab-width 4; indent-width 4; indent-mode cstyle;

#include "textui/textUI_support.h"

#include "core/math/vector2.h"
#include "core/os/dir_access.h"
#include "core/os/file_access.h"

#include <fcntl.h>

/// BEGIN Host-system integration
///
/// Bug fix: these seven functions used to be permanently inert stubs
/// (keyhit() always FALSE, getkey()/getshift() always 0, mousebuttons()/
/// button_releases() always 0, get_mouseposition() always (0,0),
/// resetmouse() a no-op) -- meaning no Godot host could ever have made a
/// keystroke or a mouse click reach textUI.c's event loop through this
/// backend, regardless of what fed it. They're now backed by the small
/// event state below, fed by TextUI_FeedKey()/TextUI_SetShiftState()/
/// TextUI_FeedMouseButton()/TextUI_FeedMousePosition() from a Godot
/// Control/CanvasItem's own _gui_input()/_input() handler.
///
/// No other Godot code calls these yet -- when a real caller (e.g. a
/// console Control) needs them, declare it a proper public header then;
/// until it exists, a header for a single includer is just indirection.
///
/// Shift-mask bits match RIGHTSHIFT/LEFTSHIFT/CTRLKEY/ALTKEY (textUI.h);
/// mouse masks match the bit0=left/bit1=right convention of
/// mousebuttons()/leftbutton()/rightbutton() (also textUI.h). Screen
/// coordinates are in character cells, matching PutWindowChar/
/// MouseWindow()'s (x,y), not pixels.

#define TEXTUI_KEY_QUEUE_SIZE 32

static int _key_queue[TEXTUI_KEY_QUEUE_SIZE];
static int _key_queue_head = 0;
static int _key_queue_tail = 0;
static int _key_queue_count = 0;

static int _shift_state = 0;

BOOL system_keyhit(void) {
	return _key_queue_count > 0 ? TRUE : FALSE;
}

int system_getkey(void) {
	if (_key_queue_count == 0)
		return 0;
	const int code = _key_queue[_key_queue_head];
	_key_queue_head = (_key_queue_head + 1) % TEXTUI_KEY_QUEUE_SIZE;
	_key_queue_count--;
	return code;
}

int system_getshift(void) {
	return _shift_state;
}

/* Sets the *live* modifier-key state (RIGHTSHIFT|LEFTSHIFT|CTRLKEY|ALTKEY),
** queried back by system_getshift(). Call this every time a modifier key
** (or a key combined with one) changes state -- in particular, on every
** InputEventKey, not just on the modifier keys themselves, so a keystroke
** queued by TextUI_FeedKey() right after this call is seen by textUI.c's
** collect_events() with the correct shift state already in effect (it
** polls getshift() and keyhit()/getkey() together, in that order, once
** per dispatch_message() call). */
void TextUI_SetShiftState(int shift_mask) {
	_shift_state = shift_mask;
}

/* Queues one keystroke for system_getkey()/system_keyhit(). `code` is
** either a plain ASCII character (e.g. 'f') -- including for Alt+letter
** combinations, exactly like a real keyboard/terminal driver would
** report, relying on TextUI_SetShiftState()'s ALTKEY bit plus textUI.c's
** own AltConvert() to recognize the shortcut -- or an extended/function
** key code already in DFlat's own FKEY-offset encoding (see textUI.h's
** HOME/UP/DN/DEL/F1../ALT_A.. macros), for keys with no ASCII
** representation at all (arrows, function keys, Home/End, ...). Silently
** drops the keystroke if the internal queue is full; real typing is far
** slower than this queue drains. */
void TextUI_FeedKey(int code) {
	if (_key_queue_count >= TEXTUI_KEY_QUEUE_SIZE)
		return; // queue full: drop it, typing is far slower than this drains
	_key_queue[_key_queue_tail] = code;
	_key_queue_tail = (_key_queue_tail + 1) % TEXTUI_KEY_QUEUE_SIZE;
	_key_queue_count++;
}

static int _mouse_button_mask = 0; // bit0 = left held, bit1 = right held
static int _mouse_release_flags = 0; // sticky until the next system_button_releases()
static Point2 _pointer_position;

void system_resetmouse(void) {
	_mouse_button_mask = 0;
	_mouse_release_flags = 0;
}

int system_mousebuttons(void) {
	return _mouse_button_mask;
}

void system_get_mouseposition(int *x, int *y) {
	*x = _pointer_position.x;
	*y = _pointer_position.y;
}

int system_button_releases(void) {
	const int released = _mouse_release_flags;
	_mouse_release_flags = 0;
	return released;
}

/* Reports a mouse button transition. `button_mask` is 1 for the left
** button, 2 for the right button (matching mousebuttons()'s bit
** layout); `pressed` is true on button-down, false on button-up. Button
** state is level-tracked (system_mousebuttons() reports "currently
** held"); releases are additionally latched until the next
** system_button_releases() call, matching what collect_events() expects
** (it polls button_releases() as a one-shot "was released since I last
** asked" flag, separately from the held-state mask). */
void TextUI_FeedMouseButton(int button_mask, BOOL pressed) {
	if (pressed) {
		_mouse_button_mask |= button_mask;
	} else {
		_mouse_button_mask &= ~button_mask;
		_mouse_release_flags |= button_mask;
	}
}

/* Updates the mouse position returned by system_get_mouseposition(), in
** character cells (not pixels) -- convert from the Control's local pixel
** position using its cell size before calling this. */
void TextUI_FeedMousePosition(int x, int y) {
	_pointer_position = Point2(x, y);
}

/// END Host-system integration

/*---------------------------------------------------------------------*

Name        IO file access layer.

*---------------------------------------------------------------------*/
enum ExModeFlags {
	EX_MODE_CREATE = 256, // highe values than FileAccess::ModeFlags
	EX_MODE_APPEND = 512,
};

struct _dd_ffblk {
	DirAccess *list_dir;
};

static DirAccess *_current_dir = nullptr;

// Convert file permissions

static int _conv_perm(const String &p_path, int mode) {
	int flags = 0;
	if (mode & O_RDONLY) {
		flags = FileAccess::READ;
	} else if (mode & O_WRONLY) {
		flags = FileAccess::WRITE;
	}

	if (mode & O_APPEND) {
		if (FileAccess::exists(p_path)) {
			flags = FileAccess::READ_WRITE | EX_MODE_APPEND;
		} else {
			flags = FileAccess::WRITE_READ;
		}
	}

	if (flags == 0) {
		flags = FileAccess::READ; // default
	}

	return flags;
}

static int _conv_perm(const String &p_path, const String &mode) {
#define check(C) (mode.has(C))
	// Convert permissions
	int flags = 0;
	if (check("r+"))
		flags = FileAccess::READ_WRITE;
	else if (check("w+"))
		flags = FileAccess::WRITE_READ;
	else if (check("r"))
		flags = FileAccess::READ;
	else if (check("w"))
		flags = FileAccess::WRITE;

	if (check("a")) {
		if (FileAccess::exists(p_path)) {
			flags = FileAccess::READ_WRITE | EX_MODE_APPEND;
		} else {
			flags = FileAccess::WRITE_READ;
		}
	}

	if (flags == 0)
		flags = FileAccess::READ; // default
#undef check
	return flags;
}

struct GFILE {
	FileAccess *fa;
	static String fixpath(const String &p_path) {
		if (_current_dir && !p_path.is_abs_path() && !FileAccess::exists(p_path)) {
			return _current_dir->get_current_dir().append_path(p_path);
		}
		return p_path;
	}
	static GFILE *fopen(const String &p_path, int p_mode_flags) {
		if (!p_path.empty()) {
			const String real_path = fixpath(p_path);
			const int flags = _conv_perm(real_path, p_mode_flags);
			if (FileAccess *_fa = FileAccess::open(real_path, flags & 0xff)) {
				if (flags & EX_MODE_CREATE) {
					_fa->seek_end();
				}
				return memnew(GFILE(_fa));
			} else {
				return nullptr;
			}
		}
		return nullptr;
	}
	static GFILE *fopen(const String &p_path, const String &p_mode_flags) {
		if (!p_path.empty()) {
			const String real_path = fixpath(p_path);
			const int flags = _conv_perm(real_path, p_mode_flags);
			if (FileAccess *_fa = FileAccess::open(real_path, flags & 0xff)) {
				if (flags & EX_MODE_APPEND) {
					_fa->seek_end();
				}
				return memnew(GFILE(_fa));
			} else {
				return nullptr;
			}
		}
		return nullptr;
	}
	GFILE(FileAccess *p_fa) { fa = p_fa; }
	GFILE() { fa = nullptr; }
	~GFILE() {
		if (fa) {
			memdelete(fa);
		}
	}
};

/* Directory access */

int dir_findfirst(const char *path, dir_ffblk *fb, int attrib) { return 0; }
int dir_findnext(dir_ffblk *fb) { return 0; }
int dir_getattrib(const dir_ffblk *fb) { return 0; }
const char *dir_getname(const dir_ffblk *fb) { return ""; }
