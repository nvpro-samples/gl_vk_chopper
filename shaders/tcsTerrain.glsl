#version 450 core
#extension GL_KHR_vulkan_glsl : require

layout(vertices = 4) out;

struct CameraData{
	mat4 proj_matrix;
	mat4 view_matrix;
	vec4 cameraPosition;
};

layout(std140, binding = 1) uniform cameraBuffer{
	CameraData camera;
};


in VS_OUT{
	vec4 pos;
	vec2 uv;
} tcs_in[];


out VS_OUT{
	vec4 pos;
	vec2 uv;
} tcs_out[];



void main(){


	if (gl_InvocationID == 0){
		float tf = 16;
		gl_TessLevelOuter[0] = tf;
		gl_TessLevelOuter[1] = tf;
		gl_TessLevelOuter[2] = tf;
		gl_TessLevelOuter[3] = tf;
		gl_TessLevelInner[0] = tf;
		gl_TessLevelInner[1] = tf;

		}

	tcs_out[gl_InvocationID].pos = tcs_in[gl_InvocationID].pos;
	tcs_out[gl_InvocationID].uv = tcs_in[gl_InvocationID].uv;



}

