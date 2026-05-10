/**************************************************************************/
/*  goost.h                                                               */
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

#include "core/command_line_parser.h"
#include "core/goost_engine.h"
#include "core/image/goost_image.h"
#include "core/image/goost_image_bind.h"
#include "core/image/image_blender.h"
#include "core/image/image_indexed.h"
#include "core/invoke_state.h"
#include "core/math/geometry/2d/goost_geometry_2d.h"
#include "core/math/geometry/2d/goost_geometry_2d_bind.h"
#include "core/math/geometry/2d/poly/boolean/poly_boolean.h"
#include "core/math/geometry/2d/poly/decomp/poly_decomp.h"
#include "core/math/geometry/2d/poly/offset/poly_offset.h"
#include "core/math/geometry/2d/poly/poly_backends.h"
#include "core/math/geometry/2d/random_2d.h"
#include "core/math/random.h"
#include "core/script/mixin_script/mixin_script.h"
#include "core/types/data_container.h"
#include "core/types/graph.h"
#include "core/types/linked_list.h"
#include "core/types/map_2d.h"

#include "scene/2d/editor/poly_node_2d_editor_plugin.h"
#include "scene/2d/editor/visual_shape_2d_editor_plugin.h"
#include "scene/2d/poly_generators_2d.h"
#include "scene/2d/poly_shape_2d.h"
#include "scene/2d/visual_shape_2d.h"
#include "scene/gui/grid_rect.h"
#include "scene/main/stopwatch.h"
#include "scene/physics/2d/poly_collision_shape_2d.h"
#include "scene/physics/2d/shape_cast_2d.h"
#include "scene/resources/light_texture.h"
