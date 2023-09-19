/**************************************************************************/
/*  imath.h                                                               */
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

#ifndef IMATHS_H
#define IMATHS_H

#include "core/math/math_defs.h"

typedef unsigned int uint_t;
typedef unsigned char uchar_t;

// Integer math routines

uint_t CompactLog2(uint_t i);
uint_t Log2(uint_t i);
uint_t Log10(uint_t i);
uint_t FloorSqrt(uint_t x);
uint_t Sqrt(uint_t i);
uint_t Abs(int i);
int Sign(int i);

int Ror(int i, int p);
int Rol(int i, int p);

int BitReverse(int i, int l);

void DivMod(int a, int b, int &q, int &r);
void UDivMod(uint_t a, uint_t b, uint_t &q, uint_t &r);

const uint_t FastDivTest = 223092870; // 2*3*5*7*11*13*17*19*23
const real_t GoldenRatio = 1.6180339887; // (1+Sqrt(5)) / 2
const real_t InverseGoldenRatio = 0.6180339887;

uint_t pgfc(uint_t a, uint_t b);
void SimplifyFraction(int &a, int &b);
uint_t ExpoDiscrete(uint_t a, uint_t p);
uint_t ModExpoDiscrete(uint_t a, uint_t p, uint_t m);
void Factorise(uint_t i, uint_t facteurs[], int &factptr);
void FacteursPropres(uint_t i, uint_t **facteursPropres, int &factptr);
int Premier(uint_t i);
int RelativementPremier(uint_t a, uint_t b);
uint_t RelativementPremierSqrt(uint_t a);
uint_t RelativementPremierPhi(uint_t a);
uint_t GenerateurStochastique(uint_t q);
uint_t PlusPetitGenerateur(uint_t q);
uint_t NiemeRacineUniteStochastique(uint_t q, uint_t m);
uint_t PlusPetiteNiemeRacineUnite(uint_t q, uint_t m);

#include "imath.inl"

#endif // IMATHS_H
