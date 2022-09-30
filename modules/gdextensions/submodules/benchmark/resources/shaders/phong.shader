#ifdef GL_ES
#ifdef GL_FRAGMENT_PRECISION_HIGH
 precision highp float;
#else
 precision mediump float;
#endif
#endif

// input constants
uniform vec3 lightPosition;
uniform sampler2D texture0;
uniform mat4 view;
uniform mat4 mvp;

// input variables, different for each vertex
attribute vec3 vertexPosition;
attribute vec3 vertexNormal;
attribute vec2 vertexTexCoord;

// outputs
varying vec3 position_viewspace;
varying vec3 normal_viewspace;
varying vec2 texCoord0;

void fragment()
{
    // normal vector in viewspace
    // we must renormalize the normal vector, because the vertex shader might have scaled it
    vec3 N = normalize(normal_viewspace);

    // light vector: points from the surface towards the light source
    vec3 L = normalize(lightPosition - position_viewspace);

    // eye vector: points from the surface towards the camera
    // we are already in viewspace, so camera position is (0,0,0)
    vec3 E = normalize(-position_viewspace);

    // reflection vector: reflects the light around the normal vector
    vec3 R = -reflect(L,N);

    // ambient lighting: a better shader would pass this as an input rather than hard-coding it
    float ambient = 0.1;

    // diffuse lighting: clamp the dot product to 0, so it doesn't go negative when the light is
    // behind the surface
    float diffuse = max(0.0, dot(N,L));

    // specular lighting: a better shader would pass the shininess as an input - here we assume
    // the shininess is 10. clamp the dot product to 0, so it doesn't go negative when the
    // reflection points away from the camera.
    float specular = 0.0;
    if (diffuse > 0.0)
        specular = pow(max(0.0, dot(R,E)), 10.0);

    // final color: a better shader would pass the light's color as an input - here we assume
    // the light is bright white.
    vec3 lightColor = vec3(1.0, 1.0, 1.0);
    vec4 lightTotal = vec4((ambient + diffuse + specular) * lightColor, 1.0);
    gl_FragColor =  lightTotal * texture2D(texture0, texCoord0);
}

void vertex()
{
    // pass the texture coordinate unchanged
    texCoord0 = vertexTexCoord;

    // vertex position in viewspace is needed for lighting in the fragment shader
    position_viewspace = vec3(view * vec4(vertexPosition, 1));

    // normal vector in viewspace is also needed for lighting in the fragment shader
    // this math is only correct if view (modelview) matrix has a uniform scale! Otherwise use its inverse transpose.
    normal_viewspace = vec3(view * vec4(vertexNormal,0));

    // OpenGL needs the fully-projected vertex position for rendering the triangle
    gl_Position = mvp * vec4(vertexPosition, 1);
}
