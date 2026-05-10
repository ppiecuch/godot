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
module blaze.dynamics.fluid.smoothingKernals.bzSKSpiky;

import blaze.common.bzMath;
import blaze.dynamics.fluid.smoothingKernals.bzSmoothingKernel;

/// <summary>
/// Implementation of the Spiky Smoothing-Kernel for SPH-based fluid simulation
/// </summary>
public class bzSKSpiky : bzSmoothingKernel {

    this() {
        super();
    }

    this(float kernelSize) {
        super(kernelSize);
    }

    protected override void calculateFactor() {
        float kernelRad6 = pow(m_kernelSize, 6.0f);
        m_factor = (15.0f / (PI * kernelRad6));
    }

    public override float calculate(bzVec2 distance) {
        float lenSq = distance.lengthSquared;

        if (lenSq > m_kernelSizeSq) {
            return 0.0f;
        }
        if (lenSq < float.epsilon) {
            lenSq = float.epsilon;
        }
        float f = m_kernelSize - sqrt(lenSq);
        return m_factor * f * f * f;
    }

    public override bzVec2 calculateGradient(bzVec2 distance) {
        float lenSq = distance.lengthSquared;
        if (lenSq > m_kernelSizeSq) {
            return bzVec2.zeroVect;
        }
        if (lenSq < float.epsilon) {
            lenSq = float.epsilon;
        }
        float len = sqrt(lenSq);
        float f = -m_factor * 3.0f * (m_kernelSize - len) * (m_kernelSize - len) / len;
        return bzVec2(distance.x * f, distance.y * f);
    }

    public override float calculateLaplacian(bzVec2 distance) {
        throw new Exception("Not yet implemented!");
    }
}
