#ifndef ROCKGEN_H
#define ROCKGEN_H

#include "core/variant.h"

Array rock_gen(int depth = 3, int randseed = 0, real_t smoothness = 1, bool smoothed = false); // main generator

#endif // ROCKGEN_H
