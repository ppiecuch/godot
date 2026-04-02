shader_type spatial;
render_mode blend_mix, depth_draw_alpha_prepass, cull_disabled;

// OpenVAT Instanced Shader (Godot 3.x)
//
// Export settings in Blender OpenVAT:
//   "Vertex Normals" = "Packed"
//   "Use Single Row" checked
//   "Export Model" checked
//   "Model Format" = "glTF Binary"
//
// INSTANCE_CUSTOM channels:
//   R = animation offset (randomize instances playing the same track)
//   G = animation start frame
//   B = animation end frame
//   A = alpha (fade in/out per instance)

// OpenVAT
uniform sampler2D vertex_animation_texture;
uniform vec3 min_values;
uniform vec3 max_values;
uniform float speed = 30.0;

// Albedo
uniform vec4 albedo : hint_color = vec4(1.0);
uniform sampler2D albedo_texture : hint_albedo;

// Metallic
uniform sampler2D metallic_texture : hint_white;
uniform float metallic : hint_range(0.0, 1.0) = 0.0;
uniform vec4 metallic_texture_channel : hint_color = vec4(1.0, 0.0, 0.0, 0.0);
uniform float specular : hint_range(0.0, 1.0) = 0.5;

// Roughness
uniform sampler2D roughness_texture : hint_white;
uniform float roughness : hint_range(0.0, 1.0) = 1.0;
uniform vec4 roughness_texture_channel : hint_color = vec4(1.0, 0.0, 0.0, 0.0);

// Normal Map
uniform sampler2D normal_map_texture : hint_normal;
uniform float normal_scale : hint_range(-16.0, 16.0) = 1.0;

varying vec4 custom_data;

vec3 blenderToGodot(vec3 v) {
	return vec3(v.x, v.z, -v.y);
}

vec3 interpolateVatOffset(vec2 current_offset_uv, vec2 next_offset_uv, float blend) {
	vec3 current_offset = texture(vertex_animation_texture, current_offset_uv).rgb;
	vec3 next_offset = texture(vertex_animation_texture, next_offset_uv).rgb;
	return mix(current_offset, next_offset, blend);
}

vec3 rescalePosition(vec3 position) {
	return min_values + position * (max_values - min_values);
}

vec3 rescaleNormal(vec3 normal) {
	return 2.0 * normal - 1.0;
}

void vertex() {
	custom_data = INSTANCE_CUSTOM;

	float frame_time;
	float current_frame;

	float start_frame = custom_data.g;
	float end_frame = custom_data.b;
	float num_frames = end_frame - start_frame;
	float frame_offset = num_frames * custom_data.r;

	ivec2 resolution = textureSize(vertex_animation_texture, 0);

	float time_scale_normalized = speed / num_frames;
	frame_time = mod(TIME * time_scale_normalized, 1.0);
	current_frame = start_frame + mod((frame_time * num_frames) + frame_offset, num_frames);

	float next_frame = current_frame + 1.0;
	if (next_frame > end_frame) {
		next_frame = start_frame;
	}

	float blend = fract(frame_time);

	float frame_step = 1.0 / float(resolution.y);
	vec2 current_offset_uv = UV2 + vec2(0.0, current_frame * frame_step);
	vec2 next_offset_uv = UV2 + vec2(0.0, next_frame * frame_step);

	VERTEX += blenderToGodot(rescalePosition(
		interpolateVatOffset(current_offset_uv, next_offset_uv, blend)
	));

	vec2 normals_uv_shift = vec2(0.0, 0.5);
	NORMAL = normalize(blenderToGodot(rescaleNormal(
		interpolateVatOffset(
			current_offset_uv + normals_uv_shift,
			next_offset_uv + normals_uv_shift,
			blend
		)
	)));

	TANGENT = normalize(vec3(abs(NORMAL.y) + abs(NORMAL.z), 0.0, -abs(NORMAL.x)));
	BINORMAL = normalize(vec3(0.0, abs(NORMAL.x) + abs(NORMAL.z), -abs(NORMAL.y)));
}

void fragment() {
	ALBEDO = albedo.rgb * texture(albedo_texture, UV).rgb;
	METALLIC = dot(texture(metallic_texture, UV), metallic_texture_channel) * metallic;
	SPECULAR = specular;
	ROUGHNESS = dot(texture(roughness_texture, UV), roughness_texture_channel) * roughness;
	NORMAL_MAP = texture(normal_map_texture, UV).rgb;
	NORMAL_MAP_DEPTH = normal_scale;
	ALPHA = custom_data.a;
}
