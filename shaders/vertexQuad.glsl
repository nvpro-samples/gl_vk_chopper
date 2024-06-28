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


#version 450 core

struct CameraData{
	mat4 proj_view_matrix;
	mat4 inverse_proj_view_matrix;
	vec4 camera_position;
};

layout(std140, binding = 1) uniform cameraBuffer{
	CameraData camera;
};


in layout(location = 0) vec4 pos;
in layout(location = 1) vec2 uv;

layout(location=0) out VS_OUT{
	vec3 uv;
} vs_out;

void main(){
	// Copy the NDC vertex positions to the output:
	gl_Position = vec4(pos.xyz, 1.0);

	// Given an NDC coordinate `pos`, we want to know what input direction
	// would have generated that position.
	// One answer is to back-project from the far plane:
	vs_out.uv = vec3(camera.inverse_proj_view_matrix * vec4(pos.xy, 1.0, 1.0) + camera.camera_position);
}

