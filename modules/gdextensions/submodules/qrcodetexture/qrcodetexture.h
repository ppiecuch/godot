/*************************************************************************/
/*  qrcodetexture.h                                                      */
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

#ifndef QRCODETEXTURE_H
#define QRCODETEXTURE_H

#include "qrcodegen.hpp"
#include "scene/resources/texture.h"
using qrcodegen::QrCode;

class QRCodeTexture : public ImageTexture {
	GDCLASS(QRCodeTexture, ImageTexture);

protected:
	static void _bind_methods();
	String content = "";
	int border = 1;
	Color color = Color(0, 0, 0);
	void update_qrcode();

public:
	// Error correction level : from 1 = lowest to 4 = highest
	int error_correction_level = 1;
	Ref<Image> GetImage(String p_string, const int p_border = 1, Color p_color = Color(0, 0, 0));
	void set_content(const String &p_string);
	String get_content() const;
	void set_border(const int &p_border);
	void set_color(const Color &p_color);
	Color get_color() const;
	int get_border() const;
	void set_ecl(const int &p_ecl);
	int get_ecl() const;
	QRCodeTexture();
};

#endif // QRCODETEXTURE_H
