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
uniform vec4 colDiffuse;
uniform vec2 resolution;            // render size

// NOTE: Add here your custom variables

const float samples = 5.0;          // pixels per axis; higher = bigger glow, worse performance
const float quality = 2.5;          // lower = smaller glow, better quality

void fragment()
{
    vec4 sum = vec4(0);
    vec2 sizeFactor = vec2(1)/resolution*quality;

    // Texel color fetching from texture sampler
    vec4 source = texture(TEXTURE, UV);

    const int range = 2;            // should be = (samples - 1)/2;

    for (int x = -range; x <= range; x++)
    {
        for (int y = -range; y <= range; y++)
        {
            sum += texture(TEXTURE, UV + vec2(x, y)*sizeFactor);
        }
    }

    // Calculate final fragment color
    COLOR = ((sum/(samples*samples)) + source)*colDiffuse;
}
