
// Copyright 2015 Markus Ilmola
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.

#ifndef UUID_D7746018E901B4EE35CEDC635307152F
#define UUID_D7746018E901B4EE35CEDC635307152F

#include "Edge.hpp"
#include "LineShape.hpp"
#include "MergeShape.hpp"
#include "RepeatShape.hpp"
#include "ShapeVertex.hpp"

namespace generator {

/**
 * A 2d regular grid.
 */
class GridShape {
private:
	using Impl = MergeShape<RepeatShape<LineShape>, RepeatShape<LineShape>>;

	Impl mImpl;

public:
	using Edges = Impl::Edges;

	using Vertices = Impl::Vertices;

	/// @param size A half of the side length of the grid.
	/// @param segments The Number of cells in the grid.
	/// If <1 an empty shape results.
	/// @param subSegments The number of segment along each cell edge.
	/// If <1 an empty shape results.
	explicit GridShape(
			const gml::dvec2 &size = { 1.0, 1.0 },
			const gml::ivec2 &segments = { 4, 4 },
			const gml::ivec2 &subSegments = { 2, 2 }) noexcept;

	Edges edges() const noexcept;

	Vertices vertices() const noexcept;
};

} // namespace generator

#endif
