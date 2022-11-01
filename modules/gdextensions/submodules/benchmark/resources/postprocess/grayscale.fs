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

// NOTE: Add here your custom variables

void fragment()
{
    // Texel color fetching from texture sampler
    vec4 texelColor = texture(TEXTURE, UV)*colDiffuse*fragColor;

    // Convert texel color to grayscale using NTSC conversion weights
    float gray = dot(texelColor.rgb, vec3(0.299, 0.587, 0.114));

    // Calculate final fragment color
    COLOR = vec4(gray, gray, gray, texelColor.a);
}
