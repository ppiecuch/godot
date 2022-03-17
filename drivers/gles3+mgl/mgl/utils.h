/*
 * Copyright (C) Michael Larson on 1/6/2022
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * utils.h
 * MGL
 *
 */

#ifndef utils_h
#define utils_h

#include "GL/glcorearb.h"

#include <algorithm>
#include <limits>

template <typename T>
inline constexpr bool is_pow2(T x) {
	static_assert(std::is_integral<T>::value, "is_pow2 must be called on an integer type.");
	return (x & (x - 1)) == 0 && (x != 0);
}

template <typename T>
T round_up(const T value, const T alignment) {
	auto temp = value + alignment - static_cast<T>(1);
	return temp - temp % alignment;
}

template <typename T>
constexpr T round_up_pow2(const T value, const T alignment) {
	static_assert(is_pow2(alignment), "is_pow2 returns 0");
	return (value + alignment - 1) & ~(alignment - 1);
}

GLuint maxLevels(GLuint width, GLuint height, GLuint depth);
GLboolean checkMaxLevels(GLuint levels, GLuint width, GLuint height, GLuint depth);

#endif /* utils_h */
