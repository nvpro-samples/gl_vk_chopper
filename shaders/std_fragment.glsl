/*
 * Copyright (c) 2014-2024, NVIDIA CORPORATION.  All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * SPDX-FileCopyrightText: Copyright (c) 2014-2024 NVIDIA CORPORATION
 * SPDX-License-Identifier: Apache-2.0
 */


#version 440 core

layout(location=0) in VS_OUT{
	vec3 wpos;
	vec3 nml;
	vec2 uv;
	flat ivec4 lut;
} vs_in;

struct CameraData{
	mat4 proj_view_matrix;
	mat4 inverse_proj_view_matrix;
	vec4 camera_position;
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


layout(set = 1, binding = 0) uniform sampler2D tex[6];

layout(set = 0,binding = 0) uniform samplerCube env;


void main(){

	vec3 light_pos = vec3(-100.0, 100.0, 100.0);
	vec3 light_vector = normalize(light_pos - vs_in.wpos.xyz);
	vec3 view_pos = camera.camera_position.xyz;
	vec3 view_vector = normalize(view_pos - vs_in.wpos.xyz);

	const float minFog = 80.0;
	const float maxFog = 360.0;
	float fogRng = maxFog - minFog;

	float pLen = length(vs_in.wpos.xyz);
	float fogFactor = clamp((pLen - minFog) / fogRng,0.0,0.85);
	
	vec3 nml = normalize(vs_in.nml);
	vec3 ref_vector = 2.0 * dot(vs_in.nml.xyz, view_vector) * vs_in.nml.xyz - view_vector;
	vec3 light_ref_vector = 2.0 * dot(vs_in.nml.xyz, light_vector) * vs_in.nml.xyz - light_vector;

	float diffuse = clamp(dot(light_vector, normalize(vs_in.nml.xyz)), 0.1, 1.0);
	float spec = pow(clamp(dot(light_ref_vector, view_vector), 0.0, 1.0), 1.0*material.shininess);

	vec4 texColor = texture(tex[vs_in.lut.x],vs_in.uv.xy);
	vec2 lod = textureQueryLod(tex[vs_in.lut.x], vs_in.uv.xy);
	vec4 refColor = texture(env, ref_vector);
	vec3 combinedColor = mix(texColor.xyz, refColor.xyz, material.reflectivity*2.0)*diffuse + spec;
	vec3 combinedColor2 = mix(combinedColor, vec3(1.0), fogFactor);

	out_color = vec4(combinedColor2, texColor.w);
}

