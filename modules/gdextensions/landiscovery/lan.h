#include "core/object.h"

class GdLanAdvertiser : public Object {
	GDCLASS(GdLanAdvertiser, Object);
public:
	GdLanAdvertiser();
};

class GdLanListener : public Object {
	GDCLASS(GdLanListener, Object);
public:
	GdLanListener();
};
