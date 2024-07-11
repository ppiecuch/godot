/**************************************************************************/
/*  procrockgen.cpp                                                       */
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

#ifdef TOOLS_ENABLED
#include "demo.h"
#endif

#include <procrocklib/gen/icosahedron_generator.h>
#include <procrocklib/mod/subdivision_modifier.h>
#include <procrocklib/par/xatlas_parameterizer.h>
#include <procrocklib/texgen/noise_texture_generator.h>

#include "abstracted_pipeline/abstracted_pipeline_factory.h"
#include "abstracted_pipeline/igneous_pipeline.h"
#include "abstracted_pipeline/metamorphic_pipeline.h"
#include "abstracted_pipeline/sedimentary_pipeline.h"

#include "common/gd_core.h"

#include <memory>

using namespace procrock;

Pipeline *create_pipeline() {
	Pipeline *pipeline = memnew(Pipeline);
	pipeline->setGenerator(std::make_unique<IcosahedronGenerator>());
	pipeline->addModifier(std::make_unique<SubdivisionModifier>());
	pipeline->setParameterizer(std::make_unique<XAtlasParameterizer>());
	pipeline->setTextureGenerator(std::make_unique<NoiseTextureGenerator>());
	return pipeline;
}

AbstractedPipeline *create_abstracted_pipeline(int p_kind) {
	if (auto p = createAbstractPipelineFromId(p_kind)) {
		return p.release();
	}
	return nullptr;
}
