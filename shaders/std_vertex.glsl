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

struct NodeUniform{
	mat4 node_matrix;
	mat4 inverse_node_matrix;
	ivec4 lut;
	vec4 lutPad[3];
};

struct InstanceData{
	mat4 flight_matrix;
};

struct CameraData{
	mat4 proj_view_matrix;
	mat4 inverse_proj_view_matrix;
	vec4 camera_position;
};

layout(std140, set=0, binding = 2) uniform nodeUniformBuffer{
	// Node uniform data for each draw.
	NodeUniform nodes[32];
};

layout(std140,set=0, binding = 1) uniform cameraBuffer{
	CameraData camera;
};

layout(std140, set=2, binding = 0) uniform transformBuffer{
	InstanceData instdata[384];
}tra;

in layout(location = 0) vec4 pos;
in layout(location = 1) vec4 nml;

layout(location=0) out VS_OUT{
	vec3 wpos;
	vec3 nml;
	vec2 uv;
	flat ivec4 lut;
} vs_out;

void main(){
	// Flip UVs vertically:
	vs_out.uv = vec2(pos.w, 1.0f - nml.w);

	int instCount = nodes[0].lut.y;
	int bufferIndex = gl_InstanceIndex / instCount;
	int instanceIndex = gl_InstanceIndex % instCount;
	mat4 flightMat = tra.instdata[instanceIndex].flight_matrix;

	// nml.xyz * mat3(...inverse_node_matrix) multiplies the normal by the
	// inverse transpose of the node matrix, which is the correct matrix to use
	// for normals.
	// We can use the flight matrix as-is, because we know it's only composed
	// of a translation and a rotation -- so its inverse transpose
	// is proportional to the matrix itself.
	vs_out.nml = mat3(flightMat) * (mat3(nodes[bufferIndex].inverse_node_matrix) * nml.xyz);

	vs_out.wpos = (flightMat * (nodes[bufferIndex].node_matrix * vec4(pos.xyz, 1.0))).xyz;
	vs_out.lut = nodes[bufferIndex].lut;

	gl_Position = camera.proj_view_matrix * vec4(vs_out.wpos, 1.0f);
}