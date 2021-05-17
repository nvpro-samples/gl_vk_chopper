/*
 * Copyright (c) 2014-2021, NVIDIA CORPORATION.  All rights reserved.
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
 * SPDX-FileCopyrightText: Copyright (c) 2014-2021 NVIDIA CORPORATION
 * SPDX-License-Identifier: Apache-2.0
 */


#version 450 core

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

layout(location=0) out VS_OUT{
	vec4 pos;
	vec2 uv;
} vs_out;

void main(){

	int sz = 16;

	float terrainScale = 32.0;

	float offsetX = float(gl_InstanceIndex % sz);
	float offsetZ = float(gl_InstanceIndex / sz);

	vec2 baseUV = vec2(1.0 /sz) * vec2(offsetX,offsetZ);

	vs_out.uv = baseUV + (uv / sz);

	vs_out.pos = vec4((pos.xyz*terrainScale) + vec3(offsetX*terrainScale*2.0, 0.0, offsetZ*terrainScale*2.0) -vec3(220.0, 10.0, 240.0), 1.0);

	gl_Position = vs_out.pos;
}

