#ifndef LIBMAP_ENTITY_H
#define LIBMAP_ENTITY_H

#include "libmap/vector.h"

#include <stdlib.h>

typedef struct LMBrush LMBrush;

enum ENTITY_SPAWN_TYPE {
	EST_WORLDSPAWN = 0,
	EST_MERGE_WORLDSPAWN = 1,
	EST_ENTITY = 2,
	EST_GROUP = 3
};

typedef struct LMProperty {
	char *key = nullptr;
	char *value = nullptr;
} LMProperty;

class LMEntity {
public:
	int property_count = 0;
	LMProperty *properties = nullptr;

	int brush_count = 0;
	LMBrush *brushes = nullptr;

	vec3 center;
	enum ENTITY_SPAWN_TYPE spawn_type;
};

#endif // LIBMAP_ENTITY_H
