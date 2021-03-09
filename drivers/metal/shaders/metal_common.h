#ifndef METAL_COMMON_H
#define METAL_COMMON_H

#ifndef SKIP_STD_HEADERS
# include <simd/simd.h>
# include <metal_stdlib>
#endif

using namespace metal;

// Full screen triangle's vertices
constant float2 gCorners[3] = { float2(-1.0f, -1.0f), float2(3.0f, -1.0f), float2(-1.0f, 3.0f) };


// Functions

#define MTL_KERNEL_GUARD(IDX, MAX_COUNT) \
    if (IDX >= MAX_COUNT)                \
    {                                    \
        return;                          \
    }

static float linstep( float low, float high, float v )
{
    return clamp( (v - low) / (high - low), 0.0f, 1.0f );
}


// Shaders

fragment float4 dummyFS()
{
    return float4(0, 0, 0, 0);
}

#endif // METAL_COMMON_H
