
// Copyright 2015 Markus Ilmola
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.

#include "generator/EmptyPath.hpp"

#include "common/gd_core.h"

using namespace generator;

Edge EmptyPath::Edges::generate() const {
	ERR_THROW_V(std::out_of_range("Called generate on an EmptyPath!"), Edge());
}

bool EmptyPath::Edges::done() const noexcept {
	return true;
}

void EmptyPath::Edges::next() {
	ERR_THROW(std::out_of_range("Called next on an EmptyPath!"));
}

EmptyPath::Edges::Edges() {}

PathVertex EmptyPath::Vertices::generate() const {
	ERR_THROW_V(std::out_of_range("Called generate on an EmptyPath!"), PathVertex());
}

bool EmptyPath::Vertices::done() const noexcept {
	return true;
}

void EmptyPath::Vertices::next() {
	ERR_THROW(std::out_of_range("Called next on an EmptyPath!"));
}

EmptyPath::Vertices::Vertices() {}

EmptyPath::EmptyPath() {}

EmptyPath::Edges EmptyPath::edges() const noexcept {
	return {};
}

EmptyPath::Vertices EmptyPath::vertices() const noexcept {
	return {};
}
