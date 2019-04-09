/*-----------------------------------------------------------------------
  Copyright (c) 2014-2016, NVIDIA. All rights reserved.
  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:
   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   * Neither the name of its contributors may be used to endorse 
     or promote products derived from this software without specific
     prior written permission.
  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
  PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
  OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
-----------------------------------------------------------------------*/

#version 440 core

layout(location=0) in VS_OUT{
	vec4 pos;
	vec4 wpos;
	vec4 nml;
	vec2 uv;
	flat ivec4 lut;
} vs_in;

struct CameraData{
	mat4 proj_matrix;
	mat4 view_matrix;
	vec4 cameraPosition;
};

struct MaterialData{
	float reflectivity;
	float shininess;
	float opacity;

	float padding;
};

layout(location = 0) out vec4 out_color;
layout(set = 0,binding = 3) uniform materialBuffer{
	MaterialData material;
};

layout(std140, set=0, binding = 1) uniform cameraBuffer{
	CameraData camera;
};


layout(set = 1, binding = 0) uniform sampler2D tex[16];

layout(set = 0,binding = 0) uniform samplerCube env;


void main(){

	vec3 light_pos = vec3(-100.0, 100.0, 100.0);
	vec3 light_vector = normalize(light_pos - vs_in.wpos.xyz);
	vec3 view_pos = camera.cameraPosition.xyz;
	vec3 view_vector = normalize(view_pos - vs_in.wpos.xyz);

	const float minFog = 80.0;
	const float maxFog = 360.0;
	float fogRng = maxFog - minFog;

	float pLen = length(vs_in.pos.xyz);
	float fogFactor = clamp((pLen - minFog) / fogRng,0.0,0.85);
	

	vec3 ref_vector = 2.0 * dot(vs_in.nml.xyz, view_vector) * vs_in.nml.xyz - view_vector;
	vec3 light_ref_vector = 2.0 * dot(vs_in.nml.xyz, light_vector) * vs_in.nml.xyz - light_vector;

	float diffuse = clamp(dot(light_vector, normalize(vs_in.nml.xyz)), 0.1, 1.0);
	float spec = pow(clamp(dot(light_ref_vector, view_vector), 0.0, 1.0), 1.0*material.shininess);


	vec4 texColor = texture(tex[vs_in.lut.x],vs_in.uv.xy);
	vec4 refColor = texture(env, ref_vector);
	vec3 combinedColor = mix(texColor.xyz, refColor.xyz, material.reflectivity*2.0)*diffuse + spec;
	vec3 combinedColor2 = mix(combinedColor, vec3(1.0), fogFactor);


	out_color = vec4(combinedColor2, texColor.w);

}

