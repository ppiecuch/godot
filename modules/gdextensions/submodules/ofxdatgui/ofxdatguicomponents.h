/// Copyright (C) 2015 Stephen Braitsch [http://braitsch.io]

#ifndef OFX_DATGUICOMPONENTS_H
#define OFX_DATGUICOMPONENTS_H

#include "ofxdatgui.h"

#include "core/math/vector2.h"

/// ofxDatGuiButton

class ofxDatGuiButton : public ofxDatGuiComponent {
protected:
	void onMouseRelease(Point2 m) {
		ofxDatGuiComponent::onFocusLost();
		ofxDatGuiComponent::onMouseRelease(m);
		dispatchEvent();
	}

public:
	void setTheme(const ofxDatGuiTheme *theme) {
		setComponentStyle(theme);
		mStyle.stripe.color = theme->stripe.button;
		setWidth(theme->layout.width, theme->layout.labelWidth);
	}

	void setWidth(int width, float labelWidth = 1) {
		ofxDatGuiComponent::setWidth(width, labelWidth);
		mLabel.width = mStyle.width;
		mLabel.rightAlignedXpos = mLabel.width - mLabel.margin;
		ofxDatGuiComponent::positionLabel();
	}

	void draw() {
		if (mVisible) {
			// anything that extends ofxDatGuiButton has the same rollover effect
			ofPushStyle();
			if (mStyle.border.visible)
				drawBorder();
			ofFill();
			if (mFocused && mMouseDown) {
				ofSetColor(mStyle.color.onMouseDown, mStyle.opacity);
			} else if (mMouseOver) {
				ofSetColor(mStyle.color.onMouseOver, mStyle.opacity);
			} else {
				ofSetColor(mStyle.color.background, mStyle.opacity);
			}
			ofDrawRectangle(x, y, mStyle.width, mStyle.height);
			drawLabel();
			if (mStyle.stripe.visible)
				drawStripe();
			ofPopStyle();
		}
	}

	void dispatchEvent() {
		if (buttonEventCallback != nullptr) {
			ofxDatGuiButtonEvent e(this);
			buttonEventCallback(e);
		} else {
			ofxDatGuiLog::write(ofxDatGuiMsg::EVENT_HANDLER_NULL);
		}
	}

	static ofxDatGuiButton *getInstance() { return new ofxDatGuiButton("X"); }

	ofxDatGuiButton(std::string label) :
			ofxDatGuiComponent(label) {
		mType = ofxDatGuiType::BUTTON;
		setTheme(ofxDatGuiComponent::getTheme());
	}
};

class ofxDatGuiToggle : public ofxDatGuiButton {
	bool mChecked;
	std::shared_ptr<ofImage> radioOn;
	std::shared_ptr<ofImage> radioOff;

protected:
	void onMouseRelease(Point2 m) {
		mChecked = !mChecked;
		ofxDatGuiComponent::onFocusLost();
		ofxDatGuiComponent::onMouseRelease(m);
		dispatchEvent();
	}

public:
	ofxDatGuiToggle(string label, bool checked = false) :
			ofxDatGuiButton(label) {
		mChecked = checked;
		mType = ofxDatGuiType::TOGGLE;
		setTheme(ofxDatGuiComponent::getTheme());
	}

	void setTheme(const ofxDatGuiTheme *theme) {
		setComponentStyle(theme);
		radioOn = theme->icon.radioOn;
		radioOff = theme->icon.radioOff;
		mStyle.stripe.color = theme->stripe.toggle;
		setWidth(theme->layout.width, theme->layout.labelWidth);
	}

	void setWidth(int width, float labelWidth = 1) {
		ofxDatGuiComponent::setWidth(width, labelWidth);
		mLabel.width = mStyle.width;
		mLabel.rightAlignedXpos = mIcon.x - mLabel.margin;
		ofxDatGuiComponent::positionLabel();
	}

	void toggle() { mChecked = !mChecked; }
	void setChecked(bool check) { mChecked = check; }
	bool getChecked() { return mChecked; }

	void draw() {
		if (mVisible) {
			ofPushStyle();
			ofxDatGuiButton::draw();
			ofSetColor(mIcon.color);
			if (mChecked == true) {
				radioOn->draw(x + mIcon.x, y + mIcon.y, mIcon.size, mIcon.size);
			} else {
				radioOff->draw(x + mIcon.x, y + mIcon.y, mIcon.size, mIcon.size);
			}
			ofPopStyle();
		}
	}

	void dispatchEvent() {
		if (toggleEventCallback == nullptr) {
			ofxDatGuiButton::dispatchEvent(); // attempt to call generic button callback
		} else {
			toggleEventCallback(ofxDatGuiToggleEvent(this, mChecked));
		}
	}

	static ofxDatGuiToggle *getInstance() { return new ofxDatGuiToggle("X"); }
};

/// ofxDatGuiLabel

class ofxDatGuiBreak : public ofxDatGuiComponent {
public:
	void setTheme(const ofxDatGuiTheme *theme) {
		setComponentStyle(theme);
		mStyle.height = theme->layout.breakHeight;
	}

	void setHeight(float height) { mStyle.height = height; }

	void draw() {
		if (!mVisible)
			return;
		ofPushStyle();
		ofFill();
		ofSetColor(mStyle.color.background, mStyle.opacity);
		ofDrawRectangle(x, y, mStyle.width, mStyle.height);
		ofPopStyle();
	}

	int getHeight() { return mStyle.height; }

	ofxDatGuiBreak() :
			ofxDatGuiComponent("break") {
		setTheme(ofxDatGuiComponent::getTheme());
	}
};

class ofxDatGuiLabel : public ofxDatGuiComponent {
public:
	void setTheme(const ofxDatGuiTheme *theme) {
		setComponentStyle(theme);
		mStyle.stripe.color = theme->stripe.label;
	}

	void setWidth(int width, float labelWidth = 1) {
		ofxDatGuiComponent::setWidth(width, labelWidth);
		mLabel.width = mStyle.width;
		mLabel.rightAlignedXpos = mLabel.width - mLabel.margin;
		ofxDatGuiComponent::positionLabel();
	}

	void draw() {
		ofxDatGuiComponent::draw();
	}

	static ofxDatGuiLabel *getInstance() { return new ofxDatGuiLabel("X"); }

	ofxDatGuiLabel(string label) :
			ofxDatGuiComponent(label) {
		mType = ofxDatGuiType::LABEL;
		setTheme(ofxDatGuiComponent::getTheme());
	}
};

/// ofxDatGuiTextInputField

class ofxDatGuiTextInputField : public ofxDatGuiInteractiveObject {
	std::string mText;
	std::string mRendered;
	bool mFocused;
	bool mTextChanged;
	bool mHighlightText;
	bool mUpperCaseText;
	float mCursorX;
	Rect2 mTextRect;
	Rect2 mInputRect;
	unsigned int mCursorIndex;
	unsigned int mMaxCharacters;
	unsigned int mHighlightPadding;
	struct {
		struct {
			Color text;
			Color background;
		} active;
		struct {
			Color text;
			Color background;
		} inactive;
		Color cursor;
		Color highlight;
	} color;
	ofxDatGuiInputType mType;
	std::shared_ptr<ofxSmartFont> mFont;

public:
	ofxDatGuiTextInputField() {
		mFocused = false;
		mTextChanged = false;
		mHighlightText = false;
		mMaxCharacters = 99;
		mType = ofxDatGuiInputType::ALPHA_NUMERIC;
		setTheme(ofxDatGuiComponent::getTheme());
	}

	void setWidth(int w) { mInputRect.width = w; }

	void setPosition(int x, int y) {
		mInputRect.x = x;
		mInputRect.y = y;
	}

	void setTheme(const ofxDatGuiTheme *theme) {
		mFont = theme->font.ptr;
		mInputRect.height = theme->layout.height - (theme->layout.padding * 2);
		color.active.background = theme->color.textInput.backgroundOnActive;
		color.inactive.background = theme->color.inputAreaBackground;
		color.active.text = theme->color.label;
		color.inactive.text = theme->color.textInput.text;
		color.highlight = theme->color.textInput.highlight;
		mUpperCaseText = theme->layout.textInput.forceUpperCase;
		mHighlightPadding = theme->layout.textInput.highlightPadding;
		setText(mText);
	}

	void draw() {
		// center the text
		int tx = mInputRect.x + mInputRect.width / 2 - mTextRect.width / 2;
		int ty = mInputRect.y + mInputRect.height / 2 + mTextRect.height / 2;
		ofPushStyle();
		// draw the input field background
		if (mFocused && mType != ofxDatGuiInputType::COLORPICKER) {
			ofSetColor(color.active.background);
		} else {
			ofSetColor(color.inactive.background);
		}
		ofDrawRectangle(mInputRect);
		// draw the highlight rectangle
		if (mHighlightText) {
			ofRectangle hRect;
			hRect.x = tx - mHighlightPadding;
			hRect.width = mTextRect.width + (mHighlightPadding * 2);
			hRect.y = ty - mHighlightPadding - mTextRect.height;
			hRect.height = mTextRect.height + (mHighlightPadding * 2);
			ofSetColor(color.highlight);
			ofDrawRectangle(hRect);
		}
		Color tColor = mHighlightText ? color.active.text : color.inactive.text; // draw the text
		ofSetColor(tColor);
		mFont->draw(mType == ofxDatGuiInputType::COLORPICKER ? "#" + mRendered : mRendered, tx, ty);
		if (mFocused) {
			ofDrawLine(Point2(tx + mCursorX, mInputRect.getTop()), Point2(tx + mCursorX, mInputRect.getBottom())); // draw the cursor
		}
		ofPopStyle();
	}

	int getWidth() { return mInputRect.width; }
	int getHeight() { return mInputRect.height; }

	bool hasFocus() { return mFocused; }

	bool hitTest(Point2 m) { return (m.x >= mInputRect.x && m.x <= mInputRect.x + mInputRect.width && m.y >= mInputRect.y && m.y <= mInputRect.y + mInputRect.height); }

	void setText(std::string text) {
		mText = text;
		mTextChanged = true;
		mRendered = mUpperCaseText ? ofToUpper(mText) : mText;
		mTextRect = mFont->rect(mType == ofxDatGuiInputType::COLORPICKER ? "#" + mRendered : mRendered);
	}

	std::string getText() { return mText; }

	void setTextActiveColor(ofColor c) { color.active.text = c; }
	void setTextInactiveColor(ofColor c) { color.inactive.text = c; }

	void setTextUpperCase(bool toUpper) {
		mUpperCaseText = toUpper;
		setText(mText);
	}

	bool getTextUpperCase() { return mUpperCaseText; }
	void setTextInputFieldType(ofxDatGuiInputType type) { mType = type; }
	void setBackgroundColor(ofColor c) { color.inactive.background = c; }
	void setMaxNumOfCharacters(unsigned int max) { mMaxCharacters = max; }

	void onFocus() {
		mFocused = true;
		mTextChanged = false;
		mHighlightText = true;
		setCursorIndex(mText.size());
	}

	void onFocusLost() {
		mFocused = false;
		mHighlightText = false;
		if (mTextChanged) {
			mTextChanged = false;
			ofxDatGuiInternalEvent e(ofxDatGuiEventType::INPUT_CHANGED, 0);
			internalEventCallback(e);
		}
	}

	void onKeyPressed(int key) {
		if (!keyIsValid(key))
			return;
		if (mHighlightText) {
			if ((key >= 32 && key <= 255) || key == OF_KEY_BACKSPACE || key == OF_KEY_DEL) { // if key is printable or delete
				setText("");
				setCursorIndex(0);
			}
		}
		if (key == OF_KEY_BACKSPACE) {
			if (mCursorIndex > 0) { // delete character at cursor position
				setText(mText.substr(0, mCursorIndex - 1) + mText.substr(mCursorIndex));
				setCursorIndex(mCursorIndex - 1);
			}
		} else if (key == OF_KEY_LEFT) {
			setCursorIndex(max((int)mCursorIndex - 1, 0));
		} else if (key == OF_KEY_RIGHT) {
			setCursorIndex(min(mCursorIndex + 1, (unsigned int)mText.size()));
		} else {
			setText(mText.substr(0, mCursorIndex) + (char)key + mText.substr(mCursorIndex)); // insert character at cursor position
			setCursorIndex(mCursorIndex + 1);
		}
		mHighlightText = false;
	}

	void setCursorIndex(int index) {
		if (index == 0) {
			mCursorX = mFont->rect(mRendered.substr(0, index)).getLeft();
		} else if (index > 0) {
			mCursorX = mFont->rect(mRendered.substr(0, index)).getRight();
			if (mText.at(index - 1) == ' ') // if we're at a space append the width the font's '1' character
				mCursorX += mFont->rect("1").width;
		}
		if (mType == ofxDatGuiInputType::COLORPICKER)
			mCursorX += mFont->rect("#").width;
		mCursorIndex = index;
	}

protected:
	bool keyIsValid(int key) {
		if (key == OF_KEY_BACKSPACE || key == OF_KEY_LEFT || key == OF_KEY_RIGHT) {
			return true;
		} else if (mType == ofxDatGuiInputType::COLORPICKER) {
			if (!mHighlightText && mText.size() == 6) { // limit string length to six hex characters
				return false;
			} else if (key >= 48 && key <= 57) { // allow numbers 0-9
				return true;
			} else if ((key >= 97 && key <= 102) || (key >= 65 && key <= 70)) { // allow letters a-f & A-F
				return true;
			} else {
				return false; // an invalid key was entered
			}
		} else if (mType == ofxDatGuiInputType::NUMERIC) {
			if (key == 45 || key == 46) { // allow dash (-) or dot (.)
				return true;
			} else if (key >= 48 && key <= 57) { // allow numbers 0-9
				return true;
			} else {
				return false; // an invalid key was entered
			}
		} else if (mType == ofxDatGuiInputType::ALPHA_NUMERIC) {
			if (key >= 32 && key <= 255) { // limit range to printable characters http://www.ascii-code.com
				return true;
			} else {
				return false; // an invalid key was entered
			}
		} else {
			return false; // invalid textfield type
		}
	}
};

/// ofxDatGuiTextInput

class ofxDatGuiTextInput : public ofxDatGuiComponent {
protected:
	void onFocus() {
		mInput.onFocus();
		ofxDatGuiComponent::onFocus();
	}

	void onFocusLost() {
		mInput.onFocusLost();
		ofxDatGuiComponent::onFocusLost();
	}

	void onKeyPressed(int key) {
		if (key != OF_KEY_UP && key != OF_KEY_DOWN)
			mInput.onKeyPressed(key);
	}

	virtual void onInputChanged(ofxDatGuiInternalEvent e) { dispatchEvent(); } //  dispatch event out to main application

	ofxDatGuiTextInputField mInput;

public:
	ofxDatGuiTextInput(string label, string text = "") :
			ofxDatGuiComponent(label) {
		mInput.setText(text);
		mInput.onInternalEvent(this, &ofxDatGuiTextInput::onInputChanged);
		mType = ofxDatGuiType::TEXT_INPUT;
		setTheme(ofxDatGuiComponent::getTheme());
	}

	void setTheme(const ofxDatGuiTheme *theme) {
		setComponentStyle(theme);
		mStyle.stripe.color = theme->stripe.textInput;
		mInput.setTheme(theme);
		setWidth(theme->layout.width, theme->layout.labelWidth);
	}

	void setWidth(int width, float labelWidth) {
		ofxDatGuiComponent::setWidth(width, labelWidth);
		mInput.setPosition(x + mLabel.width, y + mStyle.padding);
		mInput.setWidth(mStyle.width - mStyle.padding - mLabel.width);
	}

	void setPosition(int x, int y) {
		ofxDatGuiComponent::setPosition(x, y);
		mInput.setPosition(x + mLabel.width, y + mStyle.padding);
	}

	void setText(string text) { mInput.setText(text); }

	std::string getText() { return mInput.getText(); }

	void setTextUpperCase(bool toUpper) { mInput.setTextUpperCase(toUpper); }

	bool getTextUpperCase() {
		return mInput.getTextUpperCase();
	}

	void setInputType(ofxDatGuiInputType type) { mInput.setTextInputFieldType(type); }

	void draw() {
		if (mVisible) {
			ofxDatGuiComponent::draw();
			mInput.draw();
		}
	}

	bool hitTest(Point2 m) {
		return mInput.hitTest(m);
	}

	void dispatchEvent() {
		if (textInputEventCallback != nullptr) {
			ofxDatGuiTextInputEvent e(this, mInput.getText());
			textInputEventCallback(e);
		} else {
			ofxDatGuiLog::write(ofxDatGuiMsg::EVENT_HANDLER_NULL);
		}
	}

	static ofxDatGuiTextInput *getInstance() { return new ofxDatGuiTextInput("X"); }
};

/// ofxDatGuiSlider

class ofxDatGuiSlider : public ofxDatGuiComponent {
	float mMin;
	float mMax;
	float mValue;
	float mScale;
	int mPrecision;
	int mInputX;
	int mInputWidth;
	int mSliderWidth;
	Color mSliderFill;
	Color mBackgroundFill;
	ofxDatGuiTextInputField *mInput;

	static const int MAX_PRECISION = 4;

	int *mBoundi = nullptr;
	float *mBoundf = nullptr;
	void onParamI(int &n) { setValue(n); }
	void onParamF(float &n) { setValue(n); }

	void calculateScale() {
		mScale = ofxDatGuiScale(mValue, mMin, mMax);
		setTextInput();
	}

	void setTextInput() {
		string v = ofToString(Math::round(mValue, mPrecision));
		if (mValue != mMin && mValue != mMax) {
			int p = v.find('.');
			if (p == -1 && mPrecision != 0) {
				v += '.';
				p = v.find('.');
			}
			while (v.length() - p < (mPrecision + 1))
				v += '0';
		}
		mInput->setText(v);
	}

	float round(float num, int precision) { return Math::round(num * Math::pow(10, precision)) / Math::pow(10, precision); }

	void onInvalidMinMaxValues() {
		ofLogError() << "row #" << mIndex << " : invalid min & max values"
					 << " [setting to 50%]";
		mMin = 0;
		mMax = 100;
		mScale = 0.5;
		mValue = (mMax + mMin) * mScale;
	}

protected:
	void onMousePress(Point2 m) {
		ofxDatGuiComponent::onMousePress(m);
		if (mInput->hitTest(m)) {
			mInput->onFocus();
		} else if (mInput->hasFocus()) {
			mInput->onFocusLost();
		}
	}

	void onMouseDrag(Point2 m) {
		if (mFocused && mInput->hasFocus() == false) {
			float s = (m.x - x - mLabel.width) / mSliderWidth;
			if (s > .999)
				s = 1;
			if (s < .001)
				s = 0;
			if (s == mScale) // don't dispatch an event if scale hasn't changed
				return;
			mScale = s;
			setValue(((mMax - mMin) * mScale) + mMin);
		}
	}

	void onMouseRelease(Point2 m) {
		ofxDatGuiComponent::onMouseRelease(m);
		if (mInput->hitTest(m) == false)
			onFocusLost();
	}

	void onFocusLost() {
		ofxDatGuiComponent::onFocusLost();
		if (mInput->hasFocus())
			mInput->onFocusLost();
	}

	void onKeyPressed(int key) {
		if (mInput->hasFocus())
			mInput->onKeyPressed(key);
	}

	void onInputChanged(ofxDatGuiInternalEvent e) {
		setValue(ofToFloat(mInput->getText()));
	}

	void dispatchSliderChangedEvent() {
		// update any bound variables
		if (mBoundf != nullptr) {
			*mBoundf = mValue;
		} else if (mBoundi != nullptr) {
			*mBoundi = mValue;
		} else if (mParamI != nullptr) {
			mParamI->set(mValue);
		} else if (mParamF != nullptr) {
			mParamF->set(mValue);
		}
		dispatchEvent(); // dispatch event out to main application
	}

public:
	void setTheme(const ofxDatGuiTheme *theme) {
		setComponentStyle(theme);
		mSliderFill = theme->color.slider.fill;
		mBackgroundFill = theme->color.inputAreaBackground;
		mStyle.stripe.color = theme->stripe.slider;
		mInput->setTheme(theme);
		mInput->setTextInactiveColor(theme->color.slider.text);
		setWidth(theme->layout.width, theme->layout.labelWidth);
	}

	void setWidth(int width, float labelWidth) {
		ofxDatGuiComponent::setWidth(width, labelWidth);
		float totalWidth = mStyle.width - mLabel.width;
		mSliderWidth = totalWidth * .7;
		mInputX = mLabel.width + mSliderWidth + mStyle.padding;
		mInputWidth = totalWidth - mSliderWidth - (mStyle.padding * 2);
		mInput->setWidth(mInputWidth);
		mInput->setPosition(x + mInputX, y + mStyle.padding);
	}

	void setPosition(int x, int y) {
		ofxDatGuiComponent::setPosition(x, y);
		mInput->setPosition(x + mInputX, y + mStyle.padding);
	}

	void setPrecision(int precision) {
		mPrecision = precision;
		if (mPrecision > MAX_PRECISION)
			mPrecision = MAX_PRECISION;
	}

	void setMin(float min) {
		mMin = min;
		if (mMin < mMax) {
			calculateScale();
		} else {
			onInvalidMinMaxValues();
		}
	}

	void setMax(float max) {
		mMax = max;
		if (mMax > mMin) {
			calculateScale();
		} else {
			onInvalidMinMaxValues();
		}
	}

	void setValue(float value, bool dispatchEvent = true) {
		value = round(value, mPrecision);
		if (value != mValue) {
			mValue = value;
			if (mValue > mMax) {
				mValue = mMax;
			} else if (mValue < mMin) {
				mValue = mMin;
			}
			calculateScale();
			if (dispatchEvent)
				dispatchSliderChangedEvent();
		}
	}

	float getValue() { return mValue; }

	void setScale(float scale) {
		mScale = scale;
		if (mScale < 0 || mScale > 1) {
			ofLogError() << "row #" << mIndex << " : scale must be between 0 & 1"
						 << " [setting to 50%]";
			mScale = 0.5f;
		}
		mValue = ((mMax - mMin) * mScale) + mMin;
	}

	float getScale() { return mScale; }

	// variable binding methods

	void bind(int &val) {
		mBoundi = &val;
		mBoundf = nullptr;
	}

	void bind(float &val) {
		mBoundf = &val;
		mBoundi = nullptr;
	}

	void bind(int &val, int min, int max) {
		mMin = min;
		mMax = max;
		mBoundi = &val;
		mBoundf = nullptr;
	}

	void bind(float &val, float min, float max) {
		mMin = min;
		mMax = max;
		mBoundf = &val;
		mBoundi = nullptr;
	}

	void update(bool acceptEvents = true) {
		ofxDatGuiComponent::update(acceptEvents);
		// check for variable bindings
		if (mBoundf != nullptr && !mInput->hasFocus()) {
			setValue(*mBoundf);
		} else if (mBoundi != nullptr && !mInput->hasFocus()) {
			setValue(*mBoundi);
		}
	}

	void draw() {
		if (!mVisible)
			return;
		ofPushStyle();
		ofxDatGuiComponent::draw();
		ofSetColor(mBackgroundFill); // slider bkgd
		ofDrawRectangle(x + mLabel.width, y + mStyle.padding, mSliderWidth, mStyle.height - (mStyle.padding * 2));
		if (mScale > 0) { // slider fill
			ofSetColor(mSliderFill);
			ofDrawRectangle(x + mLabel.width, y + mStyle.padding, mSliderWidth * mScale, mStyle.height - (mStyle.padding * 2));
		}
		// numeric input field
		mInput->draw();
		ofPopStyle();
	}

	bool hitTest(Point2 m) {
		if (!mEnabled || !mVisible) {
			return false;
		} else if (m.x >= x + mLabel.width && m.x <= x + mLabel.width + mSliderWidth && m.y >= y + mStyle.padding && m.y <= y + mStyle.height - mStyle.padding) {
			return true;
		} else if (mInput->hitTest(m)) {
			return true;
		} else {
			return false;
		}
	}

	void dispatchEvent() {
		if (sliderEventCallback != nullptr) {
			ofxDatGuiSliderEvent e(this, mValue, mScale);
			sliderEventCallback(e);
		} else {
			ofxDatGuiLog::write("ofxDatGuiSlider", ofxDatGuiMsg::EVENT_HANDLER_NULL);
		}
	}

	static ofxDatGuiSlider *getInstance() { return new ofxDatGuiSlider("X", 0, 100); }

	ofxDatGuiSlider(std::string label, float min, float max, float val) :
			ofxDatGuiComponent(label) {
		mMin = min;
		mMax = max;
		setPrecision(2);
		mType = ofxDatGuiType::SLIDER;
		mInput = new ofxDatGuiTextInputField();
		mInput->setTextInputFieldType(ofxDatGuiInputType::NUMERIC);
		mInput->onInternalEvent(this, &ofxDatGuiSlider::onInputChanged);
		setTheme(ofxDatGuiComponent::getTheme());
		setValue(val, false); // don't dispatch a change event when component is constructed //
	}

	ofxDatGuiSlider(string label, float min, float max) :
			ofxDatGuiSlider(label, min, max, (max + min) / 2) {}
	ofxDatGuiSlider(ofParameter<int> &p) :
			ofxDatGuiSlider(p.getName(), p.getMin(), p.getMax(), p.get()) {
		mParamI = &p;
		setPrecision(0);
		calculateScale();
		mParamI->addListener(this, &ofxDatGuiSlider::onParamI);
	}
	~ofxDatGuiSlider() { delete mInput; }
};

/// ofxDatGuiFRM

class ofxDatGuiFRM : public ofxDatGuiTextInput {
	float mTime;
	float mRefresh;

public:
	void update(bool ignoreMouseEvents = true) {
		if (ofGetElapsedTimef() - mTime > mRefresh) {
			mTime = ofGetElapsedTimef();
			mInput.setText(ofToString(ofGetFrameRate(), 2));
		}
	}

	ofxDatGuiFRM(float refresh = 1) :
			ofxDatGuiTextInput("framerate", "XX") {
		mRefresh = refresh;
		mTime = ofGetElapsedTimef();
		mInput.setText(ofToString(ofGetFrameRate(), 2));
	}
};

/// ofxDatGui2dPad

class ofxDatGui2dPad : public ofxDatGuiComponent {
	Point2 mLocal;
	Point2 mWorld;
	Rect2 mPad;
	Rect2 mBounds;
	float mPercentX;
	float mPercentY;
	int mBallSize;
	int mLineWeight;
	bool mScaleOnResize;
	struct {
		Color fill;
		Color line;
		Color ball;
	} mColors;

protected:
	void setWorldCoordinates() {
		mWorld.x = mBounds.x + (mBounds.width * mPercentX);
		mWorld.y = mBounds.y + (mBounds.height * mPercentY);
	}

	void onMouseDrag(Point2 m) {
		if (mPad.inside(m)) {
			mPercentX = (m.x - mPad.x) / mPad.width;
			mPercentY = (m.y - mPad.y) / mPad.height;
			setWorldCoordinates();
			dispatchEvent();
		}
	}

	void onWindowResized(ofResizeEventArgs &e) {
		if (mScaleOnResize) { // scale the bounds to the resized window
			mBounds.width *= (ofGetWidth() / mBounds.width);
			mBounds.height *= (ofGetHeight() / mBounds.height);
			setWorldCoordinates();
		}
	}

public:
	void setTheme(const ofxDatGuiTheme *theme) {
		setComponentStyle(theme);
		mStyle.height = theme->layout.pad2d.height;
		mStyle.stripe.color = theme->stripe.pad2d;
		mColors.line = theme->color.pad2d.line;
		mColors.ball = theme->color.pad2d.ball;
		mColors.fill = theme->color.inputAreaBackground;
		mBallSize = theme->layout.pad2d.ballSize;
		mLineWeight = theme->layout.pad2d.lineWeight;
		mPad = ofRectangle(0, 0, mStyle.width - mStyle.padding - mLabel.width, mStyle.height - (mStyle.padding * 2));
	}

	void setPoint(Point2 pt) {
		if (mBounds.inside(pt)) {
			mPercentX = (pt.x - mBounds.x) / mBounds.width;
			mPercentY = (pt.y - mBounds.y) / mBounds.height;
			setWorldCoordinates();
		} else {
			//  the point assigment is outside of the 2d pad's bounds //
		}
	}

	Point2 getPoint() { return mWorld; }

	void setBounds(ofRectangle bounds, bool scaleOnResize = false) {
		mBounds = bounds;
		mScaleOnResize = scaleOnResize;
		setWorldCoordinates();
	}

	ofRectangle getBounds() { return mBounds; }

	void reset() {
		mPercentX = 0.5f;
		mPercentY = 0.5f;
		setWorldCoordinates();
	}

	void draw() {
		if (!mVisible)
			return;
		ofPushStyle();
		mPad.x = x + mLabel.width;
		mPad.y = y + mStyle.padding;
		mPad.width = mStyle.width - mStyle.padding - mLabel.width;
		mLocal.x = mPad.x + mPad.width * mPercentX;
		mLocal.y = mPad.y + mPad.height * mPercentY;
		ofxDatGuiComponent::draw();
		ofSetColor(mColors.fill);
		ofDrawRectangle(mPad);
		ofSetLineWidth(mLineWeight);
		ofSetColor(mColors.line);
		ofDrawLine(mPad.x, mLocal.y, mPad.x + mPad.width, mLocal.y);
		ofDrawLine(mLocal.x, mPad.y, mLocal.x, mPad.y + mPad.height);
		ofSetColor(mColors.ball);
		ofDrawCircle(mLocal, mBallSize);
		ofPopStyle();
	}

	void dispatchEvent() {
		if (pad2dEventCallback != nullptr) {
			ofxDatGui2dPadEvent e(this, mWorld.x, mWorld.y);
			pad2dEventCallback(e);
		} else {
			ofxDatGuiLog::write(ofxDatGuiMsg::EVENT_HANDLER_NULL);
		}
	}

	static ofxDatGui2dPad *getInstance() { return new ofxDatGui2dPad("X"); }

	ofxDatGui2dPad(std::string label) :
			ofxDatGuiComponent(label) {
		mPercentX = 0.5f;
		mPercentY = 0.5f;
		mType = ofxDatGuiType::PAD2D;
		setTheme(ofxDatGuiComponent::getTheme());
		setBounds(ofRectangle(0, 0, ofGetWidth(), ofGetHeight()), true);
		ofAddListener(ofEvents().windowResized, this, &ofxDatGui2dPad::onWindowResized);
	}

	ofxDatGui2dPad(string label, ofRectangle bounds) :
			ofxDatGuiComponent(label) {
		mPercentX = 0.5f;
		mPercentY = 0.5f;
		mType = ofxDatGuiType::PAD2D;
		setTheme(ofxDatGuiComponent::getTheme());
		setBounds(bounds, false);
		ofAddListener(ofEvents().windowResized, this, &ofxDatGui2dPad::onWindowResized);
	}

	~ofxDatGui2dPad() { ofRemoveListener(ofEvents().windowResized, this, &ofxDatGui2dPad::onWindowResized); }
};

/// ofxDatGuiTimeGraph

class ofxDatGuiTimeGraph : public ofxDatGuiComponent {
protected:
	int mPointSize;
	int mLineWeight;
	struct {
		Color lines;
		Color fills;
	} mColor;
	std::vector<Vector2> pts;
	Rect2 mPlotterRect;
	void (ofxDatGuiTimeGraph::*mDrawFunc)() = nullptr;

	void setTheme(const ofxDatGuiTheme *theme) {
		setComponentStyle(theme);
		mStyle.height = theme->layout.graph.height;
		mStyle.stripe.color = theme->stripe.graph;
		mColor.lines = theme->color.graph.lines;
		mColor.fills = theme->color.graph.fills;
		mPointSize = theme->layout.graph.pointSize;
		mLineWeight = theme->layout.graph.lineWeight;
		setWidth(theme->layout.width, theme->layout.labelWidth);
	}

	void setWidth(int width, float labelWidth) {
		ofxDatGuiComponent::setWidth(width, labelWidth);
		mPlotterRect.x = mLabel.width;
		mPlotterRect.y = mStyle.padding;
		mPlotterRect.width = mStyle.width - mStyle.padding - mLabel.width;
		mPlotterRect.height = mStyle.height - (mStyle.padding * 2);
	}

	void draw() {
		if (!mVisible)
			return;
		ofPushStyle();
		ofxDatGuiComponent::draw();
		ofSetColor(mStyle.color.inputArea);
		ofDrawRectangle(x + mPlotterRect.x, y + mPlotterRect.y, mPlotterRect.width, mPlotterRect.height);
		glColor3ub(mColor.fills.r, mColor.fills.g, mColor.fills.b);
		(*this.*mDrawFunc)();
		ofPopStyle();
	}

	void drawFilled() {
		float px = this->x + mPlotterRect.x;
		float py = this->y + mPlotterRect.y;
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glBegin(GL_TRIANGLE_STRIP);
		for (int i = 0; i < pts.size(); i++) {
			glVertex2f(px + pts[i].x, py + mPlotterRect.height);
			glVertex2f(px + pts[i].x, py + pts[i].y);
		}
		glEnd();
	}

	void drawOutline() {
		float px = this->x + mPlotterRect.x;
		float py = this->y + mPlotterRect.y;
		glLineWidth(mLineWeight);
		glBegin(GL_LINE_LOOP);
		glVertex2f(px + mPlotterRect.width, py + mPlotterRect.height);
		for (int i = 0; i < pts.size(); i++)
			glVertex2f(px + pts[i].x, py + pts[i].y);
		glVertex2f(px, py + mPlotterRect.height);
		glEnd();
	}

	void drawLines() {
		float px = this->x + mPlotterRect.x;
		float py = this->y + mPlotterRect.y;
		glLineWidth(mLineWeight);
		glBegin(GL_LINE_STRIP);
		for (int i = 0; i < pts.size(); i++)
			glVertex2f(px + pts[i].x, py + pts[i].y);
		glEnd();
	}

	void drawPoints() {
		float px = this->x + mPlotterRect.x;
		float py = this->y + mPlotterRect.y;
		glPointSize(mLineWeight);
		glLineWidth(mLineWeight);
		glBegin(GL_POINTS);
		for (int i = 0; i < pts.size(); i++)
			glVertex2f(px + pts[i].x, py + pts[i].y);
		glEnd();
	}

	void setPosition(int x, int y) {
		ofxDatGuiComponent::setPosition(x, y);
	}

	ofxDatGuiTimeGraph(string label) :
			ofxDatGuiComponent(label) {
		mDrawFunc = &ofxDatGuiTimeGraph::drawFilled;
		setTheme(ofxDatGuiComponent::getTheme());
	}

public:
	void setDrawMode(ofxDatGuiGraph gMode) {
		switch (gMode) {
			case ofxDatGuiGraph::LINES:
				mDrawFunc = &ofxDatGuiTimeGraph::drawLines;
				break;
			case ofxDatGuiGraph::FILLED:
				mDrawFunc = &ofxDatGuiTimeGraph::drawFilled;
				break;
			case ofxDatGuiGraph::POINTS:
				mDrawFunc = &ofxDatGuiTimeGraph::drawPoints;
				break;
			case ofxDatGuiGraph::OUTLINE:
				mDrawFunc = &ofxDatGuiTimeGraph::drawOutline;
				break;
		}
	}
};

class ofxDatGuiWaveMonitor : public ofxDatGuiTimeGraph {
public:
	ofxDatGuiWaveMonitor(string label, float frequency, float amplitude) :
			ofxDatGuiTimeGraph(label) {
		mFrequencyLimit = 100;
		setAmplitude(amplitude);
		setFrequency(frequency);
		mType = ofxDatGuiType::WAVE_MONITOR;
		setTheme(ofxDatGuiComponent::getTheme());
	}

	static ofxDatGuiWaveMonitor *getInstance() { return new ofxDatGuiWaveMonitor("X", 0, 0); }

	// amplitude is a multiplier that affect the vertical height of the wave and should be a value between 0 & 1
	void setAmplitude(float amp) {
		mAmplitude = amp;
		if (mAmplitude < 0) {
			mAmplitude = 0;
		} else if (mAmplitude > MAX_AMPLITUDE) {
			mAmplitude = MAX_AMPLITUDE;
		}
		graph();
	}

	// frequency is a percentage of the limit to ensure the value is always in range
	void setFrequency(float freq) {
		mFrequency = (freq / mFrequencyLimit) * 100.0f;
		graph();
	}

	void setFrequencyLimit(float limit) {
		mFrequencyLimit = limit;
		setFrequency(mFrequency);
	}

	void setTheme(const ofxDatGuiTheme *tmplt) {
		ofxDatGuiTimeGraph::setTheme(tmplt);
		graph();
	}

	void setWidth(int width, float labelWidth) {
		ofxDatGuiTimeGraph::setWidth(width, labelWidth);
		graph();
	}

	void graph() {
		pts.clear();
		float step = 1.0 / mPlotterRect.width;
		float yAmp = (mPlotterRect.height / 2) * (mAmplitude / float(MAX_AMPLITUDE));
		for (int i = mPlotterRect.width; i > 0; i--) {
			float yp = mPlotterRect.height / 2 + (Math::sin((i * step) * (2 * mFrequency) * PI) * yAmp);
			pts.push_back(Vector2(i, yp));
		}
	}

	void update(bool ignoreMouseEvents) {
		pts[0].y = pts[pts.size() - 1].y;
		for (int i = mPlotterRect.width - 1; i > 0; i--)
			pts[i].y = pts[i - 1].y;
	}

private:
	float mAmplitude;
	float mFrequency;
	float mFrequencyLimit;
	static const int MAX_AMPLITUDE = 1;
};

class ofxDatGuiValuePlotter : public ofxDatGuiTimeGraph {
	float mVal;
	float mMin;
	float mMax;
	float mSpeed;

public:
	static ofxDatGuiValuePlotter *getInstance() { return new ofxDatGuiValuePlotter("X", 0, 0); }

	void setRange(float min, float max) {
		mMin = min;
		mMax = max;
		setValue((max + min) / 2);
	}

	void setSpeed(float speed) {
		if (speed != mSpeed) {
			pts.clear();
			mSpeed = speed;
		}
	}

	void setValue(float value) {
		mVal = value;
		if (mVal > mMax) {
			mVal = mMax;
		} else if (mVal < mMin) {
			mVal = mMin;
		}
	}

	float getMin() { return mMin; }

	float getMax() { return mMax; }

	float getRange() { return mMax - mMin; }

	void update(bool ignoreMouseEvents) {
		for (int i = 0; i < pts.size(); i++) // shift all points over before adding new value
			pts[i].x -= mSpeed;
		int i = 0;
		while (i < pts.size()) {
			if (pts.at(i).x <= 0) {
				pts.erase(pts.end() - 1);
			} else if (pts.at(i).x <= mSpeed) {
				pts.at(i).x = mLineWeight / 2;
			}
			i++;
		}
		float height = mPlotterRect.height - (mPlotterRect.height * ofxDatGuiScale(mVal, mMin, mMax));
		pts.insert(pts.begin(), ofVec2f(mPlotterRect.width, height));
	}

	ofxDatGuiValuePlotter(string label, float min, float max) :
			ofxDatGuiTimeGraph(label) {
		mSpeed = 5.0f;
		setRange(min, max);
		mType = ofxDatGuiType::VALUE_PLOTTER;
	}
};

/// ofxDatGuiMatrix

class ofxDatGuiMatrixButton : public ofxDatGuiInteractiveObject {
	int x;
	int y;
	int mIndex;
	Point2 origin;
	Rect2 mRect;
	ofColor mBkgdColor;
	ofColor mLabelColor;
	bool mSelected;
	bool mShowLabels;
	Rect2 mFontRect;
	shared_ptr<ofxSmartFont> mFont;
	struct {
		struct {
			Color label;
			Color button;
		} normal;
		struct {
			Color label;
			Color button;
		} hover;
		struct {
			Color label;
			Color button;
		} selected;
	} colors;

public:
	void setPosition(float x, float y) {
		origin.x = x;
		origin.y = y;
	}

	void draw(int x, int y) {
		mRect.x = x + origin.x;
		mRect.y = y + origin.y;
		ofPushStyle();
		ofFill();
		ofSetColor(mBkgdColor);
		ofDrawRectangle(mRect);
		if (mShowLabels) {
			ofSetColor(mLabelColor);
			mFont->draw(ofToString(mIndex + 1), mRect.x + mRect.width / 2 - mFontRect.width / 2, mRect.y + mRect.height / 2 + mFontRect.height / 2);
		}
		ofPopStyle();
	}

	void hitTest(Point2 m, bool mouseDown) {
		if (mRect.inside(m) && !mSelected) {
			if (mouseDown) {
				mBkgdColor = colors.selected.button;
				mLabelColor = colors.selected.label;
			} else {
				mBkgdColor = colors.hover.button;
				mLabelColor = colors.hover.label;
			}
		} else {
			onMouseOut();
		}
	}

	int getIndex() { return mIndex; }

	void setSelected(bool selected) { mSelected = selected; }

	bool getSelected() { return mSelected; }

	void onMouseOut() {
		if (mSelected) {
			mBkgdColor = colors.selected.button;
			mLabelColor = colors.selected.label;
		} else {
			mBkgdColor = colors.normal.button;
			mLabelColor = colors.normal.label;
		}
	}

	void onMouseRelease(Point2 m) {
		if (mRect.inside(m)) {
			mSelected = !mSelected;
			ofxDatGuiInternalEvent e(ofxDatGuiEventType::MATRIX_BUTTON_TOGGLED, mIndex);
			internalEventCallback(e);
		}
	}

	void setTheme(const ofxDatGuiTheme *theme) {
		mFont = theme->font.ptr;
		mFontRect = mFont->rect(ofToString(mIndex + 1));
		mBkgdColor = theme->color.matrix.normal.button;
		mLabelColor = theme->color.matrix.normal.label;
		colors.normal.label = theme->color.matrix.normal.label;
		colors.normal.button = theme->color.matrix.normal.button;
		colors.hover.label = theme->color.matrix.hover.label;
		colors.hover.button = theme->color.matrix.hover.button;
		colors.selected.label = theme->color.matrix.selected.label;
		colors.selected.button = theme->color.matrix.selected.button;
	}

	ofxDatGuiMatrixButton(int size, int index, bool showLabels) {
		mIndex = index;
		mSelected = false;
		mShowLabels = showLabels;
		mRect = ofRectangle(0, 0, size, size);
	}
};

class ofxDatGuiMatrix : public ofxDatGuiComponent {
private:
	int mButtonSize;
	int mNumButtons;
	int mButtonPadding;
	bool mRadioMode;
	bool mShowLabels;
	Color mFillColor;
	Rect2 mMatrixRect;
	std::vector<ofxDatGuiMatrixButton> btns;
	ofxDatGuiMatrixButton *mLastItemSelected;

protected:
	void onMouseRelease(Point2 m) {
		ofxDatGuiComponent::onFocusLost();
		ofxDatGuiComponent::onMouseRelease(m);
		for (int i = 0; i < btns.size(); i++)
			btns[i].onMouseRelease(m);
	}

	void onButtonSelected(ofxDatGuiInternalEvent e) {
		if (mRadioMode) {
			for (int i = 0; i < btns.size(); i++) // deselect all buttons save the one that was selected
				btns[i].setSelected(e.index == i);
		}
		mLastItemSelected = &btns[e.index];
		dispatchEvent();
	}

	void attachButtons(const ofxDatGuiTheme *theme) {
		btns.clear();
		for (int i = 0; i < mNumButtons; i++) {
			ofxDatGuiMatrixButton btn(mButtonSize, i, mShowLabels);
			btn.setTheme(theme);
			btn.onInternalEvent(this, &ofxDatGuiMatrix::onButtonSelected);
			btns.push_back(btn);
		}
	}

public:
	void setTheme(const ofxDatGuiTheme *theme) {
		setComponentStyle(theme);
		mFillColor = theme->color.inputAreaBackground;
		mButtonSize = theme->layout.matrix.buttonSize;
		mButtonPadding = theme->layout.matrix.buttonPadding;
		mStyle.stripe.color = theme->stripe.matrix;
		attachButtons(theme);
		setWidth(theme->layout.width, theme->layout.labelWidth);
	}

	void setWidth(int width, float labelWidth) {
		ofxDatGuiComponent::setWidth(width, labelWidth);
		mMatrixRect.x = x + mLabel.width;
		mMatrixRect.y = y + mStyle.padding;
		mMatrixRect.width = mStyle.width - mStyle.padding - mLabel.width;
		int nCols = floor(mMatrixRect.width / (mButtonSize + mButtonPadding));
		int nRows = ceil(btns.size() / float(nCols));
		float padding = (mMatrixRect.width - (mButtonSize * nCols)) / (nCols - 1);
		for (int i = 0; i < btns.size(); i++) {
			float bx = (mButtonSize + padding) * (i % nCols);
			float by = (mButtonSize + padding) * (floor(i / nCols));
			btns[i].setPosition(bx, by + mStyle.padding);
		}
		mStyle.height = (mStyle.padding * 2) + ((mButtonSize + padding) * (nRows - 1)) + mButtonSize;
		mMatrixRect.height = mStyle.height - (mStyle.padding * 2);
	}

	void setPosition(int x, int y) {
		ofxDatGuiComponent::setPosition(x, y);
		mMatrixRect.x = x + mLabel.width;
		mMatrixRect.y = y + mStyle.padding;
	}

	void setRadioMode(bool enabled) {
		mRadioMode = enabled;
	}

	bool hitTest(Point2 m) {
		if (mMatrixRect.inside(m)) {
			for (int i = 0; i < btns.size(); i++)
				btns[i].hitTest(m, mMouseDown);
			return true;
		} else {
			for (int i = 0; i < btns.size(); i++)
				btns[i].onMouseOut();
			return false;
		}
	}

	void draw() {
		if (!mVisible)
			return;
		ofPushStyle();
		ofxDatGuiComponent::draw();
		ofSetColor(mFillColor);
		ofDrawRectangle(mMatrixRect);
		for (int i = 0; i < btns.size(); i++)
			btns[i].draw(x + mLabel.width, y);
		ofPopStyle();
	}

	void clear() {
		for (int i = 0; i < btns.size(); i++)
			btns[i].setSelected(false);
	}

	void setSelected(vector<int> v) {
		clear();
		for (int i = 0; i < v.size(); i++)
			btns[v[i]].setSelected(true);
		mLastItemSelected = &btns[v.back()];
	}

	vector<int> getSelected() {
		vector<int> selected;
		for (int i = 0; i < btns.size(); i++)
			if (btns[i].getSelected())
				selected.push_back(i);
		return selected;
	}

	ofxDatGuiMatrixButton *getButtonAtIndex(int index) {
		return &btns[index];
	}

	void dispatchEvent() {
		if (matrixEventCallback != nullptr) {
			if (btns.size() != 0) {
				if (mLastItemSelected == nullptr) {
					mLastItemSelected = &btns.back();
				}
				ofxDatGuiMatrixEvent e(this, mLastItemSelected->getIndex(), mLastItemSelected->getSelected());
				matrixEventCallback(e);
			} else {
				ofxDatGuiLog::write(ofxDatGuiMsg::MATRIX_EMPTY);
			}
		} else {
			ofxDatGuiLog::write(ofxDatGuiMsg::EVENT_HANDLER_NULL);
		}
	}

	static ofxDatGuiMatrix *getInstance() { return new ofxDatGuiMatrix("X", 0); }

	ofxDatGuiMatrix(string label, int numButtons, bool showLabels = false) :
			ofxDatGuiComponent(label) {
		mRadioMode = false;
		mNumButtons = numButtons;
		mShowLabels = showLabels;
		mType = ofxDatGuiType::MATRIX;
		setTheme(ofxDatGuiComponent::getTheme());
	}
};

/// ofxDatGuiColorPicker

class ofxDatGuiColorPicker : public ofxDatGuiTextInput {
	Color mColor;
	Color gColor;

	struct {
		std::shared_ptr<Ref<Image>> image;
		Rect2 rect;
	} rainbow;

	bool mShowPicker;
	Color pickerBorder;
	Rect2 pickerRect;
	Rect2 gradientRect;

	ofVbo vbo;
	std::vector<Vector2> gPoints;
	std::vector<ofFloatColor> gColors;

	void updateTextFieldColors() {
		mInput.setBackgroundColor(mColor);

		// Counting the perceptive luminance - human eye favors green color...
		double a = 1 - (0.299 * mColor.r + 0.587 * mColor.g + 0.114 * mColor.b) / 255;
		mInput.setTextInactiveColor(a < 0.5 ? ofColor::black : ofColor::white);
	}

public:
	ofxDatGuiColorPicker(string label, ofColor color = ofColor::black) :
			ofxDatGuiTextInput(label, "XXXXXX") {
		mColor = color;
		mShowPicker = false;
		mType = ofxDatGuiType::COLOR_PICKER;
		setTheme(ofxDatGuiComponent::getTheme());

		// center the text input field
		mInput.setTextInputFieldType(ofxDatGuiInputType::COLORPICKER);
		setTextFieldInputColor();

		// setup the vbo that draws the main gradient
		gPoints.push_back(ofVec2f(0, 0));
		gPoints.push_back(ofVec2f(0, 0));
		gPoints.push_back(ofVec2f(0, 0));
		gPoints.push_back(ofVec2f(0, 0));
		gPoints.push_back(ofVec2f(0, 0));
		gPoints.push_back(ofVec2f(0, 0));

		ofColor center = ofColor(mColor.r / 2, mColor.g / 2, mColor.b / 2); // center point of the gradient is 1/2 way between mColor & black

		// draw main gradient as a six point triangle fan //
		gColors.push_back(center); // center
		gColors.push_back(ofColor::white); // top-left
		gColors.push_back(mColor); // top-right
		gColors.push_back(ofColor::black); // btm-right
		gColors.push_back(ofColor::black); // btm-left
		gColors.push_back(ofColor::white); // top-left
		vbo.setColorData(&gColors[0], 6, GL_DYNAMIC_DRAW);
	}

	void setTheme(const ofxDatGuiTheme *theme) {
		ofxDatGuiTextInput::setTheme(theme);
		mStyle.stripe.color = theme->stripe.colorPicker;
		pickerRect = ofRectangle(0, 0, mInput.getWidth(), (mStyle.height + mStyle.padding) * 3);
		rainbow.image = theme->icon.rainbow;
		rainbow.rect = ofRectangle(0, 0, theme->layout.colorPicker.rainbowWidth, pickerRect.height - (mStyle.padding * 2));
		gradientRect = ofRectangle(0, 0, pickerRect.width - rainbow.rect.width - (mStyle.padding * 3), rainbow.rect.height);
		pickerBorder = theme->color.colorPicker.border;
		setTextFieldInputColor();
	}

	void setColor(ofColor color) {
		mColor = color;
		setTextFieldInputColor();
	}

	void setColor(int hex) {
		mColor = ofColor::fromHex(hex);
		setTextFieldInputColor();
	}

	void setColor(int r, int g, int b, int a = 255) {
		mColor = ofColor(r, g, b, a);
		setTextFieldInputColor();
	}

	Color getColor() { return mColor; }

	void draw() {
		if (!mVisible)
			return;
		ofPushStyle();
		ofxDatGuiTextInput::draw();
		if (mShowPicker) {
			pickerRect.x = this->x + mLabel.width;
			pickerRect.y = this->y + mStyle.padding + mInput.getHeight();
			pickerRect.width = mInput.getWidth();
			rainbow.rect.x = pickerRect.x + pickerRect.width - rainbow.rect.width - mStyle.padding;
			rainbow.rect.y = pickerRect.y + mStyle.padding;
			gradientRect.x = pickerRect.x + mStyle.padding;
			gradientRect.y = pickerRect.y + mStyle.padding;
			gradientRect.width = pickerRect.width - rainbow.rect.width - (mStyle.padding * 3);
			gPoints[0] = ofVec2f(gradientRect.x + gradientRect.width / 2, gradientRect.y + gradientRect.height / 2);
			gPoints[1] = ofVec2f(gradientRect.x, gradientRect.y);
			gPoints[2] = ofVec2f(gradientRect.x + gradientRect.width, gradientRect.y);
			gPoints[3] = ofVec2f(gradientRect.x + gradientRect.width, gradientRect.y + gradientRect.height);
			gPoints[4] = ofVec2f(gradientRect.x, gradientRect.y + gradientRect.height);
			gPoints[5] = ofVec2f(gradientRect.x, gradientRect.y);
			vbo.setVertexData(&gPoints[0], 6, GL_DYNAMIC_DRAW);
			ofSetColor(pickerBorder);
			ofDrawRectangle(pickerRect);
			ofSetColor(ofColor::white);
			rainbow.image->draw(rainbow.rect);
			vbo.draw(GL_TRIANGLE_FAN, 0, 6);
		}
		ofPopStyle();
	}

	void drawColorPicker() {
		if (mVisible && mShowPicker) {
			ofPushStyle();
			ofSetColor(pickerBorder);
			ofDrawRectangle(pickerRect);
			ofSetColor(ofColor::white);
			rainbow.image->draw(rainbow.rect);
			vbo.draw(GL_TRIANGLE_FAN, 0, 6);
			ofPopStyle();
		}
	}

	bool hitTest(Point2 m) {
		if (mInput.hitTest(m)) {
			return true;
		} else if (mShowPicker && pickerRect.inside(m)) {
			unsigned char p[3];
			int y = (ofGetMouseY() - ofGetHeight()) * -1;
			glReadPixels(ofGetMouseX(), y, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, &p);
			gColor.r = int(p[0]);
			gColor.g = int(p[1]);
			gColor.b = int(p[2]);
			if (rainbow.rect.inside(m) && mMouseDown) {
				gColors[2] = gColor;
				gColors[0] = ofColor(gColor.r / 2, gColor.g / 2, gColor.b / 2);
				vbo.setColorData(&gColors[0], 6, GL_DYNAMIC_DRAW);
			} else if (gradientRect.inside(m) && mMouseDown) {
				mColor = gColor;
				dispatchEvent(); // dispatch event out to main application
				setTextFieldInputColor();
			}
			return true;
		} else {
			return false;
		}
	}

	void dispatchEvent() {
		if (colorPickerEventCallback != nullptr) {
			ofxDatGuiColorPickerEvent e(this, mColor);
			colorPickerEventCallback(e);
		} else {
			ofxDatGuiLog::write(ofxDatGuiMsg::EVENT_HANDLER_NULL);
		}
	}

	static ofxDatGuiColorPicker *getInstance() { return new ofxDatGuiColorPicker("X"); }

protected:
	void onMouseEnter(Point2 mouse) {
		mShowPicker = true;
		ofxDatGuiComponent::onFocus();
		ofxDatGuiComponent::onMouseEnter(mouse);
	}

	void onMouseLeave(Point2 mouse) {
		mShowPicker = false;
		ofxDatGuiTextInput::onMouseLeave(mouse);
		if (!mInput.hasFocus())
			ofxDatGuiComponent::onFocusLost();
	}

	void onMousePress(Point2 mouse) {
		ofxDatGuiComponent::onMousePress(mouse);
		if (mInput.hitTest(mouse))
			mInput.onFocus();
	}

	void onInputChanged(ofxDatGuiInternalEvent e) {
		mColor = ofColor::fromHex(ofHexToInt(mInput.getText()));
		updateTextFieldColors(); // set the input field text & background colors
		gColors[2] = mColor; // update the gradient picker
		gColors[0] = ofColor(mColor.r / 2, mColor.g / 2, mColor.b / 2);
		vbo.setColorData(&gColors[0], 6, GL_DYNAMIC_DRAW);
		dispatchEvent(); // dispatch event out to main application
	}

	_FORCE_INLINE_ void setTextFieldInputColor() {
		std::stringstream ss; // convert color value to a six character hex string
		ss << std::hex << mColor.getHex();
		std::string res(ss.str());
		while (res.size() < 6)
			res += "0";
		mInput.setText(ofToUpper(res));
		updateTextFieldColors();
	}
};

/// ofxDatGuiScrollView

class ofxDatGuiScrollViewItem : public ofxDatGuiButton {
	friend class ofxDatGuiScrollView;

	int mIndex;

public:
	int getIndex() { return mIndex; }

	ofxDatGuiScrollViewItem(string label, int index) :
			ofxDatGuiButton(label) {
		mIndex = index;
	}
};

class ofxDatGuiScrollView : public ofxDatGuiComponent {
	ofFbo mView;
	Rect2 mRect;
	Color mBackground;
	const ofxDatGuiTheme *mTheme;

	int mY = 0;
	int mSpacing;
	int mNumVisible;
	bool mAutoHeight;
	std::vector<ofxDatGuiScrollViewItem *> mItems;
	ofxDatGuiScrollViewItem *mLastItemSelected;

	void autoSize() {
		mRect.height = ((mTheme->layout.height + mSpacing) * mNumVisible) - mSpacing;
		if (mRect.width > 0 && mRect.height > 0)
			mView.allocate(mRect.width, mRect.height);
	}

	void onMouseScrolled(ofMouseEventArgs &e) {
		if (mItems.size() > 0 && mRect.inside(e.x, e.y) == true) {
			float sy = e.scrollY * 2;
			int btnH = mItems.front()->getHeight() + mSpacing;
			int minY = mRect.height + mSpacing - (mItems.size() * btnH);
			bool allowScroll = false;
			mY = mItems.front()->getY();
			if (sy < 0) {
				if (mY > minY) {
					mY += sy;
					if (mY < minY)
						mY = minY;
					allowScroll = true;
				}
			} else if (sy > 0) {
				if (mY < 0) {
					mY += sy;
					if (mY > 0)
						mY = 0;
					allowScroll = true;
				}
			}
			if (allowScroll) {
				mItems.front()->setPosition(0, mY);
				for (int i = 0; i < mItems.size(); i++)
					mItems[i]->setPosition(0, mY + (btnH * i));
			}
		}
	}

	void onButtonEvent(ofxDatGuiButtonEvent e) {
		for (int i = 0; i < mItems.size(); i++) {
			if (mItems[i] == e.target) {
				mLastItemSelected = mItems[i];
				dispatchEvent();
				break;
			}
		}
	}

	void positionItems() {
		int y = mY;
		for (int i = 0; i < mItems.size(); i++) {
			mItems[i]->mIndex = i;
			mItems[i]->setPosition(0, y);
			y = mItems[i]->getY() + mItems[i]->getHeight() + mSpacing;
		}
	}

	bool isValidIndex(int index) { return index >= 0 && index < mItems.size(); }

public:
	// list manipulation

	void add(string label) {
		int y = 0;
		if (mItems.size() > 0)
			y = mItems.back()->getY() + mItems.back()->getHeight() + mSpacing;
		mItems.push_back(new ofxDatGuiScrollViewItem(label, mItems.size()));
		mItems.back()->setMask(mRect);
		mItems.back()->setTheme(mTheme);
		mItems.back()->setWidth(mRect.width, 0);
		mItems.back()->setPosition(0, y);
		mItems.back()->onButtonEvent(this, &ofxDatGuiScrollView::onButtonEvent);
		//  cout << "ofxDatGuiScrollView :: total items = " << mItems.size() << endl;
		if (mAutoHeight)
			autoSize();
	}

	ofxDatGuiScrollViewItem *getItemAtIndex(int index) { return mItems[index]; }

	ofxDatGuiScrollViewItem *getItemByName(string name) {
		for (auto i : mItems)
			if (i->is(name))
				return i;
		return nullptr;
	}

	void swap(int index1, int index2) {
		if (isValidIndex(index1) && isValidIndex(index2) && index1 != index2) {
			std::swap(mItems[index1], mItems[index2]);
			positionItems();
		}
	}

	void move(int from, int to) {
		if (isValidIndex(from) && isValidIndex(to) && from != to) {
			auto itr_from = mItems.begin() + from;
			auto itr_to = mItems.begin() + to;
			if (itr_from < itr_to) {
				// move down //
				rotate(itr_from, itr_from + 1, itr_to + 1);
			} else if (itr_from > itr_to) {
				// move up //
				rotate(itr_to, itr_from, itr_from + 1);
			}
			positionItems();
		} else {
			cout << "invalid move operation, check your indices" << endl;
		}
	}

	void move(ofxDatGuiComponent *item, int index) {
		for (int i = 0; i < mItems.size(); i++) {
			if (mItems[i] == item) {
				move(i, index);
				return;
			}
		}
	}

	void clear() {
		for (auto i : mItems)
			delete i;
		mItems.clear();
	}

	void remove(int index) {
		if (isValidIndex(index)) {
			delete mItems[index];
			mItems.erase(mItems.begin() + index);
		}
		positionItems();
	}

	void remove(ofxDatGuiComponent *item) {
		for (int i = 0; i < mItems.size(); i++) {
			if (mItems[i] == item) {
				delete mItems[i];
				mItems.erase(mItems.begin() + i);
				positionItems();
				return;
			}
		}
	}

	// temporary getters until mRect is implemented in ofxDatGuiComponent

	int getX() { return mRect.x; }
	int getY() { return mRect.y; }
	int getWidth() { return mRect.width; }
	int getHeight() { return mRect.height; }

	int getNumItems() { return mItems.size(); }

	// list presentation

	void setTheme(const ofxDatGuiTheme *theme) {
		mTheme = theme;
		mSpacing = theme->layout.vMargin;
		mBackground = theme->color.guiBackground;
		for (auto i : mItems)
			i->setTheme(theme);
		setWidth(theme->layout.width, theme->layout.labelWidth);
	}

	void setWidth(int width, float labelWidth = 1) {
		mRect.width = width;
		for (auto i : mItems)
			i->setWidth(mRect.width, labelWidth);
		if (mAutoHeight)
			autoSize();
	}

	void setHeight(int height) {
		mAutoHeight = false;
		mRect.height = height;
		if (mRect.width > 0 && mRect.height > 0)
			mView.allocate(mRect.width, mRect.height);
	}

	void setPosition(int x, int y) {
		mRect.x = x;
		mRect.y = y;
		for (int i = 0; i < mItems.size(); i++) // update each component's mask so mouse events track correctly
			mItems[i]->setMask(mRect);
	}

	void setItemSpacing(int spacing) { mSpacing = spacing; }

	void setBackgroundColor(ofColor color) { mBackground = color; }

	// update & draw

	void update() {
		for (auto i : mItems)
			i->update();
	}

	void draw() {
		ofPushStyle();
		ofFill();
		ofSetColor(ofColor::black); // draw a background behind the fbo
		ofDrawRectangle(mRect);
		mView.begin(); // draw into the fbo
		ofClear(255, 255, 255, 0);
		ofSetColor(mBackground);
		ofDrawRectangle(0, 0, mRect.width, mRect.height);
		for (auto i : mItems)
			i->draw();
		mView.end();
		ofSetColor(ofColor::white); // draw the fbo of list content
		mView.draw(mRect.x, mRect.y);
		ofPopStyle();
	}

	void dispatchEvent() {
		if (scrollViewEventCallback != nullptr) {
			ofxDatGuiScrollViewEvent e(this, mLastItemSelected);
			scrollViewEventCallback(e);
		} else {
			ofxDatGuiLog::write(ofxDatGuiMsg::EVENT_HANDLER_NULL);
		}
	}

	ofxDatGuiScrollView(string name, int nVisible = 6) :
			ofxDatGuiComponent(name) {
		mAutoHeight = true;
		mNumVisible = nVisible;
		setTheme(ofxDatGuiComponent::getTheme());
		ofAddListener(ofEvents().mouseScrolled, this, &ofxDatGuiScrollView::onMouseScrolled, OF_EVENT_ORDER_BEFORE_APP);
	}

	~ofxDatGuiScrollView() {
		mTheme = nullptr;
		ofRemoveListener(ofEvents().mouseScrolled, this, &ofxDatGuiScrollView::onMouseScrolled, OF_EVENT_ORDER_BEFORE_APP);
	}
};

/// ofxDatGuiGroup

class ofxDatGuiGroup : public ofxDatGuiButton {
protected:
	int mHeight;
	std::shared_ptr<Ref<Image>> mIconOpen;
	std::shared_ptr<Ref<Image>> mIconClosed;
	bool mIsExpanded;

	void layout() {
		mHeight = mStyle.height + mStyle.vMargin;
		for (int i = 0; i < children.size(); i++) {
			if (children[i]->getVisible() == false)
				continue;
			if (mIsExpanded == false) {
				children[i]->setPosition(x, y + mHeight);
			} else {
				children[i]->setPosition(x, y + mHeight);
				mHeight += children[i]->getHeight() + mStyle.vMargin;
			}
			if (i == children.size() - 1)
				mHeight -= mStyle.vMargin;
		}
	}

	void onMouseRelease(Point2 m) {
		if (mFocused) { // open & close the group when its header is clicked
			ofxDatGuiComponent::onFocusLost();
			ofxDatGuiComponent::onMouseRelease(m);
			mIsExpanded ? collapse() : expand();
		}
	}

	void onGroupToggled() {
		// dispatch an event out to the gui panel to adjust its children
		if (internalEventCallback != nullptr) {
			ofxDatGuiInternalEvent e(ofxDatGuiEventType::GROUP_TOGGLED, mIndex);
			internalEventCallback(e);
		}
	}

	void dispatchInternalEvent(ofxDatGuiInternalEvent e) {
		if (e.type == ofxDatGuiEventType::VISIBILITY_CHANGED)
			layout();
		internalEventCallback(e);
	}

public:
	void setPosition(int x, int y) {
		ofxDatGuiComponent::setPosition(x, y);
		layout();
	}

	void expand() {
		mIsExpanded = true;
		layout();
		onGroupToggled();
	}

	void toggle() {
		mIsExpanded = !mIsExpanded;
		layout();
		onGroupToggled();
	}

	void collapse() {
		mIsExpanded = false;
		layout();
		onGroupToggled();
	}

	int getHeight() { return mHeight; }

	bool getIsExpanded() { return mIsExpanded; }

	void draw() {
		if (mVisible) {
			ofPushStyle();
			ofxDatGuiButton::draw();
			if (mIsExpanded) {
				int mHeight = mStyle.height;
				ofSetColor(mStyle.guiBackground, mStyle.opacity);
				ofDrawRectangle(x, y + mHeight, mStyle.width, mStyle.vMargin);
				for (int i = 0; i < children.size(); i++) {
					mHeight += mStyle.vMargin;
					children[i]->draw();
					mHeight += children[i]->getHeight();
					if (i == children.size() - 1)
						break;
					ofSetColor(mStyle.guiBackground, mStyle.opacity);
					ofDrawRectangle(x, y + mHeight, mStyle.width, mStyle.vMargin);
				}
				ofSetColor(mIcon.color);
				mIconOpen->draw(x + mIcon.x, y + mIcon.y, mIcon.size, mIcon.size);
				for (int i = 0; i < children.size(); i++)
					children[i]->drawColorPicker();
			} else {
				ofSetColor(mIcon.color);
				mIconClosed->draw(x + mIcon.x, y + mIcon.y, mIcon.size, mIcon.size);
			}
			ofPopStyle();
		}
	}

	ofxDatGuiGroup(string label) :
			ofxDatGuiButton(label), mHeight(0) {
		mIsExpanded = false;
		layout();
	}

	~ofxDatGuiGroup() {
		for (auto i : children) // color pickers are deleted automatically when the group is destroyed
			if (i->getType() != ofxDatGuiType::COLOR_PICKER)
				delete i;
	}
};

class ofxDatGuiFolder : public ofxDatGuiGroup {
public:
	ofxDatGuiFolder(string label, ofColor color = ofColor::white) :
			ofxDatGuiGroup(label) {
		mStyle.stripe.color = color; // all items within a folder share the same stripe color
		mType = ofxDatGuiType::FOLDER;
		setTheme(ofxDatGuiComponent::getTheme());
	}

	void setTheme(const ofxDatGuiTheme *theme) {
		setComponentStyle(theme);
		mIconOpen = theme->icon.groupOpen;
		mIconClosed = theme->icon.groupClosed;
		setWidth(theme->layout.width, theme->layout.labelWidth);
		for (auto i : children) // reassign folder color to all components
			i->setStripeColor(mStyle.stripe.color);
	}

	void setWidth(int width, float labelWidth = 1) {
		ofxDatGuiComponent::setWidth(width, labelWidth);
		mLabel.width = mStyle.width;
		mLabel.rightAlignedXpos = mIcon.x - mLabel.margin;
		ofxDatGuiComponent::positionLabel();
	}

	void drawColorPicker() {
		for (int i = 0; i < pickers.size(); i++)
			pickers[i]->drawColorPicker();
	}

	void dispatchButtonEvent(ofxDatGuiButtonEvent e) {
		if (buttonEventCallback != nullptr) {
			buttonEventCallback(e);
		} else {
			ofxDatGuiLog::write(ofxDatGuiMsg::EVENT_HANDLER_NULL);
		}
	}

	void dispatchToggleEvent(ofxDatGuiToggleEvent e) {
		if (toggleEventCallback != nullptr) {
			toggleEventCallback(e);
		} else if (buttonEventCallback != nullptr) { // allow toggle events to decay into button events
			buttonEventCallback(ofxDatGuiButtonEvent(e.target));
		} else {
			ofxDatGuiLog::write(ofxDatGuiMsg::EVENT_HANDLER_NULL);
		}
	}

	void dispatchSliderEvent(ofxDatGuiSliderEvent e) {
		if (sliderEventCallback != nullptr) {
			sliderEventCallback(e);
		} else {
			ofxDatGuiLog::write(ofxDatGuiMsg::EVENT_HANDLER_NULL);
		}
	}

	void dispatchTextInputEvent(ofxDatGuiTextInputEvent e) {
		if (textInputEventCallback != nullptr) {
			textInputEventCallback(e);
		} else {
			ofxDatGuiLog::write(ofxDatGuiMsg::EVENT_HANDLER_NULL);
		}
	}

	void dispatchColorPickerEvent(ofxDatGuiColorPickerEvent e) {
		if (colorPickerEventCallback != nullptr) {
			colorPickerEventCallback(e);
		} else {
			ofxDatGuiLog::write(ofxDatGuiMsg::EVENT_HANDLER_NULL);
		}
	}

	void dispatch2dPadEvent(ofxDatGui2dPadEvent e) {
		if (pad2dEventCallback != nullptr) {
			pad2dEventCallback(e);
		} else {
			ofxDatGuiLog::write(ofxDatGuiMsg::EVENT_HANDLER_NULL);
		}
	}

	void dispatchMatrixEvent(ofxDatGuiMatrixEvent e) {
		if (matrixEventCallback != nullptr) {
			matrixEventCallback(e);
		} else {
			ofxDatGuiLog::write(ofxDatGuiMsg::EVENT_HANDLER_NULL);
		}
	}

	// component add methods

	ofxDatGuiLabel *addLabel(string label) {
		ofxDatGuiLabel *lbl = new ofxDatGuiLabel(label);
		lbl->setStripeColor(mStyle.stripe.color);
		attachItem(lbl);
		return lbl;
	}

	ofxDatGuiButton *addButton(string label) {
		ofxDatGuiButton *button = new ofxDatGuiButton(label);
		button->setStripeColor(mStyle.stripe.color);
		button->onButtonEvent(this, &ofxDatGuiFolder::dispatchButtonEvent);
		attachItem(button);
		return button;
	}

	ofxDatGuiToggle *addToggle(string label, bool enabled = false) {
		ofxDatGuiToggle *toggle = new ofxDatGuiToggle(label, enabled);
		toggle->setStripeColor(mStyle.stripe.color);
		toggle->onToggleEvent(this, &ofxDatGuiFolder::dispatchToggleEvent);
		attachItem(toggle);
		return toggle;
	}

	ofxDatGuiSlider *addSlider(string label, float min, float max) {
		ofxDatGuiSlider *slider = addSlider(label, min, max, (max + min) / 2);
		return slider;
	}

	ofxDatGuiSlider *addSlider(string label, float min, float max, double val) {
		ofxDatGuiSlider *slider = new ofxDatGuiSlider(label, min, max, val);
		slider->setStripeColor(mStyle.stripe.color);
		slider->onSliderEvent(this, &ofxDatGuiFolder::dispatchSliderEvent);
		attachItem(slider);
		return slider;
	}

	ofxDatGuiSlider *addSlider(ofParameter<int> &p) {
		ofxDatGuiSlider *slider = new ofxDatGuiSlider(p);
		slider->setStripeColor(mStyle.stripe.color);
		slider->onSliderEvent(this, &ofxDatGuiFolder::dispatchSliderEvent);
		attachItem(slider);
		return slider;
	}

	ofxDatGuiSlider *addSlider(ofParameter<float> &p) {
		ofxDatGuiSlider *slider = new ofxDatGuiSlider(p);
		slider->setStripeColor(mStyle.stripe.color);
		slider->onSliderEvent(this, &ofxDatGuiFolder::dispatchSliderEvent);
		attachItem(slider);
		return slider;
	}

	ofxDatGuiTextInput *addTextInput(string label, string value) {
		ofxDatGuiTextInput *input = new ofxDatGuiTextInput(label, value);
		input->setStripeColor(mStyle.stripe.color);
		input->onTextInputEvent(this, &ofxDatGuiFolder::dispatchTextInputEvent);
		attachItem(input);
		return input;
	}

	ofxDatGuiColorPicker *addColorPicker(string label, ofColor color = ofColor::black) {
		shared_ptr<ofxDatGuiColorPicker> picker(new ofxDatGuiColorPicker(label, color));
		picker->setStripeColor(mStyle.stripe.color);
		picker->onColorPickerEvent(this, &ofxDatGuiFolder::dispatchColorPickerEvent);
		attachItem(picker.get());
		pickers.push_back(picker);
		return picker.get();
	}

	ofxDatGuiFRM *addFRM(float refresh = 1.0f) {
		ofxDatGuiFRM *monitor = new ofxDatGuiFRM(refresh);
		monitor->setStripeColor(mStyle.stripe.color);
		attachItem(monitor);
		return monitor;
	}

	ofxDatGuiBreak *addBreak() {
		ofxDatGuiBreak *brk = new ofxDatGuiBreak();
		attachItem(brk);
		return brk;
	}

	ofxDatGui2dPad *add2dPad(string label) {
		ofxDatGui2dPad *pad = new ofxDatGui2dPad(label);
		pad->setStripeColor(mStyle.stripe.color);
		pad->on2dPadEvent(this, &ofxDatGuiFolder::dispatch2dPadEvent);
		attachItem(pad);
		return pad;
	}

	ofxDatGuiMatrix *addMatrix(string label, int numButtons, bool showLabels = false) {
		ofxDatGuiMatrix *matrix = new ofxDatGuiMatrix(label, numButtons, showLabels);
		matrix->setStripeColor(mStyle.stripe.color);
		matrix->onMatrixEvent(this, &ofxDatGuiFolder::dispatchMatrixEvent);
		attachItem(matrix);
		return matrix;
	}

	ofxDatGuiWaveMonitor *addWaveMonitor(string label, float frequency, float amplitude) {
		ofxDatGuiWaveMonitor *monitor = new ofxDatGuiWaveMonitor(label, frequency, amplitude);
		monitor->setStripeColor(mStyle.stripe.color);
		attachItem(monitor);
		return monitor;
	}

	ofxDatGuiValuePlotter *addValuePlotter(string label, float min, float max) {
		ofxDatGuiValuePlotter *plotter = new ofxDatGuiValuePlotter(label, min, max);
		plotter->setStripeColor(mStyle.stripe.color);
		attachItem(plotter);
		return plotter;
	}

	void attachItem(ofxDatGuiComponent *item) {
		item->setIndex(children.size());
		item->onInternalEvent(this, &ofxDatGuiFolder::dispatchInternalEvent);
		children.push_back(item);
	}

	ofxDatGuiComponent *getComponent(ofxDatGuiType type, string label) {
		for (int i = 0; i < children.size(); i++) {
			if (children[i]->getType() == type) {
				if (children[i]->is(label))
					return children[i];
			}
		}
		return NULL;
	}

	static ofxDatGuiFolder *getInstance() { return new ofxDatGuiFolder("X"); }

protected:
	std::vector<std::shared_ptr<ofxDatGuiColorPicker>> pickers;
};

class ofxDatGuiDropdownOption : public ofxDatGuiButton {
public:
	ofxDatGuiDropdownOption(std::string label) :
			ofxDatGuiButton(label) {
		mType = ofxDatGuiType::DROPDOWN_OPTION;
		setTheme(ofxDatGuiComponent::getTheme());
	}

	void setTheme(const ofxDatGuiTheme *theme) {
		ofxDatGuiButton::setTheme(theme);
		mStyle.stripe.color = theme->stripe.dropdown;
	}

	void setWidth(int width, float labelWidth = 1) {
		ofxDatGuiComponent::setWidth(width, labelWidth);
		mLabel.width = mStyle.width;
		mLabel.rightAlignedXpos = mIcon.x - mLabel.margin;
		ofxDatGuiComponent::positionLabel();
	}
};

class ofxDatGuiDropdown : public ofxDatGuiGroup {
	void onOptionSelected(ofxDatGuiButtonEvent e) {
		for (int i = 0; i < children.size(); i++)
			if (e.target == children[i])
				mOption = i;
		setLabel(children[mOption]->getLabel());
		collapse();
		dispatchEvent();
	}

	int mOption;

public:
	void setTheme(const ofxDatGuiTheme *theme) {
		setComponentStyle(theme);
		mIconOpen = theme->icon.groupOpen;
		mIconClosed = theme->icon.groupClosed;
		mStyle.stripe.color = theme->stripe.dropdown;
		setWidth(theme->layout.width, theme->layout.labelWidth);
	}

	void setWidth(int width, float labelWidth = 1) {
		ofxDatGuiComponent::setWidth(width, labelWidth);
		mLabel.width = mStyle.width;
		mLabel.rightAlignedXpos = mIcon.x - mLabel.margin;
		ofxDatGuiComponent::positionLabel();
	}

	void select(int cIndex) {
		// ensure value is in range //
		if (cIndex < 0 || cIndex >= children.size()) {
			ofLogError() << "ofxDatGuiDropdown->select(" << cIndex << ") is out of range";
		} else {
			setLabel(children[cIndex]->getLabel());
		}
	}

	int size() { return children.size(); }

	ofxDatGuiDropdownOption *getChildAt(int index) { return static_cast<ofxDatGuiDropdownOption *>(children[index]); }

	ofxDatGuiDropdownOption *getSelected() { return static_cast<ofxDatGuiDropdownOption *>(children[mOption]); }

	void dispatchEvent() {
		if (dropdownEventCallback != nullptr) {
			ofxDatGuiDropdownEvent e(this, mIndex, mOption);
			dropdownEventCallback(e);
		} else {
			ofxDatGuiLog::write(ofxDatGuiMsg::EVENT_HANDLER_NULL);
		}
	}

	static ofxDatGuiDropdown *getInstance() { return new ofxDatGuiDropdown("X"); }

	ofxDatGuiDropdown(std::string label, const vector<string> &options = vector<string>()) :
			ofxDatGuiGroup(label) {
		mOption = 0;
		mType = ofxDatGuiType::DROPDOWN;
		for (int i = 0; i < options.size(); i++) {
			ofxDatGuiDropdownOption *opt = new ofxDatGuiDropdownOption(options[i]);
			opt->setIndex(children.size());
			opt->onButtonEvent(this, &ofxDatGuiDropdown::onOptionSelected);
			children.push_back(opt);
		}
		setTheme(ofxDatGuiComponent::getTheme());
	}
};

/// ofxDatGuiControls

class ofxDatGuiHeader : public ofxDatGuiButton {
	bool mDraggable;
	Point2 mDragOffset;

protected:
	void onMouseEnter(Point2 m) {
		if (mDraggable) // disable hover state if we're not draggable
			ofxDatGuiComponent::onMouseEnter(m);
	}

	void onMousePress(Point2 m) {
		mDragOffset = Point2(m.x - this->x, m.y - this->y);
		ofxDatGuiComponent::onMousePress(m);
	}

	void onMouseRelease(Point2 m) {
		mDragOffset = m;
		ofxDatGuiComponent::onFocusLost();
		ofxDatGuiComponent::onMouseRelease(m);
	}

	void onFocusLost() {} // allow panel to be dragged around

	// force header label to always be centered
	void setLabelAlignment(ofxDatGuiAlignment align) {
		ofxDatGuiComponent::setLabelAlignment(ofxDatGuiAlignment::CENTER);
	}

public:
	void setTheme(const ofxDatGuiTheme *theme) {
		setComponentStyle(theme);
		mLabel.width = mStyle.width;
		mStyle.stripe.visible = false;
		mStyle.height = mStyle.height * .8;
		setLabelAlignment(ofxDatGuiAlignment::CENTER);
	}

	void setDraggable(bool draggable) { mDraggable = draggable; }
	bool getDraggable() { return mDraggable; }
	Point2 getDragOffset() { return mDragOffset; }

	ofxDatGuiHeader(string label, bool draggable = true) :
			ofxDatGuiButton(label) {
		mDraggable = draggable;
		setTheme(ofxDatGuiComponent::getTheme());
	}
};

class ofxDatGuiFooter : public ofxDatGuiButton {
	bool mGuiExpanded;
	std::string mLabelExpanded;
	std::string mLabelCollapsed;

protected:
	void onMouseRelease(Point2 m) {
		ofxDatGuiComponent::onMouseRelease(m);
		ofxDatGuiInternalEvent e(ofxDatGuiEventType::GUI_TOGGLED, mIndex); // dispatch event out to main application
		internalEventCallback(e);
	}

	// force footer label to always be centered //
	void setLabelAlignment(ofxDatGuiAlignment align) {
		ofxDatGuiComponent::setLabelAlignment(ofxDatGuiAlignment::CENTER);
	}

public:
	ofxDatGuiFooter() :
			ofxDatGuiButton("collapse controls") {
		mGuiExpanded = true;
		mLabelCollapsed = "expand controls";
		mLabelExpanded = "collapse controls";
		setTheme(ofxDatGuiComponent::getTheme());
	}

	void setTheme(const ofxDatGuiTheme *theme) {
		setComponentStyle(theme);
		mLabel.width = mStyle.width;
		mStyle.stripe.visible = false;
		mStyle.height = mStyle.height * .8;
		setLabelAlignment(ofxDatGuiAlignment::CENTER);
	}

	void setLabelWhenExpanded(string label) {
		mLabelExpanded = label;
		if (mGuiExpanded)
			setLabel(mLabelExpanded);
	}

	void setLabelWhenCollapsed(string label) {
		mLabelCollapsed = label;
		if (!mGuiExpanded)
			setLabel(mLabelCollapsed);
	}

	void setExpanded(bool expanded) {
		mGuiExpanded = expanded;
		if (mGuiExpanded) {
			setLabel(mLabelExpanded);
		} else {
			setLabel(mLabelCollapsed);
		}
	}
};

#endif // OFX_DATGUICOMPONENTS_H
