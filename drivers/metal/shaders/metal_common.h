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

static float random(float2 p)
{
    return fract(sin(dot(p, float2(15.79, 81.93)) * 45678.9123));
}

static float noise(float2 p)
{
    float2 i = floor(p);
    float2 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);
    float bottom = mix(random(i + float2(0)), random(i + float2(1.0, 0.0)), f.x);
    float top = mix(random(i + float2(0.0, 1.0)), random(i + float2(1)), f.x);
    float t = mix(bottom, top, f.y);
    return t;
}

static float fbm(float2 uv)
{
    float sum = 0;
    float amp = 0.7;
    for(int i = 0; i < 4; ++i)
    {
        sum += noise(uv) * amp;
        uv += uv * 1.2;
        amp *= 0.4;
    }
    return sum;
}


// Shaders

fragment float4 dummyFS()
{
    return float4(0, 0, 0, 0);
}

#endif // METAL_COMMON_H
