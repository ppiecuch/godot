/**************************************************************************/
/*  TLFXXMLLoader.h                                                       */
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

#ifdef _MSC_VER
#pragma once
#endif

#ifndef _TLFX_XMLLOADER_H
#define _TLFX_XMLLOADER_H

#include <list>
#include <string>

namespace TLFX {

class AnimImage;
class Effect;
class Emitter;

class XMLLoader {
public:
	XMLLoader(int shapes) :
			_existingShapeCount(shapes) {}
	virtual ~XMLLoader() {}

	virtual bool Open(const char *filename) = 0;
	virtual bool GetNextShape(AnimImage *shape) = 0;
	virtual Effect *GetNextEffect(const std::list<AnimImage *> &sprites) = 0;
	virtual Effect *GetNextSuperEffect(const std::list<AnimImage *> &sprites) = 0;
	virtual void LocateEffect() = 0;
	virtual void LocateSuperEffect() = 0;

	virtual const char *GetLastError() const { return "no error reporting implemented"; }

	int _existingShapeCount;
};

} // namespace TLFX

#endif // _TLFX_XMLLOADER_H
