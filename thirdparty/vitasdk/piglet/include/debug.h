//
// Copyright (c) 2020 by SonicMastr <sonicmastr@gmail.com>
//
// This file is part of Pigs In A Blanket
//
// Pigs in a Blanket is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Lesser General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//

#pragma once

#ifndef DEBUG_H_
#define DEBUG_H_

#ifdef DEBUG
#define LOG(...) printf(__VA_ARGS__)
#else
#define LOG(...)
#endif

#endif /* DEBUG_H_ */
