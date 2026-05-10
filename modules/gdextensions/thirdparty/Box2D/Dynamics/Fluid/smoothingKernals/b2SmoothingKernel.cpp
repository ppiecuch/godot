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
module blaze.dynamics.fluid.smoothingKernals.bzSmoothingKernel;

import blaze.common.bzMath;


public abstract class bzSmoothingKernel {
    float m_factor;
    float m_kernelSize;
    float m_kernelSizeSq;
    float m_kernelSize3;

    /// <summary>
    /// Initializes a new instance of the <see cref="bzSmoothingKernel"/> class.
    /// </summary>
    this() {
        this(1.0f);
    }

    /// <summary>
    /// Initializes a new instance of the <see cref="bzSmoothingKernel"/> class.
    /// </summary>
    /// <param name="kernelSize">Size of the kernel.</param>
    this(float kernelSize) {
        this.m_factor = 1.0f;
        this.m_kernelSize = kernelSize;
        m_kernelSizeSq = m_kernelSize * m_kernelSize;
        m_kernelSize3 = m_kernelSize * m_kernelSize * m_kernelSize;
        calculateFactor();
    }

    protected abstract void calculateFactor();

    public abstract float calculate(bzVec2 distance);

    public abstract bzVec2 calculateGradient(bzVec2 distance);

    public abstract float calculateLaplacian(bzVec2 distance);
}

