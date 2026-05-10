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
module blaze.dynamics.fluid.smoothingKernals.bzSKViscosity;

import blaze.common.bzMath;
import blaze.dynamics.fluid.smoothingKernals.bzSmoothingKernel;

/// <summary>
/// Implementation of the Viscosity Smoothing-Kernel for SPH-based fluid simulation
/// </summary>
public class bzSKViscosity : bzSmoothingKernel {
    this() {
        super();
    }

    this(float kernelSize) {
        super(kernelSize);
    }

    protected override void calculateFactor() {
        m_factor = (15.0f / (2.0f * PI * m_kernelSize3));
    }

    public override float calculate(bzVec2 distance) {
        float lenSq = distance.lengthSquared;
        if (lenSq > m_kernelSizeSq) {
            return 0.0f;
        }
        if (lenSq < float.epsilon) {
            lenSq = float.epsilon;
        }
        float len = sqrt(lenSq);
        float len3 = len * len * len;
        return m_factor * (((-len3 / (2.0f * m_kernelSize3)) + (lenSq / m_kernelSizeSq) + (m_kernelSize / (2.0f * len))) - 1.0f);
    }

    public override float calculateLaplacian(bzVec2 distance) {
        float lenSq = distance.lengthSquared;
        if (lenSq > m_kernelSizeSq) {
            return 0.0f;
        }
        if (lenSq < float.epsilon) {
            lenSq = float.epsilon;
        }
        float len = sqrt(lenSq);
        return m_factor * (6.0f / m_kernelSize3) * (m_kernelSize - len);
    }

    public override bzVec2 calculateGradient(bzVec2 distance) {
        throw new Exception("Net yet implemented!");
    }
}
