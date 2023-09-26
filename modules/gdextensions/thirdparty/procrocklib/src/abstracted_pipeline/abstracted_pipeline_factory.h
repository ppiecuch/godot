#pragma once

#include <memory>

#include "abstracted_pipeline.h"

#include "core/error_macros.h"
#include "common/gd_core.h"

namespace procrock {
std::unique_ptr<AbstractedPipeline> inline createAbstractPipelineFromId(unsigned int id) {
  switch (id) {
    case AbstractedPipeline_Igneous:
      return std::make_unique<IgneousPipeline>();
    case AbstractedPipeline_Sedimentary:
      return std::make_unique<SedimentaryPipeline>();
    case AbstractedPipeline_Metamorphic:
      return std::make_unique<MetamorphicPipeline>();
    default:
      CRASH_NOW_MSG("make sure all abstract pipeline types are handled!");
  }

  return nullptr;
}
}  // namespace procrock
