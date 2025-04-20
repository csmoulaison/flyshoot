#version 450

layout(location = 0) in vec2 in_pos;

// Question: how do we bind just a small segment like this. Trivially?
// We don't want to copy+paste the ubo_global definition between shaders,
// OBVIOUSLY!

layout(binding = 0) uniform ubo_reticle {
	vec2 reticle_pos;
} global;

void main() {
	gl_Position = vec4(global.reticle_pos + in_pos, 0.0, 1.0);
}

