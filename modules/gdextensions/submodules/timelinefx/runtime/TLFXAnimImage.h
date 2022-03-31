/*************************************************************************/
/*  TLFXAnimImage.h                                                      */
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

#ifdef _MSC_VER
#pragma once
#endif

#ifndef _TLFX_ANIMIMAGE_H
#define _TLFX_ANIMIMAGE_H

#include <string>

namespace TLFX {

class AnimImage {
public:
	AnimImage();
	virtual ~AnimImage() {}

	enum ImportOptions {
		impGreyScale,
		impFullColour,
		impPassThrough
	};

	virtual bool Load() = 0;

	void SetWidth(float width);
	virtual float GetWidth() const;
	void SetHeight(float height);
	virtual float GetHeight() const;
	void SetMaxRadius(float radius);
	virtual float GetMaxRadius() const;
	void SetFramesCount(int frames);
	virtual int GetFramesCount() const;
	void SetIndex(int index);
	virtual int GetIndex() const;
	void SetFilename(const char *filename);
	virtual const char *GetFilename() const;
	void SetImportOpt(const char *opt);
	ImportOptions GetImportOpt() const;
	void SetName(const char *name);
	virtual const char *GetName() const;

	virtual void FindRadius() {}

protected:
	float _width;
	float _height;
	float _maxRadius;
	int _index;
	int _frames;
	std::string _filename;
	ImportOptions _importOpt;
	std::string _name;
};

} // namespace TLFX

#endif // _TLFX_ANIMIMAGE_H
