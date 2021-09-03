/*************************************************************************/
/*  generator.hpp                                                        */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2021 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2021 Godot Engine contributors (cf. AUTHORS.md).   */
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

// Copyright 2015 Markus Ilmola
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.

#ifndef GENERATOR_HPP
#define GENERATOR_HPP

#include "AnyGenerator.hpp"
#include "AnyMesh.hpp"
#include "AnyPath.hpp"
#include "AnyShape.hpp"
#include "Axis.hpp"
#include "AxisFlipMesh.hpp"
#include "AxisSwapMesh.hpp"
#include "AxisSwapPath.hpp"
#include "AxisSwapShape.hpp"
#include "BezierMesh.hpp"
#include "BezierShape.hpp"
#include "BoxMesh.hpp"
#include "CappedConeMesh.hpp"
#include "CappedCylinderMesh.hpp"
#include "CappedTubeMesh.hpp"
#include "CapsuleMesh.hpp"
#include "CircleShape.hpp"
#include "ConeMesh.hpp"
#include "ConvexPolygonMesh.hpp"
#include "CylinderMesh.hpp"
#include "DiskMesh.hpp"
#include "DodecahedronMesh.hpp"
#include "Edge.hpp"
#include "EmptyMesh.hpp"
#include "EmptyPath.hpp"
#include "EmptyShape.hpp"
#include "ExtrudeMesh.hpp"
#include "FlipMesh.hpp"
#include "FlipPath.hpp"
#include "FlipShape.hpp"
#include "GridShape.hpp"
#include "HelixPath.hpp"
#include "IcoSphereMesh.hpp"
#include "IcosahedronMesh.hpp"
#include "Iterator.hpp"
#include "KnotPath.hpp"
#include "LatheMesh.hpp"
#include "LinePath.hpp"
#include "LineShape.hpp"
#include "MergeMesh.hpp"
#include "MergePath.hpp"
#include "MergeShape.hpp"
#include "MeshVertex.hpp"
#include "MirrorMesh.hpp"
#include "ObjWriter.hpp"
#include "ParametricMesh.hpp"
#include "ParametricPath.hpp"
#include "ParametricShape.hpp"
#include "PathVertex.hpp"
#include "PlaneMesh.hpp"
#include "RectangleShape.hpp"
#include "RepeatMesh.hpp"
#include "RepeatPath.hpp"
#include "RepeatShape.hpp"
#include "RotateMesh.hpp"
#include "RotatePath.hpp"
#include "RotateShape.hpp"
#include "RoundedBoxMesh.hpp"
#include "RoundedRectangleShape.hpp"
#include "ScaleMesh.hpp"
#include "ScalePath.hpp"
#include "ScaleShape.hpp"
#include "ShapeToPath.hpp"
#include "ShapeVertex.hpp"
#include "SphereMesh.hpp"
#include "SphericalConeMesh.hpp"
#include "SphericalTriangleMesh.hpp"
#include "SpherifyMesh.hpp"
#include "SpringMesh.hpp"
#include "SubdivideMesh.hpp"
#include "SubdividePath.hpp"
#include "SubdivideShape.hpp"
#include "SvgWriter.hpp"
#include "TeapotMesh.hpp"
#include "TorusKnotMesh.hpp"
#include "TorusMesh.hpp"
#include "TransformMesh.hpp"
#include "TransformPath.hpp"
#include "TransformShape.hpp"
#include "TranslateMesh.hpp"
#include "TranslatePath.hpp"
#include "TranslateShape.hpp"
#include "Triangle.hpp"
#include "TriangleMesh.hpp"
#include "TubeMesh.hpp"
#include "UvFlipMesh.hpp"
#include "UvSwapMesh.hpp"
#include "math.hpp"
#include "utils.hpp"

#endif
