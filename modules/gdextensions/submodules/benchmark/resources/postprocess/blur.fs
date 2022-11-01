shader_type canvas_item;

#ifdef GL_ES
#ifdef GL_FRAGMENT_PRECISION_HIGH
 precision highp float;
#else
 precision mediump float;
#endif
#endif

// Input vertex attributes (from vertex shader)
varying vec4 fragColor;

// Input uniform values
uniform vec2 resolution;

// NOTE: Add here your custom variables

vec3 offset = vec3(0.0, 1.3846153846, 3.2307692308);
vec3 weight = vec3(0.2270270270, 0.3162162162, 0.0702702703);

void fragment()
{
    // Texel color fetching from texture sampler
    vec3 tc = texture(TEXTURE, UV).rgb*weight.x;

    tc += texture(TEXTURE, UV + vec2(offset.y)/resolution.x, 0.0).rgb*weight.y;
    tc += texture(TEXTURE, UV - vec2(offset.y)/resolution.x, 0.0).rgb*weight.y;

    tc += texture(TEXTURE, UV + vec2(offset.z)/resolution.x, 0.0).rgb*weight.z;
    tc += texture(TEXTURE, UV - vec2(offset.z)/resolution.x, 0.0).rgb*weight.z;

    COLOR = vec4(tc, 1.0);
}
