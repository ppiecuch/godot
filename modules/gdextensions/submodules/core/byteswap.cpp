/*************************************************************************/
/*  byteswap.cpp                                                         */
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

#include "byteswap.h"

Byteswap::Byteswap() {
	std::cout << "Byteswapper instantiated!" << std::endl;
}

float Byteswap::reverseFloat(float inFloat) {
	float retVal;
	char *floatToConvert = (char *)&inFloat;
	char *returnFloat = (char *)&retVal;

	// swap the bytes into a temporary buffer
	returnFloat[0] = floatToConvert[3];
	returnFloat[1] = floatToConvert[2];
	returnFloat[2] = floatToConvert[1];
	returnFloat[3] = floatToConvert[0];
	return retVal;
}

int Byteswap::reverseInt(int inInt) {
	int retVal;
	char *intToConvert = (char *)&inInt;
	char *returnInt = (char *)&retVal;

	// swap the bytes into a temporary buffer
	returnInt[0] = intToConvert[3];
	returnInt[1] = intToConvert[2];
	returnInt[2] = intToConvert[1];
	returnInt[3] = intToConvert[0];
	return retVal;
};

void Byteswap::_bind_methods() {
	ClassDB::bind_method("reverseFloat", &Byteswap::reverseFloat);
	ClassDB::bind_method("reverseInt", &Byteswap::reverseInt);
}
