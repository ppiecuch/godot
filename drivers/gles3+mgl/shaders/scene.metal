#include "_metal_common.h"
#include "_godot_common.h"

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
