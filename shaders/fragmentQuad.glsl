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

layout(location=0) in VS_OUT{
	vec3 uv;
} vs_in;

layout(location = 0) out vec4 out_color;
layout(binding = 0) uniform samplerCube env;

void main(){
	// Sample from the cube map.
	vec3 nmlPos = normalize(vs_in.uv);
	vec3 envColor = texture(env, nmlPos).xyz;

	// The water plane fades out to 85% fog in the distance.
	// Let's add some fog to the horizon to match:
	envColor = mix(envColor, vec3(1.0), 0.85 * (1.f - abs(nmlPos.y)));

	out_color = vec4(envColor, 1.0);
}

