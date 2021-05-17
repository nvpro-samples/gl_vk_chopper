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

layout(vertices = 4) out;

struct CameraData{
	mat4 proj_matrix;
	mat4 view_matrix;
	vec4 cameraPosition;
};

layout(std140, binding = 1) uniform cameraBuffer{
	CameraData camera;
};


layout(location=0) in VS_OUT{
	vec4 pos;
	vec2 uv;
} tcs_in[];


layout(location=0) out VS_OUT{
	vec4 pos;
	vec2 uv;
} tcs_out[];



void main(){


	if (gl_InvocationID == 0){
		float tf = 16.0;
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

