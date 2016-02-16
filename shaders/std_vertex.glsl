#version 440 core
#extension GL_KHR_vulkan_glsl : require

struct SceneData{
	mat4 view_matrix;
	mat4 nml_matrix;
	ivec4 lut;
	vec4 lutPad[3];
};

struct InstanceData{
	mat4 flight_matrix;
};

struct CameraData{
	mat4 proj_matrix;
	mat4 view_matrix;
	vec4 cameraPosition;
};

layout(std140, set=0, binding = 2) uniform sceneBuffer{
	//Scene data for each draw.
	SceneData scene[100];
}scn;

layout(std140,set=0, binding = 1) uniform cameraBuffer{
	CameraData camera;
};

layout(std140, set=2, binding = 0) uniform transformBuffer{
	InstanceData instdata[64];
}tra;

in layout(location = 0) vec4 pos;
in layout(location = 1) vec4 nml;

out VS_OUT{
	vec4 pos;
	vec4 wpos;
	vec4 nml;
	vec2 uv;
	flat ivec4 lut;
} vs_out;

void main(){
	vs_out.uv = vec2(pos.w,nml.w);

	int instCount = scn.scene[0].lut.y;
	int bufferIndex = gl_InstanceIndex / instCount;
	int instanceIndex = gl_InstanceIndex % instCount;


	vs_out.nml = vec4((mat3(tra.instdata[instanceIndex%4].flight_matrix) * mat3(scn.scene[bufferIndex].nml_matrix)) * nml.xyz, 1.0);

	mat4 iMat = scn.scene[bufferIndex].view_matrix ;

	vs_out.wpos = (tra.instdata[instanceIndex].flight_matrix * (iMat * vec4(pos.xyz, 1.0)));
	vs_out.pos = camera.proj_matrix * vs_out.wpos;
	vs_out.pos.y = -vs_out.pos.y;
	vs_out.pos.z = (vs_out.pos.z + vs_out.pos.w) / 2.0;
	vs_out.lut = scn.scene[bufferIndex].lut;

	gl_Position = vs_out.pos;
}