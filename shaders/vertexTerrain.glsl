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

layout(std140, binding = 1) uniform cameraBuffer{
	CameraData camera;
};


in layout(location = 0) vec4 pos;
in layout(location = 1) vec2 uv;

out VS_OUT{
	vec4 pos;
	vec2 uv;
} vs_out;

void main(){

	int sz = 16;

	float terrainScale = 32.0;

	float offsetX = float(gl_InstanceID % sz);
	float offsetZ = float(gl_InstanceID / sz);

	vec2 baseUV = vec2(1.0 /sz) * vec2(offsetX,offsetZ);

	vs_out.uv = baseUV + (uv / sz);

	vs_out.pos = vec4((pos.xyz*terrainScale) + vec3(offsetX*terrainScale*2.0, 0.0, offsetZ*terrainScale*2.0) -vec3(220.0, 10.0, 240.0), 1.0);

	gl_Position = vs_out.pos;
}

