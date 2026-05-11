// MPM Fluid Simulation
//
// OpenFrameworks version by Golan Levin — http://www.flong.com
// ofxAddon by James George (@obviousjm) — http://www.jamesgeorge.org
//
// Original Java version: http://grantkot.com/MPM/Liquid.html
// Flash version: Copyright iunpin, MIT License
// Javascript version: Copyright Stephen Sinclair (radarsat1), MIT License

#ifndef MPMFLUID_H
#define MPMFLUID_H

#include "core/math/math_defs.h"

#include <vector>

namespace mpm {

namespace fluid {

class MPMNode {
public:
	real_t m;
	real_t d;
	real_t gx, gy;
	real_t u, v;
	real_t ax, ay;
	bool active;

	void clear() {
		m = d = gx = gy = u = v = ax = ay = 0;
		active = false;
	}

	MPMNode() {
		m = d = 0;
		gx = gy = 0;
		u = v = 0;
		ax = ay = 0;
		active = false;
	}
};

class MPMObstacle {
public:
	real_t cx, cy;
	real_t radius, radius2;

	MPMObstacle(real_t inx, real_t iny, real_t inr) {
		cx = inx;
		cy = iny;
		radius = inr;
		radius2 = radius * radius;
	}
};

class MPMParticle {
public:
	real_t x, y;
	real_t u, v;
	real_t pu, pv;
	real_t d;

	int cx, cy;

	real_t gu, gv;
	real_t T00, T01, T11;

	real_t px[3];
	real_t py[3];
	real_t gx[3];
	real_t gy[3];

	MPMParticle(real_t inx, real_t iny, real_t inu, real_t inv) {
		x = inx;
		y = iny;
		u = inu;
		v = inv;
		pu = pv = 0;
		cx = cy = 0;
		gu = gv = 0;
		d = 0;
		T00 = T01 = T11 = 0;
		for (int i = 0; i < 3; i++) { px[i] = py[i] = gx[i] = gy[i] = 0; }
	}
};

class MPMFluid {
protected:
	std::vector<MPMParticle *> particles;
	int maxNumParticles;
	std::vector<std::vector<MPMNode *>> grid;
	std::vector<MPMNode *> activeNodes;
	int numActiveNodes;

	std::vector<MPMObstacle *> obstacles;

public:
	void setup(int maxParticles);
	void update();

	// Apply a velocity impulse at (cx, cy) in grid coordinates, blending toward (dx, dy).
	void applyImpulse(real_t cx, real_t cy, real_t dx, real_t dy, real_t radius = 10.0f);

	int getGridSizeX();
	int getGridSizeY();

	real_t scaleFactor;
	int numParticles;
	real_t densitySetting;
	real_t stiffness;
	real_t bulkViscosity;
	real_t elasticity;
	real_t viscosity;
	real_t yieldRate;
	bool bGradient;
	real_t gravity;
	bool bDoObstacles;
	real_t smoothing;

	std::vector<MPMParticle *> &getParticles();

	MPMFluid();
};

} // namespace fluid

} // namespace mpm

#endif // MPMFLUID_H
