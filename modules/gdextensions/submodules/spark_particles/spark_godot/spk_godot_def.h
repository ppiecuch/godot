/*************************************************************************/
/*  spk_godot_def.h                                                      */
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

#pragma once

#ifndef SPK_GODOT_DEF_H
#define SPK_GODOT_DEF_H

#include "core/color.h"
#include "core/math/vector3.h"

// for windows platform only
#ifdef _MSC_VER
#pragma warning(disable : 4275) // disables the warning about exporting DLL classes children of non DLL classes
#endif

namespace Godot {

class Vector3D;
class Color;

//////////////////////////
// Conversion functions //
//////////////////////////

/**
 * @brief Converts a SPARK Vector3D to an Godot Vector3
 * @param v : the Vector3D to convert
 * @return the Godot Vector3
 */
inline Vector3 spk2godot(const SPK::Vector3D &v);

/**
 * @brief Converts an Godot Vector3 to a SPARK Vector3D
 * @param v : the Vector3 to convert
 * @return the SPARK Vector3D
 */
inline SPK::Vector3D godot2spk(const Vector3 &v);

/**
 * @brief Gets an Godot Color from rgba values
 * @param a : the alpha value
 * @param r : the red value
 * @param g : the green value
 * @param b : the blue value
 * @return the Godot Color
 */
inline Color spk2godot(unsigned char a, unsigned char r, unsigned char g, unsigned char b);

/**
 * @brief Gets an Godot Color from SPK::Color values
 * @param a : the SPK::Color value
 * @return the Godot Color
 */
inline const Color spk2godot(SPK::Color c);

} // namespace Godot

#endif // SPK_GODOT_DEF_H
