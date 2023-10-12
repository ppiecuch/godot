/**************************************************************************/
/*  ddls_fwd.h                                                            */
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

#pragma once

#include "core/reference.h"

/// Forward declarations

#define DDLS_FWD_ENTITY_DEFINITION(E) \
	class DDLS_##E;                   \
	typedef Ref<DDLS_##E> DDLS##E

// Data

DDLS_FWD_ENTITY_DEFINITION(ConstraintSegment);
DDLS_FWD_ENTITY_DEFINITION(ConstraintShape);
DDLS_FWD_ENTITY_DEFINITION(Face);
DDLS_FWD_ENTITY_DEFINITION(Edge);
DDLS_FWD_ENTITY_DEFINITION(Object);
DDLS_FWD_ENTITY_DEFINITION(Vertex);
DDLS_FWD_ENTITY_DEFINITION(Mesh);
DDLS_FWD_ENTITY_DEFINITION(GraphEdge);
DDLS_FWD_ENTITY_DEFINITION(GraphNode);
DDLS_FWD_ENTITY_DEFINITION(Graph);

// Ai

DDLS_FWD_ENTITY_DEFINITION(EntityAI);
DDLS_FWD_ENTITY_DEFINITION(PathFinder);
DDLS_FWD_ENTITY_DEFINITION(AStar);
DDLS_FWD_ENTITY_DEFINITION(Funnel);
DDLS_FWD_ENTITY_DEFINITION(FieldOfView);
DDLS_FWD_ENTITY_DEFINITION(LinearPathSampler);
