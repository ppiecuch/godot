
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
