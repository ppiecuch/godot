/*
 *  Copyright (c) 2008 Rene Schulte. http://www.rene-schulte.info/
 *  Ported to D by Mason Green. http:/www.dsource.org/projects/blaze
 *
 *   This software is provided 'as-is', without any express or implied
 *   warranty. In no event will the authors be held liable for any damages
 *   arising from the use of this software.
 *
 *   Permission is granted to anyone to use this software for any purpose,
 *   including commercial applications, and to alter it and redistribute it
 *   freely, subject to the following restrictions:
 *
 *   1. The origin of this software must not be misrepresented; you must not
 *   claim that you wrote the original software. If you use this software
 *   in a product, an acknowledgment in the product documentation would be
 *   appreciated but is not required.
 *
 *   2. Altered source versions must be plainly marked as such, and must not be
 *   misrepresented as being the original software.
 *
 *   3. This notice may not be removed or altered from any source
 *   distribution.
 */

// Documentation comments modified by Brian Schott (SirAlaran)

module blaze.dynamics.fluid.bzSPHSimulation;

import blaze.common.bzMath;
import blaze.common.bzConstants;
import blaze.bzWorld;
import blaze.collision.shapes.bzFluidParticle;
import blaze.dynamics.fluid.smoothingKernals.bzSmoothingKernel;
import blaze.dynamics.fluid.smoothingKernals.bzSKPoly6;
import blaze.dynamics.fluid.smoothingKernals.bzSKSpiky;
import blaze.dynamics.fluid.smoothingKernals.bzSKViscosity;

/**
 * Implementation of a SPH-based fluid simulation
 */
public class bzSPHSimulation {

    public float cellSpace;
    public bzSmoothingKernel SKGeneral;
    public bzSmoothingKernel SKPressure;
    public bzSmoothingKernel SKViscos;
    public float Viscosity;

	bzWorld world;

    private bzFluidParticle[] m_particles;

    this(bzWorld world) {
		this.world = world;
        cellSpace = CELL_SPACE;
        Viscosity = VISCOSCITY;
        SKGeneral = new bzSKPoly6(cellSpace);
        SKPressure = new bzSKSpiky(cellSpace);
        SKViscos = new bzSKViscosity(cellSpace);
    }

    /**
     * Add particle to simulation
     * Params: particle = the particle to add
     */
    void addParticle(bzFluidParticle particle) {
        particle.ID = cast(ushort)(m_particles.length);
		particle.neighbors[particle.ID] = particle;
        m_particles ~= particle;
    }

    /**
     * Remove particle from simulation
     * Params: particle = the particle to remove
     */
    void removeParticle(bzFluidParticle particle) {
        bzKill(m_particles, particle);
    }

    /**
     * Returns: the number of particles
     */
    int numParticles() {
        return m_particles.length;
    }

    /**
     * Returns: the particles in the simulation as an array
     */
    bzFluidParticle[] particles() {
        return m_particles;
    }

    /**
     * Resets the forces of all the particles to zero
     */
    void resetForce() {
        foreach(p; m_particles) {
            p.force = bzVec2.zeroVect;
        }
    }

    /**
     * Simulates the particles.
     * Params:
     *     gravity = The gravity
     *     step = The time step
     */
    void update(bzVec2 gravity, bzTimeStep step) {
        calculatePressureAndDensities();
        calculateForces();
        updateParticles(step, gravity);
        checkParticleDistance();
        foreach (ref particle; m_particles) {
			bzXForm foo, bar;
            particle.synchronize(world.broadPhase, foo, bar);
        }
    }

    /**
     * Calculates the pressure and densities.
     */
    private void calculatePressureAndDensities() {
        bzVec2 dist;
        foreach (ref particle; m_particles) {
            particle.density = 0.0f;
            foreach (neighbor; particle.neighbors) {
                dist = particle.position - neighbor.position;
                particle.density += (particle.mass * SKGeneral.calculate(dist));
            }
            particle.updatePressure();
        }
    }

    /**
     * Calculates the pressure and viscosity forces.
     */
    private void calculateForces() {
        bzVec2 dist, force;
        float scalar;
        for (int i = 0; i < m_particles.length; i++) {
            foreach (neighbor; m_particles[i].neighbors) {
				// Prevent double tests
				if (neighbor.ID < i) {
                    if (neighbor.density > float.epsilon) {
                        dist = m_particles[i].position - neighbor.position;
                        // pressure
                        scalar = neighbor.mass * (m_particles[i].pressure
                                                         + neighbor.pressure) / (2.0f * neighbor.density);
                        force = SKPressure.calculateGradient(dist);
                        force *= scalar;
                        m_particles[i].force -= force;
                        neighbor.force += force;

                        // viscosity
                        scalar = neighbor.mass * SKViscos. calculateLaplacian(dist)
                                 * Viscosity * 1 / neighbor.density;
                        force = neighbor.velocity - m_particles[i].velocity;
                        force *= scalar;
                        m_particles[i].force += force;
                        neighbor.force -= force;
                    }
                }
            }
        }
    }


    /**
     * Updates the particles positions using integration and clips them to the
     * domain space.
     * Params:
     *     step = the time step
     *     gravity = the gravity applied to the simulation
     */
    private void updateParticles(bzTimeStep step, bzVec2 gravity) {
        foreach (particle; m_particles) {
            // Update velocity + position using forces
            particle.update(step, gravity);
        }
    }

    /**
     * Checks the distance between the particles and corrects it, if they are
     * too near.
     */
    private void checkParticleDistance() {
        float minDist = 0.5f * cellSpace;
        float minDistSq = minDist * minDist;
        for (int i = 0; i < m_particles.length; i++) {
            foreach (neighbor; m_particles[i].neighbors) {
                bzVec2 dist = neighbor.position - m_particles[i].position;
                float distLenSq = dist.lengthSquared;
                if (distLenSq < minDistSq) {
                    if (distLenSq > float.epsilon) {
                        float distLen = sqrt(distLenSq);
                        dist *= (0.5f * (distLen - minDist) / distLen);
                        neighbor.position -= dist;
                        neighbor.positionOld -= dist;
                        m_particles[i].position += dist;
                        m_particles[i].positionOld += dist;
                    } else {
                        float diff = 0.5f * minDist;
                        neighbor.position.y -= diff;
                        neighbor.positionOld.y -= diff;
                        m_particles[i].position.y += diff;
                        m_particles[i].positionOld.y += diff;
                    }
                }
            }
        }
    }
}
