/*************************************************************************/
/*  TLFXPugiXMLLoader.h                                                  */
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

#ifndef _TLFX_PUGIXMLLOADER_H
#define _TLFX_PUGIXMLLOADER_H

#include "TLFXXMLLoader.h"
#include "misc/pugixml.h"

namespace TLFX {

struct AttributeNode;

class PugiXMLLoader : public XMLLoader {
public:
	PugiXMLLoader(int shapes, const char *libraryfile = 0) :
			XMLLoader(shapes), _library(libraryfile) {}
	PugiXMLLoader(const char *libraryfile) :
			XMLLoader(0), _library(libraryfile) {}

	virtual bool Open(const char *filename);
	virtual bool GetNextShape(AnimImage *shape);
	virtual Effect *GetNextEffect(const std::list<AnimImage *> &sprites);
	virtual Effect *GetNextSuperEffect(const std::list<AnimImage *> &sprites);

	virtual void LocateEffect();
	virtual void LocateSuperEffect();

	virtual const char *GetLastError() const;

protected:
	const char *_library;

	char _error[128];
	pugi::xml_document _doc;
	pugi::xml_node _currentShape;
	pugi::xml_node _currentEffect; // can be in root or in a folder
	pugi::xml_node _currentFolder;

	Effect *LoadEffect(pugi::xml_node &node, const std::list<AnimImage *> &sprites, Emitter *parent = NULL, const char *folderPath = "");
	Effect *LoadSuperEffect(pugi::xml_node &node, const std::list<AnimImage *> &sprites, Emitter *parent = NULL, const char *folderPath = "");
	void LoadAttributeNode(pugi::xml_node &node, AttributeNode *attr);
	Emitter *LoadEmitter(pugi::xml_node &node, const std::list<AnimImage *> &sprites, Effect *parent);
	AnimImage *GetSpriteInList(const std::list<AnimImage *> &sprites, int index) const;
};

} // namespace TLFX

#endif // _TLFX_EFFECTSLIBRARY_H
