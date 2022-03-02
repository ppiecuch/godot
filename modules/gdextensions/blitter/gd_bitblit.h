#ifndef GD_BITBLIT_H
#define GD_BITBLIT_H

#include "core/object.h"

class BitBlit : public Object {
	GDCLASS(BitBlit, Object)

	static BitBlit *singleton;

protected:
	static void _bind_methods();

public:
	static BitBlit *get_singleton() { return singleton; }

	BitBlit();
	~BitBlit();
};

#endif // GD_BITBLIT_H
