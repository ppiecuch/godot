#include "mpmfluid.h"

#include "core/2d/core_item.h"
#include "core/math/vector2.h"

// Demo controls:
// --------------
// gui.addSlider("# Particles",    "N_PARTICLES",    100000/4, 1000, 100000, true);
// gui.addSlider("Density",        "DENSITY",        5.0,      0,    30.0,   false);
// gui.addSlider("Stiffness",      "STIFFNESS",      0.5,      0,    2.0,    false);
// gui.addSlider("Bulk Viscosity", "BULK_VISCOSITY", 3.0,      0,    10.0,   false);
// gui.addSlider("Elasticity",     "ELASTICITY",     1.0,      0,    4.0,    false);
// gui.addSlider("Viscosity",      "VISCOSITY",      1.0,      0,    4.0,    false);
// gui.addSlider("Yield Rate",     "YIELD_RATE",     1.0,      0,    2.0,    false);
// gui.addSlider("Gravity",        "GRAVITY",        0.002,    0,    0.02,   false);
// gui.addSlider("Smoothing",      "SMOOTHING",      1.0,      0,    3.0,    false);
// gui.addToggle("Do Obstacles?",  "DO_OBSTACLES",   true);

// Now, this horizontal gradient in the Density parameter is just for yuks.
// It demonstrates that spatial variations in the Density parameter can yield interesting results.
// For an interesting experiment, try making Density proportional to the luminance of a photograph.

// gui.addToggle("Horizontal Density Gradient?", "DENSITY_GRADIENT", false);
// -----

//TODO make varying
#define gridSizeX 160
#define gridSizeY 120

MPMFluid::MPMFluid() :
		densitySetting(5.0),
		stiffness(.5),
		bulkViscosity(3.0),
		elasticity(1.0),
		viscosity(1.0),
		yieldRate(1.0),
		gravity(.002),
		bGradient(false),
		bDoObstacles(true),
		elapsed(0.0),
		scaleFactor(1.0),
		smoothing(1.0) {
	//
}

void MPMFluid::setup(int maxParticles) {
	maxNumParticles = maxParticles;

	// This creates a 2-dimensional array (i.e. grid) of Node objects.
	for (int i = 0; i < gridSizeX; i++) {
		grid.push_back(std::vector<MPMNode *>());
		for (int j = 0; j < gridSizeY; j++) {
			grid[i].push_back(new MPMNode());
		}
	}

	for (int i = 0; i < (gridSizeX * gridSizeY); i++) {
		activeNodes.push_back(new MPMNode());
	}

	for (int i = 0; i < maxParticles; i++) {
		int x0 = 5;
		int x1 = gridSizeX - 5;
		real_t rx = ofRandom(x0, x1);
		real_t ry = ofRandom(5, gridSizeY / 5);
		particles.push_back(new MPMParticle(rx, ry, 0.0, 0.0));
	}

	//TODO: JG add and remove obistacles through API
	obstacles.push_back(new MPMObstacle(gridSizeX * 0.75, gridSizeY * 0.75, gridSizeX * 0.075));
}

void MPMFluid::update() {
	numParticles = MIN(numParticles, maxNumParticles); // Important: can't exceed maxNParticles!

	// Clear the grid. Necessary to begin the simulation.
	for (int i = 0; i < gridSizeX; i++) {
		for (int j = 0; j < gridSizeY; j++) {
			grid[i][j]->clear();
		}
	}

	numActiveNodes = 0;

	long t0 = ofGetElapsedTimeMillis();

	// -- Particles pass 1
	real_t phi;
	int pcxTmp, pcyTmp;
	for (int ip = 0; ip < numParticles; ip++) {
		MPMParticle *p = particles[ip];

		int pcx = p->cx = (int)(p->x - 0.5);
		int pcy = p->cy = (int)(p->y - 0.5);

		real_t *px = p->px;
		real_t *py = p->py;
		real_t *gx = p->gx;
		real_t *gy = p->gy;
		real_t pu = p->u;
		real_t pv = p->v;
		p->pu = pu;
		p->pv = pv;

		// N.B.: The constants below are not playthings.
		real_t x = (real_t)p->cx - p->x;
		px[0] = (0.5 * x * x + 1.5 * x) + 1.125;
		gx[0] = x + 1.5;
		x++;
		px[1] = -x * x + 0.75;
		gx[1] = -2 * x;
		x++;
		px[2] = (0.5 * x * x - 1.5 * x) + 1.125;
		gx[2] = x - 1.5;

		real_t y = (real_t)p->cy - p->y;
		py[0] = (0.5 * y * y + 1.5 * y) + 1.125;
		gy[0] = y + 1.5;
		y++;
		py[1] = -y * y + 0.75;
		gy[1] = -2 * y;
		y++;
		py[2] = (0.5 * y * y - 1.5 * y) + 1.125;
		gy[2] = y - 1.5;

		int pcxi, pcyj;
		for (int i = 0; i < 3; i++) {
			pcxi = pcx + i;

			if ((pcxi >= 0) && (pcxi < gridSizeX)) {
				std::vector<MPMNode *> &nrow = grid[pcxi]; // potential for array index out of bounds here if simulation explodes.
				real_t pxi = px[i];
				real_t gxi = gx[i];

				for (int j = 0; j < 3; j++) {
					pcyj = pcy + j;

					if ((pcyj >= 0) && (pcyj < gridSizeY)) {
						MPMNode *n = nrow[pcyj]; // potential for array index out of bounds here if simulation explodes.

						if (!n->active) {
							n->active = true;
							activeNodes[numActiveNodes] = n;
							numActiveNodes++;
						}
						phi = pxi * py[j];
						n->m += phi;
						n->gx += gxi * py[j];
						n->gy += pxi * gy[j];
						n->u += phi * pu;
						n->v += phi * pv;
					}
				}
			}
		}
	}

	for (int ni = 0; ni < numActiveNodes; ni++) {
		MPMNode *n = activeNodes[ni];
		if (n->m > 0) {
			n->u /= n->m;
			n->v /= n->m;
		}
	}

	long t1 = ofGetElapsedTimeMillis();

	// -- Particles pass 2
	real_t stiffnessBulk = stiffness * bulkViscosity;
	int nBounced = 0;

	for (int ip = 0; ip < numParticles; ip++) {
		MPMParticle *p = particles[ip];
		real_t *px = p->px;
		real_t *py = p->py;
		real_t *gx = p->gx;
		real_t *gy = p->gy;
		int pcy = p->cy;
		int pcx = p->cx;

		real_t dudx = 0.0F;
		real_t dudy = 0.0F;
		real_t dvdx = 0.0F;
		real_t dvdy = 0.0F;

		real_t gxi, pxi;
		real_t gxf, gyf;

		int pcxi;
		for (int i = 0; i < 3; i++) {
			std::vector<MPMNode *> &nrow = grid[pcx + i];
			gxi = gx[i];
			pxi = px[i];

			for (int j = 0; j < 3; j++) {
				MPMNode *nj = nrow[pcy + j];
				gxf = gxi * py[j];
				gyf = pxi * gy[j];
				dudx += nj->u * gxf;
				dudy += nj->u * gyf;
				dvdx += nj->v * gxf;
				dvdy += nj->v * gyf;
			}
		}

		real_t w1 = dudy - dvdx;
		real_t wT0 = w1 * p->T01;
		real_t wT1 = 0.5 * w1 * (p->T00 - p->T11);
		real_t D00 = dudx;
		real_t D01 = 0.5 * (dudy + dvdx);
		real_t D11 = dvdy;
		real_t trace = 0.5 * (D00 + D11);
		D00 -= trace;
		D11 -= trace;

		p->T00 += (-wT0 + D00) - yieldRate * p->T00;
		p->T01 += (wT1 + D01) - yieldRate * p->T01;
		p->T11 += (wT0 + D11) - yieldRate * p->T11;

		// here's our protection against exploding simulations...
		real_t norma = p->T00 * p->T00 + 2 * p->T01 * p->T01 + p->T11 * p->T11;
		if (norma > 10) {
			p->T00 = p->T01 = p->T11 = 0;
		}

		int cx0 = (int)p->x;
		int cy0 = (int)p->y;
		int cx1 = cx0 + 1;
		int cy1 = cy0 + 1;
		MPMNode *n00 = grid[cx0][cy0];
		MPMNode *n01 = grid[cx0][cy1];
		MPMNode *n10 = grid[cx1][cy0];
		MPMNode *n11 = grid[cx1][cy1];

		real_t p00 = n00->m;
		real_t x00 = n00->gx;
		real_t y00 = n00->gy;
		real_t p01 = n01->m;
		real_t x01 = n01->gx;
		real_t y01 = n01->gy;
		real_t p10 = n10->m;
		real_t x10 = n10->gx;
		real_t y10 = n10->gy;
		real_t p11 = n11->m;
		real_t x11 = n11->gx;
		real_t y11 = n11->gy;

		real_t pdx = p10 - p00;
		real_t pdy = p01 - p00;
		real_t C20 = 3 * pdx - x10 - 2 * x00;
		real_t C02 = 3 * pdy - y01 - 2 * y00;
		real_t C30 = -2 * pdx + x10 + x00;
		real_t C03 = -2 * pdy + y01 + y00;
		real_t csum1 = p00 + y00 + C02 + C03;
		real_t csum2 = p00 + x00 + C20 + C30;
		real_t C21 = 3 * p11 - 2 * x01 - x11 - 3 * csum1 - C20;
		real_t C31 = (-2 * p11 + x01 + x11 + 2 * csum1) - C30;
		real_t C12 = 3 * p11 - 2 * y10 - y11 - 3 * csum2 - C02;
		real_t C13 = (-2 * p11 + y10 + y11 + 2 * csum2) - C03;
		real_t C11 = x01 - C13 - C12 - x00;

		real_t u1 = p->x - (real_t)cx0;
		real_t u2 = u1 * u1;
		real_t u3 = u1 * u2;
		real_t v1 = p->y - (real_t)cy0;
		real_t v2 = v1 * v1;
		real_t v3 = v1 * v2;
		real_t density =
				p00 +
				x00 * u1 +
				y00 * v1 +
				C20 * u2 +
				C02 * v2 +
				C30 * u3 +
				C03 * v3 +
				C21 * u2 * v1 +
				C31 * u3 * v1 +
				C12 * u1 * v2 +
				C13 * u1 * v3 +
				C11 * u1 * v1;

		real_t DS = densitySetting;
		if (bGradient) {
			// Just for yuks, a spatially varying density function
			DS = densitySetting * (Math::pow(p->x / (real_t)gridSizeX, 4.0));
		}

		real_t pressure = (stiffness / MAX(1.0, DS)) * (density - DS);
		if (pressure > 2) {
			pressure = 2;
		}

		p->d = 1.0 / MAX(0.001, density);

		// COLLISIONS-1
		// Determine if there has been a collision with the wall.
		real_t fx = 0.0F;
		real_t fy = 0.0F;
		bool bounced = false;

		if (p->x < 3.0F) {
			fx += 3.0F - p->x;
			bounced = true;
		} else if (p->x > (real_t)(gridSizeX - 3)) {
			fx += (gridSizeX - 3.0) - p->x;
			bounced = true;
		}

		if (p->y < 3) {
			fy += 3 - p->y;
			bounced = true;
		} else if (p->y > (real_t)(gridSizeY - 3)) {
			fy += (gridSizeY - 3) - p->y;
			bounced = true;
		}

		// Interact with a simple demonstration obstacle.
		// Note: an accurate obstacle implementation would also need to implement
		// some velocity fiddling as in the section labeled "COLLISIONS-2" below.
		// Otherwise, this obstacle is "soft"; particles can enter it slightly.
		if (bDoObstacles && obstacles.size() > 0) {
			// circular obstacle
			real_t oR = obstacles[0]->radius;
			real_t oR2 = obstacles[0]->radius2;
			real_t odx = obstacles[0]->cx - p->x;
			real_t ody = obstacles[0]->cy - p->y;
			real_t oD2 = odx * odx + ody * ody;
			if (oD2 < oR2) {
				real_t oD = sql::sqrtf(oD2);
				real_t dR = oR - oD;
				fx -= dR * (odx / oD);
				fy -= dR * (ody / oD);
				bounced = true;
			}
		}

		trace *= stiffnessBulk;
		real_t T00 = elasticity * p->T00 + viscosity * D00 + pressure + trace;
		real_t T01 = elasticity * p->T01 + viscosity * D01;
		real_t T11 = elasticity * p->T11 + viscosity * D11 + pressure + trace;
		real_t dx, dy;

		if (bounced) {
			for (int i = 0; i < 3; i++) {
				std::vector<MPMNode *> &nrow = grid[pcx + i];
				real_t ppxi = px[i];
				real_t pgxi = gx[i];

				for (int j = 0; j < 3; j++) {
					MPMNode *nj = nrow[pcy + j];
					phi = ppxi * py[j];
					dx = pgxi * py[j];
					dy = ppxi * gy[j];
					nj->ax += fx * phi - (dx * T00 + dy * T01);
					nj->ay += fy * phi - (dx * T01 + dy * T11);
				}
			}

		} else {
			real_t *pppxi = &px[0];
			real_t *ppgxi = &gx[0];

			for (int i = 0; i < 3; i++) {
				std::vector<MPMNode *> &nrow = grid[pcx + i];

				real_t ppxi = *(pppxi++); //px[i];
				real_t pgxi = *(ppgxi++); //gx[i];
				for (int j = 0; j < 3; j++) {
					MPMNode *nj = nrow[pcy + j];
					dx = pgxi * py[j];
					dy = ppxi * gy[j];
					nj->ax -= (dx * T00 + dy * T01);
					nj->ay -= (dx * T01 + dy * T11);
				}
			}
		}
	}

	for (int ni = 0; ni < numActiveNodes; ni++) {
		MPMNode *n = activeNodes[ni];
		if (n->m > 0) {
			n->ax /= n->m;
			n->ay /= n->m;
			n->u = 0;
			n->v = 0;
		}
	}

	long t2 = ofGetElapsedTimeMillis();

	// -- Particles pass 3
	const real_t rightEdge = gridSizeX - 3;
	const real_t bottomEdge = gridSizeY - 3;

	for (int ip = 0; ip < numParticles; ip++) {
		MPMParticle *p = particles[ip];

		real_t *px = p->px;
		real_t *py = p->py;
		int pcy = p->cy;
		int pcx = p->cx;
		for (int i = 0; i < 3; i++) {
			std::vector<MPMNode *> &nrow = grid[pcx + i];
			real_t ppxi = px[i];
			for (int j = 0; j < 3; j++) {
				ofxMPMNode *nj = nrow[pcy + j];
				phi = ppxi * py[j];
				p->u += phi * nj->ax;
				p->v += phi * nj->ay;
			}
		}

		p->v += gravity;
		if (ofGetMousePressed(0)) {
			real_t vx = Math::abs(p->x - ofGetMouseX() / scaleFactor);
			real_t vy = Math::abs(p->y - ofGetMouseY() / scaleFactor);
			real_t mdx = (ofGetMouseX() - ofGetPreviousMouseX()) / scaleFactor;
			real_t mdy = (ofGetMouseY() - ofGetPreviousMouseY()) / scaleFactor;
			if (vx < 10 && vy < 10) {
				real_t weight = (1 - vx / 10) * (1 - vy / 10);
				p->u += weight * (mdx - p->u);
				p->v += weight * (mdy - p->v);
			}
		}

		// COLLISIONS-2
		// Plus, an opportunity to add randomness when accounting for wall collisions.
		real_t xf = p->x + p->u;
		real_t yf = p->y + p->v;
		real_t wallBounceMaxRandomness = 0.03;
		if (xf < 2) {
			p->u += (2 - xf) + ofRandom(wallBounceMaxRandomness);
		} else if (xf > rightEdge) {
			p->u += rightEdge - xf - ofRandom(wallBounceMaxRandomness);
		}
		if (yf < 2) {
			p->v += (2 - yf) + ofRandom(wallBounceMaxRandomness);
		} else if (yf > bottomEdge) {
			p->v += bottomEdge - yf - ofRandom(wallBounceMaxRandomness);
		}

		real_t pu = p->u;
		real_t pv = p->v;
		for (int i = 0; i < 3; i++) {
			std::vector<MPMNode *> &nrow = grid[pcx + i];
			real_t ppxi = px[i];
			for (int j = 0; j < 3; j++) {
				ofxMPMNode *nj = nrow[pcy + j];
				phi = ppxi * py[j];
				nj->u += phi * pu;
				nj->v += phi * pv;
			}
		}
	}

	for (int ni = 0; ni < numActiveNodes; ni++) {
		MPMNode *n = activeNodes[ni];
		if (n->m > 0) {
			n->u /= n->m;
			n->v /= n->m;
		}
	}

	long t3 = ofGetElapsedTimeMillis();

	// -- Particles pass 4
	real_t gu, gv;
	for (int ip = 0; ip < numParticles; ip++) {
		MPMParticle *p = particles[ip];

		gu = 0;
		gv = 0;

		real_t *px = p->px;
		real_t *py = p->py;
		int pcy = p->cy;
		int pcx = p->cx;
		for (int i = 0; i < 3; i++) {
			std::vector<MPMNode *> &nrow = grid[pcx + i];
			real_t ppxi = px[i];
			for (int j = 0; j < 3; j++) {
				MPMNode *nj = nrow[pcy + j];
				phi = ppxi * py[j];
				gu += phi * nj->u;
				gv += phi * nj->v;
			}
		}

		p->x += (p->gu = gu);
		p->y += (p->gv = gv);
		p->u += smoothing * (gu - p->u);
		p->v += smoothing * (gv - p->v);
	}

	//----------------------------------
	long t4 = ofGetElapsedTimeMillis();

	long dt0 = t1 - t0;
	long dt1 = t2 - t1;
	long dt2 = t3 - t2;
	long dt3 = t4 - t3;
	long dt = t4 - t0;
	elapsed = 0.95 * elapsed + 0.05 * (dt);

	// Timing: in case you're curious about CPU consumption, uncomment this:
	// printf("Elapsed = %d	%d	%d	%d	%f\n", dt0, dt1, dt2, dt3, elapsed);
}

void MPMFluid::draw(CanvasItem *canvas) {
	ERR_FAIL_NULL(canvas);

	// Draw the active particles as a short line,
	// using their velocity for their length.
	Vector<Point2> verts;

	for (int ip = 0; ip < numParticles; ip++) {
		MPMParticle *p = particles[ip];
		verts.push_back(Vector2(p->x, p->y));
		verts.push_back(Vector2(p->x - p->u, p->y - p->v));
	}
	canvas->draw_lines(verts, Color(255, 255, 255, 204), Transform2(0, Size2(scaleFactor, scaleFactor)));
}

std::vector<MPMParticle *> &MPMFluid::getParticles() { return particles; }

int MPMFluid::getGridSizeX() { return gridSizeX; }

int MPMFluid::getGridSizeY() { return gridSizeY; }
