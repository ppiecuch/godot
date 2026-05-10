/**************************************************************************/
/*  GRResources.h                                                         */
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

/* GRBinResources.h */
#ifndef NO_GODOTREMOTE_DEFAULT_RESOURCES

#ifndef GRRESOURCES_H
#define GRRESOURCES_H

#define GetPoolVectorFromBin(to_var, res)         \
	PoolByteArray to_var;                         \
	to_var.resize(res##_size);                    \
	auto to_var##write = to_var.write();          \
	memcpy(to_var##write.ptr(), res, res##_size); \
	to_var##write.release()

namespace GRResources {

extern const char *Txt_CRT_Shader;

extern const unsigned int Bin_NoSignalPNG_size;
extern const unsigned char Bin_NoSignalPNG[];

extern const unsigned int Bin_NoSignalVerticalPNG_size;
extern const unsigned char Bin_NoSignalVerticalPNG[];

// NOTIFICATION ICONS

extern const unsigned int Bin_CloseIconPNG_size;
extern const unsigned char Bin_CloseIconPNG[];

extern const unsigned int Bin_ConnectedIconPNG_size;
extern const unsigned char Bin_ConnectedIconPNG[];

extern const unsigned int Bin_DisconnectedIconPNG_size;
extern const unsigned char Bin_DisconnectedIconPNG[];

extern const unsigned int Bin_ErrorIconPNG_size;
extern const unsigned char Bin_ErrorIconPNG[];

extern const unsigned int Bin_WarningIconPNG_size;
extern const unsigned char Bin_WarningIconPNG[];

} // namespace GRResources

#endif // !GRRESOURCES_H

#endif
