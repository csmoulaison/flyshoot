#version 450

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec3 in_color;

layout(location = 0) out vec3 frag_color;

layout(binding = 0) uniform ubo_global {
	mat4 view;
	mat4 projection;
	vec3 clear_color;
	float max_draw_distance_z;
} global;

layout(binding = 1) uniform ubo_inst {
	mat4 model;
} inst;

void main() {
	vec4 projection = global.projection * global.view * inst.model * vec4(in_pos, 1.0);
    gl_Position = projection;

    vec3 base = mix(in_color, global.clear_color, pow(clamp(projection.z / global.max_draw_distance_z, 0, 1), 0.5));
    frag_color = base;
}
