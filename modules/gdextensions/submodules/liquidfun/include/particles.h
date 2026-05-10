#ifndef BOX2D_PARTICLES_H
#define BOX2D_PARTICLES_H

#include "core/object.h"

class ParticleSystemB2 : public Object {
	GDCLASS(ParticleSystemB2, Object);
	BOX2D_CLASS(ParticleSystem);

protected:
	static void _bind_methods();
};

#endif // BOX2D_PARTICLES_H
