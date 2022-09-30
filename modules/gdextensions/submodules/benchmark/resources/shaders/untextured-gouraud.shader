#ifdef GL_ES
#ifdef GL_FRAGMENT_PRECISION_HIGH
 precision highp float;
#else
 precision mediump float;
#endif
#endif

 // input variables, different for each fragment
varying vec4 lightTotal;

void fragment()
{
    gl_FragColor =  lightTotal; 
}
