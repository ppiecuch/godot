/**************************************************************************/
/*  ofxdatgui.h                                                           */
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

/// Copyright (C) 2015 Stephen Braitsch [http://braitsch.io]

#ifndef OFX_DATGUI_H
#define OFX_DATGUI_H

#include "core/color.h"
#include "core/image.h"
#include "core/math/vector2.h"
#include "core/reference.h"
#include "core/ustring.h"
#include "scene/2d/canvas_item.h"
#include "scene/2d/node_2d.h"
#include "scene/resources/dynamic_font.h"
#include "scene/resources/font.h"

#include <iostream>
#include <memory>
#include <string>
#include <vector>

/// ofxSmartFont

class ofxSmartFont : public Reference {
	GDCLASS(ofxSmartFont, Reference);
	int mSize;
	String mFile;
	String mName;
	Ref<DynamicFont> ttf;

public:
	ofxSmartFont() :
			mSize(12), mFile(""), mName("") {} // Default constructor for Ref<>::instance()

	ofxSmartFont(String file, int size, String name = "") {
		mSize = size;
		mFile = file;
		if (name != "") {
			mName = name;
		} else {
			int pos = MAX(file.rfind("/"), file.rfind("\\"));
			mName = pos >= 0 ? file.substr(pos + 1) : file;
		}

		// Create and setup DynamicFont for Godot
		ttf.instance();
		Ref<DynamicFontData> font_data;
		font_data.instance();
		font_data->set_font_path(mFile);
		ttf->set_font_data(font_data);
		ttf->set_size(mSize);

		if (ttf.is_null() || ttf->get_font_data().is_null()) {
			log(String("ERROR!! file : ") + mFile + String(" NOT FOUND"));
		} else {
			log(String("new font added : ") + mName + String(" @ pt size ") + String::num(mSize));
		}
	}

	static void log(String msg);

public:
	// Set font from pre-created DynamicFont (for embedded INCBIN data)
	void set_embedded_font(Ref<DynamicFont> p_font, int p_size) {
		ttf = p_font;
		mSize = p_size;
		mName = "embedded";
		mFile = "<embedded>";
	}

	String file();
	int size();
	String name();
	void name(String name);
	void draw(String s, int x, int y);

	Rect2 rect(String s, int x = 0, int y = 0);
	float width(String s, int x = 0, int y = 0);
	float height(String s, int x = 0, int y = 0);
	float getLineHeight();

	// static methods
	static Ref<ofxSmartFont> add(String file, int size, String name = "");
	static Ref<ofxSmartFont> get(String name);
	static Ref<ofxSmartFont> get(String name, int size);
	static Ref<ofxSmartFont> get(PoolStringArray keys, int size);
	static void list();

	static Vector<Ref<ofxSmartFont>> mFonts;
};

/// ofxDatGuiConstants

enum class ofxDatGuiAnchor {
	NO_ANCHOR = 0,
	TOP_LEFT = 1,
	TOP_RIGHT = 2,
	BOTTOM_LEFT = 3,
	BOTTOM_RIGHT = 4,
};

enum ofxDatGuiEvent {
	DATGUI_RESIZE = 0,
	DATGUI_KEY_PRESSED,
	DATGUI_MOUSE_PRESSED,
	DATGUI_MOUSE_RELEASED,
	DATGUI_MOUSE_DRAGGED,
	DATGUI_FOCUS_LOST,
	DATGUI_FOCUS_GAINED,
	DATGUI_UPDATE,
	DATGUI_DRAW
};

enum class ofxDatGuiGraph {
	LINES = 0,
	FILLED,
	POINTS,
	OUTLINE,
};

enum class ofxDatGuiAlignment {
	LEFT = 1,
	CENTER = 2,
	RIGHT = 3,
};

enum class ofxDatGuiInputType {
	NUMERIC = 1,
	ALPHA_NUMERIC = 2,
	COLORPICKER = 3,
};

enum class ofxDatGuiType {
	LABEL = 0,
	BREAK,
	BUTTON,
	TOGGLE,
	PAD2D,
	HEADER,
	FOOTER,
	MATRIX,
	SLIDER,
	FOLDER,
	DROPDOWN,
	DROPDOWN_OPTION,
	TEXT_INPUT,
	FRAME_RATE,
	COLOR_PICKER,
	WAVE_MONITOR,
	VALUE_PLOTTER,
};

/// ofxDatGuiEvents

class ofxDatGuiButton;
class ofxDatGuiToggle;
class ofxDatGuiSlider;
class ofxDatGuiDropdown;
class ofxDatGuiTextInput;
class ofxDatGui2dPad;
class ofxDatGuiColorPicker;
class ofxDatGuiMatrix;
class ofxDatGuiScrollView;
class ofxDatGuiScrollViewItem;
class ofxDatGuiHeader;
class ofxDatGuiFooter;
class ofxDatGuiLabel;
class ofxDatGuiFRM;
class ofxDatGuiBreak;
class ofxDatGuiWaveMonitor;
class ofxDatGuiValuePlotter;
class ofxDatGuiFolder;

enum ofxDatGuiEventType {
	GUI_TOGGLED = 0,
	BUTTON_CLICKED,
	BUTTON_TOGGLED,
	INPUT_CHANGED,
	COLOR_CHANGED,
	SLIDER_CHANGED,
	OPTION_SELECTED,
	GROUP_TOGGLED,
	VISIBILITY_CHANGED,
	MATRIX_BUTTON_TOGGLED
};

class ofxDatGuiInternalEvent {
public:
	ofxDatGuiInternalEvent(int eType, int eIndex) {
		type = eType;
		index = eIndex;
	};
	int type;
	int index;
};

class ofxDatGuiButtonEvent {
public:
	ofxDatGuiButtonEvent(ofxDatGuiButton *t) {
		target = t;
	}
	ofxDatGuiButton *target;
};

class ofxDatGuiToggleEvent {
public:
	ofxDatGuiToggleEvent(ofxDatGuiToggle *t, bool e = false) {
		target = t;
		checked = e;
	}
	bool checked;
	ofxDatGuiToggle *target;
};

class ofxDatGuiSliderEvent {
public:
	ofxDatGuiSliderEvent(ofxDatGuiSlider *t, float v, float s) {
		value = v;
		scale = s;
		target = t;
	}
	float value;
	float scale;
	ofxDatGuiSlider *target;
};

class ofxDatGuiTextInputEvent {
public:
	ofxDatGuiTextInputEvent(ofxDatGuiTextInput *t, String s) {
		text = s;
		target = t;
	}
	String text;
	ofxDatGuiTextInput *target;
};

class ofxDatGuiColorPickerEvent {
public:
	ofxDatGuiColorPickerEvent(ofxDatGuiColorPicker *t, Color c) {
		color = c;
		target = t;
	}
	Color color;
	ofxDatGuiColorPicker *target;
};

class ofxDatGuiDropdownEvent {
public:
	ofxDatGuiDropdownEvent(ofxDatGuiDropdown *t, int p, int c) {
		child = c;
		parent = p;
		target = t;
	}
	int child;
	int parent;
	ofxDatGuiDropdown *target;
};

class ofxDatGuiScrollViewEvent {
public:
	ofxDatGuiScrollViewEvent(ofxDatGuiScrollView *p, ofxDatGuiScrollViewItem *b) {
		parent = p;
		target = b;
	}
	ofxDatGuiScrollView *parent;
	ofxDatGuiScrollViewItem *target;
};

class ofxDatGui2dPadEvent {
public:
	ofxDatGui2dPadEvent(ofxDatGui2dPad *t, float xp, float yp) {
		x = xp;
		y = yp;
		target = t;
	}
	float x;
	float y;
	ofxDatGui2dPad *target;
};

class ofxDatGuiMatrixEvent {
public:
	ofxDatGuiMatrixEvent(ofxDatGuiMatrix *t, int i, bool e) {
		child = i;
		target = t;
		enabled = e;
	}
	int child;
	bool enabled;
	ofxDatGuiMatrix *target;
};

/// ofxDatGuiTheme

class ofxDatGuiTheme {
public:
	// This is the base class for all custom themes.
	// The properites here can be overridden by any class that extends this class.

	ofxDatGuiTheme(bool autoInitialize = false) {
		if (autoInitialize)
			init();
	}

	void init(); // defined in ofxdatgui.cpp (uses INCBIN embedded data)

	void scale(float sc) {
		font.size *= sc;
		stripe.width *= sc;
		layout.width *= sc;
		layout.height *= sc;
		layout.padding *= sc;
		layout.vMargin *= sc;
		layout.iconSize *= sc;
		layout.labelWidth *= sc;
		layout.labelMargin *= sc;
		layout.graph.height *= sc;
		layout.pad2d.height *= sc;
		layout.pad2d.ballSize *= sc;
		layout.pad2d.lineWeight *= sc;
		layout.matrix.height *= sc;
		layout.matrix.buttonSize *= sc;
		layout.matrix.buttonPadding *= sc;
		layout.colorPicker.rainbowWidth *= sc;
		layout.textInput.highlightPadding *= sc;
	}

	// gui & component colors

	struct {
		// global gui background color
		Color guiBackground = Color::hex(0x303030);

		// general colors that are shared by all components
		Color label = Color::hex(0xEEEEEE);
		Color icons = Color::hex(0xEEEEEE);
		Color background = Color::hex(0x1A1A1A);
		Color backgroundOnMouseOver = Color::hex(0x777777);
		Color backgroundOnMouseDown = Color::hex(0x222222);
		Color inputAreaBackground = Color::hex(0x303030);

		// component specific colors
		struct {
			Color fill = Color::hex(0x2FA1D6);
			Color text = Color::hex(0x2FA1D6);
		} slider;

		struct {
			Color text = Color::hex(0x00FF00);
			Color highlight = Color::hex(0x688EB5);
			Color backgroundOnActive = Color::hex(0x777777);
		} textInput;

		struct {
			Color border = Color::hex(0xEEEEEE);
		} colorPicker;

		struct {
			Color line = Color::hex(0xEEEEEE);
			Color ball = Color::hex(0xEEEEEE);
		} pad2d;

		struct {
			Color lines = Color::hex(0xEEEEEE);
			Color fills = Color::hex(0xEEEEEE);
		} graph;

		struct {
			struct {
				Color label = Color::hex(0x303030);
				Color button = Color::hex(0xEEEEEE);
			} normal;
			struct {
				Color label = Color::hex(0xEEEEEE);
				Color button = Color::hex(0x2FA1D6);
			} hover;
			struct {
				Color label = Color::hex(0xEEEEEE);
				Color button = Color::hex(0x555555);
			} selected;
		} matrix;

	} color;

	// colored stripes that appear on the left edge of the component

	struct {
		int width = 2;
		bool visible = true;
		Color label = Color::hex(0xEEEEEE);
		Color button = Color::hex(0xFFD00B);
		Color toggle = Color::hex(0xFFD00B);
		Color slider = Color::hex(0x2FA1D6);
		Color pad2d = Color::hex(0x9AF398);
		Color matrix = Color::hex(0xB2770D);
		Color graph = Color::hex(0x9AF398);
		Color dropdown = Color::hex(0xC63256);
		Color textInput = Color::hex(0x1ED36F);
		Color colorPicker = Color::hex(0xFFD00B);
	} stripe;

	// component border, disabled by default

	struct {
		int width = 1.0f;
		bool visible = false;
		Color color = hex(0x000000);
	} border;

	// layout, sizing and rendering rules

	struct {
		// general rules that are shared by all components
		float width = 270;
		float height = 26;
		float padding = 2;
		float vMargin = 1; // vertical spacing between gui components
		float iconSize = 10;
		float labelWidth = 95;
		float labelMargin = 12;
		float breakHeight = 3;
		bool upperCaseLabels = true;

		// component specific rules & settings

		struct {
			int highlightPadding = 5;
			bool forceUpperCase = true;
		} textInput;

		struct {
			int rainbowWidth = 10;
		} colorPicker;

		struct {
			int height = 82;
			int ballSize = 5;
			int lineWeight = 1;
		} pad2d;

		struct {
			int height = 70;
			int pointSize = 2;
			int lineWeight = 2;
		} graph;

		struct {
			int height = 82;
			int buttonSize = 23;
			int buttonPadding = 1;
		} matrix;

	} layout;

	// typography & icons

	static String AssetPath;

	struct {
		int size = 6;
		String file;
		Ref<DynamicFont> ptr;
	} font;

	struct {
		Ref<Image> rainbow;
		Ref<Image> radioOn;
		Ref<Image> radioOff;
		Ref<Image> groupOpen;
		Ref<Image> groupClosed;
		String rainbowPath;
		String radioOnPath;
		String radioOffPath;
		String groupOpenPath;
		String groupClosedPath;
	} icon;

	static Color hex(int n) {
		return Color::hex(n);
	}
};

/// ofxDatGuiIntObjects

namespace ofxDatGuiMsg {
extern const String EVENT_HANDLER_NULL;
extern const String COMPONENT_NOT_FOUND;
extern const String MATRIX_EMPTY;
} //namespace ofxDatGuiMsg

class ofxDatGuiLog {
public:
	static void write(String m1, String m2 = "") {
		if (!mQuiet) {
			if (m2 != "")
				print_line(m1 + " : " + m2);
			else
				print_line(m1);
		}
	}
	static void quiet() {
		mQuiet = true;
	}
	static bool mQuiet;
};

inline static float ofxDatGuiScale(float val, float min, float max) {
	if (min < 0) {
		float n = Math::abs(min);
		float a = min + n;
		float b = max + n;
		float c = val + n;
		return (c - a) / (b - a);
	} else {
		return (val - min) / (max - min);
	}
}

class ofxDatGuiInteractiveObject {
public:
	// button events
	typedef std::function<void(ofxDatGuiButtonEvent)> onButtonEventCallback;
	onButtonEventCallback buttonEventCallback;

	template <typename T, typename args, class ListenerClass>
	void onButtonEvent(T *owner, void (ListenerClass::*listenerMethod)(args)) { buttonEventCallback = std::bind(listenerMethod, owner, std::placeholders::_1); }

	void onButtonEvent(onButtonEventCallback callback) { buttonEventCallback = callback; }

	// toggle events
	typedef std::function<void(ofxDatGuiToggleEvent)> onToggleEventCallback;
	onToggleEventCallback toggleEventCallback;

	template <typename T, typename args, class ListenerClass>
	void onToggleEvent(T *owner, void (ListenerClass::*listenerMethod)(args)) { toggleEventCallback = std::bind(listenerMethod, owner, std::placeholders::_1); }

	void onToggleEvent(onToggleEventCallback callback) { toggleEventCallback = callback; }

	// slider events
	typedef std::function<void(ofxDatGuiSliderEvent)> onSliderEventCallback;
	onSliderEventCallback sliderEventCallback;

	template <typename T, typename args, class ListenerClass>
	void onSliderEvent(T *owner, void (ListenerClass::*listenerMethod)(args)) { sliderEventCallback = std::bind(listenerMethod, owner, std::placeholders::_1); }

	void onSliderEvent(onSliderEventCallback callback) { sliderEventCallback = callback; }

	// text input events
	typedef std::function<void(ofxDatGuiTextInputEvent)> onTextInputEventCallback;
	onTextInputEventCallback textInputEventCallback;

	template <typename T, typename args, class ListenerClass>
	void onTextInputEvent(T *owner, void (ListenerClass::*listenerMethod)(args)) { textInputEventCallback = std::bind(listenerMethod, owner, std::placeholders::_1); }

	void onTextInputEvent(onTextInputEventCallback callback) { textInputEventCallback = callback; }

	// color picker events
	typedef std::function<void(ofxDatGuiColorPickerEvent)> onColorPickerEventCallback;
	onColorPickerEventCallback colorPickerEventCallback;

	template <typename T, typename args, class ListenerClass>
	void onColorPickerEvent(T *owner, void (ListenerClass::*listenerMethod)(args)) { colorPickerEventCallback = std::bind(listenerMethod, owner, std::placeholders::_1); }

	void onColorPickerEvent(onColorPickerEventCallback callback) { colorPickerEventCallback = callback; }

	// dropdown events
	typedef std::function<void(ofxDatGuiDropdownEvent)> onDropdownEventCallback;
	onDropdownEventCallback dropdownEventCallback;

	template <typename T, typename args, class ListenerClass>
	void onDropdownEvent(T *owner, void (ListenerClass::*listenerMethod)(args)) { dropdownEventCallback = std::bind(listenerMethod, owner, std::placeholders::_1); }

	void onDropdownEvent(onDropdownEventCallback callback) { dropdownEventCallback = callback; }

	// 2d pad events
	typedef std::function<void(ofxDatGui2dPadEvent)> on2dPadEventCallback;
	on2dPadEventCallback pad2dEventCallback;

	template <typename T, typename args, class ListenerClass>
	void on2dPadEvent(T *owner, void (ListenerClass::*listenerMethod)(args)) { pad2dEventCallback = std::bind(listenerMethod, owner, std::placeholders::_1); }

	void on2dPadEvent(on2dPadEventCallback callback) { pad2dEventCallback = callback; }

	// matrix events
	typedef std::function<void(ofxDatGuiMatrixEvent)> onMatrixEventCallback;
	onMatrixEventCallback matrixEventCallback;

	template <typename T, typename args, class ListenerClass>
	void onMatrixEvent(T *owner, void (ListenerClass::*listenerMethod)(args)) { matrixEventCallback = std::bind(listenerMethod, owner, std::placeholders::_1); }

	void onMatrixEvent(onMatrixEventCallback callback) { matrixEventCallback = callback; }

	// scrollview events
	typedef std::function<void(ofxDatGuiScrollViewEvent)> onScrollViewEventCallback;
	onScrollViewEventCallback scrollViewEventCallback;

	template <typename T, typename args, class ListenerClass>
	void onScrollViewEvent(T *owner, void (ListenerClass::*listenerMethod)(args)) { scrollViewEventCallback = std::bind(listenerMethod, owner, std::placeholders::_1); }

	void onScrollViewEvent(onScrollViewEventCallback callback) { scrollViewEventCallback = callback; }

	// internal events
	typedef std::function<void(ofxDatGuiInternalEvent)> onInternalEventCallback;
	onInternalEventCallback internalEventCallback;

	template <typename T, typename args, class ListenerClass>
	void onInternalEvent(T *owner, void (ListenerClass::*listenerMethod)(args)) { internalEventCallback = std::bind(listenerMethod, owner, std::placeholders::_1); }

	void onInternalEvent(onInternalEventCallback callback) { internalEventCallback = callback; }
};

/// ofxDatGuiComponent

class ofxDatGuiComponent : public CanvasItem, public ofxDatGuiInteractiveObject {
	GDCLASS(ofxDatGuiComponent, CanvasItem);
	static std::unique_ptr<ofxDatGuiTheme> theme;

protected:
	int x;
	int y;
	int mIndex;
	String mName;
	bool mFocused;
	bool mVisible;
	bool mEnabled;
	bool mMouseOver;
	bool mMouseDown;
	Rect2 mMask;
	ofxDatGuiType mType;
	ofxDatGuiAnchor mAnchor;
	Ref<ofxSmartFont> mFont;

	struct {
		float width;
		float height;
		float padding;
		float vMargin;
		float opacity;
		struct {
			Color inputArea;
			Color background;
			Color onMouseOver;
			Color onMouseDown;
		} color;
		struct {
			int width;
			bool visible;
			Color color;
		} border;
		struct {
			int width;
			bool visible;
			Color color;
		} stripe;
		Color guiBackground;
	} mStyle;

	struct {
		int x;
		String text;
		String rendered;
		bool visible;
		Color color;
		float width;
		int margin;
		int rightAlignedXpos;
		Rect2 rect;
		bool forceUpperCase;
		ofxDatGuiAlignment alignment;
	} mLabel;

	struct {
		int x;
		int y;
		int size;
		Color color;
	} mIcon;

	void drawLabel();
	void drawBorder();
	void drawStripe();
	void drawBackground();
	void positionLabel();
	void setComponentStyle(const ofxDatGuiTheme *t);

public:
	int getX();
	int getY();
	void setIndex(int index);
	int getIndex();
	void setName(String name);
	String getName();
	bool is(String name);

	void setLabel(String label);
	String getLabel();
	void setLabelColor(Color color);
	Color getLabelColor();
	void setLabelUpperCase(bool toUpper);
	bool getLabelUpperCase();

	void setBackgroundColor(Color color);
	void setBackgroundColorOnMouseOver(Color color);
	void setBackgroundColorOnMouseDown(Color color);
	void setBackgroundColors(Color bkgd, Color mOver, Color mDown);

	void setStripe(Color color, int width);
	void setStripeWidth(int width);
	void setStripeColor(Color color);
	void setStripeVisible(bool visible);

	void setBorder(Color color, int width);
	void setBorderVisible(bool visible);

	void setMask(const Rect2 &mask);
	void setAnchor(ofxDatGuiAnchor anchor);
	void setEnabled(bool visible);
	bool getEnabled();
	void setVisible(bool visible);
	bool getVisible();
	void setFocused(bool focused);
	bool getFocused();
	void setOpacity(float opacity);
	bool getMouseDown();
	ofxDatGuiType getType();

	// Godot property binding: bind component value to an object property.
	// The component polls the property each frame (in update()) and writes back on change.
	// Works with any Variant-compatible property.
	struct GodotPropertyBinding {
		ObjectID object_id;
		String property;
		bool active;
		GodotPropertyBinding() :
				object_id(0), active(false) {}
	};
	GodotPropertyBinding mPropertyBinding;

	void bind_godot_property(Object *p_object, const String &p_property);
	void unbind_godot_property();
	bool has_godot_property_binding() const { return mPropertyBinding.active; }

	std::vector<ofxDatGuiComponent *> children;

	virtual void draw();
	virtual void update(bool acceptEvents = true);
	virtual bool hitTest(Point2 m);

	virtual void setPosition(int x, int y);
	virtual void setTheme(const ofxDatGuiTheme *theme) = 0;
	virtual void setWidth(int width, float labelWidth);
	virtual void setLabelAlignment(ofxDatGuiAlignment align);

	virtual int getWidth();
	virtual int getHeight();
	virtual bool getIsExpanded();
	virtual void drawColorPicker();

	virtual void onFocus();
	virtual void onFocusLost();
	virtual void onWindowResized();
	virtual void onKeyPressed(int key);
	virtual void onMouseEnter(Point2 m);
	virtual void onMousePress(Point2 m);
	virtual void onMouseDrag(Point2 m);
	virtual void onMouseLeave(Point2 m);
	virtual void onMouseRelease(Point2 m);
	void onWindowResized(int width, int height);

	// Central event handler
	virtual void _message(ofxDatGuiEvent event);

	static const ofxDatGuiTheme *getTheme();

	ofxDatGuiComponent(String label);
	virtual ~ofxDatGuiComponent();
};

/// ofxDataGuiThemes

// Stock Themes

class ofxDatGuiThemeSmoke : public ofxDatGuiTheme {
public:
	ofxDatGuiThemeSmoke() {
		stripe.visible = false;
		color.label = hex(0xF8F3F0);
		color.guiBackground = hex(0x2C3137);
		color.background = hex(0x343B41);
		color.slider.fill = hex(0x60B9ED);
		color.slider.text = hex(0xFFFFFF);
		color.inputAreaBackground = hex(0x434A50);
		color.textInput.text = hex(0xFFFFFF);
		color.textInput.highlight = hex(0x434A50);
		color.textInput.backgroundOnActive = hex(0x2C3137);
		color.backgroundOnMouseOver = hex(0x434A50);
		color.backgroundOnMouseDown = hex(0x2C3137);
		color.matrix.hover.button = hex(0x60B9ED);
		color.matrix.selected.button = hex(0x2C3137);
		init();
	}
};

class ofxDatGuiThemeWireframe : public ofxDatGuiTheme {
public:
	ofxDatGuiThemeWireframe() {
		stripe.visible = false;
		color.label = hex(0x6E6E6E);
		color.icons = hex(0x6E6E6E);
		color.background = hex(0xFCFAFD);
		color.guiBackground = hex(0xD8D8DB);
		color.inputAreaBackground = hex(0xE9E9E9);
		color.slider.fill = hex(0x6E6E6E);
		color.slider.text = hex(0x6E6E6E);
		color.textInput.text = hex(0x6E6E6E);
		color.textInput.highlight = hex(0xFCFAFD);
		color.colorPicker.border = hex(0xDFDDDF);
		color.textInput.backgroundOnActive = hex(0xD1D1D1);
		color.backgroundOnMouseOver = hex(0xECECEC);
		color.backgroundOnMouseDown = hex(0xDFDDDF);
		color.matrix.normal.button = hex(0xDFDDDF);
		color.matrix.hover.button = hex(0x9C9DA1);
		color.matrix.selected.button = hex(0x6E6E6E);
		color.pad2d.line = hex(0x6E6E6E);
		color.pad2d.ball = hex(0x6E6E6E);
		color.graph.fills = hex(0x6E6E6E);
		init();
	}
};

class ofxDatGuiThemeMidnight : public ofxDatGuiTheme {
public:
	ofxDatGuiThemeMidnight() {
		stripe.visible = false;
		color.label = hex(0xffffff);
		color.background = hex(0x011726);
		color.guiBackground = hex(0x4C5B66);
		color.inputAreaBackground = hex(0x273946);
		color.slider.fill = hex(0x0A1E2E);
		color.slider.text = hex(0xffffff);
		color.textInput.text = hex(0xffffff);
		color.textInput.highlight = hex(0x596872);
		color.textInput.backgroundOnActive = hex(0x0A1E2E);
		color.backgroundOnMouseOver = hex(0x273946);
		color.backgroundOnMouseDown = hex(0x000000);
		color.matrix.hover.button = hex(0x596872);
		color.matrix.selected.button = hex(0x60B1CC);
		color.graph.fills = hex(0x596872);
		init();
	}
};

class ofxDatGuiThemeAqua : public ofxDatGuiTheme {
public:
	ofxDatGuiThemeAqua() {
		stripe.visible = false;
		color.label = hex(0xF8F3F0);
		color.guiBackground = hex(0xF8F3F0);
		color.background = hex(0x445966);
		color.inputAreaBackground = hex(0x61717D);
		color.slider.fill = hex(0xF8F3F0);
		color.slider.text = hex(0xFFFFFF);
		color.textInput.text = hex(0xFFFFFF);
		color.textInput.highlight = hex(0x445966);
		color.textInput.backgroundOnActive = hex(0x334553);
		color.backgroundOnMouseOver = hex(0x61717D);
		color.backgroundOnMouseDown = hex(0x334553);
		color.matrix.hover.button = hex(0x55666F);
		color.matrix.selected.button = hex(0x334553);
		init();
	}
};

class ofxDatGuiThemeCharcoal : public ofxDatGuiTheme {
public:
	ofxDatGuiThemeCharcoal() {
		stripe.visible = false;
		color.label = hex(0x9C9DA1);
		color.icons = hex(0x9C9DA1);
		color.background = hex(0x28292E);
		color.guiBackground = hex(0x1E1F24);
		color.inputAreaBackground = hex(0x42424A);
		color.slider.fill = hex(0xF4BF39);
		color.slider.text = hex(0x9C9DA1);
		color.textInput.text = hex(0x9C9DA1);
		color.textInput.highlight = hex(0x28292E);
		color.colorPicker.border = hex(0xEEEEEE);
		color.textInput.backgroundOnActive = hex(0x1D1E22);
		color.backgroundOnMouseOver = hex(0x42424A);
		color.backgroundOnMouseDown = hex(0x1D1E22);
		color.matrix.hover.button = hex(0x9C9DA1);
		color.graph.fills = hex(0x9C9DA1);
		init();
	}
};

class ofxDatGuiThemeAutumn : public ofxDatGuiTheme {
public:
	ofxDatGuiThemeAutumn() {
		stripe.visible = false;
		color.label = hex(0xF8F3F0);
		color.guiBackground = hex(0x7d7066);
		color.background = hex(0x4C4743);
		color.inputAreaBackground = hex(0xB5BCB2);
		color.slider.fill = hex(0xFFB230);
		color.slider.text = hex(0xF8F3F0);
		color.textInput.text = hex(0xF8F3F0);
		color.textInput.highlight = hex(0x4C4743);
		color.textInput.backgroundOnActive = hex(0x7d7066);
		color.backgroundOnMouseOver = hex(0x7d7066);
		color.backgroundOnMouseDown = hex(0x333333);
		color.matrix.hover.button = hex(0xC3A279);
		color.matrix.selected.button = hex(0x7d7066);
		init();
	}
};

class ofxDatGuiThemeCandy : public ofxDatGuiTheme {
public:
	ofxDatGuiThemeCandy() {
		stripe.visible = false;
		color.label = hex(0xFFFFFF);
		color.icons = hex(0xFFFFFF);
		color.background = hex(0xFF4081);
		color.guiBackground = hex(0xEEEEEE);
		color.inputAreaBackground = hex(0xFF80AB);
		color.slider.fill = hex(0xF50057);
		color.slider.text = hex(0xFFFFFF);
		color.textInput.text = hex(0xFFFFFF);
		color.textInput.highlight = hex(0xFF4081);
		color.colorPicker.border = hex(0xDFDDDF);
		color.textInput.backgroundOnActive = hex(0xF50057);
		color.backgroundOnMouseOver = hex(0xFF80AB);
		color.backgroundOnMouseDown = hex(0xF50057);
		color.matrix.normal.button = hex(0xFFFFFF);
		color.matrix.normal.label = hex(0xFF4081);
		color.matrix.hover.button = hex(0xFF4081);
		color.matrix.selected.button = hex(0xF50057);
		color.pad2d.line = hex(0xFFFFFF);
		color.pad2d.ball = hex(0xFFFFFF);
		color.graph.fills = hex(0xF50057);
		init();
	}
};

/// ofxDatGui

class ofxDatGui : public Node2D, public ofxDatGuiInteractiveObject {
	GDCLASS(ofxDatGui, Node2D);
	int mIndex;
	int mWidth;
	int mHeight;
	int mRowSpacing;
	float mAlpha;
	float mLabelWidth;
	bool mMoving;
	bool mVisible;
	bool mEnabled;
	bool mExpanded;
	bool mAutoDraw;
	bool mMouseDown;
	bool mAlphaChanged;
	bool mWidthChanged;
	bool mThemeChanged;
	bool mAlignmentChanged;
	Color mGuiBackground;

	Point2 mPosition;
	Rect2 mGuiBounds;
	ofxDatGuiAnchor mAnchor;
	ofxDatGuiHeader *mGuiHeader;
	ofxDatGuiFooter *mGuiFooter;
	ofxDatGuiTheme *mTheme;
	ofxDatGuiAlignment mAlignment;
	std::vector<ofxDatGuiComponent *> items;
	std::vector<ofxDatGuiComponent *> trash;
	static ofxDatGui *mActiveGui;
	static std::vector<ofxDatGui *> mGuis;
	static std::unique_ptr<ofxDatGuiTheme> theme;

	void init();
	void layoutGui();
	void positionGui();
	void moveGui(Point2 pt);
	bool hitTest(Point2 pt);
	void attachItem(ofxDatGuiComponent *item);

	void onDraw();
	void onUpdate();
	void onWindowResized(int width, int height);

	ofxDatGuiComponent *getComponent(String key);
	ofxDatGuiComponent *getComponent(ofxDatGuiType type, String label);
	void onInternalEventCallback(ofxDatGuiInternalEvent e);
	void onButtonEventCallback(ofxDatGuiButtonEvent e);
	void onToggleEventCallback(ofxDatGuiToggleEvent e);
	void onSliderEventCallback(ofxDatGuiSliderEvent e);
	void onTextInputEventCallback(ofxDatGuiTextInputEvent e);
	void onDropdownEventCallback(ofxDatGuiDropdownEvent e);
	void on2dPadEventCallback(ofxDatGui2dPadEvent e);
	void onColorPickerEventCallback(ofxDatGuiColorPickerEvent e);
	void onMatrixEventCallback(ofxDatGuiMatrixEvent e);

public:
	void draw();
	void update();
	void focus();
	void expand();
	void toggle();
	void collapse();

	void setWidth(int width, float labelWidth = 0.35f);
	void setVisible(bool visible);
	void setEnabled(bool enabled);
	void setOpacity(float opacity);
	void setPosition(int x, int y);
	void setPosition(ofxDatGuiAnchor anchor);
	void setTheme(ofxDatGuiTheme *t, bool applyImmediately = false);
	void setAutoDraw(bool autodraw, int priority = 0);
	void setLabelAlignment(ofxDatGuiAlignment align);
	static void setAssetPath(String path);
	static String getAssetPath();

	int getWidth();
	int getHeight();
	bool getFocused();
	bool getVisible();
	bool getAutoDraw();
	bool getMouseDown();
	Point2 getPosition();

	ofxDatGuiHeader *addHeader(String label = "", bool draggable = true);
	ofxDatGuiFooter *addFooter();
	ofxDatGuiLabel *addLabel(String label);
	ofxDatGuiButton *addButton(String label);
	ofxDatGuiToggle *addToggle(String label, bool state = false);
	ofxDatGuiSlider *addSlider(String label, float min, float max);
	ofxDatGuiSlider *addSlider(String label, float min, float max, float val);
	ofxDatGuiTextInput *addTextInput(String label, String value = "");
	ofxDatGuiDropdown *addDropdown(String label, PoolStringArray options);
	ofxDatGuiFRM *addFRM(float refresh = 1);
	ofxDatGuiBreak *addBreak();
	ofxDatGui2dPad *add2dPad(String label);
	ofxDatGui2dPad *add2dPad(String label, Rect2 bounds);
	ofxDatGuiWaveMonitor *addWaveMonitor(String label, float min, float max);
	ofxDatGuiValuePlotter *addValuePlotter(String label, float min, float max);
	ofxDatGuiColorPicker *addColorPicker(String label, Color color = Color::named("black"));
	ofxDatGuiMatrix *addMatrix(String label, int numButtons, bool showLabels = false);
	ofxDatGuiFolder *addFolder(String label, Color color = Color::named("white"));
	ofxDatGuiFolder *addFolder(ofxDatGuiFolder *folder);

	ofxDatGuiHeader *getHeader();
	ofxDatGuiFooter *getFooter();
	ofxDatGuiLabel *getLabel(String label, String folder = "");
	ofxDatGuiButton *getButton(String label, String folder = "");
	ofxDatGuiToggle *getToggle(String label, String folder = "");
	ofxDatGuiSlider *getSlider(String label, String folder = "");
	ofxDatGui2dPad *get2dPad(String label, String folder = "");
	ofxDatGuiTextInput *getTextInput(String label, String folder = "");
	ofxDatGuiColorPicker *getColorPicker(String label, String folder = "");
	ofxDatGuiMatrix *getMatrix(String label, String folder = "");
	ofxDatGuiWaveMonitor *getWaveMonitor(String label, String folder = "");
	ofxDatGuiValuePlotter *getValuePlotter(String label, String folder = "");
	ofxDatGuiFolder *getFolder(String label);
	ofxDatGuiDropdown *getDropdown(String label);

	ofxDatGui(int x, int y);
	ofxDatGui(ofxDatGuiAnchor anchor = ofxDatGuiAnchor::TOP_LEFT);
	~ofxDatGui();
};

#endif // OFX_DATGUI_H
