#include "_metal_common.h"
#include "_godot_common.h"


#ifdef VERTEX_VEC3_USED
#else
#endif

#ifdef USE_ATTRIB_LIGHT_ANGLE
#endif

#ifdef USE_ATTRIB_MODULATE
#endif

#ifdef USE_ATTRIB_MODULATE
#else
#endif

#ifdef USE_ATTRIB_LARGE_VERTEX
#endif

#ifdef USE_SKELETON
#endif

#ifdef USE_TEXTURE_RECT
#else

#ifdef USE_INSTANCING

#ifdef USE_INSTANCE_CUSTOM
#endif

#endif

#endif


vertex float4 vertexFunction (
	const device attributes_t* vertexArray [[ buffer(0) ]],
	constant uniforms_t& uniforms [[ buffer(1) ]],
	unsigned int vID [[ vertex_id ]]
){
	return float4(vertexArray[vID].position, 1.0);
}

fragment half4 fragmentFunction (
	constant uniforms_t& uniforms [[ buffer(1) ]]
){
	return half4(1.0);
}
