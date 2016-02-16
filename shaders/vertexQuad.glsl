#version 450 core
#extension GL_KHR_vulkan_glsl : require
struct SceneData{
	mat4 view_matrix;
};

struct CameraData{
	mat4 proj_matrix;
	mat4 view_matrix;
	vec4 cameraPosition;
};

layout(std140, binding = 0) uniform sceneBuffer{
	SceneData scene;
};

layout(std140, binding = 2) uniform cameraBuffer{
	CameraData camera;
};


in layout(location = 0) vec4 pos;
in layout(location = 1) vec2 uv;

out VS_OUT{
	vec4 pos;
	vec3 uv;
} vs_out;

void main(){

	vec4 uvPos = camera.view_matrix * vec4(uv*2.0 - 1.0, -1.0, 1.0);

	vs_out.uv = uvPos.xyz;
	vs_out.uv.y *= -1;

	vs_out.pos =  vec4(pos.xyz,1.0);
	vs_out.pos = scene.view_matrix * vs_out.pos;
	vs_out.pos.y = -vs_out.pos.y;
	vs_out.pos.z = (vs_out.pos.z + vs_out.pos.w) / 2.0;
	gl_Position = vs_out.pos;
}

