#ifdef GL_ES
#ifdef GL_FRAGMENT_PRECISION_HIGH
 precision highp float;
#else
 precision mediump float;
#endif
#endif

uniform vec2 screenHalfSize;
uniform sampler2D texture0;

attribute vec2 vertPosition_screenspace;
attribute vec2 texCoord0;

varying vec2 vTexCoord;

void fragment()
{
    gl_FragColor = vec4(1, 1, 1, texture2D(texture0, vTexCoord).a);
}

void vertex()
{
    // map screen coordinates to [-1..1][-1..1]
    vec2 vertPosition_clipspace = (vertPosition_screenspace / screenHalfSize) - vec2(1,1);
    // invert Y coordinate, because screenspace has its origin at the upper-left instead of lower-left
    gl_Position =  vec4(vertPosition_clipspace.x,-vertPosition_clipspace.y,0,1);

    vTexCoord = texCoord0;
}
