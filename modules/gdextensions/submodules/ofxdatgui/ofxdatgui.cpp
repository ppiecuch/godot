/// Copyright (C) 2015 Stephen Braitsch [http://braitsch.io]

#include "ofxdatgui.h"

#include "core/io/image_loader.h"

#include <algorithm>
#include <memory>
#include <vector>

// Embed data resources via INCBIN so no external files needed at runtime
#undef INCBIN_PREFIX
#define INCBIN_PREFIX
#define INCBIN_STYLE INCBIN_STYLE_SNAKE
#define INCBIN_SILENCE_BITCODE_WARNING
#include "misc/incbin.h"

INCBIN(ofxdg_group_closed, "submodules/ofxdatgui/data/icon-group-closed.png");
INCBIN(ofxdg_group_open, "submodules/ofxdatgui/data/icon-group-open.png");
INCBIN(ofxdg_radio_off, "submodules/ofxdatgui/data/icon-radio-off.png");
INCBIN(ofxdg_radio_on, "submodules/ofxdatgui/data/icon-radio-on.png");
INCBIN(ofxdg_rainbow, "submodules/ofxdatgui/data/picker-rainbow.png");

// Use the shared vera.ttf embedded in register_types.cpp
extern const uint8_t vera_ttf_data[];
extern const uint8_t *vera_ttf_end;
extern const unsigned int vera_ttf_size;

// Helper: load Image from embedded PNG data
static Ref<Image> _load_embedded_png(const uint8_t *p_data, unsigned int p_size) {
	Ref<Image> img;
	img.instance();
	PoolByteArray buf;
	buf.resize(p_size);
	{
		PoolByteArray::Write w = buf.write();
		memcpy(w.ptr(), p_data, p_size);
	}
	img->load_png_from_buffer(buf);
	return img;
}

// Helper: create DynamicFont from embedded vera.ttf data
static Ref<DynamicFont> _create_embedded_font(int p_size) {
	Ref<DynamicFont> font;
	font.instance();
	Ref<DynamicFontData> font_data;
	font_data.instance();
	font_data->set_font_ptr(vera_ttf_data, vera_ttf_size);
	font->set_font_data(font_data);
	font->set_size(p_size);
	return font;
}

// Static constant definitions
String ofxDatGuiTheme::AssetPath = "/res/ofxdatgui";

void ofxDatGuiTheme::init() {
	// Load icons from embedded INCBIN data (no external files needed)
	icon.radioOn = _load_embedded_png(ofxdg_radio_on_data, ofxdg_radio_on_size);
	icon.radioOff = _load_embedded_png(ofxdg_radio_off_data, ofxdg_radio_off_size);
	icon.groupOpen = _load_embedded_png(ofxdg_group_open_data, ofxdg_group_open_size);
	icon.groupClosed = _load_embedded_png(ofxdg_group_closed_data, ofxdg_group_closed_size);
	icon.rainbow = _load_embedded_png(ofxdg_rainbow_data, ofxdg_rainbow_size);

	// Create font from embedded vera.ttf (shared with other modules via register_types.cpp)
	font.ptr = _create_embedded_font(font.size);
}

namespace ofxDatGuiMsg {
const String EVENT_HANDLER_NULL = "[WARNING] :: Event Handler Not Set";
const String COMPONENT_NOT_FOUND = "[ERROR] :: Component Not Found";
const String MATRIX_EMPTY = "[WARNING] :: Matrix is Empty";
} //namespace ofxDatGuiMsg

// Initialize theme struct members that depend on AssetPath
void initializeThemeAssets() {
	// This would be called to initialize the theme paths and images
}

/// BEGIN ofxDataGui

// Basic stub implementations for missing GUI component classes
class ofxDatGuiHeader : public ofxDatGuiComponent {
public:
	ofxDatGuiHeader(String label, bool draggable = false) :
			ofxDatGuiComponent(label) {
		mType = ofxDatGuiType::BUTTON; // Temporary type
		mDraggable = draggable;
	}
	void setTheme(const ofxDatGuiTheme *theme) override {} // Required pure virtual
	void setExpanded(bool expanded) { mExpanded = expanded; }
	void setPosition(int x, int y) {
		this->x = x;
		this->y = y;
	}
	int getHeight() { return mStyle.height; }
	bool getDraggable() { return mDraggable; }
	Point2 getDragOffset() { return Point2(0, 0); }
	bool getFocused() { return mFocused; }
	template <class T, typename M>
	void onInternalEvent(T *owner, M method) {} // Event binding stub
	// CanvasItem pure virtual methods
	void _edit_set_position(const Point2 &p_position) override {
		x = p_position.x;
		y = p_position.y;
	}
	Point2 _edit_get_position() const override { return Point2(x, y); }
	void _edit_set_scale(const Size2 &p_scale) override { /* stub */
	}
	Size2 _edit_get_scale() const override { return Size2(1, 1); }
	Transform2D get_transform() const override { return Transform2D(0, Point2(x, y)); }

protected:
	bool mDraggable = false;
	bool mExpanded = true;
};

class ofxDatGuiFooter : public ofxDatGuiComponent {
public:
	ofxDatGuiFooter() :
			ofxDatGuiComponent(String("Footer")) {
		mType = ofxDatGuiType::BUTTON; // Temporary type
	}
	void setTheme(const ofxDatGuiTheme *theme) override {} // Required pure virtual
	void setExpanded(bool expanded) { mExpanded = expanded; }
	void setPosition(int x, int y) {
		this->x = x;
		this->y = y;
	}
	int getHeight() { return mStyle.height; }
	template <class T, typename M>
	void onInternalEvent(T *owner, M method) {} // Event binding stub
	// CanvasItem pure virtual methods
	void _edit_set_position(const Point2 &p_position) override {
		x = p_position.x;
		y = p_position.y;
	}
	Point2 _edit_get_position() const override { return Point2(x, y); }
	void _edit_set_scale(const Size2 &p_scale) override { /* stub */
	}
	Size2 _edit_get_scale() const override { return Size2(1, 1); }
	Transform2D get_transform() const override { return Transform2D(0, Point2(x, y)); }

protected:
	bool mExpanded = true;
};

// Other stub implementations for missing component classes
class ofxDatGuiLabel : public ofxDatGuiComponent {
public:
	ofxDatGuiLabel(String label) :
			ofxDatGuiComponent(label) { mType = ofxDatGuiType::LABEL; }
	void setTheme(const ofxDatGuiTheme *theme) override {} // Required pure virtual
	static ofxDatGuiLabel *getInstance() { return new ofxDatGuiLabel("Placeholder"); }
	// CanvasItem pure virtual methods
	void _edit_set_position(const Point2 &p_position) override {
		x = p_position.x;
		y = p_position.y;
	}
	Point2 _edit_get_position() const override { return Point2(x, y); }
	void _edit_set_scale(const Size2 &p_scale) override { /* stub */
	}
	Size2 _edit_get_scale() const override { return Size2(1, 1); }
	Transform2D get_transform() const override { return Transform2D(0, Point2(x, y)); }
};

class ofxDatGuiButton : public ofxDatGuiComponent {
public:
	ofxDatGuiButton(String label) :
			ofxDatGuiComponent(label) { mType = ofxDatGuiType::BUTTON; }
	void setTheme(const ofxDatGuiTheme *theme) override {} // Required pure virtual
	static ofxDatGuiButton *getInstance() { return new ofxDatGuiButton("Placeholder"); }
	template <class T, typename M>
	void onButtonEvent(T *owner, M method) {} // Event binding stub
	// CanvasItem pure virtual methods
	void _edit_set_position(const Point2 &p_position) override {
		x = p_position.x;
		y = p_position.y;
	}
	Point2 _edit_get_position() const override { return Point2(x, y); }
	void _edit_set_scale(const Size2 &p_scale) override { /* stub */
	}
	Size2 _edit_get_scale() const override { return Size2(1, 1); }
	Transform2D get_transform() const override { return Transform2D(0, Point2(x, y)); }
};

class ofxDatGuiToggle : public ofxDatGuiComponent {
public:
	ofxDatGuiToggle(String label, bool enabled) :
			ofxDatGuiComponent(label) { mType = ofxDatGuiType::TOGGLE; }
	void setTheme(const ofxDatGuiTheme *theme) override {} // Required pure virtual
	static ofxDatGuiToggle *getInstance() { return new ofxDatGuiToggle("Placeholder", false); }
	template <class T, typename M>
	void onToggleEvent(T *owner, M method) {} // Event binding stub
	// CanvasItem pure virtual methods
	void _edit_set_position(const Point2 &p_position) override {
		x = p_position.x;
		y = p_position.y;
	}
	Point2 _edit_get_position() const override { return Point2(x, y); }
	void _edit_set_scale(const Size2 &p_scale) override { /* stub */
	}
	Size2 _edit_get_scale() const override { return Size2(1, 1); }
	Transform2D get_transform() const override { return Transform2D(0, Point2(x, y)); }
};

class ofxDatGuiSlider : public ofxDatGuiComponent {
public:
	ofxDatGuiSlider(String label, float min, float max, float val) :
			ofxDatGuiComponent(label) { mType = ofxDatGuiType::SLIDER; }
	void setTheme(const ofxDatGuiTheme *theme) override {} // Required pure virtual
	static ofxDatGuiSlider *getInstance() { return new ofxDatGuiSlider("Placeholder", 0, 100, 50); }
	template <class T, typename M>
	void onSliderEvent(T *owner, M method) {} // Event binding stub
	// CanvasItem pure virtual methods
	void _edit_set_position(const Point2 &p_position) override {
		x = p_position.x;
		y = p_position.y;
	}
	Point2 _edit_get_position() const override { return Point2(x, y); }
	void _edit_set_scale(const Size2 &p_scale) override { /* stub */
	}
	Size2 _edit_get_scale() const override { return Size2(1, 1); }
	Transform2D get_transform() const override { return Transform2D(0, Point2(x, y)); }
};

class ofxDatGuiTextInput : public ofxDatGuiComponent {
public:
	ofxDatGuiTextInput(String label, String value) :
			ofxDatGuiComponent(label) { mType = ofxDatGuiType::TEXT_INPUT; }
	void setTheme(const ofxDatGuiTheme *theme) override {} // Required pure virtual
	static ofxDatGuiTextInput *getInstance() { return new ofxDatGuiTextInput("Placeholder", ""); }
	template <class T, typename M>
	void onTextInputEvent(T *owner, M method) {} // Event binding stub
	// CanvasItem pure virtual methods
	void _edit_set_position(const Point2 &p_position) override {
		x = p_position.x;
		y = p_position.y;
	}
	Point2 _edit_get_position() const override { return Point2(x, y); }
	void _edit_set_scale(const Size2 &p_scale) override { /* stub */
	}
	Size2 _edit_get_scale() const override { return Size2(1, 1); }
	Transform2D get_transform() const override { return Transform2D(0, Point2(x, y)); }
};

class ofxDatGuiColorPicker : public ofxDatGuiComponent {
public:
	ofxDatGuiColorPicker(String label, Color color) :
			ofxDatGuiComponent(label) { mType = ofxDatGuiType::COLOR_PICKER; }
	// CanvasItem pure virtual methods
	void _edit_set_position(const Point2 &p_position) override {
		x = p_position.x;
		y = p_position.y;
	}
	Point2 _edit_get_position() const override { return Point2(x, y); }
	void _edit_set_scale(const Size2 &p_scale) override { /* stub */
	}
	Size2 _edit_get_scale() const override { return Size2(1, 1); }
	Transform2D get_transform() const override { return Transform2D(0, Point2(x, y)); }
	void setTheme(const ofxDatGuiTheme *theme) override {} // Required pure virtual
	static ofxDatGuiColorPicker *getInstance() { return new ofxDatGuiColorPicker("Placeholder", Color::named("black")); }
	template <class T, typename M>
	void onColorPickerEvent(T *owner, M method) {} // Event binding stub
};

class ofxDatGuiWaveMonitor : public ofxDatGuiComponent {
public:
	ofxDatGuiWaveMonitor(String label, float frequency, float amplitude) :
			ofxDatGuiComponent(label) { mType = ofxDatGuiType::WAVE_MONITOR; }
	void setTheme(const ofxDatGuiTheme *theme) override {} // Required pure virtual
	static ofxDatGuiWaveMonitor *getInstance() { return new ofxDatGuiWaveMonitor("Placeholder", 1.0, 1.0); }
	// CanvasItem pure virtual methods
	void _edit_set_position(const Point2 &p_position) override {
		x = p_position.x;
		y = p_position.y;
	}
	Point2 _edit_get_position() const override { return Point2(x, y); }
	void _edit_set_scale(const Size2 &p_scale) override { /* stub */
	}
	Size2 _edit_get_scale() const override { return Size2(1, 1); }
	Transform2D get_transform() const override { return Transform2D(0, Point2(x, y)); }
};

class ofxDatGuiValuePlotter : public ofxDatGuiComponent {
public:
	ofxDatGuiValuePlotter(String label, float min, float max) :
			ofxDatGuiComponent(label) { mType = ofxDatGuiType::VALUE_PLOTTER; }
	void setTheme(const ofxDatGuiTheme *theme) override {} // Required pure virtual
	static ofxDatGuiValuePlotter *getInstance() { return new ofxDatGuiValuePlotter("Placeholder", 0, 100); }
	// CanvasItem pure virtual methods
	void _edit_set_position(const Point2 &p_position) override {
		x = p_position.x;
		y = p_position.y;
	}
	Point2 _edit_get_position() const override { return Point2(x, y); }
	void _edit_set_scale(const Size2 &p_scale) override { /* stub */
	}
	Size2 _edit_get_scale() const override { return Size2(1, 1); }
	Transform2D get_transform() const override { return Transform2D(0, Point2(x, y)); }
};

class ofxDatGuiDropdown : public ofxDatGuiComponent {
public:
	ofxDatGuiDropdown(String label, PoolStringArray options) :
			ofxDatGuiComponent(label) { mType = ofxDatGuiType::DROPDOWN; }
	void setTheme(const ofxDatGuiTheme *theme) override {} // Required pure virtual
	static ofxDatGuiDropdown *getInstance() { return new ofxDatGuiDropdown("Placeholder", {}); }
	template <class T, typename M>
	void onDropdownEvent(T *owner, M method) {} // Event binding stub
	// CanvasItem pure virtual methods
	void _edit_set_position(const Point2 &p_position) override {
		x = p_position.x;
		y = p_position.y;
	}
	Point2 _edit_get_position() const override { return Point2(x, y); }
	void _edit_set_scale(const Size2 &p_scale) override { /* stub */
	}
	Size2 _edit_get_scale() const override { return Size2(1, 1); }
	Transform2D get_transform() const override { return Transform2D(0, Point2(x, y)); }
};

class ofxDatGuiFRM : public ofxDatGuiComponent {
public:
	ofxDatGuiFRM(float refresh) :
			ofxDatGuiComponent(String("FRM")) { mType = ofxDatGuiType::FRAME_RATE; }
	void setTheme(const ofxDatGuiTheme *theme) override {} // Required pure virtual
	static ofxDatGuiFRM *getInstance() { return new ofxDatGuiFRM(1.0); }
	// CanvasItem pure virtual methods
	void _edit_set_position(const Point2 &p_position) override {
		x = p_position.x;
		y = p_position.y;
	}
	Point2 _edit_get_position() const override { return Point2(x, y); }
	void _edit_set_scale(const Size2 &p_scale) override { /* stub */
	}
	Size2 _edit_get_scale() const override { return Size2(1, 1); }
	Transform2D get_transform() const override { return Transform2D(0, Point2(x, y)); }
};

class ofxDatGuiBreak : public ofxDatGuiComponent {
public:
	ofxDatGuiBreak() :
			ofxDatGuiComponent(String("Break")) { mType = ofxDatGuiType::BREAK; }
	void setTheme(const ofxDatGuiTheme *theme) override {} // Required pure virtual
	static ofxDatGuiBreak *getInstance() { return new ofxDatGuiBreak(); }
	// CanvasItem pure virtual methods
	void _edit_set_position(const Point2 &p_position) override {
		x = p_position.x;
		y = p_position.y;
	}
	Point2 _edit_get_position() const override { return Point2(x, y); }
	void _edit_set_scale(const Size2 &p_scale) override { /* stub */
	}
	Size2 _edit_get_scale() const override { return Size2(1, 1); }
	Transform2D get_transform() const override { return Transform2D(0, Point2(x, y)); }
};

class ofxDatGui2dPad : public ofxDatGuiComponent {
public:
	ofxDatGui2dPad(String label) :
			ofxDatGuiComponent(label) { mType = ofxDatGuiType::PAD2D; }
	ofxDatGui2dPad(String label, Rect2 bounds) :
			ofxDatGuiComponent(label) { mType = ofxDatGuiType::PAD2D; }
	void setTheme(const ofxDatGuiTheme *theme) override {} // Required pure virtual
	static ofxDatGui2dPad *getInstance() { return new ofxDatGui2dPad("Placeholder"); }
	template <class T, typename M>
	void on2dPadEvent(T *owner, M method) {} // Event binding stub
	// CanvasItem pure virtual methods
	void _edit_set_position(const Point2 &p_position) override {
		x = p_position.x;
		y = p_position.y;
	}
	Point2 _edit_get_position() const override { return Point2(x, y); }
	void _edit_set_scale(const Size2 &p_scale) override { /* stub */
	}
	Size2 _edit_get_scale() const override { return Size2(1, 1); }
	Transform2D get_transform() const override { return Transform2D(0, Point2(x, y)); }
};

class ofxDatGuiMatrix : public ofxDatGuiComponent {
public:
	ofxDatGuiMatrix(String label, int numButtons, bool showLabels) :
			ofxDatGuiComponent(label) { mType = ofxDatGuiType::MATRIX; }
	void setTheme(const ofxDatGuiTheme *theme) override {} // Required pure virtual
	static ofxDatGuiMatrix *getInstance() { return new ofxDatGuiMatrix("Placeholder", 4, false); }
	template <class T, typename M>
	void onMatrixEvent(T *owner, M method) {} // Event binding stub
	// CanvasItem pure virtual methods
	void _edit_set_position(const Point2 &p_position) override {
		x = p_position.x;
		y = p_position.y;
	}
	Point2 _edit_get_position() const override { return Point2(x, y); }
	void _edit_set_scale(const Size2 &p_scale) override { /* stub */
	}
	Size2 _edit_get_scale() const override { return Size2(1, 1); }
	Transform2D get_transform() const override { return Transform2D(0, Point2(x, y)); }
};

class ofxDatGuiFolder : public ofxDatGuiComponent {
public:
	ofxDatGuiFolder(String label, Color color) :
			ofxDatGuiComponent(label) { mType = ofxDatGuiType::FOLDER; }
	ofxDatGuiFolder(ofxDatGuiFolder *folder) :
			ofxDatGuiComponent("Folder Copy") { mType = ofxDatGuiType::FOLDER; }
	void setTheme(const ofxDatGuiTheme *theme) override {} // Required pure virtual
	static ofxDatGuiFolder *getInstance() { return new ofxDatGuiFolder("Placeholder", Color::named("white")); }
	ofxDatGuiComponent *getComponent(ofxDatGuiType type, String label) { return nullptr; } // Stub
	template <class T, typename M>
	void onButtonEvent(T *owner, M method) {} // Event binding stubs
	template <class T, typename M>
	void onToggleEvent(T *owner, M method) {}
	template <class T, typename M>
	void onSliderEvent(T *owner, M method) {}
	template <class T, typename M>
	void on2dPadEvent(T *owner, M method) {}
	template <class T, typename M>
	void onMatrixEvent(T *owner, M method) {}
	template <class T, typename M>
	void onTextInputEvent(T *owner, M method) {}
	template <class T, typename M>
	void onColorPickerEvent(T *owner, M method) {}
	template <class T, typename M>
	void onDropdownEvent(T *owner, M method) {}
	// CanvasItem pure virtual methods
	void _edit_set_position(const Point2 &p_position) override {
		x = p_position.x;
		y = p_position.y;
	}
	Point2 _edit_get_position() const override { return Point2(x, y); }
	void _edit_set_scale(const Size2 &p_scale) override { /* stub */
	}
	Size2 _edit_get_scale() const override { return Size2(1, 1); }
	Transform2D get_transform() const override { return Transform2D(0, Point2(x, y)); }
};

ofxDatGui *ofxDatGui::mActiveGui;
std::vector<ofxDatGui *> ofxDatGui::mGuis;

ofxDatGui::ofxDatGui(int x, int y) {
	mPosition.x = x;
	mPosition.y = y;
	mAnchor = ofxDatGuiAnchor::NO_ANCHOR;
	init();
}

ofxDatGui::ofxDatGui(ofxDatGuiAnchor anchor) {
	init();
	mAnchor = anchor;
}

ofxDatGui::~ofxDatGui() {
	for (auto i : items)
		delete i;
	mGuis.erase(std::remove(mGuis.begin(), mGuis.end(), this), mGuis.end());
	if (mActiveGui == this)
		mActiveGui = mGuis.size() > 0 ? mGuis[0] : nullptr;
	// Note: OpenFrameworks event listeners removed - need to implement Godot event system
}

void ofxDatGui::init() {
	mMoving = false;
	mVisible = true;
	mEnabled = true;
	mExpanded = true;
	mGuiHeader = nullptr;
	mGuiFooter = nullptr;
	mAlphaChanged = false;
	mWidthChanged = false;
	mThemeChanged = false;
	mAlignmentChanged = false;
	mAlignment = ofxDatGuiAlignment::LEFT;
	mAlpha = 1.0f;
	mWidth = ofxDatGuiComponent::getTheme()->layout.width;
	mRowSpacing = ofxDatGuiComponent::getTheme()->layout.vMargin;
	mGuiBackground = ofxDatGuiComponent::getTheme()->color.guiBackground;

	// enable autodraw by default //
	setAutoDraw(true, mGuis.size());

	// assign focus to this newly created gui //
	mActiveGui = this;
	mGuis.push_back(this);
	// ofAddListener removed - need Godot event system implementation
}

// public api

void ofxDatGui::focus() {
	if (mActiveGui != this) {
		// enable and make visible if hidden //
		mVisible = true;
		mEnabled = true;
		mActiveGui = this;
		// update the draw order //
		for (int i = 0; i < mGuis.size(); i++) {
			if (mGuis[i] == mActiveGui) {
				std::swap(mGuis[i], mGuis[mGuis.size() - 1]);
				break;
			}
		}
		for (int i = 0; i < mGuis.size(); i++) {
			if (mGuis[i]->getAutoDraw())
				mGuis[i]->setAutoDraw(true, i);
		}
	}
}

void ofxDatGui::expand() {
	if (mGuiFooter != nullptr) {
		mExpanded = true;
		mGuiFooter->setExpanded(mExpanded);
		mGuiFooter->setPosition(mPosition.x, mPosition.y + mHeight - mGuiFooter->getHeight() - mRowSpacing);
	}
}

void ofxDatGui::collapse() {
	if (mGuiFooter != nullptr) {
		mExpanded = false;
		mGuiFooter->setExpanded(mExpanded);
		mGuiFooter->setPosition(mPosition.x, mPosition.y);
	}
}

void ofxDatGui::toggle() { mExpanded ? collapse() : expand(); }

bool ofxDatGui::getVisible() { return mVisible; }

bool ofxDatGui::getFocused() { return mActiveGui == this; }

void ofxDatGui::setWidth(int width, float labelWidth) {
	mWidth = width;
	mLabelWidth = labelWidth;
	mWidthChanged = true;
	if (mAnchor != ofxDatGuiAnchor::NO_ANCHOR)
		positionGui();
}

void ofxDatGui::setTheme(ofxDatGuiTheme *t, bool applyImmediately) {
	if (applyImmediately) {
		for (auto item : items)
			item->setTheme(t);
	} else {
		// apply on next update call
		mTheme = t;
		mThemeChanged = true;
	}
	mRowSpacing = t->layout.vMargin;
	mGuiBackground = t->color.guiBackground;
	setWidth(t->layout.width, t->layout.labelWidth);
}

void ofxDatGui::setOpacity(float opacity) {
	mAlpha = opacity;
	mAlphaChanged = true;
}

void ofxDatGui::setPosition(int x, int y) {
	moveGui(Point2(x, y));
}

void ofxDatGui::setPosition(ofxDatGuiAnchor anchor) {
	mAnchor = anchor;
	if (mAnchor != ofxDatGuiAnchor::NO_ANCHOR)
		positionGui();
}

void ofxDatGui::setVisible(bool visible) { mVisible = visible; }

void ofxDatGui::setEnabled(bool enabled) { mEnabled = enabled; }

void ofxDatGui::setAutoDraw(bool autodraw, int priority) {
	mAutoDraw = autodraw;
	// ofRemoveListener removed - need Godot event system implementation
	// ofRemoveListener removed - need Godot event system implementation
	if (mAutoDraw) {
		mIndex = priority;
		// ofAddListener removed - need Godot event system implementation
		// ofAddListener removed - need Godot event system implementation
	}
}

bool ofxDatGui::getAutoDraw() { return mAutoDraw; }

bool ofxDatGui::getMouseDown() { return mMouseDown; }

void ofxDatGui::setLabelAlignment(ofxDatGuiAlignment align) {
	mAlignment = align;
	mAlignmentChanged = true;
}

int ofxDatGui::getWidth() { return mWidth; }

int ofxDatGui::getHeight() { return mHeight; }

Point2 ofxDatGui::getPosition() { return Point2(mPosition.x, mPosition.y); }

void ofxDatGui::setAssetPath(String path) { ofxDatGuiTheme::AssetPath = path; }

String ofxDatGui::getAssetPath() { return ofxDatGuiTheme::AssetPath; }

// add component methods

ofxDatGuiHeader *ofxDatGui::addHeader(String label, bool draggable) {
	if (mGuiHeader == nullptr) {
		mGuiHeader = new ofxDatGuiHeader(label, draggable);
		if (items.size() == 0) {
			items.push_back(mGuiHeader);
		} else {
			// always ensure header is at the top of the panel //
			items.insert(items.begin(), mGuiHeader);
		}
		layoutGui();
	}
	return mGuiHeader;
}

ofxDatGuiFooter *ofxDatGui::addFooter() {
	if (mGuiFooter == nullptr) {
		mGuiFooter = new ofxDatGuiFooter();
		items.push_back(mGuiFooter);
		mGuiFooter->onInternalEvent(this, &ofxDatGui::onInternalEventCallback);
		layoutGui();
	}
	return mGuiFooter;
}

ofxDatGuiLabel *ofxDatGui::addLabel(String label) {
	ofxDatGuiLabel *lbl = new ofxDatGuiLabel(label);
	attachItem(lbl);
	return lbl;
}

ofxDatGuiButton *ofxDatGui::addButton(String label) {
	ofxDatGuiButton *button = new ofxDatGuiButton(label);
	button->onButtonEvent(this, &ofxDatGui::onButtonEventCallback);
	attachItem(button);
	return button;
}

ofxDatGuiToggle *ofxDatGui::addToggle(String label, bool enabled) {
	ofxDatGuiToggle *button = new ofxDatGuiToggle(label, enabled);
	button->onToggleEvent(this, &ofxDatGui::onToggleEventCallback);
	attachItem(button);
	return button;
}

// TODO: Implement proper parameter binding for sliders
// The original ofParameter template system needs to be replaced with Godot's property system

ofxDatGuiSlider *ofxDatGui::addSlider(String label, float min, float max) {
	// default to halfway between min & max values //
	ofxDatGuiSlider *slider = addSlider(label, min, max, (max + min) / 2);
	return slider;
}

ofxDatGuiSlider *ofxDatGui::addSlider(String label, float min, float max, float val) {
	ofxDatGuiSlider *slider = new ofxDatGuiSlider(label, min, max, val);
	slider->onSliderEvent(this, &ofxDatGui::onSliderEventCallback);
	attachItem(slider);
	return slider;
}

ofxDatGuiTextInput *ofxDatGui::addTextInput(String label, String value) {
	ofxDatGuiTextInput *input = new ofxDatGuiTextInput(label, value);
	input->onTextInputEvent(this, &ofxDatGui::onTextInputEventCallback);
	attachItem(input);
	return input;
}

ofxDatGuiColorPicker *ofxDatGui::addColorPicker(String label, Color color) {
	ofxDatGuiColorPicker *picker = new ofxDatGuiColorPicker(label, color);
	picker->onColorPickerEvent(this, &ofxDatGui::onColorPickerEventCallback);
	attachItem(picker);
	return picker;
}

ofxDatGuiWaveMonitor *ofxDatGui::addWaveMonitor(String label, float frequency, float amplitude) {
	ofxDatGuiWaveMonitor *monitor = new ofxDatGuiWaveMonitor(label, frequency, amplitude);
	attachItem(monitor);
	return monitor;
}

ofxDatGuiValuePlotter *ofxDatGui::addValuePlotter(String label, float min, float max) {
	ofxDatGuiValuePlotter *plotter = new ofxDatGuiValuePlotter(label, min, max);
	attachItem(plotter);
	return plotter;
}

ofxDatGuiDropdown *ofxDatGui::addDropdown(String label, PoolStringArray options) {
	ofxDatGuiDropdown *dropdown = new ofxDatGuiDropdown(label, options);
	dropdown->onDropdownEvent(this, &ofxDatGui::onDropdownEventCallback);
	attachItem(dropdown);
	return dropdown;
}

ofxDatGuiFRM *ofxDatGui::addFRM(float refresh) {
	ofxDatGuiFRM *monitor = new ofxDatGuiFRM(refresh);
	attachItem(monitor);
	return monitor;
}

ofxDatGuiBreak *ofxDatGui::addBreak() {
	ofxDatGuiBreak *brk = new ofxDatGuiBreak();
	attachItem(brk);
	return brk;
}

ofxDatGui2dPad *ofxDatGui::add2dPad(String label) {
	ofxDatGui2dPad *pad = new ofxDatGui2dPad(label);
	pad->on2dPadEvent(this, &ofxDatGui::on2dPadEventCallback);
	attachItem(pad);
	return pad;
}

ofxDatGui2dPad *ofxDatGui::add2dPad(String label, Rect2 bounds) {
	ofxDatGui2dPad *pad = new ofxDatGui2dPad(label, bounds);
	pad->on2dPadEvent(this, &ofxDatGui::on2dPadEventCallback);
	attachItem(pad);
	return pad;
}

ofxDatGuiMatrix *ofxDatGui::addMatrix(String label, int numButtons, bool showLabels) {
	ofxDatGuiMatrix *matrix = new ofxDatGuiMatrix(label, numButtons, showLabels);
	matrix->onMatrixEvent(this, &ofxDatGui::onMatrixEventCallback);
	attachItem(matrix);
	return matrix;
}

ofxDatGuiFolder *ofxDatGui::addFolder(String label, Color color) {
	ofxDatGuiFolder *folder = new ofxDatGuiFolder(label, color);
	folder->onButtonEvent(this, &ofxDatGui::onButtonEventCallback);
	folder->onToggleEvent(this, &ofxDatGui::onToggleEventCallback);
	folder->onSliderEvent(this, &ofxDatGui::onSliderEventCallback);
	folder->on2dPadEvent(this, &ofxDatGui::on2dPadEventCallback);
	folder->onMatrixEvent(this, &ofxDatGui::onMatrixEventCallback);
	folder->onTextInputEvent(this, &ofxDatGui::onTextInputEventCallback);
	folder->onColorPickerEvent(this, &ofxDatGui::onColorPickerEventCallback);
	folder->onInternalEvent(this, &ofxDatGui::onInternalEventCallback);
	attachItem(folder);
	return folder;
}

ofxDatGuiFolder *ofxDatGui::addFolder(ofxDatGuiFolder *folder) {
	attachItem(folder);
	return folder;
}

void ofxDatGui::attachItem(ofxDatGuiComponent *item) {
	if (mGuiFooter != nullptr) {
		items.insert(items.end() - 1, item);
	} else {
		items.push_back(item);
	}
	item->onInternalEvent(this, &ofxDatGui::onInternalEventCallback);
	layoutGui();
}

// component retrieval methods

ofxDatGuiLabel *ofxDatGui::getLabel(String bl, String fl) {
	ofxDatGuiLabel *o = nullptr;
	if (fl != "") {
		ofxDatGuiFolder *f = static_cast<ofxDatGuiFolder *>(getComponent(ofxDatGuiType::FOLDER, fl));
		if (f)
			o = static_cast<ofxDatGuiLabel *>(f->getComponent(ofxDatGuiType::LABEL, bl));
	} else {
		o = static_cast<ofxDatGuiLabel *>(getComponent(ofxDatGuiType::LABEL, bl));
	}
	if (o == nullptr) {
		o = ofxDatGuiLabel::getInstance();
		ofxDatGuiLog::write(ofxDatGuiMsg::COMPONENT_NOT_FOUND, fl != "" ? fl + "-" + bl : bl);
		trash.push_back(o);
	}
	return o;
}

ofxDatGuiButton *ofxDatGui::getButton(String bl, String fl) {
	ofxDatGuiButton *o = nullptr;
	if (fl != "") {
		ofxDatGuiFolder *f = static_cast<ofxDatGuiFolder *>(getComponent(ofxDatGuiType::FOLDER, fl));
		if (f)
			o = static_cast<ofxDatGuiButton *>(f->getComponent(ofxDatGuiType::BUTTON, bl));
	} else {
		o = static_cast<ofxDatGuiButton *>(getComponent(ofxDatGuiType::BUTTON, bl));
	}
	if (o == nullptr) {
		o = ofxDatGuiButton::getInstance();
		ofxDatGuiLog::write(ofxDatGuiMsg::COMPONENT_NOT_FOUND, fl != "" ? fl + "-" + bl : bl);
		trash.push_back(o);
	}
	return o;
}

ofxDatGuiToggle *ofxDatGui::getToggle(String bl, String fl) {
	ofxDatGuiToggle *o = nullptr;
	if (fl != "") {
		ofxDatGuiFolder *f = static_cast<ofxDatGuiFolder *>(getComponent(ofxDatGuiType::FOLDER, fl));
		if (f)
			o = static_cast<ofxDatGuiToggle *>(f->getComponent(ofxDatGuiType::TOGGLE, bl));
	} else {
		o = static_cast<ofxDatGuiToggle *>(getComponent(ofxDatGuiType::TOGGLE, bl));
	}
	if (o == nullptr) {
		o = ofxDatGuiToggle::getInstance();
		ofxDatGuiLog::write(ofxDatGuiMsg::COMPONENT_NOT_FOUND, fl != "" ? fl + "-" + bl : bl);
		trash.push_back(o);
	}
	return o;
}

ofxDatGuiSlider *ofxDatGui::getSlider(String sl, String fl) {
	ofxDatGuiSlider *o = nullptr;
	if (fl != "") {
		ofxDatGuiFolder *f = static_cast<ofxDatGuiFolder *>(getComponent(ofxDatGuiType::FOLDER, fl));
		if (f)
			o = static_cast<ofxDatGuiSlider *>(f->getComponent(ofxDatGuiType::SLIDER, sl));
	} else {
		o = static_cast<ofxDatGuiSlider *>(getComponent(ofxDatGuiType::SLIDER, sl));
	}
	if (o == nullptr) {
		o = ofxDatGuiSlider::getInstance();
		ofxDatGuiLog::write(ofxDatGuiMsg::COMPONENT_NOT_FOUND, fl != "" ? fl + "-" + sl : sl);
		trash.push_back(o);
	}
	return o;
}

ofxDatGuiTextInput *ofxDatGui::getTextInput(String tl, String fl) {
	ofxDatGuiTextInput *o = nullptr;
	if (fl != "") {
		ofxDatGuiFolder *f = static_cast<ofxDatGuiFolder *>(getComponent(ofxDatGuiType::FOLDER, fl));
		if (f)
			o = static_cast<ofxDatGuiTextInput *>(f->getComponent(ofxDatGuiType::TEXT_INPUT, tl));
	} else {
		o = static_cast<ofxDatGuiTextInput *>(getComponent(ofxDatGuiType::TEXT_INPUT, tl));
	}
	if (o == nullptr) {
		o = ofxDatGuiTextInput::getInstance();
		ofxDatGuiLog::write(ofxDatGuiMsg::COMPONENT_NOT_FOUND, fl != "" ? fl + "-" + tl : tl);
		trash.push_back(o);
	}
	return o;
}

ofxDatGui2dPad *ofxDatGui::get2dPad(String pl, String fl) {
	ofxDatGui2dPad *o = nullptr;
	if (fl != "") {
		ofxDatGuiFolder *f = static_cast<ofxDatGuiFolder *>(getComponent(ofxDatGuiType::FOLDER, fl));
		if (f)
			o = static_cast<ofxDatGui2dPad *>(f->getComponent(ofxDatGuiType::PAD2D, pl));
	} else {
		o = static_cast<ofxDatGui2dPad *>(getComponent(ofxDatGuiType::PAD2D, pl));
	}
	if (o == nullptr) {
		o = ofxDatGui2dPad::getInstance();
		ofxDatGuiLog::write(ofxDatGuiMsg::COMPONENT_NOT_FOUND, fl != "" ? fl + "-" + pl : pl);
		trash.push_back(o);
	}
	return o;
}

ofxDatGuiColorPicker *ofxDatGui::getColorPicker(String cl, String fl) {
	ofxDatGuiColorPicker *o = nullptr;
	if (fl != "") {
		ofxDatGuiFolder *f = static_cast<ofxDatGuiFolder *>(getComponent(ofxDatGuiType::FOLDER, fl));
		if (f)
			o = static_cast<ofxDatGuiColorPicker *>(f->getComponent(ofxDatGuiType::COLOR_PICKER, cl));
	} else {
		o = static_cast<ofxDatGuiColorPicker *>(getComponent(ofxDatGuiType::COLOR_PICKER, cl));
	}
	if (o == nullptr) {
		o = ofxDatGuiColorPicker::getInstance();
		ofxDatGuiLog::write(ofxDatGuiMsg::COMPONENT_NOT_FOUND, fl != "" ? fl + "-" + cl : cl);
		trash.push_back(o);
	}
	return o;
}

ofxDatGuiWaveMonitor *ofxDatGui::getWaveMonitor(String cl, String fl) {
	ofxDatGuiWaveMonitor *o = nullptr;
	if (fl != "") {
		ofxDatGuiFolder *f = static_cast<ofxDatGuiFolder *>(getComponent(ofxDatGuiType::FOLDER, fl));
		if (f)
			o = static_cast<ofxDatGuiWaveMonitor *>(f->getComponent(ofxDatGuiType::WAVE_MONITOR, cl));
	} else {
		o = static_cast<ofxDatGuiWaveMonitor *>(getComponent(ofxDatGuiType::WAVE_MONITOR, cl));
	}
	if (o == nullptr) {
		o = ofxDatGuiWaveMonitor::getInstance();
		ofxDatGuiLog::write(ofxDatGuiMsg::COMPONENT_NOT_FOUND, fl != "" ? fl + "-" + cl : cl);
		trash.push_back(o);
	}
	return o;
}

ofxDatGuiValuePlotter *ofxDatGui::getValuePlotter(String cl, String fl) {
	ofxDatGuiValuePlotter *o = nullptr;
	if (fl != "") {
		ofxDatGuiFolder *f = static_cast<ofxDatGuiFolder *>(getComponent(ofxDatGuiType::FOLDER, fl));
		if (f)
			o = static_cast<ofxDatGuiValuePlotter *>(f->getComponent(ofxDatGuiType::VALUE_PLOTTER, cl));
	} else {
		o = static_cast<ofxDatGuiValuePlotter *>(getComponent(ofxDatGuiType::VALUE_PLOTTER, cl));
	}
	if (o == nullptr) {
		o = ofxDatGuiValuePlotter::getInstance();
		ofxDatGuiLog::write(ofxDatGuiMsg::COMPONENT_NOT_FOUND, fl != "" ? fl + "-" + cl : cl);
		trash.push_back(o);
	}
	return o;
}

ofxDatGuiMatrix *ofxDatGui::getMatrix(String ml, String fl) {
	ofxDatGuiMatrix *o = nullptr;
	if (fl != "") {
		ofxDatGuiFolder *f = static_cast<ofxDatGuiFolder *>(getComponent(ofxDatGuiType::FOLDER, fl));
		if (f)
			o = static_cast<ofxDatGuiMatrix *>(f->getComponent(ofxDatGuiType::MATRIX, ml));
	} else {
		o = static_cast<ofxDatGuiMatrix *>(getComponent(ofxDatGuiType::MATRIX, ml));
	}
	if (o == nullptr) {
		o = ofxDatGuiMatrix::getInstance();
		ofxDatGuiLog::write(ofxDatGuiMsg::COMPONENT_NOT_FOUND, fl != "" ? fl + "-" + ml : ml);
		trash.push_back(o);
	}
	return o;
}

ofxDatGuiDropdown *ofxDatGui::getDropdown(String dl) {
	ofxDatGuiDropdown *o = static_cast<ofxDatGuiDropdown *>(getComponent(ofxDatGuiType::DROPDOWN, dl));
	if (o == NULL) {
		o = ofxDatGuiDropdown::getInstance();
		ofxDatGuiLog::write(ofxDatGuiMsg::COMPONENT_NOT_FOUND, dl);
		trash.push_back(o);
	}
	return o;
}

ofxDatGuiFolder *ofxDatGui::getFolder(String fl) {
	ofxDatGuiFolder *o = static_cast<ofxDatGuiFolder *>(getComponent(ofxDatGuiType::FOLDER, fl));
	if (o == NULL) {
		o = ofxDatGuiFolder::getInstance();
		ofxDatGuiLog::write(ofxDatGuiMsg::COMPONENT_NOT_FOUND, fl);
		trash.push_back(o);
	}
	return o;
}

ofxDatGuiHeader *ofxDatGui::getHeader() {
	ofxDatGuiHeader *o;
	if (mGuiHeader != nullptr) {
		o = mGuiHeader;
	} else {
		o = new ofxDatGuiHeader("X");
		ofxDatGuiLog::write(ofxDatGuiMsg::COMPONENT_NOT_FOUND, "HEADER");
		trash.push_back(o);
	}
	return o;
}

ofxDatGuiFooter *ofxDatGui::getFooter() {
	ofxDatGuiFooter *o;
	if (mGuiFooter != nullptr) {
		o = mGuiFooter;
	} else {
		o = new ofxDatGuiFooter();
		ofxDatGuiLog::write(ofxDatGuiMsg::COMPONENT_NOT_FOUND, "FOOTER");
		trash.push_back(o);
	}
	return o;
}

ofxDatGuiComponent *ofxDatGui::getComponent(ofxDatGuiType type, String label) {
	for (int i = 0; i < items.size(); i++) {
		if (items[i]->getType() == type) {
			if (items[i]->is(label))
				return items[i];
		}
		// iterate over component's children and return the first match we find //
		for (int j = 0; j < items[i]->children.size(); j++) {
			if (items[i]->children[j]->is(label))
				return items[i]->children[j];
		}
	}
	return NULL;
}

// event callbacks

void ofxDatGui::onButtonEventCallback(ofxDatGuiButtonEvent e) {
	if (buttonEventCallback != nullptr) {
		buttonEventCallback(e);
	} else {
		ofxDatGuiLog::write(ofxDatGuiMsg::EVENT_HANDLER_NULL);
	}
}

void ofxDatGui::onToggleEventCallback(ofxDatGuiToggleEvent e) {
	if (toggleEventCallback != nullptr) {
		toggleEventCallback(e);
	} else if (buttonEventCallback != nullptr) { // allow toggle events to decay into button events
		// Cast toggle to button for the event callback - this is a workaround
		buttonEventCallback(ofxDatGuiButtonEvent(reinterpret_cast<ofxDatGuiButton *>(e.target)));
	} else {
		ofxDatGuiLog::write(ofxDatGuiMsg::EVENT_HANDLER_NULL);
	}
}

void ofxDatGui::onSliderEventCallback(ofxDatGuiSliderEvent e) {
	if (sliderEventCallback != nullptr) {
		sliderEventCallback(e);
	} else {
		ofxDatGuiLog::write(ofxDatGuiMsg::EVENT_HANDLER_NULL);
	}
}

void ofxDatGui::onTextInputEventCallback(ofxDatGuiTextInputEvent e) {
	if (textInputEventCallback != nullptr) {
		textInputEventCallback(e);
	} else {
		ofxDatGuiLog::write(ofxDatGuiMsg::EVENT_HANDLER_NULL);
	}
}

void ofxDatGui::onDropdownEventCallback(ofxDatGuiDropdownEvent e) {
	if (dropdownEventCallback != nullptr) {
		dropdownEventCallback(e);
	} else {
		ofxDatGuiLog::write(ofxDatGuiMsg::EVENT_HANDLER_NULL);
	}
	layoutGui(); // adjust the gui after a dropdown is closed
}

void ofxDatGui::on2dPadEventCallback(ofxDatGui2dPadEvent e) {
	if (pad2dEventCallback != nullptr) {
		pad2dEventCallback(e);
	} else {
		ofxDatGuiLog::write(ofxDatGuiMsg::EVENT_HANDLER_NULL);
	}
}

void ofxDatGui::onColorPickerEventCallback(ofxDatGuiColorPickerEvent e) {
	if (colorPickerEventCallback != nullptr) {
		colorPickerEventCallback(e);
	} else {
		ofxDatGuiLog::write(ofxDatGuiMsg::EVENT_HANDLER_NULL);
	}
}

void ofxDatGui::onMatrixEventCallback(ofxDatGuiMatrixEvent e) {
	if (matrixEventCallback != nullptr) {
		matrixEventCallback(e);
	} else {
		ofxDatGuiLog::write(ofxDatGuiMsg::EVENT_HANDLER_NULL);
	}
}

void ofxDatGui::onInternalEventCallback(ofxDatGuiInternalEvent e) {
	if (e.type == ofxDatGuiEventType::GROUP_TOGGLED) { // these events are not dispatched out to the main application
		layoutGui();
	} else if (e.type == ofxDatGuiEventType::GUI_TOGGLED) {
		mExpanded ? collapse() : expand();
	} else if (e.type == ofxDatGuiEventType::VISIBILITY_CHANGED) {
		layoutGui();
	}
}

// layout, position, anchor and check for focus

bool ofxDatGui::hitTest(Point2 pt) {
	if (mMoving) {
		return true;
	} else {
		return mGuiBounds.has_point(pt);
	}
}

void ofxDatGui::moveGui(Point2 pt) {
	mPosition.x = pt.x;
	mPosition.y = pt.y;
	mAnchor = ofxDatGuiAnchor::NO_ANCHOR;
	positionGui();
}

void ofxDatGui::layoutGui() {
	mHeight = 0;
	for (int i = 0; i < items.size(); i++) {
		items[i]->setIndex(i);
		if (items[i]->getVisible() == false) // skip over any components that are currently invisible
			continue;
		mHeight += items[i]->getHeight() + mRowSpacing;
	}
	positionGui();
}

void ofxDatGui::positionGui() {
	// TODO: Get actual viewport dimensions from Godot
	// For now using placeholder dimensions - will need proper Godot viewport integration
	int screen_width = 1920; // Placeholder - should get from Godot viewport
	int screen_height = 1080; // Placeholder - should get from Godot viewport

	if (mAnchor == ofxDatGuiAnchor::TOP_LEFT) {
		mPosition.y = 0;
		mPosition.x = 0;
	} else if (mAnchor == ofxDatGuiAnchor::TOP_RIGHT) {
		mPosition.y = 0;
		mPosition.x = screen_width - mWidth;
	} else if (mAnchor == ofxDatGuiAnchor::BOTTOM_LEFT) {
		mPosition.x = 0;
		mPosition.y = screen_height - mHeight;
	} else if (mAnchor == ofxDatGuiAnchor::BOTTOM_RIGHT) {
		mPosition.x = screen_width - mWidth;
		mPosition.y = screen_height - mHeight;
	}
	int h = 0;
	for (int i = 0; i < items.size(); i++) {
		if (items[i]->getVisible() == false) // skip over any components that are currently invisible
			continue;
		items[i]->setPosition(mPosition.x, mPosition.y + h);
		h += items[i]->getHeight() + mRowSpacing;
	}
	if (!mExpanded) // move the footer back to the top of the gui
		mGuiFooter->setPosition(mPosition.x, mPosition.y);
	mGuiBounds = Rect2(mPosition.x, mPosition.y, mWidth, mHeight);
}

// update & draw loop

void ofxDatGui::update() {
	if (!mVisible)
		return;

	for (int i = 0; i < items.size(); i++) { // check if we need to update components
		if (mAlphaChanged)
			items[i]->setOpacity(mAlpha);
		if (mThemeChanged)
			items[i]->setTheme(mTheme);
		if (mWidthChanged)
			items[i]->setWidth(mWidth, mLabelWidth);
		if (mAlignmentChanged)
			items[i]->setLabelAlignment(mAlignment);
	}

	if (mThemeChanged || mWidthChanged)
		layoutGui();

	mTheme = nullptr;
	mAlphaChanged = false;
	mWidthChanged = false;
	mThemeChanged = false;
	mAlignmentChanged = false;

	// check for gui focus change //
	// TODO: Replace with Godot input handling
	bool mouse_pressed = false; // Placeholder
	if (mouse_pressed && mActiveGui->mMoving == false) {
		Point2 mouse = Point2(0, 0); // Placeholder - should get from Godot input
		for (int i = mGuis.size() - 1; i > -1; i--) {
			// ignore guis that are invisible //
			if (mGuis[i]->getVisible() && mGuis[i]->hitTest(mouse)) {
				if (mGuis[i] != mActiveGui)
					mGuis[i]->focus();
				break;
			}
		}
	}

	if (!getFocused() || !mEnabled) {
		for (int i = 0; i < items.size(); i++) // update children but ignore mouse & keyboard events
			items[i]->update(false);
	} else {
		mMoving = false;
		mMouseDown = false;
		if (mExpanded == false) { // this gui has focus so let's see if any of its components were interacted with
			mGuiFooter->update();
			mMouseDown = mGuiFooter->getMouseDown();
		} else {
			bool hitComponent = false;
			for (int i = 0; i < items.size(); i++) {
				if (hitComponent == false) {
					items[i]->update(true);
					if (items[i]->getFocused()) {
						hitComponent = true;
						mMouseDown = items[i]->getMouseDown();
						if (mGuiHeader != nullptr && mGuiHeader->getDraggable() && mGuiHeader->getFocused()) {
							mMoving = true; // track that we're moving to force preserve focus
							Point2 mouse = Point2(0, 0); // Placeholder - should get from Godot input
							moveGui(mouse - mGuiHeader->getDragOffset());
						}
					} else if (items[i]->getIsExpanded()) {
						for (int j = 0; j < items[i]->children.size(); j++) { // check if one of its children has focus
							if (items[i]->children[j]->getFocused()) {
								hitComponent = true;
								mMouseDown = items[i]->children[j]->getMouseDown();
								break;
							}
						}
					}
				} else {
					items[i]->update(false); // update component but ignore mouse & keyboard events
					if (items[i]->getFocused())
						items[i]->setFocused(false);
				}
			}
		}
	}
	for (int i = 0; i < trash.size(); i++) // empty the trash
		delete trash[i];
	trash.clear();
}

void ofxDatGui::draw() {
	if (mVisible == false)
		return;
	// TODO: Replace with Godot drawing API
	// Drawing background rectangle
	if (mExpanded == false) {
		// Draw collapsed state
		mGuiFooter->draw();
	} else {
		// Draw expanded state
		for (int i = 0; i < items.size(); i++)
			items[i]->draw();
		for (int i = 0; i < items.size(); i++) // color pickers overlap other components when expanded so they must be drawn last
			items[i]->drawColorPicker();
	}
}

void ofxDatGui::onDraw() { draw(); } // TODO: Remove ofEventArgs

void ofxDatGui::onUpdate() { update(); } // TODO: Remove ofEventArgs

// TODO: Implement window resize handling with Godot notifications

/// END ofxDataGui

/// BEGIN ofxSmartFont

Vector<Ref<ofxSmartFont>> ofxSmartFont::mFonts;

// instance methods

void ofxSmartFont::draw(String s, int x, int y) { /* TODO: Implement drawing with Godot */
}

String ofxSmartFont::name() { return mName; }

void ofxSmartFont::name(String name) { mName = name; }

String ofxSmartFont::file() { return mFile; }

int ofxSmartFont::size() { return mSize; }

Rect2 ofxSmartFont::rect(String s, int x, int y) { return Rect2(); /* TODO: Implement with Godot font API */ }

float ofxSmartFont::width(String s, int x, int y) { return 0.0f; /* TODO: Implement with Godot font API */ }

float ofxSmartFont::height(String s, int x, int y) { return 0.0f; /* TODO: Implement with Godot font API */ }

float ofxSmartFont::getLineHeight() { return 0.0f; /* TODO: Implement with Godot font API */ }

// static methods

Ref<ofxSmartFont> ofxSmartFont::add(String file, int size, String name) {
	for (auto f : mFonts) {
		if (f->file() == file && f->size() == size) {
			// log(f->name() + "@ pt size "+std::to_string(f->size()) + " is already in memory.");
			return f;
		}
	}
	// TODO: Proper Ref<> construction for Godot
	Ref<ofxSmartFont> f;
	f.instance(); // Create new instance
	mFonts.push_back(f);
	return f;
}

Ref<ofxSmartFont> ofxSmartFont::get(String name) {
	for (auto f : mFonts) {
		if (f->name() == name)
			return f;
	}
	log("requested font : " + name + " not found");
	return Ref<ofxSmartFont>(); // Return null Ref
}

Ref<ofxSmartFont> ofxSmartFont::get(String name, int size) {
	for (auto f : mFonts) {
		if (f->file().find(name) != -1 && f->size() == size) {
			return f;
		}
	}
	log("requested font : " + name + " @ " + String::num(size) + " not found");
	return Ref<ofxSmartFont>(); // Return null Ref
}

Ref<ofxSmartFont> ofxSmartFont::get(PoolStringArray keys, int size) {
	for (auto f : mFonts) {
		bool match = true;
		for (int i = 0; i < keys.size(); i++) {
			// TODO: Replace ofToLower with proper string conversion
			if (f->file().find(keys[i]) == -1) // Simplified - no case conversion
				match = false;
		}
		if (match && f->size() == size)
			return f;
	}
	String keys_str = "";
	for (int i = 0; i < keys.size(); i++) {
		keys_str += keys[i];
		if (i < keys.size() - 1)
			keys_str += ",";
	}
	log("nothing found in search for : " + keys_str + " @ " + String::num(size) + "pt");
	return Ref<ofxSmartFont>(); // Return null Ref
}

void ofxSmartFont::list() {
	log("----------------------------------");
	log("total # of fonts cached = " + String::num(mFonts.size()));
	for (auto f : mFonts) {
		log(f->name() + " (" + String::num(f->size()) + "pt -> " + f->file() + ")");
	}
	log("----------------------------------");
}

void ofxSmartFont::log(String msg) { print_line("ofxSmartFont :: " + msg); }

/// END ofxSmartFont

/// BEGIN ofxDataComponent

bool ofxDatGuiLog::mQuiet = false;
// Duplicate AssetPath definition removed - already defined above
std::unique_ptr<ofxDatGuiTheme> ofxDatGuiComponent::theme;

ofxDatGuiComponent::ofxDatGuiComponent(String label) {
	mName = label;
	mVisible = true;
	mEnabled = true;
	mFocused = false;
	mMouseOver = false;
	mMouseDown = false;
	mStyle.opacity = 255;
	this->x = 0;
	this->y = 0;
	mAnchor = ofxDatGuiAnchor::NO_ANCHOR;
	mLabel.text = label;
	mLabel.alignment = ofxDatGuiAlignment::LEFT;
}

ofxDatGuiComponent::~ofxDatGuiComponent() {
	// ofRemoveListener removed - need Godot event system implementation
	// ofRemoveListener removed - need Godot event system implementation
}

// instance getters & setters

void ofxDatGuiComponent::setIndex(int index) { mIndex = index; }

int ofxDatGuiComponent::getIndex() { return mIndex; }

void ofxDatGuiComponent::setName(String name) { mName = name; }

String ofxDatGuiComponent::getName() { return mName; }

bool ofxDatGuiComponent::is(String name) {
	// Use Godot String for case-insensitive comparison
	return mName.nocasecmp_to(name) == 0;
}

ofxDatGuiType ofxDatGuiComponent::getType() { return mType; }

const ofxDatGuiTheme *ofxDatGuiComponent::getTheme() {
	if (theme == nullptr)
		theme = std::make_unique<ofxDatGuiTheme>(true);
	return theme.get();
}

void ofxDatGuiComponent::setComponentStyle(const ofxDatGuiTheme *theme) {
	mStyle.height = theme->layout.height;
	mStyle.padding = theme->layout.padding;
	mStyle.vMargin = theme->layout.vMargin;
	mStyle.color.background = theme->color.background;
	mStyle.color.inputArea = theme->color.inputAreaBackground;
	mStyle.color.onMouseOver = theme->color.backgroundOnMouseOver;
	mStyle.color.onMouseDown = theme->color.backgroundOnMouseDown;
	mStyle.stripe.width = theme->stripe.width;
	mStyle.stripe.visible = theme->stripe.visible;
	mStyle.border.width = theme->border.width;
	mStyle.border.color = theme->border.color;
	mStyle.border.visible = theme->border.visible;
	mStyle.guiBackground = theme->color.guiBackground;
	mFont = theme->font.ptr;
	mIcon.y = mStyle.height * .33;
	mIcon.color = theme->color.icons;
	mIcon.size = theme->layout.iconSize;
	mLabel.color = theme->color.label;
	mLabel.margin = theme->layout.labelMargin;
	mLabel.forceUpperCase = theme->layout.upperCaseLabels;
	setLabel(mLabel.text);
	setWidth(theme->layout.width, theme->layout.labelWidth);
	for (int i = 0; i < children.size(); i++)
		children[i]->setTheme(theme);
}

void ofxDatGuiComponent::setWidth(int width, float labelWidth) {
	mStyle.width = width;
	if (labelWidth > 1) {
		mLabel.width = labelWidth; // we received a pixel value
	} else {
		mLabel.width = mStyle.width * labelWidth; // we received a percentage
	}
	mIcon.x = mStyle.width - (mStyle.width * .05) - mIcon.size;
	mLabel.rightAlignedXpos = mLabel.width - mLabel.margin;
	for (int i = 0; i < children.size(); i++)
		children[i]->setWidth(width, labelWidth);
	positionLabel();
}

int ofxDatGuiComponent::getWidth() { return mStyle.width; }

int ofxDatGuiComponent::getHeight() { return mStyle.height; }

int ofxDatGuiComponent::getX() { return this->x; }

int ofxDatGuiComponent::getY() { return this->y; }

void ofxDatGuiComponent::setPosition(int x, int y) {
	this->x = x;
	this->y = y;
	for (int i = 0; i < children.size(); i++)
		children[i]->setPosition(x, this->y + (mStyle.height + mStyle.vMargin) * (i + 1));
}

void ofxDatGuiComponent::setVisible(bool visible) {
	mVisible = visible;
	if (internalEventCallback != nullptr) {
		ofxDatGuiInternalEvent e(ofxDatGuiEventType::VISIBILITY_CHANGED, mIndex);
		internalEventCallback(e);
	}
}

bool ofxDatGuiComponent::getVisible() { return mVisible; }

void ofxDatGuiComponent::setOpacity(float opacity) {
	mStyle.opacity = opacity * 255;
	for (int i = 0; i < children.size(); i++)
		children[i]->setOpacity(opacity);
}

void ofxDatGuiComponent::setEnabled(bool enabled) { mEnabled = enabled; }

bool ofxDatGuiComponent::getEnabled() {
	return mEnabled;
}

void ofxDatGuiComponent::setFocused(bool focused) {
	if (focused) {
		onFocus();
	} else {
		onFocusLost();
	}
}

bool ofxDatGuiComponent::getFocused() {
	return mFocused;
}

bool ofxDatGuiComponent::getMouseDown() { return mMouseDown; }

void ofxDatGuiComponent::setMask(const Rect2 &mask) { mMask = mask; }

void ofxDatGuiComponent::setAnchor(ofxDatGuiAnchor anchor) {
	mAnchor = anchor;
	if (mAnchor != ofxDatGuiAnchor::NO_ANCHOR) {
		// ofAddListener removed - need Godot event system implementation
	} else {
		// ofRemoveListener removed - need Godot event system implementation
	}
	onWindowResized();
}

bool ofxDatGuiComponent::getIsExpanded() { return false; }

// component label

void ofxDatGuiComponent::setLabel(String label) {
	mLabel.text = label;
	// Use Godot String for case conversion
	if (mLabel.forceUpperCase) {
		mLabel.rendered = mLabel.text.to_upper();
	} else {
		mLabel.rendered = mLabel.text;
	}
	mLabel.rect = mFont->rect(mLabel.rendered);
	positionLabel();
}

String ofxDatGuiComponent::getLabel() { return mLabel.text; }

void ofxDatGuiComponent::setLabelColor(Color c) { mLabel.color = c; }

Color ofxDatGuiComponent::getLabelColor() { return mLabel.color; }

void ofxDatGuiComponent::setLabelUpperCase(bool toUpper) {
	mLabel.forceUpperCase = toUpper;
	setLabel(mLabel.text);
}

bool ofxDatGuiComponent::getLabelUpperCase() {
	return mLabel.forceUpperCase;
}

void ofxDatGuiComponent::setLabelAlignment(ofxDatGuiAlignment align) {
	mLabel.alignment = align;
	for (int i = 0; i < children.size(); i++)
		children[i]->setLabelAlignment(align);
	positionLabel();
}

void ofxDatGuiComponent::positionLabel() {
	if (mLabel.alignment == ofxDatGuiAlignment::LEFT) {
		mLabel.x = mLabel.margin;
	} else if (mLabel.alignment == ofxDatGuiAlignment::CENTER) {
		mLabel.x = (mLabel.width / 2) - (mLabel.rect.size.width / 2);
	} else if (mLabel.alignment == ofxDatGuiAlignment::RIGHT) {
		mLabel.x = mLabel.rightAlignedXpos - mLabel.rect.size.width;
	}
}

// visual customization

void ofxDatGuiComponent::setBackgroundColor(Color color) { mStyle.color.background = color; }

void ofxDatGuiComponent::setBackgroundColorOnMouseOver(Color color) { mStyle.color.onMouseOver = color; }

void ofxDatGuiComponent::setBackgroundColorOnMouseDown(Color color) { mStyle.color.onMouseDown = color; }

void ofxDatGuiComponent::setBackgroundColors(Color c1, Color c2, Color c3) {
	mStyle.color.background = c1;
	mStyle.color.onMouseOver = c2;
	mStyle.color.onMouseDown = c3;
}

void ofxDatGuiComponent::setStripe(Color color, int width) {
	mStyle.stripe.color = color;
	mStyle.stripe.width = width;
}

void ofxDatGuiComponent::setStripeColor(Color color) {
	mStyle.stripe.color = color;
}

void ofxDatGuiComponent::setStripeWidth(int width) {
	mStyle.stripe.width = width;
}

void ofxDatGuiComponent::setStripeVisible(bool visible) {
	mStyle.stripe.visible = visible;
}

void ofxDatGuiComponent::setBorder(Color color, int width) {
	mStyle.border.color = color;
	mStyle.border.width = width;
	mStyle.border.visible = true;
}

void ofxDatGuiComponent::setBorderVisible(bool visible) {
	mStyle.border.visible = visible;
}

/*
	draw methods
*/

void ofxDatGuiComponent::update(bool acceptEvents) {
	if (acceptEvents && mEnabled && mVisible) {
		// TODO: Replace with Godot input handling
		bool mp = false; // Placeholder
		Point2 mouse = Point2(0 - mMask.position.x, 0 - mMask.position.y); // Placeholder
		if (hitTest(mouse)) {
			if (!mMouseOver) {
				onMouseEnter(mouse);
			}
			if (!mMouseDown && mp) {
				onMousePress(mouse);
				if (!mFocused)
					onFocus();
			}
		} else {
			// the mouse is not over the component //
			if (mMouseOver) {
				onMouseLeave(mouse);
			}
			if (!mMouseDown && mp && mFocused) {
				onFocusLost();
			}
		}
		if (mMouseDown) {
			if (mp) {
				onMouseDrag(mouse);
			} else {
				onMouseRelease(mouse);
			}
		}
	}
	// don't update children unless they're visible //
	if (this->getIsExpanded()) {
		for (int i = 0; i < children.size(); i++) {
			children[i]->update(acceptEvents);
			if (children[i]->getFocused()) {
				if (acceptEvents == false)
					children[i]->setFocused(false);
				acceptEvents = false;
			}
		}
	}
}

void ofxDatGuiComponent::draw() {
	// TODO: Replace with Godot drawing API
	if (mStyle.border.visible)
		drawBorder();
	drawBackground();
	drawLabel();
	if (mStyle.stripe.visible)
		drawStripe();
}

void ofxDatGuiComponent::drawBackground() {
	// Use Godot CanvasItem drawing API
	Color bg_color = mStyle.color.background;
	bg_color.a = mStyle.opacity / 255.0f; // Convert opacity to alpha
	draw_rect(Rect2(x, y, mStyle.width, mStyle.height), bg_color, true);
}

void ofxDatGuiComponent::drawLabel() {
	// TODO: Implement label drawing with Godot font API
	// Color label_color = mLabel.color;
	// Point2 pos = Point2(x + mLabel.x, y + mStyle.height / 2 + mLabel.rect.size.height / 2);
	// if (mType != ofxDatGuiType::DROPDOWN_OPTION) {
	//     draw_string(font, pos, String(mLabel.rendered.c_str()), label_color);
	// } else {
	//     draw_string(font, pos, String(("* " + mLabel.rendered).c_str()), label_color);
	// }
}

void ofxDatGuiComponent::drawStripe() {
	// Use Godot CanvasItem drawing API
	draw_rect(Rect2(x, y, mStyle.stripe.width, mStyle.height), mStyle.stripe.color, true);
}

void ofxDatGuiComponent::drawBorder() {
	// Use Godot CanvasItem drawing API
	int w = mStyle.border.width;
	Color border_color = mStyle.border.color;
	border_color.a = mStyle.opacity / 255.0f; // Convert opacity to alpha
	draw_rect(Rect2(x - w, y - w, mStyle.width + (w * 2), mStyle.height + (w * 2)), border_color, false, w);
}

void ofxDatGuiComponent::drawColorPicker() {}

/*
	events
*/

bool ofxDatGuiComponent::hitTest(Point2 m) {
	if (mMask.size.height > 0 && (m.y < 0 || m.y > mMask.size.height))
		return false;
	return (m.x >= x && m.x <= x + mStyle.width && m.y >= y && m.y <= y + mStyle.height);
}

void ofxDatGuiComponent::onMouseEnter(Point2 m) {
	mMouseOver = true;
}

void ofxDatGuiComponent::onMouseLeave(Point2 m) {
	mMouseOver = false;
}

void ofxDatGuiComponent::onMousePress(Point2 m) {
	mMouseDown = true;
}

void ofxDatGuiComponent::onMouseRelease(Point2 m) {
	mMouseDown = false;
}

void ofxDatGuiComponent::onFocus() {
	mFocused = true;
	// ofAddListener removed - need Godot event system implementation
}

void ofxDatGuiComponent::onFocusLost() {
	mFocused = false;
	mMouseDown = false;
	// ofRemoveListener removed - need Godot event system implementation
}

void ofxDatGuiComponent::onKeyPressed(int key) {}
void ofxDatGuiComponent::onMouseDrag(Point2 m) {}

void ofxDatGuiComponent::bind_godot_property(Object *p_object, const String &p_property) {
	ERR_FAIL_COND(!p_object);
	mPropertyBinding.object_id = p_object->get_instance_id();
	mPropertyBinding.property = p_property;
	mPropertyBinding.active = true;
}

void ofxDatGuiComponent::unbind_godot_property() {
	mPropertyBinding.active = false;
	mPropertyBinding.object_id = 0;
	mPropertyBinding.property = "";
}

void ofxDatGuiComponent::_message(ofxDatGuiEvent event) {
	switch (event) {
		case DATGUI_RESIZE:
			onWindowResized();
			break;
		case DATGUI_KEY_PRESSED:
			// Call existing key handler with placeholder key
			onKeyPressed(0);
			break;
		case DATGUI_MOUSE_PRESSED:
			// TODO: Implement mouse pressed handler
			break;
		case DATGUI_MOUSE_RELEASED:
			// TODO: Implement mouse released handler
			break;
		case DATGUI_MOUSE_DRAGGED:
			// TODO: Implement mouse dragged handler
			break;
		case DATGUI_FOCUS_LOST:
			onFocusLost();
			break;
		case DATGUI_FOCUS_GAINED:
			// TODO: Implement focus gained handler
			break;
		case DATGUI_UPDATE:
			update();
			break;
		case DATGUI_DRAW:
			draw();
			break;
	}
}

void ofxDatGuiComponent::onWindowResized() {
	if (mAnchor == ofxDatGuiAnchor::TOP_LEFT) {
		setPosition(0, 0);
	} else if (mAnchor == ofxDatGuiAnchor::TOP_RIGHT) {
		// TODO: Replace with proper viewport dimensions
		setPosition(1920 - mStyle.width, 0); // Placeholder
	}
}

/// END ofxDataComponent

// =========================================================================
// Tests
// =========================================================================

#ifdef DOCTEST
#include "doctest/doctest.h"

TEST_SUITE("[[ofxdatgui]] Embedded resources") {
	TEST_CASE("Embedded PNG icons load successfully") {
		Ref<Image> img;

		img = _load_embedded_png(ofxdg_radio_on_data, ofxdg_radio_on_size);
		CHECK(img.is_valid());
		CHECK(img->get_width() > 0);
		CHECK(img->get_height() > 0);

		img = _load_embedded_png(ofxdg_radio_off_data, ofxdg_radio_off_size);
		CHECK(img.is_valid());
		CHECK(img->get_width() > 0);

		img = _load_embedded_png(ofxdg_group_open_data, ofxdg_group_open_size);
		CHECK(img.is_valid());
		CHECK(img->get_width() > 0);

		img = _load_embedded_png(ofxdg_group_closed_data, ofxdg_group_closed_size);
		CHECK(img.is_valid());
		CHECK(img->get_width() > 0);

		img = _load_embedded_png(ofxdg_rainbow_data, ofxdg_rainbow_size);
		CHECK(img.is_valid());
		CHECK(img->get_width() > 0);
	}

	TEST_CASE("Embedded font creates valid DynamicFont") {
		Ref<DynamicFont> font = _create_embedded_font(14);
		CHECK(font.is_valid());
		CHECK(font->get_font_data().is_valid());
		CHECK(font->get_size() == 14);
	}

	TEST_CASE("Embedded font at different sizes") {
		Ref<DynamicFont> small = _create_embedded_font(8);
		Ref<DynamicFont> large = _create_embedded_font(32);
		CHECK(small.is_valid());
		CHECK(large.is_valid());
		CHECK(small->get_size() == 8);
		CHECK(large->get_size() == 32);
	}
}

TEST_SUITE("[[ofxdatgui]] Theme") {
	TEST_CASE("Default theme has valid colors") {
		ofxDatGuiTheme theme;
		// Colors should be non-zero (not black-transparent)
		CHECK(theme.color.label.r > 0);
		CHECK(theme.color.background.a > 0);
		CHECK(theme.color.guiBackground.a > 0);
	}

	TEST_CASE("Default theme layout values are positive") {
		ofxDatGuiTheme theme;
		CHECK(theme.layout.width > 0);
		CHECK(theme.layout.height > 0);
		CHECK(theme.layout.padding > 0);
	}

	TEST_CASE("Theme init loads resources") {
		ofxDatGuiTheme theme(true); // autoInitialize=true
		CHECK(theme.icon.radioOn.is_valid());
		CHECK(theme.icon.radioOff.is_valid());
		CHECK(theme.icon.groupOpen.is_valid());
		CHECK(theme.icon.groupClosed.is_valid());
		CHECK(theme.icon.rainbow.is_valid());
		CHECK(theme.font.ptr.is_valid());
	}

	TEST_CASE("Theme scale modifies layout") {
		ofxDatGuiTheme theme;
		int orig_width = theme.layout.width;
		int orig_height = theme.layout.height;
		theme.scale(2.0f);
		CHECK(theme.layout.width == orig_width * 2);
		CHECK(theme.layout.height == orig_height * 2);
	}

	TEST_CASE("Smoke theme is default") {
		ofxDatGuiTheme theme;
		// Smoke uses dark background colors
		CHECK(theme.color.guiBackground.r < 0.3f);
	}

	TEST_CASE("Aqua theme has different colors") {
		ofxDatGuiThemeAqua theme;
		// Aqua has teal/cyan accents
		CHECK(theme.color.slider.fill != ofxDatGuiTheme().color.slider.fill);
	}
}

TEST_SUITE("[[ofxdatgui]] SmartFont") {
	TEST_CASE("Default constructor") {
		ofxSmartFont font;
		CHECK(font.size() == 12);
		CHECK(font.file().empty());
	}

	TEST_CASE("set_embedded_font works") {
		ofxSmartFont font;
		Ref<DynamicFont> df = _create_embedded_font(16);
		font.set_embedded_font(df, 16);
		CHECK(font.size() == 16);
		CHECK(font.name() == "embedded");
	}

	TEST_CASE("Static font list starts empty") {
		// Clear any residual state
		ofxSmartFont::mFonts.clear();
		CHECK(ofxSmartFont::mFonts.size() == 0);
	}
}

TEST_SUITE("[[ofxdatgui]] Component base") {
	TEST_CASE("Component has default type") {
		ofxDatGuiButton btn("Test");
		CHECK(btn.getType() == ofxDatGuiType::BUTTON);
	}

	TEST_CASE("Component label") {
		ofxDatGuiButton btn("MyButton");
		CHECK(btn.getLabel() == "MyButton");
	}

	TEST_CASE("Component visibility") {
		ofxDatGuiButton btn("Test");
		CHECK(btn.getVisible());
		btn.setVisible(false);
		CHECK_FALSE(btn.getVisible());
	}

	TEST_CASE("Component enabled state") {
		ofxDatGuiButton btn("Test");
		CHECK(btn.getEnabled());
		btn.setEnabled(false);
		CHECK_FALSE(btn.getEnabled());
	}

	TEST_CASE("Component position") {
		ofxDatGuiButton btn("Test");
		btn.setPosition(100, 200);
		CHECK(btn.getX() == 100);
		CHECK(btn.getY() == 200);
	}
}

TEST_SUITE("[[ofxdatgui]] Toggle component") {
	TEST_CASE("Toggle construction") {
		ofxDatGuiToggle toggle("Switch", false);
		CHECK(toggle.getType() == ofxDatGuiType::TOGGLE);
		CHECK(toggle.getLabel() == "Switch");
	}
}

TEST_SUITE("[[ofxdatgui]] Slider component") {
	TEST_CASE("Slider construction") {
		ofxDatGuiSlider slider("Value", 0, 100, 50);
		CHECK(slider.getType() == ofxDatGuiType::SLIDER);
		CHECK(slider.getLabel() == "Value");
	}
}

TEST_SUITE("[[ofxdatgui]] Label component") {
	TEST_CASE("Label text") {
		ofxDatGuiLabel label("Hello World");
		CHECK(label.getLabel() == "Hello World");
	}

	TEST_CASE("Label set text") {
		ofxDatGuiLabel label("Old");
		label.setLabel("New");
		CHECK(label.getLabel() == "New");
	}
}

TEST_SUITE("[[ofxdatgui]] Godot property binding") {
	TEST_CASE("[ofxdatgui] bind_godot_property sets active") {
		ofxDatGuiButton btn("Test");
		CHECK_FALSE(btn.has_godot_property_binding());

		Node2D node;
		btn.bind_godot_property(&node, "position");
		CHECK(btn.has_godot_property_binding());
		CHECK(btn.mPropertyBinding.property == "position");
	}

	TEST_CASE("[ofxdatgui] unbind_godot_property clears binding") {
		ofxDatGuiButton btn("Test");
		Node2D node;
		btn.bind_godot_property(&node, "visible");
		CHECK(btn.has_godot_property_binding());

		btn.unbind_godot_property();
		CHECK_FALSE(btn.has_godot_property_binding());
		CHECK(btn.mPropertyBinding.property.empty());
	}

	TEST_CASE("[ofxdatgui] binding stores correct object id") {
		ofxDatGuiButton btn("Test");
		Node2D node;
		btn.bind_godot_property(&node, "modulate");
		CHECK(btn.mPropertyBinding.object_id == node.get_instance_id());
	}

	TEST_CASE("[ofxdatgui] binding stores property name") {
		ofxDatGuiButton btn("Test");
		Node2D node;
		btn.bind_godot_property(&node, "rotation");
		CHECK(btn.mPropertyBinding.property == "rotation");
	}

	TEST_CASE("[ofxdatgui] default binding is inactive") {
		ofxDatGuiLabel label("Info");
		CHECK_FALSE(label.has_godot_property_binding());
		CHECK(label.mPropertyBinding.object_id == 0);
	}

	TEST_CASE("[ofxdatgui] rebind replaces previous binding") {
		ofxDatGuiButton btn("Test");
		Node2D node1;
		Node2D node2;
		btn.bind_godot_property(&node1, "position");
		CHECK(btn.mPropertyBinding.object_id == node1.get_instance_id());

		btn.bind_godot_property(&node2, "scale");
		CHECK(btn.mPropertyBinding.object_id == node2.get_instance_id());
		CHECK(btn.mPropertyBinding.property == "scale");
	}
}

#endif // DOCTEST
