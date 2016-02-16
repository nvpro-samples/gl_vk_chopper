#version 450 core
#extension GL_KHR_vulkan_glsl : require

in VS_OUT{
	vec4 pos;
	vec3 uv;
} vs_in;



layout(location = 0) out vec4 out_color;
layout(binding = 1) uniform samplerCube env;

void main(){

	vec3 nmlPos = normalize(vs_in.uv);

	vec3 envColor = texture(env, nmlPos).xyz;

	out_color = vec4(mix(envColor, vec3(1.0), 0.7), 1.0);
}

