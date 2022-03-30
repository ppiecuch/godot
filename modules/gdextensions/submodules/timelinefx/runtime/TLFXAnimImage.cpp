/*************************************************************************/
/*  TLFXAnimImage.cpp                                                    */
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

#include "TLFXAnimImage.h"

#include <string.h> // strcmp

namespace TLFX {

AnimImage::AnimImage() :
		_width(0), _height(0), _maxRadius(0), _index(0), _frames(1), _importOpt(impPassThrough) {
}

void AnimImage::SetMaxRadius(float radius) {
	_maxRadius = radius;
}

float AnimImage::GetMaxRadius() const {
	return _maxRadius;
}

void AnimImage::SetWidth(float width) {
	_width = width;
}

float AnimImage::GetWidth() const {
	return _width;
}

void AnimImage::SetHeight(float height) {
	_height = height;
}

float AnimImage::GetHeight() const {
	return _height;
}

void AnimImage::SetFramesCount(int frames) {
	_frames = frames;
}

int AnimImage::GetFramesCount() const {
	return _frames;
}

void AnimImage::SetIndex(int index) {
	_index = index;
}

int AnimImage::GetIndex() const {
	return _index;
}

void AnimImage::SetFilename(const char *filename) {
	_filename = filename;
}

const char *AnimImage::GetFilename() const {
	return _filename.c_str();
}

void AnimImage::SetImportOpt(const char *opt) {
	if (strcmp(opt, "GREYSCALE") == 0)
		_importOpt = impGreyScale;
	else if (strcmp(opt, "FULLCOLOR") == 0)
		_importOpt = impFullColour;
	else if (strcmp(opt, "PASSTHROUGH") == 0)
		_importOpt = impPassThrough;
}

AnimImage::ImportOptions AnimImage::GetImportOpt() const {
	return _importOpt;
}

void AnimImage::SetName(const char *name) {
	_name = name;
}

const char *AnimImage::GetName() const {
	return _name.c_str();
}

} // namespace TLFX
