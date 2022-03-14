
// Copyright 2015 Markus Ilmola
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.

#include "generator/EmptyShape.hpp"

#include <stdexcept>

#include "common/gd_core.h"

using namespace generator;

EmptyShape::Edges::Edges() {}

Edge EmptyShape::Edges::generate() const {
	ERR_THROW_V(std::out_of_range("Called generate on an EmptyShape!"), Edge());
}

bool EmptyShape::Edges::done() const noexcept {
	return true;
}

void EmptyShape::Edges::next() {
	ERR_THROW(std::out_of_range("Called next on an EmptyShape!"));
}

EmptyShape::Vertices::Vertices() {}

ShapeVertex EmptyShape::Vertices::generate() const {
	ERR_THROW_V(std::out_of_range("Called generate on an EmptyShape!"), ShapeVertex());
}

bool EmptyShape::Vertices::done() const noexcept {
	return true;
}

void EmptyShape::Vertices::next() {
	ERR_THROW(std::out_of_range("Called next on an EmptyShape!"));
}

EmptyShape::Edges EmptyShape::edges() const noexcept {
	return {};
}

EmptyShape::Vertices EmptyShape::vertices() const noexcept {
	return {};
}
