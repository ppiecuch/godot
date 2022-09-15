#ifdef GL_ES
#ifdef GL_FRAGMENT_PRECISION_HIGH
 precision highp float;
#else
 precision mediump float;
#endif
#endif

// input constants
uniform mat4 mvp;

// input variables, different for each vertex
attribute vec3 vertexPosition;
attribute vec3 vertexNormal;

// outputs
varying vec4 color;

void fragment()
{
    gl_FragColor = color;
}

void vertex() {
{
    // vertex color = vertex normal
    color = vec4(vertexNormal, 1.0);

    // OpenGL needs the fully-projected vertex position for rendering the triangle
    gl_Position = mvp * vec4(vertexPosition, 1);
}
}
