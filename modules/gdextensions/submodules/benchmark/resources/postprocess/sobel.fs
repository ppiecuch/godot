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
vec2 resolution = vec2(800.0, 450.0);

void fragment()
{
	float x = 1.0/resolution.x;
	float y = 1.0/resolution.y;

	vec4 horizEdge = vec4(0.0);
	horizEdge -= texture(TEXTURE, vec2(UV.x - x, UV.y - y))*1.0;
	horizEdge -= texture(TEXTURE, vec2(UV.x - x, UV.y    ))*2.0;
	horizEdge -= texture(TEXTURE, vec2(UV.x - x, UV.y + y))*1.0;
	horizEdge += texture(TEXTURE, vec2(UV.x + x, UV.y - y))*1.0;
	horizEdge += texture(TEXTURE, vec2(UV.x + x, UV.y    ))*2.0;
	horizEdge += texture(TEXTURE, vec2(UV.x + x, UV.y + y))*1.0;

	vec4 vertEdge = vec4(0.0);
	vertEdge -= texture(TEXTURE, vec2(UV.x - x, UV.y - y))*1.0;
	vertEdge -= texture(TEXTURE, vec2(UV.x    , UV.y - y))*2.0;
	vertEdge -= texture(TEXTURE, vec2(UV.x + x, UV.y - y))*1.0;
	vertEdge += texture(TEXTURE, vec2(UV.x - x, UV.y + y))*1.0;
	vertEdge += texture(TEXTURE, vec2(UV.x    , UV.y + y))*2.0;
	vertEdge += texture(TEXTURE, vec2(UV.x + x, UV.y + y))*1.0;

	vec3 edge = sqrt((horizEdge.rgb*horizEdge.rgb) + (vertEdge.rgb*vertEdge.rgb));

	COLOR = vec4(edge, texture(TEXTURE, UV).a);
}
